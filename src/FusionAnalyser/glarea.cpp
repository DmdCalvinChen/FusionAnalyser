/****************************************************************************
* FusionAnalyser - Digital Dental Model Analysis Software
*
* Copyright (C) 2024-2026 AI-Align (基骨智能)
*
* This file is part of FusionAnalyser.
*
* FusionAnalyser is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* FusionAnalyser is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with FusionAnalyser. If not, see <https://www.gnu.org/licenses/>.
*
* SPDX-License-Identifier: GPL-3.0-or-later
****************************************************************************/

#include <common/interfaces.h>

#include "glarea.h"
#include "mainwindow.h"
#include "multiViewer_Container.h"
#include "ml_default_decorators.h"
#include "meshshaderrender.h"

#include <wrap/gl/picking.h>
#include <wrap/gl/pick.h>
#include <wrap/qt/trackball.h>
#include <wrap/qt/col_qt_convert.h>
#include <wrap/qt/shot_qt.h>
#include <wrap/qt/checkGLError.h>
#include <wrap/qt/gl_label.h>
#include <wrap/gl/addons.h>
#include <vcg/complex/algorithms/update/color.h>
#include <vcg/complex/algorithms/intersection.h>
#include <thread>
#include <QApplication>
#include <QGuiApplication>
#include <QPainterPath>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>
#include <common/ml_shared_data_context.h>
#include <common/config.h>
#include <common_ext/data/dentalmanager.h>
#include <common_ext/util/utility_tools.h>
#include <UI_Common/guicontrolcustomdesign/custombuttondesignbase.h>
#include <UI_Common/guicontrolcustomdesign/collectfolded.h>
#include <UI_Common/guicontrolcustomdesign/texturemap.h>
#include <QDesktopWidget>
#include <QScreen>

#include "UI_Common/fusionViewGui/fusionViewGui.h"
#include "UI_Common/FusionAnalyserHomeGui/fusionanalyserhomegui.h"
#include "UI_Common/navAnabutton/navanabutton.h"

using namespace ui_common;

#define MODEL_MIN_SCALE_FACTOR 0.1f
#define MODEL_MAX_SCALE_FACTOR 8.0f

GLArea::GLArea(QWidget* parent, MultiViewer_Container* mvcont, RichParameterSet* current)
	: QGLWidget(parent, mvcont->sharedDataContext()), interrbutshow(false)
{
	makeCurrent();
	lightPos = Point3m(0, 0, 1);
	parentmultiview = mvcont;
	this->updateCustomSettingValues(*current);
	animMode = AnimNone;
	iRenderer = 0; //Shader support
	iEdit = 0;
	currentEditor = 0;
	suspendedEditor = false;
	lastModelEdited = 0;
	cfps = 0;
	lastTime = 0;
	hasToPick = false;
	hasToSelectMesh = false;
	hasToGetPickPos = false;
	takeSnapTile = false;
	activeDefaultTrackball = true;
	infoAreaVisible = false;
	trackBallVisible = false;
	currentShader = NULL;
	lastFilterRef = NULL;
	setAttribute(Qt::WA_DeleteOnClose, true);
	fov = fovDefault();
	clipRatioFar = 5;
	clipRatioNear = clipRatioNearDefault();
	nearPlane = .2f;
	farPlane = 5.f;

	id = mvcont->getNextViewerId();

	bWhetherOpenGlobalTrackball = true;

	updateMeshSetVisibilities();
	setAutoFillBackground(false);

	//Raster support
	_isRaster = false;
	opacity = 0.5;
	zoom = false;
	targetTex = 0;

	connect(this->md(), SIGNAL(currentMeshChanged(int)),
		this, SLOT(manageCurrentMeshChange()), Qt::QueuedConnection);
	connect(this->md(), SIGNAL(meshSetChanged()),
		this, SLOT(updateMeshSetVisibilities()));
	connect(this->md(), SIGNAL(documentUpdated()),
		this, SLOT(completeUpdateRequested()));
	connect(this, SIGNAL(updateLayerTable()),
		this->mw(), SIGNAL(updateLayerTable()));
	connect(md(), SIGNAL(meshRemoved(int)),
		this, SLOT(meshRemoved(int)));

	MainWindow* mainwindow = this->mw();
	//connecting the MainWindow Slots to GLArea signal (simple passthrough)
	if (mainwindow != NULL) {
		connect(this, SIGNAL(updateMainWindowMenus()),
			mainwindow, SLOT(updateMenus()));
		connect(mainwindow, SIGNAL(dispatchCustomSettings(RichParameterSet&)),
			this, SLOT(updateCustomSettingValues(RichParameterSet&)));
	}
	else {
		qDebug("The parent of the GLArea parent is not a pointer to the meshlab MainWindow.");
	}
	lastloadedraster = -1;

	connect(parent, SIGNAL(setUpperToothShowStatus(int)),
		this, SLOT(setShowOrHideUpperToothCommand(int)));

	connect(parent, SIGNAL(setLowerToothShowStatus(int)),
		this, SLOT(setShowOrHideLowerToothCommand(int)));

	// load background image
	texture_background_ = new ui_common::TextureMap(QString(":/%1/res/%1/images/background.jpg").arg("dark"));

	// mesh shader render init
	SAFE_DELETE(mesh_shader_render_);
	mesh_shader_render_ = new MeshShaderRender;
	mesh_shader_render_->Init();

	tempTimer = NULL;

	if (bEnableShowCollectAndFoldedGui)
	{

		tipLabelPen = vcg::glLabel::Mode(vcg::Color4b::White);
		tipLabelPen.qFont.setFamily("Microsoft YaHei");
		tipLabelPen.qFont.setStyleStrategy(QFont::PreferAntialias);
		tipLabelPen.qFont.setPixelSize(14);

		ui_common::BaseAttributeInfoButton baseAttributeInfoButton(
			Point3m(0, 0, 0),
			0.08f,
			tipLabelPen,
			heightTipAllControls,
			vcg::Color4f::Yellow,
			vcg::Color4f(26 / 255.0f, 40 / 255.0f, 53 / 255.0f, 1.0),
			vcg::Color4f::DarkGray,
			vcg::Color4f(26 / 255.0f, 40 / 255.0f, 53 / 255.0f, 1.0),
			vcg::Color4f::Yellow,
			vcg::Color4f(26 / 255.0f, 40 / 255.0f, 53 / 255.0f, 1.0),
			true
		);

		baseAttributeInfoButton.setBaseAttributeBackgroundDisabled(
			vcg::Color4f(205 / 255.0f, 205 / 255.0f, 205 / 255.0f, 1.0));
		baseAttributeInfoButton.setBaseAttributeMouseHoverStatusColor(
			vcg::Color4f(84 / 255.0f, 99 / 255.0f, 114 / 255.0f, 1.0));

		collect_folded_gui_ = new ui_common::CollectFoldedGui(
			tr("unfold"),
			tr("unfold"),
			ui_common::TipShowDirection::DOWN_TIP_SHOW_DIRECTION,
			ui_common::ControlTypeId::COLLECT_FOLDED);

		collect_folded_gui_->setAttributeInfomation(baseAttributeInfoButton);
		collect_folded_gui_->setVisible(false);

	}

	setMouseTracking(true);

	connect(PSIGNALMANAGER, &SignalManager::setViewDirByPhotosMouseSelectedSignal,
		this, &GLArea::showViewDirectionExpand);
	connect(PSIGNALMANAGER, &SignalManager::switchDentalRenderObjectSignal,
		this, &GLArea::dentalRenderObjectSwitch);
	connect(PSIGNALMANAGER, &SignalManager::deleteMeshModelSignal,
		this, &GLArea::deleteMeshModel);
	connect(PSIGNALMANAGER, &SignalManager::setGlobalTrackBallEnableSignal,
		this, &GLArea::setGlobalTrackBallEnableSlot);
	connect(PSIGNALMANAGER, &SignalManager::setMeshDocCurrentMesh,
		this, &GLArea::setMeshDocCurrentMeshSlot);
	connect(PSIGNALMANAGER, &SignalManager::occlusalColoringInProgress,
		this, &GLArea::occlusalColoringInProgressSlot);

	connect(PSIGNALMANAGER, &SignalManager::updateSceneSignal, this, static_cast<void (GLArea::*)()>(&GLArea::update));
	connect(PSIGNALMANAGER, &SignalManager::onePartOfInitalAnimationOverSignal_PP, this, &GLArea::onePartOfInitalAnimationOverSlot);
	connect(PSIGNALMANAGER, &SignalManager::updateMeshVertexInfoBuffer_PP, this, &GLArea::udpateEachMeshVertexInfoBuffer);
	connect(PSIGNALMANAGER, &SignalManager::getQtDeviceToLogicalValueSignal, this, &GLArea::getQtDeviceToLogicalValueSlot);
	connect(PSIGNALMANAGER, &SignalManager::getQtLogicalToDeviceValueSignal, this, &GLArea::getQtLogicalToDeviceValueSlot);

	p_view_gui_ = new ui_common::fusionViewGui(this, mw());
	p_view_gui_->show();
	tips_button_gui_ = new tipsWidget(this, mw());
	connect(p_view_gui_, &fusionViewGui::setViewDirSignal, this, &GLArea::showViewDirectionExpand);
	connect(p_view_gui_, &fusionViewGui::setViewDirSignal, PFusionAlignData, &FusionAlignData::unFocusFramesSlot);
	connect(p_view_gui_, &fusionViewGui::setSelectDentalSignal, this, &GLArea::onSelectDentalSlot);
	connect(p_view_gui_, &fusionViewGui::setSelectDentalSignal, PSIGNALMANAGER, &SignalManager::setSelectDentalSignal);

	compare_switch_link_btn_ = new QPushButton(this);
	changeLinkBackground(change_link_status);
	connect(compare_switch_link_btn_, &QPushButton::clicked, this, [&]() {
		change_link_status = !change_link_status;
		if (change_link_status)
		{
			MainWindow* window = qobject_cast<MainWindow*>(QApplication::activeWindow());
			if (window && mvc()->currentId == id)
			{
				mvc()->updateTrackballInViewers();
				udpateEachMeshVertexInfoBuffer();
			}
		}
		else
		{

		}
		changeLinkBackground(change_link_status);
		MainWindow* window = qobject_cast<MainWindow*>(QApplication::activeWindow());
		if (window && window->linkViewersAct)
			window->linkViewersAct->setChecked(change_link_status);
		});
	compare_switch_link_btn_->hide();
	auto createSpecialBtn = [&](QPushButton*& btn, QString icon, QString text, QString objName)
	{
		btn = new QPushButton(this);
		btn->setText(text);
		btn->setIcon(QIcon(icon));
		btn->setObjectName(objName);
		btn->setAttribute(Qt::WA_TranslucentBackground);
		btn->setLayoutDirection(Qt::RightToLeft);
		btn->setFont(QFont("Microsoft YaHei", 12));
		if (objName == "complete_btn")
		{
			btn->setStyleSheet("QPushButton{background-color:#344a55; color:white; border-radius: 8px; border:1px solid #b6c4d1; }\
                                  .QPushButton:pressed{background-color:#344A55; border:1px solid #a3d3d3;}\
									.QPushButton:hover{background-color:#43606B;border:1px solid #a3d3d3;}");
		}
		else
		{
			btn->setStyleSheet("QPushButton{background-color:#36404c; color:white; border-radius: 8px; border:1px solid #b6c4d1; }\
                                  .QPushButton:pressed{background-color:#36404c; border:1px solid #a3d3d3;}\
									.QPushButton:hover{background-color:#5e6976;border:1px solid #a3d3d3;}");

		}
		if (mw()->screen_height >= 1080)
		{
			if (objName == "Forward step")
			{
				btn->setFixedWidth(80);
			}
			else
			{
				btn->setFixedWidth(180);
			}
			btn->setFixedHeight(60);
		}
		else {
			if (objName == "Forward step")
			{
				btn->setFixedWidth(60);
			}
			else
			{
				btn->setFixedWidth(140);
			}
			btn->setFixedHeight(45);
		}
	};
	auto createAnaBtn = [&](navAnabutton*& btn, QString icon, QString text, QString objName, QColor Frame, QColor press, QColor normal)
	{
		btn = new navAnabutton(this);
		btn->setText(text);
		btn->setpixmapName(icon);
		btn->setObjectName(objName);
		btn->setFrameColor(Frame);
		btn->setPressColor(press);
		btn->setNormalColor(normal);
		if (mw()->screen_height >= 1080)
		{
			btn->setFixedWidth(180);
			btn->setFixedHeight(60);
		}
		else {
			btn->setFixedWidth(140);
			btn->setFixedHeight(45);
		}
	};
	auto createBtn = [&](QPushButton*& btn, QString icon, QString text, QString objName)
	{
		btn = new QPushButton(this);
		btn->setFlat(true);
		btn->setStyleSheet(QString::fromUtf8(""));
		btn->setObjectName(objName);
		btn->setAttribute(Qt::WA_TranslucentBackground, true);
		if (mw()->screen_height >= 1080)
		{
			btn->setFixedWidth(180);
			btn->setFixedHeight(60);
		}
		else {
			btn->setFixedWidth(140);
			btn->setFixedHeight(45);
		}

	};
	createSpecialBtn(Import_jaw_alignment, ":/dark/res/default/images/svg/importJaw.svg", tr("Cont Jaw"), "Import_jaw_alignment");
	createSpecialBtn(next_step_button, ":/dark/res/default/images/svg/next.svg", tr("Next"), "next_step_button");
	createSpecialBtn(Fixed_point_analysis, ":/dark/res/default/images/svg/fixedPointAnalysis.svg", tr("Analys"), "Fixed_point_analysis");
	createSpecialBtn(Skip_the_upper_jaw, ":/dark/res/default/images/svg/skip.svg", tr("Skip Up."), "Skip_the_upper_jaw");
	createSpecialBtn(complete_btn, ":/dark/res/default/images/svg/completeAnalysis.svg", tr("Complete"), "complete_btn");
	createSpecialBtn(Previous_step, ":/dark/res/default/images/svg/returnToUpperJaw.svg", tr("Pre"), "Previous_step");
	createSpecialBtn(forward_step_button_, ":/dark/res/default/images/svg/returnToUpperJaw.svg", tr(""), "Forward step");

	Import_jaw_alignment->hide();
	Fixed_point_analysis->hide();
	Skip_the_upper_jaw->hide();
	complete_btn->hide();
	Previous_step->hide();
	next_step_button->hide();
	forward_step_button_->hide();
	Previous_step->move(600, 600);
	connect(PSIGNALMANAGER, &SignalManager::markingDoneSignal, Skip_the_upper_jaw, [&] (bool done){if (done)
	{
		Skip_the_upper_jaw->setText(tr("Mark Lo."));
	}
	else {
		Skip_the_upper_jaw->setText(tr("Skip Up."));
	}
	 });

	connect(PSIGNALMANAGER, &SignalManager::markingAnyOneDentalDoneSignal, complete_btn, [&](bool done) {if (done)
	{
		complete_btn->setText(tr("Complete"));
	}
	else {
		complete_btn->setText(tr("Exit"));
	}
		});
	connect(forward_step_button_, &QPushButton::clicked, mw(), &MainWindow::forwardStepSlot);
	connect(next_step_button, &QPushButton::clicked, mw(), &MainWindow::nextStepSlot);
	connect(Previous_step, &QPushButton::clicked, mw(), &MainWindow::PreviousStepSlot);
	connect(Skip_the_upper_jaw, &QPushButton::clicked, mw(), &MainWindow::SkipUpperJaw);
	connect(Fixed_point_analysis, &QPushButton::clicked, mw(), &MainWindow::FixedPointAnalysisSlot);
	connect(complete_btn, &QPushButton::clicked, mw(), &MainWindow::startAnalysisSlot);
	connect(Import_jaw_alignment, &QPushButton::clicked, mw(), &MainWindow::importJawSlot);

	timer_alpha_model_ = new QTimer;
	timer_alpha_model_->setInterval(10);
	connect(timer_alpha_model_, &QTimer::timeout, this, [&]
		{
			if (alpha_model_lab > 0)
			{
				--alpha_model_lab;
				update();
			}
			else
			{
				timer_alpha_model_->stop();
			}
		});
}

GLArea::~GLArea()
{
	SAFE_DELETE(texture_background_);
	SAFE_DELETE(collect_folded_gui_);

	SAFE_DELETE(mesh_shader_render_);
}

void GLArea::setFixAnaBtnText()
{
	if (PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageSignalNotComplete || PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageDualNotComplete)
	{
		Fixed_point_analysis->setText(tr("Analys"));
	}
	else if (PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageSignalComplete || PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageDualComplete)
	{
		Fixed_point_analysis->setText(tr("Change"));
	}
}

void GLArea::importJawAlignment(int size)
{
	if (size == 1)
	{
		Import_jaw_alignment->show();
		Fixed_point_analysis->show();
	}
	else {
		Import_jaw_alignment->hide();
		Fixed_point_analysis->show();
	}
}

QString GLArea::GetMeshInfoString()
{
	QString info;
	if (mm()->hasDataMask(MeshModel::MM_VERTQUALITY)) { info.append("VQ "); }
	if (mm()->hasDataMask(MeshModel::MM_VERTCOLOR)) { info.append("VC "); }
	if (mm()->hasDataMask(MeshModel::MM_VERTRADIUS)) { info.append("VR "); }
	if (mm()->hasDataMask(MeshModel::MM_VERTTEXCOORD)) { info.append("VT "); }
	if (mm()->hasDataMask(MeshModel::MM_VERTCURV)) { info.append("VK "); }
	if (mm()->hasDataMask(MeshModel::MM_VERTCURVDIR)) { info.append("VD "); }
	if (mm()->hasDataMask(MeshModel::MM_FACECOLOR)) { info.append("FC "); }
	if (mm()->hasDataMask(MeshModel::MM_FACEQUALITY)) { info.append("FQ "); }
	if (mm()->hasDataMask(MeshModel::MM_WEDGTEXCOORD)) { info.append("WT "); }
	if (mm()->hasDataMask(MeshModel::MM_CAMERA)) { info.append("MC "); }
	if (mm()->hasDataMask(MeshModel::MM_POLYGONAL)) { info.append("MP "); }

	return info;
}

void GLArea::Logf(int Level, const char* f, ...)
{
	makeCurrent();
	if (this->md() == 0) return;

	char buf[4096];
	va_list marker;
	va_start(marker, f);

	vsprintf(buf, f, marker);
	va_end(marker);
	this->md()->Log.Log(Level, buf);
}

QSize GLArea::minimumSizeHint() const
{
	QDesktopWidget* desktopWidget = QApplication::desktop(); //获取可用桌面大小
	QRect deskRect = desktopWidget->availableGeometry();  //获取设备屏幕大小
	QRect screenRect = desktopWidget->screenGeometry();
#ifdef Q_OS_WIN
	HDC desk = GetDC(NULL);   //亲测可行，测dpi
	float hDPI = GetDeviceCaps(desk, LOGPIXELSX);
	float vDPI = GetDeviceCaps(desk, LOGPIXELSY);
	int dpi = (hDPI + vDPI) / 2;
#elif defined(Q_OS_MAC)
	QScreen *screen = QGuiApplication::primaryScreen();
	int dpi = screen->logicalDotsPerInch();
#else
	int dpi = 96;
#endif
	if (dpi > 170 || deskRect.height() < 1000)
	{
		return QSize(1280, 720);
	}
	else if ((deskRect.height() >= 1100 && dpi < 180 && dpi >= 150) || (deskRect.height() < 1100 && deskRect.height() >= 1000 && dpi >= 140 && dpi < 180))
	{
		return QSize(1800, 1000);
	}
	else
	{
		return QSize(1500, 900);
	}
}

QSize GLArea::sizeHint() const
{
	QDesktopWidget* desktopWidget = QApplication::desktop(); //获取可用桌面大小
	QRect deskRect = desktopWidget->availableGeometry();  //获取设备屏幕大小
	QRect screenRect = desktopWidget->screenGeometry();
#ifdef Q_OS_WIN
	HDC desk = GetDC(NULL);   //亲测可行，测dpi
	float hDPI = GetDeviceCaps(desk, LOGPIXELSX);
	float vDPI = GetDeviceCaps(desk, LOGPIXELSY);
	int dpi = (hDPI + vDPI) / 2;
#elif defined(Q_OS_MAC)
	QScreen *screen = QGuiApplication::primaryScreen();
	int dpi = screen->logicalDotsPerInch();
#else
	int dpi = 96;
#endif
	if (dpi > 170 || deskRect.height() < 1000)
	{
		return QSize(1280, 720);
	}
	else if ((deskRect.height() >= 1100 && dpi < 180 && dpi >= 150) || (deskRect.height() < 1100 && deskRect.height() >= 1000 && dpi >= 140 && dpi < 180))
	{
		return QSize(1800, 1000);
	}
	else
	{
		return QSize(1500, 900);
	}
}

void GLArea::initializeGL()
{
	makeCurrent();
	glShadeModel(GL_SMOOTH);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	static float diffuseColor[] = { 1.0,1.0,1.0,1.0 };
	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);
	glDisable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseColor);
	trackball.center = Point3f(0, 0, 0);
	trackball.radius = 1;

	trackball_light.center = Point3f(0, 0, 0);
	trackball_light.radius = 1;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		assert(0);
	}
}

void GLArea::drawGradient()
{
	makeCurrent();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	if (!takeSnapTile)
	{
		glBegin(GL_TRIANGLE_STRIP);
		glColor(glas.backgroundTopColor);
		glColor3f(55 / 255.0f, 61 / 255.0f, 109 / 255.0f);
		glVertex2f(-1, 1);
		glVertex2f(-1, -1);
		glVertex2f(1, 1);
		glVertex2f(1, -1);
		glEnd();
	}
	else
	{
		float xb = (-tileCol * 2) - 1;
		float yb = (-tileRow * 2) - 1;
		float xt = ((totalCols - tileCol) * 2) - 1;
		float yt = ((totalRows - tileRow) * 2) - 1;

		glBegin(GL_TRIANGLE_STRIP);
		glColor3f(0.0f, 0.0f, 0.0f); glVertex2f(-1, 1);
		glVertex2f(-1, -1);
		glVertex2f(1, 1);
		glVertex2f(1, -1);
		glEnd();
	}

	glPopAttrib();
	glPopMatrix(); // restore modelview
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void GLArea::drawLight()
{
	makeCurrent();
	glPushMatrix();
	trackball_light.GetView();
	trackball_light.Apply();

	float lightPosF[] = { lightPos.X(),lightPos.Y(),lightPos.Z(),0.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosF);
	static float lightPosB[] = { 0.0,0.0,-lightPos.Z(),0.0 };
	glLightfv(GL_LIGHT1, GL_POSITION, lightPosB);

	float specularClr[] = { 0.4,0.4,0.4,1.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specularClr);   //指定材料对镜面光的反应
	glMateriali(GL_FRONT, GL_SHININESS, 100);             //指定反射系数
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
	glPopMatrix();
}

int GLArea::renderForSelection(int pickX, int pickY)
{
	makeCurrent();
	if (mvc() == NULL)
		return -1;

	MLSceneGLSharedDataContext* datacont = mvc()->sharedDataContext();
	if (datacont == NULL)
		return -1;

	int sz = int(md()->meshList.size()) * 5;
	GLuint* selectBuf = new GLuint[sz];
	glSelectBuffer(sz, selectBuf);
	glRenderMode(GL_SELECT);
	glInitNames();

	glPushName(-1);
	double mp[16];

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_PROJECTION);
	glGetDoublev(GL_PROJECTION_MATRIX, mp);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix(pickX, pickY, 4, 4, viewport);
	glMultMatrixd(mp);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	if (PFusionAlignData != nullptr && PFusionAlignData->p_cur_mesh_ != nullptr)
	{
		Point3m cur_view_direct, cur_camera_pos;
		UtilityTools::getInstance()->getCurrentCameraState(PFusionAlignData->p_cur_mesh_->original_mesh_, this->width(), this->height(), cur_view_direct, cur_camera_pos);

		vector<MeshModel*> model_list;
		if (b_is_clone_)
		{
			model_list = PFusionAlignData->sortComparableSourceMeshPointersSegment(cur_camera_pos);
		}
		else
		{
			model_list = PFusionAlignData->sortMeshModelPointersSegment(cur_camera_pos);
		}
		for(auto &mp : model_list)
		{
			glLoadName(mp->id());
			datacont->setMeshTransformationMatrix(mp->id(), mp->cm.Tr);
			datacont->draw(mp->id(), context());
		}
	}
	else
	{
		foreach(MeshModel * mp, this->md()->meshList)
		{
			glLoadName(mp->id());
			datacont->setMeshTransformationMatrix(mp->id(), mp->cm.Tr);
			datacont->draw(mp->id(), context());
		}
	}

	long hits;
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	hits = glRenderMode(GL_RENDER);
	glPopAttrib();

	std::vector< std::pair<double, unsigned int> > H;
	for (long ii = 0; ii < hits; ii++) {
		H.push_back(
			std::pair<double, unsigned int>(
				selectBuf[ii * 4 + 1] / 4294967295.0, selectBuf[ii * 4 + 3]));
	}
	std::sort(H.begin(), H.end());
	delete[] selectBuf;
	if (hits == 0) return -1;
	return H.front().second;
}

void GLArea::selectMeshOperate()
{
	id_mesh_selected = renderForSelection(this->pointToPick.X(), this->pointToPick.Y());

	if (id_mesh_selected >= 0)
	{
		alpha_model_lab = 255;
		timer_alpha_model_->start();

		static bool is_first_get_mesh_color = true;
		bool is_upper_selected = true;
		foreach(MeshModel * mp, this->md()->meshList)
		{
			if (is_first_get_mesh_color)
			{
				color_origion_mesh_ = mp->cm.vert.front().C();
				is_first_get_mesh_color = false;
			}

			if (mp->id() == id_mesh_selected)
			{
				vcg::tri::UpdateColor<CMeshO>::PerVertexConstant(mp->cm, Color4b(64, 224, 205, 255));
				is_upper_selected &= mp->upperOrLowerToothModelMark == UpperToothModel;
			}
			else
			{
				vcg::tri::UpdateColor<CMeshO>::PerVertexConstant(mp->cm, color_origion_mesh_);
			}

		}
		if (is_upper_selected)
		{
			emit PSIGNALMANAGER->setUpperDentalSelectedSignal();
		}
		else
		{
			emit PSIGNALMANAGER->setLowerDentalSelectedSignal();
		}
		emit PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();

		if (!mesh_operate_menu_)
		{
			mesh_operate_menu_ = new QMenu(this);
			QAction* toggle_act = new QAction(tr("Transposition of maxilla/mandible"));
			QAction* remove_act = new QAction(tr("Remove the  model"));
			connect(toggle_act, &QAction::triggered, this, [&]() {mw()->toggleUpperLowerMesh();
			alpha_model_lab = 255;timer_alpha_model_->start(); });

			connect(remove_act, &QAction::triggered, this, [&]() {mw()->removeCurrentMesh(id_mesh_selected); });

			mesh_operate_menu_->addAction(remove_act);
			mesh_operate_menu_->addAction(toggle_act);
			mesh_operate_menu_->setStyleSheet("QMenu {background-color: rgb(17, 26, 36);} \
					 QMenu::item {background-color: rgb(17, 26, 36);color:rgb(182, 196, 209); font: 14pt /""微软雅黑/"";} \
					QMenu::item:selected{ background-color: rgb(64, 76, 87);}");

		}

		mesh_operate_menu_->move(QCursor::pos());
		mesh_operate_menu_->show();

	}
}

void GLArea::recoverSelectedMeshColor()
{
	if (id_mesh_selected < 0)
	{
		return;
	}

	foreach(MeshModel * mp, this->md()->meshList)
	{
		if (mp->id() == id_mesh_selected)
		{
			vcg::tri::UpdateColor<CMeshO>::PerVertexConstant(mp->cm, color_origion_mesh_);
			emit PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
		}
	}
	id_mesh_selected = -1;
}

void GLArea::eulerRotateModelSlot(Matrix44f& _m)
{
	this->eulerRotateMatrix = _m;
	this->bEnableErlueRotate = true;
}

void GLArea::paintEvent(QPaintEvent* /*event*/)
{
	if (mvc() == NULL)
		return;
	updateViewGui();
	tips_button_gui_->update();
	QPainter painter(this);
	painter.beginNativePainting();

	makeCurrent();

	if (!isValid())
		return;

	if (b_is_clone_)
	{
		compare_switch_link_btn_->setGeometry(this->width() - 100, 30, 40, 40);
	}

	QTime time;
	time.start();

	glClearColor(1.0, 1.0, 1.0, 0.0);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setView();  // Set Modelview and Projection matrix

	// draws the background
	float screenRatio = float(this->width()) / float(this->height());

	texture_background_->drawBackground(screenRatio);

	drawLight();

	//////////////////////////////////////////////////////////////

	glPushMatrix();
	if (resetTrackCenter)
	{
		trackball.center.SetZero();
		trackball.track.tra.Import(-(viewTabPlane.axisYV * cameraDistance));
		resetTrackCenter = false;
	}
	if (moveCamera)
	{
		trackball.center.SetZero();
		if (cameraTransStartScale > 0 && cameraTransEndScale > 0)
		{
			trackball.track.SetScale(((this->cameraTransEndScale - this->cameraTransStartScale) * this->iCamStep / 50.0f + this->cameraTransStartScale));
		}
		trackball.track.tra.Import(-((this->cameraTransEndPoint - this->cameraTransStartPoint) * this->iCamStep / 50.0f + this->cameraTransStartPoint));

		getCurrentCameraState(*this->md()->meshList.begin());
		Axis curAxis, aimAxis;
		Point3m tempX, tempY, tempZ;
		tempX = view_RH_Direct;
		tempZ = viewDirect;
		tempY = (tempZ ^ tempX).Normalize();
		curAxis = Axis(Point3m(0, 0, 0), tempX, tempY, tempZ);

		tempX = this->aimCameraView_RH_Axis;
		tempZ = this->aimCameraViewAxis;
		tempY = (tempZ ^ tempX).Normalize();
		aimAxis = Axis(Point3m(0, 0, 0), tempX, tempY, tempZ);

		Point3m moveVector, rotateVector;
		aimAxis.computeTransformVectors(curAxis, moveVector, rotateVector);
		Matrix44f transformValue;
		vcg::Quaternionf rot1st, rot2nd, rot3rd;
		transformValue.SetRotateDeg(rotateVector.X(), curAxis.axisXVector);
		rot1st.FromMatrix(transformValue);
		transformValue.SetRotateDeg(rotateVector.Y(), curAxis.axisYVector);
		rot2nd.FromMatrix(transformValue);
		transformValue.SetRotateDeg(rotateVector.Z(), curAxis.axisZVector);
		rot3rd.FromMatrix(transformValue);
		trackball.track.rot = rot1st * rot2nd * rot3rd;
		trackball.SetCurrentAction();

		moveCamera = false;
	}

	trackball.GetView();
	trackball.Apply();
	glPushMatrix();

	if (setCameraRotateParameters)
	{
		autoRotateCameraToWantedDirect(this->cameraRotateEndAxis);
		setCameraRotateParameters = false;
	}

	if (!this->md()->isBusy())
	{
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		if (iEdit) {
			glEnable(GL_COLOR_MATERIAL);
			iEdit->DecorateBeforeMeshRender(*mm(), this, &painter);
		}

		if (bEnableErlueRotate)
		{
			glTranslate(trackball.center);
			glMultMatrix(this->eulerRotateMatrix);
			glTranslate(-trackball.center);
			bEnableErlueRotate = false;
		}

		if (bOpenTransparencyAdjust)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		// if we have this shader,we will use shader firsty, then use Fixed pipe
		if (mesh_shader_render_->isSupported())
		{
			MLSceneGLSharedDataContext* shared = mvc()->sharedDataContext();
			MLPerViewGLOptions opts;
			setLightingColors(opts);

			penetrateCorrection();

			if (!need_sort_mesh_render_)// 不排序
			{
				mesh_shader_render_->Render();
				foreach(MeshModel * mp, this->md()->meshList)
				{
					shared->setMeshTransformationMatrix(mp->id(), mp->cm.Tr);
					if (mp->visible)
					{
						shared->draw(mp->id(), context());
					}
				}
				glUseProgramObjectARB(0);
			}
			else // 排序渲染网格，添加网格时需加入指定网格管理数据结构中
			{
				//new sort
				if (PFusionAlignData != nullptr && PFusionAlignData->curManager() != nullptr)
				{
					Point3m cur_view_direct, cur_camera_pos;
					UtilityTools::getInstance()->getCurrentCameraState(PFusionAlignData->curManager()->cDental.p_model_, this->width(), this->height(), cur_view_direct, cur_camera_pos);

					if (b_is_clone_)
					{
						vector<MeshModel*> model_list = PFusionAlignData->sortComparableSourceMeshPointers(cur_camera_pos);
						mesh_shader_render_->Render();
						foreach(MeshModel * mp, model_list)
						{
							shared->setMeshTransformationMatrix(mp->id(), mp->cm.Tr);
							if (mp->visible)
							{
								shared->draw(mp->id(), context());
							}
						}
						vector<MeshModel*>().swap(model_list);
						showComparisonWindowDescription(painter);
						glUseProgramObjectARB(0);
					}
					else
					{
						vector<MeshModel*> model_list = PFusionAlignData->sortMeshModelPointers(cur_camera_pos);
						mesh_shader_render_->Render();
						foreach(MeshModel * mp, model_list)
						{
							shared->setMeshTransformationMatrix(mp->id(), mp->cm.Tr);
							if (mp->visible)
							{
								if (mp->cullMode() == CULL_NONE)
								{
									glDisable(GL_CULL_FACE);
								}
								else if (mp->cullMode() == CULL_FRONT)
								{
									glEnable(GL_CULL_FACE);
									glCullFace(GL_FRONT);
								}
								else if (mp->cullMode() == CULL_BACK)
								{
									glEnable(GL_CULL_FACE);
									glCullFace(GL_BACK);
								}
								shared->draw(mp->id(), context());
							}
						}
						vector<MeshModel*>().swap(model_list);
						glUseProgramObjectARB(0);
					}
				}
				else if (PFusionAlignData != nullptr && PFusionAlignData->p_cur_mesh_ != nullptr)
				{
					Point3m cur_view_direct, cur_camera_pos;
					UtilityTools::getInstance()->getCurrentCameraState(PFusionAlignData->p_cur_mesh_->original_mesh_, this->width(), this->height(), cur_view_direct, cur_camera_pos);

					if (b_is_clone_)
					{
						vector<MeshModel*> model_list = PFusionAlignData->sortComparableSourceMeshPointersSegment(cur_camera_pos);
						mesh_shader_render_->Render();
						foreach(MeshModel * mp, model_list)
						{
							if (mp != NULL)
							{
								shared->setMeshTransformationMatrix(mp->id(), mp->cm.Tr);
								if (mp->visible)
								{
									if (mp->cullMode() == CULL_NONE)
									{
										glDisable(GL_CULL_FACE);
									}
									else if (mp->cullMode() == CULL_FRONT)
									{
										glEnable(GL_CULL_FACE);
										glCullFace(GL_FRONT);
									}
									else if (mp->cullMode() == CULL_BACK)
									{
										glEnable(GL_CULL_FACE);
										glCullFace(GL_BACK);
									}

									shared->draw(mp->id(), context());
								}
							}
						}
						vector<MeshModel*>().swap(model_list);
						glUseProgramObjectARB(0);

						if (b_first_open_clone_)
						{
							b_first_open_clone_ = false;
							udpateEachMeshVertexInfoBuffer();
							update();
						}
					}
					else
					{
						vector<MeshModel*> model_list = PFusionAlignData->sortMeshModelPointersSegment(cur_camera_pos);
						mesh_shader_render_->Render();
						foreach(MeshModel * mp, model_list)
						{
							if (mp != NULL)
							{
								shared->setMeshTransformationMatrix(mp->id(), mp->cm.Tr);
								if (mp->visible)
								{
									if (mp->cullMode() == CULL_NONE)
									{
										glDisable(GL_CULL_FACE);
									}
									else if (mp->cullMode() == CULL_FRONT)
									{
										glEnable(GL_CULL_FACE);
										glCullFace(GL_FRONT);
									}
									else if (mp->cullMode() == CULL_BACK)
									{
										glEnable(GL_CULL_FACE);
										glCullFace(GL_BACK);
									}

									shared->draw(mp->id(), context());
								}
							}
						}
						vector<MeshModel*>().swap(model_list);
						glUseProgramObjectARB(0);
					}
				}
			}

			foreach(MeshModel * mp, this->md()->meshList)
			{
				if (mp->visible)
				{
					MLRenderingData curr;
					MLDefaultMeshDecorators defdec(mw());
					shared->getRenderInfoPerMeshView(mp->id(), context(), curr);
					defdec.decorateMesh(*mp, curr, &painter, md()->Log);

					QList<QAction*>& tmpset = iPerMeshDecoratorsListMap[mp->id()];
					for (QList<QAction*>::iterator it = tmpset.begin(); it != tmpset.end(); ++it)
					{
						MeshDecorateInterface* decorInterface = qobject_cast<MeshDecorateInterface*>((*it)->parent());
						decorInterface->decorateMesh(*it, *mp, this->glas.currentGlobalParamSet, this, &painter, md()->Log);
					}
				}
			}
		}
		else
		{
			MLSceneGLSharedDataContext* datacont = mvc()->sharedDataContext();
			if (datacont == NULL)
				return;
			// note: render order need to consider  when two meshs are transparency
			if (bDrawUpperMeshOrder)
			{
				foreach(MeshModel * mp, this->md()->meshList)
				{
					if (mp->visible && belongToLowerDentalPart(mp->upperOrLowerToothModelMark))
					{
						MLRenderingData curr;
						datacont->getRenderInfoPerMeshView(mp->id(), context(), curr);
						MLPerViewGLOptions opts;
						if (curr.get(opts) == false)
							throw MLException(QString("GLArea: invalid MLPerViewGLOptions"));
						setLightingColors(opts);

						if (opts._back_face_cull)
							glEnable(GL_CULL_FACE);
						else
							glDisable(GL_CULL_FACE);

						datacont->setMeshTransformationMatrix(mp->id(), mp->cm.Tr);

						datacont->draw(mp->id(), context());
					}
				}
				foreach(MeshModel * mp, this->md()->meshList)
				{
					if (mp->visible && belongToUpperDentalPart(mp->upperOrLowerToothModelMark))
					{
						MLRenderingData curr;
						datacont->getRenderInfoPerMeshView(mp->id(), context(), curr);
						MLPerViewGLOptions opts;
						if (curr.get(opts) == false)
							throw MLException(QString("GLArea: invalid MLPerViewGLOptions"));
						setLightingColors(opts);

						if (opts._back_face_cull)
							glEnable(GL_CULL_FACE);
						else
							glDisable(GL_CULL_FACE);

						datacont->setMeshTransformationMatrix(mp->id(), mp->cm.Tr);

						datacont->draw(mp->id(), context());
					}
				}
			}
			else
			{
				foreach(MeshModel * mp, this->md()->meshList)
				{
					if (mp->visible && belongToUpperDentalPart(mp->upperOrLowerToothModelMark))
					{
						MLRenderingData curr;
						datacont->getRenderInfoPerMeshView(mp->id(), context(), curr);
						MLPerViewGLOptions opts;
						if (curr.get(opts) == false)
							throw MLException(QString("GLArea: invalid MLPerViewGLOptions"));
						setLightingColors(opts);

						if (opts._back_face_cull)
							glEnable(GL_CULL_FACE);
						else
							glDisable(GL_CULL_FACE);

						datacont->setMeshTransformationMatrix(mp->id(), mp->cm.Tr);

						datacont->draw(mp->id(), context());
					}
				}
				foreach(MeshModel * mp, this->md()->meshList)
				{
					if (mp->visible && belongToLowerDentalPart(mp->upperOrLowerToothModelMark))
					{
						MLRenderingData curr;
						datacont->getRenderInfoPerMeshView(mp->id(), context(), curr);
						MLPerViewGLOptions opts;
						if (curr.get(opts) == false)
							throw MLException(QString("GLArea: invalid MLPerViewGLOptions"));
						setLightingColors(opts);

						if (opts._back_face_cull)
							glEnable(GL_CULL_FACE);
						else
							glDisable(GL_CULL_FACE);

						datacont->setMeshTransformationMatrix(mp->id(), mp->cm.Tr);

						datacont->draw(mp->id(), context());
					}
				}
			}

			foreach(MeshModel * mp, this->md()->meshList)
			{
				//if (mp->visible)
				//{
				MLRenderingData curr;
				MLDefaultMeshDecorators defdec(mw());
				datacont->getRenderInfoPerMeshView(mp->id(), context(), curr);
				defdec.decorateMesh(*mp, curr, &painter, md()->Log);

				QList<QAction*>& tmpset = iPerMeshDecoratorsListMap[mp->id()];
				for (QList<QAction*>::iterator it = tmpset.begin(); it != tmpset.end(); ++it)
				{
					MeshDecorateInterface* decorInterface = qobject_cast<MeshDecorateInterface*>((*it)->parent());
					decorInterface->decorateMesh(*it, *mp, this->glas.currentGlobalParamSet, this, &painter, md()->Log);
				}
				//}
			}
		}

		if (iEdit && !b_is_clone_) {
			iEdit->setLog(&md()->Log);
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glEnable(GL_COLOR_MATERIAL);
			glEnable(GL_LIGHTING);
			iEdit->Decorate(*mm(), this, &painter);
			glPopAttrib();
		}

		if (bOpenTransparencyAdjust)
		{
			glDisable(GL_BLEND);
		}
		glPopAttrib();
	} ///end if busy

	/*if (bEnableDisplayDentalOverlapArea)
	{
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_LIGHTING);
		drawDentalOverlapArea(&painter);
		glPopAttrib();
		glDisable(GL_LIGHTING);
	}*/

	if (hasToPick)// 如果当前有分割插件运行的时候,则不进行测量数据点的传输，防止鼠标双击冲突事件
	{
		hasToPick = false;

		Point3f pp;
		if (Pick<Point3f>(pointToPick[0], pointToPick[1], pp))
		{
			emit PSIGNALMANAGER->transmitPickedPosSignal(pp[0], pp[1], pp[2]);
		}
	}

	if (!have_acttion_decorate_doc_ && !getCurrentMeshEditor() && hasToSelectMesh)
	{
		hasToSelectMesh = false;

		selectMeshOperate();
	}

	displayModelLabel(&painter);

	glPopMatrix();
	// We restore the state to immediately after the trackball (and before the bbox scaling/translating)

	if (trackBallVisible && !(iEdit && !suspendedEditor))
		trackball.DrawPostApply();
	// The picking of the surface position has to be done in object space,
	// so after trackball transformation (and before the matrix associated to each mesh);

	have_acttion_decorate_doc_ = false;
	foreach(QAction * p, iPerDocDecoratorlist)
	{
		MeshDecorateInterface* decorInterface =
			qobject_cast<MeshDecorateInterface*>(p->parent());

		have_acttion_decorate_doc_ |= decorInterface->decorateDoc(p, *this->md(),
			this->glas.currentGlobalParamSet, this, &painter, md()->Log);
	}

	glPopMatrix();

	if (bEnableShowCollectAndFoldedGui && !b_is_clone_)
	{
		drawCollectFoldedGui(&painter);
	}

	// dispaly patient infomation
	if (bDisplayPatientInfo)
	{
		displayPatientInfomation();
	}

	// Finally display HELP if requested
	if (isHelpVisible())
	{
		if (!isMeshInfomationVisible())
		{
			displayHelp();
			glPushAttrib(GL_ENABLE_BIT);
			glDisable(GL_DEPTH_TEST);
			displayRealTimeLog(&painter);
			glPopAttrib();
		}
	}

	QString error = checkGLError::makeString("There are gl errors: ");
	if (!error.isEmpty()) {
		Logf(GLLogStream::WARNING, qPrintable(error));
	}
	//check if viewers are linked
	MainWindow* window = qobject_cast<MainWindow*>(QApplication::activeWindow());
	if (window && window->linkViewersAct->isChecked() && mvc()->currentId == id)
		mvc()->updateTrackballInViewers();

	// Draw the log area background
	// on the bottom of the glArea
	if (isMeshInfomationVisible())
	{
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		displayInfo(&painter);
		if (isHelpVisible())
		{
			displayHelp();
		}
		displayRealTimeLog(&painter);
		updateFps(time.elapsed());
		glPopAttrib();
	}
	painter.endNativePainting();

}

void GLArea::penetrateCorrection()
{
	static float last_adj_value = 0;
	if (!this->md()->meshList.empty())
	{
		getCurrentCameraState(*this->md()->meshList.begin());
		//qDebug() << "VIEW DIRECT : " << viewDirect.X() << ", " << viewDirect.Y() << ", " << viewDirect.Z() << endl;
		float min_distance = 0;
		foreach(MeshModel * mp, this->md()->meshList)
		{
			if (PFusionAlignData->mesh_cloud_Octree_[mp] != nullptr)
			{
				float temp_min_distance = PFusionAlignData->mesh_cloud_Octree_[mp]->sectionDistance(mp->cm.Tr, cameraPosition, viewDirect);
				if (temp_min_distance < min_distance)
				{
					min_distance = temp_min_distance;
					//qDebug() << "MININININ Distance : " << min_distance << endl;
				}
			}
		}

		if (min_distance < 0)
		{
			float adj_value = abs(min_distance);
			if (adj_value <= 1e-5 || cameraDistance > 200)
			{
				adj_value = 0;
			}
			cameraDistance += adj_value;
			last_adj_value += adj_value;
			penetrate_happend_ = true;
		}
		else
		{
			if (cameraDistance > 35)
			{
				cameraDistance -= 1.0f;
			}
			penetrate_happend_ = false;
		}
	}
}

bool GLArea::belongToLowerDentalPart(ToothModelType _type)
{
	return true;
	if (PFusionAlignData->lowerManager() == nullptr)
	{
		return false;
	}
	if (!PFusionAlignData->lowerManager()->bVisible_)
	{
		return false;
	}

	if (_type == LowerToothModel ||
		_type == LowerSingleDental ||
		_type == LowerSingleTooth ||
		_type == LowerSingleRoot ||
		_type == LowerSingleAccessory ||
		_type == LowerSingleComparedTooth ||
		_type == LowerSingleComparedRoot ||
		_type == LowerSingleBasalLabel ||
		_type == LowerSingleExportableMesh)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool GLArea::belongToUpperDentalPart(ToothModelType _type)
{
	return true;
	if (PFusionAlignData->upperManager() == nullptr)
	{
		return false;
	}
	if (!PFusionAlignData->upperManager()->bVisible_)
	{
		return false;
	}

	if (_type == UpperToothModel ||
		_type == UpperSingleDental ||
		_type == UpperSingleTooth ||
		_type == UpperSingleRoot ||
		_type == UpperSingleAccessory ||
		_type == UpperSingleComparedTooth ||
		_type == UpperSingleComparedRoot ||
		_type == UpperSingleBasalLabel ||
		_type == UpperSingleExportableMesh)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void GLArea::displayRealTimeLog(QPainter* painter)
{
	makeCurrent();
	painter->endNativePainting();
	painter->save();
	painter->setPen(Qt::white);
	Color4b logAreaColor(30, 30, 70, 128);
	if (mvc()->currentId != id) logAreaColor /= 2.0;

	qFont.setFamily("Microsoft YaHei UI");
	qFont.setStyleStrategy(QFont::PreferAntialias);
	qFont.setPixelSize(17 * this->mw()->scaleResolutionDpi);

	painter->setFont(qFont);
	float margin = qFont.pixelSize();
	QFontMetrics metrics = QFontMetrics(font());
	int border = qMax(4, metrics.leading());
	qreal roundness = 10.0f;
	QTextDocument doc;
	doc.setDefaultFont(qFont);
	int startingpoint = border;
	foreach(QString keyIt, md()->Log.RealTimeLogText.uniqueKeys())
	{
		QList< QPair<QString, QString> > valueList = md()->Log.RealTimeLogText.values(keyIt);
		QPair<QString, QString> itVal;
		// the map contains pairs of meshname, text
		// the meshname is used only to disambiguate when there are more than two boxes with the same title
		foreach(itVal, valueList)
		{
			QString HeadName = keyIt;
			if (md()->Log.RealTimeLogText.count(keyIt) > 1)
				HeadName += " - " + itVal.first;
			doc.clear();
			doc.setDocumentMargin(margin * 0.75);
			QColor textColor(200, 200, 200);
			QColor headColor(200, 200, 200);
			doc.setHtml("<font color=\"" + headColor.name() + "\" size=\"+2\" ><p><b>" + HeadName + "</b></p></font>"
				"<font color=\"" + textColor.name() + "\"             >" + itVal.second + "</font>");
			QRect outrect(border, startingpoint, doc.size().width(), doc.size().height());
			QPainterPath path;
			painter->setBrush(QBrush(ColorConverter::ToQColor(logAreaColor), Qt::SolidPattern));
			painter->setPen(ColorConverter::ToQColor(logAreaColor));
			path.addRoundedRect(outrect, roundness, roundness);
			painter->drawPath(path);
			painter->save();
			painter->translate(border, startingpoint);
			doc.drawContents(painter);
			painter->restore();
			startingpoint = startingpoint + doc.size().height() + margin * .75;
		}
	}

	// After the rederaw we clear the RealTimeLog buffer!
	md()->Log.RealTimeLogText.clear();
	painter->restore();
	painter->beginNativePainting();
}

void GLArea::displayModelLabel(QPainter* painter)
{
	makeCurrent();
	if ((mvc() == NULL) || (md() == NULL))
		return;

	painter->setRenderHint(QPainter::HighQualityAntialiasing);

	vcg::glLabel::Mode pen(Color4b(0, 122, 255, alpha_model_lab));
	pen.qFont.setFamily("Microsoft YaHei UI");
	pen.qFont.setStyleStrategy(QFont::PreferAntialias);
	pen.qFont.setPixelSize(30);
	painter->setFont(pen.qFont);

	QString col1Text, col0Text;

	QString upper_lab = tr("Maxilla");
	QString lower_lab = tr("Mandible");
	for (auto& mesh : md()->meshList)
	{
		QString text = mesh->upperOrLowerToothModelMark == UpperToothModel ? upper_lab : lower_lab;
		Point3f center = mesh->cm.bbox.Center();
		vcg::glLabel::render(painter, center,text, pen);
	}
}

void GLArea::displayInfo(QPainter* painter)
{
	makeCurrent();
	if ((mvc() == NULL) || (md() == NULL))
		return;
	painter->endNativePainting();
	painter->save();

	painter->setRenderHint(QPainter::HighQualityAntialiasing);
	QPen textPen(QColor(255, 255, 255, 200));
	textPen.setWidthF(0.2f);
	painter->setPen(textPen);

	qFont.setStyleStrategy(QFont::PreferAntialias);
	qFont.setFamily("Helvetica");
	qFont.setPixelSize(24);
	painter->setFont(qFont);
	QFontMetrics metrics = QFontMetrics(qFont);
	int border = qMax(4, metrics.leading()) / 2;
	int numLines = 3;

	float barHeight = ((metrics.height() + metrics.leading()) * numLines) + 2 * border;

	int startPos = width() / 30;

	QRect Column_1(startPos, this->height() - 2 * barHeight,
		width() * 3 / 4, this->height());
	QRect Column_0(startPos, this->height() - barHeight, width() / 2, this->height());

	glas.logAreaColor[0] = 21;
	glas.logAreaColor[1] = 16;
	glas.logAreaColor[2] = 8;
	glas.logAreaColor[3] = 128;
	Color4b logAreaColor = glas.logAreaColor;

	if (mvc()->currentId != id) logAreaColor /= 2.0;

	int widthBorder = 3;
	// left
	//painter->fillRect(QRect(0, 0, widthBorder, this->height()), ColorConverter::ToQColor(Color4b(21, 26, 62, 255)));
	// right
	painter->fillRect(QRect(this->width() - widthBorder, 0, this->width(), this->height()), ColorConverter::ToQColor(Color4b(21, 26, 62, 255)));
	// top
	painter->fillRect(QRect(0, 0, this->width(), widthBorder), ColorConverter::ToQColor(Color4b(21, 26, 62, 255)));
	// bottom
	//painter->fillRect(QRect(0, this->height() - widthBorder, this->width(), this->height()), ColorConverter::ToQColor(Color4b(21, 26, 62, 255)));
	QString col1Text, col0Text;

	if (this->md()->size() > 0)
	{
		if (this->md()->size() == 1)
		{
			QLocale engLocale(QLocale::English, QLocale::UnitedStates);
			col1Text += QString("Mesh: %1\n").arg(mm()->label());
			col1Text += "Vertices: " + engLocale.toString(mm()->cm.vn) + " \n";
			col1Text += "Faces: " + engLocale.toString(mm()->cm.fn);
		}
		else
		{
			QLocale engLocale(QLocale::English, QLocale::UnitedStates);
			QFileInfo inf = mm()->label();
			col1Text += QString("Current Mesh: %1\n").arg(inf.completeBaseName());
			col1Text += "Vertices: " + engLocale.toString(mm()->cm.vn) + "    (" + engLocale.toString(this->md()->vn()) + ") \n";
			col1Text += "Faces: " + engLocale.toString(mm()->cm.fn) + "    (" + engLocale.toString(this->md()->fn()) + ")";
		}

		if ((cfps > 0) && (cfps < 1999))
			col0Text += QString("FPS: %1\n").arg(cfps, 7, 'f', 1);

		if (clipRatioNear != clipRatioNearDefault())
			col0Text += QString("\nClipping Near:%1\n").arg(clipRatioNear, 7, 'f', 2);

		painter->drawText(Column_0, Qt::AlignLeft | Qt::TextWordWrap, col0Text);
		painter->drawText(Column_1, Qt::AlignLeft | Qt::TextWordWrap, col1Text);
	}
	painter->restore();
	painter->beginNativePainting();
}

void GLArea::displayHelp()
{
	makeCurrent();
	static QString tableText;
	if (tableText.isEmpty())
	{
		QFile helpFile(QString(":/default/res/default/images/onscreenHelp.txt"));
		if (helpFile.open(QFile::ReadOnly))
			tableText = helpFile.readAll();
		else assert(0);
#ifdef Q_OS_MAC
		tableText.replace("Ctrl", "Command");
#endif
	}
	md()->Log.RealTimeLog("Help", "", tableText);
}

void GLArea::displayPatientInfomation()
{
	makeCurrent();
	QString patientInfoStr;
	if (patientInfo.bHaveName)
	{
		patientInfoStr += QStringLiteral("<br>姓名: ") + patientInfo.getName() + QString("<br>");
	}
	if (patientInfo.bHaveOrderId)
	{
		patientInfoStr += QStringLiteral("<br>订单号: ") + patientInfo.getOrderId() + QString("<br>");
	}
	md()->Log.RealTimeLog(QStringLiteral("患者信息"), "", patientInfoStr);
}

void GLArea::toggleHelpVisible()
{
	helpVisible = !helpVisible;
	update();
}

// Slot called when the current mesh has changed.
void GLArea::manageCurrentMeshChange()
{
	//if we have an edit tool open, notify it that the current layer has changed
	if (iEdit)
	{
		if (iEdit->isSingleMeshEdit())
			assert(lastModelEdited);  // if there is an editor that works on a single mesh
									  // last model edited should always be set when start edit is called

		//iEdit->LayerChanged(*this->md(), *lastModelEdited, this, parentmultiview->sharedDataContext());

		//now update the last model edited
		//TODO this is not the best design....   iEdit should maybe keep track of the model on its own
		lastModelEdited = this->md()->mm();
	}
	emit this->updateMainWindowMenus();
	// if the layer has changed update also the decoration.
	//	updateAllPerMeshDecorators();
}

void GLArea::updateAllDecorators()
{
	updateAllPerMeshDecorators();
	if (md() == NULL)
		return;
	foreach(QAction * p, iPerDocDecoratorlist)
	{
		MeshDecorateInterface* decorInterface = qobject_cast<MeshDecorateInterface*>(p->parent());
		decorInterface->endDecorate(p, *md(), this->glas.currentGlobalParamSet, this);
		decorInterface->setLog(&md()->Log);
		decorInterface->startDecorate(p, *md(), this->glas.currentGlobalParamSet, this);
	}
	if (mvc() != NULL)
		mvc()->updateAllViewers();
}

void GLArea::setCurrentEditAction(QAction* editAction)
{
	if ((parentmultiview == NULL) || (parentmultiview->sharedDataContext() == NULL))
		return;
	if (Fixed_point_analysis->isVisible())
	{
		Fixed_point_analysis->hide();
	}
	makeCurrent();
	assert(editAction);
	currentEditor = editAction;
	MeshEditInterface* tempEdit = nullptr;
	tempEdit = actionToMeshEditMap.value(currentEditor);
	if (tempEdit == NULL)
		return;

	lastModelEdited = this->md()->mm();

	MLRenderingData dt;
	if (tempEdit->isSingleMeshEdit())
	{
		if (md()->mm() != NULL)
		{
			parentmultiview->sharedDataContext()->getRenderInfoPerMeshView(md()->mm()->id(), context(), dt);
			tempEdit->suggestedRenderingData(*(md()->mm()), dt);
			MLPoliciesStandAloneFunctions::disableRedundatRenderingDataAccordingToPriorities(dt);
			parentmultiview->sharedDataContext()->setRenderingDataPerMeshView(md()->mm()->id(), context(), dt);
			parentmultiview->sharedDataContext()->manageBuffers(md()->mm()->id());
		}
	}
	else
	{
		foreach(MeshModel * mm, md()->meshList)
		{
			if (mm != NULL)
			{
				parentmultiview->sharedDataContext()->getRenderInfoPerMeshView(mm->id(), context(), dt);
				tempEdit->suggestedRenderingData(*(mm), dt);
				MLPoliciesStandAloneFunctions::disableRedundatRenderingDataAccordingToPriorities(dt);
				parentmultiview->sharedDataContext()->setRenderingDataPerMeshView(mm->id(), context(), dt);
				parentmultiview->sharedDataContext()->manageBuffers(mm->id());
			}
		}
	}

	if (!tempEdit->StartEdit(*this->md(), this, parentmultiview->sharedDataContext()))
	{
		//iEdit->EndEdit(*(this->md()->mm()), this);
		iEdit = tempEdit;
		endEdit();
	}
	else
	{
		iEdit = tempEdit;
		Logf(GLLogStream::SYSTEM, "Started Mode %s", qPrintable(currentEditor->text()));
		if (mm() != NULL)
			mm()->meshModified() = true;
		else assert(!iEdit->isSingleMeshEdit());
		update();
	}
}

bool GLArea::readyToClose()
{
	makeCurrent();
	// Now do the actual closing of the glArea
	if (getCurrentEditAction())
	{
		endEdit();
		md()->meshDocStateData().clear();
	}
	if (iRenderer)
		iRenderer->Finalize(currentShader, this->md(), this);

	// Now manage the closing of the decorator set;
	foreach(QAction * act, iPerDocDecoratorlist)
	{
		MeshDecorateInterface* mdec = qobject_cast<MeshDecorateInterface*>(act->parent());
		mdec->endDecorate(act, *md(), glas.currentGlobalParamSet, this);
		mdec->setLog(NULL);
	}
	iPerDocDecoratorlist.clear();
	QSet<QAction*> dectobeclose;
	for (QMap<int, QList<QAction*> >::iterator it = iPerMeshDecoratorsListMap.begin(); it != iPerMeshDecoratorsListMap.end(); ++it)
	{
		foreach(QAction * curract, it.value())
			dectobeclose.insert(curract);
	}

	for (QSet<QAction*>::iterator it = dectobeclose.begin(); it != dectobeclose.end(); ++it)
	{
		MeshDecorateInterface* mdec = qobject_cast<MeshDecorateInterface*>((*it)->parent());
		if (mdec != NULL)
		{
			mdec->endDecorate(*it, *md(), glas.currentGlobalParamSet, this);
			mdec->setLog(NULL);
		}
	}
	dectobeclose.clear();
	iPerMeshDecoratorsListMap.clear();
	if (targetTex) glDeleteTextures(1, &targetTex);
	emit glareaClosed();
	return true;
}

void GLArea::keyReleaseEvent(QKeyEvent* e)
{
	makeCurrent();
	e->accept();
	if (iEdit && !suspendedEditor)  iEdit->keyReleaseEvent(e, *mm(), this);
	else {
		if (e->key() == Qt::Key_Control) trackball.ButtonUp(QT2VCG(Qt::NoButton, Qt::ControlModifier));
		if (e->key() == Qt::Key_Shift) trackball.ButtonUp(QT2VCG(Qt::NoButton, Qt::ShiftModifier));
		if (e->key() == Qt::Key_Alt) trackball.ButtonUp(QT2VCG(Qt::NoButton, Qt::AltModifier));
	}

	update();
}

void GLArea::keyPressEvent(QKeyEvent* e)
{

	makeCurrent();
	e->accept();
	if (iEdit && !suspendedEditor)  iEdit->keyPressEvent(e, *mm(), this);
	else {
		if (e->key() == Qt::Key_Control) trackball.ButtonUp(QT2VCG(Qt::NoButton, Qt::ControlModifier));
		if (e->key() == Qt::Key_Shift) trackball.ButtonUp(QT2VCG(Qt::NoButton, Qt::ShiftModifier));
		if (e->key() == Qt::Key_Alt) trackball.ButtonUp(QT2VCG(Qt::NoButton, Qt::AltModifier));
	}

	update();
}

void GLArea::keyPressEventSpecial(QKeyEvent* e)
{
	makeCurrent();
	e->accept();

	switch (e->key())
	{
	case Qt::Key_Tab:
		mw()->tabUpperOrLowerToothShowStatus();
		break;
	case Qt::Key_I:
		bShowMeshInfoVisible = !bShowMeshInfoVisible;
		break;
	case Qt::Key_T:
		trackBallVisible = !trackBallVisible;
		break;

	case Qt::Key_S:
		emit PSIGNALMANAGER->switchDentalRenderObjectSignal();
		udpateEachMeshVertexInfoBuffer();
		break;
	}

	if (iEdit && !suspendedEditor)
	{
		iEdit->keyPressEvent(e, *mm(), this);
	}
	else
	{
		if (e->key() == Qt::Key_Control) trackball.ButtonDown(QT2VCG(Qt::NoButton, Qt::ControlModifier));
		if (e->key() == Qt::Key_Shift) trackball.ButtonDown(QT2VCG(Qt::NoButton, Qt::ShiftModifier));
		if (e->key() == Qt::Key_Alt) trackball.ButtonDown(QT2VCG(Qt::NoButton, Qt::AltModifier));
	}

	update();
}

void GLArea::keyReleaseEventSpecial(QKeyEvent* e)
{
	makeCurrent();
	e->accept();

	if (iEdit && !suspendedEditor)
	{
		iEdit->keyReleaseEvent(e, *mm(), this);
	}

	update();
}

void GLArea::mousePressEvent(QMouseEvent* e)
{

	makeCurrent();
	e->accept();

	mvc()->currentgla = this;
	mvc()->currentId = id;

	foreach(MeshModel * mp, this->md()->meshList)
	{
		if (mp->visible)
		{
			QList<QAction*>& tmpset = iPerMeshDecoratorsListMap[mp->id()];
			for (QList<QAction*>::iterator it = tmpset.begin(); it != tmpset.end(); ++it)
			{
				MeshDecorateInterface* decorInterface = qobject_cast<MeshDecorateInterface*>((*it)->parent());
				decorInterface->mousePressEvent(e, *mp, this);
			}
			if (!tmpset.empty())
			{
				break;
			}
		}
	}

	if (bEnableShowCollectAndFoldedGui)
	{
		collect_folded_gui_->mousePressEvent(e->pos());
	}

	if ((iEdit != NULL) && !suspendedEditor)
	{
		iEdit->mousePressEvent(e, *mm(), this);
	}

	if (bWhetherOpenGlobalTrackball)
	{

		if (e->button() == Qt::RightButton) // Select a new current mesh
		{
			hasToSelectMesh = true;
			is_right_mouse_pressed_ = true;
			this->pointToPick = Point2i(QT2VCG_X(this, e), QT2VCG_Y(this, e));
		}
		else
		{
			if ((e->modifiers() & Qt::ShiftModifier) &&
				(e->modifiers() & Qt::ControlModifier) &&
				(e->button() == Qt::LeftButton))
				activeDefaultTrackball = false;
			else activeDefaultTrackball = true;

			if (isDefaultTrackBall())
			{
				if (QApplication::keyboardModifiers() & Qt::Key_Control) trackball.ButtonDown(QT2VCG(Qt::NoButton, Qt::ControlModifier));
				else trackball.ButtonUp(QT2VCG(Qt::NoButton, Qt::ControlModifier));
				if (QApplication::keyboardModifiers() & Qt::Key_Shift) trackball.ButtonDown(QT2VCG(Qt::NoButton, Qt::ShiftModifier));
				else trackball.ButtonUp(QT2VCG(Qt::NoButton, Qt::ShiftModifier));
				if (QApplication::keyboardModifiers() & Qt::Key_Alt) trackball.ButtonDown(QT2VCG(Qt::NoButton, Qt::AltModifier));
				else trackball.ButtonUp(QT2VCG(Qt::NoButton, Qt::AltModifier));

				trackball.MouseDown(QT2VCG_X(this, e), QT2VCG_Y(this, e), QT2VCG(e->button(), e->modifiers()));
			}
			else trackball_light.MouseDown(QT2VCG_X(this, e), QT2VCG_Y(this, e), QT2VCG(e->button(), Qt::NoModifier));
		}
	}

	if (this->bEnableDisplayDentalCompareModel || this->occlusal_coloring_in_progress_)
	{
		bRolling = true;
	}

	foreach(QAction * p, iPerDocDecoratorlist)
	{
		MeshDecorateInterface* decorInterface =
			qobject_cast<MeshDecorateInterface*>(p->parent());

		decorInterface->mousePressEvent(p, e, *this->md()->mm(), this);
	}

	update();
}

void GLArea::mouseMoveEvent(QMouseEvent* e)
{

	makeCurrent();

	if ((iEdit && !suspendedEditor))
		iEdit->mouseMoveEvent(e, *mm(), this);

	for (auto& action : iPerDocDecoratorlist)
	{
		MeshDecorateInterface* decorInterface = qobject_cast<MeshDecorateInterface*>(action->parent());
		decorInterface->mouseMoveEvent(e, *mm(), this);
		break;// just response once
	}

	/*if (dental_analysis_ != nullptr)
	{
		mouseX_ = e->pos().x();
		mouseY_ = height() - e->pos().y();
		dental_analysis_move_ = true;
	}*/

	if (bWhetherOpenGlobalTrackball && (!bBanTrackballSphereMode || trackball.current_mode == NULL || (bBanTrackballSphereMode && trackball.current_mode->Name() != "SphereMode")))
	{
		if (isDefaultTrackBall())
		{
			trackball.MouseMove(QT2VCG_X(this, e), QT2VCG_Y(this, e));
			setCursorTrack(trackball.current_mode);
		}
		else trackball_light.MouseMove(QT2VCG_X(this, e), QT2VCG_Y(this, e));
	}

	update();
}

// When mouse is released we set the correct mouse cursor
void GLArea::mouseReleaseEvent(QMouseEvent* e)
{
	makeCurrent();

	hasToSelectMesh = false;
	recoverSelectedMeshColor();

	for (auto& action : iPerDocDecoratorlist)
	{
		MeshDecorateInterface* decorInterface = qobject_cast<MeshDecorateInterface*>(action->parent());
		decorInterface->mouseReleaseEvent(e, *mm(), this);
		break;// just response once
	}

	if (bEnableShowCollectAndFoldedGui)
	{
		collect_folded_gui_->mouseReleaseEvent(e->pos());
	}

	activeDefaultTrackball = true;
	if ((iEdit && !suspendedEditor))
		iEdit->mouseReleaseEvent(e, *mm(), this);

	/*if (dental_analysis_ != nullptr)
	{
		mouseX_ = e->pos().x();
		mouseY_ = height() - e->pos().y();
		dental_analysis_release_ = true;
	}*/

	if (bWhetherOpenGlobalTrackball)
	{
		if (isDefaultTrackBall()) trackball.MouseUp(QT2VCG_X(this, e), QT2VCG_Y(this, e), QT2VCG(e->button(), e->modifiers()));
		else trackball_light.MouseUp(QT2VCG_X(this, e), QT2VCG_Y(this, e), QT2VCG(e->button(), e->modifiers()));
		setCursorTrack(trackball.current_mode);
	}

	if (this->bEnableDisplayDentalCompareModel || this->occlusal_coloring_in_progress_)
	{
		bRolling = false;
	}
	update();
}

//Processing of tablet events, interesting only for painting plugins
void GLArea::tabletEvent(QTabletEvent* e)
{
	makeCurrent();
	if (iEdit && !suspendedEditor) iEdit->tabletEvent(e, *mm(), this);
	else e->ignore();
}

void GLArea::wheelEvent(QWheelEvent* e)
{
	makeCurrent();

	if (wheelTimer == nullptr)
	{
		wheelTimer = new QTimer(this);
		connect(wheelTimer, SIGNAL(timeout()), this, SLOT(wheelStopSlot()));
		wheelTimer->start(300);
	}
	else
	{
		wheelTimer->start(300);
	}
	bRolling = true;

	if ((iEdit && !suspendedEditor))
	{
		iEdit->wheelEvent(e, *mm(), this);
	}
	if (bWhetherOpenGlobalTrackball)
	{
		const int WHEEL_STEP = 120;
		float notch = e->delta() / float(WHEEL_STEP);
		switch (e->modifiers())
		{
		case Qt::ControlModifier:
		{
			clipRatioNear = math::Clamp(clipRatioNear * powf(1.1f, notch), 0.01f, 500.0f);
			break;
		}
		case Qt::ShiftModifier:
		{
			fov = math::Clamp(fov + 1.2f * notch, 5.0f, 90.0f);
			break;
		}
		case Qt::AltModifier:
		{
			glas.pointSize = math::Clamp(glas.pointSize * powf(1.2f, notch), 0.01f, MLPerViewGLOptions::maxPointSize());
			MLSceneGLSharedDataContext* cont = mvc()->sharedDataContext();
			if (cont != NULL)
			{
				foreach(MeshModel * mp, this->md()->meshList)
				{
					MLRenderingData dt;
					cont->getRenderInfoPerMeshView(mp->id(), context(), dt);
					MLPerViewGLOptions opt;
					dt.get(opt);
					opt._perpoint_pointsize = glas.pointSize;
					opt._perpoint_pointsmooth_enabled = glas.pointSmooth;
					opt._perpoint_pointattenuation_enabled = glas.pointDistanceAttenuation;
					cont->setGLOptions(mp->id(), context(), opt);
				}

			}
			break;
		}
		default:
		{
			/*float minScale = MODEL_MIN_SCALE_FACTOR / this->md()->bbox().Diag(), maxScale = MODEL_MAX_SCALE_FACTOR / this->md()->bbox().Diag();
			float newScale = trackball.track.sca * pow(1.2f, e->delta() / float(WHEEL_STEP));
			if (newScale >= minScale && newScale <= maxScale)
			{
				trackball.MouseWheel(-e->delta() / float(WHEEL_STEP));
			}*/

			if (!this->md()->meshList.empty())
			{
				if (penetrate_happend_)
				{
					if (e->delta() > 0)
					{
						//trackball.MouseWheel(-e->delta() / float(WHEEL_STEP));
					}
					else
					{
						trackball.MouseWheel(-e->delta() / float(WHEEL_STEP));
					}
				}
				else
				{
					trackball.MouseWheel(-e->delta() / float(WHEEL_STEP));
				}
			}

			break;
		}
		}
	}
	update();
}

void GLArea::mouseDoubleClickEvent(QMouseEvent* e)
{
	makeCurrent();

	if ((iEdit && !suspendedEditor))
		iEdit->mouseDoubleClickEvent(e, *mm(), this);

	if (e->button() == Qt::LeftButton)
	{
		hasToPick = true;
		pointToPick = Point2i(QT2VCG_X(this, e), QT2VCG_Y(this, e));
	}

	update();
}

void GLArea::setCursorTrack(vcg::TrackMode* tm)
{
	makeCurrent();
	//static QMap<QString,QCursor> curMap;
	if (curMap.isEmpty())
	{
		curMap[QString("")] = QCursor(Qt::ArrowCursor);
		curMap["SphereMode"] = QCursor(QPixmap(":/res/default/images/cursors/plain_trackball.png"), 1, 1);
		curMap["PanMode"] = QCursor(QPixmap(":/res/default/images/cursors/plain_pan.png"), 1, 1);
		curMap["ScaleMode"] = QCursor(QPixmap(":/res/default/images/cursors/plain_zoom.png"), 1, 1);
		curMap["PickMode"] = QCursor(QPixmap(":/res/default/images/cursors/plain_pick.png"), 1, 1);
	}
	if (tm) setCursor(curMap[tm->Name()]);
	else
		if (hasToGetPickPos) setCursor(curMap["PickMode"]);
		else setCursor(curMap[""]);

}

void GLArea::setDecorator(QString name, bool state)
{
	makeCurrent();
	updateDecorator(name, false, state);
}

void GLArea::toggleDecorator(QString name)
{
	if (b_is_clone_)return;
	makeCurrent();
	updateDecorator(name, true, false);
}

void GLArea::updateDecorator(QString name, bool toggle, bool stateToSet)
{
	if (b_is_clone_)return;
	makeCurrent();
	MeshDecorateInterface* iDecorateTemp = this->mw()->PM.getDecoratorInterfaceByName(name);
	QAction* action = iDecorateTemp->action(name);
	bool checked = action->isChecked();

	if (iDecorateTemp->getDecorationClass(action) == MeshDecorateInterface::PerDocument)
	{
		bool found = this->iPerDocDecoratorlist.removeOne(action);
		if (found)
		{
			if (toggle || stateToSet == false) {
				iDecorateTemp->endDecorate(action, *md(), glas.currentGlobalParamSet, this);
				iDecorateTemp->setLog(NULL);
				this->Logf(GLLogStream::SYSTEM, "Disabled Decorate mode %s", qPrintable(action->text()));
			}
			else
				this->Logf(GLLogStream::SYSTEM, "Trying to disable an already disabled Decorate mode %s", qPrintable(action->text()));
		}
		else {
			if (checked)
			{
				if (toggle || stateToSet == true) {
					if (Fixed_point_analysis->isVisible())
					{
						Fixed_point_analysis->hide();
					}
					iDecorateTemp->setLog(&(this->md()->Log));
					bool ret = iDecorateTemp->startDecorate(action, *md(), glas.currentGlobalParamSet, this);
					if (ret) {
						this->iPerDocDecoratorlist.push_back(action);
						this->Logf(GLLogStream::SYSTEM, "Enabled Decorate mode %s", qPrintable(action->text()));
					}
					else this->Logf(GLLogStream::SYSTEM, "Failed start of Decorate mode %s", qPrintable(action->text()));
				}
				else
					this->Logf(GLLogStream::SYSTEM, "Trying to enable an already enabled Decorate mode %s", qPrintable(action->text()));
			}
		}

		if (!this->iPerDocDecoratorlist.empty())
		{
			Fixed_point_analysis->hide();
//			Import_jaw_alignment->hide();
		}
		else if(this->iPerDocDecoratorlist.empty() && iEdit == nullptr)
		{
			Fixed_point_analysis->show();
		}
	}

	if (iDecorateTemp->getDecorationClass(action) == MeshDecorateInterface::PerMesh)
	{
		MeshModel& currentMeshModel = *mm();
		bool found = this->iCurPerMeshDecoratorList().removeOne(action);
		if (found)
		{
			if (toggle || stateToSet == false) {
				iDecorateTemp->endDecorate(action, currentMeshModel, glas.currentGlobalParamSet, this);
				iDecorateTemp->setLog(NULL);
				this->Logf(0, "Disabled Decorate mode %s", qPrintable(action->text()));
			}
			else
				this->Logf(GLLogStream::SYSTEM, "Trying to disable an already disabled Decorate mode %s", qPrintable(action->text()));
		}
		else {
			if (toggle || stateToSet == true) {
				QString errorMessage;
				if (iDecorateTemp->isDecorationApplicable(action, currentMeshModel, errorMessage))
				{
					iDecorateTemp->setLog(&md()->Log);
					bool ret = iDecorateTemp->startDecorate(action, currentMeshModel, glas.currentGlobalParamSet, this);
					if (ret) {
						this->iCurPerMeshDecoratorList().push_back(action);
						this->Logf(GLLogStream::SYSTEM, "Enabled Decorate mode %s", qPrintable(action->text()));
					}
					else this->Logf(GLLogStream::SYSTEM, "Failed Decorate mode %s", qPrintable(action->text()));
				}
				else
					this->Logf(GLLogStream::SYSTEM, "Error in Decorate mode %s: %s", qPrintable(action->text()), qPrintable(errorMessage));

			}
		}
	}
}

void GLArea::setLightingColors(const MLPerViewGLOptions& opts)
{
	makeCurrent();

	glLightfv(GL_LIGHT0, GL_AMBIENT, Color4f(0.1, 0.1, 0.08, 1.0).V());
	glLightfv(GL_LIGHT0, GL_DIFFUSE, Color4f(0.9, 0.829, 0.829, 1.0).V());
	glLightfv(GL_LIGHT0, GL_SPECULAR, Color4f(0.2966, 0.2966, 0.2966, 1.0).V());
}

void GLArea::setView()
{
	makeCurrent();
	glViewport(0, 0, (GLsizei)QTLogicalToDevice(this, width()), (GLsizei)QTLogicalToDevice(this, height()));

	GLfloat fAspect = (GLfloat)width() / height();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float cameraDist = this->getCameraDistance();

	if (fov <= 5) cameraDist = 8.0f;
	// small hack for orthographic projection where camera distance is rather meaningless...

	//cameraDist = 6.0f;

	nearPlane = 0.1f;
	farPlane = 1000.0f;

	/*if (!penetrate_happend_)
	{
		cameraDistance = cameraDist;
	}*/

	glOrtho(-viewRatio() * fAspect, viewRatio() * fAspect,
		-viewRatio(), viewRatio(), nearPlane, farPlane);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	Point3f _cameraPos(0, 0, 0);
	cameraPos = _cameraPos;

	if (bHaveOcclusalPlane)
	{
		if (viewDir == DEFAULT_VIEW)
		{
			lightPos = Point3m(0, 0, cameraDistance);

			_cameraPos = cameraPos;

			gluLookAt(
				_cameraPos.X(), _cameraPos.Y(), _cameraPos.Z(),
				0, 0, 0,
				0, 1, 0);
		}
		/*else if (viewDir == BACK_VIEW)
		{
			_cameraPos = -viewTabPlane.axisYV * cameraDistance;
			lightPos = _cameraPos;

			gluLookAt(
				_cameraPos.X(), _cameraPos.Y(), _cameraPos.Z(),
				0, 0, 0,
				-viewTabPlane.axisZV.X(),
				-viewTabPlane.axisZV.Y(),
				-viewTabPlane.axisZV.Z());
		}*/
		else
		{
			_cameraPos = viewTabPlane.axisYV * cameraDistance;
			lightPos = viewTabPlane.axisYV * (cameraDistance + 1000);

			gluLookAt(
				_cameraPos.X(), _cameraPos.Y(), _cameraPos.Z(),
				0, 0, 0,
				-viewTabPlane.axisZV.X(),
				-viewTabPlane.axisZV.Y(),
				-viewTabPlane.axisZV.Z());
		}
	}
	else {
		lightPos = Point3m(0, 0, cameraDistance + 1000);
		_cameraPos = Point3f(0, 0, cameraDistance);

		gluLookAt(
			_cameraPos.X(), _cameraPos.Y(), _cameraPos.Z(),
			0, 0, 0,
			0, 1, 0);
	}

	cameraPos = _cameraPos;
	//    0, 0, 0,
	//    0, 1, 0);
}

void GLArea::setTiledView(GLdouble fovY, float viewRatio, float fAspect,
	GLdouble zNear, GLdouble zFar, float /*cameraDist*/)
{
	makeCurrent();
	if (fovY <= 5)
	{
		GLdouble fLeft = -viewRatio * fAspect;
		GLdouble fRight = viewRatio * fAspect;
		GLdouble fBottom = -viewRatio;
		GLdouble fTop = viewRatio;

		GLdouble tDimX = fabs(fRight - fLeft) / totalCols;
		GLdouble tDimY = fabs(fTop - fBottom) / totalRows;

		glOrtho(fLeft + tDimX * tileCol, fLeft + tDimX * (tileCol + 1),     /* left, right */
			fBottom + tDimY * tileRow, fBottom + tDimY * (tileRow + 1),     /* bottom, top */
			zNear, zFar);
	}
	else
	{
		GLdouble fTop = zNear * tan(math::ToRad(fovY / 2.0));
		GLdouble fRight = fTop * fAspect;
		GLdouble fBottom = -fTop;
		GLdouble fLeft = -fRight;

		// tile Dimension
		GLdouble tDimX = fabs(fRight - fLeft) / totalCols;
		GLdouble tDimY = fabs(fTop - fBottom) / totalRows;

		glFrustum(fLeft + tDimX * tileCol, fLeft + tDimX * (tileCol + 1),
			fBottom + tDimY * tileRow, fBottom + tDimY * (tileRow + 1), zNear, zFar);
	}
}

void GLArea::updateFps(float deltaTime)
{
	const int avgSize = 10;
	static float fpsVector[avgSize];
	static int j = 0;
	float averageFps = 0;
	if (deltaTime > 0) {
		fpsVector[j] = deltaTime;
		j = (j + 1) % avgSize;
	}
	for (int i = 0; i < avgSize; i++) averageFps += fpsVector[i];
	cfps = 1000.0f / (averageFps / avgSize);
	lastTime = deltaTime;
}

void GLArea::resetTrackBall()
{
	makeCurrent();
	trackball.Reset();
	float newScale = 3.0f / this->md()->bbox().Diag();
	trackball.track.sca = newScale;
	trackball.track.tra.Import(-this->md()->bbox().Center());
	clipRatioNear = clipRatioNearDefault();

	fov = fovDefault();

	update();
}

void GLArea::hideEvent(QHideEvent* /*event*/)
{
	trackball.current_button = 0;
}

void GLArea::resizeGL(int w, int h)
{
	if (next_step_button->height() > (this->height())* 0.075)
	{
		int heightBtn = next_step_button->height();
		if (PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageFixUpper)
		{
			if (forward_step_button_->width() > (this->width()) * 0.04)
			{
				forward_step_button_->setGeometry((this->width()) * 0.85 - forward_step_button_->width(),
					(this->height()) * 0.9 - heightBtn,
					forward_step_button_->width() * 1.5,
					forward_step_button_->height() * 1.5);
			}
			else
			{
				forward_step_button_->setGeometry((this->width()) * 0.81,
				(this->height()) * 0.9 - heightBtn,
				forward_step_button_->width() * 1.5,
				forward_step_button_->height() * 1.5);
			}
		}
		else if (PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageMissUpper)
		{
			next_step_button->setGeometry((this->width()) * 0.85,
				(this->height()) * 0.9 - heightBtn * 2,
				next_step_button->width() * 1.5,
				next_step_button->height() * 1.5);
		}
		else if (PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageMissLower)
		{
			if (forward_step_button_->width() > (this->width()) * 0.04)
			{
				forward_step_button_->setGeometry((this->width()) * 0.85 - forward_step_button_->width(),
					(this->height()) * 0.9 - heightBtn,
					forward_step_button_->width() * 1.5,
					forward_step_button_->height() * 1.5);
				next_step_button->setGeometry((this->width()) * 0.85,
					(this->height()) * 0.9 - heightBtn,
					next_step_button->width() * 1.5,
					next_step_button->height() * 1.5);
			}
			else {
				next_step_button->setGeometry((this->width()) * 0.85,
					(this->height()) * 0.9 - heightBtn,
					next_step_button->width() * 1.5,
					next_step_button->height() * 1.5);
				forward_step_button_->setGeometry((this->width()) * 0.81,
					(this->height()) * 0.9 - heightBtn,
					forward_step_button_->width() * 1.5,
					forward_step_button_->height() * 1.5);
			}
		}
		else if (PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageFixLower)
		{
			if (forward_step_button_->width() > (this->width()) * 0.04)
			{
				forward_step_button_->setGeometry((this->width()) * 0.85 - forward_step_button_->width(),
					(this->height()) * 0.9,
					forward_step_button_->width() * 1.5,
					forward_step_button_->height() * 1.5);
			}
			else
			{

				forward_step_button_->setGeometry((this->width()) * 0.81,
					(this->height()) * 0.9,
					forward_step_button_->width() * 1.5,
					forward_step_button_->height() * 1.5);
			}
		}

		Import_jaw_alignment->setGeometry((this->width()) * 0.85,
			(this->height()) * 0.9 - heightBtn,
			Import_jaw_alignment->width() * 1.5,
			Import_jaw_alignment->height() * 1.5);
		Fixed_point_analysis->setGeometry((this->width()) * 0.85,
			(this->height()) * 0.9,
			Fixed_point_analysis->width() * 1.5,
			Fixed_point_analysis->height() * 1.5);
		Skip_the_upper_jaw->setGeometry((this->width()) * 0.85,
			(this->height()) * 0.9 - heightBtn,
			Skip_the_upper_jaw->width(),
			Skip_the_upper_jaw->height());
		Previous_step->setGeometry((this->width()) * 0.1,
			(this->height()) * 0.9,
			Previous_step->width() * 1.5,
			Previous_step->height() * 1.5);
		complete_btn->setGeometry((this->width()) * 0.85,
			(this->height()) * 0.9,
			Previous_step->width() * 1.5,
			Previous_step->height() * 1.5);
		emit PSIGNALMANAGER->updateSceneSizeSignal(width(), height());
		emit PSIGNALMANAGER->updateSceneSignal();
		this->update();
		return;
	}
	if (PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageFixUpper)
	{
		if (forward_step_button_->width() > (this->width()) * 0.04) {
			forward_step_button_->setGeometry((this->width()) * 0.85 - forward_step_button_->width(),
				(this->height()) * 0.825,
				forward_step_button_->width() * 1.5,
				forward_step_button_->height() * 1.5);
		}
		else
		{
			forward_step_button_->setGeometry((this->width()) * 0.81,
				(this->height()) * 0.825,
				forward_step_button_->width() * 1.5,
				forward_step_button_->height() * 1.5);
		}
	}
	else if (PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageMissUpper)
	{
		next_step_button->setGeometry((this->width()) * 0.85,
			(this->height()) * 0.75,
		    next_step_button->width() * 1.5,
			next_step_button->height() * 1.5);
	}
	else if (PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageMissLower)
	{
		if (forward_step_button_->width() > (this->width()) * 0.04) {
			forward_step_button_->setGeometry((this->width()) * 0.85 - forward_step_button_->width(),
				(this->height()) * 0.825,
				forward_step_button_->width() * 1.5,
				forward_step_button_->height() * 1.5);
			next_step_button->setGeometry((this->width()) * 0.85,
				(this->height()) * 0.825,
				next_step_button->width() * 1.5,
				next_step_button->height() * 1.5);
		}
		else
		{
			next_step_button->setGeometry((this->width()) * 0.85,
				(this->height()) * 0.825,
				next_step_button->width() * 1.5,
				next_step_button->height() * 1.5);
			forward_step_button_->setGeometry((this->width()) * 0.81,
				(this->height()) * 0.825,
				forward_step_button_->width() * 1.5,
				forward_step_button_->height() * 1.5);
		}
	}
	else if (PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageFixLower)
	{
		if (forward_step_button_->width() > (this->width()) * 0.04) {
			forward_step_button_->setGeometry((this->width()) * 0.85 - forward_step_button_->width(),
				(this->height()) * 0.9,
				forward_step_button_->width() * 1.5,
				forward_step_button_->height() * 1.5);
		}
		else
		{
			forward_step_button_->setGeometry((this->width()) * 0.81,
				(this->height()) * 0.9,
				forward_step_button_->width() * 1.5,
				forward_step_button_->height() * 1.5);
		}
	}

	Import_jaw_alignment->setGeometry((this->width()) * 0.85,
		(this->height()) * 0.825,
		Import_jaw_alignment->width() * 1.5,
		Import_jaw_alignment->height() * 1.5);
	Fixed_point_analysis->setGeometry((this->width()) * 0.85,
		(this->height()) * 0.9,
		Fixed_point_analysis->width(),
		Fixed_point_analysis->height());
	Skip_the_upper_jaw->setGeometry((this->width()) * 0.85,
		(this->height()) * 0.825,
		Skip_the_upper_jaw->width(),
		Skip_the_upper_jaw->height());
	Previous_step->setGeometry((this->width()) * 0.1,
		(this->height()) * 0.9,
		Previous_step->width(),
		Previous_step->height());
	complete_btn->setGeometry((this->width()) * 0.85,
		(this->height()) * 0.9,
		Previous_step->width() * 1.5,
		Previous_step->height());
	emit PSIGNALMANAGER->updateSceneSizeSignal(width(), height());
	emit PSIGNALMANAGER->updateSceneSignal();
	this->update();
}

void GLArea::updateViewGui()
{
	p_view_gui_->update();
	QPoint pointOri = this->mapToGlobal(QPoint(0, 0));
	int posX = pointOri.x() + this->width() / 2 - p_view_gui_->width() / 2;
	int posY = pointOri.y() + 5;
	p_view_gui_->setGeometry(posX, posY, p_view_gui_->width(), p_view_gui_->height());

	tips_button_gui_->update();
	//QPoint pointOri = this->mapToGlobal(QPoint(0, 0));
	int posTipsX = pointOri.x() + this->width() - tips_button_gui_->width() / 1.2;
	int posTipsY = pointOri.y();
	tips_button_gui_->setGeometry(posTipsX,
		posTipsY,
		tips_button_gui_->width(), tips_button_gui_->height());
}

void GLArea::resizeEvent(QResizeEvent* event)
{
	updateViewGui();
	return QGLWidget::resizeEvent(event);
}

void GLArea::sendViewPos(QString name)
{
#ifndef VCG_USE_EIGEN
	Point3f pos =
		trackball.track.InverseMatrix() *
		Inverse(trackball.camera.model) * Point3f(0, 0, 0);
#else
	Point3f pos =
		Eigen::Transform3f(trackball.track.InverseMatrix()) *
		Eigen::Transform3f(Inverse(trackball.camera.model)).translation();
#endif
	emit transmitViewPos(name, pos);
}

void GLArea::sendSurfacePos(QString name)
{
	qDebug("sendSurfacePos %s", qPrintable(name));
	nameToGetPickPos = name;
	hasToGetPickPos = true;
}

void GLArea::sendPickedPos(QString name)
{
	qDebug("sendPickedPos %s", qPrintable(name));
	nameToGetPickCoords = name;
	hasToGetPickCoords = true;
}

void GLArea::sendViewDir(QString name)
{
	Point3f dir = getViewDir();
	emit transmitViewDir(name, dir);
}

void GLArea::sendMeshShot(QString name)
{
	Shotm curShot = this->md()->mm()->cm.shot;
	emit transmitShot(name, curShot);
}

void GLArea::sendMeshMatrix(QString name)
{
	Matrix44m mat = this->md()->mm()->cm.Tr;
	emit transmitMatrix(name, mat);
}

void GLArea::sendViewerShot(QString name)
{
	Shotm curShot = shotFromTrackball().first;
	emit transmitShot(name, curShot);
}

void GLArea::sendRasterShot(QString name)
{
	Shotm curShot;
	if (this->md()->rm() != NULL)
		curShot = this->md()->rm()->shot;
	emit transmitShot(name, curShot);
}

void GLArea::sendCameraPos(QString name)
{
	Point3f pos = trackball.camera.ViewPoint();
	emit transmitCameraPos(name, pos);
}

void GLArea::sendTrackballPos(QString name)
{
	Point3f pos = -trackball.track.tra;
	emit transmitTrackballPos(name, pos);
}
void GLArea::shootPointList(Point3m* p, int num)
{
	emit updateOverlayOutline(p, num);
}

Point3f GLArea::getViewDir()
{
	vcg::Matrix44f rotM;
	trackball.track.rot.ToMatrix(rotM);
	return vcg::Inverse(rotM) * vcg::Point3f(0, 0, 1);
}

void GLArea::updateCustomSettingValues(RichParameterSet& rps)
{
	makeCurrent();
	glas.updateGlobalParameterSet(rps);

	this->update();
}

void GLArea::initGlobalParameterSet(RichParameterSet* defaultGlobalParamSet)
{
	GLAreaSetting::initGlobalParameterSet(defaultGlobalParamSet);
}

//Don't alter the state of the other elements in the visibility map
void GLArea::updateMeshSetVisibilities()
{
	meshVisibilityMap.clear();
	foreach(MeshModel * mp, this->md()->meshList)
	{
		//Insert the new pair in the map; If the key is already in the map, its value will be overwritten
		if (mp)
		{

			meshVisibilityMap.insert(mp->id(), mp->visible);
		}
	}
}

void GLArea::meshSetVisibility(MeshModel* mp, bool visibility)
{
	mp->visible = visibility;
	mp->visible = visibility;
}

void GLArea::weatherSwitchToMainMenu(bool _state)
{
	this->bSwitchToMainMenu = _state;
}

void GLArea::weatherParallelSwitched(bool _state)
{
	this->bParallelSwitched = _state;
}

// --------------- Methods involving shots -------------------------------------

float GLArea::getCameraDistance()
{
	float cameraDist = viewRatio() / tanf(vcg::math::ToRad(fov * .5f));
	return cameraDist;
}

void GLArea::initializeShot(Shotm& shot)
{
	shot.Intrinsics.PixelSizeMm[0] = 0.036916077;
	shot.Intrinsics.PixelSizeMm[1] = 0.036916077;

	shot.Intrinsics.DistorCenterPx[0] = width() / 2;
	shot.Intrinsics.DistorCenterPx[1] = height() / 2;
	shot.Intrinsics.CenterPx[0] = width() / 2;
	shot.Intrinsics.CenterPx[1] = height() / 2;
	shot.Intrinsics.ViewportPx[0] = width();
	shot.Intrinsics.ViewportPx[1] = height();

	double viewportYMm = shot.Intrinsics.PixelSizeMm[1] * shot.Intrinsics.ViewportPx[1];
	shot.Intrinsics.FocalMm = viewportYMm / (2 * tanf(vcg::math::ToRad(this->fovDefault() / 2.0f))); //27.846098mm

	shot.Extrinsics.SetIdentity();
}

QPair<Shotm, float> GLArea::shotFromTrackball()
{
	Shotm shot;
	initializeShot(shot);

	double viewportYMm = shot.Intrinsics.PixelSizeMm[1] * shot.Intrinsics.ViewportPx[1];
	shot.Intrinsics.FocalMm = viewportYMm / (2 * tanf(vcg::math::ToRad(fov / 2)));

	// in MeshLab, fov < 5.0 means orthographic camera
	if (fov > 5.0)
		shot.Intrinsics.cameraType = 0; //perspective
	else
		shot.Intrinsics.cameraType = 1; //orthographic

	float cameraDist = getCameraDistance();

	//add the translation introduced by gluLookAt() (0,0,cameraDist), in order to have te same view---------------
	//T(gl)*S*R*T(t) => SR(gl+t) => S R (S^(-1)R^(-1)gl + t)
	//Add translation S^(-1) R^(-1)(gl)
	//Shotd doesn't introduce scaling
	//---------------------------------------------------------------------
	shot.Extrinsics.SetTra(shot.Extrinsics.Tra() + (Inverse(shot.Extrinsics.Rot()) * Point3m(0, 0, cameraDist)));

	Shotm newShot = track2ShotCPU<Shotm::ScalarType>(shot, &trackball);

	return QPair<Shotm, float>(newShot, trackball.track.sca);
}

void GLArea::loadShot(const QPair<Shotm, float>& shotAndScale) {

	Shotm shot = shotAndScale.first;

	fov = (shot.Intrinsics.cameraType == 0) ? shot.GetFovFromFocal() : 5.0;

	float cameraDist = getCameraDistance();

	//reset trackball. The point of view must be set only by the shot
	trackball.Reset();
	trackball.track.sca = shotAndScale.second;

	shot2Track(shot, cameraDist, trackball);

	update();
}

void GLArea::createOrthoView(QString dir)
{
	resetGlobalTrackBall();
}

void GLArea::toggleOrtho()
{
	if (fov == 5.0)
		fov = 35.0;
	else
		fov = 5.0;

	update();
}

void GLArea::trackballStep(QString dir)
{
	float stepAngle = M_PI / 12.0;

	if (dir == tr("Horizontal +"))
		trackball.track.rot = Quaternionf(-stepAngle, Point3f(0.0, 1.0, 0.0)) * trackball.track.rot;
	else if (dir == tr("Horizontal -"))
		trackball.track.rot = Quaternionf(stepAngle, Point3f(0.0, 1.0, 0.0)) * trackball.track.rot;
	else if (dir == tr("Vertical +"))
		trackball.track.rot = Quaternionf(-stepAngle, Point3f(1.0, 0.0, 0.0)) * trackball.track.rot;
	else if (dir == tr("Vertical -"))
		trackball.track.rot = Quaternionf(stepAngle, Point3f(1.0, 0.0, 0.0)) * trackball.track.rot;
	else if (dir == tr("Axial +"))
		trackball.track.rot = Quaternionf(-stepAngle, Point3f(0.0, 0.0, 1.0)) * trackball.track.rot;
	else if (dir == tr("Axial -"))
		trackball.track.rot = Quaternionf(stepAngle, Point3f(0.0, 0.0, 1.0)) * trackball.track.rot;

	update();
}

void GLArea::completeUpdateRequested()
{
	makeCurrent();

	update();
}

void GLArea::meshAdded(int /*index*/)
{
	emit updateLayerTable();
}

void GLArea::meshRemoved(int /*index*/)
{
	emit updateLayerTable();
}

void GLArea::setupTextureEnv(GLuint textid)
{
	makeCurrent();
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textid);
	if (glas.textureMagFilter == 0)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (glas.textureMinFilter == 0)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopAttrib();
}

MainWindow* GLArea::mw()
{
	QObject* curParent = parent();
	while (qobject_cast<MainWindow*>(curParent) == 0)
	{
		curParent = curParent->parent();
	}
	return qobject_cast<MainWindow*>(curParent);
}

void GLArea::setWhichToothAdjustSlot(QString toothFdiLabel)
{
	if (toothFdiLabel.isEmpty())
	{
		return;
	}

	emit setWhichToothAdjustMovementTableShowSignal(toothFdiLabel);

	emit tabUpperOrLowerJawMovemnetTableShowSignal(bCurrentOperationToothUpperLower);

}

void GLArea::showViewDirectionExpand(int viewDirection)
{
	showViewDirection((ViewDirection)viewDirection);
}

void GLArea::showViewDirection(ViewDirection viewDirection)
{
	bool bUpperJawChecked = true;
	bool bLowerJawChecked = true;
	p_view_gui_->setDentalBtnState((ui_common::eViewDirection)viewDirection);

	if (viewDirection == UPPER_VISIBLE_SWITCH)
	{
		if (PFusionAlignData->p_upper_mesh_ != nullptr)
		{
			PFusionAlignData->p_upper_mesh_->setVisible(true);
		}
		if (PFusionAlignData->p_lower_mesh_ != nullptr)
		{
			PFusionAlignData->p_lower_mesh_->switchVisible();
		}

		if (PFusionAlignData->upper_dental_features_ != nullptr)
		{
			PFusionAlignData->upper_dental_features_->setVisible(true);
		}
		if (PFusionAlignData->lower_dental_features_ != nullptr)
		{
			PFusionAlignData->lower_dental_features_->setVisible(!PFusionAlignData->lower_dental_features_->bVisible_);
		}

		if (PFusionAlignData->upperManager() != nullptr)
		{
			PFusionAlignData->upperManager()->setVisible(true);
		}
		if (PFusionAlignData->lowerManager() != nullptr)
		{
			PFusionAlignData->lowerManager()->setVisible(!PFusionAlignData->lowerManager()->bVisible_);
		}
		update();
		return;
	}
	if (viewDirection == LOWER_VISIBLE_SWITCH)
	{
		if (PFusionAlignData->p_upper_mesh_ != nullptr)
		{
			PFusionAlignData->p_upper_mesh_->switchVisible();
		}
		if (PFusionAlignData->p_lower_mesh_ != nullptr)
		{
			PFusionAlignData->p_lower_mesh_->setVisible(true);
		}

		if (PFusionAlignData->upper_dental_features_ != nullptr)
		{
			PFusionAlignData->upper_dental_features_->setVisible(!PFusionAlignData->upper_dental_features_->bVisible_);
		}
		if (PFusionAlignData->lower_dental_features_ != nullptr)
		{
			PFusionAlignData->lower_dental_features_->setVisible(true);
		}

		if (PFusionAlignData->upperManager() != nullptr)
		{
			PFusionAlignData->upperManager()->setVisible(!PFusionAlignData->upperManager()->bVisible_);
		}
		if (PFusionAlignData->lowerManager() != nullptr)
		{
			PFusionAlignData->lowerManager()->setVisible(true);
		}

		update();
		return;
	}

	viewDir = viewDirection;
	switch (viewDirection)
	{
	case TOP_VIEW:
		bUpperJawChecked = false;
		bLowerJawChecked = true;
		p_view_gui_->setDentalBtnState(false, true);
		break;
	case BOTTOM_VIEW:
		bUpperJawChecked = true;
		bLowerJawChecked = false;
		p_view_gui_->setDentalBtnState(true, true);
		break;
	case FRONT_VIEW:
	case BACK_VIEW:
	case LEFT_VIEW:
	case RIGHT_VIEW:
		bUpperJawChecked = true;
		bLowerJawChecked = true;
		break;
	default:
		break;
	}

	bShowUpperTooth = bUpperJawChecked;
	bShowLowerTooth = bLowerJawChecked;

	if (PFusionAlignData->p_upper_mesh_ != nullptr)
	{
		PFusionAlignData->p_upper_mesh_->setVisible(bShowUpperTooth);
	}
	if (PFusionAlignData->p_lower_mesh_ != nullptr)
	{
		PFusionAlignData->p_lower_mesh_->setVisible(bShowLowerTooth);
	}
	if (PFusionAlignData->upper_dental_features_ != nullptr)
	{
		PFusionAlignData->upper_dental_features_->setVisible(bShowUpperTooth);
	}
	if (PFusionAlignData->lower_dental_features_ != nullptr)
	{
		PFusionAlignData->lower_dental_features_->setVisible(bShowLowerTooth);
	}
	if (PFusionAlignData->upperManager())
	{
		PFusionAlignData->upperManager()->setVisible(bShowUpperTooth);
	}
	if (PFusionAlignData->lowerManager())
	{
		PFusionAlignData->lowerManager()->setVisible(bShowLowerTooth);
	}

	Point3m _cameraPos = viewTabPlane.center + viewTabPlane.axisYV * 10.0f;
	switch (viewDir)
	{
	case TOP_VIEW:
	{
		autoRotateCameraToWantedDirectSlot(viewTabPlane.axisZV, viewTabPlane.axisXV, _cameraPos, 0.04);
	}
	break;

	case BOTTOM_VIEW:
	{
		autoRotateCameraToWantedDirectSlot(-viewTabPlane.axisZV, viewTabPlane.axisXV, _cameraPos, 0.04);
	}
	break;

	case FRONT_VIEW:
	{
		autoRotateCameraToWantedDirectSlot(-viewTabPlane.axisYV, viewTabPlane.axisXV, _cameraPos);
	}
	break;

	case BACK_VIEW:
	{
		autoRotateCameraToWantedDirectSlot(viewTabPlane.axisYV, -viewTabPlane.axisXV, _cameraPos);
	}
	break;

	case LEFT_VIEW:
	{
		if (bHavePreferViewDirect_)
		{
			Point3m temp_right_hand_direct = -(left_prefer_view_direct_ ^ viewTabPlane.axisZV).Normalize();
			autoRotateCameraToWantedDirectSlot(left_prefer_view_direct_, temp_right_hand_direct, _cameraPos);
		}
		else
		{
			autoRotateCameraToWantedDirectSlot(-viewTabPlane.axisXV, -viewTabPlane.axisYV, _cameraPos);
		}
	}
	break;

	case RIGHT_VIEW:
	{
		if (bHavePreferViewDirect_)
		{
			Point3m temp_right_hand_direct = -(right_prefer_view_direct_ ^ viewTabPlane.axisZV).Normalize();
			autoRotateCameraToWantedDirectSlot(right_prefer_view_direct_, temp_right_hand_direct, _cameraPos);
		}
		else
		{
			autoRotateCameraToWantedDirectSlot(viewTabPlane.axisXV, viewTabPlane.axisYV, _cameraPos);
		}
	}
	break;
	}
	update();
}

void GLArea::dentalCompareModelShow()
{
	bEnableDisplayDentalCompareModel = !bEnableDisplayDentalCompareModel;
	if (PFusionAlignData->upperManager())
	{
		PFusionAlignData->upperManager()->cDental.setCompareModelVisible(bEnableDisplayDentalCompareModel);
	}
	if (PFusionAlignData->lowerManager())
	{
		PFusionAlignData->lowerManager()->cDental.setCompareModelVisible(bEnableDisplayDentalCompareModel);
	}
	udpateEachMeshVertexInfoBuffer();
	update();
}

void GLArea::dentalRenderObjectSwitch()
{
	if (PFusionAlignData->upperManager() != nullptr)
	{
		if (PFusionAlignData->upperManager()->cDental.render_obj_ == Dental::VIRTUAL_JAW_ARRANGE)
		{
			PFusionAlignData->upperManager()->cDental.setRenderObject(Dental::REAL_ROOT_JAWBONE);
		}
		else if (PFusionAlignData->upperManager()->cDental.render_obj_ == Dental::REAL_ROOT_JAWBONE)
		{
			PFusionAlignData->upperManager()->cDental.setRenderObject(Dental::VIRTUAL_JAW_ARRANGE);
		}
	}
	if (PFusionAlignData->lowerManager() != nullptr)
	{
		if (PFusionAlignData->lowerManager()->cDental.render_obj_ == Dental::VIRTUAL_JAW_ARRANGE)
		{
			PFusionAlignData->lowerManager()->cDental.setRenderObject(Dental::REAL_ROOT_JAWBONE);
		}
		else if (PFusionAlignData->lowerManager()->cDental.render_obj_ == Dental::REAL_ROOT_JAWBONE)
		{
			PFusionAlignData->lowerManager()->cDental.setRenderObject(Dental::VIRTUAL_JAW_ARRANGE);
		}
	}
	update();
}

void GLArea::deleteMeshModel(int _id)
{
	for (int i = 0; i < md()->meshList.size(); ++i)
	{
		if (md()->meshList[i]->id() == _id)
		{
			MeshModel* pMesh = md()->meshList[i];
			this->md()->delMesh(pMesh);
			break;
		}
	}
}

void GLArea::setStepIterateSmoothReceive(int step)
{
	emit setStepIterateSmooth(step);
}

void GLArea::resetGlobalTrackBall()
{
	trackball.Reset();
	trackball.center.SetZero();
	float newScale = 3.0f / this->md()->bbox().Diag();
	trackball.track.sca = newScale;
	trackball.track.tra.Import(-this->md()->bbox().Center());
}

void GLArea::updateNoAdjustValueSlot(QString frontNoAdjustValue, QString allNoAdjustValue)
{
	emit updateNoAdjustValueSignal(frontNoAdjustValue, allNoAdjustValue);
}

void GLArea::setBoltonValueTeethSlot(QString frontTeetBoltonValue, QString allTeetBoltonValue)
{
	emit getBoltonValueTeethSignal(frontTeetBoltonValue, allTeetBoltonValue);
}

void GLArea::setCrowdingDegreeParaInfo(float dentalArchShouldHave, float dentalArchNowHave,
	float crowdingDegreeValue)
{
	emit getCrowdingDegreeParaInfo(dentalArchShouldHave, dentalArchNowHave, crowdingDegreeValue);
}

void GLArea::reDrawDentalArchCommandReceive()
{
	emit reDrawDentalArchCommand();
}

void GLArea::setShowOrHideUpperToothCommand(int status)
{
}

void GLArea::setShowOrHideLowerToothCommand(int status)
{
}

void GLArea::setShowOrHideUpperTooth(int status)
{
	bShowUpperTooth = status > 0 ? true : false;

	if (PFusionAlignData->upperManager() != nullptr)
	{
		PFusionAlignData->upperManager()->setVisible(bShowUpperTooth);
	}
	update();
}

void GLArea::setShowOrHideLowerTooth(int status)
{
	bShowLowerTooth = status > 0 ? true : false;

	if (PFusionAlignData->lowerManager() != nullptr)
	{
		PFusionAlignData->lowerManager()->setVisible(bShowLowerTooth);
	}

	update();
}

void GLArea::setLightnessAlphaUpperTooth(int step)
{
	bDrawUpperMeshOrder = true;
	bOpenTransparencyAdjust = true;

	foreach(MeshModel * mp, this->md()->meshList)
	{
		if (mp->visible && belongToUpperDentalPart(mp->upperOrLowerToothModelMark))
		{
			for (auto& v : mp->cm.vert)
			{
				v.C().W() = step;
			}
			MLSceneGLSharedDataContext* shared = mvc()->sharedDataContext();
			if (shared != NULL)
			{
				MLRenderingData::RendAtts atts;
				atts[MLRenderingData::ATT_NAMES::ATT_VERTPOSITION] = true;
				atts[MLRenderingData::ATT_NAMES::ATT_VERTCOLOR] = true;
				shared->meshAttributesUpdated(mp->id(), false, atts);
				shared->manageBuffers(mp->id());
			}
		}
	}
	update();
}

void GLArea::setLightnessAlphaLowerTooth(int step)
{
	bDrawUpperMeshOrder = false;
	bOpenTransparencyAdjust = true;

	foreach(MeshModel * mp, this->md()->meshList)
	{
		if (mp->visible && belongToLowerDentalPart(mp->upperOrLowerToothModelMark))
		{
			for (auto& v : mp->cm.vert)
			{
				v.C().W() = step;
			}
			MLSceneGLSharedDataContext* shared = mvc()->sharedDataContext();
			if (shared != NULL)
			{
				MLRenderingData::RendAtts atts;
				atts[MLRenderingData::ATT_NAMES::ATT_VERTPOSITION] = true;
				atts[MLRenderingData::ATT_NAMES::ATT_VERTCOLOR] = true;
				shared->meshAttributesUpdated(mp->id(), false, atts);
				shared->manageBuffers(mp->id());
			}
		}
	}
	update();
}

void GLArea::udpateEachMeshVertexInfoBuffer()
{
	foreach(MeshModel * mp, this->md()->meshList)
	{
		bool enable = false;
		if (this->b_is_clone_)
		{
			if (mp->_labelSegmentMark == COMPAREOBJ_LABEL)
			{
				enable = true;
			}
		}
		else
		{
			//enable = mp->visible;
			enable = true;
		}

		if (enable)
		{
			MLRenderingData dt;
			parentmultiview->sharedDataContext()->getRenderInfoPerMeshView(mp->id(), context(), dt);

			MLSceneGLSharedDataContext* shared = mvc()->sharedDataContext();
			if (shared != NULL)
			{
				MLRenderingData::PRIMITIVE_MODALITY pr = MLRenderingData::PR_POINTS;
				if (mp->cm.FN() > 0)
				{
					pr = MLRenderingData::PR_SOLID;
				}
				MLRenderingData::RendAtts atts;
				atts[MLRenderingData::ATT_NAMES::ATT_VERTPOSITION] = true;
				atts[MLRenderingData::ATT_NAMES::ATT_VERTNORMAL] = true;
				atts[MLRenderingData::ATT_NAMES::ATT_VERTCOLOR] = true;
				MLPerViewGLOptions opts;
				dt.get(opts);
				opts._sel_enabled = true;
				opts._face_sel = true;
				opts._vertex_sel = true;
				dt.set(opts);
				dt.set(MLRenderingData::PR_SOLID, atts);
				MLPoliciesStandAloneFunctions::disableRedundatRenderingDataAccordingToPriorities(dt);
				parentmultiview->sharedDataContext()->setRenderingDataPerMeshView(mp->id(), context(), dt);
				parentmultiview->sharedDataContext()->manageBuffers(mp->id());

				shared->meshAttributesUpdated(mp->id(), false, atts);
				shared->manageBuffers(mp->id());
			}
		}
	}
	update();
}

void GLArea::getQtDeviceToLogicalValueSlot(float _a, float& _b)
{
	_b = QTDeviceToLogical(this, _a);
}

void GLArea::getQtLogicalToDeviceValueSlot(float _a, float& _b)
{
	_b = QTLogicalToDevice(this, _a);
}

void GLArea::setTransparencyUpperTooth(int step)
{
}

void GLArea::setTransparencyLowerTooth(int step)
{

}
//
//
//
//	lastModelEdited = this->md()->mm();
//
//	/*_oldvalues.clear();
//	parentmultiview->sharedDataContext()->getRenderInfoPerMeshView(context(), _oldvalues);*/
//
//	else
//
//	if (!iEdit->StartEdit(*this->md(), this, parentmultiview->sharedDataContext()))
//	{
//
//	else
//	{
//		endEdit();
void GLArea::drawDentalCompareModel(QPainter* painter)
{
}

void GLArea::getCurrentCameraState(MeshModel* curMesh)
{
	// Set camera view direction as projection plane normal vector, determine curAxis direction based on rotation axis direction
	Point3d tempProjPoint1 = UtilityTools::getInstance()->projectPoint(Point3m(this->width() / 2.0f, this->height() / 2.0f, 0), curMesh);
	Point3d tempProjPoint2 = UtilityTools::getInstance()->projectPoint(Point3m(this->width() / 2.0f, this->height() / 2.0f, 1), curMesh);
	Point3d tempProjPoint3 = UtilityTools::getInstance()->projectPoint(Point3m(this->width(), this->height() / 2.0f, 0), curMesh);
	this->viewDirect.X() = tempProjPoint2.X() - tempProjPoint1.X();
	this->viewDirect.Y() = tempProjPoint2.Y() - tempProjPoint1.Y();
	this->viewDirect.Z() = tempProjPoint2.Z() - tempProjPoint1.Z();
	this->viewDirect = this->viewDirect.Normalize();

	this->view_RH_Direct.X() = tempProjPoint3.X() - tempProjPoint1.X();
	this->view_RH_Direct.Y() = tempProjPoint3.Y() - tempProjPoint1.Y();
	this->view_RH_Direct.Z() = tempProjPoint3.Z() - tempProjPoint1.Z();
	this->view_RH_Direct = this->view_RH_Direct.Normalize();

	this->cameraPosition = Point3m(tempProjPoint1.X(), tempProjPoint1.Y(), tempProjPoint1.Z());
}

void GLArea::isShowToothAdjustCtlBtnSlot(bool bShow)
{
	bShowToothAdjustCtlBtn = bShow;
}

void GLArea::autoRotateCameraToWantedDirectSlot(Point3m aimViewDirect, Point3m aimView_RH_Direct, Point3m aimCameraPos, float aimScale /*= -1*/)
{
	aimViewDirect = aimViewDirect.Normalize();
	this->cameraRotateEndAxis = aimViewDirect;
	this->cameraTransEndPoint = aimCameraPos;

	Point3m tempX = aimView_RH_Direct;
	Point3m tempZ = aimViewDirect;
	Point3m tempY = (tempZ ^ tempX).Normalize();
	this->cameraEndAxis = Axis(Point3m(0, 0, 0), tempX, tempY, tempZ);

	this->cameraTransEndScale = aimScale;

	setCameraRotateParameters = !setCameraRotateParameters;
	this->update();
}

void GLArea::autoRotateCameraToWantedDirect(Point3m aimViewDirect)
{
	if (this->rotTimer != nullptr)
	{
		this->iCamStep = 0;
		this->moveCamera = false;
		disconnect(this->rotTimer);
		this->rotTimer->stop();
		delete this->rotTimer;
		this->rotTimer = nullptr;
		update();
	}

	if (this->rotTimer == nullptr)
	{
		emit PSIGNALMANAGER->setEnableToothOcclusionSignal(false);
		//vector<Axis>().swap(camTransTrial);
		getCurrentCameraState(*this->md()->meshList.begin());
		this->cameraTransStartPoint = -this->trackball.track.tra;
		Point3m tempX = this->view_RH_Direct;
		Point3m tempZ = this->viewDirect;
		Point3m tempY = (tempZ ^ tempX).Normalize();
		this->cameraStartAxis = Axis(Point3m(0, 0, 0), tempX, tempY, tempZ);
		this->cameraStartAxis.computeTransformVectors(this->cameraEndAxis, this->cameraMoveVector, this->cameraRotateVector);
		this->cameraTransStartScale = this->trackball.track.sca;
		if (this->cameraTransStartScale >= 0.5)
		{
			cameraTransStartScale = 0.5f;
		}
		//qDebug() << "CamRotateSum : " << cameraRotateVector.X() << ", " << cameraRotateVector.Y() << ", " << cameraRotateVector.Z() << endl;

		this->iCamStep = 0;
		this->rotTimer = new QTimer(this);
		connect(rotTimer, SIGNAL(timeout()), this, SLOT(adjustRotateStep()));
		this->rotTimer->start(1);
	}
}

void GLArea::resetTrackballCenterSlot()
{
	this->resetTrackCenter = true;
	this->update();
}

void GLArea::adjustRotateStep()
{
	if (this->iCamStep <= 50)
	{
		float angle = (PI / 2.0f) * (iCamStep / 50.0f);
		float t = sinf(angle);
		Point3m curRotateVector = this->cameraRotateVector * t;
		Point3m axisXVert = this->cameraStartAxis.axisXVector;
		Point3m axisYVert = this->cameraStartAxis.axisYVector;
		Point3m axisZVert = this->cameraStartAxis.axisZVector;
		vcg::Matrix44f transMat;
		vcg::Matrix44f transform, transformInverse;
		transform.SetIdentity();
		transform.SetTranslate(this->cameraStartAxis.centerPoint);
		transformInverse.SetTranslate(-this->cameraStartAxis.centerPoint);

		vcg::Matrix44f transformValue, rotMatLocal;
		transMat.SetIdentity();
		rotMatLocal.SetIdentity();

		transformValue.SetRotateDeg(curRotateVector.X(), this->cameraStartAxis.axisXVector);
		rotMatLocal *= transformValue;
		transformValue.SetRotateDeg(curRotateVector.Y(), this->cameraStartAxis.axisYVector);
		rotMatLocal *= transformValue;
		transformValue.SetRotateDeg(curRotateVector.Z(), this->cameraStartAxis.axisZVector);
		rotMatLocal *= transformValue;

		transform *= rotMatLocal * transMat;
		transform *= transformInverse;

		axisXVert = (transform * axisXVert).Normalize();
		axisYVert = (transform * axisYVert).Normalize();
		axisZVert = (transform * axisZVert).Normalize();
		this->aimCameraViewAxis = axisZVert;
		this->aimCameraView_RH_Axis = axisXVert;

		if (iCamStep == 50)
		{
			this->aimCameraViewAxis = this->cameraEndAxis.axisZVector;
			this->aimCameraView_RH_Axis = this->cameraEndAxis.axisXVector;
		}

		iCamStep += 1;
		this->moveCamera = true;

		update();
		return;
	}
	else
	{
		this->cameraTransStartScale = -1;
		this->cameraTransEndScale = -1;
		this->iCamStep = 0;
		this->moveCamera = false;
		disconnect(this->rotTimer);
		this->rotTimer->stop();
		delete this->rotTimer;
		this->rotTimer = nullptr;
		emit PSIGNALMANAGER->setEnableToothOcclusionSignal(true);
		update();
		return;
	}
}

void GLArea::adjustCamera()
{
	if (this->tempTimer == NULL)
	{
		this->tempTimer = new QTimer(this);
		connect(tempTimer, SIGNAL(timeout()), this, SLOT(adjustForwardStep()));
		this->tempTimer->start(1);
	}
}

void GLArea::adjustForwardStep()
{
	Point3m sCamPos, eCamPos;
	sCamPos = Point3m(0, 0, 10);
	eCamPos = Point3m(0, 0, 2);

	if (this->iCamStep <= 50)
	{
		float angle = (PI / 2.0f) * (iCamStep / 50.0f);
		float t = sinf(angle);
		cameraPos = sCamPos * (1 - t) + eCamPos * t;
		Point3m startCamPos;
		viewDir = DEFAULT_VIEW;
		resetGlobalTrackBall();
		update();
		iCamStep += 1;
		return;
	}
	else
	{
		this->iCamStep = 0;
		disconnect(this->tempTimer);
		this->tempTimer->stop();
		delete this->tempTimer;
		this->tempTimer = NULL;

		cameraPos = eCamPos;
		Point3m startCamPos;
		viewDir = DEFAULT_VIEW;
		resetGlobalTrackBall();
		update();
		return;
	}
}

void GLArea::setMainWindowFocus()
{
	bool bFocus = this->mw()->hasFocus();
	if (!bFocus)
	{
		this->mw()->setFocus();
	}
	return;
}

void GLArea::setAlignProjectToSaveAuto(bool _bSaveAlignProject /* = true */)
{
	this->bSaveAlignProjectAuto = _bSaveAlignProject;
}

void GLArea::setSegmentProjectToSaveAuto(bool _bSaveSegmentProject /* = true */)
{
	this->bSaveSegmentProjectAuto = _bSaveSegmentProject;
}

bool GLArea::getWhetherAlignProjectNeedToSaveAuto()const
{
	return this->bSaveAlignProjectAuto;
}

bool GLArea::getWhetherSegmentProjectNeedToSaveAuto()const
{
	return this->bSaveSegmentProjectAuto;
}

void GLArea::setCurrentToothMeshOperateSlot(bool bUpperOrTooth)
{
	bCurrentOperationToothUpperLower = bUpperOrTooth;
	emit needUpdateTabNow();
	if (PFusionAlignData->curManager() != nullptr)
	{
		emit PSIGNALMANAGER->setTreatmentTabSelectItem(PFusionAlignData->curManager()->cDental.currentSelectedStage + 1, 0);
	}
	update();
}

void GLArea::onSelectDentalSlot(int dentalType)
{
	if (dentalType == E_SELECT_UP_DENTAL)
	{
		mw()->setUpperToothOperator();
		setCurrentToothMeshOperateSlot(true);
	}
	else
	{
		mw()->setLowerToothOperator();
		setCurrentToothMeshOperateSlot(false);
	}
}

void GLArea::setGlobalTrackBallEnableSlot(bool enble)
{
	this->bWhetherOpenGlobalTrackball = enble;
	this->update();
}

void GLArea::setMeshDocCurrentMeshSlot(int _index)
{
	md()->setCurrentMesh(_index);
}

void GLArea::clearUI()
{
	weatherSwitchToMainMenu(false);

	/*if (bEnableDisplayDentalOverlapArea)
	{
		emit closeOcclusionToothAct();
	}*/
	if (bEnableDisplayDentalCompareModel)
	{
		emit closeTreatmentCompareAct();
	}
	if (bEnableDisplayToothMovemnetTable)
	{
		emit closeDisplayToothMovemnetTableAct();
	}
	emit PSIGNALMANAGER->setPlayAnimationGuiShowStatusIAndInitSignal(false, 0, 0);

	this->update();
}

void GLArea::onePartOfInitalAnimationOverSlot()
{

}

void GLArea::wheelStopSlot()
{
	this->bRolling = false;
	wheelTimer->stop();
	this->update();
}

void GLArea::setTrackballSphereEnable(bool bState)
{
	bBanTrackballSphereMode = bState;
}

void GLArea::drawCollectFoldedGui(QPainter* painter)
{
	makeCurrent();
	float screenRatio = float(this->width()) / float(this->height());
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(-1.0f * screenRatio, 1.0f * screenRatio, -1, 1);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDepthMask(false);
	glDepthFunc(GL_LESS);

	/////////////////////////////////////////////////////////////////////////////////

	// draw checkbox and slider here...
	float blankStartX = screenRatio / 10;
	float startPosXPlayButton = screenRatio - collect_folded_gui_->getAllLengthControl();
	float startPosYPlayButton = 0.0f;

	QPoint pointCursor = mapFromGlobal(QCursor::pos());

	Matrix44f matrixControl;
	matrixControl.SetIdentity();
	matrixControl[0][3] = startPosXPlayButton;
	matrixControl[1][3] = startPosYPlayButton;
	bool tempBoolen;
	collect_folded_gui_->draw(this, painter, pointCursor, matrixControl, tempBoolen);
	/////////////////////////////////////////////////////////////////////////////////

	glMatrixMode(GL_PROJECTION);
	glPopAttrib();
	glEnable(GL_LIGHTING);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	//restore view
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
//
void GLArea::showComparisonWindowDescription(QPainter& _p)
{
	_p.beginNativePainting();
	_p.save();
	_p.setRenderHint(QPainter::HighQualityAntialiasing);

	QFont font("yu gothic ui semibold", 15);
	font.setBold(true);
	font.setCapitalization(QFont::SmallCaps);
	font.setLetterSpacing(QFont::AbsoluteSpacing, 10);
	_p.setFont(font);

	// Set a rectangle
	QRectF rect(0, 0, width(), height());
	QString context;

	context = QString(tr("compareState : ")) + PFusionAlignData->getAnalyserData().compare_stage_name_;
	_p.setPen(QColor(Qt::white));
	_p.drawText(rect, Qt::AlignHCenter, context);

	_p.endNativePainting();
	_p.restore();
}

void GLArea::setGlareaIsClone(bool is_clone)
{

	b_is_clone_ = is_clone;

	if (compare_switch_link_btn_)
	{
		if (b_is_clone_)
		{
			compare_switch_link_btn_->show();
		}
		else
		{
			compare_switch_link_btn_->hide();
		}
	}
}

void GLArea::changeLinkBackground(bool status)
{
	if (compare_switch_link_btn_)
	{
		if (status)
		{
			compare_switch_link_btn_->setToolTip(tr("has been linked"));
			compare_switch_link_btn_->setStyleSheet(
				"QPushButton{background-color: rgba(17,31,42, 255);margin:1px; \
		border-image: url(:/res/dark/images/comparelink/link.svg) 4 4 4 4; \
		border:4px  white; } \
        QPushButton:hover{ \
	    background-color:rgb(121,129,138); }");
		}
		else
		{
			compare_switch_link_btn_->setToolTip(tr("No linkage"));
			compare_switch_link_btn_->setStyleSheet(
				"QPushButton{background-color: rgba(17,31,42, 255);margin:1px; \
		border-image: url(:/res/dark/images/comparelink/releaselink.svg) 4 4 4 4; \
		border:4px  white; } \
        QPushButton:hover{ \
	    background-color:rgb(121,129,138); }");
		}
	}
}

void GLArea::occlusalColoringInProgressSlot(bool _state)
{
	this->occlusal_coloring_in_progress_ = _state;
	/*if (occlusal_coloring_in_progress_)
	{
		if (b_is_occlusal_split_)
		{
			setViewDirection(BOTTOM_VIEW);
		}
		else
		{
			setViewDirection(TOP_VIEW);
		}
		emit PSIGNALMANAGER->setLMRViewDirectConsoleVisibleSignal(false);
		emit PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
	}
	else
	{
		showViewDirection(FRONT_VIEW);
		emit PSIGNALMANAGER->setLMRViewDirectConsoleVisibleSignal(true);
		udpateEachMeshVertexInfoBuffer();
	}*/
}
