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

#include "translatehandle.h"
#include "meshExt/AbsMesh.h"
#include "util/utility_tools.h"

TranslateHandle::TranslateHandle(AbsMesh *_obj, AdjDirect _direct, float _radius, float _fine_step, bool _b_limit, float* _limit_value)
	:CustomAdjustingHandle(_obj, _direct, _radius, _fine_step)
{
	updateCtrlSys();
	direct_ = _direct;
	b_limit_ = _b_limit;
	limit_value_ = _limit_value;
}

TranslateHandle::TranslateHandle(AbsMesh *_obj, AdjDirect _direct, float _radius, float _coeffient, float _fine_step, bool _b_limit, float* _limit_value)
	: CustomAdjustingHandle(_obj, _direct, _radius, _coeffient, _fine_step)
{
	updateCtrlSys();
	direct_ = _direct;
	b_limit_ = _b_limit;
	limit_value_ = _limit_value;
}

TranslateHandle::TranslateHandle(AbsMesh *_obj, Point3m *_origin, Point3m *_axis, float _radius, float _fine_step, bool _b_limit, float* _limit_value)
	: CustomAdjustingHandle(_obj, _origin, _axis, _radius, _fine_step)
{
	updateCtrlSys();
	b_limit_ = _b_limit;
	limit_value_ = _limit_value;
}

TranslateHandle::~TranslateHandle()
{
}

bool TranslateHandle::drawHandle()
{
	glPushMatrix();
	if (obj_->p_father_trans_matrix_ != nullptr)
	{
		glMultMatrix(*obj_->p_father_trans_matrix_);
	}
	//draw handles
	{
		glPushMatrix();
		glEnable(GL_LIGHTING);
		glColor3f(color_.X(), color_.Y(), color_.Z());
		vcg::Add_Ons::glCone<vcg::Add_Ons::DMSolid>(node_.first, node_.second, CONE_WIDTH);
		glDisable(GL_LIGHTING);
		glPopMatrix();
	}

	//draw stright lines
	{
		float fLineWidth = 2.0f;
		if (fine_tune_)
		{
			fLineWidth = 6.0f;
		}
		paintStraightLineH(node_.first, *origin_, color_, fLineWidth);
	}
	glPopMatrix();

	return true;
}

bool TranslateHandle::pickHandel(int _mouseX, int _mouseY)
{
	Matrix44f adj_matrix;
	if (obj_->p_father_trans_matrix_ != nullptr)
	{
		adj_matrix = *obj_->p_father_trans_matrix_;
	}
	else
	{
		adj_matrix = Matrix44f::Identity();
	}
	glPushMatrix();
	glMultMatrix(adj_matrix);

	bool picked = false;
	int adjParameter;
	std::vector<std::pair<Point3m, Point3m>> nodeList;
	nodeList.push_back(node_);
	picked = UtilityTools::getInstance()->pickConeHandle(_mouseX, _mouseY, adjParameter, nodeList, CONE_WIDTH);
	if (picked)
	{
		glPopMatrix();
		return true;
	}
	glPopMatrix();
	return false;
}

bool TranslateHandle::adjustAction(int _mouseX, int _mouseY)
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
		adj_matrix = Matrix44f::Identity();
	}
	glPushMatrix();
	glMultMatrix(adj_matrix);

	Point3m handleP, projLineV, normalV, normalV2, mixP;
	handleP = adj_matrix * node_.first;
	if (direct_ == X_MINUS || direct_ == Y_MINUS || direct_ == Z_MINUS)
	{
		projLineV = adj_matrix * (-(*axis_vec_));
	}
	else
	{
		projLineV = adj_matrix * (*axis_vec_);
	}
	normalV = adj_matrix * (*assist_normal_);
	normalV2 = adj_matrix * (*assist_normal2_);

	mixP = UtilityTools::getInstance()->getMixedPointFromScreenToWorld2(_mouseX, _mouseY, normalV, normalV2, handleP, nullptr);
	Point3m dragVector = mixP - handleP;
	float actualMoveValue = dragVector * projLineV;

	if (actualMoveValue < 0)
	{
		this->real_time_translate_direct_ = -projLineV;
	}
	else
	{
		this->real_time_translate_direct_ = projLineV;
	}

	if (adjust_axis_self_)
	{
		float range = dragVector * projLineV;

		Point3m aim_center = obj_->localAxis.centerPoint + dragVector * range;
		Point3m cur_vec = aim_center - obj_->p_mesh_->bbox.Center();
		float cur_adj_radius = sqrtf(cur_vec * cur_vec);
		if (cur_adj_radius < adj_enable_radius_)
		{
			carryoutLocalAxisTransformH(obj_->localAxis.centerPoint, obj_->localAxis, range);
		}
		else
		{
			qDebug() << "cur_adj_radius : " << cur_adj_radius << endl;
			qDebug() << "adj_enable_radius : " << adj_enable_radius_ << endl;
		}
		glPopMatrix();
		return true;
	}

	float fmoveResistance, fMaxMoveDegree = 0.2f, fBaseResistanceRatio = 0.5f;
	if (b_limit_ && limit_value_ != nullptr)
	{
		if (actualMoveValue > 0)
		{
			fmoveResistance = fBaseResistanceRatio * fMaxMoveDegree * (1.0f - (*limit_value_ / MAX_OVERLAP_DEPTH));
		}
		else
		{
			fmoveResistance = fBaseResistanceRatio * fMaxMoveDegree * (1.0f - (*limit_value_ / MAX_OVERLAP_DEPTH));
		}
	}
	else
	{
		fmoveResistance = fMaxMoveDegree;
	}

	float range = actualMoveValue * fmoveResistance;
	carroutTransform(range);
	glPopMatrix();
	return true;
}

bool TranslateHandle::adjustFineTuneAction(int _direct)
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

void TranslateHandle::updateCtrlSys()
{
	float beginPos = 1.4f, endPos = 1.6f;
	if (direct_ == X_MINUS || direct_ == Y_MINUS || direct_ == Z_MINUS)
	{
		node_ = std::make_pair(
			*origin_ + (-*axis_vec_) * radius_ * beginPos,
			*origin_ + (-*axis_vec_) * radius_ * endPos);
	}
	else
	{
		node_ = std::make_pair(
			*origin_ + (*axis_vec_) * radius_ * beginPos,
			*origin_ + (*axis_vec_) * radius_ * endPos);
	}
	setLocalAxisAdjRange(0.4f * radius_);
}

void TranslateHandle::carroutTransform(float _value)
{
	vcg::Matrix44f transMat;
	vcg::Matrix44f transform, transformInverse;
	transform.SetIdentity();
	transform.SetTranslate(*origin_);
	transformInverse.SetTranslate(-(*origin_));

	vcg::Matrix44f transformValue, rotMatLocal;
	transMat.SetIdentity();
	rotMatLocal.SetIdentity();
	Point3f trans;

	if (direct_ == X_MINUS || direct_ == Y_MINUS || direct_ == Z_MINUS)
	{
		trans = (-*axis_vec_) * _value;
	}
	else
	{
		trans = (*axis_vec_) * _value;
	}
	transformValue.SetTranslate(trans);
	transMat *= transformValue;

	transform *= transMat * rotMatLocal;
	transform *= transformInverse;

	obj_->transformMatrix_ = transform * obj_->transformMatrix_;
	obj_->updateModelRenderingMatrix();
	obj_->realTimeAxis = obj_->updateRealTimeAxisSys(obj_->transformMatrix_);
	obj_->afterChangeModelMatrix();
}

void TranslateHandle::carryoutLocalAxisTransformH(Point3m origin, Axis curAxis, float _value)
{
	vcg::Matrix44f transMat;
	vcg::Matrix44f transform, transformInverse;
	transform.SetIdentity();
	transform.SetTranslate(origin);
	transformInverse.SetTranslate(-(origin));

	vcg::Matrix44f transformValue, rotMatLocal;
	transMat.SetIdentity();
	rotMatLocal.SetIdentity();

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

	Point3f trans;
	if (curDirect() == X_MINUS || curDirect() == Y_MINUS || curDirect() == Z_MINUS)
	{
		trans = (-axis_vec) * _value;
	}
	else
	{
		trans = (axis_vec) * _value;
	}
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	transform *= transMat * rotMatLocal;
	transform *= transformInverse;

	Point3m center, axisXVert, axisYVert, axisZVert;
	center = obj_->localAxis.centerPoint;
	axisXVert = center +obj_->localAxis.axisXVector;
	axisYVert = center +obj_->localAxis.axisYVector;
	axisZVert = center +obj_->localAxis.axisZVector;
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

void TranslateHandle::setLocalAxisAdjRange(float _range)
{
	adj_enable_radius_ = _range;
}

Point3m TranslateHandle::currentMoveDirect()
{
	return this->real_time_translate_direct_;
}
