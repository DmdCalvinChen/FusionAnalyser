/****************************************************************************
 * MeshLab                                                           o o     *
 * A versatile mesh processing toolbox                             o     o   *
 *                                                                _   O  _   *
 * Copyright(C) 2005                                                \/)\/    *
 * Visual Computing Lab                                            /\/|      *
 * ISTI - Italian National Research Council                           |      *
 *                                                                    \      *
 * All rights reserved.                                                      *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
 * for more details.                                                         *
 *                                                                           *
 ****************************************************************************/
/****************************************************************************
  History
$Log: meshedit.cpp,v $
****************************************************************************/

#include <math.h>
#include <stdlib.h>
#include "FusionAnalyser/glarea.h"
#include "overlay.h"
#include <wrap/gl/pick.h>
#include <wrap/qt/gl_label.h>
#include <wrap/qt/device_to_logical.h>
#include <QMainWindow>
#include <common_ext/data/fusionaligndata.h>
#include <common_ext/util/utility_tools.h>
#include "common_base/util/uitools.h"
#include "util/utility_tools.h"

using namespace std;
using namespace vcg;

Overlay::Overlay()
{
	QString(tr("Normal overjet"));
	QString(tr("I-degree overjet"));
	QString(tr("II-degree overjet"));
	QString(tr("III-degree overjet"));
	QString(tr("Crossbite"));
	QString(tr("Open bite"));
	QString(tr("Normal "));
	QString(tr("overbite"));
	QString(tr("I-degree open bite"));
	QString(tr("II-degree open bite"));
	QString(tr("III-degree open bite"));
	QString(tr("I-degree "));
	QString(tr("II-degree "));
	QString(tr("III-degree "));
	QString(tr("overbite"));
	QString(tr("Normal overbite"));
	QString(tr("I-degree overbite"));
	QString(tr("II-degree overbite"));
	QString(tr("III-degree overbite"));

	haveToPick = false;
	bUpdateCutOutline = false;
	bAction = false;
	bActionReady = false;
	cutface = nullptr;
	cutVertCP.clear();
	projCutVertCP.clear();
	iSelectedHandle = -1;
	pointlist = nullptr;
	connect(PSIGNALMANAGER, &SignalManager::updateSceneSizeSignal, this, &Overlay::updateLabels);
}
Overlay::Overlay(QAction* pAct)
{
	QString(tr("Normal overjet"));
	QString(tr("I-degree overjet"));
	QString(tr("II-degree overjet"));
	QString(tr("III-degree overjet"));
	QString(tr("Crossbite"));
	QString(tr("Open bite"));
	QString(tr("Normal "));
	QString(tr("overbite"));
	QString(tr("I-degree open bite"));
	QString(tr("II-degree open bite"));
	QString(tr("III-degree open bite"));
	QString(tr("I-degree "));
	QString(tr("II-degree "));
	QString(tr("III-degree "));
	QString(tr("overbite"));
	QString(tr("Normal overbite"));
	QString(tr("I-degree overbite"));
	QString(tr("II-degree overbite"));
	QString(tr("III-degree overbite"));

	haveToPick = false;
	bUpdateCutOutline = false;
	bAction = false;
	bActionReady = false;
	cutface = nullptr;
	cutVertCP.clear();
	projCutVertCP.clear();
	iSelectedHandle = -1;
	pointlist = nullptr;
	p_act_ = pAct;
	connect(PSIGNALMANAGER, &SignalManager::updateSceneSizeSignal, this, &Overlay::updateLabels);
	connect(PSIGNALMANAGER, &SignalManager::showOverlaySignal, this, &Overlay::showOverlayGifSlot);
	connect(PSIGNALMANAGER, &SignalManager::updateSceneSizeSignal, this, &Overlay::updateSceneSizeSlot);
}
const QString Overlay::Info()
{
	return tr("Overbite/Overjet");
}

void Overlay::updateSceneSizeSlot(int _width, int _height)
{
	updateAnaGuiSize();
	updateUI();
}

bool Overlay::StartEdit(MeshModel &m, GLArea *gla, MLSceneGLSharedDataContext* cont)
{
	emit PSIGNALMANAGER->mutualExclusionWithOverlaySignal();

	if (p_act_)
	{
		p_act_->setIcon(QIcon(":/images/overlap_pressed.svg"));
	}
	if (PFusionAlignData->upperManager() && PFusionAlignData->lowerManager() && !PFusionAlignData->curManager()->cDental.bBasalPlaneIsReady)
	{
		UiUtilityTools::getInstance()->showInfoMessageBox(tr("Info"), tr("The jaw does not have a occlusal plane"));
		return false;
	}

	GLenum err = glewInit();
	if (err != GLEW_OK)
		return false;

	if (this->parent != gla)
	{
		this->parent = gla;
		connect(this, &Overlay::setNeedUpdateOverlayOutlineSignal, PSIGNALMANAGER, &SignalManager::setNeedUpdateOverlayOutlineSignal);
		connect(gla, SIGNAL(changeDentalVisible()), this,  SLOT(updateCurrentSectionOutline()));
	}
	createGui(&m, gla);
	updateUI();

	if (PFusionAlignData->upperManager() && PFusionAlignData->lowerManager() && PFusionAlignData->curManager()->cDental.bArchIsReady && PFusionAlignData->curManager()->cDental.bBasalPlaneIsReady)
	{
		static bool first_time_inital = true;
		if (first_time_inital)
		{
			initalCutFaceTool();
			first_time_inital = false;
		}
		else
		{
			if (guiAna_)
			{
				guiAna_->switchToRightSide();
			}
			if (gui_)
			{
				gui_->switchToRightSide();
			}
		}
	}
	else if (PFusionAlignData->upper_dental_features_ && PFusionAlignData->lower_dental_features_)
	{
		if (PFusionAlignData->dental_analysis_)
		{
			PFusionAlignData->dental_analysis_->addBrowsedPrompt(prompt_context_);
		}

		static bool first_time_inital = true;
		bool load_record_cut_face = false;

		if (last_upper_dental_features_ != PFusionAlignData->upper_dental_features_ ||
			last_lower_dental_features_ != PFusionAlignData->lower_dental_features_)
		{
			load_record_cut_face = true;
			last_upper_dental_features_ = PFusionAlignData->upper_dental_features_;
			last_lower_dental_features_ = PFusionAlignData->lower_dental_features_;
			first_time_inital = true;
		}

		if (first_time_inital)
		{
			initalCutFaceTool(load_record_cut_face);
			first_time_inital = false;
		}

		if (guiAna_)
		{
			guiAna_->switchToRightSide();
		}
		if (gui_)
		{
			gui_->switchToRightSide();
		}
	}
	else
	{
		return false;
	}

	cutVertCP.clear();
	projCutVertCP.clear();

	emit setNeedUpdateOverlayOutlineSignal(true);

	connect(PSIGNALMANAGER, &SignalManager::setCurrentTreatmentStepSignal, this, &Overlay::updateCurrentSectionOutline);
	need_align_camera_view_ = true;
	need_update_sub_dlg_ = true;

	updateSceneSizeSlot(parent->width(), parent->height());
	return true;
}

void Overlay::EndEdit(MeshModel &m, GLArea *gla, MLSceneGLSharedDataContext* cont)
{
	if (p_act_)
	{
		QPixmap enabled_icon(QString(":/images/overlap_normal.svg"));
		QPixmap disabled_icon(QString(":/images/overlap_disabled.svg"));
		QIcon icon(enabled_icon);
		icon.addPixmap(disabled_icon, QIcon::Disabled);
		p_act_->setIcon(icon);
	}
	closeGui();
	haveToPick = false;
	emit updateDlgVisible(false);
	emit setNeedUpdateOverlayOutlineSignal(false);
	emit doneSignal();
	disconnect(PSIGNALMANAGER, &SignalManager::setCurrentTreatmentStepSignal, this, &Overlay::updateCurrentSectionOutline);
	need_align_camera_view_ = false;
	need_update_sub_dlg_ = false;
}

void Overlay::createGui(MeshModel* _mesh, GLArea* parent)
{
	// FusionAnalyser only supports Analyser mode - gui_ (Designer/Viewer) removed
	if (guiAna_ == nullptr)
	{
		QObject* pMainWindow = parent->window();
		while (qobject_cast<QMainWindow*>(pMainWindow) == 0)
		{
			pMainWindow = pMainWindow->parent();
		}

		guiAna_ = new OverlapOverbiteAnaGui(parent->window());

		// Add signals
		connect(this, &Overlay::updatePointList, guiAna_->overlayOutlinePreview, &OverlayOutlinePreview::getNewVertCouple);
		connect(this, &Overlay::loadLeftMeasureRecordSignal, guiAna_->overlayOutlinePreview, &OverlayOutlinePreview::loadLeftMeasureRecord);
		connect(this, &Overlay::loadRightMeasureRecordSignal, guiAna_->overlayOutlinePreview, &OverlayOutlinePreview::loadRighttMeasureRecord);
		connect(guiAna_->overlayOutlinePreview, &OverlayOutlinePreview::updateOverlayStateSignal, this, &Overlay::updateOverlayStateSlot);
		connect(guiAna_->overlayOutlinePreview, &OverlayOutlinePreview::updateLeftAndRightParameters, this, &Overlay::updateLeftAndRightParametersSlot);
		connect(guiAna_->overlayOutlinePreview, &OverlayOutlinePreview::setCheekSideSignal, this, &Overlay::setCheekSideSlot);
		connect(guiAna_, &OverlapOverbiteAnaGui::setStepOverlaySendSignal, this, &Overlay::updateAccordingSlider);
		connect(guiAna_, &OverlapOverbiteAnaGui::updateCutOutlineSignal, this, &Overlay::updateCurrentSectionOutline);
		connect(guiAna_, &OverlapOverbiteAnaGui::doneSignal, PSIGNALMANAGER, &SignalManager::endEditSignal);

		//QPoint pos = parent->mapToGlobal(QPoint(parent->width() - ana_gui_width_, 0));
		QPoint pos = parent->mapToGlobal(QPoint(0, 0));
		QPoint pos1 = parent->mapToGlobal(EDIT_OVERLAY_POS);
		guiAna_->setFixedWidth(ana_gui_width_);
		guiAna_->setFixedHeight(ana_gui_height_);
		guiAna_->setGeometry(pos.x(), pos.y(), ana_gui_width_, ana_gui_height_);
	}

	if (guiAna_)
	{
		guiAna_->show();
	}
	QSettings settings(QCoreApplication::applicationDirPath() + "./configs/config.ini", QSettings::IniFormat);
//		QSettings settings;
	if (gif_overlay_ui == nullptr)
	{
		bool showBtn = true;
		if (!settings.value(OverlayGif).isValid())
			settings.value(OverlayGif) = 0;
		if (settings.value(OverlayGif) == 1)
			showBtn = false;
		QMainWindow* pMainWindow = UtilityTools::getInstance()->getSubMainWindow(parent->window());
		gif_overlay_ui = new Gifviewgui(showBtn, pMainWindow);
		gif_overlay_ui->gifType = OverlayGif;
		QList<std::pair<QString, QString>> imagepairlst;
		if (PFusionAppData->getAppLanguage() == E_CHINESE)
		{
			imagepairlst = { std::make_pair(":/images/overlay.gif", ""),
			};
		}
		else if (PFusionAppData->getAppLanguage() == E_ENGLISH)
		{
			imagepairlst = { std::make_pair(":/images/overlay_ENG.gif", ""),
			};
		}
		gif_overlay_ui->showGif(imagepairlst);
		gif_overlay_ui->setGeometry(parent->window()->width() - 400, 60,
			gif_overlay_ui->width(), gif_overlay_ui->height());
	}
	if (settings.value(OverlayGif) != 1)
	{
		gif_overlay_ui->show();
	}
	if (tipBtn == nullptr)
	{
		tipBtn = new tipsWidget(parent, parent);
		tipBtn->open_gif_type = OverlayGif;
		tipBtn->show();
	}
}

void Overlay::closeGui()
{
	// FusionAnalyser only supports Analyser mode - gui_ (Designer/Viewer) removed
	if (guiAna_)
	{
		guiAna_->hide();
	}
	if (gif_overlay_ui)
	{
		gif_overlay_ui->hide();
	}
	if (tipBtn)
	{
		tipBtn->hide();
	}
}

void Overlay::updateUI(bool visible)
{
	// FusionAnalyser only supports Analyser mode - gui_ (Designer/Viewer) removed
	if (guiAna_ && need_update_sub_dlg_)
	{
		//QPoint pos = parent->mapToGlobal(QPoint(parent->width() - ana_gui_width_, 0));
		QPoint pos = parent->mapToGlobal(QPoint(0, 0));
		QPoint pos1 = parent->mapToGlobal(EDIT_OVERLAY_POS);
		guiAna_->setFixedWidth(ana_gui_width_);
		guiAna_->setFixedHeight(ana_gui_height_);
		guiAna_->setGeometry(pos.x(),
			pos.y(), ana_gui_width_, ana_gui_height_);

		guiAna_->update();
		guiAna_->setVisible(visible);
		guiAna_->show();
	}
	QSettings settings(QCoreApplication::applicationDirPath() + "./configs/config.ini", QSettings::IniFormat);
//		QSettings settings;
	if (gif_overlay_ui)
	{
		if (settings.value(OverlayGif) == 1)
		{
			gif_overlay_ui->showButton(false);
		}
		gif_overlay_ui->setGeometry(parent->window()->width() - 400, 60,
			gif_overlay_ui->width(), gif_overlay_ui->height());
	}
	if (tipBtn && need_update_sub_dlg_)
	{
		QPoint pointOri = tipBtn->p_gla_->mapToGlobal(QPoint(0, 0));
		tipBtn->setGeometry(pointOri.x() + tipBtn->p_gla_->width() - tipBtn->width() / 1.2,
			pointOri.y(),
			tipBtn->width(), tipBtn->height());
		tipBtn->setVisible(visible);
	}
}

void Overlay::setCheekSideSlot(OverlayOutlinePreview::CheekSide _side)
{
	this->cheek_side_ = _side;
	if (cheek_side_ == OverlayOutlinePreview::RIGHT_SIDE)
	{
		*cutface = right_face_;
	}
	else if (cheek_side_ == OverlayOutlinePreview::LEFT_SIDE)
	{
		*cutface = left_face_;
	}
	bUpdateCutOutline = true;
	parent->update();
}

void Overlay::Decorate(MeshModel &m, GLArea * gla, QPainter *p)
{
	updateUI();

	if (need_align_camera_view_)
	{
		emit PSIGNALMANAGER->setViewDirByPhotosMouseSelectedSignal(Interactive2DLabel::FRONT_VIEW);
		need_align_camera_view_ = false;
	}

	if (cutface != nullptr)
	{
		Point3m viewDirect, cameraPos;
		UtilityTools::getInstance()->getCurrentCameraState(&m, parent->width(), parent->height(), viewDirect, cameraPos);
		cutface->updateCtrlSystem(cameraPos, viewDirect);
		cutface->draw(parent->width(), parent->height());
	}

	if (bCanHandles)
	{
		if (cutface->pickAdjHandle(mouseX, mouseY, pickedIndex_))
		{
			parent->bWhetherOpenGlobalTrackball = false;
			bActionReady = true;
		}
		else
		{
			parent->bWhetherOpenGlobalTrackball = true;
			bActionReady = false;
			pickedIndex_ = NONE_CTRL_NODE;
		}
		bCanHandles = false;
	}

	if (bAction)
	{
		cutface->transformAction(pickedIndex_, mouseX, mouseY,
			parent->width(), parent->height());

		if (pickedIndex_ == TRANSLATE_CTRL_NODE)
		{
			adjustCutFacePos(cutface);
		}
		//bUpdateCutOutline = true;
		bAction = false;
	}

	if (bUpdateCutOutline)
	{
		cutVertCP.clear();
		getCutOutline();
		getProjectCutOutline();
		setNewPointList();

		bUpdateCutOutline = false;
	}

	if (guiAna_ && guiAna_->overlayOutlinePreview)
	{
		if (guiAna_->overlayOutlinePreview->left_measure_edge_)
		{
			Point3m a = guiAna_->overlayOutlinePreview->left_measure_edge_->a;
			Point3m b = guiAna_->overlayOutlinePreview->left_measure_edge_->b;
			PFusionAlignData->dental_analysis_->left_overlay_measure_edge_ = FEdge(a, b);
		}
		else
		{
			Point3m a(0,0,0), b(0, 0, 0);
			PFusionAlignData->dental_analysis_->left_overlay_measure_edge_ = FEdge(a, b);
		}

		if (guiAna_->overlayOutlinePreview->right_measure_edge_)
		{
			Point3m a = guiAna_->overlayOutlinePreview->right_measure_edge_->a;
			Point3m b = guiAna_->overlayOutlinePreview->right_measure_edge_->b;
			PFusionAlignData->dental_analysis_->right_overlay_measure_edge_ = FEdge(a, b);
		}
		else
		{
			Point3m a(0, 0, 0), b(0, 0, 0);
			PFusionAlignData->dental_analysis_->right_overlay_measure_edge_ = FEdge(a, b);
		}
	}

	myDraw(p);
}

void Overlay::DecorateBeforeMeshRender(MeshModel &m, GLArea * gla, QPainter *p)
{

}

void Overlay::myDraw(QPainter *p)
{
	//drawDentalArch();

	// FusionAnalyser only supports Analyser mode
	if (!labels_.empty())
	{
		for (int i = 0; i < labels_.size(); ++i)
		{
			labels_[i].draw(p);
		}
	}

	if (!frames_.empty())
	{
		for (int i = 0; i < frames_.size(); ++i)
		{
			frames_[i].draw(p);
		}
	}
}

void Overlay::mousePressEvent(QMouseEvent * event, MeshModel &m, GLArea *gla)
{
	mouseX = QTLogicalToDevice(gla, event->pos().x());
	mouseY = QTLogicalToDevice(gla, gla->height() - event->pos().y());

	mouseStart.X() = mouseX;
	mouseStart.Y() = mouseY;
	mouseStart.Z() = 0;
	bCanHandles = true;
}

void Overlay::mouseMoveEvent(QMouseEvent * event, MeshModel &m, GLArea *gla)
{
	mouseX = QTLogicalToDevice(gla, event->pos().x());
	mouseY = QTLogicalToDevice(gla, gla->height() - event->pos().y());

	if (bActionReady)
	{
		mouseEnd.X() = mouseX;
		mouseEnd.Y() = mouseY;
		mouseEnd.Z() = 0;

		if (mouseEnd == mouseStart)
		{
			bAction = false;
		}
		else
		{
			bAction = true;
		}

		setNewPointList();
	}
	gla->update();
}

void Overlay::mouseReleaseEvent(QMouseEvent * event, MeshModel &m, GLArea *gla)
{
	const Point2f loc = QTLogicalToOpenGL(gla, event->pos());
	curGL = QPoint(int(loc.X()), int(loc.Y()));

	if (bActionReady)
	{
		bUpdateCutOutline = true;
		copyCurrentStateToCheekSide();
	}
	haveToPick = true;
	bActionReady = false;
	bAction = false;
	bCanHandles = false;
	gla->bWhetherOpenGlobalTrackball = true;
	iSelectedHandle = -1;

	gla->update();
}

void Overlay::keyReleaseEvent(QKeyEvent *event, MeshModel &m, GLArea *gla)
{
	if (event->key() == Qt::Key_Right)
	{
		if (fSliderDistance + 0.1f < fSliderDistanceSum)
			fSliderDistance += 0.1f;
		ensureCutFaceBySilderDistance();
		setNewPointList();
	}
	if (event->key() == Qt::Key_Left)
	{
		if (fSliderDistance - 0.1f > 0)
			fSliderDistance -= 0.1f;
		ensureCutFaceBySilderDistance();
		setNewPointList();
	}

	gla->update();
}

void Overlay::initalCutFaceTool(bool _load_record_cut_face)
{
	if (PFusionAlignData->upperManager() || PFusionAlignData->lowerManager())
	{
		this->iSegmentNumber = PFusionAlignData->curManager()->cDental.dentalArch.size() - 1;
		this->fSliderDistanceSum = PFusionAlignData->curManager()->cDental.fDentalArchLength;
		this->fSliderDistance = fSliderDistanceSum / 2.0f;
		archPoints = PFusionAlignData->curManager()->cDental.dentalArch;
	}
	else if (PFusionAlignData->upper_dental_features_)
	{
		this->iSegmentNumber = PFusionAlignData->upper_dental_features_->dentalArch_.size() - 1;
		this->fSliderDistanceSum = PFusionAlignData->upper_dental_features_->fDentalArchLength_;
		this->fSliderDistance = fSliderDistanceSum / 2.0f;
		archPoints = PFusionAlignData->upper_dental_features_->dentalArch_;

		float cut_face_radius = PFusionAlignData->upper_dental_features_->mesh_->original_mesh_->cm.bbox.DimX();
		cut_face_radius = cut_face_radius < PFusionAlignData->upper_dental_features_->mesh_->original_mesh_->cm.bbox.DimY() ?
			cut_face_radius : PFusionAlignData->upper_dental_features_->mesh_->original_mesh_->cm.bbox.DimY();
		cut_face_radius = cut_face_radius < PFusionAlignData->upper_dental_features_->mesh_->original_mesh_->cm.bbox.DimZ() ?
			cut_face_radius : PFusionAlignData->upper_dental_features_->mesh_->original_mesh_->cm.bbox.DimZ();

		CutFace::setDefaultCutPlaneRadiusMax(cut_face_radius);
	}

	for (int i = 0; i < iSegmentNumber; i++)
	{
		float x, y, z;
		x = archPoints[i].X() - archPoints[i + 1].X();
		y = archPoints[i].Y() - archPoints[i + 1].Y();
		z = archPoints[i].Z() - archPoints[i + 1].Z();
		float tempDistance = sqrt(x * x + y * y + z * z);
		distanceList.push_back(tempDistance);
	}
	for (int i = 0; i < iSegmentNumber; i++)
	{
		float tempDistanceSummary = 0;
		for (int j = 0; j <= i; j++)
			tempDistanceSummary += distanceList[j];
		distanceAccumulate.push_back(tempDistanceSummary);
	}

	ensureCutFaceBySilderDistance();

	if (PFusionAlignData->upperManager())
	{
		for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
		{
			if (PFusionAlignData->upperManager()->cDental.bToothExist[i])
			{
				if (PFusionAlignData->upperManager()->cDental.teeth[i].strFDI.toInt() == 11)
				{
					Axis local_axis = PFusionAlignData->upperManager()->cDental.teeth[i].localAxis;
					Point3m vert = local_axis.centerPoint;
					this->right_face_ = CutFace(vert, nv, axisX, axisY);
					*this->cutface = CutFace(vert, nv, axisX, axisY);
				}
				else if (PFusionAlignData->upperManager()->cDental.teeth[i].strFDI.toInt() == 21)
				{
					Axis local_axis = PFusionAlignData->upperManager()->cDental.teeth[i].localAxis;
					Point3m vert = local_axis.centerPoint;
					this->left_face_ = CutFace(vert, nv, axisX, axisY);
				}
			}
		}
	}

	if (PFusionAlignData->upper_dental_features_)
	{
		// TODO: Redesign initialization plane calculation method, relate to lower jaw tooth positions
		int left_side_fdi_list[7] = { 11,12,13,14,15,16,17 };
		for (int i = 0; i < 7; ++i)
		{
			bool found = false;
			for (auto& tooth : PFusionAlignData->upper_dental_features_->teeth_list_)
			{
				if (tooth.fdi_ == left_side_fdi_list[i])
				{
					Point3m vert;
					if (tooth.axis_ready_)
					{
						Axis local_axis = tooth.axis_;
						vert = local_axis.centerPoint;
					}
					else
					{
						vert = tooth.marks_[2];
					}

					if (_load_record_cut_face && PFusionAlignData->dental_analysis_ && PFusionAlignData->dental_analysis_->right_overlay_cut_face_)
					{
						this->right_face_ = *PFusionAlignData->dental_analysis_->right_overlay_cut_face_;
						*this->cutface = *PFusionAlignData->dental_analysis_->right_overlay_cut_face_;
						emit loadRightMeasureRecordSignal(PFusionAlignData->dental_analysis_->right_overlay_measure_edge_);
					}
					else
					{
						this->right_face_ = CutFace(vert, nv, axisX, axisY);
						*this->cutface = CutFace(vert, nv, axisX, axisY);
						emit loadRightMeasureRecordSignal(FEdge(Point3m(0,0,0), Point3m(0, 0, 0)));
					}

					SAFE_DELETE(PFusionAlignData->dental_analysis_->right_overlay_cut_face_);
					PFusionAlignData->dental_analysis_->right_overlay_cut_face_ = &this->right_face_;
					found = true;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}

		int right_side_fdi_list[7] = { 21,22,23,24,25,26,27 };
		for (int i = 0; i < 7; ++i)
		{
			bool found = false;
			for (auto& tooth : PFusionAlignData->upper_dental_features_->teeth_list_)
			{
				if (tooth.fdi_ == right_side_fdi_list[i])
				{
					Point3m vert;
					if (tooth.axis_ready_)
					{
						Axis local_axis = tooth.axis_;
						vert = local_axis.centerPoint;
					}
					else
					{
						vert = tooth.marks_[2];
					}

					if (_load_record_cut_face && PFusionAlignData->dental_analysis_ && PFusionAlignData->dental_analysis_->left_overlay_cut_face_)
					{
						this->left_face_ = *PFusionAlignData->dental_analysis_->left_overlay_cut_face_;
						emit loadLeftMeasureRecordSignal(PFusionAlignData->dental_analysis_->left_overlay_measure_edge_);
					}
					else
					{
						this->left_face_ = CutFace(vert, nv, axisX, axisY);
						emit loadLeftMeasureRecordSignal(FEdge(Point3m(0, 0, 0), Point3m(0, 0, 0)));
					}

					SAFE_DELETE(PFusionAlignData->dental_analysis_->left_overlay_cut_face_);
					PFusionAlignData->dental_analysis_->left_overlay_cut_face_ = &this->left_face_;
					found = true;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}
	}
	bUpdateCutOutline = true;
	parent->update();
}

int Overlay::getCurrentSegmentIndex()
{
	int result = distanceAccumulate.size() - 1;
	int i = 0;
	for(auto tempDistance : distanceAccumulate)
	{
		if (fSliderDistance < tempDistance && fSliderDistance >= 0)
		{
			result = i;
			break;
		}
		else
			i++;
	}

	return result;
}

void Overlay::updateCurrentBasalPoint()
{
	if (archPoints.empty())
	{
		return;
	}
	Point3m startP, endP;
	startP = this->archPoints[iSegment];
	endP   = this->archPoints[iSegment + 1];
	float minusDistance;
	float t = 0;
	if (this->iSegment >= 1)
	{
		minusDistance = this->fSliderDistance - this->distanceAccumulate[iSegment - 1];
		t = minusDistance / distanceList[iSegment];
	}
	else
		t = fSliderDistance / distanceList[iSegment];

	basalPoint = startP + (endP - startP) * t;
	nv = endP - startP;

	if (PFusionAlignData->upperManager() || PFusionAlignData->lowerManager())
	{
		if (!PFusionAlignData->curManager()->cDental.bUpperDental)
		{
			nv *= -1;
		}
		if (PFusionAlignData->curManager()->cDental.bUpperDental)
		{
			axisY = PFusionAlignData->curManager()->cDental.basePlane.axisZV;
		}
		else
		{
			axisY = -PFusionAlignData->curManager()->cDental.basePlane.axisZV;
		}
	}
	else if (PFusionAlignData->upper_dental_features_ || PFusionAlignData->lower_dental_features_)
	{
		if (PFusionAlignData->upper_dental_features_->b_upper_)
		{
			nv *= -1;
		}
		axisY = PFusionAlignData->upper_dental_features_->base_plane_.axisZV;
	}
	axisX = nv ^ axisY;

	nv = nv.Normalize();
	axisX = axisX.Normalize();
	axisY = axisY.Normalize();
}

void Overlay::drawDentalArch()
{
	for (int i = 0; i < this->archPoints.size() - 1; i++)
	{
		glPushMatrix();
		glDisable(GL_LIGHTING);
		glColor3f(1, 0, 0);
		glLineWidth(5.0f);
		glBegin(GL_LINES);
		glVertex3f(archPoints[i].X(), archPoints[i].Y(), archPoints[i].Z());
		glVertex3f(archPoints[i + 1].X(), archPoints[i + 1].Y(), archPoints[i + 1].Z());
		glEnd();
		glLineWidth(1.0f);
		glColor3f(1, 1, 1);
		glPopMatrix();
	}
}

void Overlay::ensureCutFaceBySilderDistance()
{
	this->iSegment = getCurrentSegmentIndex();
	updateCurrentBasalPoint();
	if (cutface != nullptr)
	{
		SAFE_DELETE(cutface);
	}
	cutface = new CutFace(basalPoint, nv, axisX, axisY);
	copyCurrentStateToCheekSide();
	bUpdateCutOutline = true;
	parent->update();
}

void Overlay::copyCurrentStateToCheekSide()
{
	if (cheek_side_ == OverlayOutlinePreview::RIGHT_SIDE)
	{
		this->right_face_ = *cutface;
	}
	else
	{
		this->left_face_ = *cutface;
	}
}

bool Overlay::computeCrossPoint(Point3m startP, Point3m endP, Point3m cutFaceNormalV, Point3m cutFacePosP, Point3m &CrossPoint)
{
	// 1. Check if two endpoints of line segment are on opposite sides of plane, if yes continue calculation, otherwise return false
	float fPointMulit;
	Point3m sideVector1, sideVector2;
	sideVector1 = startP - cutFacePosP;
	sideVector2 = endP - cutFacePosP;
	if ((sideVector1 * cutFaceNormalV) * (sideVector2 * cutFaceNormalV) > 0) // Two points on the same side of plane
		return false;

	Point3m cs, se;
	cs = startP - cutFacePosP;
	se = endP - startP;
	float base = se *cutFaceNormalV;
	if (base == 0)
		return false;

	float t = -(cs * cutFaceNormalV) / base;
	if (0 <= t && t <= 1)
	{
		CrossPoint = startP + se * t;
		return true;
	}
	return false;
}

void Overlay::computeCrossSegmentByFace(CFaceO *face, Point3m cutFaceNormalV, Point3m cutFacePosP, vcg::Color4b _color, vcg::Matrix44f _matrix, ToothModelType _type)
{
	Point3m p1, p2, p3;
	bool bCrossed1, bCrossed2, bCrossed3;
	Point3m tempStartP, tempEndP;
	tempStartP = _matrix * face->V(0)->P();
	tempEndP = _matrix * face->V(1)->P();
	bCrossed1 = computeCrossPoint(tempStartP, tempEndP, cutFaceNormalV, cutFacePosP, p1);
	tempStartP = _matrix * face->V(1)->P();
	tempEndP = _matrix * face->V(2)->P();
	bCrossed2 = computeCrossPoint(tempStartP, tempEndP, cutFaceNormalV, cutFacePosP, p2);
	tempStartP = _matrix * face->V(2)->P();
	tempEndP = _matrix * face->V(0)->P();
	bCrossed3 = computeCrossPoint(tempStartP, tempEndP, cutFaceNormalV, cutFacePosP, p3);

	VertCP newVertCP;
	if (bCrossed1 && bCrossed2)
	{
		newVertCP.Init(p1, p2, _color, _type);
		cutVertCP.push_back(newVertCP);
		return;
	}
	if (bCrossed2 && bCrossed3)
	{
		newVertCP.Init(p2, p3, _color, _type);
		cutVertCP.push_back(newVertCP);
		return;
	}
	if (bCrossed3 && bCrossed1)
	{
		newVertCP.Init(p3, p1, _color, _type);
		cutVertCP.push_back(newVertCP);
		return;
	}

	return;
}

Point3m Overlay::proj3DPointOnCutFace(Point3m p, Point3m axis_X, Point3m axis_Y)
{
	float x, y;
	x = p * axis_X;
	y = p * axis_Y;
	return Point3m(x, y, 0);
}

void Overlay::getCutOutline()
{
	if (parent == nullptr)
	{
		return;
	}
	Point3m cur_view_direct, cur_camera_pos;
	UtilityTools::getInstance()->getCurrentCameraState(nullptr, parent->width(), parent->height(), cur_view_direct, cur_camera_pos);
	vector<MeshModel*> model_list;
	if (PFusionAlignData->upperManager() || PFusionAlignData->lowerManager())
	{
		model_list = PFusionAlignData->sortMeshModelPointers(cur_camera_pos);
	}
	if (PFusionAlignData->upper_dental_features_ || PFusionAlignData->lower_dental_features_)
	{
		model_list = PFusionAlignData->sortMeshModelPointersSegment(cur_camera_pos);
	}

	foreach(MeshModel * mp, model_list)
	{
		if (mp->visible && !mp->cm.vert.empty())
		{
			vcg::Color4b color = mp->cm.vert[0].C();
			vcg::Matrix44f  matrix = mp->cm.Tr;

			if (!PFusionAlignData->mesh_cloud_Octree_.isEmpty())
			{
				vector<int> possible_faces;
				if (PFusionAlignData->mesh_cloud_Octree_[mp] != nullptr)
				{
					HBall ball(matrix * cutface->axis_.centerPoint, cutface->fRadius_ * 1.2f);
					possible_faces = PFusionAlignData->mesh_cloud_Octree_[mp]->collectIntersectNodes(ball);
				}
				for (auto& i : possible_faces)
				{
					computeCrossSegmentByFace(&mp->cm.face[i], cutface->axis_.axisZVector, cutface->axis_.centerPoint, color, matrix, mp->upperOrLowerToothModelMark);
				}
			}
			else
			{
				for (int i = 0; i < mp->cm.face.size(); ++i)
				{
					computeCrossSegmentByFace(&mp->cm.face[i], cutface->axis_.axisZVector, cutface->axis_.centerPoint, color, matrix, mp->upperOrLowerToothModelMark);
				}
			}
		}
	}

	vector<MeshModel*>().swap(model_list);
}

void Overlay::getProjectCutOutline()
{
	projCutVertCP.clear();

	VertCP newProjVertCP;
	Point3m cur_center = cutface->axis_.centerPoint;
	Point3m axis_x = cutface->axis_.axisXVector.Normalize();
	Point3m axis_y = cutface->axis_.axisYVector.Normalize();
	for (int i = 0; i < cutVertCP.size(); i++)
	{
		Point3m projA, projB;
		projA = proj3DPointOnCutFace(cutVertCP[i].a - cur_center, axis_x, axis_y);
		projB = proj3DPointOnCutFace(cutVertCP[i].b - cur_center, axis_x, axis_y);
		newProjVertCP.Init(projA, projB, cutVertCP[i].color_, cutVertCP[i].type_);
		projCutVertCP.push_back(newProjVertCP);
	}
}

Point3m Overlay::getMixedPointFromScreenToWorld(float mX, float mY, Point3m nV, Point3m hP)
{
	Point3m resultP;
	vcg::Point3d tempProjPoint;
	Point3m nearP, farP;

	tempProjPoint = UtilityTools::getInstance()->projectPoint(Point3m(mX, mY, 0));
	nearP.X() = tempProjPoint.X();
	nearP.Y() = tempProjPoint.Y();
	nearP.Z() = tempProjPoint.Z();
	tempProjPoint = UtilityTools::getInstance()->projectPoint(Point3m(mX, mY, 1));
	farP.X() = tempProjPoint.X();
	farP.Y() = tempProjPoint.Y();
	farP.Z() = tempProjPoint.Z();

	if (computeCrossPoint(nearP, farP, nV, hP, resultP))
	{
		return resultP;
	}
	else
	{
		return hP;
	}
}

vcg::Matrix44f Overlay::computeTransformMatrix(Point3m pos, Point3m axisVector, float angle)
{
	vcg::Matrix44f transMat, transformValue, rotMatLocal;;
	vcg::Matrix44f transform, transformInverse;
	transform.SetIdentity();

	transform.SetTranslate(pos);
	transformInverse.SetTranslate(-pos);

	transMat.SetIdentity();
	rotMatLocal.SetIdentity();
	transformValue.SetIdentity();

	transformValue.SetRotateDeg(angle, axisVector);
	rotMatLocal *= transformValue;

	transform *= rotMatLocal;
	transform *= transformInverse;

	return transform;
}

void Overlay::updateAnaGuiSize()
{
	int screen_width = parent->width();
	int screen_height = parent->height();
	int labels_height = screen_height - getCurrentLabelAreaHeight();
	this->ana_gui_height_ = screen_height * 0.75f;
	if (ana_gui_height_ >= labels_height)
	{
		ana_gui_height_ = labels_height - 1;
	}
	this->ana_gui_width_ = ana_gui_height_ * 0.9f;
}

///Signal & Slots
void Overlay::setNewPointList()
{
	emit updatePointList(projCutVertCP);
}

void Overlay::adjustCutFacePos(CutFace* _face)
{
	if (_face == nullptr)
	{
		return;
	}
	Point3m plane_normal;
	if (PFusionAlignData->upper_dental_features_)
	{
		plane_normal = PFusionAlignData->upper_dental_features_->base_plane_.axisZV;
	}
	else if (PFusionAlignData->upperManager())
	{
		plane_normal = PFusionAlignData->upperManager()->cDental.basePlane.axisZV;
	}

	int index = 0;
	float min_distance = FLT_MAX, cur_distance;
	Point3m vec, node;
	for (int i = 0; i < archPoints.size() - 1; ++i)
	{
		cur_distance = (_face->axis_.centerPoint - archPoints[i]).Norm();
		if (cur_distance < min_distance)
		{
			min_distance = cur_distance;
			index = i;
		}
	}

	float segment_length = (archPoints[index + 1] - archPoints[index]).Norm();
	vec = (archPoints[index + 1] - archPoints[index]).Normalize();
	cur_distance = (_face->axis_.centerPoint - archPoints[index]) * vec;
	if (cur_distance < 0)
	{
		cur_distance = 0;
	}
	if (cur_distance > segment_length)
	{
		cur_distance = segment_length;
	}
	node = archPoints[index] + vec * cur_distance;

	nv = vec;
	if (PFusionAlignData->upper_dental_features_ || PFusionAlignData->lower_dental_features_)
	{
		if (PFusionAlignData->upper_dental_features_->b_upper_)
		{
			nv *= -1;
		}
		axisY = PFusionAlignData->upper_dental_features_->base_plane_.axisZV;
	}
	axisX = nv ^ axisY;

	nv = nv.Normalize();
	axisX = axisX.Normalize();
	axisY = axisY.Normalize();

	_face->initFace(node, nv, axisX, axisY);
}

void Overlay::updateAccordingSlider(float fSlider)
{
	if (fSlider <= 0)
	{
		fSlider = 0.001f;
	}
	else if (fSlider >= 1)
	{
		fSlider = 0.999f;
	}
	fSliderDistance = fSlider * fSliderDistanceSum;

	ensureCutFaceBySilderDistance();
	parent->update();
}

void Overlay::updateCurrentSectionOutline()
{
	bUpdateCutOutline = true;
	parent->update();
}

void Overlay::updateOverlayStateSlot(float _overjet_value, QString _overjet_rank, Point3m _overjet_color, float _overbite_value, QString _overbite_rank, Point3m _overbite_color)
{
	show_overjet_value_ = _overjet_value;
	show_overjet_rank_ = _overjet_rank;
	show_overjet_value_color_ = QColor(_overjet_color.X(), _overjet_color.Y(), _overjet_color.Z());

	show_overbite_value_ = _overbite_value;
	show_overbite_rank_ = _overbite_rank;
	show_overbite_value_color_ = QColor(_overbite_color.X(), _overbite_color.Y(), _overbite_color.Z());

	if (PFusionAlignData->analysisData() != nullptr)
	{
		PFusionAlignData->analysisData()->setOverjetValue(_overjet_value);
		PFusionAlignData->analysisData()->setOverjetRank(_overjet_rank);
		PFusionAlignData->analysisData()->setOverbiteValue(_overbite_value);
		PFusionAlignData->analysisData()->setOverbiteRank(_overbite_rank);
	}
	updateLabels();
	emit PSIGNALMANAGER->updateDentalAnalysisDataReportSignal();
	parent->update();
}

void Overlay::updateLeftAndRightParametersSlot(float _left_overbite, float _left_overjet, float _right_overbite, float _right_overjet)
{
	show_left_overbite_ = _left_overbite;
	show_left_overjet_ = _left_overjet;
	show_right_overbite_ = _right_overbite;
	show_right_overjet_ = _right_overjet;
}

void Overlay::showOverlayGifSlot()
{
	if (gif_overlay_ui && gif_overlay_ui->isVisible())
	{
		gif_overlay_ui->hide();
	}
	else if(gif_overlay_ui && !gif_overlay_ui->isVisible())
	{
		gif_overlay_ui->show();
	}
}

float Overlay::getCurrentLabelAreaHeight()
{
	if (frames_.empty())
	{
		return 0;
	}
	int interval_height = label_row_height_ * 10;
	/*for (auto& frame : frames_)
	{
		interval_height += frame.rect_.height();
	}*/
	return interval_height;
}

void Overlay::updateLabels()
{
	if (!parent)
	{
		return;
	}
	vector<Interactive2DLabel>().swap(labels_);
	vector<Interactive2DFrame>().swap(frames_);

	QFont temp_font("Yu Gothic Ui Semibold", Interactive2DLabel::font_size_, QFont::Bold, false);
	temp_font.setLetterSpacing(QFont::AbsoluteSpacing, 1);
	QFontMetrics fm(temp_font);
	QRect rec = fm.boundingRect(QString(tr("Posterior segment : ")));
	int append_width = rec.width() + 10;

	int row_height = rec.height() * 1.1f;
	label_row_height_ = row_height;
	int begin_x = 10, begin_y = parent->height() - row_height * 2;
	int begin_x_append = begin_x + append_width;

	{
		vector<Interactive2DLabel> labels;
		labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString::fromLocal8Bit("Ⅲ ").append(QString(tr("Deep Overjet     "))).append(QString(tr("overjet "))).append("> 8mm"), Point2i(begin_x, begin_y), false));
		begin_y -= row_height;
		labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString::fromLocal8Bit("Ⅱ ").append(QString(tr("Deep Overjet     "))).append(QString("5mm < ")).append(QString(tr("overjet "))).append(QString::fromLocal8Bit("≤")).append(" 8mm"), Point2i(begin_x, begin_y), false));
		begin_y -= row_height;
		labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString::fromLocal8Bit("Ⅰ").append(QString(tr("Deep Overjet     "))).append(QString("3mm < ")).append(QString(tr("overjet "))).append(QString::fromLocal8Bit("≤")).append(" 5mm"), Point2i(begin_x, begin_y), false));
		begin_y -= row_height;
		labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Reference : ")), Point2i(begin_x, begin_y), false));

		frames_.push_back(Interactive2DFrame(Interactive2DLabel::DEFAULT_VIEW, labels, false, true));
		begin_y -= row_height * 2;
	}

	{
		vector<Interactive2DLabel> labels;
		if (show_overjet_value_ >= 0)
		{
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Overjet : ")).append(QString::number(show_overjet_value_)).append("mm (").append(tr(show_overjet_rank_.toStdString().c_str())).append(")"), Point2i(begin_x, begin_y), false));
		}
		else
		{
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Overjet : ")).append("/"), Point2i(begin_x, begin_y), false));
		}
		labels.back().setTextColor(show_overjet_value_color_);
		begin_y -= row_height;
		if (show_overbite_value_ >= 0)
		{
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Overbite : ")).append(QString::number(show_overbite_value_)).append("mm (").append(tr(show_overbite_rank_.toStdString().c_str())).append(")"), Point2i(begin_x, begin_y), false));
		}
		else
		{
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Overbite : ")).append("/"), Point2i(begin_x, begin_y), false));
		}
		labels.back().setTextColor(show_overbite_value_color_);

		frames_.push_back(Interactive2DFrame(Interactive2DLabel::DEFAULT_VIEW, labels, false, false));
	}
}
