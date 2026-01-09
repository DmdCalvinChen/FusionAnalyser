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

#include "bilateralfiltercsr.h"

BilateralFilterCSR::BilateralFilterCSR(vector<CloudNode> *_nodes) : CloudSurfaceReconstruction(_nodes)
{
}

BilateralFilterCSR::~BilateralFilterCSR()
{
}

float BilateralFilterCSR::getDistanceValue(Point3m& _p)
{
	int ksize = K_SIZE;

	vector<int> k_list = this->collectNearsetVertIndexs(_p, ksize);
	Point3m fit_normal = this->fittingNormal(_p, k_list);
	float a = 0, b = 0;
	for (auto& index : k_list)
	{
		Point3m near_p = this->p_nodes_->at(index).pos();
		std::pair<float, float> element = computeOnceElement(_p, near_p, fit_normal);
		a += element.first;
		b += element.second;
	}
	return a / b;
}

float BilateralFilterCSR::getFunctionValue(Point3m &_p, Point3m &_fit_n, Point3m& _color, bool& _too_faraway, float _thickness)
{
	int ksize = K_SIZE;

	vector<int> k_list = this->collectNearsetVertIndexs(_p, ksize);
	Point3m fit_normal = this->fittingNormal(_p, k_list);
	_fit_n = fit_normal;
	_color = this->fittingColor(k_list);

	if (this->isOutlier2(_p, k_list, _thickness))
	{
		if (_thickness == 0)
		{
			_too_faraway = true;
		}
		else
		{
			_too_faraway = false;
		}
		return 1000;
	}

	_too_faraway = false;
	float a = 0, b = 0;
	for (auto &index : k_list)
	{
		Point3m near_p = this->p_nodes_->at(index).pos();
		std::pair<float, float> element = computeOnceElement(_p, near_p, fit_normal);
		a += element.first;
		b += element.second;
	}
	return a / b;
}

float BilateralFilterCSR::getFunctionValue2(Point3m &_p, Point3m _n, Point3m& _color)
{
	int ksize = 15;
	vector<int> k_list = this->collectNearsetVertIndexs(_p, ksize);
	Point3m fit_normal = _n;
	_color = this->fittingColor(k_list);

	float a = 0, b = 0;
	for (auto &index : k_list)
	{
		Point3m near_p = this->p_nodes_->at(index).pos();
		std::pair<float, float> element = computeOnceElement(_p, near_p, fit_normal);
		a += element.first;
		b += element.second;
	}
	return a / b;
}

float BilateralFilterCSR::getFunctionValue3(Point3m _p, Point3m _n, vector<Point3m>& _neigh_verts)
{
	Point3m fit_normal = _n;
	float a = 0, b = 0;
	for (auto& near_p : _neigh_verts)
	{
		std::pair<float, float> element = computeOnceElement(_p, near_p, fit_normal);
		a += element.first;
		b += element.second;
	}
	return a / b;
}

bool BilateralFilterCSR::isOutlier(Point3m& _p, Point3m _n)
{
	int ksize = 5;
	vector<int> k_list = this->collectNearsetVertIndexs(_p, ksize);

	int sum = 0;
	for (auto& index : k_list)
	{
		Point3m near_p = this->p_nodes_->at(index).pos();
		Point3m vec = near_p - _p;
		float distance_square = vec * vec;
		if (distance_square <= 0.09)
		{
			sum += 1;
		}
	}
	if (sum >= 3)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool BilateralFilterCSR::isOutlier2(Point3m& _p, const vector<int>& _k_list, float _range)
{
	float cur_range;
	if (_range == 0)
	{
		cur_range = 0.5;
	}
	else
	{
		cur_range = _range * _range;
	}

	int sum = 0;
	for (auto& index : _k_list)
	{
		Point3m near_p = this->p_nodes_->at(index).pos();
		Point3m vec = near_p - _p;
		float distance_square = vec * vec;
		if (distance_square <= cur_range)
		{
			sum += 1;
			if (sum >= 3)
			{
				return false;
			}
		}
	}
	return true;
}

void BilateralFilterCSR::sourceDenoising(int _time)
{
	int i_time = 0;
	while (i_time < _time)
	{
		for (int i = 0; i < this->p_nodes_->size(); ++i)
		{
			nodeDenoising(p_nodes_->at(i));
		}
		++i_time;
		this->initalKdTree();
	}
}

void BilateralFilterCSR::nodeDenoising(CloudNode &_node)
{
	Point3m pos = _node.pos();
	Point3m normal = _node.normal();
	Point3m color = _node.color();
	float adj_d = getFunctionValue2(pos, normal, color);
	Point3m adj_p = pos + normal * adj_d;
	_node.setPos(adj_p);
}

Point3m BilateralFilterCSR::fittingNormal(Point3m &_p, vector<int> &_K)
{
	Point3m normal(0, 0, 0), temp_edge;
	float radius, temp_w, sum = 0;
	for (auto &i : _K)
	{
		temp_edge = _p - p_nodes_->at(i).pos();
		radius = sqrtf(temp_edge * temp_edge);
		temp_w = Wn(radius);
		sum += temp_w;
		normal += p_nodes_->at(i).normal() * temp_w;
	}
	normal = normal / sum;
	normal = normal.Normalize();
	return normal;
}

Point3m BilateralFilterCSR::fittingColor(vector<int>& _K)
{
	Point3m color(0, 0, 0);
	for (auto& i : _K)
	{
		color += p_nodes_->at(i).color();
	}
	color /= (float)(_K.size());
	return color;
}

std::pair<float, float> BilateralFilterCSR::computeOnceElement(Point3m &_p, Point3m &_near_p, Point3m &_normal)
{
	Point3m vec = _p - _near_p;
	float len = sqrtf(vec * vec);
	float dot = vec * _normal;
	float wc = Wc(len);
	float ws = Ws(dot);
	return make_pair(wc * ws * dot, wc * ws);
}

float BilateralFilterCSR::Wn(float &_r)
{
	return 1 / _r;
}

float BilateralFilterCSR::Wc(float &_r)
{
	return expf((_r * _r) / (2 * SIGMA_C));
}

float BilateralFilterCSR::Ws(float &_r)
{
	return expf((_r * _r) / (2 * SIGMA_S));
}
