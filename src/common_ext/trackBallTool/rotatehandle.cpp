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

#include "rotatehandle.h"
#include "MeshExt/AbsMesh.h"
#include "util/utility_tools.h"


RotateHandle::RotateHandle(AbsMesh *_obj, AdjDirect _direct, float _radius, float _fine_step)
	:CustomAdjustingHandle(_obj, _direct, _radius, _fine_step)
{
	updateCtrlSys();
}

RotateHandle::RotateHandle(AbsMesh *_obj, AdjDirect _direct, float _radius, float _coeffient, float _fine_step)
	: CustomAdjustingHandle(_obj, _direct, _radius, _coeffient, _fine_step)
{
	updateCtrlSys();
}

RotateHandle::RotateHandle(AbsMesh *_obj, Point3m *_origin, Point3m *_axis, float _radius, float _fine_step)
	: CustomAdjustingHandle(_obj, _origin, _axis, _radius, _fine_step)
{
	updateCtrlSys();
}

RotateHandle::~RotateHandle()
{
}

bool RotateHandle::drawHandle()
{
	glPushMatrix();
	if (obj_->p_father_trans_matrix_ != nullptr)
	{
		glMultMatrix(*obj_->p_father_trans_matrix_);
	}
	//draw circles
	{
		float fLineWidth = 2.0f;
		if (fine_tune_)
		{
			fLineWidth = 6.0f;
		}
		paintCircleLinesH(circle_loop_, color_, fLineWidth);
		if (b_draw_normal_plane)
		{
			drawNormalPlane();
		}
	}
	if (b_draw_picked_node_)
	{
		glEnable(GL_LIGHTING);
		glColor3f(color_.X(), color_.Y(), color_.Z());
		vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(picked_node_, 0.5f);
		glDisable(GL_LIGHTING);
	}

	glPopMatrix();
	return true;
}

bool RotateHandle::pickHandel(int _mouseX, int _mouseY)
{
	Matrix44f adj_matrix;
	if (obj_->p_father_trans_matrix_ != nullptr)
	{
		adj_matrix = *obj_->p_father_trans_matrix_;
	}
	else
	{
		adj_matrix = vcg::Matrix44f::Identity();
	}
	glPushMatrix();
	glMultMatrix(adj_matrix);

	bool picked = false;
	int adjParameter;
	picked = UtilityTools::getInstance()->pickOnCircle(_mouseX, _mouseY, *origin_, *axis_vec_, radius_, picked_node_);
	if (picked)
	{
		b_draw_picked_node_ = true;
		glPopMatrix();
		return true;
	}
	b_draw_picked_node_ = false;
	glPopMatrix();
	return false;
}

bool RotateHandle::adjustAction(int _mouseX, int _mouseY)
{
	if (obj_ == nullptr)
	{
		return false;
	}
	Matrix44f adj_matrix;
	if (obj_->p_father_trans_matrix_ != nullptr)
	{
		adj_matrix = *obj_->p_father_trans_matrix_;
	}
	else
	{
		adj_matrix = vcg::Matrix44f::Identity();
	}
	glPushMatrix();
	glMultMatrix(adj_matrix);

	Point3m origin, normalV, mixP, handle;
	handle = picked_node_;
	origin = *origin_;
	normalV = *axis_vec_;
	mixP = UtilityTools::getInstance()->getMixedPointFromScreenToWorld2(_mouseX, _mouseY, normalV, normalV, origin, nullptr);
	if (mixP == origin)
	{
		glPopMatrix();
		return false;
	}

	float angle, tempCosValue;
	{
		angle = 0;
		Point3m v1, v2;
		v1 = handle - origin;
		v2 = mixP - origin;
		v1 = v1.Normalize();
		v2 = v2.Normalize();
		Point3m tempNV = v1 ^ v2;
		tempCosValue = v1 * v2;
		if (abs(tempCosValue - 1) < 1e-5)
		{
			angle = 0;
		}
		else
		{
			angle = acos(tempCosValue);
			angle = float((angle * 180.0f) / PI);
		}
		if (tempNV * normalV < 0)
		{
			angle *= -1;
		}
		angle *= max_adj_coeffient_;
	}
	picked_node_ = origin + (mixP - origin).Normalize() * radius_;

	if (adjust_axis_self_)
	{
		carryoutLocalAxisTransformH(obj_->localAxis.centerPoint, obj_->localAxis, angle);
		glPopMatrix();
		return true;
	}

	carroutTransform(angle);
	glPopMatrix();
	return true;
}

bool RotateHandle::adjustFineTuneAction(int _direct)
{
	if (obj_ == nullptr)
	{
		return false;
	}
	float unit = fine_tune_unit;
	if (_direct < 0)
	{
		unit = -fine_tune_unit;
	}

	carroutTransform(unit);
	return true;
}

void RotateHandle::updateCtrlSys()
{
	Point3m p;
	float costhita = 0.0f;
	float sinthita = 0.0f;
	std::vector<Point3m>().swap(circle_loop_);
	if (this->direct_ == CUSTOM_DIRECT)
	{
		this->updateAssistNormalVector();
	}
	for (int i = 0; i < CIRCLE_DIVIDE_NUMBER; ++i)
	{
		costhita = cos(STEP_RADS * i);
		sinthita = sin(STEP_RADS * i);
		p = (*origin_) + ((*assist_normal_) * costhita + (*assist_normal2_) * sinthita) * radius_;
		this->circle_loop_.push_back(p);
	}
}

void RotateHandle::carroutTransform(float _value)
{
	vcg::Matrix44f transMat;
	vcg::Matrix44f transform, transformInverse;
	transform.SetIdentity();
	transform.SetTranslate(*origin_);
	transformInverse.SetTranslate(-(*origin_));

	vcg::Matrix44f transformValue, rotMatLocal;
	transMat.SetIdentity();
	rotMatLocal.SetIdentity();

	transformValue.SetRotateDeg(_value, *axis_vec_);
	rotMatLocal *= transformValue;

	transform *= transMat * rotMatLocal;
	transform *= transformInverse;

	obj_->transformMatrix_ = transform * obj_->transformMatrix_;
	obj_->updateModelRenderingMatrix();
	obj_->realTimeAxis = obj_->updateRealTimeAxisSys(obj_->transformMatrix_);
	obj_->afterChangeModelMatrix();
}

void RotateHandle::carryoutLocalAxisTransformH(Point3m origin, Axis curAxis, float _value)
{
	Point3m axis_vec;
	switch (this->curDirect())
	{
	case  X_PLUS:
	case X_MINUS:
		axis_vec = curAxis.axisXVector;
		break;

	case Y_PLUS:
	case Y_MINUS:
		axis_vec = curAxis.axisYVector;
		break;

	case Z_PLUS:
	case Z_MINUS:
		axis_vec = curAxis.axisZVector;
		break;

	default:
		break;
	}

	vcg::Matrix44f transMat;
	vcg::Matrix44f transform, transformInverse;
	transform.SetIdentity();
	transform.SetTranslate(origin);
	transformInverse.SetTranslate(-(origin));

	vcg::Matrix44f transformValue, rotMatLocal;
	transMat.SetIdentity();
	rotMatLocal.SetIdentity();

	transformValue.SetRotateDeg(_value, axis_vec);
	rotMatLocal *= transformValue;

	transform *= transMat * rotMatLocal;
	transform *= transformInverse;

	Point3m center, axisXVert, axisYVert, axisZVert;
	center = obj_->localAxis.centerPoint;
	axisXVert = center + obj_->localAxis.axisXVector;
	axisYVert = center + obj_->localAxis.axisYVector;
	axisZVert = center + obj_->localAxis.axisZVector;
	center = transform * center;
	axisXVert = transform * axisXVert;
	axisYVert = transform * axisYVert;
	axisZVert = transform * axisZVert;
	obj_->localAxis.centerPoint = center;
	obj_->localAxis.axisXVector = (axisXVert - center).Normalize();
	obj_->localAxis.axisYVector = (axisYVert - center).Normalize();
	obj_->localAxis.axisZVector = (axisZVert - center).Normalize();

	obj_->updateModelRenderingMatrix();
	obj_->realTimeAxis = obj_->updateRealTimeAxisSys(obj_->transformMatrix_);
	obj_->afterChangeModelMatrix();
}

void RotateHandle::afterMouseRelease()
{
	this->b_draw_picked_node_ = false;
}

void RotateHandle::initalNormalPlaneVerts()
{
	float halfWidth, halfHeight;
	halfWidth = 20.0f;
	halfHeight = 20.0f;
	plane_vert1 = *origin_ +(*assist_normal_) * halfWidth * 2.0f + (*assist_normal2_)  * halfHeight * 2.0f;
	plane_vert2 = *origin_ - (*assist_normal_)* halfWidth * 2.0f + (*assist_normal2_) * halfHeight * 2.0f;
	plane_vert3 = *origin_ - (*assist_normal_)* halfWidth * 2.0f - (*assist_normal2_) * halfHeight * 2.0f;
	plane_vert4 = *origin_ +(*assist_normal_) * halfWidth * 2.0f - (*assist_normal2_) * halfHeight * 2.0f;
}

void RotateHandle::setDrawNormalPlaneEnable(bool _state)
{
	this->b_draw_normal_plane = _state;
	if (_state)
	{
		initalNormalPlaneVerts();
	}
}

void RotateHandle::drawNormalPlane()
{
	initalNormalPlaneVerts();
	glPushMatrix();
	glDisable(GL_LIGHTING);
	//Draw Plane Ctrl Vertex
	glColor3f(0, 1.0f, 0);
	Point3m p1, p2, p3, p4;
	p1 = plane_vert1;
	p2 = plane_vert2;
	p3 = plane_vert3;
	p4 = plane_vert4;

	//Draw Transparent Plane Surface
	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_CULL_FACE);
	glColor4f(0, 1, 0, 0.5f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_TRIANGLES);
	glVertex3f(p1.X(), p1.Y(), p1.Z());
	glVertex3f(p2.X(), p2.Y(), p2.Z());
	glVertex3f(p3.X(), p3.Y(), p3.Z());
	glEnd();
	glDisable(GL_BLEND);
	glColor3f(1, 1, 1);
	glPopMatrix();

	glPushMatrix();
	glColor4f(0, 1, 0, 0.5f);
	glEnable(GL_BLEND);
	glBegin(GL_TRIANGLES);
	glVertex3f(p3.X(), p3.Y(), p3.Z());
	glVertex3f(p4.X(), p4.Y(), p4.Z());
	glVertex3f(p1.X(), p1.Y(), p1.Z());
	glEnd();
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glColor3f(1, 1, 1);
	glPopMatrix();
	glPopAttrib();
	glEnable(GL_LIGHTING);
	glPopMatrix();
}
