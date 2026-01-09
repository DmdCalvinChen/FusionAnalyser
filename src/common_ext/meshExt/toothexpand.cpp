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

#include "Tooth.h"
#include <GL/glew.h>
#include <wrap/gl/addons.h>
#include <wrap/gl/trimesh.h>
#include "util/utility_tools.h"
#include "common/config.h"
#include "common/meshmodel.h"
#include "machine/convexhellmachine.h"
#include "machine/melkman_convexhull2D.h"

void Tooth::updateToothCtrlSys()
{
	updateCtrlSys();
	updateSpecialRotateCircle();
}

void Tooth::updateSpecialRotateCircle()
{
	Point3m p;
	float circleRadius = this->fCtrlRadius_;
	float costhita = 0.0f;
	float sinthita = 0.0f;
	std::vector<Point3m>().swap(this->circle_LeftSide);
	std::vector<Point3m>().swap(this->circle_RightSide);
	std::vector<Point3m>().swap(this->circle_Root);
	for (int i = 0; i < CIRCLE_DIVIDE_NUMBER; ++i)
	{
		costhita = cos(STEP_RADS * i);
		sinthita = sin(STEP_RADS * i);
		p = this->lateralOrigin_Left + (realTimeAxis.axisZVector * costhita + realTimeAxis.axisXVector * sinthita) * circleRadius;
		this->circle_LeftSide.push_back(p);
		p = this->lateralOrigin_Right + (realTimeAxis.axisZVector * costhita + realTimeAxis.axisXVector * sinthita) * circleRadius;
		this->circle_RightSide.push_back(p);
		p = this->rootOrigin + (realTimeRootAxis.axisZVector * costhita + realTimeRootAxis.axisYVector * sinthita) * circleRadius;
		this->circle_Root.push_back(p);
	}
}

void Tooth::updateKeySignVerts(HexaVec _transVec)
{
	vcg::Matrix44f transform = getTransformMatrix(_transVec, localAxis);
	realTimeAxis = updateRealTimeAxisSys(transform);
	updateBoundary(transform);
	updateConvexHull(transform);
}

void Tooth::updateKeySignVerts()
{
	updateBoundary(p_mesh_->Tr);
	updateConvexHull(p_mesh_->Tr);
}

void Tooth::updateBoundary(const vcg::Matrix44f &_matrix)
{
	realtimeBoundary.clear();
	for (auto i : boundaryIndex)
	{
        if(i < p_mesh_->vert.size())
            realtimeBoundary.push_back(_matrix * p_mesh_->vert[i].P());
	}
}

void Tooth::updateConvexHull(const vcg::Matrix44f &_matrix)
{
	for (int i = 0; i < frozenConvexVerts.size(); i++)
	{
		convexVerts[i] = _matrix * frozenConvexVerts[i];
	}
}

void Tooth::drawSpecialCtrlSys()
{
	Point3m v1, v2;
	float line_width;
	if (bFineTuneing)
	{
		line_width = 6.0f;
	}
	else
	{
		line_width = 2.0f;
	}

	if (cur_adj_mode_ == TOOTH_LEFT_LATERALSIDE)
	{
		v1 = lateralOrigin_Left + realTimeAxis.axisYVector * 5.0f;
		v2 = lateralOrigin_Left - realTimeAxis.axisYVector * 5.0f;
		paintStraightLine(v1, v2, Point3m(1, 1, 1), 2.0f);
		paintCircleLines(circle_LeftSide, Point3m(0.58f, 0.54f, 0.52f), line_width);
	}
	if (cur_adj_mode_ == TOOTH_RIGHT_LATERALSIDE)
	{
		v1 = lateralOrigin_Right + realTimeAxis.axisYVector * 5.0f;
		v2 = lateralOrigin_Right - realTimeAxis.axisYVector * 5.0f;
		paintStraightLine(v1, v2, Point3m(1, 1, 1), 2.0f);
		paintCircleLines(circle_RightSide, Point3m(0.58f, 0.54f, 0.52f), line_width);
	}
	if (cur_adj_mode_ == TOOTH_ROOT)
	{
		v1 = rootOrigin + realTimeRootAxis.axisXVector * 5.0f;
		v2 = rootOrigin - realTimeRootAxis.axisXVector * 5.0f;
		paintStraightLine(v1, v2, Point3m(1, 1, 1), 2.0f);
		paintCircleLines(circle_Root, Point3m(0.58f, 0.54f, 0.52f), line_width);
	}
}

bool Tooth::pickSpecialCtrlSys(int x, int y)
{
	bool bPickOneHandle = false;
	switch (cur_adj_mode_)
	{
	case TOOTH_LEFT_LATERALSIDE:
		bPickOneHandle = UtilityTools::getInstance()->pickOnCircle(x, y, lateralOrigin_Left, realTimeAxis.axisYVector, fCtrlRadius_, rotateCircleHandle_);
		break;

	case TOOTH_RIGHT_LATERALSIDE:
		bPickOneHandle = UtilityTools::getInstance()->pickOnCircle(x, y, lateralOrigin_Right, realTimeAxis.axisYVector, fCtrlRadius_, rotateCircleHandle_);
		break;

	case TOOTH_ROOT:
		bPickOneHandle = UtilityTools::getInstance()->pickOnCircle(x, y, rootOrigin, realTimeRootAxis.axisXVector, fCtrlRadius_, rotateCircleHandle_);
		break;
	}
	return bPickOneHandle;
}

void Tooth::drawAdjLocalAxisCtrlSys()
{
	float fXLineWidth = 2.0f, fYLineWidth = 2.0f, fZLineWidth = 2.0f;
	Point3m x_color(1, 0.07, 0.57), y_color(0, 0, 0.7), z_color(0.7, 0.7, 0);
	// Draw handles
	{
		glPushMatrix();
		glEnable(GL_LIGHTING);
		glColor3f(x_color.X(), x_color.Y(), x_color.Z());
		vcg::Add_Ons::glCone<vcg::Add_Ons::DMSolid>(moveCtrlNodesPair_[X_PLUS_PART].first, moveCtrlNodesPair_[X_PLUS_PART].second, CONE_WIDTH);
		vcg::Add_Ons::glCone<vcg::Add_Ons::DMSolid>(moveCtrlNodesPair_[X_MINUS_PART].first, moveCtrlNodesPair_[X_MINUS_PART].second, CONE_WIDTH);
		glColor3f(y_color.X(), y_color.Y(), y_color.Z());
		vcg::Add_Ons::glCone<vcg::Add_Ons::DMSolid>(moveCtrlNodesPair_[Y_PLUS_PART].first, moveCtrlNodesPair_[Y_PLUS_PART].second, CONE_WIDTH);
		vcg::Add_Ons::glCone<vcg::Add_Ons::DMSolid>(moveCtrlNodesPair_[Y_MINUS_PART].first, moveCtrlNodesPair_[Y_MINUS_PART].second, CONE_WIDTH);
		glColor3f(z_color.X(), z_color.Y(), z_color.Z());
		vcg::Add_Ons::glCone<vcg::Add_Ons::DMSolid>(moveCtrlNodesPair_[Z_PLUS_PART].first, moveCtrlNodesPair_[Z_PLUS_PART].second, CONE_WIDTH);
		vcg::Add_Ons::glCone<vcg::Add_Ons::DMSolid>(moveCtrlNodesPair_[Z_MINUS_PART].first, moveCtrlNodesPair_[Z_MINUS_PART].second, CONE_WIDTH);
		glDisable(GL_LIGHTING);
		glPopMatrix();
	}
	// Draw straight lines
	{
		paintStraightLine(moveCtrlNodesPair_[X_PLUS_PART].first, moveCtrlNodesPair_[X_MINUS_PART].first, x_color, fXLineWidth);
		paintStraightLine(moveCtrlNodesPair_[Y_PLUS_PART].first, moveCtrlNodesPair_[Y_MINUS_PART].first, y_color, fYLineWidth);
		paintStraightLine(moveCtrlNodesPair_[Z_PLUS_PART].first, moveCtrlNodesPair_[Z_MINUS_PART].first, z_color, fZLineWidth);
	}
	// Draw circles
	{
		paintCircleLines(circleYOZ, x_color, fXLineWidth);
		paintCircleLines(circleZOX, y_color, fYLineWidth);
		paintCircleLines(circleXOY, z_color, fZLineWidth);
	}
}

void Tooth::setFineTuneState(bool _state)
{
	this->bFineTuneing = _state;
}

void Tooth::toothAdjLocalAxisAction(int _mouseX, int _mouseY)
{
	if (cur_adj_mode_ == ABS_ADJ_NONE)
	{
		return;
	}

	if (cur_adj_mode_ == ABS_ADJ_TRANSLATE)
	{
		Point3m handleP, projLineV, normalV, normalV2, mixP;
		handleP = moveCtrlNodesPair_[cur_adj_direct_].first;
		switch (cur_adj_direct_)
		{
		case X_PLUS_PART:
			projLineV = realTimeAxis.axisXVector;
			normalV = realTimeAxis.axisYVector;
			normalV2 = realTimeAxis.axisZVector;
			break;
		case X_MINUS_PART:
			projLineV = -realTimeAxis.axisXVector;
			normalV = realTimeAxis.axisYVector;
			normalV2 = realTimeAxis.axisZVector;
			break;
		case Y_PLUS_PART:
			projLineV = realTimeAxis.axisYVector;
			normalV = realTimeAxis.axisXVector;
			normalV2 = realTimeAxis.axisZVector;
			break;
		case Y_MINUS_PART:
			projLineV = -realTimeAxis.axisYVector;
			normalV = realTimeAxis.axisXVector;
			normalV2 = realTimeAxis.axisZVector;
			break;
		case Z_PLUS_PART:
			projLineV = realTimeAxis.axisZVector;
			normalV = realTimeAxis.axisYVector;
			normalV2 = realTimeAxis.axisXVector;
			break;
		case Z_MINUS_PART:
			projLineV = -realTimeAxis.axisZVector;
			normalV = realTimeAxis.axisYVector;
			normalV2 = realTimeAxis.axisXVector;
			break;
		default:
			break;
		}
		mixP = UtilityTools::getInstance()->getMixedPointFromScreenToWorld2(_mouseX, _mouseY, normalV, normalV2, handleP, nullptr);
		Point3m dragVector = mixP - handleP;
		float range = dragVector * projLineV;
		switch (cur_adj_direct_)
		{
		case X_PLUS_PART:
			moveVector = Point3m(range, 0, 0);
			break;
		case X_MINUS_PART:
			moveVector = Point3m(-range, 0, 0);
			break;
		case Y_PLUS_PART:
			moveVector = Point3m(0, range, 0);
			break;
		case Y_MINUS_PART:
			moveVector = Point3m(0, -range, 0);
			break;
		case Z_PLUS_PART:
			moveVector = Point3m(0, 0, range);
			break;
		case Z_MINUS_PART:
			moveVector = Point3m(0, 0, -range);
			break;
		default:
			break;
		}
		carryoutLocalAxisTransform(localAxis.centerPoint, localAxis);
	}
	else if (cur_adj_mode_ == ABS_ADJ_ROTATE)
	{
		Point3m origin, normalV, mixP, handle;
		handle = rotateCircleHandle_;
		origin = realTimeAxis.centerPoint;
		switch (cur_adj_direct_)
		{
		case X_PLUS_PART:
		case X_MINUS_PART:
			normalV = realTimeAxis.axisXVector;
			break;
		case Y_PLUS_PART:
		case Y_MINUS_PART:
			normalV = realTimeAxis.axisYVector;
			break;
		case Z_PLUS_PART:
		case Z_MINUS_PART:
			normalV = realTimeAxis.axisZVector;
			break;
		default:
			break;
		}
		mixP = UtilityTools::getInstance()->getMixedPointFromScreenToWorld2(_mouseX, _mouseY, normalV, normalV, origin, nullptr);
		if (mixP == origin)
		{
			return;
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
				angle *= -1;

			angle *= 0.2f;
		}

		switch (cur_adj_direct_)
		{
		case X_PLUS_PART:
		case X_MINUS_PART:
			rotateVector = Point3m(angle, 0, 0);
			break;

		case Y_PLUS_PART:
		case Y_MINUS_PART:
			rotateVector = Point3m(0, angle, 0);
			break;

		case Z_PLUS_PART:
		case Z_MINUS_PART:
			rotateVector = Point3m(0, 0, angle);
			break;

		default:
			break;
		}
		rotateCircleHandle_ = origin + (mixP - origin).Normalize() * fCtrlRadius_;
		carryoutLocalAxisTransform(localAxis.centerPoint, localAxis);
	}
}

void Tooth::carryoutLocalAxisTransform(Point3m origin, Axis curAxis)
{
	vcg::Matrix44f transMat;
	vcg::Matrix44f transform, transformInverse;
	transform.SetIdentity();
	transform.SetTranslate(origin);
	transformInverse.SetTranslate(-origin);
	vcg::Matrix44f transformValue, rotMatLocal;
	transMat.SetIdentity();
	rotMatLocal.SetIdentity();
	Point3f trans;
	trans = curAxis.axisXVector * moveVector.X();
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	trans = curAxis.axisYVector * moveVector.Y();
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	trans = curAxis.axisZVector * moveVector.Z();
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	transformValue.SetRotateDeg(rotateVector.X(), curAxis.axisXVector);
	rotMatLocal *= transformValue;
	transformValue.SetRotateDeg(rotateVector.Y(), curAxis.axisYVector);
	rotMatLocal *= transformValue;
	transformValue.SetRotateDeg(rotateVector.Z(), curAxis.axisZVector);
	rotMatLocal *= transformValue;
	transform *= transMat * rotMatLocal;
	transform *= transformInverse;

	Point3m center, axisXVert, axisYVert, axisZVert;
	center = this->localAxis.centerPoint;
	axisXVert = center + this->localAxis.axisXVector;
	axisYVert = center + this->localAxis.axisYVector;
	axisZVert = center + this->localAxis.axisZVector;
	center = transform * center;
	axisXVert = transform * axisXVert;
	axisYVert = transform * axisYVert;
	axisZVert = transform * axisZVert;
	this->localAxis.centerPoint = center;
	this->localAxis.axisXVector = (axisXVert - center).Normalize();
	this->localAxis.axisYVector = (axisYVert - center).Normalize();
	this->localAxis.axisZVector = (axisZVert - center).Normalize();

	updateModelRenderingMatrix();
	realTimeAxis = updateRealTimeAxisSys(transformMatrix_);
	this->afterChangeModelMatrix();

	moveVector = Point3m(0, 0, 0);
	rotateVector = Point3m(0, 0, 0);
}

void Tooth::carroutTransform(Point3m origin, Axis curAxis)
{
	vcg::Matrix44f transMat;
	vcg::Matrix44f transform, transformInverse;
	transform.SetIdentity();
	transform.SetTranslate(origin);
	transformInverse.SetTranslate(-origin);

	vcg::Matrix44f transformValue, rotMatLocal;
	transMat.SetIdentity();
	rotMatLocal.SetIdentity();
	Point3f trans;

	trans = curAxis.axisXVector * moveVector.X();
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	trans = curAxis.axisYVector * moveVector.Y();
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	trans = curAxis.axisZVector * moveVector.Z();
	transformValue.SetTranslate(trans);
	transMat *= transformValue;

	transformValue.SetRotateDeg(rotateVector.X(), curAxis.axisXVector);
	rotMatLocal *= transformValue;
	transformValue.SetRotateDeg(rotateVector.Y(), curAxis.axisYVector);
	rotMatLocal *= transformValue;
	transformValue.SetRotateDeg(rotateVector.Z(), curAxis.axisZVector);
	rotMatLocal *= transformValue;

	transform *= transMat * rotMatLocal;
	transform *= transformInverse;

	transformMatrix_ = transform * transformMatrix_;
	updateModelRenderingMatrix();
	realTimeAxis = updateRealTimeAxisSys(transformMatrix_);
	this->afterChangeModelMatrix();

	moveVector = Point3m(0, 0, 0);
	rotateVector = Point3m(0, 0, 0);
}

void Tooth::carryoutTransform(HexaVec _vec)
{
	this->trans_vec_ = _vec;
	transformMatrix_ = getTransformMatrix(_vec, localAxis);
	updateModelRenderingMatrix();
	realTimeAxis = updateRealTimeAxisSys(this->transformMatrix_);
	updateRealTimeSpecialOrigins(this->transformMatrix_);
	updateToothCtrlSys();
	updateKeySignVerts();

	Point3m moveRecord(0, 0, 0), rotateRecord(0, 0, 0);
	this->AimHexaVec = computeTransformVectors(moveRecord, rotateRecord);
	this->AimHexaVec_Root = computeTransformVectors_Root(moveRecord, rotateRecord);
	this->updateDisplayedParameters();
}

void Tooth::afterChangeModelMatrix()
{
	updateRealTimeSpecialOrigins(transformMatrix_);
	updateToothCtrlSys();
	updateKeySignVerts();

	Point3m moveRecord(0, 0, 0), rotateRecord(0, 0, 0);
	this->AimHexaVec = computeTransformVectors(moveRecord, rotateRecord);
	this->AimHexaVec_Root = computeTransformVectors_Root(moveRecord, rotateRecord);
	this->updateDisplayedParameters();
}

void Tooth::sortObject(Point3m _camera_pos, bool _b_pulled_compared, vector<MeshModel*> &_model_list)
{
	if (!bPulled_out_ && !this->children_.empty())
	{
		for (int i = 0; i < children_.size(); ++i)
		{
			_model_list.push_back(children_[i]->p_model_);
		}
	}
	if (!_b_pulled_compared && this->compare_obj_ != nullptr)
	{
		_model_list.push_back(compare_obj_->p_model_);
	}
	if (!bPulled_out_)
	{
		_model_list.push_back(this->p_model_);
	}
}

void Tooth::sortComparableObject(Point3m _camera_pos, vector<MeshModel*>& _model_list)
{
	if (this->compare_obj2_ != nullptr)
	{
		_model_list.push_back(compare_obj2_->p_model_);
	}
}

void Tooth::setAtrificial(ArtificialToothRecord *_record)
{

}

void Tooth::setPulledOut(bool _state)
{
	this->bPulled_out_ = _state;
	if (_state)
	{
		this->setVisible(false);
	}
	else
	{
		this->setVisible(true);
	}
}

void Tooth::setVisible(bool _visible)
{
	if (p_model_ == nullptr)
	{
		return;
	}
	p_model_->visible = _visible;
	if (!children_.empty())
	{
		for (auto &child : children_)
		{
			if (child->p_model_->upperOrLowerToothModelMark != UpperSingleRoot &&
				child->p_model_->upperOrLowerToothModelMark != LowerSingleRoot)
			{
				child->setVisible(_visible);
			}
		}
	}
}

void Tooth::computeArtificialData(float cur_interval_length, FEdge cur_crevice_edge, Tooth *tooth_a, Tooth *tooth_b)
{
	if (!tooth_a || !tooth_b)
	{
		return;
	}
	Point3m new_axis_x, new_axis_y, new_axis_z, new_axis_center;
	// Use midpoint of distance measurement line between two teeth for positioning
	new_axis_center = (cur_crevice_edge.vertA + cur_crevice_edge.vertB) / 2.0f;
	new_axis_z = ((tooth_a->realTimeAxis.axisZVector + tooth_b->realTimeAxis.axisZVector) / 2.0f).Normalize();
	new_axis_y = ((tooth_a->realTimeAxis.axisYVector + tooth_b->realTimeAxis.axisYVector) / 2.0f).Normalize();
	new_axis_x = (new_axis_y ^ new_axis_z).Normalize();

	this->realTimeAxis = Axis(new_axis_center, new_axis_x, new_axis_y, new_axis_z);

	float primitive_width = this->localBoundbox.fWidth;
	this->trans_vec_.fZX = cur_interval_length / primitive_width;
}

void Tooth::initialWidthDefinitionPoint()
{
	Point3m a, b, c, plane_normal;
	float half_length = localBoundbox.fLength * 0.5f;
	c = getDirectionalPeak(Point3m(0, 0, 0), localAxis.axisYVector, Point3m(0, 0, 0), false);
	a = c + realTimeAxis.axisXVector * half_length;
	b = c - realTimeAxis.axisXVector * half_length;
	setWidthDefinitionPoint(a, b, c);
}

void Tooth::setWidthDefinitionPoint(Point3m _a, Point3m _b, Point3m _c)
{
	this->width_definition_point_a_ = _a;
	this->width_definition_point_b_ = _b;
	this->width_definition_point_c_ = _c;

	Point3m vec = _a - _b;
	float width = sqrtf(vec * vec);
	int temp = (width * 100) / (int)1;
	width = temp / 100.0f;
	this->fToothWidth = width;
}

void Tooth::eliminateOuterCircleVerts(vector<std::pair<int, Point2m>>& _result, CMeshO* _curMesh, Axis& _axis, float& _reduction)
{
	// Find boundary vertex points
	vector<int> area_verts;
	for (auto& r : _result)
	{
		area_verts.push_back(r.first);
	}

	vector<Point3m> boundary;
	vector<Point2m> boundary2d;
	vector<std::pair<int, Point2m>>::iterator ite = _result.begin();
	while (ite != _result.end())
	{
		bool is_outer_vert = false;
		vector<int> neighbor_verts = UtilityTools::getInstance()->oneRingNeighborhoodVV(ite->first, _curMesh);
		for (auto& index : neighbor_verts)
		{
			if (find(area_verts.begin(), area_verts.end(), index) == area_verts.end())
			{
				is_outer_vert = true;
				break;
			}
		}

		if (is_outer_vert)
		{
			boundary2d.push_back(ite->second);
		}
		++ite;
	}

	MelkmanConvexhull2D melkman_convexhull;
	boundary2d = melkman_convexhull.getResults(boundary2d);
	for (auto& vert : boundary2d)
	{
		boundary.push_back(Point3m(vert.X(), vert.Y(), 0));
	}

	// Sort boundary
	set<pair<float, int>> sorted_boundary;
	int i = 0;
	for (auto& vert : boundary)
	{
		Point3m vec = vert;
		vec = vec.Normalize();
		float degree = acos(vec.X());
		if (vec.Y() < 0)
		{
			degree = 2 * PI - degree;
		}
		sorted_boundary.insert(make_pair(degree, i));
		++i;
	}

	// Get reduced boundary points
	float reduce_value = _reduction; // mm
	vector<Point3m> reduce_boundary;
	for (auto& data : sorted_boundary)
	{
		int ivert = data.second;
		Point3m vec = -boundary[ivert];
		vec = vec.Normalize();

		Point3m reduce_vert = boundary[ivert] + vec * reduce_value;
		reduce_boundary.push_back(reduce_vert);
	}

	// Fit reduced boundary
	vector<Point3m> fitting_ctrls = reduce_boundary;
	if (fitting_ctrls.empty()) return;
	fitting_ctrls.push_back(fitting_ctrls.front());
	vector<Point3m> fitting_reduce_boundary = UtilityTools::getInstance()->catmull_Rom_SplinePoints_new(fitting_ctrls);

	whitening_boundary_.clear();
	for (auto& v : fitting_reduce_boundary)
	{
		Point3m vert = _axis.centerPoint + _axis.axisXVector * v.X() + _axis.axisYVector * v.Y();
		Point3m dir = -_axis.axisZVector;
		pair<bool, Point3m>  intersection = UtilityTools::getInstance()->intersectMeshAndGetNearestIntersection(vert, dir, _curMesh);
		if (intersection.first)
		{
			whitening_boundary_.push_back(intersection.second);
		}
		else
		{
			whitening_boundary_.push_back(vert);
		}
	}

	// Delete outer vertices
	ite = _result.begin();
	while (ite != _result.end())
	{
		Point3m vert = Point3m(ite->second.X(), ite->second.Y(), 0);
		bool is_outer_vert = UtilityTools::getInstance()->vertOutOfPolygon(vert, fitting_reduce_boundary);

		if (is_outer_vert)
		{
			_result.erase(ite);
		}
		else
		{
			++ite;
		}
	}
}
