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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//None of this should happen if we are compiling c, not c++
#ifdef __cplusplus

#include <GL/glew.h>

#include "../common/pluginmanager.h"
#include "../common/scriptinterface.h"
#include "common_base/logsingleton.h"
#include "common_ext/projectParser/ProjectParser.h"
#include "common_ext/projectParser/fusionProjectParser.h"
#include "common_ext/data/fusionaligndata.h"
#include "common_ext/util/VectorAssist.h"
#include "common_ext/util/utility_tools.h"
#include "common_ext/meshExt/dentalfeatures.h"
#include "common_ext/pointcloud/cloudkdtree.h"
#include "common_ext/data/fusionDataCommon.h"

#include <wrap/qt/qt_thread_safe_memory_info.h>

#include "glarea.h"
#include "layerDialog.h"
#include "ml_std_par_dialog.h"
#include "xmlstdpardialog.h"
#include "xmlgeneratorgui.h"
#include "multiViewer_Container.h"
#include "ml_render_gui.h"
#include "fileimportdlg.h"
#include "colortable.h"
#include "patientinfo.h"

#include <UI_Common/guicontrolcustomdesign/collectfolded.h>
#include "UI_Common/FusionAnalyserHomeGui/fusionanalyserhomegui.h"

#include "toolbar/fusionAlignToolBar.h"

#include <QtScript>
#include <QDir>
#include <QMainWindow>
#include <QMdiArea>
#include <QStringList>
#include <QColorDialog>
#include <QMdiSubWindow>
#include <QSplitter>
#include <QTimer>
#include <QScreen>
#define MAXRECENTFILES 5

#define AUTOSEGMENT
class QAction;
class QActionGroup;
class QMenu;
class QScrollArea;
class QSignalMapper;
class QProgressDialog;
class QNetworkAccessManager;
class QNetworkReply;
class QToolBar;

namespace ui_common
{
	class CollectFoldedGui;
    class FusionTitleBar;
    class QFusionProgressDlg;
	class FusionAnalyserHomeGui;
}

namespace fusionAlign
{
	struct DataContentColumnFormat;
	struct ToothMoveTable;
}

struct sFusionBSHStageFileData;
class WorkDirSetingDlg;
class FusionChangeLanguageDlg;

class MainWindowSetting
{	
public:
	static void initGlobalParameterSet(RichParameterSet* gblset);
	void updateGlobalParameterSet(RichParameterSet& rps);

	std::ptrdiff_t maxgpumem;
	inline static QString maximumDedicatedGPUMem()
	{
		return "Fusion::System::maxGPUMemDedicatedToGeometry";
	}

	bool permeshtoolbar;
	inline static QString perMeshRenderingToolBar() { return "Fusion::GUI::perMeshToolBar"; }

	bool highprecision;
	inline static QString highPrecisionRendering() { return "Fusion::System::highPrecisionRendering"; }

	size_t perbatchprimitives;
	inline static QString perBatchPrimitives() { return "Fusion::System::perBatchPrimitives"; }

	size_t minpolygonpersmoothrendering;
	inline static QString minPolygonNumberPerSmoothRendering()
	{
		return "Fusion::System::minPolygonNumberPerSmoothRendering";
	}

	std::ptrdiff_t maxTextureMemory;
	inline static QString maxTextureMemoryParam() { return "Fusion::System::maxTextureMemory"; }
};

class MainWindow : public QMainWindow, public MainWindowInterface
{
	Q_OBJECT

public:

	MainWindow();
	~MainWindow();
	static bool QCallBack(int pos, const char* str);
	MainWindowSetting mwsettings;
	LogSingleton* logInstance = LogSingleton::getInstance();

	QFont fontTitle;
	float scaleResolutionDpi = 1.0f;
public:
	void setSplit();
	void onChangeAnaStage(eProjAnalyserStage stage);
	bool b_compare_view_split_ = false;
	int compare_view_id_;
	QScreen* screen = nullptr;
	int screen_width;
	int screen_height;
signals:
	void setSwitchToMainMenu(bool);
	void setParallelSwitched(bool);

	void setUpperToothShowStatus(int status);
	void setLowerToothShowStatus(int status);

	void toothAdjustTableShowStateSend(int status);
	void exportMeshModelTreatmentSignal(const QString& filePath);
	void clearUI();

signals:
	void dispatchCustomSettings(RichParameterSet& rps);
	void filterExecuted();
	void updateLayerTable();
	void skipupSignal();

protected:
	void showEvent(QShowEvent* event);
    void init();
	/*
	*	创建进度条对话框
	*/
	void createProgressGui();

private slots:
	void createEmptyProject(const QString& projName = QString());
	void newProject(const QString& projName = QString());
	void saveProjectAs();
	void closeProject();
    void saveProject(QString fileName, bool bAlignProject = true);
	void autoSaveProject(bool bAlignProject = true);
	void canUndoOrRedoStatusCommand(bool bCanUndo, bool bCanRedo);
	void unDo();
	void reDo();
	void meshAdded(int mid);
	void meshRemoved(int mid);

public slots:
	// Quick open file from command line
	void quickOpenFile(QString file);
	void onSaveSubmitAlignProjectSlot();
    void onNewAlignProjectSlot();
    void onSaveAlignProjectSlot();
    void onSaveAlignProjectAsSlot();
    void onOpenHistorySchemesSlot();
    void onOpenHistoryAlignProjSlot(QString projName);
    void onRenameHistroyAlignProjSlot(QString projName);
    void onDeleteHistoryAlignProjSlot(QString projName);
	void isMaxillaSlot(bool isOk);
	bool clearOldProject();// 清理旧的工程模型等内存信息
	void FixedPointAnalysisSlot();
	void startAnalysisSlot();
	void importJawSlot(); //导入对颌
	void loadJawMoelsSlot(QString& models_file);  //加载对颌模型
	void SkipUpperJaw();  // 跳过上颌标记
	void PreviousStepSlot(); //上一步
	void forwardStepSlot();
	void nextStepSlot();
	bool loadMeshModel(const QString& fileName);
	void loadMeshMoelsSlot(QStringList& models_file);
	// Verify if model is upper jaw
	void tellMeThisModelIsUpper();
	// Create marking project
	void createMarkingProject();
	// Create project name
	QString createProjectName(QFileInfoList &fileInfoList, QString patient_name, int number);
	// Create structure record data
	void createRecordData(bool need_empty_first = true);
	// Classify models after importing upper/lower jaw models
	void classifyModels();
	// Toggle upper/lower jaw marker properties (when both upper and lower models exist)
	void toggleUpperLowerMesh();
	// Remove currently selected mesh
	void removeCurrentMesh(int id_mesh);

	bool importMeshWithLayerManagement(QString fileName = QString());
	void saveProject();
	bool openProject(QString fileName = QString());
    bool openZip(QString fileName);
    bool openFas(QString fileName, bool submit = true);
	void doToothAdjustPluginOnceSlot();
	void updateCustomSettings();
	bool addRenderingDataIfNewlyGeneratedMesh(int meshid);
	bool readFeatureSignData(QString _fileName);
	bool parseBoundaryPtsFromIndex();

	void updateRenderingDataAccordingToActions(int meshid, const QList<MLRenderingAction*>& acts);
	void updateRenderingDataAccordingToActionsToAllVisibleLayers(const QList<MLRenderingAction*>& acts);
	void updateRenderingDataAccordingToActions(int meshid, MLRenderingAction* act,
		QList<MLRenderingAction*>& acts);
	void updateRenderingDataAccordingToAction(int meshid, MLRenderingAction* act);
	void updateRenderingDataAccordingToActionToAllVisibleLayers(MLRenderingAction* act);
	void updateRenderingDataAccordingToAction(int meshid, MLRenderingAction* act, bool);
	void updateRenderingDataAccordingToActions(QList<MLRenderingGlobalAction*> actlist);

    
	void onCreateProjectSlot(bool, bool);
	
	void updateTreatmentInfoSlot();
	void initialDentalAnalysisDataFeatureMode(bool _read_mlp_record);
	void updateDentalAnalysisDataSlot(bool _read_mlp_record);
	void setCrowdingAnalysisModeSlot(bool _six_to_six_mode);

	void open_stl_slot();

private:
	void updateRenderingDataAccordingToActionsCommonCode(
		int meshid, const QList<MLRenderingAction*>& acts);
	void updateRenderingDataAccordingToActionCommonCode(
		int meshid, MLRenderingAction* act);

private slots:
	void documentUpdateRequested();
	bool importMesh(QString fileName = QString(), bool isareload = false);
	void endEdit();
	void updateProgressBar(const int pos, const QString& text);
	void updateTexture(int meshid);

public:
	bool exportMesh(QString fileName, MeshModel* mod, const bool saveAllPossibleAttributes);
	bool loadMesh(const QString& fileName, MeshIOInterface* pCurrentIOPlugin,
		MeshModel* mm, int& mask, RichParameterSet* prePar,
		const Matrix44m& mtr = Matrix44m::Identity(), bool isareload = false);
	void computeRenderingDataOnLoading(MeshModel* mm, bool isareload);
	bool loadMeshWithStandardParams(QString& fullPath, MeshModel* mm,
		const Matrix44m& mtr = Matrix44m::Identity(), bool isareload = false);
	void defaultPerViewRenderingData(MLRenderingData& dt) const;
	void getRenderingData(int mid, MLRenderingData& dt) const;
	void setRenderingData(int mid, const MLRenderingData& dt);
	unsigned int viewsRequiringRenderingActions(int meshid, MLRenderingAction* act);

private slots:
	void closeCurrentDocument();
	//////////// Slot Menu File //////////////////////p
	void openRecentMesh();
	void openRecentProj();
	bool saveAs(QString fileName = QString(), const bool saveAllPossibleAttributes = false);
	bool save(const bool saveAllPossibleAttributes = false);
	void changeFileExtension(const QString&);
	///////////Slot Menu Edit ////////////////////////
	void applyEditToothAdjust(QAction* action);
	void applyEditMode();
	void suspendEditMode();
	void showTooltip(QAction*);
	void applyRenderMode();
	void applyDecorateMode();
	void switchOffDecorator(QAction*);
	///////////Slot Menu View ////////////////////////
	void fullScreen();
	void showToolbarFile();
	void showInfoPane();
	void showTrackBall();
	///////////Slot Menu Windows /////////////////////
	void updateMenus();
	void updateMenuItems(QMenu* menu, const bool enabled);
	void enableDocumentSensibleActionsContainer(const bool enable);
	void linkViewers();
	void viewFrom(QAction* qa);
	void toggleOrtho();
	void trackballStep(QAction* qa);

	///////////Slot Menu Help ////////////////////////
	void about();
	void sendUsAMail();
	void dropEvent(QDropEvent* event);
	void dragEnterEvent(QDragEnterEvent*);
	void connectionDone(QNetworkReply* reply);
	//void sendHistory();
	///////////Solt Wrapper for QMdiArea //////////////////
	void wrapSetActiveSubWindow(QWidget* window);
	void switchCurrentContainer(QMdiSubWindow*);
	void scriptCodeExecuted(const QScriptValue& val, const int time, const QString& output);

public slots:
	void updateTreamtmentStepGUISlot();
	void onParseAlnProjFinishedSlot();

	//void setColorTableVisible(bool);
	void updateLog();

	// enable upper or lower tooth mesh current operator slots
	void setUpperToothOperator();
	void setLowerToothOperator();

	void tabUpperOrLowerToothShowStatus();
	void setPluginExitEnable(bool _value);
	void clickAccessoryVisibleBtnSlot();
	void getAccessoryVisibleBtnCheckStateSlot(bool &state);
	void getPatientFiles();
	void onNeedUpdatePluginSlot();
	void setSixToSixModeCheckedState(bool _six_to_six, bool _modifyed);

private:
	void initGlobalParameters();
	void createActions();
	void createMenus();
	void fillDecorateMenu();
	void fillRenderMenu();
	void fillEditMenu();
	void createToolBars();
	void loadMeshLabSettings();
	void keyPressEvent(QKeyEvent*);
	void keyReleaseEvent(QKeyEvent*);
	void updateRecentFileActions();
	void updateRecentProjActions();
	void saveRecentFileList(const QString& fileName);
	void saveRecentProjectList(const QString& projName);
	void addToMenu(QList<QAction*>, QMenu* menu, const char* slot);
	void setCurrentMeshBestTab();
	void clearDecoratorState();

	QNetworkAccessManager* httpReq;

	int idHost;
	int idGet;
	bool VerboseCheckingFlag;

	static QProgressBar* qb;

private:
	QList<QDockWidget*> rightDockWidgets;///< 记录所有dockWidget的指针
	// Show dock windows
	void setDockwidgetsWantToShow(QList<QDockWidget*>& _dockWidgets, const QList<int>& index = QList<int>());

public slots:
	// Set right dock widget to show
	void setRightDockWhichWidgetToShowSlot(int indexWidget);

private:
	FileImportDialog* fileImportDlg = nullptr;
	bool bWhetherShowToothPreview = true;

	QSignalMapper* windowMapper;
	vcg::QtThreadSafeMemoryInfo* gpumeminfo = nullptr;

	RichParameterSet currentGlobalParams;
	RichParameterSet defaultGlobalParams;

	QByteArray toolbarState;								//stato delle toolbar e dockwidgets

	QDir lastUsedDirectory;  //This will hold the last directory that was used to load/save a file/project in

public:
	PluginManager PM;

	MeshDocument* meshDoc() {
		if (currentViewContainer() != NULL)
			return &currentViewContainer()->meshDoc;
		return NULL;
	}

	inline vcg::QtThreadSafeMemoryInfo* memoryInfoManager() const { return gpumeminfo; }
	const RichParameterSet& currentGlobalPars() const { return currentGlobalParams; }
	RichParameterSet& currentGlobalPars() { return currentGlobalParams; }
	const RichParameterSet& defaultGlobalPars() const { return defaultGlobalParams; }

	GLArea* GLA() const {
		static bool callOnce = true;
		//	  if(mdiarea->currentSubWindow()==0) return 0;
		MultiViewer_Container* mvc = currentViewContainer();

		if (!mvc) return 0;
       // return  mvc->currentgla;
		if (callOnce)
		{
            callOnce = false;
		}
		return  mvc->main_gla_;
	}

	MultiViewer_Container* currentViewContainer() const {
		return _currviewcontainer;
	}

	const PluginManager& pluginManager() const { return PM; }

	static QStatusBar*& globalStatusBar()
	{
		static QStatusBar* _qsb = 0;
		return _qsb;
	}

private:
	WordActionsMapAccessor wama;
	//////// ToolBars ///////////////
	QToolBar* mainToolBar;
	QToolBar* projectSaveToolBar;
	QToolBar* decorateToolBar;
	QToolBar* undoRedoToolBar;
	QToolBar* tabUpperLowerToolBar;
	QToolBar* tabViewDirectionToolBar;
	QToolBar* analyzeToolBar;
	QToolBar* createModelToolBar;
	QToolBar* treatmentModelToolBar;

	MLRenderingGlobalToolbar* globrendtoolbar;
	///////// Menus ///////////////
	QMenu* fileMenu;
	QMenu* recentProjMenu;
	QMenu* recentFileMenu;
	QMenu* editMenu;
	QMenu* p_language_menu_ = nullptr;
	QMenu* p_bolton_change_menu_ = nullptr;

	//Render Menu and SubMenu ////
	QMenu* shadersMenu;
	QMenu* renderMenu;

	//View Menu and SubMenu //////
	QMenu* viewMenu;
	QMenu* toolBarMenu;
	//////////////////////////////
	QMenu* windowsMenu;
	QMenu* preferencesMenu;
	QMenu* helpMenu;
	QMenu* splitModeMenu;
	QMenu* viewFromMenu;
	QMenu* trackballStepMenu;
	//////////// Split/Unsplit Menu from handle///////////
	QMenu* handleMenu;
	QMenu* splitMenu;
	QMenu* unSplitMenu;
	SearchMenu* searchMenu;

	// FusionAnalyser menu
	QAction* p_open_stl_act_ = nullptr;
	QAction* p_open_proj_act_ = nullptr;
	QAction* p_save_proj_act_ = nullptr;
	//QAction* p_recent_projs_act = nullptr;

	//////////// Actions Menu File ///////////////////////
	QAction* newProjectAct = nullptr;
	QAction* openProjectAct, * saveProjectAct, * saveProjectAsAct;
	QAction* exportMeshAct, * exportMeshAsAct;
	QAction* unDoAct, * reDoAct;
	QWidget* p_seg_undo_btn_ = nullptr, *p_seg_redo_btn_ = nullptr;
	QWidget* p_align_undo_btn_ = nullptr, *p_align_redo_btn_ = nullptr;
	QAction* closeProjectAct;
	QTimer* autoSaveProjectTimer;
	QAction* recentFileActs[MAXRECENTFILES] = { 0 };
	//QAction* recentProjActs[MAXRECENTFILES] = { 0 };
	QAction* separatorAct;
	QAction* exitAct;
	QAction* showToolbarStandardAct;
	QAction* showTrackBallAct;
	///////////Actions Menu Windows /////////////////////
	QAction* windowsTileAct;
	QAction* windowsCascadeAct;
	QAction* windowsNextAct;
	QAction* closeAllAct;

    QAction *p_new_align_proj_act_ = nullptr, *p_save_align_proj_act_ = nullptr, *p_save_as_align_proj_act_ = nullptr;
	QAction* p_tooth_seg_act_ = nullptr;
	QAction* p_tooth_width_act_ = nullptr;
	QAction* p_change_language_act_ = nullptr;
	QActionGroup* p_change_language_group_ = nullptr;
	QAction* p_change_ch_language_act_ = nullptr;
	QAction* p_change_en_language_act_ = nullptr;
	QActionGroup* p_bolton_group_ = nullptr;
	QAction* p_bolton_six_act_ = nullptr;
	QAction* p_bolton_seven_act_ = nullptr;

public:
	QAction* linkViewersAct;
	ProjectParser* projectParser = nullptr;

private:
	///////////Actions Menu Help ////////////////////////
	QAction* aboutAct;
	QAction* aboutPluginsAct;
	QAction* submitBugAct;
	////////////////////////////////////////////////////
	static QString getDecoratedFileName(const QString& name);

	MultiViewer_Container* _currviewcontainer = nullptr;

public:
	QSize sizeImageEditBtn;

public:
	// Treatment preview and analysis group removed from FusionAnalyser

	// ToothAdjustPlayAnimationGui removed - files deleted

    private:
        void initLayout();
		int whetherSaveProject();

    public slots:
        void onCategoryToolBarToggledSlot(bool toogled);
        void onChangeProjStageSlot(eProjStage stage);

		void onUploadProgress(qint64 sended, qint64 total);
		void onDownloadingProgress(qint64 sended, qint64 total);
        void onShowBusyProgressSlot(bool isBusy = true);
		void updateProgressGuiSlot(const QString& text, int value, int max);
		void onNetworkFinished(QNetworkReply*);

		void onSetWorkPathSlot(bool triggered);
		void onShowChangeLanguageDlgSlot(bool triggerd);
		void onChangeLanguageSlot(int index);

		// Set home page visibility
		void setHomePageVisible(bool visible);
		void onGetStageListSlot();
		void openRecentStageProjSlot();

		bool checkProjectExist();
		void httpTest();

		void readAutoFeatureResult(QString fileName);
    protected:
        virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result);
		virtual void closeEvent(QCloseEvent* event);
		virtual void resizeEvent(QResizeEvent* event);
		virtual void paintEvent(QPaintEvent* event);
		void initVersionInfo();
#ifdef Q_OS_WIN
        LRESULT onTestBorder(const QPoint &pt);
#endif

    private:
            void updateGUI();

        // Toolbar
    private:
        QToolBar *p_category_toolbar_ = nullptr;               //标题栏

        FusionAlignToolBar   *p_align_toolbar_ = nullptr;
		QToolButton          *p_show_accessory_toolbtn_ = nullptr;
        QWidget              *p_edit_widget_ = nullptr;

        QVector<QToolButton*> v_decorate_btns_;
        QVector<QToolButton*> v_decorate_temp_btns_;

		// Home page for importing models and opening recent projects
		ui_common::FusionAnalyserHomeGui* p_home_gui_ = nullptr;

        // Layout related
        ui_common::FusionTitleBar *p_fusion_title_bar_ = nullptr;
        QWidget* p_main_widget_ = nullptr;
        QWidget* p_sub_central_widget_ = nullptr;
        QMainWindow *p_sub_main_window_ = nullptr;
        QDialog* p_cur_edit_toolbar_ = nullptr;
        QAction * p_cur_category_act_ = nullptr;

        QAction *p_seg_category_act_ = nullptr;
        QAction *p_align_category_act_ = nullptr;
        QAction *p_export_category_act_ = nullptr;

        QVBoxLayout *p_main_layout_ = nullptr;

		QAction *p_help_act_ = nullptr;
		QAction *p_about_act_ = nullptr;
		QTabWidget  *p_category_widget_ = nullptr;

        ui_common::QFusionProgressDlg *p_progress_dlg_ = nullptr;

		FusionChangeLanguageDlg* p_change_language_dlg_ = nullptr;

		QMargins frames_;
		bool b_maximized_ = false;
		bool b_need_update_ui_ = false;
		QMargins  margins_;

		QAction* p_auto_seg_act_ = nullptr;
		QAction* p_reseg_act_ = nullptr;
		QAction* p_set_work_dir_act_ = nullptr;
		QAction* p_show_gingiva_act_ = nullptr;
		QAction* p_show_occlusion_act_ = nullptr;
		QAction* p_show_fdi_act_ = nullptr;
		QActionGroup* p_compare_act_group_ = nullptr;
		QAction* p_cur_compare_act_ = nullptr;
		QTranslator       translator_;

		WorkDirSetingDlg* p_work_path_dlg_ = nullptr;

		bool b_seg_proj_file_exist_ = false;
		bool b_align_proj_file_exist_ = false;

		public:
			bool eventFilter(QObject* watched, QEvent* event) override;
};

/// Event filter that is installed to intercept the open events sent directly by the Operative System
class FileOpenEater : public QObject
{
	Q_OBJECT

public:
	FileOpenEater(MainWindow* _mainWindow)
	{
		mainWindow = _mainWindow;
		noEvent = true;
	}

	MainWindow* mainWindow;
	bool noEvent;

protected:

	bool eventFilter(QObject* obj, QEvent* event)
	{
		if (event->type() == QEvent::FileOpen) {
			noEvent = false;
			QFileOpenEvent* fileEvent = static_cast<QFileOpenEvent*>(event);
			mainWindow->importMeshWithLayerManagement(fileEvent->file());
			qDebug("event fileopen %s", qPrintable(fileEvent->file()));
			return true;
		}
		else {
			// standard event processing
			return QObject::eventFilter(obj, event);
		}
	}
};

#endif
#endif
