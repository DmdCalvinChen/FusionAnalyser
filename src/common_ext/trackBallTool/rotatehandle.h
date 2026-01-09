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

#ifndef  COMMON_ROTATEHANDLE_H
#define COMMON_ROTATEHANDLE_H
#define CIRCLE_DIVIDE_NUMBER 360
#define PI_2X_ANGLE 360
#define PI 3.1415926
#define PI_2X  6.283185
#define STEP_RADS  0.0174532 /**((2*PI)/(CIRCLE_DIVIDE_NUMBER))*/

#include "trackBallTool/customadjustinghandle.h"
#include "common_ext_global.h"
#include "common_ext/util/assist_geometry.h"

class AbsMesh;
class COMMON_EXT_EXPORT RotateHandle :
	public CustomAdjustingHandle
{
public:
	RotateHandle(AbsMesh *_obj, AdjDirect _direct, float _radius, float _fine_step);
	RotateHandle(AbsMesh *_obj, AdjDirect _direct, float _radius, float _coeffient, float _fine_step);
	RotateHandle(AbsMesh *_obj, Point3m *_origin, Point3m *_axis, float _radius, float _fine_step);
	~RotateHandle();

	bool drawHandle();
	void drawNormalPlane();
	bool pickHandel(int _mouseX, int _mouseY);
	bool adjustAction(int _mouseX, int _mouseY);
	bool adjustFineTuneAction(int _direct);
	void updateCtrlSys();
	void carroutTransform(float _value);
	void carryoutLocalAxisTransformH(Point3m origin, Axis curAxis, float _value);
	void afterMouseRelease();
	void initalNormalPlaneVerts();
	void setDrawNormalPlaneEnable(bool _state);
	Point3m currentMoveDirect() { return Point3m(0, 0, 0); }

private:
	std::vector<Point3m> circle_loop_;
	Point3m picked_node_;
	bool b_draw_picked_node_ = false;
	Point3m plane_vert1, plane_vert2, plane_vert3, plane_vert4;
	bool b_draw_normal_plane = false;

};

#endif // ! COMMON_ROTATEHANDLE_H
