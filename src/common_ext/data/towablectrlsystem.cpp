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

#include "towablectrlsystem.h"

float node_radius = 0.5f;
float cylinder_radius = 0.2f;
float arrow_radius = 0.5f, arrow_length = 1.0f;
Point3m picked_color(186 / 255.0f, 227 / 255.0f, 249 / 255.0f);
Point3m normal_color(84 / 255.0f, 195 / 255.0f, 241 / 255.0f);

void TowableCtrlEdge::draw()
{
	if (visible_ && a_ && b_)
	{
		glPushMatrix();
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		if (picked_up_)
		{
			glColor3f(picked_color.X(), picked_color.Y(), picked_color.Z());
		}
		else
		{
			glColor3f(normal_color.X(), normal_color.Y(), normal_color.Z());
		}
		vcg::Add_Ons::glCylinder<vcg::Add_Ons::DMSolid>(a_->p_, b_->p_, cylinder_radius);
		glDisable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glPopMatrix();
	}
}

bool TowableCtrlEdge::mousePress(int _mouse_x, int _mouse_y)
{
	if (!visible_ || !a_ || !b_)
	{
		picked_up_ = false;
		return false;
	}

	int adjParameter;
	vector<FEdge> edge_list;
	edge_list.push_back(FEdge(a_->p_, b_->p_));
	picked_up_ = UtilityTools::getInstance()->pickCylinder(
		_mouse_x, _mouse_y,
		adjParameter, edge_list, cylinder_radius);

	if (picked_up_)
	{
		Point3m cam_normal, cam_pos;
		UtilityTools::getInstance()->getCurrentCameraState(nullptr, screen_width_, screen_height_, cam_normal, cam_pos);
		Point3m plane_normal = -cam_normal;
		Point3m plane_pos = (a_->p_ + b_->p_) * 0.5f;

		glPushMatrix();
		this->picked_pos_ = UtilityTools::getInstance()->getMixedPointFromScreenToWorld2(_mouse_x, _mouse_y, plane_normal, plane_normal, plane_pos, nullptr);
		glPopMatrix();
	}

	return picked_up_;
}

bool TowableCtrlEdge::mouseRelease(int _mouse_x, int _mouse_y)
{
	picked_up_ = false;
	return false;
}

bool TowableCtrlEdge::mouseMove(int _mouse_x, int _mouse_y)
{
	if (picked_up_ && a_ && b_)
	{
		Point3m cam_normal, cam_pos;
		UtilityTools::getInstance()->getCurrentCameraState(nullptr, screen_width_, screen_height_, cam_normal, cam_pos);
		Point3m plane_normal = -cam_normal;
		Point3m plane_pos = (a_->p_ + b_->p_) * 0.5f;

		glPushMatrix();
		Point3m mixP = UtilityTools::getInstance()->getMixedPointFromScreenToWorld2(_mouse_x, _mouse_y, plane_normal, plane_normal, plane_pos, nullptr);
		if (mixP == this->picked_pos_)
		{
			glPopMatrix();
			return false;
		}

		Point3m vec = mixP - this->picked_pos_;
		this->a_->p_ += vec;
		this->b_->p_ += vec;
		this->picked_pos_ = mixP;

		glPopMatrix();
		return true;
	}
	return false;
}

void TowableCtrlNode::draw()
{
	if (visible_)
	{
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushMatrix();
		glDisable(GL_DEPTH_TEST);
		if (picked_up_)
		{
			glColor3f(picked_color.X(), picked_color.Y(), picked_color.Z());
		}
		else
		{
			glColor3f(normal_color.X(), normal_color.Y(), normal_color.Z());
		}
		if (this->arrow_mode_)
		{

			glEnable(GL_LIGHTING);
			Point3m arrow_direct;
			if (this->adjoint_node_ != nullptr)
			{
				arrow_direct = (p_ - adjoint_node_->p_).Normalize();
			}
			vcg::Add_Ons::glCone<vcg::Add_Ons::DMFlat>(p_ - arrow_direct * arrow_length, p_, arrow_radius, 20);
		}
		else
		{
			glEnable(GL_LIGHTING);
			vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(p_, node_radius);
		}
		glDisable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glPopMatrix();
		glPopAttrib();
	}
}

bool TowableCtrlNode::mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn )
{
	if (!visible_)
	{
		picked_up_ = false;
		return false;
	}

	if (this->arrow_mode_)
	{
		Point3m arrow_direct;
		if (this->adjoint_node_ != nullptr)
		{
			arrow_direct = (p_ - adjoint_node_->p_).Normalize();

			if (_mouse_btn == Qt::RightButton)
			{
				draw_guidelines_ = true;
			}
		}

		int adjParameter;
		std::vector<pair<Point3m, Point3m>> point_list;
		point_list.push_back(make_pair(p_ - arrow_direct * arrow_length, p_));

		picked_up_ = UtilityTools::getInstance()->pickConeHandle(
			_mouse_x, _mouse_y,
			adjParameter, point_list, arrow_radius);
		return picked_up_;
	}
	else
	{
		int adjParameter;
		vector<Point3m> point_list;
		point_list.push_back(p_);
		picked_up_ = UtilityTools::getInstance()->pickHandle(
			_mouse_x, _mouse_y,
			adjParameter, point_list, node_radius);
		return picked_up_;
	}
}

bool TowableCtrlNode::mouseRelease(int _mouse_x, int _mouse_y)
{
	picked_up_ = false;
	draw_guidelines_ = false;
	return false;
}

void TowableCtrlNode::setArrowMode(bool _state)
{
	this->arrow_mode_ = _state;
}

bool TowableCtrlNode::spaceConstraintsJudge(Point3m _p)
{
	if (spce_constraints_.first)
	{
		Point3m vec = _p - spce_constraints_.second.first;
		if (vec * spce_constraints_.second.second > 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool TowableCtrlNodeOnMesh::mouseMove(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn )
{
	if (picked_up_)
	{
		Point3m picked_point;
		std::vector<CFaceO*> vf;

		glPushMatrix();
		for (int i = 0; i < mesh_list_.size(); ++i)
		{
			if (mesh_list_[i] != nullptr)
			{
				if (cloud_pick_unvisible_faces_)
				{
					bool picked = UtilityTools::getInstance()->getPickedPointOnMesh(_mouse_x, _mouse_y, vf, mesh_list_[i], picked_point);
					if (picked && spaceConstraintsJudge(picked_point))
					{
						this->p_ = picked_point;
						glPopMatrix();
						return true;
					}
				}
				else
				{
					Point3d pointPicked = UtilityTools::getInstance()->transformPointFrom2DscreenTo3Dworld(_mouse_x, _mouse_y, mesh_list_[i]);
					int vIdPicked = UtilityTools::getInstance()->getIndexPointClosestMesh(mesh_list_[i], Point3m(pointPicked.X(), pointPicked.Y(), pointPicked.Z()));
					picked_point = Point3f(pointPicked.X(), pointPicked.Y(), pointPicked.Z());
					if (vIdPicked >= 0 && spaceConstraintsJudge(picked_point))
					{
						this->p_ = picked_point;
						glPopMatrix();
						return true;
					}
				}
			}
		}
		glPopMatrix();
		return false;
	}
	return false;
}

bool TowableCtrlNodeOnPlane::mouseMove(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn )
{
	if (picked_up_)
	{
		draw_guidelines_ = false;
		if (plane_normal_is_view_direct_)
		{
			Point3m cam_normal, cam_pos;
			UtilityTools::getInstance()->getCurrentCameraState(nullptr, screen_width_, screen_height_, cam_normal, cam_pos);
			plane_normal_ = -cam_normal;

			if (this->adjoint_node_ != nullptr)
			{
				if (_mouse_btn == Qt::LeftButton)
				{
					plane_pos_ = adjoint_node_->p_;
				}
				else if (_mouse_btn == Qt::RightButton)
				{
					draw_guidelines_ = true;
					plane_pos_ = p_;
				}
			}
			else
			{
				plane_pos_ = this->p_;
			}
		}
		glPushMatrix();
		Point3m mixP = UtilityTools::getInstance()->getMixedPointFromScreenToWorld2(_mouse_x, _mouse_y, plane_normal_, plane_normal_, plane_pos_, nullptr);
		if (mixP == this->p_)
		{
			glPopMatrix();
			return false;
		}

		if (this->arrow_mode_)
		{
			Point3m arrow_direct;
			if (_mouse_btn == Qt::LeftButton)
			{
				if (this->adjoint_node_ != nullptr)
				{
					arrow_direct = (mixP - adjoint_node_->p_).Normalize();
				}
				this->p_ = mixP + arrow_direct * arrow_length * 0.5f;
			}
			else if (_mouse_btn == Qt::RightButton)
			{
				if (this->adjoint_node_ != nullptr)
				{
					arrow_direct = (p_ - adjoint_node_->p_).Normalize();
				}
				float proj_len = (mixP - adjoint_node_->p_) * arrow_direct;
				this->p_ = adjoint_node_->p_ + arrow_direct * (arrow_length * 0.5f + proj_len);
			}
		}
		else
		{
			this->p_ = mixP;
		}
		glPopMatrix();
		return true;
	}
	return false;
}

void TowableCtrlNodeOnPlane::draw()
{
	Point3m view_direct, camera_pos;
	UtilityTools::getInstance()->getCurrentCameraState(nullptr, screen_width_, screen_height_, view_direct, camera_pos);

	visible_ = true;
	if (!plane_normal_is_view_direct_ && abs(view_direct * this->plane_normal_) < 0.5f)
	{
		visible_ = false;
	}

	if (this->visible_)
	{
		if (picked_up_ && draw_guidelines_ && this->adjoint_node_ != nullptr)
		{
			Point3m vec = (p_ - adjoint_node_->p_).Normalize();
			Point3m a = p_ + vec * 1000.0f;
			Point3m b = adjoint_node_->p_ - vec * 1000.0f;

			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glPushMatrix();
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_LIGHTING);
			glEnable(GL_LINE_SMOOTH);
			glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
			glEnable(GL_LINE_STIPPLE);
			glLineStipple(1, 0x0F0F);
			glColor3f(0, 1, 0);
			glLineWidth(5.0f);
			glColor3f(116 / 255.0f, 182 / 255.0f, 173 / 255.0f);
			glBegin(GL_LINES);
			glVertex3f(a.X(), a.Y(), a.Z());
			glVertex3f(b.X(), b.Y(), b.Z());
			glEnd();
			glLineWidth(1.0f);
			glColor3f(1, 1, 1);
			glDisable(GL_LINE_STIPPLE);
			glEnable(GL_DEPTH_TEST);
			glPopMatrix();
			glPopAttrib();
		}

		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushMatrix();
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		if (picked_up_)
		{
			glColor3f(picked_color.X(), picked_color.Y(), picked_color.Z());
		}
		else
		{
			glColor3f(normal_color.X(), normal_color.Y(), normal_color.Z());
		}
		if (this->arrow_mode_)
		{
			Point3m arrow_direct;
			if (this->adjoint_node_ != nullptr)
			{
				arrow_direct = (p_ - adjoint_node_->p_).Normalize();
			}
			vcg::Add_Ons::glCone<vcg::Add_Ons::DMFlat>(p_ - arrow_direct * arrow_length, p_, arrow_radius, 20);
		}
		else
		{
			vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(p_, node_radius);
		}
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glPopMatrix();
		glPopAttrib();
	}
}

TowableCtrlSystem::TowableCtrlSystem(const TowableCtrlSystem& _sys)
{
	iNo_ = _sys.iNo_;
	length_ = _sys.length_;
	fitting_curve_ = _sys.fitting_curve_;
	edges_ = _sys.edges_;
	mesh_list_ = _sys.mesh_list_;
	picked_ctrl_node_ = _sys.picked_ctrl_node_;
	ctrl_nodes_ = _sys.ctrl_nodes_;
	visible_ = _sys.visible_;
	visual_angle_ctrl_ = _sys.visual_angle_ctrl_;
	center_ = _sys.center_;
	radius_ = _sys.radius_;
}

TowableCtrlSystem::TowableCtrlSystem(int _ino, Point3m _node_a, CMeshO* _mesh_a, Point3m _node_b, CMeshO* _mesh_b, bool _fitting_curve, bool _arrow_mode)
{
	iNo_ = _ino;
	vector<CMeshO*> mesh_a_list;
	mesh_a_list.push_back(_mesh_a);
	this->ctrl_nodes_.push_back(new TowableCtrlNodeOnMesh(_node_a, mesh_a_list, screen_height_, screen_width_));

	vector<CMeshO*> mesh_b_list;
	mesh_b_list.push_back(_mesh_b);
	this->ctrl_nodes_.push_back(new TowableCtrlNodeOnMesh(_node_b, mesh_b_list, screen_height_, screen_width_));

	this->fitting_curve_ = _fitting_curve;
	visible_ = true;
	visual_angle_ctrl_ = make_pair(false, Point3m(0, 0, 0));
	updateBoundBall();
	update();
}

TowableCtrlSystem::TowableCtrlSystem(int _ino, Point3m _node_a, vector<CMeshO*> _mesh_a_list, Point3m _node_b, vector<CMeshO*> _mesh_b_list, bool _fitting_curve)
{
	iNo_ = _ino;
	this->ctrl_nodes_.push_back(new TowableCtrlNodeOnMesh(_node_a, _mesh_a_list, screen_height_, screen_width_));
	this->ctrl_nodes_.push_back(new TowableCtrlNodeOnMesh(_node_b, _mesh_b_list, screen_height_, screen_width_));
	this->fitting_curve_ = _fitting_curve;
	visible_ = true;
	visual_angle_ctrl_ = make_pair(false, Point3m(0, 0, 0));
	updateBoundBall();
	update();
}

TowableCtrlSystem::TowableCtrlSystem(int _ino, vector<Point3m> _points, CMeshO* _mesh, bool _normal_is_view_direct, bool _fitting_curve, bool _arrow_mode)
{
	iNo_ = _ino;

	vector<CMeshO*> mesh_list;
	mesh_list.push_back(_mesh);
	for (int i = 0; i < _points.size(); ++i)
	{
		this->ctrl_nodes_.push_back(new TowableCtrlNodeOnMesh(_points[i], mesh_list, screen_height_, screen_width_));
	}

	for (int i = 0; i < _points.size(); ++i)
	{
		this->ctrl_nodes_[i]->setArrowMode(_arrow_mode);
	}

	if (_normal_is_view_direct && _points.size() == 2)
	{
		this->ctrl_nodes_[0]->setAdjointPoint(ctrl_nodes_[1]);
		this->ctrl_nodes_[1]->setAdjointPoint(ctrl_nodes_[0]);
	}

	this->fitting_curve_ = _fitting_curve;
	visible_ = true;
	visual_angle_ctrl_ = make_pair(false, Point3m(0, 0, 0));
	updateBoundBall();
	update();
}

TowableCtrlSystem::TowableCtrlSystem(int _ino, vector<pair<Point3m, vector<CMeshO*>>> _node_info_list, bool _fitting_curve, bool _cloud_pick_unvisible_face)
{
	iNo_ = _ino;
	for (auto& info : _node_info_list)
	{
		this->ctrl_nodes_.push_back(new TowableCtrlNodeOnMesh(info.first, info.second, screen_height_, screen_width_, _cloud_pick_unvisible_face));
	}
	this->fitting_curve_ = _fitting_curve;
	visible_ = true;
	visual_angle_ctrl_ = make_pair(false, Point3m(0, 0, 0));
	updateBoundBall();
	update();
}

TowableCtrlSystem::TowableCtrlSystem(int _ino, vector<Point3m> _points, Point3m _plane_pos, Point3m _plane_normal, bool _normal_is_view_direct, bool _fitting_curve, bool _arrow_mode)
{
	iNo_ = _ino;
	for (int i = 0; i < _points.size(); ++i)
	{
		this->ctrl_nodes_.push_back(new TowableCtrlNodeOnPlane(_points[i], _plane_pos, _plane_normal, _normal_is_view_direct, screen_height_, screen_width_));
	}

	for (int i = 0; i < _points.size(); ++i)
	{
		this->ctrl_nodes_[i]->setArrowMode(_arrow_mode);
	}

	if (_normal_is_view_direct && _points.size() == 2)
	{
		this->ctrl_nodes_[0]->setAdjointPoint(ctrl_nodes_[1]);
		this->ctrl_nodes_[1]->setAdjointPoint(ctrl_nodes_[0]);
	}

	this->fitting_curve_ = _fitting_curve;
	visible_ = true;
	visual_angle_ctrl_ = make_pair(false, Point3m(0, 0, 0));
	updateBoundBall();
	update();
}

void TowableCtrlSystem::update()
{
	if (ctrl_nodes_.size() < 2)
	{
		return;
	}

	vector<FEdge>().swap(edges_);
	if (!fitting_curve_)
	{
		for (int i = 0; i < ctrl_nodes_.size() - 1; ++i)
		{
			if (ctrl_nodes_[i]->arrow_mode_)
			{
				Point3m p, p_next, p_direct, p_next_direct;
				p_direct = (ctrl_nodes_[i]->p_ - ctrl_nodes_[i]->adjoint_node_->p_).Normalize();
				p_next_direct = (ctrl_nodes_[i + 1]->p_ - ctrl_nodes_[i + 1]->adjoint_node_->p_).Normalize();
				p = ctrl_nodes_[i]->p_ - p_direct * arrow_length;
				p_next = ctrl_nodes_[i + 1]->p_ - p_next_direct * arrow_length;
				edges_.push_back(FEdge(p, p_next));
			}
			else
			{
				edges_.push_back(FEdge(ctrl_nodes_[i]->p_, ctrl_nodes_[i + 1]->p_));
			}
		}
	}
	else
	{
		vector<Point3m> ctrl_verts;
		for (auto& node : ctrl_nodes_)
		{
			ctrl_verts.push_back(node->p_);
		}

		vector<Point3m> fitting_verts;
		if (auxiliary_)
		{
			fitting_verts = UtilityTools::getInstance()->catmull_Rom_SplinePoints_Equidistant(ctrl_verts, 0.5f);
		}
		else
		{
			fitting_verts = UtilityTools::getInstance()->catmull_Rom_SplinePoints_new(ctrl_verts);
		}

		for (int i = 0; i < fitting_verts.size() - 1; ++i)
		{
			edges_.push_back(FEdge(fitting_verts[i], fitting_verts[i + 1]));
		}
		vector<Point3m>().swap(ctrl_verts);
		vector<Point3m>().swap(fitting_verts);
	}

	length_ = 0;
	for (auto& edge : edges_)
	{
		Point3m vec = edge.vertA - edge.vertB;
		length_ += sqrtf(vec * vec);
	}
}

void TowableCtrlSystem::draw()
{
	visible_ = true;
	if (visual_angle_ctrl_.first)
	{
		visible_ = judgeVisibleByVisualAngle(visual_angle_ctrl_.second);
	}
	for (auto& node : ctrl_nodes_)
	{
		node->visible_ = visible_;
	}

	if (!visible_)
	{
		return;
	}

	if (auxiliary_)
	{
		drawAuxiliary();
		return;
	}

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glColor3f(0, 1, 0);
	glLineWidth(5.0f);
	glColor3f(0 / 255.0f, 123 / 255.0f, 187 / 255.0f);
	glBegin(GL_LINES);
	for (auto& edge : this->edges_)
	{
		glVertex3f(edge.vertA.X(), edge.vertA.Y(), edge.vertA.Z());
		glVertex3f(edge.vertB.X(), edge.vertB.Y(), edge.vertB.Z());
	}
	glEnd();
	glLineWidth(1.0f);
	glColor3f(1, 1, 1);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glPopAttrib();

	for (auto& node : ctrl_nodes_)
	{
		node->draw();
	}
}

void TowableCtrlSystem::drawAuxiliary()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glColor3f(0, 1, 0);
	glLineWidth(5.0f);
	glColor3f(116 / 255.0f, 182 / 255.0f, 173 / 255.0f);
	glBegin(GL_LINES);
	for (int i = 0; i < this->edges_.size(); i +=2)
	{
		glVertex3f(edges_[i].vertA.X(), edges_[i].vertA.Y(), edges_[i].vertA.Z());
		glVertex3f(edges_[i].vertB.X(), edges_[i].vertB.Y(), edges_[i].vertB.Z());
	}
	glEnd();
	glLineWidth(1.0f);
	glColor3f(1, 1, 1);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glPopAttrib();
}

void TowableCtrlSystem::clear()
{
	vector<FEdge>().swap(edges_);
	for (int i = 0; i < ctrl_nodes_.size(); ++i)
	{
		delete ctrl_nodes_[i];
		ctrl_nodes_[i] = nullptr;
	}
	vector<TowableCtrlNode*>().swap(ctrl_nodes_);
	picked_ctrl_node_ = nullptr;
	for (int i = 0; i < ctrl_edges_.size(); ++i)
	{
		delete ctrl_edges_[i];
		ctrl_edges_[i] = nullptr;
	}
	vector<TowableCtrlEdge*>().swap(ctrl_edges_);
	picked_ctrl_edge_ = nullptr;
}

void TowableCtrlSystem::setVisualAngleDirect(Point3m _direct)
{
	this->visual_angle_ctrl_ = make_pair(true, _direct);
}

void TowableCtrlSystem::setScreenWidthAndHeight(int _width, int _height)
{
	screen_width_ = _width;
	screen_height_ = _height;
}

bool TowableCtrlSystem::mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn )
{
	for (int i = 0; i < this->ctrl_nodes_.size(); ++i)
	{
		if (ctrl_nodes_[i]->mousePress(_mouse_x, _mouse_y, _mouse_btn))
		{
			picked_ctrl_node_ = ctrl_nodes_[i];
			return true;
		}
	}
	for (int i = 0; i < this->ctrl_edges_.size(); ++i)
	{
		if (ctrl_edges_[i]->mousePress(_mouse_x, _mouse_y))
		{
			picked_ctrl_edge_ = ctrl_edges_[i];
			return true;
		}
	}
	picked_ctrl_node_ = nullptr;
	picked_ctrl_edge_ = nullptr;
	return false;
}

bool TowableCtrlSystem::mouseMove(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn )
{
	if (picked_ctrl_node_ != nullptr)
	{
		picked_ctrl_node_->mouseMove(_mouse_x, _mouse_y, _mouse_btn);
		update();
		return true;
	}

	if (picked_ctrl_edge_ != nullptr)
	{
		picked_ctrl_edge_->mouseMove(_mouse_x, _mouse_y);
		update();
		return true;
	}
	return false;
}

bool TowableCtrlSystem::mouseRelease(int _mouse_x, int _mouse_y)
{
	if (picked_ctrl_node_ != nullptr)
	{
		picked_ctrl_node_->mouseRelease(_mouse_x, _mouse_y);
		picked_ctrl_node_ = nullptr;
		update();
		return true;
	}

	if (picked_ctrl_edge_ != nullptr)
	{
		picked_ctrl_edge_->mouseRelease(_mouse_x, _mouse_y);
		picked_ctrl_edge_ = nullptr;
		update();
		return true;
	}
	return false;
}

bool TowableCtrlSystem::pickedHandle()
{
	if (picked_ctrl_node_ != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool TowableCtrlSystem::judgeVisibleByVisualAngle(Point3m& _visible_direct)
{
	Point3m view_direct, cam_pos;
	UtilityTools::getInstance()->getCurrentCameraState(nullptr, screen_width_, screen_height_, view_direct, cam_pos);
	if (_visible_direct * view_direct >= 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void TowableCtrlSystem::updateBoundBall()
{
	if (ctrl_nodes_.empty())
	{
		return;
	}
	center_ = Point3m(0, 0, 0);
	for (int i = 0; i < ctrl_nodes_.size(); ++i)
	{
		center_ += ctrl_nodes_[i]->p_;
	}
	center_ /= (float)(ctrl_nodes_.size());

	Point3m vec;
	radius_ = FLT_MIN;
	for (int i = 0; i < ctrl_nodes_.size(); ++i)
	{
		vec = ctrl_nodes_[i]->p_ - center_;
		float value = vec * vec;
		if(value > radius_)
		{
			radius_ = value;
		}
	}
	radius_ = sqrtf(radius_);
}

void TowableCtrlSystem::initialTowableCtrlEdges()
{
	if (ctrl_nodes_.size() < 2)
	{
		return;
	}

	TowableCtrlNode* a, * b;
	for (int i = 0; i < ctrl_nodes_.size() - 1; ++i)
	{
		a = ctrl_nodes_[i];
		b = ctrl_nodes_[i + 1];
		TowableCtrlEdge* new_edge = new TowableCtrlEdge(a, b, screen_height_, screen_width_);
		this->ctrl_edges_.push_back(new_edge);
	}
}

void TowableCtrlSystem::udpateCtrlNodePos(const vector<Point3m> &_pos_list)
{
	if (_pos_list.size() == ctrl_nodes_.size())
	{
		for (int i = 0; i < ctrl_nodes_.size(); ++i)
		{
			ctrl_nodes_[i]->p_ = _pos_list[i];
		}
		update();
	}
}

void TowableCtrlSystem::setSpaceConstraints(pair<bool, pair<Point3m, Point3m>> _data)
{
	for (auto& node : this->ctrl_nodes_)
	{
		node->setSpaceConstraints(_data);
	}
}
