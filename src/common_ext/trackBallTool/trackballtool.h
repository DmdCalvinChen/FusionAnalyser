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

#ifndef TRACK_BALL_TOOL_H
#define TRACK_BALL_TOOL_H

#include <vector>
#include <QMouseEvent>
#include "common_ext_global.h"
#include "common/ml_mesh_type.h"
#include <util/assist_geometry.h>

/*
   trackball for using in model

*/

class MeshModel;
class QGLWidget;

#define NOTING_MODE    0
#define TRANSLATE_MODE 1
#define ROTATE_MODE    2
#define SCALE_MODE     3

#define CIRCLE_DIVIDE_NUMBER 360
#define PI_2X_ANGLE 360
#define PI 3.1415926
#define PI_2X  6.283185
#define  STEP_RADS  0.0174532 /**((2*PI)/(CIRCLE_DIVIDE_NUMBER))*/
enum {
	XP = 0,
	XM,
	YP,
	YM,
	ZP,
	ZM
};
enum {
	X_PLUS_TRANSLATE = 0,
	X_MINS_TRANSLATE,
	Y_PLUS_TRANSLATE,
	Y_MINS_TRANSLATE,
	Z_PLUS_TRANSLATE,
	Z_MINS_TRANSLATE
};
enum {
	X_PLUS_ROTATE = 0,
	X_MINS_ROTATE,
	Y_PLUS_ROTATE,
	Y_MINS_ROTATE,
	Z_PLUS_ROTATE,
	Z_MINS_ROTATE
};

enum {
	X_PLUS_SCALE = 0,
	X_MINS_SCALE,
	Y_PLUS_SCALE,
	Y_MINS_SCALE,
	Z_PLUS_SCALE,
	Z_MINS_SCALE
};

class COMMON_EXT_EXPORT TrackBallTool
{
public:
	TrackBallTool();
	~TrackBallTool();

public:
	void initMesh(MeshModel* _mesh);
	void initMesh(MeshModel* _mesh, Axis& _axis, Axis* changing_axis = nullptr);
	void initAxis(const Axis& _axis);
	void initAxis(const Axis& _axis, Axis* changing_axis);
	void setNeedChangeMesh(bool need);
	void mousePressEvent(QMouseEvent* event, QGLWidget* gla);
	void mouseMoveEvent(QMouseEvent* event, QGLWidget* gla);
	void mouseReleaseEvent(QMouseEvent* event, QGLWidget* gla);
	void Decorate(QGLWidget* gla, QPainter* p);
	void setEnabled(bool b_enable_show_status);
	void setScaleModeStatus(bool b_is_scale_mode = false);
	bool isChangedAxisOnce()const { return updateOnce; }

private:
	vcg::Point3d unProjectPoint(Point3m p);
	vcg::Point3d projectPoint(Point3m p);
	void computeMainActionVector(int iHandles, Point3m& mainVector);
	bool pickHandle(int x, int y, int& indexPicked);
	void generateActionVectorEvent(QGLWidget* gla);
	void drawIsAdjustingLocalToothAxis();
	void adjustAction(int actionMode);
	void transformAndUpdateMesh(const vcg::Matrix44f& transform, MeshModel* current_mesh);
	void updateMesh(MeshModel* current_mesh);
	void rotateToothCoordinate(int iActionMode, float fAngle);
	Point3m getMixedPointFromScreenToWorld1(
		float mX, float mY, Point3m nV, Point3m hP, Point3m mainV);
	Point3m getMixedPointFromScreenToWorld2(
		float mX, float mY, Point3m nV1, Point3m nV2, Point3m hP);
	Point3m getMixedPointFromScreenToWorldOnVerticalPlane(
		float mX, float mY, Point3m origin, Point3m mainV);
	void updateCtrlHandles();
	void updateLocalCircle();
	void rotateOnePoint(Point3m origionCopy,
		Point3m axis, float angle, Point3m& P);

	void transformAndUpdatePoint(const vcg::Matrix44f& transform, Point3m& p);
	vcg::Matrix44f getRotateMatrix(Point3m origionCopy, Point3m axis, float angle);
	vcg::Matrix44f setZoomDeg(float value, Point3m axis);
	vcg::Matrix44f zoom(Axis curSys, Point3m zoomVector);
	vcg::Matrix44f getTranslateMatrix(Point3m translate);
	vcg::Matrix44f getScaleMatrix(Point3m scale);

public:
	bool b_is_enabled = true;
	bool b_need_modify_mesh = true;
	bool need_update_mesh_position = false;
	Axis axis;
	Axis* changing_axis = nullptr;

private:
	MeshModel* mesh = nullptr;
	QGLWidget* parent;
	int mouseX, mouseY;
	float angle = 0;
	int ActionMode = -1;
	int iSelectedHandle = -1;
	float scale_factor = 0.005;
	int iOperateMode = ROTATE_MODE;
	QPoint currentScreenPoint;
	bool bCanDrawHandles = false;
	bool bCanHandles = false;
	bool bAction = false;
	bool bActionReady = false;
	bool PickOnehandle = false;
	bool bEndAdjustAxis = false;
	bool updateOnce = false;
	bool pickEnabled = false;
	int slices = 20;
	int stacks = 20;
	float widthCone = 0.8f;
	float cuboid_length = 0.4f;
	float cuboid_height = 0.3f;;
	float cuboid_width = 0.4f;

	float width = 100.0f;
	float height = 100.0f;
	float fCtrlRadius = 1.5f;
	float radiusControlAxis = 0.8f;
	float fMangnifValue = 8.0;

	Point3m currentCenter;
	Point3m           moveVector;
	Point3m           rotateVector;
	Point3m actMainV = Point3m(0, 0, 0);
	vcg::Point3d curProjhandle;
	vcg::Point3d mouseProjPoint;
	Point3m mouseStart, mouseEnd;

	std::vector<Point3m> translateHandles;
	std::vector<std::pair<Point3m, Point3m>> translateHandlesTemp;
	std::vector<Point3m> rotateHandles;

	vcg::Point3f rotateInfo;
	vcg::Point3f transInfo;
	std::vector<vcg::Point3f> circlePointYOZ;
	std::vector<vcg::Point3f> circlePointZOX;
	std::vector<vcg::Point3f> circlePointXOY;

};

#endif
