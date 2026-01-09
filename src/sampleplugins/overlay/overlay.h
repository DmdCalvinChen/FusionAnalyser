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

#ifndef OVERLAY_H
#define OVERLAY_H

#define PI 3.141592653

#include <QObject>
#include <common/interfaces.h>
#include <common_base/SignalManager.h>
#include <vcg/simplex/face/pos.h>
#include <vcg/simplex/face/topology.h>
#include <vcg/complex/complex.h>
#include <qdebug.h>
#include <wrap/gl/pick.h>
#include <wrap/gl/addons.h>
#include <qdesktopwidget.h>
#include <wrap/qt/device_to_logical.h>

#include "ui/overlapoverbitegui.h"
#include "ui/overlapoverbiteAnagui.h"
#include "common_ext/data/CutFace.h"
#include "common_ext/data/interactive2Dlabel.h"
#include "common_ext/data/interactive2Dframe.h"
#include "common_ext/data/dentalanalysisdata.h"
#include <UI_Common/gifViewLabel/gifviewgui.h>
#include <UI_Common//FusionAnalyserHomeGui/fusionanalyserbuttongui/tipswidget.h>

class Overlay : public QObject, public MeshEditInterface
{
	Q_OBJECT
	Q_INTERFACES(MeshEditInterface)

public:
    Overlay();
	Overlay(QAction* pAct);
    virtual ~Overlay() {}

    static const QString Info();

    bool StartEdit(MeshModel &/*m*/, GLArea * /*parent*/, MLSceneGLSharedDataContext* /*cont*/);
    void EndEdit(MeshModel &/*m*/, GLArea * /*parent*/, MLSceneGLSharedDataContext* /*cont*/);
    void Decorate(MeshModel &/*m*/, GLArea * /*parent*/, QPainter *p);
	void DecorateBeforeMeshRender(MeshModel &/*m*/, GLArea * /*parent*/, QPainter *p);
    void mousePressEvent(QMouseEvent *, MeshModel &, GLArea * );
    void mouseMoveEvent(QMouseEvent *, MeshModel &, GLArea * );
	void mouseDoubleClickEvent(QMouseEvent *event, MeshModel &/*m*/, GLArea *) {};
    void mouseReleaseEvent(QMouseEvent *event, MeshModel &/*m*/, GLArea * );
	void keyReleaseEvent(QKeyEvent *, MeshModel &, GLArea *);
	void copyCurrentStateToCheekSide();

public:
	Point3m basalPoint;
	Point3m nv, axisY, axisX;
	CutFace* cutface = nullptr;
	CutFace left_face_, right_face_;

public:
	void myDraw(QPainter *p);
	void drawDentalArch();

	void initalCutFaceTool(bool _load_record_cut_face = false);
	int  getCurrentSegmentIndex();
	void updateCurrentBasalPoint();
	bool computeCrossPoint(Point3m startP, Point3m endP, Point3m cutFaceNormalV, Point3m cutFacePosP, Point3m &CrossPoint);
	void computeCrossSegmentByFace(CFaceO *face, Point3m cutFaceNormalV, Point3m cutFacePosP, vcg::Color4b _color, vcg::Matrix44f _matrix, ToothModelType _type);
	Point3m proj3DPointOnCutFace(Point3m p, Point3m axis_X, Point3m axis_Y);
	void getCutOutline();
	void getProjectCutOutline();
	Point3m *pointlist = nullptr;
	int pointNum;
	Gifviewgui* gif_overlay_ui = nullptr;  // Overbite/overjet GIF
	tipsWidget* tipBtn = nullptr;
	vector<Interactive2DLabel> labels_;
	vector<Interactive2DFrame> frames_;
	OverlayOutlinePreview::CheekSide cheek_side_ = OverlayOutlinePreview::RIGHT_SIDE;

private:
	DentalFeatures* last_upper_dental_features_ = nullptr, * last_lower_dental_features_ = nullptr;
	int mouseX, mouseY;
	Point3m mouseStart, mouseEnd;
	bool bCanHandles = false;
	bool bAction = false;
	bool bActionReady = false;
	bool PickOnehandle = false;
	QAction* p_act_ = nullptr;
	CtrlNodeIndex pickedIndex_ = CtrlNodeIndex::NONE_CTRL_NODE;
	bool bUpdateCutOutline = false;
	//MeshModel *mesh;
	GLArea *parent = nullptr;
    QPoint curGL;
    bool haveToPick;
	CMeshO::FacePointer  curFacePtr;
	int pIndex;
	float radius = 1.0f;
	int slices = 16;
	int stacks = 16;

	int   iSegment;
	int   iSegmentNumber;
	float fStep = 0.01f;
	float fSliderDistance = 0;
	float fSliderDistanceSum = 0;
	vector<Point3m> archPoints;
	vector<float>   distanceList;
	vector<float>   distanceAccumulate;

	vector<VertCP> cutVertCP;
	vector<VertCP> projCutVertCP;

	/** Rotation system */
	int currentHandle;
	int iSelectedHandle;
	Point3m handle;
	Point3m rotateOrigin;
	Point3m vA, vB, vC;
	Point3m rotateAxisVector;
	bool bFirstTime = true;
	float show_overjet_value_ = -1;
	QString show_overjet_rank_;
	float show_overbite_value_ = -1;
	QString show_overbite_rank_;
	float show_left_overbite_, show_left_overjet_, show_right_overbite_, show_right_overjet_;
	QColor show_overbite_value_color_, show_overjet_value_color_;
	int label_row_height_ = 0;

	Point3m getMixedPointFromScreenToWorld(float mX, float mY, Point3m nV, Point3m hP);
	vcg::Matrix44f computeTransformMatrix(Point3m pos, Point3m axisVector, float angle);
	void setNewPointList();
	void updateAnaGuiSize();
	float getCurrentLabelAreaHeight();

signals:
	void suspendEditToggle();
	void updatePointList(const vector<VertCP> &cpList);
	void updateDlgVisible(bool);
	void setNeedUpdateOverlayOutlineSignal(bool);
	void doneSignal();
	void loadLeftMeasureRecordSignal(FEdge _edge);
	void loadRightMeasureRecordSignal(FEdge _edge);

public slots:
	void adjustCutFacePos(CutFace* _face);
	void updateAccordingSlider(float fSlider);
	void updateCurrentSectionOutline();
	void ensureCutFaceBySilderDistance();
	void updateOverlayStateSlot(float _overjet_value, QString _overjet_rank, Point3m _overjet_color, float _overbite_value, QString _overbite_rank, Point3m _overbite_color);
	void updateLeftAndRightParametersSlot(float _left_overbite, float _left_overjet, float _right_overbite, float _right_overjet);
	void setCheekSideSlot(OverlayOutlinePreview::CheekSide _side);
	void updateLabels();
	void showOverlayGifSlot();
	void updateSceneSizeSlot(int _width, int _height);
private:
	OverlapOverbiteGui* gui_ = nullptr;
	OverlapOverbiteAnaGui* guiAna_ = nullptr;
	void createGui(MeshModel* _mesh, GLArea* parent);
	void closeGui();
	void updateUI(bool visible = true);
	bool need_align_camera_view_ = false;
	bool need_update_sub_dlg_ = false;
	int ana_gui_width_ = 380, ana_gui_height_ = 380;
	QString prompt_context_ = QString("OJ/OB");
};

#endif
