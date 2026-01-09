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

#include "cloudnode.h"

void HBall::draw(vector<float>& _elements, CameraState& _cam, float* _matrix)
{
	if (proj_radius_ > MAX_SHOW_RADIUS)
	{
		return;
	}

	Point3m col(color_.X() / 255.0f, color_.Y() / 255.0f, color_.Z() / 255.0f);
	appendTo(_elements, p_, n_, col, r_, _matrix, z_);
}

void HBall::appendTo(vector<float>& _vec, Point3m& _pos, Point3m& _normal, Point3m& _color, float& _radius, float* _matrix, float _z)
{
	_vec.push_back(_z);
	if (_matrix == nullptr)
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				if (i == j)
				{
					_vec.push_back(1);
				}
				else
				{
					_vec.push_back(0);
				}
			}
		}
	}
	else
	{
		for (int i = 15; i >= 0; --i)
		{
			_vec.push_back(_matrix[i]);
		}
	}

	_vec.push_back(_radius);
	_vec.push_back(_color.Z()); _vec.push_back(_color.Y()); _vec.push_back(_color.X());
	_vec.push_back(_normal.Z()); _vec.push_back(_normal.Y()); _vec.push_back(_normal.X());
	_vec.push_back(_pos.Z()); _vec.push_back(_pos.Y()); _vec.push_back(_pos.X());
}

bool HBall::intersectWithBall(HBall& _ball, float* _m)
{
	if (_m == nullptr)
	{
		return true;
	}

	Point4m pp(p_.X(), p_.Y(), p_.Z(), 1);
	vcg::Matrix44f m(_m);
	pp = m * pp;
	Point3m actual_pos(pp.X(), pp.Y(), pp.Z());

	vcg::Box3f self_box(actual_pos, this->r_ * 1.73205f);
	vcg::Box3f other_box(_ball.p_, _ball.r_ * 1.73205f);
	return self_box.Collide(other_box);
}

void HBall::useTrueColor(bool _yes)
{
	if (_yes)
	{
		color_ = true_color_;
	}
	else
	{
		color_ = default_color_;
	}
}

CloudNode::CloudNode()
{
}

CloudNode::CloudNode(int _index, Point3m _pos)
{
	this->index_ = _index;
	this->pos_ = _pos;
	this->color_ = Point3m(255, 251, 240);
}

CloudNode::CloudNode(int _index, Point3m _pos, Point3m _normal)
{
	this->index_ = _index;
	this->pos_ = _pos;
	this->normal_ = _normal;
	this->color_ = Point3m(255, 251, 240);
}

CloudNode::CloudNode(int _index, Point3m _pos, Point3m _normal, Point3m _color) : CloudNode(_index, _pos, _normal)
{
	this->color_ = _color;
}

CloudNode::~CloudNode()
{
}

void CloudNode::draw()
{
	if (!b_draw_normal_)
	{
		if (b_ball_)
		{
			glPushMatrix();
			glEnable(GL_LIGHTING);
			glColor3f(color_.X(), color_.Y(), color_.Z());
			vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(pos_, 0.1f);
			glDisable(GL_LIGHTING);
			glPopMatrix();
		}
		else if (b_proximity_feature_)
		{
			glPushMatrix();
			glEnable(GL_LIGHTING);
			glColor3f(color_.X(), color_.Y(), color_.Z());
			vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(pos_, 0.1f);
			glDisable(GL_LIGHTING);
			glPopMatrix();
		}
		else
		{
			glPushMatrix();
			glDisable(GL_LIGHTING);
			glColor3f(color_.X(), color_.Y(), color_.Z());
			glPointSize(2.0f);
			glBegin(GL_POINTS);
			glVertex3f(pos_.X(), pos_.Y(), pos_.Z());
			glEnd();
			glPopMatrix();
		}
	}
	if (b_draw_normal_)
	{
		glPushMatrix();
		glDisable(GL_LIGHTING);
		glLineWidth(1.0f);
		glBegin(GL_LINES);
		glColor3f(color_.X(), color_.Y(), color_.Z());
		glVertex3f(pos_.X(), pos_.Y(), pos_.Z());
		glColor3f(1 - color_.X(), 1 - color_.Y(), 1 - color_.Z());
		glVertex3f(pos_.X() + normal_.X(), pos_.Y() + normal_.Y(), pos_.Z() + normal_.Z());
		glEnd();
		glPopMatrix();
	}
}

void CloudNode::getNodeType()
{
	if (K_ > 0)
	{
		if (H_ > 0)
		{
			node_type_ = PIT;
			this->color_ = Point3m(1, 1, 1);
		}
		else if (H_ == 0)
		{
			node_type_ = NONENODE;
		}
		else if (H_ < 0)
		{
			node_type_ = PEAK;
			this->color_ = Point3m(0, 1, 5 / 255.0f);
		}
	}
	else if (K_ == 0)
	{
		if (H_ > 0)
		{
			node_type_ = VALLEY;
			this->color_ = Point3m(6/255.0f, 250/ 255.0f, 254/ 255.0f);
		}
		else if (H_ == 0)
		{
			node_type_ = FLAT;
			this->color_ = Point3m(1,1,0);
		}
		else if (H_ < 0)
		{
			node_type_ = RIDGE;
			this->color_ = Point3m(1, 0, 0);
		}
	}
	else if (K_ < 0)
	{
		if (H_ > 0)
		{
			node_type_ = SADDLE_VALLEY;
			this->color_ = Point3m(254/255.0f, 0, 251/ 255.0f);
		}
		else if (H_ == 0)
		{
			node_type_ = MINIMAL;
			this->color_ = Point3m(0,0,1);
		}
		else if (H_ < 0)
		{
			node_type_ = SADDLE_RIDGE;
			this->color_ = Point3m(0, 0, 0);
		}
	}
}

void CloudNode::getColorWithCurvature(float _max, float _min)
{
	float t;
	t = (H_ - _min) / (_max - _min);
	this->color_ = Point3m(t, 0, (1 - t));
}

void CloudNode::setColor(float _u, float _v, QImage *_img)
{
	if (_img)
	{
		int img_width = _img->width();
		int img_height = _img->height();
		int x = img_width * _u;
		int y = img_height * _v;
		QColor cur_col = QColor(_img->pixel(QPoint(x, y)));

		color_ = Point3m(cur_col.blue(), cur_col.green(), cur_col.red());
	}
}
