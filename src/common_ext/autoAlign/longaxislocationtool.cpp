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

#include "longaxislocationtool.h"
#include "../../common/ml_mesh_type.h"

// Control node colors for visualization
static Point3m top_color(0.55f, 0.34f, 0.7f);
static Point3m top_picked_color(0.42f, 0.29f, 0.91f);
static Point3m bottom_color(0.68f, 0.2f, 0.23f);
static Point3m bottom_picked_color(1.0f, 0.12f, 0.12f);

LongAxisLocationTool::LongAxisLocationTool(Axis* _aim_axis, CMeshO* _cur_mesh, vcg::Matrix44f _matrix, int _fdi)
{
	this->aim_axis_ = _aim_axis;
	this->cur_mesh_ = _cur_mesh;
	this->model_matrix_ = _matrix;
	this->face_range_ = cur_mesh_->face.size();
	this->fdi_ = _fdi;

	int tooth_sign = this->fdi_ % 10;
	this->znode_t_ = 0.5f;
	switch (tooth_sign)
	{
	case 1:
	case 2:
	case 3:
		this->znode_t_ = 0.525f;
		break;

	default:
		this->znode_t_ = 0.5f;
		break;
	}

	initalKdTree();
	initalTool();
	adjustAction();
}

LongAxisLocationTool::LongAxisLocationTool(Axis* _aim_axis, CMeshO* _cur_mesh, vcg::Matrix44f _matrix, int _fdi, int _range)
{
	this->aim_axis_ = _aim_axis;
	this->cur_mesh_ = _cur_mesh;
	this->model_matrix_ = _matrix;

	if (_range == -1)
	{
		this->face_range_ = cur_mesh_->face.size();
	}
	else
	{
		this->face_range_ = _range;
	}
	this->fdi_ = _fdi;

	int tooth_sign = this->fdi_ % 10;
	this->znode_t_ = 0.5f;
	switch (tooth_sign)
	{
	case 1:
	case 2:
	case 3:
		this->znode_t_ = 0.525f;
		break;

	default:
		this->znode_t_ = 0.5f;
		break;
	}

	initalKdTree();
	initalTool();
	adjustAction();
}

LongAxisLocationTool::LongAxisLocationTool(Axis* _aim_axis, CMeshO* _cur_mesh, vcg::Matrix44f _matrix, int _fdi, QString& _context):LongAxisLocationTool(_aim_axis, _cur_mesh, _matrix, _fdi)
{
	this->display_context_ = _context;
}

void LongAxisLocationTool::setFaceRange(int _range)
{
	this->face_range_ = _range;
}

void LongAxisLocationTool::setModelMatrix(vcg::Matrix44f _matrix)
{
	this->model_matrix_ = _matrix;
}

void LongAxisLocationTool::initalTool()
{
	getOriginNode();
	updateEnableFaceIndexes();
	updateProfile();
	initalCtrlNode(this->profile_);
}

void LongAxisLocationTool::getOriginNode()
{
	if (this->cur_mesh_ == nullptr)
	{
		return;
	}

	int tooth_sign = fdi_ % 10;
	if (tooth_sign == 6 || tooth_sign == 7 || tooth_sign == 8)
	{
		this->origin_node_ = centerOfBoundary();
	}
	else
	{
		this->origin_node_ = Point3m(0, 0, 0);
		for (int i = 0; i < cur_mesh_->vert.size(); ++i)
		{
			origin_node_ += cur_mesh_->vert[i].P();
		}
		origin_node_ /= float(cur_mesh_->vert.size());
	}
	this->focus_center_ = origin_node_;
}

Point3m LongAxisLocationTool::centerOfBoundary()
{
	if (cur_mesh_ == nullptr)
	{
		return Point3m(0,0,0);
	}
	UtilityTools::getInstance()->updateMeshData(cur_mesh_);
	Point3m center(0, 0, 0);
	float icounter = 0;
	// Find boundary vertices (vertices where vertex count - face count == 1)
	for (int i = 0; i < cur_mesh_->vert.size(); ++i)
	{
		vector<int> vlist = UtilityTools::getInstance()->oneRingNeighborhoodVV(i, cur_mesh_);
		vector<int> flist = UtilityTools::getInstance()->oneRingNeighborhoodVF(i, cur_mesh_);
		if (vlist.size() - flist.size() == 1)
		{
			center += cur_mesh_->vert[i].P();
			icounter += 1.0f;
		}
		vector<int>().swap(vlist);
		vector<int>().swap(flist);
	}
	return center / icounter;
}

void LongAxisLocationTool::initalKdTree()
{
	if (this->kt_ != nullptr)
	{
		delete this->kt_;
		this->kt_ = nullptr;
	}

	// Build KdTree from face centroids for spatial queries
	Point3m v0, v1, v2;
	vector<Point3m> points;
	for (int i = 0; i < this->cur_mesh_->face.size(); ++i)
	{
		v0 = cur_mesh_->face[i].V(0)->P();
		v1 = cur_mesh_->face[i].V(1)->P();
		v2 = cur_mesh_->face[i].V(2)->P();
		points.push_back((v0 + v1 + v2) / 3.0f);
	}
	if (points.size() == 0)
	{
		return;
	}
	this->kt_ = new KdTree<float>(points);
	vector<Point3m>().swap(points);
}

Point3m LongAxisLocationTool::adjustMeshOrigin()
{
	if (this->kt_ == nullptr)
	{
		initalKdTree();
	}

	int tooth_sign = this->fdi_ % 10;
	if (tooth_sign >= 6 || kt_ == nullptr)
	{
		this->origin_node_ = focus_center_ + this->aim_axis_->axisYVector;
	}
	else
	{
		Point3m middle_node = top_node_ * 0.5f + bottom_node_ * 0.5f;
		typename KdTree<float>::PriorityQueue pq;
		kt_->doQueryK(middle_node, 5, pq);
		int iface = pq.getIndex(0);

		Point3m adj_vec = this->cur_mesh_->face[iface].N();

		float proj_len = (focus_center_ - middle_node) * adj_vec;
		this->origin_node_ = middle_node + adj_vec * proj_len;
	}
	Point3m z_node = top_node_ * (1 - znode_t_) + bottom_node_ * znode_t_;
	return z_node;
}

void LongAxisLocationTool::updateEnableFaceIndexes()
{
	if (this->aim_axis_ == nullptr || this->cur_mesh_ == nullptr)
	{
		return;
	}

	float judge_value;
	int tooth_sign = this->fdi_ % 10;
	if (tooth_sign == 6 || tooth_sign == 7 || tooth_sign == 8)
	{
		judge_value = 0.25882f;
	}
	else
	{
		judge_value = 0.17364f;
	}

	vector<int>().swap(enable_faces_);
	Point3m judge_vec = aim_axis_->axisZVector;
	Point3m judge_origin = origin_node_ + judge_vec * 0.5f;
	for (int i = 0; i < face_range_; ++i)
	{
		if (judge_vec * cur_mesh_->face[i].N() > judge_value)
		{
			Point3m temp_vec = cur_mesh_->face[i].V(0)->P() - judge_origin;
			if (temp_vec * judge_vec > 0)
			{
				enable_faces_.push_back(i);
			}
		}
	}

	// Fallback: use relaxed criteria if no faces found
	if (enable_faces_.empty())
	{
		for (int i = 0; i < face_range_; ++i)
		{
			if (judge_vec * cur_mesh_->face[i].N() > 0)
			{
				Point3m temp_vec = cur_mesh_->face[i].V(0)->P() - origin_node_;
				if (temp_vec * judge_vec > 0)
				{
					enable_faces_.push_back(i);
				}
			}
		}
	}
}

void LongAxisLocationTool::updateProfile()
{
	if (this->aim_axis_ == nullptr || this->cur_mesh_ == nullptr || enable_faces_.empty())
	{
		return;
	}

	vector<FColorEdge>().swap(profile_);
	for (auto index : enable_faces_)
	{
		FColorEdge line_seg;
		if (computeCrossSegmentByFace(&cur_mesh_->face[index], aim_axis_->axisXVector, origin_node_, model_matrix_, line_seg))
		{
			profile_.push_back(line_seg);
		}
	}
	updateProfileColor(profile_);
}

bool LongAxisLocationTool::computeCrossSegmentByFace(CFaceO* face, Point3m cutFaceNormalV, Point3m cutFacePosP, vcg::Matrix44f _matrix, FColorEdge& _edge)
{
	Point3m p1, p2, p3;
	bool bCrossed1, bCrossed2, bCrossed3;
	Point3m tempStartP, tempEndP;
	tempStartP = face->V(0)->P();
	tempEndP = face->V(1)->P();
	bCrossed1 = UtilityTools::getInstance()->computeCrossPoint(tempStartP, tempEndP, cutFaceNormalV, cutFacePosP, p1);
	tempStartP = face->V(1)->P();
	tempEndP = face->V(2)->P();
	bCrossed2 = UtilityTools::getInstance()->computeCrossPoint(tempStartP, tempEndP, cutFaceNormalV, cutFacePosP, p2);
	tempStartP = face->V(2)->P();
	tempEndP = face->V(0)->P();
	bCrossed3 = UtilityTools::getInstance()->computeCrossPoint(tempStartP, tempEndP, cutFaceNormalV, cutFacePosP, p3);

	if (bCrossed1 && bCrossed2)
	{
		_edge = FColorEdge(p1, p2);
		return true;
	}
	if (bCrossed2 && bCrossed3)
	{
		_edge = FColorEdge(p2, p3);
		return true;
	}
	if (bCrossed3 && bCrossed1)
	{
		_edge = FColorEdge(p3, p1);
		return true;
	}

	return false;
}

void LongAxisLocationTool::updateProfileColor(vector<FColorEdge>& _edges)
{
	if (this->aim_axis_ == nullptr || _edges.empty())
	{
		return;
	}

	Point3m vec;
	float proj_a, proj_b;
	float max_proj_len = FLT_MIN;
	float min_proj_len = FLT_MAX;
	std::vector<std::pair<float, float>> proj_len_list;

	std::set<std::pair<float, Point3m>> records;
	for (int i = 0; i < _edges.size(); ++i)
	{
		Point3m vert = (_edges[i].vertA + _edges[i].vertB) / 2.0f;
		vec = vert - origin_node_;
		proj_a = vec * aim_axis_->axisYVector;

		max_proj_len = (proj_a > max_proj_len ? proj_a : max_proj_len);
		min_proj_len = (proj_a < min_proj_len ? proj_a : min_proj_len);

		records.insert(make_pair(proj_a, vert));
	}

	_edges.clear();
	std::set<std::pair<float, Point3m>>::iterator ite = records.begin();
	while (ite != --records.end())
	{
		Point3m a = ite->second;
		float proj_a = ite->first;

		++ite;

		Point3m b = ite->second;
		float proj_b = ite->first;

		_edges.push_back(FColorEdge(a, b));
		proj_len_list.push_back(make_pair(proj_a, proj_b));
	}

	float t;
	Point3m color_a, color_b;
	for (int i = 0; i < proj_len_list.size(); ++i)
	{
		t = (proj_len_list[i].first - min_proj_len) / (max_proj_len - min_proj_len);
		color_a = top_color * t + bottom_color * (1.0f - t);
		t = (proj_len_list[i].second - min_proj_len) / (max_proj_len - min_proj_len);
		color_b = top_color * t + bottom_color * (1.0f - t);

		_edges[i].setColor(color_a, color_b);
	}
	std::vector<std::pair<float, float>>().swap(proj_len_list);
	std::set<std::pair<float, Point3m>>().swap(records);
}

void LongAxisLocationTool::initalCtrlNode(vector<FColorEdge>& _edges)
{
	if (this->aim_axis_ == nullptr || _edges.empty())
	{
		if (_edges.empty())
		{
			forceInital();
		}
		return;
	}

	Point3m vec;
	float proj_a, proj_b;
	float max_proj_len = FLT_MIN;
	float min_proj_len = FLT_MAX;
	for (int i = 0; i < _edges.size(); ++i)
	{
		vec = _edges[i].vertA - origin_node_;
		proj_a = vec * aim_axis_->axisYVector;
		vec = _edges[i].vertB - origin_node_;
		proj_b = vec * aim_axis_->axisYVector;

		if (proj_a > max_proj_len)
		{
			this->top_node_ = _edges[i].vertA;
			max_proj_len = proj_a;
		}
		if (proj_b > max_proj_len)
		{
			this->top_node_ = _edges[i].vertB;
			max_proj_len = proj_b;
		}

		if (proj_a < min_proj_len)
		{
			this->bottom_node_ = _edges[i].vertA;
			min_proj_len = proj_a;
		}
		if (proj_b < min_proj_len)
		{
			this->bottom_node_ = _edges[i].vertB;
			min_proj_len = proj_b;
		}
	}
	if (max_proj_len == FLT_MIN && min_proj_len != FLT_MAX)
	{
		this->top_node_ = this->bottom_node_ + aim_axis_->axisYVector * 0.1f;
	}
	if (max_proj_len != FLT_MIN && min_proj_len == FLT_MAX)
	{
		this->bottom_node_ = this->top_node_ - aim_axis_->axisYVector * 0.1f;
	}
}

void LongAxisLocationTool::adjustAction()
{
	Point3m znode = adjustMeshOrigin();

	Point3m vec1, vec2;
	Point3m axis_x, axis_y, axis_z;
	vec1 = (top_node_ - origin_node_).Normalize();
	vec2 = (bottom_node_ - origin_node_).Normalize();
	axis_x = (vec1 ^ vec2).Normalize();

	axis_z = (znode - origin_node_).Normalize();

	axis_y = (axis_z ^ axis_x).Normalize();

	this->aim_axis_->centerPoint = this->origin_node_;
	this->aim_axis_->axisXVector = axis_x;
	this->aim_axis_->axisYVector = axis_y;
	this->aim_axis_->axisZVector = axis_z;
	updateEnableFaceIndexes();
	updateProfile();
}

void LongAxisLocationTool::forceInital()
{
	if (this->cur_mesh_ == nullptr)
	{
		return;
	}
	if (cur_mesh_->face.size() == 0)
	{
		return;
	}

	int top_index = 0;
	int bottom_index = this->cur_mesh_->face.size()-1;
	this->top_node_ = cur_mesh_->face[top_index].V(0)->P();
	this->bottom_node_ = cur_mesh_->face[bottom_index].V(0)->P();
	adjustAction();
}

void LongAxisLocationTool::initalDirectly(Axis* _aim_axis, CMeshO* _cur_mesh, vcg::Matrix44f _matrix, int _fdi)
{
	this->aim_axis_ = _aim_axis;
	this->cur_mesh_ = _cur_mesh;
	this->model_matrix_ = _matrix;
	this->face_range_ = cur_mesh_->face.size();
	this->fdi_ = _fdi;

	int tooth_sign = this->fdi_ % 10;
	this->znode_t_ = 0.5f;
	switch (tooth_sign)
	{
	case 1:
	case 2:
	case 3:
		this->znode_t_ = 0.525f;
		break;

	default:
		this->znode_t_ = 0.5f;
		break;
	}

	initalKdTree();
	origin_node_ = aim_axis_->centerPoint;
	this->focus_center_ = origin_node_;
	updateEnableFaceIndexes();
	updateProfile();
	initalCtrlNode(this->profile_);
}

void LongAxisLocationTool::draw()
{
	glPushMatrix();
	glMultMatrix(this->model_matrix_);

	drawProfile();
	drawCtrlNodes();
	if (!this->display_context_.isNull())
	{
		drawContext();
	}
	glPopMatrix();
}

void LongAxisLocationTool::drawCtrlNodes()
{
	glPushMatrix();

	if (picked_state_ == TOP_PICKED)
	{
		glColor3f(top_picked_color.X(), top_picked_color.Y(), top_picked_color.Z());
	}
	else
	{
		glColor3f(top_color.X(), top_color.Y(), top_color.Z());
	}
	vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(this->top_node_, CTRL_NODE_WIDTH);
	if (picked_state_ == BOTTOM_PICKED)
	{
		glColor3f(bottom_picked_color.X(), bottom_picked_color.Y(), bottom_picked_color.Z());
	}
	else
	{
		glColor3f(bottom_color.X(), bottom_color.Y(), bottom_color.Z());
	}
	vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(this->bottom_node_, CTRL_NODE_WIDTH);

	glPopMatrix();
}

void LongAxisLocationTool::drawProfile()
{
	glPushMatrix();
	Point3m adj_vec = this->aim_axis_->axisZVector * 0.1f;

	glDisable(GL_LIGHTING);
	glLineWidth(4.0f);
	glBegin(GL_LINES);
	for (auto edge : this->profile_)
	{
		glColor3f(edge.colorA.X(), edge.colorA.Y(), edge.colorA.Z());
		glVertex3f(edge.vertA.X() + adj_vec.X(), edge.vertA.Y() + adj_vec.Y(), edge.vertA.Z() + adj_vec.Z());
		glColor3f(edge.colorB.X(), edge.colorB.Y(), edge.colorB.Z());
		glVertex3f(edge.vertB.X() + adj_vec.X(), edge.vertB.Y() + adj_vec.Y(), edge.vertB.Z() + adj_vec.Z());
	}
	glEnd();
	glEnable(GL_LIGHTING);

	glPopMatrix();
}

void LongAxisLocationTool::drawContext()
{
	// TODO: Implement context drawing if needed
}

bool LongAxisLocationTool::mousePressAction(int _mousex, int _mousey)
{
	glPushMatrix();
	glMultMatrix(this->model_matrix_);

	bool picked = false;
	int parameter;
	std::vector<Point3m> node_list;
	node_list.push_back(top_node_);
	node_list.push_back(bottom_node_);
	picked = UtilityTools::getInstance()->pickHandle(_mousex, _mousey, parameter, node_list,  CTRL_NODE_WIDTH);
	if (picked)
	{
		if (parameter == 0)
		{
			picked_state_ = TOP_PICKED;
		}
		else if (parameter == 1)
		{
			picked_state_ = BOTTOM_PICKED;
		}

		glPopMatrix();
		return true;
	}
	glPopMatrix();
	picked_state_ = NONE_PICKED;
	return false;
}

bool LongAxisLocationTool::mouseMoveAction(int _mousex, int _mousey)
{
	if (picked_state_ == NONE_PICKED)
	{
		return false;
	}

	glPushMatrix();
	std::vector<CFaceO*> vf;
	Point3m picked_point;
	bool picked = UtilityTools::getInstance()->getPickedPointOnMesh(_mousex, _mousey, vf, this->cur_mesh_, picked_point);
	if (picked)
	{
		Point3m vcenter = picked_point;

		if (picked_state_ == TOP_PICKED)
		{
			this->top_node_ = vcenter;
		}
		else if (picked_state_ == BOTTOM_PICKED)
		{
			this->bottom_node_ = vcenter;
		}
		adjustAction();

		glPopMatrix();
		return true;
	}
	glPopMatrix();
	return false;
}

bool LongAxisLocationTool::mouseReleaseAction(int _mousex, int _mousey)
{
	picked_state_ = NONE_PICKED;
	return true;
}
