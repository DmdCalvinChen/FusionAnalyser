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

#include "ZoomHandle.h"
#include "meshExt/AbsMesh.h"
#include "util/utility_tools.h"

ZoomHandle::ZoomHandle(AbsMesh *_obj, AdjDirect _direct, float _radius, float _base_length, HexaVec *_assist_vec, float _fine_step)
	:CustomAdjustingHandle(_obj, _direct, _radius, _fine_step)
{
	direct_ = _direct;
	base_length_ = _base_length;
	assist_vec_ = _assist_vec;
	switch (direct_)
	{
	case X_PLUS:
	case X_MINUS:
		zoom_value_ = &assist_vec_->fZX;
		adj_move_value = &assist_vec_->fVX;
		break;
	case Y_PLUS:
	case Y_MINUS:
		zoom_value_ = &assist_vec_->fZY;
		adj_move_value = &assist_vec_->fVY;
		break;
	case Z_PLUS:
	case Z_MINUS:
		zoom_value_ = &assist_vec_->fZZ;
		adj_move_value = &assist_vec_->fVZ;
		break;
	default:
		break;
	}
	updateCtrlSys();
}

ZoomHandle::ZoomHandle(AbsMesh *_obj, AdjDirect _direct, float _radius, float _coeffient, float _base_length, HexaVec *_assist_vec, float _fine_step)
	: CustomAdjustingHandle(_obj, _direct, _radius, _coeffient, _fine_step)
{
	direct_ = _direct;
	base_length_ = _base_length;
	assist_vec_ = _assist_vec;
	switch (direct_)
	{
	case X_PLUS:
	case X_MINUS:
		zoom_value_ = &assist_vec_->fZX;
		adj_move_value = &assist_vec_->fVX;
		break;
	case Y_PLUS:
	case Y_MINUS:
		zoom_value_ = &assist_vec_->fZY;
		adj_move_value = &assist_vec_->fVY;
		break;
	case Z_PLUS:
	case Z_MINUS:
		zoom_value_ = &assist_vec_->fZZ;
		adj_move_value = &assist_vec_->fVZ;
		break;
	default:
		break;
	}
	updateCtrlSys();
}

ZoomHandle::~ZoomHandle()
{
}

bool ZoomHandle::drawHandle()
{
	glPushMatrix();
	if (obj_->p_father_trans_matrix_ != nullptr)
	{
		glMultMatrix(*obj_->p_father_trans_matrix_);
	}
	//draw handles|
	{
		glPushMatrix();
		glEnable(GL_LIGHTING);
		glColor3f(color_.X(), color_.Y(), color_.Z());
		vcg::Add_Ons::glPoint< vcg::Add_Ons::DMSolid>(node_, ZOOM_CONE_WIDTH, 16, 16);
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
		paintStraightLineH(node_, *origin_, color_, fLineWidth);
	}
	glPopMatrix();
	return true;
}

bool ZoomHandle::pickHandel(int _mouseX, int _mouseY)
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
	std::vector<Point3m> zoom_node_list;
	zoom_node_list.push_back(node_);
	picked = UtilityTools::getInstance()->pickHandle(_mouseX, _mouseY, adjParameter, zoom_node_list, ZOOM_CONE_WIDTH);
	if (picked)
	{
		glPopMatrix();
		return true;
	}
	glPopMatrix();
	return false;
}

bool ZoomHandle::adjustAction(int _mouseX, int _mouseY)
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
	handleP = node_;
	projLineV = *axis_vec_;
	normalV = *assist_normal_;
	normalV2 = *assist_normal2_;

	mixP = UtilityTools::getInstance()->getMixedPointFromScreenToWorld2(_mouseX, _mouseY, normalV, normalV2, handleP, nullptr);
	Point3m dragVector = mixP - (*origin_);
	float length = abs((dragVector * (*axis_vec_))) - ZOOM_HANDLE_EXTEND_LEN;
	float range = length / base_length_;
	if (range < 0.1f)
	{
		range = 0.1f;
	}
	else if (range > 2.0f)
	{
		range = 2.0f;
	}

	*zoom_value_ = range;
	obj_->transformMatrix_ = obj_->getTransformMatrix(*assist_vec_, obj_->localAxis);
	obj_->updateModelRenderingMatrix();
	obj_->afterChangeModelMatrix();
	updateCtrlSys();

	glPopMatrix();
	return true;
}

bool ZoomHandle::adjustFineTuneAction(int _direct)
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
	float range = (*zoom_value_) + unit;
	if (range < 0.1f)
	{
		range = 0.1f;
	}
	else if(range > 2.0f)
	{
		range = 2.0f;
	}

	float old_zoom_value = *zoom_value_;
	this->obj_->realTimeAxis.centerPoint -= (*this->axis_vec_) * (old_zoom_value - range) * base_length_;
	Point3m move, rotate;
	obj_->localAxis.computeTransformVectors(obj_->realTimeAxis, move, rotate);
	(*assist_vec_).fVX = move.X();
	(*assist_vec_).fVY = move.Y();
	(*assist_vec_).fVZ = move.Z();
	(*assist_vec_).fA = rotate.X();
	(*assist_vec_).fB = rotate.Y();
	(*assist_vec_).fG = rotate.Z();

	*zoom_value_ = range;
	obj_->transformMatrix_ = obj_->getTransformMatrix(*assist_vec_, obj_->localAxis);
	obj_->updateModelRenderingMatrix();
	obj_->afterChangeModelMatrix();
	updateCtrlSys();
	return true;
}

void ZoomHandle::updateCtrlSys()
{
	node_ = (*origin_) + (*axis_vec_) * (base_length_ * (*zoom_value_) + ZOOM_HANDLE_EXTEND_LEN);
}
