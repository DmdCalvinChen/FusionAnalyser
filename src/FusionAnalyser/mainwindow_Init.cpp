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

#include "../common/interfaces.h"
#include "../common/xmlfilterinfo.h"
#include "../common/searcher.h"
#include "../common/mlapplication.h"

#include <QToolBar>
#include <QProgressBar>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFileOpenEvent>
#include <QFile>
#include <QtXml>
#include <QSysInfo>
#include <QDesktopServices>
#include <QStatusBar>
#include <QMenuBar>
#include <QWidgetAction>
#include <QDesktopWidget>
#include <QScreen>
#include <QProgressDialog>

#include "mainwindow.h"
#include "plugindialog.h"
#include "saveSnapshotDialog.h"
#include "ui_congratsDialog.h"
#include "../common/config.h"

#include "UI_Common/framelessWindow/fusionTitlebar.h"
#include <UI_Common/ImportModelGui/importmodelgui.h>
#include "UI_Common/qFusionProgressBar/QFusionProgressDlg.h"
#include "common_ext/util/utility_tools.h"


#include "common/version.h"
using namespace ui_common;

#ifdef Q_OS_WIN
#include <windowsx.h>
#include <windows.h>
#include <Uxtheme.h>
#include <dwmapi.h>
#pragma comment (lib,"Dwmapi.lib") // Add shadow
#pragma comment (lib,"user32.lib")
#endif

using namespace ui_common;
const int PROGRESSBARHEIGHT = 2;
QProgressBar* MainWindow::qb;

void MainWindow::httpTest()
{
	return;
}
MainWindow::MainWindow()
	:mwsettings(), gpumeminfo(NULL), wama()
{
#ifdef Q_OS_WIN
    HWND hwnd = (HWND)this->winId();
    DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
    ::SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
    const MARGINS shadow = { 1, 1, 1, 1 }; // Reserve border for shadow
    DwmExtendFrameIntoClientArea(HWND(winId()), &shadow);
#endif
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    this->setObjectName("MainWindow");
    this->setMinimumSize(QSize(1280, 768));
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    p_fusion_title_bar_ = new FusionTitleBar(this);
    installEventFilter(p_fusion_title_bar_);
    p_fusion_title_bar_->setWindowTitle(":/default/res/default/images/titlebar/title.svg");
    p_main_widget_ = new QWidget(this);
    p_sub_central_widget_ = new QWidget(this);
    p_sub_main_window_ = new QMainWindow(this);
    p_sub_main_window_->setObjectName("sub_main_window");
	screen = QGuiApplication::primaryScreen();
	screen_width = screen->availableGeometry().width();
	screen_height = screen->availableGeometry().height();

	auto dpi = qApp->primaryScreen()->logicalDotsPerInch() * 1.0f / 96;
	if (dpi > 1.0f)
	{
		scaleResolutionDpi /= dpi;
	}
	else
	{
		scaleResolutionDpi *= dpi;
	}

	fontTitle.setPixelSize(14 * scaleResolutionDpi);
	fontTitle.setFamily("Microsoft YaHei");
	fontTitle.setStyleStrategy(QFont::PreferQuality);

	sizeImageEditBtn = QSize(43 * scaleResolutionDpi, 43 * scaleResolutionDpi);

	// default style qss
	setContextMenuPolicy(Qt::NoContextMenu);

	// Allow nested docks
	setDockNestingEnabled(true);

	connect(PSIGNALMANAGER, &SignalManager::updateTreamtmentStepGUISignal, this, &MainWindow::updateTreamtmentStepGUISlot);

	connect(PSIGNALMANAGER, &SignalManager::updateProgressGuiSignal, this, &MainWindow::updateProgressGuiSlot);
	connect(PSIGNALMANAGER, &SignalManager::showBusyProgressBarSignal, this, &MainWindow::onShowBusyProgressSlot);
	connect(PSIGNALMANAGER, &SignalManager::updateDentalAnalysisDataSignal, this, &MainWindow::updateDentalAnalysisDataSlot);
	httpReq = new QNetworkAccessManager(this);
	connect(httpReq, SIGNAL(finished(QNetworkReply*)), this, SLOT(connectionDone(QNetworkReply*)));

	this->setWindowIcon(QIcon(":/default/res/default/images/fusionAnalyser.ico"));

	PM.loadPlugins(defaultGlobalParams); // Load plugins
	QSettings settings;
	QVariant vers = settings.value(MeshLabApplication::versionRegisterKeyName());
	//should update those values only after I run MeshLab for the very first time or after I installed a new version
	if (!vers.isValid() || vers.toString() < MeshLabApplication::appVer())
	{
		settings.setValue(MeshLabApplication::pluginsPathRegisterKeyName(), PluginManager::getDefaultPluginDirPath());
		settings.setValue(MeshLabApplication::versionRegisterKeyName(), MeshLabApplication::appVer());
		settings.setValue(MeshLabApplication::wordSizeKeyName(), QSysInfo::WordSize);
		foreach(QString plfile, PM.pluginsLoaded)
			settings.setValue(PluginManager::osIndependentPluginName(plfile), MeshLabApplication::appVer());
	}

	init();

}

MainWindow::~MainWindow()
{
	SAFE_DELETE(gpumeminfo);
	SAFE_DELETE(fileImportDlg);
    SAFE_DELETE(projectParser);
	SPDLOG->flush();
}

void MainWindow::initLayout()
{
    p_main_layout_ = new QVBoxLayout;
    p_main_layout_->setMargin(0);
    p_main_layout_->addWidget(p_fusion_title_bar_);
	p_main_layout_->setContentsMargins(0, 0, 0, 0);
    QFrame *line = new QFrame(this);
    line->setFixedHeight(1);
    line->setObjectName("mainindow_separator");
    p_main_layout_->addWidget(line);
    p_main_layout_->addWidget(p_align_toolbar_);

    QVBoxLayout *vSubLayout = new QVBoxLayout;
    vSubLayout->setMargin(0);
    vSubLayout->addWidget(_currviewcontainer);
	vSubLayout->addWidget(p_home_gui_);
    p_sub_central_widget_->setLayout(vSubLayout);
    p_sub_main_window_->setCentralWidget(p_sub_central_widget_);
    p_sub_main_window_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    p_main_layout_->addWidget(p_sub_main_window_);
    p_main_layout_->setSpacing(0);
	p_main_layout_->addWidget(qb);
	qb->setTextVisible(false);
	qb->setFixedHeight(PROGRESSBARHEIGHT);
    p_main_widget_->setLayout(p_main_layout_);
    this->setCentralWidget(p_main_widget_);

}
void MainWindow::createActions()
{
	//////////////Action Menu File ////////////////////////////////////////////////////////////////////////////
	p_open_stl_act_ = new QAction(QIcon(":/dark/res/dark/images/menu/openFile.svg"), tr("Open STL File"), this);
	p_open_stl_act_->setShortcutContext(Qt::ApplicationShortcut);
	p_open_stl_act_->setShortcut(Qt::CTRL + Qt::Key_N);
	p_open_stl_act_->setFont(fontTitle);
	connect(p_open_stl_act_, &QAction::triggered, [&]() {if (whetherSaveProject() >= 0) open_stl_slot(); });

	//openProjectAct = new QAction(QIcon(":/dark/res/dark/images/menu/open.png"), tr("&Open project..."), this);
	openProjectAct = new QAction(QIcon(":/dark/res/dark/images/menu/openRecord.svg"), tr("&Open Record"), this);
	openProjectAct->setShortcutContext(Qt::ApplicationShortcut);
	openProjectAct->setShortcut(Qt::CTRL + Qt::Key_O);
	openProjectAct->setFont(fontTitle);
	connect(openProjectAct, &QAction::triggered, [&]() {if (whetherSaveProject() >= 0) openProject(); });

	saveProjectAct = new QAction(QIcon(":/dark/res/dark/images/menu/saveRecord.svg"), tr("&Save Record"), this);
	saveProjectAct->setShortcutContext(Qt::ApplicationShortcut);
	saveProjectAct->setShortcut(Qt::CTRL + Qt::Key_S);
	saveProjectAct->setFont(fontTitle);
	connect(saveProjectAct, SIGNAL(triggered()), this, SLOT(saveProject()));

	saveProjectAsAct = new QAction(QIcon(":/default/res/default/images/saveprojectas.png"), tr("&Save Project As"), this);
	saveProjectAsAct->setFont(fontTitle);
	connect(saveProjectAsAct, SIGNAL(triggered()), this, SLOT(saveProjectAs()));

	closeProjectAct = new QAction(QIcon(":/dark/res/dark/images/menu/close.svg"), tr("Close Project"), this);
	closeProjectAct->setFont(fontTitle);
	connect(closeProjectAct, SIGNAL(triggered()), this, SLOT(closeProject()));

	//unDoAct = new QAction(QIcon(":images/undo.png"), tr("&Undo"), this);
	unDoAct = new QAction(tr("undo"), this);
	QIcon undoIcon;
	undoIcon.addPixmap(QPixmap(":/default/res/default/images/undo.svg"), QIcon::Normal);
	undoIcon.addPixmap(QPixmap(":/default/res/default/images/undo_selected.svg"), QIcon::Active,QIcon::On);

	unDoAct->setIcon(undoIcon);

	unDoAct->setShortcutContext(Qt::ApplicationShortcut);
	unDoAct->setShortcut(Qt::CTRL + Qt::Key_Z);
	//unDoAct->setFont(fontTitle);
	connect(unDoAct, SIGNAL(triggered()), this, SLOT(unDo()));
	//connect(unDoAct, SIGNAL(hovered()), [=]{undoIcon.Active}

	reDoAct = new QAction(tr("redo"), this);
	reDoAct->setIcon(QIcon(":/default/res/default/images/redo.svg"));
	//reDoAct = new QAction(QIcon(":images/redo.png"), tr("&Redo"), this);
	reDoAct->setShortcutContext(Qt::ApplicationShortcut);
	reDoAct->setShortcut(Qt::CTRL + Qt::Key_Y);
	//reDoAct->setFont(fontTitle);
	connect(reDoAct, SIGNAL(triggered()), this, SLOT(reDo()));

	exportMeshAct = new QAction(QIcon(":/default/res/default/images/savemesh.png"), tr("&Export Mesh..."), this);
	exportMeshAct->setFont(fontTitle);
	connect(exportMeshAct, SIGNAL(triggered()), this, SLOT(save()));

	exportMeshAsAct = new QAction(QIcon(":/default/res/default/images/savemeshas.png"), tr("&Export Mesh As..."), this);
	exportMeshAsAct->setFont(fontTitle);
	connect(exportMeshAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

	for (int i = 0; i < MAXRECENTFILES; ++i)
	{
		recentFileActs[i] = new QAction(this);
		recentFileActs[i]->setVisible(true);
		recentFileActs[i]->setEnabled(true);
		recentFileActs[i]->setShortcutContext(Qt::ApplicationShortcut);
		connect(recentFileActs[i], SIGNAL(triggered()), this, SLOT(openRecentMesh()));
	}

	exitAct = new QAction(QIcon(":/dark/res/dark/images/menu/close.svg"),tr("E&xit"), this);
	exitAct->setShortcutContext(Qt::ApplicationShortcut);
	exitAct->setShortcut(Qt::CTRL + Qt::Key_Q);
	//exitAct->setFont(fontTitle);
	connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

	//////////////Render Actions for Toolbar and Menu /////////////////////////////////////////////////////////
	showTrackBallAct = new QAction(tr("Show &Trackball"), this);
	showTrackBallAct->setCheckable(true);
	showTrackBallAct->setShortcut(Qt::SHIFT + Qt::Key_H);
	connect(showTrackBallAct, SIGNAL(triggered()), this, SLOT(showTrackBall()));

	linkViewersAct = new QAction(tr("Link Viewers"), this);
	linkViewersAct->setCheckable(true);
	connect(linkViewersAct, SIGNAL(triggered()), this, SLOT(linkViewers()));

	//////////////Action Menu About ///////////////////////////////////////////////////////////////////////////
	aboutAct = new QAction(QIcon(":/dark/res/dark/images/menu/about.svg"),tr("&About"), this);
	//aboutAct->setFont(fontTitle);
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

	p_new_align_proj_act_ = new QAction(QIcon(":/default/res/default/images/toolbar/newAlign.svg"), tr("new project"), this);
    p_save_align_proj_act_ = new QAction(QIcon(":/default/res/default/images/toolbar/saveAlign.svg"), tr("save"), this);
    p_save_as_align_proj_act_ = new QAction(QIcon(":/default/res/default/images/toolbar/saveAlignAs.svg"), tr("saveAs"), this);

    connect(p_new_align_proj_act_, &QAction::triggered, this, &MainWindow::onNewAlignProjectSlot);
    connect(p_save_align_proj_act_, &QAction::triggered, this, &MainWindow::onSaveAlignProjectSlot);
    connect(p_save_as_align_proj_act_, &QAction::triggered, this, &MainWindow::onSaveAlignProjectAsSlot);

    // Change password functionality removed - login feature removed

	p_set_work_dir_act_ = new QAction(QIcon(":/dark/res/dark/images/menu/path.png"), tr("setting dir"), this);
	connect(p_set_work_dir_act_, &QAction::triggered, this, &MainWindow::onSetWorkPathSlot);

	p_change_language_act_ = new QAction(QIcon(":/dark/res/dark/images/menu/path.png"), tr("Languages"), this);
	connect(p_change_language_act_, &QAction::triggered, this, &MainWindow::onShowChangeLanguageDlgSlot);

	if (p_change_language_group_ == nullptr)
		p_change_language_group_ = new QActionGroup(this);
	//p_change_ch_language_act_ = new QAction(QIcon(":/dark/res/dark/images/menu/path.png"), tr("change ch language"), p_change_language_group_);
	p_change_ch_language_act_ = new QAction( tr("Chinese"), p_change_language_group_);
	connect(p_change_ch_language_act_, &QAction::triggered, []() { emit PSIGNALMANAGER->changeLanguageSignal(0); });
	p_change_ch_language_act_->setData(E_CHINESE);
	p_change_ch_language_act_->setCheckable(true);
	p_change_en_language_act_ = new QAction( tr("English"), p_change_language_group_);
	//p_change_en_language_act_ = new QAction(QIcon(":/dark/res/dark/images/menu/path.png"), tr("change language"), this);
	connect(p_change_en_language_act_, &QAction::triggered, []() { emit PSIGNALMANAGER->changeLanguageSignal(1); });
	p_change_en_language_act_->setData(E_ENGLISH);
	p_change_en_language_act_->setCheckable(true);

	if (PFusionAppData->getAppSettingLanguage() == E_CHINESE)
	{
		p_change_ch_language_act_->setChecked(true);
	}
	else
	{
		p_change_en_language_act_->setChecked(true);
	}

	if (p_bolton_group_ == nullptr)
		p_bolton_group_ = new QActionGroup(this);
	p_bolton_six_act_ = new QAction( "5-5", p_bolton_group_);
	connect(p_bolton_six_act_, &QAction::triggered, [=]() { this->setCrowdingAnalysisModeSlot(true); });
	p_bolton_six_act_->setCheckable(true);
	p_bolton_seven_act_ = new QAction( "7-7", p_bolton_group_);
	connect(p_bolton_seven_act_, &QAction::triggered, [=]() { this->setCrowdingAnalysisModeSlot(false); });
	p_bolton_seven_act_->setCheckable(true);
	connect(PSIGNALMANAGER, &SignalManager::setSixToSixModeCheckedStateSignal, this, &MainWindow::setSixToSixModeCheckedState);

	QSettings settings;
	eBoltonComputeType bolton;
	if (settings.value(BoltonComputeKey).isValid())
	{
		bolton = (eBoltonComputeType)settings.value(BoltonComputeKey).toInt();
	}
	else
	{
		bolton = E_BOLOTON_SIX;
	}

	if (bolton == E_BOLOTON_SIX)
	{
		this->setCrowdingAnalysisModeSlot(true);
		p_bolton_six_act_->setChecked(true);
	}
	else if (bolton == E_BOLOTON_SEVEN)
	{
		this->setCrowdingAnalysisModeSlot(false);
		p_bolton_seven_act_->setChecked(true);
	}

	// Report preview action removed from FusionAnalyser
}

void MainWindow::setSixToSixModeCheckedState(bool _six_to_six, bool _modifyed)
{
	if (_six_to_six)
	{
		if (p_bolton_six_act_)
		{
			p_bolton_six_act_->setChecked(true);

			if (_modifyed)
			{
				setCrowdingAnalysisModeSlot(true);
			}
			else
			{
				PFusionAlignData->setBoltonMode(E_BOLOTON_SIX);
			}
		}
	}
	else
	{
		if (p_bolton_seven_act_)
		{
			p_bolton_seven_act_->setChecked(true);

			if (_modifyed)
			{
				setCrowdingAnalysisModeSlot(false);
			}
			else
			{
				PFusionAlignData->setBoltonMode(E_BOLOTON_SEVEN);
			}
		}
	}
}

void MainWindow::createToolBars()
{
	p_align_toolbar_ = new FusionAlignToolBar(this);
	connect(p_align_toolbar_, &FusionAlignToolBar::openHistoryProjsSignal, this, &MainWindow::onOpenHistorySchemesSlot);
	connect(p_align_toolbar_, &FusionAlignToolBar::saveSubmitProjSignal, this, &MainWindow::onSaveSubmitAlignProjectSlot);

	QAction *enamelReductionAction = nullptr;
    QAction *pShowAccessoryAct = nullptr;

    foreach(MeshDecorateInterface * iDecorate, PM.meshDecoratePlugins())
    {
        foreach(QAction * decorateAction, iDecorate->actions())
        {
            if (decorateAction && (decorateAction->data().toUInt()))
            {
				if (enamelReductionAction == nullptr && decorateAction->objectName() == QString("show enamel reduction"))
				{
					enamelReductionAction = decorateAction;
				}
				else if (decorateAction->objectName() == SHOW_FDI_ACT_NAME)
				{
					p_show_fdi_act_ = decorateAction;
					continue;
				}
				else if (decorateAction->objectName() == SHOW_OCCLUSION_ACT_NAME)
				{
					p_show_occlusion_act_ = decorateAction;
					continue;
				}

                if (p_align_toolbar_)
                {
                    p_align_toolbar_->getDecorateToolBar()->addAction(decorateAction);
                }
            }
        }
    }

	if (p_align_toolbar_)
	{
		p_align_toolbar_->getEditDecorateToolBar()->addAction(p_show_fdi_act_);
		//p_align_toolbar_->getEditDecorateToolBar()->addAction(p_show_fdi_act_);
		p_align_toolbar_->getEditDecorateToolBar()->addAction(p_show_occlusion_act_);
	}

	foreach(MeshEditInterfaceFactory* plugin, PM.meshEditFactoryPlugins())
	{
		foreach(QAction * pAct, plugin->actions())
		{
			if (pAct->data().toUInt())
			{
				if (pAct->objectName() == /*EDIT_SEGMENT_ACTION_NAME*/EDIT_MODEL_MARKING_ACTION_NAME)
				{
					p_tooth_seg_act_ = pAct;
					continue;
				}
				else if (pAct->objectName() == EDIT_TOOTH_WIDTH_ANA_ACT_NAME)
				{
					p_tooth_width_act_ = pAct;
				}
				if (p_align_toolbar_)
				{
					p_align_toolbar_->getAnalysisToolBar()->addAction(pAct);
				}
			}
		}
	}

	// Report preview action removed from FusionAnalyser
	if (p_align_toolbar_)
	{
		//p_align_toolbar_->installEventFilter(this);
		if (p_tooth_seg_act_)p_tooth_seg_act_->setIcon(QIcon(":/dark/res/dark/images/cut_normal.svg"));
		p_align_toolbar_->getSegmentBtn()->setDefaultAction(p_tooth_seg_act_);
	}
	p_align_toolbar_->setEnabled(false);
	onChangeProjStageSlot(E_ProjStageStart);

    return;
}

void MainWindow::createMenus()
{
	//////////////////// Menu File ////////////////////////////////////////////////////////////////////////////
    fileMenu = p_fusion_title_bar_->getMainMenu();
//#ifndef NETWORK_DESIGN
	fileMenu->addAction(p_open_stl_act_);
	fileMenu->addAction(openProjectAct);
	fileMenu->addAction(saveProjectAct);
	//separatorAct = fileMenu->addSeparator();
	recentProjMenu = fileMenu->addMenu(QIcon(":/dark/res/dark/images/menu/recentRecords.svg"),tr("Recent Record"));
	updateRecentProjActions();

//#endif

	fillEditMenu();
	fillDecorateMenu();

 //   fileMenu->addAction(p_changePwd_act_);
	p_language_menu_ = fileMenu->addMenu(QIcon(":/dark/res/dark/images/menu/language.svg"), tr("Languages"));
	p_language_menu_->setObjectName("language_menu");
	p_language_menu_->addAction(p_change_ch_language_act_);
	p_language_menu_->addAction(p_change_en_language_act_);

	p_bolton_change_menu_ = fileMenu->addMenu(QIcon(":/dark/res/dark/images/menu/crowding.svg"),tr("Teeth Range of Crowding"));
	p_bolton_change_menu_->addAction(p_bolton_six_act_);
	p_bolton_change_menu_->addAction(p_bolton_seven_act_);
	p_bolton_change_menu_->setObjectName("bolton_menu");

	//fileMenu->addAction(p_change_language_act_);
	fileMenu->addAction(aboutAct);
	fileMenu->addAction(exitAct);

    return;

}

QString MainWindow::getDecoratedFileName(const QString& name)
{
	return  QString("<br><b><i>(") + name + ")</i></b>";
}

void MainWindow::fillDecorateMenu()
{
	foreach(MeshDecorateInterface * iDecorate, PM.meshDecoratePlugins())
	{
		foreach(QAction * decorateAction, iDecorate->actions())
		{
			connect(decorateAction, SIGNAL(triggered()), this, SLOT(applyDecorateMode()));
			decorateAction->setToolTip(iDecorate->decorationInfo(decorateAction));
		}
	}
}

void MainWindow::fillRenderMenu()
{
	QAction* qaNone = new QAction("None", this);
	qaNone->setCheckable(false);
	shadersMenu->addAction(qaNone);
	connect(qaNone, SIGNAL(triggered()), this, SLOT(applyRenderMode()));
	foreach(MeshRenderInterface * iRender, PM.meshRenderPlugins())
	{
		addToMenu(iRender->actions(), shadersMenu, SLOT(applyRenderMode()));
	}
}

void MainWindow::setPluginExitEnable(bool _value)
{
	foreach(MeshEditInterfaceFactory * iEditFactory, PM.meshEditFactoryPlugins())
	{
		foreach(QAction * a, iEditFactory->actions())
		{
			a->setEnabled(_value);
		}
	}

}

void MainWindow::fillEditMenu()
{

    foreach(MeshEditInterfaceFactory * iEditFactory, PM.meshEditFactoryPlugins())
    {
        foreach(QAction * editAction, iEditFactory->actions())
        {
            connect(editAction, SIGNAL(triggered()), this, SLOT(applyEditMode()));
        }

    }

}

void MainWindow::loadMeshLabSettings()
{
	// I have already loaded the plugins so the default parameters for the settings
	// of the plugins are already in the <defaultGlobalParams> .
	// we just miss the globals default of meshlab itself
	MainWindowSetting::initGlobalParameterSet(&defaultGlobalParams);
	GLArea::initGlobalParameterSet(&defaultGlobalParams);

	QSettings settings;
	QStringList klist = settings.allKeys();

	// 1) load saved values into the <currentGlobalParams>
	for (int ii = 0; ii < klist.size(); ++ii)
	{
		QDomDocument doc;
		doc.setContent(settings.value(klist.at(ii)).toString());

		QString st = settings.value(klist.at(ii)).toString();
		QDomElement docElem = doc.firstChild().toElement();

		RichParameter* rpar = NULL;
		if (!docElem.isNull())
		{
			bool ret = RichParameterFactory::create(docElem, &rpar);
			if (!ret)
			{
				continue;
			}
			if (!defaultGlobalParams.hasParameter(rpar->name))
			{
			}
			else
				currentGlobalParams.addParam(rpar);
		}
	}

	// 2) eventually fill missing values with the hardwired defaults
	for (int ii = 0; ii < defaultGlobalParams.paramList.size(); ++ii)
	{
		//		qDebug("Searching param[%i] %s of the default into the loaded settings. ",ii,qPrintable(defaultGlobalParams.paramList.at(ii)->name));
		if (!currentGlobalParams.hasParameter(defaultGlobalParams.paramList.at(ii)->name))
		{
			qDebug("Warning! a default param was not found in the saved settings. This should happen only on the first run...");
			RichParameterCopyConstructor v;
			defaultGlobalParams.paramList.at(ii)->accept(v);
			currentGlobalParams.paramList.push_back(v.lastCreated);

			QDomDocument doc("MeshLabSettings");
			RichParameterXMLVisitor vxml(doc);
			v.lastCreated->accept(vxml);
			doc.appendChild(vxml.parElem);
			QString docstring = doc.toString();
			QSettings setting;
			setting.setValue(v.lastCreated->name, QVariant(docstring));
		}
	}
}

void MainWindow::addToMenu(QList<QAction*> actionList, QMenu* menu, const char* slot)
{
	foreach(QAction * a, actionList)
	{
		connect(a, SIGNAL(triggered()), this, slot);
		menu->addAction(a);
	}
}

// this function update the app settings with the current recent file list
// and update the loaded mesh counter
void MainWindow::saveRecentFileList(const QString& fileName)
{
	QSettings settings;
	QStringList files = settings.value("recentFileList").toStringList();
	files.removeAll(fileName);
	files.prepend(fileName);
	while (files.size() > MAXRECENTFILES)
		files.removeLast();

	//avoid the slash/back-slash path ambiguity
	for (int ii = 0; ii < files.size(); ++ii)
		files[ii] = QDir::fromNativeSeparators(files[ii]);
	settings.setValue("recentFileList", files);

	foreach(QWidget * widget, QApplication::topLevelWidgets()) {
		MainWindow* mainWin = qobject_cast<MainWindow*>(widget);
		if (mainWin) mainWin->updateRecentFileActions();
	}
}

void MainWindow::sendUsAMail()
{
}

void MainWindow::saveRecentProjectList(const QString& projName)
{
	QString path = QDir::fromNativeSeparators(projName);

	QSettings settings;
	QStringList files = settings.value("recentProjList").toStringList();
	files.removeAll(path);
	files.prepend(path);
	while (files.size() > MAXRECENTFILES)
		files.removeLast();

	for (int ii = 0; ii < files.size(); ++ii)
		files[ii] = QDir::fromNativeSeparators(files[ii]);

	settings.setValue("recentProjList", files);

// 	foreach(QWidget * widget, QApplication::topLevelWidgets())
	updateRecentProjActions();
}

void MainWindow::connectionDone(QNetworkReply* reply)
{
}

void MainWindow::wrapSetActiveSubWindow(QWidget* window) {
}

void MainWindowSetting::initGlobalParameterSet(RichParameterSet* glbset)
{
	glbset->addParam(
		new RichInt(
			maximumDedicatedGPUMem(),
			350, "Maximum GPU Memory Dedicated to MeshLab (Mb)",
			"Maximum GPU Memory Dedicated to MeshLab (megabyte) for the storing of the geometry attributes. The dedicated memory must NOT be all the GPU memory presents on the videocard."));
	glbset->addParam(
		new RichInt(
			perBatchPrimitives(),
			1000000,
			"Per batch primitives loaded in GPU",
			"Per batch primitives (vertices and faces) loaded in the GPU memory. It's used in order to do not overwhelm the system memory with an entire temporary copy of a mesh."));
	glbset->addParam(
		new RichInt(
			minPolygonNumberPerSmoothRendering(),
			1000000,
			"Default Face number per smooth rendering",
			"Minimum number of faces in order to automatically render a newly created mesh layer with the per vertex normal attribute activated."));

	glbset->addParam(
		new RichBool(
			perMeshRenderingToolBar(),
			true,
			"Show Per-Mesh Rendering Side ToolBar",
			"If true the per-mesh rendering side toolbar will be redendered inside the layerdialog."));

	//WARNING!!!! REMOVE THIS LINE AS SOON AS POSSIBLE! A plugin global variable has been introduced by MeshLab Core!
	glbset->addParam(
		new RichString(
			"MeshLab::Plugins::sketchFabKeyCode",
			"0000000", "SketchFab KeyCode", ""));
	/****************************************************************************************************************/

	if (MeshLabScalarTest<Scalarm>::doublePrecision())
		glbset->addParam(
			new RichBool(
				highPrecisionRendering(),
				false,
				"High Precision Rendering",
				"If true all the models in the scene will be rendered at the center of the world"));
	glbset->addParam(
		new RichInt(
			maxTextureMemoryParam(),
			256,
			"Max Texture Memory (in MB)",
			"The maximum quantity of texture memory allowed to load mesh textures"));
}

void MainWindowSetting::updateGlobalParameterSet(RichParameterSet& rps)
{
	maxgpumem = (std::ptrdiff_t)rps.getInt(maximumDedicatedGPUMem()) * (float)(1024 * 1024);
	perbatchprimitives = (size_t)rps.getInt(perBatchPrimitives());
	minpolygonpersmoothrendering = (size_t)rps.getInt(minPolygonNumberPerSmoothRendering());
	permeshtoolbar = rps.getBool(perMeshRenderingToolBar());
	highprecision = false;
	if (MeshLabScalarTest<Scalarm>::doublePrecision())
		highprecision = rps.getBool(highPrecisionRendering());
	maxTextureMemory = (std::ptrdiff_t) rps.getInt(this->maxTextureMemoryParam()) * (float)(1024 * 1024);
}

void MainWindow::defaultPerViewRenderingData(MLRenderingData& dt) const
{
	MLRenderingData::RendAtts tmpatts;
	tmpatts[MLRenderingData::ATT_NAMES::ATT_VERTPOSITION] = true;
	tmpatts[MLRenderingData::ATT_NAMES::ATT_VERTNORMAL] = true;
	tmpatts[MLRenderingData::ATT_NAMES::ATT_VERTCOLOR] = true;
	tmpatts[MLRenderingData::ATT_NAMES::ATT_WEDGETEXTURE] = true;
	dt.set(MLRenderingData::PR_SOLID, tmpatts);
	MLPerViewGLOptions opts;
	dt.set(opts);
}

void MainWindow::setUpperToothOperator()
{
	// Select upper jaw as current operating jaw slot
	if (!GLA())
	{
		return;
	}
	if (meshDoc()->meshList.empty())
	{
		return;
	}

	PFusionAlignData->setUpperManagerSelected(true);
}

void MainWindow::setLowerToothOperator()
{
	// Select lower jaw as current operating jaw slot
	if (!GLA())
	{
		return;
	}
	if (meshDoc()->meshList.empty())
	{
		return;
	}

	PFusionAlignData->setUpperManagerSelected(false);
	if (PFusionAlignData->curManager() != nullptr)
	{
		emit PSIGNALMANAGER->setTreatmentTabSelectItem(PFusionAlignData->curManager()->cDental.currentSelectedStage + 1, 0);
	}
}

void MainWindow::tabUpperOrLowerToothShowStatus()
{
	if (!GLA() || meshDoc()->meshList.empty())
	{
		return;
	}
	if (PFusionAlignData->dental_summary_ < 2)
	{
		return;
	}

	if (PFusionAlignData->bUpperDentalSelected)
	{
		emit PSIGNALMANAGER->setLowerDentalSelectedSignal();
	}
	else
	{
		emit PSIGNALMANAGER->setUpperDentalSelectedSignal();
	}
}

// Show dock widgets with specified indices
void MainWindow::setDockwidgetsWantToShow(QList<QDockWidget*>& _dockWidgets, const QList<int>& index /* = QList<int>() */)
{
	foreach(int i, index)
	{
		_dockWidgets[i]->hide();
	}
	foreach(int i, index)
	{
		if (i < _dockWidgets.size())
		{
			_dockWidgets[i]->show();
		}
	}
}

void MainWindow::setRightDockWhichWidgetToShowSlot(int indexWidget)
{
	// Treatment preview removed from FusionAnalyser
	setDockwidgetsWantToShow(rightDockWidgets, QList<int>() << indexWidget);
	update();
}

using namespace  fusionAlign;
// Treatment preview initialization removed from FusionAnalyser
// Report preview GUI removed from FusionAnalyser

void MainWindow::initVersionInfo()
{
	QCoreApplication::setApplicationVersion(QString(FUSIONMAINVERSION) + QString::number(FUSIONVERSION));
}
void MainWindow::init()
{
	initVersionInfo();
	// HTTP functionality removed - FusionAnalyser does not depend on network
    loadMeshLabSettings();//----------------------------------->加载Meshlab设置
    mwsettings.updateGlobalParameterSet(currentGlobalParams);
    createActions();//----------------------------------------->创建行为动作
    createToolBars();//---------------------------------------->创建工具栏
	createMenus();//------------------------------------------->创建菜单
	createProgressGui();
    gpumeminfo = new vcg::QtThreadSafeMemoryInfo(mwsettings.maxgpumem);

    setAcceptDrops(true);
    //setWindowTitle(MeshLabApplication::shortName());
    //setStatusBar(new QStatusBar(this));
    globalStatusBar() = statusBar();
	globalStatusBar()->setObjectName("fusionStatusBar");
	//globalStatusBar()->setStyleSheet("color:black;");
    qb = new QProgressBar(this);
    qb->setObjectName("fusionDesignProcessBar");
    qb->setMaximum(100);
    qb->setMinimum(0);
    qb->reset();
    //statusBar()->addPermanentWidget(qb, 0);
    qb->hide();

    updateCustomSettings();

    fileImportDlg = new FileImportDialog(this);
    createEmptyProject();

	// Create home page
	p_home_gui_ = new ui_common::FusionAnalyserHomeGui(this);
	connect(p_home_gui_, &ui_common::FusionAnalyserHomeGui::importModelSignal, this, &MainWindow::loadMeshMoelsSlot);
	connect(p_home_gui_, &ui_common::FusionAnalyserHomeGui::openRecentlyProject,this, &MainWindow::openProject);

    //p_fusion_title_bar_->addCategoryToolBar(p_category_toolbar_);
    initLayout();

	setHomePageVisible(false);
    if (this->projectParser == nullptr)
    {
        this->projectParser = new ProjectParser(GLA()->getSceneGLSharedContext(), GLA());
    }

    connect(PSIGNALMANAGER, &SignalManager::saveProjectSignal, this, static_cast<void (MainWindow::*)()>(&MainWindow::saveProject));
    connect(PSIGNALMANAGER, &SignalManager::saveWhatProjectSignal, this, &MainWindow::autoSaveProject);
	connect(PSIGNALMANAGER, &SignalManager::setProgressBarSignal, this, &MainWindow::updateProgressBar);
	connect(PSIGNALMANAGER, &SignalManager::setProgressBarSignal_async, this, &MainWindow::updateProgressBar, Qt::QueuedConnection);
	connect(PSIGNALMANAGER, &SignalManager::updateProgressGuiSignal_async, this, &MainWindow::updateProgressGuiSlot, Qt::QueuedConnection);
    connect(PSIGNALMANAGER, &SignalManager::endEditSignal, this, &MainWindow::endEdit);
    connect(PSIGNALMANAGER, &SignalManager::doToothAdjustPluginOnceSignal, this, &MainWindow::doToothAdjustPluginOnceSlot);
    connect(PSIGNALMANAGER, &SignalManager::canUndoOrRedoStatusSignal, this, &MainWindow::canUndoOrRedoStatusCommand);
    connect(PSIGNALMANAGER, &SignalManager::parseAlnProjFinishedSignal, this, &MainWindow::onParseAlnProjFinishedSlot);
	connect(PSIGNALMANAGER, &SignalManager::setWhetherThePluginCanExit, this, &MainWindow::setPluginExitEnable);
	connect(PSIGNALMANAGER, &SignalManager::clickAccessoryVisibleBtnSignal, this, &MainWindow::clickAccessoryVisibleBtnSlot);
	connect(PSIGNALMANAGER, &SignalManager::getAccessoryVisibleBtnCheckStateSignal, this, &MainWindow::getAccessoryVisibleBtnCheckStateSlot);
    connect(PSIGNALMANAGER, &SignalManager::setAccessoryVisibleActionSignal, this, &MainWindow::clickAccessoryVisibleBtnSlot);
    connect(PSIGNALMANAGER, &SignalManager::showBusyProgressBarSignal, this, &MainWindow::onShowBusyProgressSlot);
	connect(PSIGNALMANAGER, &SignalManager::changeLanguageSignal, this, &MainWindow::onChangeLanguageSlot);
	connect(PSIGNALMANAGER, &SignalManager::updatePluginUISignal, this, &MainWindow::onNeedUpdatePluginSlot);

	QString path = qApp->applicationDirPath();
	PFusionAlignData->getToothFeatureConfig().readConfig(path + "/configs/toothMark.json");
}

void MainWindow::createProgressGui()
{
    if (p_progress_dlg_ == nullptr)
        p_progress_dlg_ = new ui_common::QFusionProgressDlg(this);
	// HTTP functionality removed - FusionAnalyser does not depend on network
	// #ifdef NETWORK_DESIGN
	// #endif
}

void MainWindow::clickAccessoryVisibleBtnSlot()
{
    if(p_show_accessory_toolbtn_)
    {
        if(!p_show_accessory_toolbtn_->defaultAction()->isChecked())
        {
            p_show_accessory_toolbtn_->defaultAction()->setChecked(true);
            p_show_accessory_toolbtn_->defaultAction()->triggered(true);
        }
    }
}

void MainWindow::getAccessoryVisibleBtnCheckStateSlot(bool &state)
{
	if (p_show_accessory_toolbtn_)
	{
		state = p_show_accessory_toolbtn_->defaultAction()->isChecked();
	}
}
