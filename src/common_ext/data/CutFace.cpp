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

#include "CutFace.h"
#include "GL/glew.h"

#include <QDebug>

float CutFace::DEFAULT_CUTPLANE_RADIUS_MIN = 5.0f;
float CutFace::DEFAULT_CUTPLANE_RADIUS_MAX = 30.0f;
Point3m rotate_handle_color(84 / 255.0f, 195 / 255.0f, 241 / 255.0f);

CutFace::CutFace()
{
	boundary_color_ = Point4m(0.00784f, 0.41176f, 0.298039f, 1);
	face_color_ = Point4m(0.55f, 0.788f, 0.32f, 0.5f);
}

CutFace::CutFace(Point3m _basalP, Point3m _normalV, Point3m _axisXV, Point3m _axisYV)
{
	boundary_color_ = Point4m(0.00784f, 0.41176f, 0.298039f, 1);
	face_color_ = Point4m(0.55f, 0.788f, 0.32f, 0.5f);
    initFace(_basalP, _normalV, _axisXV, _axisYV);
}

void CutFace::initFace(Point3m &_basalP, Point3m &_normalV, Point3m &_axisXV, Point3m &_axisYV)
{
	this->axis_ = Axis(_basalP, _axisXV, _axisYV, _normalV);
	this->fRadius_ = DEFAULT_CUTPLANE_RADIUS_MAX;
	this->fRadius_CtrlNode = CTRLNODE_RADIUS_MAX;
	updateCutFace();
}

void CutFace::initFace2(Point3m& _basalP, Point3m& _normalV, Point3m& _axisXV, Point3m& _axisYV, float _radius)
{
	this->axis_ = Axis(_basalP, _axisXV, _axisYV, _normalV);
	this->fRadius_ = _radius;
	this->fRadius_CtrlNode = CTRLNODE_RADIUS_MAX;
	updateCutFace();
}

void CutFace::updateCutFace()
{
	std::vector<Point3m>().swap(boundaryVerts);
	Point3m p;
	float costhita = 0.0f;
	float sinthita = 0.0f;
	Point3m currentCenter = axis_.centerPoint;
	for (int i = 0; i < CIRCLE_DIVIDE_NUMBER; ++i)
	{
		costhita = cos(STEP_RADS * i);
		sinthita = sin(STEP_RADS * i);
		p = currentCenter + (axis_.axisXVector * costhita + axis_.axisYVector * sinthita) * fRadius_;
		this->boundaryVerts.push_back(p);
	}
}

void CutFace::draw(int _screen_wid, int _screen_hei)
{
	drawCtrlNodes(_screen_wid, _screen_hei);
	drawCutFace();
}

void CutFace::drawCtrlNodeAdjustingDirection(Point3m _node, float _radius, std::pair<bool, bool>& _left_right_enable, const Axis &_axis, const Point3m& _camPos, const Point3m& _viewDir)
{
	int numOfTime = 15;
	float step_rotated = PI / 180.f;
	Point3m origin = _axis.centerPoint;
	Point3m axis_x = (_node - origin).Normalize();
	Point3m axis_y = _axis.axisXVector;

	Point3m rotate_axis = (axis_x ^ axis_y).Normalize();

	bool b_left_enable = true, b_right_enable = true;
	Point3m cameraVec = (_camPos - axis_.centerPoint).Normalize();
	if (abs(cameraVec * axis_x) <= CTRLSYS_ROTATION_INCLINATION && abs(rotate_axis * _axis.axisYVector) > 0.5f)
	{
		if (cameraVec * axis_y > 0)
		{
			b_left_enable = false;
		}
		else if (cameraVec * axis_y < 0)
		{
			b_right_enable = false;
		}
	}
	_left_right_enable = make_pair(b_left_enable, b_right_enable);

	vector<Point3m> arc_nodes_left;
	for (int i = 0; i < numOfTime; ++i)
	{
		float degree = -i * step_rotated;
		arc_nodes_left.push_back(origin + (axis_x * cos(degree) + axis_y * sin(degree)).Normalize() * _radius);
	}

	vector<Point3m> arc_nodes_right;
	for (int i = 0; i < numOfTime; ++i)
	{
		float degree = i * step_rotated;
		arc_nodes_right.push_back(origin + (axis_x * cos(degree) + axis_y * sin(degree)).Normalize() * _radius);
	}

	Point3m vec_ab;
	float arrow_degree = -PI / 6.0f;
	float arrow_len = 0.5f;
	Point3m arrow_a_node0, arrow_a_node1, arrow_a_node2;
	vec_ab = (arc_nodes_left[numOfTime - 1] - arc_nodes_left[numOfTime -2]).Normalize();
	axis_y = (vec_ab ^ _viewDir).Normalize();
	arrow_a_node2 = arc_nodes_left.back();
	arrow_a_node0 = arrow_a_node2 - (vec_ab * cos(arrow_degree ) + axis_y * sin(arrow_degree )).Normalize() * arrow_len;
	arrow_a_node1 = arrow_a_node2 - (vec_ab * cos(arrow_degree ) - axis_y * sin(arrow_degree )).Normalize() * arrow_len;

	Point3m arrow_b_node0, arrow_b_node1, arrow_b_node2;
	vec_ab = (arc_nodes_right[numOfTime - 1] - arc_nodes_right[numOfTime - 2]).Normalize();
	axis_y = (vec_ab ^ _viewDir).Normalize();
	arrow_b_node2 = arc_nodes_right.back();
	arrow_b_node0 = arrow_b_node2 - (vec_ab * cos(arrow_degree ) + axis_y * sin(arrow_degree )).Normalize() * arrow_len;
	arrow_b_node1 = arrow_b_node2 - (vec_ab * cos(arrow_degree ) - axis_y * sin(arrow_degree )).Normalize() * arrow_len;

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glLineWidth(2.0f);
	glColor3f(rotate_handle_color.X(), rotate_handle_color.Y(), rotate_handle_color.Z());
	if (b_left_enable)
	{
		glBegin(GL_LINES);
		for (int i = 0; i < numOfTime - 1; ++i)
		{
			glVertex3f(arc_nodes_left[i].X(), arc_nodes_left[i].Y(), arc_nodes_left[i].Z());
			glVertex3f(arc_nodes_left[i + 1].X(), arc_nodes_left[i + 1].Y(), arc_nodes_left[i + 1].Z());
		}
		glEnd();
	}

	if (b_right_enable)
	{
		glBegin(GL_LINES);
		for (int i = 0; i < numOfTime - 1; ++i)
		{
			glVertex3f(arc_nodes_right[i].X(), arc_nodes_right[i].Y(), arc_nodes_right[i].Z());
			glVertex3f(arc_nodes_right[i + 1].X(), arc_nodes_right[i + 1].Y(), arc_nodes_right[i + 1].Z());
		}
		glEnd();
	}

	glLineWidth(1.0f);
	if (b_left_enable)
	{
		glBegin(GL_TRIANGLES);
		glVertex3f(arrow_a_node0.X(), arrow_a_node0.Y(), arrow_a_node0.Z());
		glVertex3f(arrow_a_node1.X(), arrow_a_node1.Y(), arrow_a_node1.Z());
		glVertex3f(arrow_a_node2.X(), arrow_a_node2.Y(), arrow_a_node2.Z());
		glEnd();
	}

	if (b_right_enable)
	{
		glBegin(GL_TRIANGLES);
		glVertex3f(arrow_b_node1.X(), arrow_b_node1.Y(), arrow_b_node1.Z());
		glVertex3f(arrow_b_node2.X(), arrow_b_node2.Y(), arrow_b_node2.Z());
		glVertex3f(arrow_b_node0.X(), arrow_b_node0.Y(), arrow_b_node0.Z());
		glEnd();
	}

	glLineWidth(1.0f);
	glColor3f(1, 1, 1);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glPopAttrib();
}

void CutFace::drawCtrlNodes(int _screen_wid, int _screen_hei)
{
	if (this->ctrlPoints.empty())
	{
		return;
	}

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();
	glEnable(GL_LIGHTING);
	glDepthFunc(GL_ALWAYS);
	glColor3f(1,0,0);
	glBlendFunc(GL_ONE, GL_ZERO);
	vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(ctrlPoints[TRANSLATE_CTRL_NODE], fRadius_CtrlNode, CTRLNODE_SLICE, CTRLNODE_STACK);
	glDepthFunc(GL_LESS);
	glDisable(GL_LIGHTING);
	glPopMatrix();
	glPopAttrib();

	if (_screen_wid > 0 && _screen_hei > 0)
	{
		Point3m camPos, viewDir;
		UtilityTools::getInstance()->getCurrentCameraState(nullptr, _screen_wid, _screen_hei, viewDir, camPos);
		drawCtrlNodeAdjustingDirection(ctrlPoints[ROTATE_CTRL_NODE_1], this->fRadius_, leftAndRightAdjEnable_[ROTATE_CTRL_NODE_1], this->ctrl_axis_, camPos, viewDir);
		drawCtrlNodeAdjustingDirection(ctrlPoints[ROTATE_CTRL_NODE_2], this->fRadius_, leftAndRightAdjEnable_[ROTATE_CTRL_NODE_2], this->ctrl_axis_, camPos, viewDir);
		drawCtrlNodeAdjustingDirection(ctrlPoints[ROTATE_CTRL_NODE_3], this->fRadius_, leftAndRightAdjEnable_[ROTATE_CTRL_NODE_3], this->ctrl_axis_, camPos, viewDir);
	}

	glPushMatrix();
	glEnable(GL_LIGHTING);
	glColor3f(rotate_handle_color.X(), rotate_handle_color.Y(), rotate_handle_color.Z());
	vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(ctrlPoints[ROTATE_CTRL_NODE_1], fRadius_CtrlNode, CTRLNODE_SLICE, CTRLNODE_STACK);
	vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(ctrlPoints[ROTATE_CTRL_NODE_2], fRadius_CtrlNode, CTRLNODE_SLICE, CTRLNODE_STACK);
	vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(ctrlPoints[ROTATE_CTRL_NODE_3], fRadius_CtrlNode, CTRLNODE_SLICE, CTRLNODE_STACK);
	glDisable(GL_LIGHTING);
	glPopMatrix();
}

void CutFace::drawCutFace()
{
	if (boundaryVerts.empty())
	{
		return;
	}
	Point3m p1, p2, p3;

	Point3m vert = *this->boundaryVerts.begin();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
	glLineWidth(2.0f);
	glColor3f(boundary_color_.X(), boundary_color_.Y(), boundary_color_.Z());
	glBegin(GL_LINE_STRIP);
	for (auto v : this->boundaryVerts)
	{
		glVertex3f(v.X(), v.Y(), v.Z());
	}
	glVertex3f(vert.X(), vert.Y(), vert.Z());
	glEnd();
	glColor3f(1.0f, 1.0f, 1.0f);
	glLineWidth(1.0f);
	glEnable(GL_LIGHTING);
	glPopMatrix();
	glPopAttrib();

	if (!cut_profile_.empty())
	{
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushMatrix();
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0, 1);
		glDisable(GL_LIGHTING);
		glEnable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
		glLineWidth(2.0f);
		glColor3f(1, 0, 0);
		for (auto& edge : cut_profile_)
		{
			glBegin(GL_LINES);
			glVertex3f(edge.vertA.X(), edge.vertA.Y(), edge.vertA.Z());
			glVertex3f(edge.vertB.X(), edge.vertB.Y(), edge.vertB.Z());
			glEnd();
		}
		glColor3f(1.0f, 1.0f, 1.0f);
		glLineWidth(1.0f);
		glEnable(GL_LIGHTING);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glEnable(GL_DEPTH_TEST);
		glPopMatrix();
		glPopAttrib();
	}

	glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0, 1);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glColor4f(face_color_.X(), face_color_.Y(), face_color_.Z(), face_color_.W());
    glBegin(GL_TRIANGLES);
	for (int i = 0; i < boundaryVerts.size() - 1; ++i)
	{
		p1 = axis_.centerPoint;
		p2 = boundaryVerts[i];
		p3 = boundaryVerts[i + 1];
		glVertex3f(p1.X(), p1.Y(), p1.Z());
		glVertex3f(p2.X(), p2.Y(), p2.Z());
		glVertex3f(p3.X(), p3.Y(), p3.Z());
	}
	p1 = axis_.centerPoint;
	p2 = boundaryVerts.back();
	p3 = boundaryVerts.front();
	glVertex3f(p1.X(), p1.Y(), p1.Z());
	glVertex3f(p2.X(), p2.Y(), p2.Z());
	glVertex3f(p3.X(), p3.Y(), p3.Z());
    glEnd();
	glEnable(GL_LIGHTING);
	glDisable(GL_POLYGON_OFFSET_FILL);
    glPopMatrix();
	glPopAttrib();
}

void CutFace::updateCtrlSystem(Point3m _cameraPos, Point3m _viewDirect)
{
	std::vector<Point3m>().swap(ctrlPoints);

	Point3m tempVec = (_cameraPos - axis_.centerPoint).Normalize();
	Point3m curVect = _cameraPos - axis_.centerPoint;
	float curDis = sqrtf(curVect * curVect);
	float t = (curDis - 0.1f) / (40.0f - 0.1f);
	t = t > 1.0f ? 1.0f : t;
	t = t < 0.1f ? 0.1f : t;
	this->fRadius_ = DEFAULT_CUTPLANE_RADIUS_MAX * t + DEFAULT_CUTPLANE_RADIUS_MIN * (1 - t);
	this->fRadius_CtrlNode = CTRLNODE_RADIUS_MAX * t + CTRLNODE_RADIUS_MIN * (1 - t);
	updateCutFace();

	if (tempVec * _viewDirect >= 0)
	{
		return;
	}

	Point3m signVert = UtilityTools::getInstance()->getProjPointOnPlane(_cameraPos, axis_.centerPoint, axis_.axisZVector);
	Point3m ctrl_axisX = axis_.axisZVector;
	Point3m ctrl_axisZ = (signVert - axis_.centerPoint).Normalize();
	Point3m ctrl_axisY = (ctrl_axisZ ^ ctrl_axisX).Normalize();
	this->ctrl_axis_ = Axis(axis_.centerPoint, ctrl_axisX, ctrl_axisY, ctrl_axisZ);
	this->ctrlPoints.push_back(ctrl_axis_.centerPoint + ctrl_axis_.axisYVector * fRadius_);
	this->ctrlPoints.push_back(ctrl_axis_.centerPoint - ctrl_axis_.axisYVector * fRadius_);
	this->ctrlPoints.push_back(ctrl_axis_.centerPoint + ctrl_axis_.axisZVector * fRadius_);
	this->ctrlPoints.push_back(ctrl_axis_.centerPoint);

	if (leftAndRightAdjEnable_.empty())
	{
		leftAndRightAdjEnable_.push_back(make_pair(true, true));
		leftAndRightAdjEnable_.push_back(make_pair(true, true));
		leftAndRightAdjEnable_.push_back(make_pair(true, true));
		leftAndRightAdjEnable_.push_back(make_pair(true, true));
	}
}

bool CutFace::pickAdjHandle(int x, int y, CtrlNodeIndex &_ctrlNodeIndex)
{
	bool picked = false;
	int adjParameter;
	picked = UtilityTools::getInstance()->pickHandle(x, y, adjParameter, this->ctrlPoints, fRadius_CtrlNode);
	if (picked)
	{
		if (adjParameter == 0)
		{
			_ctrlNodeIndex = ROTATE_CTRL_NODE_1;
		}
		else if (adjParameter == 1)
		{
			_ctrlNodeIndex = ROTATE_CTRL_NODE_2;
		}
		else if (adjParameter == 2)
		{
			_ctrlNodeIndex = ROTATE_CTRL_NODE_3;
		}
		else if (adjParameter == 3)
		{
			_ctrlNodeIndex = TRANSLATE_CTRL_NODE;
		}
		return true;
	}
	return false;
}

void CutFace::transformAction(CtrlNodeIndex _ctrlNodeIndex, int _mouseX, int _mouseY, int _width, int _height)
{
	switch (_ctrlNodeIndex)
	{
	case ROTATE_CTRL_NODE_1:
	case ROTATE_CTRL_NODE_2:
	case ROTATE_CTRL_NODE_3:
		rotateAction(_ctrlNodeIndex, _mouseX, _mouseY, _width, _height);
		break;
	case TRANSLATE_CTRL_NODE:
		translateAction(_ctrlNodeIndex, _mouseX, _mouseY, _width, _height);
		break;
	default:
		break;
	}
}

void CutFace::rotateAction(CtrlNodeIndex _ctrlNodeIndex, int _mouseX, int _mouseY, int _width, int _height)
{
	if (ctrlPoints.empty())
	{
		return;
	}

	Point3m viewDirect, cameraPos;
	UtilityTools::getInstance()->getCurrentCameraState(nullptr, _width, _height, viewDirect, cameraPos);

	Point3m handleP, normalV, normalV2, mixP, rotateAxisVec;
	handleP = this->ctrlPoints[_ctrlNodeIndex];
	normalV = -viewDirect;
	normalV2 = normalV;
	mixP = UtilityTools::getInstance()->getMixedPointFromScreenToWorld2(_mouseX, _mouseY, normalV, normalV2, handleP, nullptr);
	Point3m handleAxis = (mixP - ctrl_axis_.centerPoint).Normalize();
	Point3m last_handleAxis = (handleP - ctrl_axis_.centerPoint).Normalize();

	bool rotate_to_right = false;
	if (handleAxis * ctrl_axis_.axisXVector > 0)
	{
		rotate_to_right = true;
	}
	if (!rotate_to_right && !leftAndRightAdjEnable_[_ctrlNodeIndex].first)
	{
		return;
	}
	if (rotate_to_right && !leftAndRightAdjEnable_[_ctrlNodeIndex].second)
	{
		return;
	}

	float cosValue;
	if (_ctrlNodeIndex == ROTATE_CTRL_NODE_3)
	{
		rotateAxisVec = ctrl_axis_.axisYVector;
	}
	else
	{
		rotateAxisVec = ctrl_axis_.axisZVector;
	}

	Point3m proj_vert = UtilityTools::getInstance()->getProjPointOnPlane(mixP, handleP, -rotateAxisVec);
	handleAxis = (proj_vert - ctrl_axis_.centerPoint).Normalize();
	cosValue = handleAxis * last_handleAxis;
	cosValue = cosValue > 1.0f ? 1.0f : cosValue;
	cosValue = cosValue < -1.0f ? -1.0f : cosValue;
	float rotateRad = acosf(cosValue);

	if ((last_handleAxis ^ handleAxis) * rotateAxisVec < 0)
	{
		rotateRad *= -1;
	}

	Point3m xNode, yNode, zNode;
	xNode = this->axis_.centerPoint + this->axis_.axisXVector;
	yNode = this->axis_.centerPoint + this->axis_.axisYVector;
	zNode = this->axis_.centerPoint + this->axis_.axisZVector;

	vcg::Matrix44f rotateMatrix = vcg::Matrix44f().SetRotateRad(rotateRad, rotateAxisVec);
	vcg::Matrix44f transMat, transform, transformInverse;
	transform.SetIdentity();
	transform.SetTranslate(this->axis_.centerPoint);
	transformInverse.SetTranslate(-this->axis_.centerPoint);
	transform *= rotateMatrix;
	transform *= transformInverse;
	xNode = transform * xNode;
	yNode = transform * yNode;
	zNode = transform * zNode;
	axis_.axisXVector = (xNode - this->axis_.centerPoint).Normalize();
	axis_.axisYVector = (yNode - this->axis_.centerPoint).Normalize();
	axis_.axisZVector = (zNode - this->axis_.centerPoint).Normalize();

	updateCutFace();
}

void CutFace::translateAction(CtrlNodeIndex _ctrlNodeIndex, int _mouseX, int _mouseY, int _width, int _height)
{
	Point3m handleP, normalV, normalV2, mixP;
    if(_ctrlNodeIndex<this->ctrlPoints.size())
    {
        handleP = this->ctrlPoints[_ctrlNodeIndex];
        normalV = ctrl_axis_.axisZVector;
        normalV2 = normalV;
        mixP = UtilityTools::getInstance()->getMixedPointFromScreenToWorld2(_mouseX, _mouseY, normalV, normalV2, handleP, nullptr);

        axis_.centerPoint = mixP;
        ctrl_axis_.centerPoint = mixP;
        updateCutFace();
    }
}

void CutFace::setDefaultCutPlaneRadiusMin(float _radius)
{
	CutFace::DEFAULT_CUTPLANE_RADIUS_MIN = _radius;
}

void CutFace::setDefaultCutPlaneRadiusMax(float _radius)
{
	CutFace::DEFAULT_CUTPLANE_RADIUS_MAX = _radius;
}

void CutFace::cutMeshModel(MeshModel* _mp, CloudOctree* _p_tree)
{
	if (_mp == nullptr)
	{
		return;
	}
	vcg::Matrix44f  matrix = _mp->cm.Tr;
	cut_profile_.clear();

	vector<int> possible_faces;
	if (_p_tree != nullptr)
	{
		HBall ball(matrix * axis_.centerPoint, fRadius_ * 1.2f);
		possible_faces = _p_tree->collectIntersectNodes(ball);
	}

	if (_mp->visible && !_mp->cm.vert.empty() && !possible_faces.empty())
	{
		vcg::Color4b color = _mp->cm.vert[0].C();
		for (auto &index : possible_faces)
		{
			computeCrossSegmentByFace(&_mp->cm.face[index], axis_.axisZVector, axis_.centerPoint, color, matrix);
		}
	}
	vector<int>().swap(possible_faces);
}

bool CutFace::computeCrossPoint(Point3m startP, Point3m endP, Point3m cutFaceNormalV, Point3m cutFacePosP, Point3m& CrossPoint)
{

	float fPointMulit;
	Point3m sideVector1, sideVector2;
	sideVector1 = startP - cutFacePosP;
	sideVector2 = endP - cutFacePosP;
	if ((sideVector1 * cutFaceNormalV) * (sideVector2 * cutFaceNormalV) > 0)
		return false;

	Point3m cs, se;
	cs = startP - cutFacePosP;
	se = endP - startP;
	float base = se * cutFaceNormalV;
	if (base == 0)
		return false;

	float t = -(cs * cutFaceNormalV) / base;
	if (0 <= t && t <= 1)
	{
		CrossPoint = startP + se * t;
		return true;
	}
	return false;
}

void CutFace::computeCrossSegmentByFace(CFaceO* face, Point3m cutFaceNormalV, Point3m cutFacePosP, vcg::Color4b _color, vcg::Matrix44f _matrix)
{
	Point3m p1, p2, p3;
	bool bCrossed1, bCrossed2, bCrossed3;
	Point3m tempStartP, tempEndP;
	tempStartP = _matrix * face->V(0)->P();
	tempEndP = _matrix * face->V(1)->P();
	bCrossed1 = computeCrossPoint(tempStartP, tempEndP, cutFaceNormalV, cutFacePosP, p1);
	tempStartP = _matrix * face->V(1)->P();
	tempEndP = _matrix * face->V(2)->P();
	bCrossed2 = computeCrossPoint(tempStartP, tempEndP, cutFaceNormalV, cutFacePosP, p2);
	tempStartP = _matrix * face->V(2)->P();
	tempEndP = _matrix * face->V(0)->P();
	bCrossed3 = computeCrossPoint(tempStartP, tempEndP, cutFaceNormalV, cutFacePosP, p3);

	Point3m vec, adj_vec;
	FEdge newVertCP;
	if (bCrossed1 && bCrossed2)
	{
		vec = p1 - p2;
		adj_vec = vec ^ cutFaceNormalV;
		if (adj_vec * face->N() < 0)
		{
			adj_vec *= -1.0f;
		}
		adj_vec = adj_vec.Normalize();
		p1 += adj_vec * 0.01f;
		p2 += adj_vec * 0.01f;

		if ((p1 - cutFacePosP) * (p1 - cutFacePosP) <= fRadius_ * fRadius_ &&
			(p2 - cutFacePosP) * (p2 - cutFacePosP) <= fRadius_ * fRadius_)
		{
			newVertCP = FEdge(p1, p2);
			cut_profile_.push_back(newVertCP);
			return;
		}
	}
	if (bCrossed2 && bCrossed3)
	{
		vec = p2 - p3;
		adj_vec = vec ^ cutFaceNormalV;
		if (adj_vec * face->N() < 0)
		{
			adj_vec *= -1.0f;
		}
		adj_vec = adj_vec.Normalize();
		p2 += adj_vec * 0.01f;
		p3 += adj_vec * 0.01f;

		if ((p2 - cutFacePosP) * (p2 - cutFacePosP) <= fRadius_ * fRadius_ &&
			(p3 - cutFacePosP) * (p3 - cutFacePosP) <= fRadius_ * fRadius_)
		{
			newVertCP = FEdge(p2, p3);
			cut_profile_.push_back(newVertCP);
			return;
		}
	}
	if (bCrossed3 && bCrossed1)
	{
		vec = p3 - p1;
		adj_vec = vec ^ cutFaceNormalV;
		if (adj_vec * face->N() < 0)
		{
			adj_vec *= -1.0f;
		}
		adj_vec = adj_vec.Normalize();
		p3 += adj_vec * 0.01f;
		p1 += adj_vec * 0.01f;

		if ((p3 - cutFacePosP) * (p3 - cutFacePosP) <= fRadius_ * fRadius_ &&
			(p1 - cutFacePosP) * (p1 - cutFacePosP) <= fRadius_ * fRadius_)
		{
			newVertCP = FEdge(p3, p1);
			cut_profile_.push_back(newVertCP);
			return;
		}
	}
	return;
}
