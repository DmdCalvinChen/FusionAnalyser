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

#ifndef CUT_FACE_H_
#define CUT_FACE_H_
#define CIRCLE_DIVIDE_NUMBER 360
#define PI_2X_ANGLE 360
#define PI 3.1415926
#define PI_2X  6.283185
#define  STEP_RADS  0.0174532
#define CTRLSYS_VISIBLE_INCLINATION 0.866025f
#define CTRLSYS_ROTATION_INCLINATION 0.7f
#define CTRLNODE_RADIUS_MIN  0.2f
#define CTRLNODE_RADIUS_MAX  1.0f

#include "common/ml_mesh_type.h"
#include "common/meshmodel.h"
#include "common_ext/util/assist_geometry.h"
#include "common_ext/util/utility_tools.h"
#include "common_ext/pointcloud/cloudkdtree.h"
#include "common_ext/common_ext_global.h"

enum CtrlNodeIndex { NONE_CTRL_NODE = -1, ROTATE_CTRL_NODE_1, ROTATE_CTRL_NODE_2, ROTATE_CTRL_NODE_3, TRANSLATE_CTRL_NODE };
class COMMON_EXT_EXPORT CutFace
{
public:
	CutFace();
	CutFace(Point3m _basalP, Point3m _normalV, Point3m _axisXV, Point3m _axisYV);
	void initFace(Point3m& _basalP, Point3m& _normalV, Point3m& _axisXV, Point3m& _axisYV);
	void initFace2(Point3m& _basalP, Point3m& _normalV, Point3m& _axisXV, Point3m& _axisYV, float _radius);
	void updateCutFace();
	void draw(int _screen_wid = 0, int _screen_hei = 0);
	void drawCtrlNodeAdjustingDirection(Point3m _node, float _radius, std::pair<bool, bool>& _left_right_enable, const Axis& _axis, const Point3m &_camPos, const Point3m& _viewDir);
	void drawCtrlNodes(int _screen_wid = 0, int _screen_hei = 0);
	void drawCutFace();
	void updateCtrlSystem(Point3m _cameraPos, Point3m _viewDirect);
	bool pickAdjHandle(int x, int y, CtrlNodeIndex& _ctrlNodeIndex);
	void transformAction(CtrlNodeIndex _ctrlNodeIndex, int _mouseX, int _mouseY, int _width, int _height);
	void rotateAction(CtrlNodeIndex _ctrlNodeIndex, int _mouseX, int _mouseY, int _width, int _height);
	void translateAction(CtrlNodeIndex _ctrlNodeIndex, int _mouseX, int _mouseY, int _width, int _height);
	void cutMeshModel(MeshModel *_model, CloudOctree* _p_tree);
	bool computeCrossPoint(Point3m startP, Point3m endP, Point3m cutFaceNormalV, Point3m cutFacePosP, Point3m& CrossPoint);
	void computeCrossSegmentByFace(CFaceO* face, Point3m cutFaceNormalV, Point3m cutFacePosP, vcg::Color4b _color, vcg::Matrix44f _matrix);

	static void setDefaultCutPlaneRadiusMin(float _radius);
	static void setDefaultCutPlaneRadiusMax(float _radius);
	inline void setFaceColor(Point4m _color) { face_color_ = _color; }
	inline void setBoundaryColor(Point4m _color) { boundary_color_ = _color; }

public:
	float fRadius_;
	float fRadius_CtrlNode;
	Axis axis_;
	Axis ctrl_axis_;
	std::vector<Point3m> ctrlPoints;
	std::vector<std::pair<bool, bool>> leftAndRightAdjEnable_;
	std::vector<Point3m> boundaryVerts;
	std::vector<FEdge> cut_profile_;
	Point4m boundary_color_, face_color_;

	static float DEFAULT_CUTPLANE_RADIUS_MIN;
	static float DEFAULT_CUTPLANE_RADIUS_MAX;
};
#endif
