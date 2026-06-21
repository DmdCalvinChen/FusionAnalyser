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

#ifndef  COMMON_CUSTOMADJHANDLE_H
#define COMMON_CUSTOMADJHANDLE_H
#define TOOTH_BASE_CTRL_RADIUS 6.0f
#define ACCESSORY_BASE_CTRL_RADIUS 4.0f
#define ZOOM_HANDLE_EXTEND_LEN 2.0f
#define MAX_OVERLAP_DEPTH 0.5f
#define CONE_WIDTH 1.0f
#define ZOOM_CONE_WIDTH 0.2f

#include "common/ml_mesh_type.h"
#include <GL/glew.h>
#include <wrap/gl/math.h>
#include <wrap/gl/pick.h>
#include <wrap/gui/trackball.h>
#include <wrap/gl/addons.h>
#include <wrap/gl/trimesh.h>

#include "common_ext_global.h"

enum AdjDirect { X_PLUS = 0, X_MINUS, Y_PLUS, Y_MINUS, Z_PLUS, Z_MINUS , CUSTOM_DIRECT};

class AbsMesh;
class COMMON_EXT_EXPORT CustomAdjustingHandle
{
public:
	CustomAdjustingHandle(AbsMesh *_obj, AdjDirect _direct, float _radius, float _fine_step);
	CustomAdjustingHandle(AbsMesh *_obj, AdjDirect _direct, float _radius, float _max_coeffient, float _fine_step);
	CustomAdjustingHandle(AbsMesh *_obj, Point3m *_origin, Point3m *_axis, float _radius, float _fine_step);
	CustomAdjustingHandle(AbsMesh *_obj, Point3m _origin, float _fine_step);
	virtual ~CustomAdjustingHandle();

	virtual bool drawHandle() = 0;
	virtual bool pickHandel(int _mouseX, int _mouseY) = 0;
	virtual bool adjustAction(int _mouseX, int _mouseY) = 0;
	virtual bool adjustFineTuneAction(int _direct) = 0;
	virtual void updateCtrlSys() = 0;
	virtual void carroutTransform(float _value) = 0;
	virtual void afterMouseRelease() = 0;
	virtual Point3m currentMoveDirect() = 0;
	Point3m getColor(AdjDirect _direct);
	Point3m* getAxisVector(AdjDirect _direct);
	void setRadius(float _radius);
	void setFineTune(bool _switch);
	void paintStraightLineH(const Point3m &_v1, const  Point3m &_v2, const  Point3m &_color, const float &_width);
	void paintCircleLinesH(const std::vector< Point3m> &_circle, const  Point3m &_color, const float &_width);
	void getAssistNormalVector(AdjDirect _direct);
	void setObject(AbsMesh *_obj);
	void updateAssistNormalVector();
	bool fineTuneEnable();
	AdjDirect curDirect();
	bool setAdjustAxisSelf(bool _state);
	bool operatePositiveHandle();

protected:
	// Target object
	AbsMesh *obj_ = nullptr;
	// Maximum adjustment coefficient
	float max_adj_coeffient_ = 1.0f;
	// Center point
	Point3m *origin_ = nullptr;
	// Axis vector
	Point3m *axis_vec_ = nullptr;
	// Drawing color
	Point3m color_;
	// Drawing radius
	float radius_;
	// Fine tune switch
	bool fine_tune_ = false;
	float fine_tune_unit;
	// Auxiliary plane normal vectors
	Point3m *assist_normal_ = nullptr, *assist_normal2_ = nullptr;
	// Adjustment direction
	AdjDirect direct_;
	// Adjust self coordinate system
	bool adjust_axis_self_ = false;
};

#endif // ! COMMON_CUSTOMADJHANDLE_H

