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

#ifndef CLOUDNODE_H
#define CLOUDNODE_H

#define MAX_SHOW_RADIUS 30.0f
#define DEFAULT_COLOR  Point3m(255, 251, 240)
#define DEFAULT_COLOR2  Point3m(123, 219, 36)

#include <GL/glew.h>
#include "common_ext/meshExt/AbsMesh.h"
#include "common_ext/meshExt/Tooth.h"
#include <wrap/gl/addons.h>
#include "camerastate.h"

class HBall
{
public:
	HBall(Point3m _p, float _r)
	{
		p_ = _p;
		r_ = _r;
	}
	HBall(Point3m _p, Point3m _n, Point3m _tan, float _r, Point3m _true_color, Point3m _default_color, bool _use_truecolor)
	{
		p_ = _p;
		n_ = _n;
		tan_vec_ = _tan;
		r_ = _r;
		n_angle_ = 0.5f;
		true_color_ = _true_color;
		default_color_ = _default_color;
		z_ = 0;
		useTrueColor(_use_truecolor);
	}
	HBall(vector<HBall*> _balls, bool _use_truecolor)
	{
		true_color_ = Point3m(0, 0, 0);
		p_ = Point3m(0, 0, 0);
		n_ = Point3m(0, 0, 0);
		tan_vec_ = Point3m(0, 0, 0);
		for (int i = 0; i < _balls.size(); ++i)
		{
			if (_balls[i] != nullptr)
			{
				p_ += _balls[i]->p_;
				n_ += _balls[i]->n_;
				tan_vec_ += _balls[i]->tan_vec_;
				true_color_ += _balls[i]->true_color_;
			}
		}
		p_ /= float(_balls.size());
		n_ /= float(_balls.size());
		tan_vec_ /= float(_balls.size());
		n_ = n_.Normalize();
		tan_vec_ = tan_vec_.Normalize();
		true_color_ /= float(_balls.size());
		r_ = 0;
		for (int i = 0; i < _balls.size(); ++i)
		{
			if (_balls[i] != nullptr)
			{
				float temp_r = sqrtf((p_ - _balls[i]->p_) * (p_ - _balls[i]->p_)) + _balls[i]->r_;
				if (temp_r > r_)
				{
					r_ = temp_r;
				}
			}
		}
		n_angle_ = 0.5f;
		z_ = 0;
		useTrueColor(_use_truecolor);
	}
	~HBall() {}

	inline void setProjectRadius(float _r)
	{
		proj_radius_ = _r;
	}
	inline void setScreenPos(Point2f _p)
	{
		screen_pos_ = _p;
	}
	inline void setDepthZ(float _z)
	{
		z_ = _z;
	}
	inline void setNdcPos(Point4f _p)
	{
		ndc_pos_ = _p;
	}
	inline float projRadius() { return proj_radius_; }
	inline double depthZ() { return z_; }
	inline Point2f projPos() { return screen_pos_; }
	inline Point4f ndcPos() { return ndc_pos_; }

	void draw(vector<float>& _elements, CameraState& _cam, float* _matrix);
	void appendTo(vector<float>& _vec, Point3m& _pos, Point3m& _normal, Point3m& _color, float& _radius, float* _matrix, float _z);
	bool intersectWithBall(HBall& _ball, float* _m);
	void useTrueColor(bool _yes);
public:
	Point3m p_;//position
	Point3m n_;//normal
	Point3m tan_vec_;
	float r_;//radius
	float n_angle_;//normal angle
	Point3m color_, true_color_, default_color_;//color

private:
	float proj_radius_ = 0;//ball投影在屏幕上的半径
	float z_ = -1;//深度
	Point4f ndc_pos_;
	Point2f screen_pos_;
};

class COMMON_EXT_EXPORT CloudNode
{
public:
	enum NodeType {PEAK = 0, RIDGE, SADDLE_RIDGE, FLAT, MINIMAL, PIT, VALLEY, SADDLE_VALLEY, NONENODE};
	CloudNode();
	CloudNode(int _index, Point3m _pos);
	CloudNode(int _index, Point3m _pos, Point3m _normal);
	CloudNode(int _index, Point3m _pos, Point3m _normal, Point3m _color);
	~CloudNode();

	inline int index() { return index_; }
	inline Point3m pos() { return pos_; }
	inline Point3m normal() { return normal_; }
	inline Point3m color() { return color_; }
	inline vector<int> neighbor() { return K_neighbor_; }
	inline bool visited() { return b_visited_; }
	inline bool isBoundary() { return b_boundary_; }
	inline float K() { return K_; }
	inline float H() { return H_; }
	inline float proximityValue() { return proximity_; }
	inline float proximityFeature() { return b_proximity_feature_; }
	inline int tempSign() {return temp_sign_;}
	inline int nodeType() { return node_type_; }
	inline HBall* nodeBall() { return node_ball_; }

	inline void setPos(Point3m _pos) { pos_ = _pos; }
	inline void setIndex(int _index) { index_ = _index; }
	inline void setNeighbor(vector<int> _list) { K_neighbor_ = _list; }
	inline void setNormal(Point3m _n) { normal_ = _n; }
	inline void setNormalVisible(bool _state) { b_draw_normal_ = _state; };
	inline void setK(float _value) { K_ = _value; }
	inline void setH(float _value) { H_ = _value; }
	inline void setProximityValue(float _value) { proximity_ = _value; }
	inline void setBallMode(bool _state) { b_ball_ = _state; }
	inline void setVisited(bool _state) { b_visited_ = _state; }
	inline void setBoundary(bool _state) { b_boundary_ = _state; }
	inline void setProximityFeature(bool _state) { b_proximity_feature_ = _state; }
	inline void setColor(Point3m _c) { color_ = _c; }
	inline void setTempSign(int _v) { temp_sign_ = _v; }
	inline void setLeafBallPointer(HBall* _ball) { node_ball_ = _ball; }
	void setColor(float _u, float _v, QImage *_img);

	void draw();
	void getNodeType();
	void getColorWithCurvature(float _max, float _min);
private:
	int index_;
	Point3m pos_;
	Point3m normal_;
	float K_, H_;//高斯曲率，平均曲率
	vector<int> K_neighbor_;//K个邻近点的索引
	NodeType node_type_;
	Point3m color_;
	bool b_draw_normal_ = false;
	bool b_ball_ = false;
	bool b_visited_ = false;
	bool b_proximity_feature_ = false;
	bool b_boundary_ = false;
	float proximity_;
	int temp_sign_;
	HBall* node_ball_ = nullptr;
};

#endif // !CLOUDNODE_H
