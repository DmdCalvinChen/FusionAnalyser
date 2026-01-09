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

#ifndef GLAREA_H
#define GLAREA_H
#include <GL/glew.h>

#include <vcg/math/matrix44.h>
#include <wrap/gl/math.h>
#include <wrap/gui/trackball.h>
#include <vcg/math/shot.h>
#include <wrap/gl/shot.h>
#include <wrap/gl/trimesh.h>
#include <wrap/qt/gl_label.h>

#include <QTimer>
#include <vector>
#include <common/interfaces.h>

#include "glarea_setting.h"
#include "snapshotsetting.h"
#include "multiViewer_Container.h"
#include "common/ml_selection_buffers.h"
#include "ml_default_decorators.h"
#include "patientinfo.h"
#include "common_base/SignalManager.h"
#include "common_ext/data/fusionaligndata.h"
#include <UI_Common/navAnabutton/navanabutton.h>
#include <UI_Common//FusionAnalyserHomeGui/fusionanalyserbuttongui/tipswidget.h>
#include "common_ext/util/assist_geometry.h"

class DentalManager;

using namespace std;
enum ViewDirection
{
	DEFAULT_VIEW,
	TOP_VIEW,
	BOTTOM_VIEW,
	FRONT_VIEW,
	BACK_VIEW,
	LEFT_VIEW,
	RIGHT_VIEW,
	UPPER_VISIBLE_SWITCH,
	LOWER_VISIBLE_SWITCH
};

enum LightingModel { LDOUBLE, LFANCY };

class MeshModel;
class MainWindow;
class MeshShaderRender;
struct MLPerViewGLOptions;

namespace ui_common
{
	class TextureMap;
	class BaseAttributeInfoButton;
	class CustomButtonDesignBase;
	class CollectFoldedGui;
	class fusionViewGui;
}

using namespace vcg;

class GLArea : public QGLWidget
{
	Q_OBJECT

		//typedef vcg::Shot<double> Shot;

public:
	GLArea(QWidget* parent, MultiViewer_Container* mvcont, RichParameterSet* current);
	~GLArea();
	static void initGlobalParameterSet(RichParameterSet* /*globalparam*/);
private:
	int id;  //the very important unique id of each subwindow.
	MultiViewer_Container* parentmultiview;

public:
	vcg::GlTrimesh<CMeshO> glWrap, glWrapLower;
	int getId() { return id; }

	MultiViewer_Container* mvc()
	{
		return parentmultiview;
	}

	void updateSelection(int meshid, bool vertsel, bool facesel)
	{
		makeCurrent();
		if (md() != NULL)
		{
			MeshModel* mm = md()->getMesh(meshid);
			if (mm != NULL)
			{
				CMeshO::PerMeshAttributeHandle< MLSelectionBuffers* > selbufhand =
					vcg::tri::Allocator<CMeshO>::GetPerMeshAttribute<MLSelectionBuffers* >
					(mm->cm, MLDefaultMeshDecorators::selectionAttName());
				if ((selbufhand() != NULL) && (facesel))
					selbufhand()->updateBuffer(MLSelectionBuffers::ML_PERFACE_SEL);

				if ((selbufhand() != NULL) && (vertsel))
					selbufhand()->updateBuffer(MLSelectionBuffers::ML_PERVERT_SEL);
			}
		}
	}

	void updateAllSiblingsGLAreas()
	{
		if (mvc() == NULL)
			return;
		foreach(GLArea * viewer, mvc()->viewerList)
		{
			if (viewer != NULL)
				viewer->update();
		}
	}

	void requestForRenderingAttsUpdate(int meshid, MLRenderingData::ATT_NAMES attname)
	{
		if (parentmultiview != NULL)
		{
			MLSceneGLSharedDataContext* cont = parentmultiview->sharedDataContext();
			if (cont != NULL)
			{
				MLRenderingData::RendAtts atts;
				atts[attname] = true;
				cont->meshAttributesUpdated(meshid, false, atts);
				cont->manageBuffers(meshid);
			}
		}
	}

	MainWindow* mw();

	MeshModel* mm() { if (mvc() == NULL) return NULL; return mvc()->meshDoc.mm(); }
	inline MeshDocument* md() {
		if (mvc() == NULL) return NULL; return &(mvc()->meshDoc);
	}

	vcg::Trackball trackball;
	vcg::Trackball trackball_light;

	bool bWhetherOpenGlobalTrackball;
	void Logf(int Level, const char* f, ...);

	GLAreaSetting glas;
	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	QAction* getLastAppliedFilter() { return lastFilterRef; }
	void		setLastAppliedFilter(QAction* qa) { lastFilterRef = qa; }

	void updateFps(float deltaTime);

	bool isCurrent() {
		if (mvc() == NULL) return false; return mvc()->currentId == this->id;
	}

	void showTrackBall(bool b) { trackBallVisible = b; update(); }
	bool isHelpVisible() { return helpVisible; }
	bool isTrackBallVisible() { return trackBallVisible; }
	bool isDefaultTrackBall() { return activeDefaultTrackball; }
	void toggleHelpVisible();

	bool isMeshInfomationVisible()
	{
		return bShowMeshInfoVisible;
	}

	void toggleMeshInfomationVisible()
	{
		bShowMeshInfoVisible = !bShowMeshInfoVisible;
		update();
	}

	void setView();
	int renderForSelection(int pickX, int pickY);

	bool is_right_mouse_pressed_ = true;
	Color4b color_origion_mesh_;
	void selectMeshOperate();
	void recoverSelectedMeshColor();

	// Stores for each mesh what are the per Mesh active decorations
	QMap<int, QList<QAction*> > iPerMeshDecoratorsListMap;

	QList<QAction*> iPerDocDecoratorlist;
	QList<QAction*>& iCurPerMeshDecoratorList() {
		assert(this->md()->mm());
		return iPerMeshDecoratorsListMap[this->md()->mm()->id()];
	}

	void setRenderer(MeshRenderInterface* rend, QAction* shader) {
		iRenderer = rend; currentShader = shader;
	}
	MeshRenderInterface* getRenderer() { return iRenderer; }
	QAction* getCurrentShaderAction() { return currentShader; }

public slots:
	void setupTextureEnv(GLuint textid);
	void resetTrackBall();

	//current raster will be reloaded and repaint function will be called
	void completeUpdateRequested();

	void setDecorator(QString name, bool state);
	void toggleDecorator(QString name);

	void updateDecorator(QString name, bool toggle, bool stateToSet);

	void updateCustomSettingValues(RichParameterSet& rps);

	void endEdit()
	{
		if (iEdit && currentEditor)
		{
			if (md() != NULL)
				iEdit->EndEdit(*md(), this, parentmultiview->sharedDataContext());

			if (mm() != NULL)
				iEdit->EndEdit(*mm(), this, parentmultiview->sharedDataContext());

		}
		iEdit = 0;
		currentEditor = 0;
		setCursorTrack(0);
		update();
		emit updateMainWindowMenus();
		if (!Fixed_point_analysis->isVisible() && this->iPerDocDecoratorlist.empty())
		{
			if(PFusionAlignData->getAnalyserData().cur_proj_stage_ != E_ProjAnaStageNone)
				Fixed_point_analysis->show();
		}
	}

	void suspendEditToggle()
	{
		if (currentEditor == 0)
			return;
		static QCursor qc;
		if (suspendedEditor) {
			suspendedEditor = false;
			setCursor(qc);
		}
		else {
			suspendedEditor = true;
			qc = cursor();
			setCursorTrack(0);
		}
	}

signals:
	void updateMainWindowMenus(); //updates the menus of the meshlab MainWindow
	void glareaClosed();					//someone has closed the glarea
	void insertRenderingDataForNewlyGeneratedMesh(int);
	void currentViewerChanged(int currentId);
	void setProjectFirstEnterSinceSoftwareRunTrueSignal();
	void closeOcclusionToothAct();
	void closeTreatmentCompareAct();
	void closeDisplayToothMovemnetTableAct();

public slots:
	// Called when we change layer, notifies the edit tool if one is open
	void manageCurrentMeshChange();
	// Called when we modify the document
	/// Execute a end/start pair for all the PerMesh decorator that are active in this glarea.
	/// It is used when the document is changed or when some parameter changes
	/// Note that it is rather inefficient. Such work should be done only once for each decorator.
	void updateAllPerMeshDecorators()
	{
		MeshDocument* mdoc = md();
		if (mdoc == NULL)
			return;
		makeCurrent();

		for (QMap<int, QList<QAction*> >::iterator i = iPerMeshDecoratorsListMap.begin();
			i != iPerMeshDecoratorsListMap.end(); ++i)
		{

			MeshModel* m = md()->getMesh(i.key());
			foreach(QAction * p, i.value())
			{
				MeshDecorateInterface* decorInterface =
					qobject_cast<MeshDecorateInterface*>(p->parent());
				decorInterface->endDecorate(p, *m, this->glas.currentGlobalParamSet, this);
				decorInterface->setLog(&md()->Log);
				decorInterface->startDecorate(p, *m, this->glas.currentGlobalParamSet, this);
			}
		}

		MultiViewer_Container* viewcont = mvc();
		if (viewcont == NULL)
			return;

		MLSceneGLSharedDataContext* shared = viewcont->sharedDataContext();
		if (shared == NULL)
			return;

		MLDefaultMeshDecorators defdec(mw());
		for (MeshModel* mm = mdoc->nextMesh(); mm != NULL; mm = mdoc->nextMesh(mm))
		{
			MLRenderingData dt;
			shared->getRenderInfoPerMeshView(mm->id(), context(), dt);
			defdec.cleanMeshDecorationData(*mm, dt);
			defdec.initMeshDecorationData(*mm, dt);
		}
	}
	void updateAllDecorators();

public:
	//call when the editor changes
	void setCurrentEditAction(QAction* editAction);

	//get the currently active edit action
	QAction* getCurrentEditAction() { return currentEditor; }

	//get the currently active mesh editor
	MeshEditInterface* getCurrentMeshEditor() { return iEdit; }

	// Is there a current action?
	bool have_acttion_decorate_doc_ = false;

	//see if this glAarea has a MESHEditInterface for this action
	bool editorExistsForAction(QAction* editAction) {
		return actionToMeshEditMap.contains(editAction);
	}

	//add a MeshEditInterface for the given action
	void addMeshEditor(QAction* editAction, MeshEditInterface* editor) {
		actionToMeshEditMap.insert(editAction, editor);
	}
	bool readyToClose();
	float lastRenderingTime() { return lastTime; }
	void drawGradient();
	Point3m lightPos;
	void drawLight();
	float getFov() { return fov; }
	void importJawAlignment(int size);

signals:
	void transmitViewDir(QString name, Point3m dir);
	void transmitViewPos(QString name, Point3m pos);
	void transmitSurfacePos(QString name, Point3m pos);
	void transmitPickedPos(QString name, Point2m pos);
	void transmitCameraPos(QString name, Point3m pos);
	void transmitTrackballPos(QString name, Point3m pos);
	void transmitShot(QString name, Shotm);
	void transmitMatrix(QString name, Matrix44m);
	void updateLayerTable();

public slots:
	void sendViewPos(QString name);
	void sendSurfacePos(QString name);
	void sendPickedPos(QString name);
	void sendViewDir(QString name);
	void sendCameraPos(QString name);
	void sendMeshShot(QString name);
	void sendMeshMatrix(QString name);
	void sendViewerShot(QString name);
	void sendRasterShot(QString name);
	void sendTrackballPos(QString name);
	void shootPointList(Point3m*, int);

public:
	vcg::Point3f getViewDir();
	bool	infoAreaVisible;		// Draws the lower info area ?
	bool  suspendedEditor;
	bool trackBallVisible = false;		// Draws the trackball ?
	bool bShowMeshInfoVisible = false;

protected:
	void initializeGL();
	void displayInfo(QPainter* painter);

	int alpha_model_lab = 0;
	QTimer* timer_alpha_model_ = nullptr;
	void displayModelLabel(QPainter* painter);

	void displayRealTimeLog(QPainter* painter);
	void displayHelp();
	QString GetMeshInfoString();
	void paintEvent(QPaintEvent* event);
	void keyReleaseEvent(QKeyEvent* e);
	void keyPressEvent(QKeyEvent* e);
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);
	void wheelEvent(QWheelEvent* e);
	void tabletEvent(QTabletEvent* e);
	void hideEvent(QHideEvent* event);
	void resizeGL(int w, int h);
	void updateViewGui();
	void resizeEvent(QResizeEvent*);
public:
	void keyPressEventSpecial(QKeyEvent* e);
	void keyReleaseEventSpecial(QKeyEvent* e);
private:
	void setLightingColors(const MLPerViewGLOptions& opts);
	QMap<QString, QCursor> curMap;

	void setTiledView(GLdouble fovY, float viewRatio, float fAspect,
		GLdouble zNear, GLdouble zFar, float cameraDist);

	bool helpVisible = false;				// Help on screen

	bool activeDefaultTrackball; // keep track on active trackball
	bool hasToPick;	// has to pick during the next redraw.
	bool hasToGetPickPos;// if we are waiting for a double click for getting a surface position that has to be sent back using signal/slots (for parameters)
	bool hasToGetPickCoords;// if we are waiting for a double click for getting a position on the GLArea that has to be sent back using signal/slots (for parameters)
	bool hasToSelectMesh;// if we are waiting for a double click for getting a surface position that has to be sent back using signal/slots (for parameters)
	int id_mesh_selected = -1;
	QString nameToGetPickPos; // the name of the parameter that has asked for the point on the surface
	QString nameToGetPickCoords; // the name of the parameter that has asked for the point on the GLArea
	bool interrbutshow;
	vcg::Point2i pointToPick;
	Point3m adj_camera_vector_ = Point3m(0,0,0);

	Point3f pp;
	QMenu* mesh_operate_menu_ = nullptr;

	//shader support
	MeshRenderInterface* iRenderer;
	QAction* currentShader;
	QAction* lastFilterRef; // reference to last filter applied
	QFont	qFont;			//font settings

							// Editing support
	MeshEditInterface* iEdit = nullptr;

	bool getEditIsWorking() { return iEdit != nullptr; }
	QAction* currentEditor = nullptr;
	QAction* suspendedEditRef; // reference to last Editing Mode Used
	QMap<QAction*, MeshEditInterface*> actionToMeshEditMap;

	MeshModel* lastModelEdited;

	bool resetTrackCenter = false;
	bool moveCamera = false, setCameraRotateParameters = false, bEnableErlueRotate = false;
	Point3f aimCameraViewAxis, aimCameraView_RH_Axis;
	Matrix44f eulerRotateMatrix;

	bool penetrate_happend_ = false;
	void penetrateCorrection();

public:
	inline MLSceneGLSharedDataContext* getSceneGLSharedContext() {
		return ((mvc() != NULL) ? mvc()->sharedDataContext() : NULL);
	}
	vcg::Matrix44f trackBallsMatrix;
	bool bSwitchToMainMenu = false, bParallelSwitched = false;
	inline void setNeedSortModels(bool _state) { this->need_sort_mesh_render_ = _state; }
	// view setting variables
	float fov;
	float clipRatioFar;
	float clipRatioNear;
	float nearPlane;
	float farPlane;
	SnapshotSetting ss;

	// Store for each mesh if it is visible for the current viewer.
	QMap<int, bool> meshVisibilityMap;

	// Store for each raster if it is visible for the current viewer.
	QMap<int, bool> rasterVisibilityMap;

	// Add an entry in the mesh visibility map
	void meshSetVisibility(MeshModel* mp, bool visibility);

public slots:
	void eulerRotateModelSlot(Matrix44f& _m);
	void updateMeshSetVisibilities();
	void getTrackBallMatrix() { this->trackBallsMatrix = this->trackball.Matrix(); }
	void meshAdded(int index);
	void meshRemoved(int index);
	void weatherSwitchToMainMenu(bool _state);
	void weatherParallelSwitched(bool _state);

private:
	float cfps;
	float lastTime;

	QImage snapBuffer;
	bool takeSnapTile;

	enum AnimMode { AnimNone, AnimSpin, AnimInterp };
	AnimMode animMode;
	int tileCol, tileRow, totalCols, totalRows;   // snapshot: total number of subparts and current subpart rendered
	int  currSnapLayer;            // snapshot: total number of layers and current layer rendered
	void setCursorTrack(vcg::TrackMode* tm);

	//-----------Raster support----------------------------
private:
	bool _isRaster; // true if the viewer is a RasterViewer, false if is a MeshViewer; default value is false.

	int zoomx, zoomy;
	bool zoom;
	float opacity;
	GLuint targetTex;           // here we store the reference image. The raster image is rendered as a texture
	QString lastViewBeforeRasterMode; // keep the view immediately before switching to raster mode

	int dentalAnimationLoadOverNum = 0;

public:
	int lastloadedraster;
	//-----------Shot support----------------------------

public:
	QPair<Shotm, float > shotFromTrackball();
	void resetGlobalTrackBall();
	void createOrthoView(QString);
	void toggleOrtho();
	void trackballStep(QString);
	void viewFromClipboard();
	void loadShot(const QPair<Shotm, float>&);
	float cameraDistance = 30.0f;
	void showComparisonWindowDescription(QPainter& _p);
	bool belongToLowerDentalPart(ToothModelType _type);
	bool belongToUpperDentalPart(ToothModelType _type);
private:
	float getCameraDistance();
	inline float viewRatio() const { return 1.75f; }
	inline float clipRatioNearDefault() const { return 0.001f; }
	inline float fovDefault() const { return 60.f; }
	void initializeShot(Shotm& shot);

	/*
	Given a shot "refCamera" and a trackball "track", computes a new shot which is equivalent
	to apply "refCamera" o "track" (via GPU).
	*/
	template <class T>
	vcg::Shot<T> track2ShotGPU(vcg::Shot<T>& refCamera, vcg::Trackball* track) {
		vcg::Shot<T> view;

		double _near, _far;
		_near = 0.1;
		_far = 100;

		//get OpenGL modelview matrix after applying the trackball
		GlShot<vcg::Shot<T> >::SetView(refCamera, _near, _far);
		glPushMatrix();
		track->GetView();
		track->Apply();
		vcg::Matrix44d model;
		glGetv(GL_MODELVIEW_MATRIX, model);
		glPopMatrix();
		GlShot<vcg::Shot<T> >::UnsetView();

		//get translation out of modelview
		vcg::Point3d tra;
		tra[0] = model[0][3]; tra[1] = model[1][3]; tra[2] = model[2][3];
		model[0][3] = model[1][3] = model[2][3] = 0;

		//get pure rotation out of modelview
		double det = model.Determinant();
		double idet = 1 / pow(det, 1 / 3.0); //inverse of the determinant
		model *= idet;
		model[3][3] = 1;
		view.Extrinsics.SetRot(model);

		//get pure translation out of modelview
		vcg::Matrix44d imodel = model;
		vcg::Transpose(imodel);
		tra = -(imodel * tra);
		tra *= idet;
		view.Extrinsics.SetTra(vcg::Point3<T>::Construct(tra));

		//use same current intrinsics
		view.Intrinsics = refCamera.Intrinsics;

		return view;
	}

	/*
	Given a shot "refCamera" and a trackball "track", computes a new shot which is equivalent
	to apply "refCamera" o "track" (via CPU).
	*/
	template <class T>
	vcg::Shot<T> track2ShotCPU(vcg::Shot<T>& refCamera, vcg::Trackball* track) {
		vcg::Shot<T> view;

		double _near, _far;
		_near = 0.1;
		_far = 100;

		//get shot extrinsics matrix
		vcg::Matrix44<T> shotExtr;
		refCamera.GetWorldToExtrinsicsMatrix().ToMatrix(shotExtr);

		vcg::Matrix44<T> model2;
		model2 = (shotExtr)*vcg::Matrix44<T>::Construct(track->Matrix());
		vcg::Matrix44<T> model;
		model2.ToMatrix(model);

		//get translation out of modelview
		vcg::Point3<T> tra;
		tra[0] = model[0][3]; tra[1] = model[1][3]; tra[2] = model[2][3];
		model[0][3] = model[1][3] = model[2][3] = 0;

		//get pure rotation out of modelview
		double det = model.Determinant();
		double idet = 1 / pow(det, 1 / 3.0); //inverse of the determinant
		model *= idet;
		model[3][3] = 1;
		view.Extrinsics.SetRot(model);

		//get pure translation out of modelview
		vcg::Matrix44<T> imodel = model;
		vcg::Transpose(imodel);
		tra = -(imodel * tra);
		tra *= idet;
		view.Extrinsics.SetTra(vcg::Point3<T>::Construct(tra));

		//use same current intrinsics
		view.Intrinsics = refCamera.Intrinsics;

		return view;
	}

	/*
	Given a shot "from" and a trackball "track", updates "track" with "from" extrinsics.
	A traslation involving cameraDistance is included. This is necessary to compensate a trasformation that OpenGL performs
	at the end of the graphic pipeline.
	*/
	template <class T>
	void shot2Track(const vcg::Shot<T>& from,
		const float cameraDist, vcg::Trackball& tb) {

		vcg::Quaternion<T> qfrom; qfrom.FromMatrix(from.Extrinsics.Rot());

		tb.track.rot = vcg::Quaternionf::Construct(qfrom);
		tb.track.tra = (vcg::Point3f::Construct(-from.Extrinsics.Tra()));
		tb.track.tra +=
			vcg::Point3f::Construct(
				tb.track.rot.Inverse().Rotate(
					vcg::Point3f(0, 0, cameraDist))) * (1 / tb.track.sca);
	}

	//////////////////////////////////////////
private:
	// mesh shader support
	MeshShaderRender* mesh_shader_render_ = nullptr;
	bool need_sort_mesh_render_ = true;

public:
	// patient infomation
	PatientInfo patientInfo;

public:
	// transparency adjust of mesh
	bool bOpenTransparencyAdjust = true;
	bool bDrawUpperMeshOrder = true;
	// Transparency control
	vcg::glLabel::Mode tipLabelPen;

	// tooth adjust control
	bool bShowToothAdjustCtlBtn = false;

public:
	// the index tooth selected between 1 and 16, if it belows zero
	// just not select any tooth
	int currentIndexToothSelected = -1;
	// default select mesh through right mouse double click.
	// but in the tooth segment tool or others, we will disable the status.
	bool bWhetherEnableSelectMesh = true;
	bool bCurrentOperationToothUpperLower = true;// default operation upper tooth
	bool bShowUpperTooth = true;
	bool bShowLowerTooth = true;
	bool bEnableShadowMap = false;
	bool bDisplayPatientInfo = false;
	void displayPatientInfomation();

	// background display
	ui_common::TextureMap* texture_background_ = nullptr;

	// view direction(left right front back...)
	bool bHaveOcclusalPlane = false;
	CustomPlane viewTabPlane;
	ViewDirection viewDir = FRONT_VIEW;
	bool bHavePreferViewDirect_ = false;
	Point3m left_prefer_view_direct_, right_prefer_view_direct_;

	// project way in this scene
	bool bEnableOrthProject = true;

	//display dental overlap area
	Point3m cameraPosition, viewDirect, view_RH_Direct;
	void getCurrentCameraState(MeshModel* curMesh);

	//display dental compare model
	bool bEnableDisplayDentalCompareModel = false;
	bool bRolling = false;
	bool occlusal_coloring_in_progress_ = false;
	void drawDentalCompareModel(QPainter* painter);

	// display tooth movement table
	bool bEnableDisplayToothMovemnetTable = false;

signals:
	// tooth movement table
	//void updateToothAdjustTableContentSignal();
	void needUpdateTabNow();
	void tabUpperOrLowerJawMovemnetTableShowSignal(bool bUpperJaw);
	void setWhichToothAdjustMovementTableShowSignal(QString toothFdiLabel);

	// play animation gui connect todo
	// as your signal to connect

	// as your slot to connect

	void autoSaveProjectSignal(bool bAlignProject = true);
	bool importMeshWithLayerManagement(QString fileName);
	void setWhichToothAdjustSignal(QString toothFdiLabel);
	void getNewToothParameter(float x, float y, float z, float a, float b, float g);

	void updateOverlayOutline(Point3m*, int);
	void updateColorTableParams(float, float);
	//void updateColorTabVisible(bool);
	void shootLockSignal(bool);
	void setNewToothAdjustModeSignal(int Mode);
	void setStepIterateSmooth(int step);

	void updateNoAdjustValueSignal(QString frontNoAdjustValue, QString allNoAdjustValue);
	void getBoltonValueTeethSignal(QString frontTeetBoltonValue, QString allTeetBoltonValue);
	void getCrowdingDegreeParaInfo(float dentalArchShouldHave,
		float dentalArchNowHave, float crowdingDegreeValue);
	void reDrawDentalArchCommand();
	//distance coloring
	void sendNewTopDistance(float, float, Point3m);

public slots:
	void setGlobalTrackBallEnableSlot(bool enble);// 设置全局轨迹球是否响应信号槽
	void clearUI();
	void onePartOfInitalAnimationOverSlot();
	//void openToothTreatmentProjectSlot(QAction *action);
	void setCurrentToothMeshOperateSlot(bool bUpperOrTooth);
	void occlusalColoringInProgressSlot(bool _state);

	////// tooth view dir
	void showViewDirectionExpand(int viewDirection);
	void showViewDirection(ViewDirection viewDirection);
	///// some global tool slot
	void setStepIterateSmoothReceive(int step);
	void dentalCompareModelShow();
	void dentalRenderObjectSwitch();
	void deleteMeshModel(int);

	///// tooth crowding degree
	void updateNoAdjustValueSlot(QString frontNoAdjustValue, QString allNoAdjustValue);
	void setBoltonValueTeethSlot(QString frontTeetBoltonValue, QString allTeetBoltonValue);
	void setCrowdingDegreeParaInfo(float dentalArchShouldHave,
		float dentalArchNowHave, float crowdingDegreeValue);
	void reDrawDentalArchCommandReceive();

	void getQtDeviceToLogicalValueSlot(float _a, float& _b);
	void getQtLogicalToDeviceValueSlot(float _a, float& _b);

	void setWhichToothAdjustSlot(QString toothFdiLabel);
	//void changeColorTalbeVisible(bool);
	void setShowOrHideUpperTooth(int status);///上颌部分可见性勾选框slot
	void setShowOrHideLowerTooth(int status);///下颌部分可见性勾选框slot
	void setShowOrHideUpperToothCommand(int status);
	void setShowOrHideLowerToothCommand(int status);
	void setLightnessAlphaUpperTooth(int step);
	void setLightnessAlphaLowerTooth(int step);
	void setTransparencyUpperTooth(int step);
	void setTransparencyLowerTooth(int step);
	void udpateEachMeshVertexInfoBuffer();

	// tooth adjust
	// todo connect

	void isShowToothAdjustCtlBtnSlot(bool bShow);
	void setMainWindowFocus();

public slots:
	void adjustCamera();
	void adjustForwardStep();
	void autoRotateCameraToWantedDirectSlot(Point3m aimViewDirect, Point3m aimView_RH_Direct, Point3m aimCameraPos, float aimScale = -1);//aimViewDirect is identity
	void autoRotateCameraToWantedDirect(Point3m aimViewDirect);
	void adjustRotateStep();
	void resetTrackballCenterSlot();

public:
	Point3m cameraPos;
	QTimer* tempTimer = NULL;
	QTimer* wheelTimer = nullptr;
	QTimer* rotTimer = nullptr;
	int iCamStep = 0;
	float cameraRotateAngleSum = 0;
	Point3m cameraRotateAxis, cameraRotateStartAxis, cameraRotateEndAxis, cameraTransStartPoint, cameraTransEndPoint;
	float cameraTransStartScale = -1, cameraTransEndScale = -1;
	Axis cameraStartAxis, cameraEndAxis;
	Point3m cameraMoveVector, cameraRotateVector;
	//vector<Axis> camTransTrial;

public slots:
	void wheelStopSlot();
	void setTrackballSphereEnable(bool bState);

private:
	// custom button of GUI designed using here.
	float alphaTransparencyPlayTimeAxisGui = 0.5f;
	bool bBanTrackballSphereMode = false;

public:
	float heightTipAllControls = 0.04f;
	float endXUpperLowerAlphaSlider = 0.0f;
	float alphaTransparencyUpperLowerGui = 0.5f;

	bool bEnableShowCollectAndFoldedGui = true;
	ui_common::CollectFoldedGui* collect_folded_gui_ = nullptr;
	void drawCollectFoldedGui(QPainter* painter);
public:
	bool bSaveAlignProjectAuto = false; // default save align project auto
	bool bSaveSegmentProjectAuto = false; // default save align project auto
public:
	void setAlignProjectToSaveAuto(bool bSaveAlignProject = true);
	void setSegmentProjectToSaveAuto(bool bSaveSegmentProject = true);
	bool getWhetherAlignProjectNeedToSaveAuto()const;
	bool getWhetherSegmentProjectNeedToSaveAuto()const;
	void setFixAnaBtnText();

	ui_common::fusionViewGui* getViewGui() const { return p_view_gui_; }
	tipsWidget* tips_button_gui_ = nullptr;
	/// View GUI related
private:
	ui_common::fusionViewGui* p_view_gui_ = nullptr;
	Point3f camera_pos_;
	Point3f camera_target_pos_;
	Point3f camera_up_dir_;
	Point3f light_pos_;

public:
		bool b_is_clone_ = false, b_first_open_clone_ = false;
		void setGlareaIsClone(bool is_clone);// 设置当前窗口是不是clone的
		QPushButton* compare_switch_link_btn_ = nullptr;
		QPushButton* Import_jaw_alignment = nullptr; //导入对颌
		QPushButton* Fixed_point_analysis = nullptr; //定点分析按钮
		QPushButton* Skip_the_upper_jaw = nullptr;  //跳过上颌
		QPushButton* complete_btn = nullptr;  //完成按钮
		QPushButton* Previous_step = nullptr; //上一步
		QPushButton* jaw_alignment = nullptr;
		QPushButton* gif_button = nullptr;
		QPushButton* next_step_button = nullptr;
		// <- Previous step
		QPushButton* forward_step_button_ = nullptr;
		eProjAnalyserStage nowStage;
		eProjAnalyserStage preStage;
		bool change_link_status = false;
		void changeLinkBackground(bool status);
		navAnabutton* Ana_Import_jaw_alignment = nullptr;
public slots:
	void onSelectDentalSlot(int dentalType);
	void setMeshDocCurrentMeshSlot(int _index);
	void setViewTabPlane(CustomPlane _plane)
	{
		Point3m axis_x = _plane.axisXV;
		Point3m axis_y = _plane.axisYV;
		Point3m axis_z = (axis_x ^ axis_y).Normalize();
		axis_y = (axis_z ^ axis_x).Normalize();

		_plane.axisXV = axis_x;
		_plane.axisYV = axis_y;
		_plane.axisZV = axis_z;

		this->viewTabPlane = _plane;
		this->bHaveOcclusalPlane = true;
		emit PSIGNALMANAGER->setViewDirectConsoleVisibleSignal(true);
	}

		public:
			bool dental_analysis_press_ = false, dental_analysis_move_ = false, dental_analysis_release_ = false;
			int mouseX_ = 0;
			int mouseY_ = 0;
};

#endif
