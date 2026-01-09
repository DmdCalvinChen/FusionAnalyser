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

#include "customadjustinghandle.h"
#include "meshExt/AbsMesh.h"
#include "util/utility_tools.h"

CustomAdjustingHandle::CustomAdjustingHandle(AbsMesh *_obj, AdjDirect _direct, float _radius, float _fine_step)
{
	obj_ = _obj;
	direct_ = _direct;
	origin_ = &obj_->realTimeAxis.centerPoint;
	fine_tune_unit = _fine_step;
	axis_vec_ = getAxisVector(_direct);
	color_ = getColor(_direct);
	getAssistNormalVector(_direct);
	setRadius(_radius);
	setAdjustAxisSelf(false);
}

CustomAdjustingHandle::CustomAdjustingHandle(AbsMesh *_obj, AdjDirect _direct, float _radius, float _max_coeffient, float _fine_step)
{
	obj_ = _obj;
	direct_ = _direct;
	origin_ = &obj_->realTimeAxis.centerPoint;
	fine_tune_unit = _fine_step;
	axis_vec_ = getAxisVector(_direct);
	color_ = getColor(_direct);
	getAssistNormalVector(_direct);
	max_adj_coeffient_ = _max_coeffient;
	setRadius(_radius);
	setAdjustAxisSelf(false);
}

CustomAdjustingHandle::CustomAdjustingHandle(AbsMesh *_obj, Point3m *_origin, Point3m *_axis, float _radius, float _fine_step)
{
	obj_ = _obj;
	origin_ = _origin;
	fine_tune_unit = _fine_step;
	axis_vec_ = _axis;
	direct_ = AdjDirect::CUSTOM_DIRECT;
	color_ = getColor(direct_);
	setRadius(_radius);
	updateAssistNormalVector();
	setAdjustAxisSelf(false);
}

CustomAdjustingHandle::CustomAdjustingHandle(AbsMesh *_obj, Point3m _origin, float _fine_step)
{
	obj_ = _obj;
	fine_tune_unit = _fine_step;
	origin_ = &_origin;
	setAdjustAxisSelf(false);
}

CustomAdjustingHandle::~CustomAdjustingHandle()
{
}

Point3m CustomAdjustingHandle::getColor(AdjDirect _direct)
{
	if (adjust_axis_self_)
	{
		return Point3m(1, 0.501f, 0);
	}

	switch (_direct)
	{
	case X_PLUS:
	case X_MINUS:
		return Point3m(0.835f, 0, 0);

	case Y_PLUS:
	case Y_MINUS:
		return Point3m(0.1607f, 0.3843f, 1);

	case Z_PLUS:
	case Z_MINUS:
		return Point3m(0.392f, 0.8667f, 0.0901f);

	default:
		break;
	}
	return Point3m(0.501f, 0.501f, 1);
}

Point3m* CustomAdjustingHandle::getAxisVector(AdjDirect _direct)
{
	switch (_direct)
	{
	case  X_PLUS:
	case X_MINUS:
		return &obj_->realTimeAxis.axisXVector;

	case Y_PLUS:
	case Y_MINUS:
		return &obj_->realTimeAxis.axisYVector;

	case Z_PLUS:
	case Z_MINUS:
		return &obj_->realTimeAxis.axisZVector;

	default:
		break;
	}
}

void CustomAdjustingHandle::setRadius(float _radius)
{
	radius_ = _radius;
}

void CustomAdjustingHandle::setFineTune(bool _switch)
{
	fine_tune_ = _switch;
}

void CustomAdjustingHandle::paintStraightLineH(const  Point3m &_v1, const  Point3m &_v2, const  Point3m &_color, const float &_width)
{
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);  // Antialias the lines
	glLineWidth(_width);
	glColor3f(_color.X(), _color.Y(), _color.Z());
	glBegin(GL_LINES);
	glVertex3f(_v1.X(), _v1.Y(), _v1.Z());
	glVertex3f(_v2.X(), _v2.Y(), _v2.Z());
	glEnd();
	glLineWidth(CONE_WIDTH);
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

void CustomAdjustingHandle::paintCircleLinesH(const std::vector< Point3m> &_circle, const  Point3m &_color, const float &_width)
{
	Point3m vert;
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);  // Antialias the lines
	glColor3f(_color.X(), _color.Y(), _color.Z());
	glLineWidth(_width);
	glBegin(GL_LINE_STRIP);
	for (auto v : _circle)
	{
		glVertex3f(v.X(), v.Y(), v.Z());
	}
	vert = _circle.front();
	glVertex3f(vert.X(), vert.Y(), vert.Z());
	glEnd();
	glLineWidth(CONE_WIDTH);
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

void CustomAdjustingHandle::getAssistNormalVector(AdjDirect _direct)
{
	assert(obj_);
	switch (_direct)
	{
	case X_PLUS:
		assist_normal_ = &obj_->realTimeAxis.axisYVector;
		assist_normal2_ = &obj_->realTimeAxis.axisZVector;
		break;
	case X_MINUS:
		assist_normal_ = &obj_->realTimeAxis.axisYVector;
		assist_normal2_ = &obj_->realTimeAxis.axisZVector;
		break;
	case Y_PLUS:
		assist_normal_ = &obj_->realTimeAxis.axisXVector;
		assist_normal2_ = &obj_->realTimeAxis.axisZVector;
		break;
	case Y_MINUS:
		assist_normal_ = &obj_->realTimeAxis.axisXVector;
		assist_normal2_ = &obj_->realTimeAxis.axisZVector;
		break;
	case Z_PLUS:
		assist_normal_ = &obj_->realTimeAxis.axisYVector;
		assist_normal2_ = &obj_->realTimeAxis.axisXVector;
		break;
	case Z_MINUS:
		assist_normal_ = &obj_->realTimeAxis.axisYVector;
		assist_normal2_ = &obj_->realTimeAxis.axisXVector;
		break;
	default:
		break;
	}
}

void CustomAdjustingHandle::setObject(AbsMesh *_obj)
{
	obj_ = _obj;
}

void CustomAdjustingHandle::updateAssistNormalVector()
{
	if (assist_normal_ == nullptr)
	{
		assist_normal_ = new Point3m;
	}
	if (assist_normal2_ == nullptr)
	{
		assist_normal2_ = new Point3m;
	}
	Point3m axisA, axisB;
	axisA = *origin_ + Point3m(axis_vec_->X(), axis_vec_->Y(), axis_vec_->Z() * 2.0f);
	axisA = UtilityTools::getInstance()->getProjPointOnPlane(axisA, *origin_, *axis_vec_);
	axisA = (axisA - *origin_).Normalize();
	axisB = (axisA ^ *axis_vec_).Normalize();
	*assist_normal_ = axisA;
	*assist_normal2_ = axisB;
}

bool CustomAdjustingHandle::fineTuneEnable()
{
	return fine_tune_;
}

AdjDirect CustomAdjustingHandle::curDirect()
{
	return direct_;
}

bool CustomAdjustingHandle::setAdjustAxisSelf(bool _state)
{
	this->adjust_axis_self_ = _state;
	color_ = getColor(direct_);
	return true;
}

bool CustomAdjustingHandle::operatePositiveHandle()
{
	if (direct_ == X_MINUS || direct_ == Y_MINUS || direct_ == Z_MINUS)
	{
		return false;
	}
	else
	{
		return true;
	}
}
