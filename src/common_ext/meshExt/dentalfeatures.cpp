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

#include "dentalfeatures.h"

ToothFeatures::ToothFeatures(int _fdi)
{
	this->fdi_ = _fdi;
	axis_ready_ = false;
	all_mark_ready_ = false;

	int dental_sign = fdi_ / 10;
	int tooth_sign = fdi_ % 10;
	if (tooth_sign == 1 || tooth_sign == 2 || tooth_sign == 3)
	{
		marks_ = vector<Point3m>(5, Point3m(0, 0, 0));
	}
	else if (tooth_sign == 4 || tooth_sign == 5)
	{
		marks_ = vector<Point3m>(6, Point3m(0, 0, 0));
	}
	else if (tooth_sign == 6)
	{
		if (dental_sign == 1 || dental_sign == 2)
		{
			marks_ = vector<Point3m>(11, Point3m(0, 0, 0));
		}
		else
		{
			marks_ = vector<Point3m>(13, Point3m(0, 0, 0));
		}
	}
	else if (tooth_sign == 7)
	{
		marks_ = vector<Point3m>(11, Point3m(0, 0, 0));
	}
}

ToothFeatures::ToothFeatures(int _fdi, vector<Point3m> &_marks, vector<Point3m> &_boundary)
{
	this->fdi_ = _fdi;
	this->marks_ = _marks;
	this->boundary_ = _boundary;

	axis_ready_ = false;
	all_mark_ready_ = true;

	computeAxis();
	updateWidthEdge();
}

ToothFeatures::ToothFeatures(int _fdi, std::map<QString, sFeaturePoint>&marks)
{
	fdi_ = _fdi;
	for (auto& mark : marks)
	{
		feature_marks_[mark.second.mark_name_] = mark.second;
	}
}

void ToothFeatures::initalWithPointList(vector<pair<int, Point3m>>& _point_list)
{
	if (this->marks_.empty())
	{
		return;
	}

	int mark_size = marks_.size();
	int tooth_sign = fdi_ % 10;
	int dental_sign = fdi_ / 10;

	for (auto& record : _point_list)
	{
		int index = record.first;
		Point3m point = record.second;
		if (0 <= index && index <= mark_size)
		{
			this->marks_[index] = point;
		}
	}

	if (tooth_sign == 6 || tooth_sign == 7)
	{
		supplyEstimateMarks();
	}
}

void ToothFeatures::initialWithThreePoint(Point3m _near_midpoint, Point3m _far_midpoint, Point3m _midpoint)
{
	if (this->marks_.empty())
	{
		return;
	}

	int tooth_sign = fdi_ % 10;
	int dental_sign = fdi_ / 10;
	if (1 <= tooth_sign && tooth_sign <= 3)
	{
		this->marks_[0] = _near_midpoint;
		this->marks_[1] = _far_midpoint;
		this->marks_[2] = _midpoint;
	}
	else if (1 <= tooth_sign && tooth_sign <= 3)
	{
		this->marks_[2] = _near_midpoint;
		this->marks_[3] = _far_midpoint;
		this->marks_[0] = _midpoint;
	}
	else if (tooth_sign == 6)
	{
		if (dental_sign == 1 || dental_sign == 2)
		{
			this->marks_[9] = _near_midpoint;
			this->marks_[10] = _far_midpoint;
			this->marks_[1] = _midpoint;
		}
		else
		{
			this->marks_[11] = _near_midpoint;
			this->marks_[12] = _far_midpoint;
			this->marks_[1] = _midpoint;
		}
	}
	else if (tooth_sign == 7)
	{
		this->marks_[9] = _near_midpoint;
		this->marks_[10] = _far_midpoint;
		this->marks_[1] = _midpoint;
	}

	if (tooth_sign == 6 || tooth_sign == 7)
	{
		supplyEstimateMarks();
	}
}

void ToothFeatures::supplyEstimateMarks()
{
	int tooth_sign = fdi_ % 10;
	int dental_sign = fdi_ / 10;
	Point3m near_mid, far_mid, far_peak;
	if (tooth_sign == 6)
	{
		if (dental_sign == 1 || dental_sign == 2)
		{
			near_mid = marks_[9];
			far_mid = marks_[10];
			far_peak = marks_[1];
		}
		else
		{
			near_mid = marks_[11];
			far_mid = marks_[12];
			far_peak = marks_[1];
		}
	}
	else
	{
		near_mid = marks_[9];
		far_mid = marks_[10];
		far_peak = marks_[1];
	}

	Point3m center, near_peak, central_sulcus;
	center = (near_mid + far_mid) * 0.5f;
	Point3m vec1 = near_mid - far_mid;
	Point3m vec2 = far_peak - far_mid;
	Point3m vec3 = far_peak - center;
	Point3m normal = vec2 ^ vec1;
	Point3m z_vec, noraml;
	if (dental_sign == 2 || dental_sign == 4)
	{
		normal = vec2 ^ vec1;
		z_vec = (vec1 ^ normal).Normalize();
	}
	else
	{
		normal = vec1 ^ vec2;
		z_vec = (normal ^ vec1).Normalize();
	}
	float extand_value = abs(vec3 * z_vec);
	central_sulcus = center + z_vec * extand_value;
	near_peak = central_sulcus * 2.0f - far_peak;

	if (tooth_sign == 6)
	{
		if (dental_sign == 1 || dental_sign == 2)
		{
			marks_[6] = center;
			marks_[4] = central_sulcus;
			marks_[0] = near_peak;
		}
		else
		{
			marks_[8] = center;
			marks_[5] = central_sulcus;
			marks_[0] = near_peak;
		}
	}
	else
	{
		marks_[6] = center;
		marks_[4] = central_sulcus;
		marks_[0] = near_peak;
	}
}

void ToothFeatures::setWidthEdge(Point3m& _a, Point3m& _b)
{
	this->width_edge_ = FEdge(_a, _b);

	if (p_accompany_edge_)
	{
		p_accompany_edge_->vertA = _a;
		p_accompany_edge_->vertB = _b;
	}

	Point3m vec = _a - _b;
	float width = sqrtf(vec * vec);
	width = (int)(width * 100) / 100.f;
	this->f_tooth_width_ = width;
}

void ToothFeatures::setAccompanyEdgePointer(FEdge* _edge)
{
	p_accompany_edge_ = _edge;
}

void ToothFeatures::updateWidthEdge()
{
	if (!marks_.empty())
	{
		Point3m a(0, 0, 0), b(0, 0, 0);
		int tooth_sign = fdi_ % 10;
		int dental_sign = fdi_ / 10;
		if (1 <= tooth_sign && tooth_sign <= 3)
		{
			a = marks_[0];
			b = marks_[1];
		}
		else if (4 <= tooth_sign && tooth_sign <= 5)
		{
			a = marks_[2];
			b = marks_[3];
		}
		else if (tooth_sign == 6)
		{
			if (dental_sign == 1 || dental_sign == 2)
			{
				a = marks_[9];
				b = marks_[10];
			}
			else
			{
				a = marks_[11];
				b = marks_[12];
			}
		}
		else if (tooth_sign == 7)
		{
			a = marks_[9];
			b = marks_[10];
		}

		if (!boundary_.empty())
		{
			if (tooth_sign == 8)
			{
				a = boundary_[0];
				b = boundary_[boundary_.size() / 2];
				setWidthEdge(a, b);
			}
			else
			{
				if (axis_ready_)
				{
					Point3m nearest_a, nearest_b, normal;
					normal = (b - a) ^ axis_.axisYVector;
					boundaryIntersectionWithXYPlane((a + b) * 0.5f, normal, a, b, nearest_a, nearest_b);
				}
				else
				{
					setWidthEdge(a, b);
				}
			}
		}
		else
		{
			setWidthEdge(a, b);
		}
	}
	else
	{
		if (!boundary_.empty())
		{
			Point3m a = boundary_[0];
			Point3m b = boundary_[boundary_.size() / 2];
			setWidthEdge(a, b);
		}
		else
		{
			Point3m a(0, 0, 0);
			setWidthEdge(a, a);
		}
	}
}

bool ToothFeatures::boundaryIntersectionWithXYPlane(Point3m _pos, Point3m _normal, Point3m _va, Point3m _vb, Point3m& _v1, Point3m& _v2)
{
	Point3m a, b;
	Point3m vec_a, vec_b;
	float proj_value_a, proj_value_b;
	bool find_first_node = false;
	vector<Point3m> candidate_verts;
	for (int i = 0; i < boundary_.size(); ++i)
	{
		if (i == boundary_.size() - 1)
		{
			a = boundary_[i];
			b = boundary_[0];
		}
		else
		{
			a = boundary_[i];
			b = boundary_[i + 1];
		}

		vec_a = a - _pos;
		proj_value_a = vec_a * _normal;
		vec_b = b - _pos;
		proj_value_b = vec_b * _normal;

		if (proj_value_a * proj_value_b < 0)
		{
			candidate_verts.push_back((a + b) * 0.5f);
		}
	}

	_v1 = findNearestPoint(_va, candidate_verts);
	_v2 = findNearestPoint(_vb, candidate_verts);

	int tooth_sign = fdi_ % 10;
	if (1 <= tooth_sign && tooth_sign <= 5)
	{
		Point3m vec = _v2 - _v1;
		float proj_len = vec * this->axis_.axisXVector;
		_v2 = _v1 + this->axis_.axisXVector * proj_len;
	}

	this->setWidthEdge(_v1, _v2);
	return true;
}

Point3m ToothFeatures::findNearestPoint(Point3m _p, vector<Point3m>& _vert_list)
{
	float min_value = FLT_MAX;
	Point3m result = _p;
	Point3m vec;
	for (auto& v : _vert_list)
	{
		vec = v - _p;
		float value = vec * vec;
		if (value < min_value)
		{
			min_value = value;
			result = v;
		}
	}

	vector<Point3m>::iterator ite = _vert_list.begin();
	while (ite != _vert_list.end())
	{
		if (*ite == result)
		{
			_vert_list.erase(ite);
			break;
		}
		else
		{
			++ite;
		}
	}
	return result;
}

void ToothFeatures::computeAxis()
{
	if (this->marks_.empty())
	{
		return;
	}
	if (!all_mark_ready_)
	{
		return;
	}

	axis_ready_ = true;
	int i_jaw = fdi_ / 10;
	int i_tooth = fdi_ % 10;
	if (i_tooth == 1 || i_tooth == 2 || i_tooth == 3)
	{
		analysisIncisorAndCannies();
	}
	else if (i_tooth == 4 || i_tooth == 5)
	{
		analysisPremolars();
	}
	else
	{
		analysisMolars();
	}

	this->fdi_show_pos_ = axis_.centerPoint;
	this->fdi_show_normal_ = axis_.axisZVector;
}

void ToothFeatures::analysisIncisorAndCannies()
{
	int fdi = fdi_;
	Point3m p0 = marks_[0];
	Point3m p1 = marks_[1];
	Point3m margin_mid_point = marks_[2];
	Point3m labial_mid_point = marks_[3];
	Point3m lingual_mid_point = marks_[4];
	Point3m axis_x;
	if (fdi / 10 == 2 || fdi / 10 == 4)
	{
		axis_x = (p0 - p1).Normalize();
	}
	else
	{
		axis_x = (p1 - p0).Normalize();
	}
	Point3m axis_y = (margin_mid_point - (labial_mid_point + lingual_mid_point) * 0.5f).Normalize();
	Point3m axis_z = (axis_x ^ axis_y).Normalize();
	axis_x = (axis_y ^ axis_z).Normalize();
	Point3m center = (margin_mid_point + (labial_mid_point + lingual_mid_point) * 0.5f) * 0.5f;
	axis_ = Axis(center, axis_x, axis_y, axis_z);
}

void ToothFeatures::analysisPremolars()
{
	Point3m p0 = marks_[0];
	Point3m p1 = marks_[1];
	Point3m p2 = marks_[2];
	Point3m p3 = marks_[3];
	Point3m p4 = marks_[4];
	Point3m p5 = marks_[5];

	Point3m vec1 = p5 - p4;
	Point3m vec2 = p0 - p4;
	Point3m axis_x = (vec1 ^ vec2).Normalize();
	Point3m axis_y = (((p0 - p4) + (p1 - p5)) * 0.5f).Normalize();
	Point3m axis_z = (axis_x ^ axis_y).Normalize();
	axis_x = (axis_y ^ axis_z).Normalize();
	Point3m center = ((p2 + p3) * 0.5f + (p4 + p5) * 0.5f) * 0.5f;
	axis_ = Axis(center, axis_x, axis_y, axis_z);
}

void ToothFeatures::analysisMolars()
{
	int fdi = fdi_;
	Point3m center_pit, labial_mid_point, lingual_mid_point, labial_ditch, lingual_ditch;
	if (fdi == 36 || fdi == 46)
	{
		center_pit = marks_[8];
		labial_mid_point = marks_[9];
		lingual_mid_point = marks_[10];
		labial_ditch = marks_[5];
		lingual_ditch = marks_[7];
	}
	else
	{
		center_pit = marks_[6];
		labial_mid_point = marks_[7];
		lingual_mid_point = marks_[8];
		labial_ditch = marks_[4];
		lingual_ditch = marks_[5];
	}

	Point3m vec1 = lingual_mid_point - labial_mid_point;
	Point3m vec2 = labial_ditch - labial_mid_point;
	Point3m axis_x = (vec1 ^ vec2).Normalize();
	Point3m axis_y = (((labial_ditch - labial_mid_point) + (lingual_ditch - lingual_mid_point)) * 0.5f).Normalize();
	Point3m axis_z = (axis_x ^ axis_y).Normalize();
	axis_x = (axis_y ^ axis_z).Normalize();
	Point3m center = ((labial_mid_point + lingual_mid_point) * 0.5f + center_pit) * 0.5f;
	axis_ = Axis(center, axis_x, axis_y, axis_z);
}

void ToothFeatures::initialFdiShowParameters(Point3m& _dental_center, Point3m& _dental_plane_normal)
{
	if (all_mark_ready_)
	{
		return;
	}

	if (all_mark_ready_)
	{
		this->fdi_show_pos_ = this->axis_.centerPoint;
		return;
	}

	int tooth_sign = fdi_ % 10;
	int dental_sign = fdi_ / 10;
	if (1 <= tooth_sign && tooth_sign <= 3)
	{
		this->fdi_show_pos_ = marks_[2];
	}
	else if (4 <= tooth_sign && tooth_sign <= 5)
	{
		//this->fdi_show_pos_ = marks_[0];
		this->fdi_show_pos_ =  (marks_[2] + marks_[3]) * 0.5f;
	}
	else if (tooth_sign == 6)
	{
		if (dental_sign == 1 || dental_sign == 2)
		{
			this->fdi_show_pos_ = marks_[6];
		}
		else
		{
			this->fdi_show_pos_ = marks_[8];
		}
	}
	else if (tooth_sign == 7)
	{
		this->fdi_show_pos_ = marks_[6];
	}

	if (1 <= tooth_sign && tooth_sign <= 3)
	{
		this->fdi_show_pos_ += _dental_plane_normal * 4.0f;
		Point3m adj_vec = (_dental_center - fdi_show_pos_).Normalize();
		this->fdi_show_pos_ += adj_vec * 2.0f;
	}
	else if (6 <= tooth_sign && tooth_sign <= 7)
	{
		this->fdi_show_pos_ += _dental_plane_normal * 1.0f;
	}
	this->fdi_show_normal_ = fdi_show_pos_ - _dental_center;
	this->fdi_show_normal_ = fdi_show_normal_.Normalize();
}

DentalFeatures::DentalFeatures():QObject()
{

}

DentalFeatures::DentalFeatures(bool _b_upper, SeparationManager* _mesh, vector<int>& _fdi_list)
{
	this->b_upper_ = _b_upper;
	this->mesh_ = _mesh;
	int isize = _fdi_list.size();
	for (int i = 0; i < isize; ++i)
	{
		this->teeth_list_.push_back(ToothFeatures(_fdi_list[i]));
	}
}

DentalFeatures::DentalFeatures(bool _b_upper, SeparationManager* _mesh, vector<int>& _fdi_list, vector<vector<pair<int, Point3m>>>& _mark_list)
	:DentalFeatures(_b_upper, _mesh, _fdi_list)
{
	if (teeth_list_.size() == _mark_list.size())
	{
		int i = 0;
		for (auto& mark : _mark_list)
		{
			teeth_list_[i].initalWithPointList(mark);
			teeth_list_[i].computeAxis();
			teeth_list_[i].updateWidthEdge();
			++i;
		}
	}
	constructOcclusalPlaneAxis();
	constructSpeePlane();

	if (teeth_list_.size() == _mark_list.size())
	{
		int i = 0;
		for (auto& mark : _mark_list)
		{
			teeth_list_[i].initialFdiShowParameters(this->base_plane_.center, base_plane_.axisZV);
			++i;
		}
	}
}

DentalFeatures::DentalFeatures(bool _b_upper, SeparationManager* _mesh, vector<int>& _fdi_list, vector<vector<Point3m>>& _mark_list, vector<vector<Point3m>>& _boundary_list) : QObject()
{
	this->b_upper_ = _b_upper;
	this->mesh_ = _mesh;
	if (_fdi_list.size() == _mark_list.size() && _mark_list.size() == _boundary_list.size())
	{
		int isize = _fdi_list.size();
		for (int i = 0; i < isize; ++i)
		{
			this->teeth_list_.push_back(ToothFeatures(_fdi_list[i], _mark_list[i], _boundary_list[i]));
		}
	}
	constructOcclusalPlaneAxis();
	constructSpeePlane();
}

DentalFeatures::DentalFeatures(bool isUpper, std::map<QString, CrownInfoSegmentedIntelligent>& marks, SeparationManager* pMesh)
{
	b_upper_ = isUpper;
	mesh_ = pMesh;

	for (auto& toothMark : marks)
	{
		teeth_list_.push_back(ToothFeatures(toothMark.first.toInt(), toothMark.second.land_marks_));
	}

}

void DentalFeatures::setVisible(bool _visible)
{
	this->bVisible_ = _visible;
	if (mesh_)
	{
		mesh_->setVisible(_visible);
	}
}

void DentalFeatures::constructOcclusalPlaneAxis()
{
	Point3m right_node(0, 0, 0), left_node(0, 0, 0), middle_left_node(0, 0, 0), middle_right_node(0, 0, 0);

	int right_last_index, left_last_index;
	int* right_list = nullptr, * left_list = nullptr, * middle_right_list = nullptr, * middle_left_list = nullptr;
	if (this->b_upper_)
	{
		right_list = new int[7]{ 16,17,15,14,13,12,11 };
		left_list = new int[7]{ 26,27,25,24,23,22,21 };
		middle_right_list = new int[7]{11,12,13,14,15,16,17};
		middle_left_list = new int[7]{ 21,22,23,24,25,26,27 };
	}
	else
	{
		right_list = new int[7]{ 37,36,35,34,33,32,31 };
		left_list = new int[7]{ 47,46,45,44,43,42,41 };
		middle_right_list = new int[7]{ 31,32,33,34,35,36,37 };
		middle_left_list = new int[7]{ 41,42,43,44,45,46,47 };
	}

	int found_time = 0;
	for (int i = 0; i < 7; ++i)
	{
		bool found = false;
		for (auto& tooth : teeth_list_)
		{
			if (tooth.fdi_ == right_list[i])
			{
				if (tooth.marks_.empty())
				{
					continue;
				}

				int tooth_sign = tooth.fdi_ % 10;
				if (tooth_sign == 5 || tooth_sign == 4)
				{
					right_node = tooth.marks_[0];
				}
				else
				{
					right_node = tooth.marks_[1];
				}
				found_time += 1;
				found = true;

				right_last_index = tooth.fdi_;
				break;
			}
		}
		if (found)
		{
			break;
		}
	}

	for (int i = 0; i < 7; ++i)
	{
		bool found = false;
		for (auto& tooth : teeth_list_)
		{
			if (tooth.fdi_ == left_list[i])
			{
				if (tooth.marks_.empty())
				{
					continue;
				}

				int tooth_sign = tooth.fdi_ % 10;
				if (tooth_sign == 5 || tooth_sign == 4)
				{
					left_node = tooth.marks_[0];
				}
				else
				{
					left_node = tooth.marks_[1];
				}
				found_time += 1;
				found = true;

				left_last_index = tooth.fdi_;
				break;
			}
		}
		if (found)
		{
			break;
		}
	}

	for (int i = 0; i < 7; ++i)
	{
		bool found = false;
		for (auto& tooth : teeth_list_)
		{
			if (tooth.fdi_ == middle_right_list[i])
			{
				if (tooth.marks_.empty())
				{
					continue;
				}

				middle_right_node = tooth.marks_[2];
				found_time += 1;
				found = true;
				break;
			}
		}
		if (found)
		{
			break;
		}
	}

	for (int i = 0; i < 7; ++i)
	{
		bool found = false;
		for (auto& tooth : teeth_list_)
		{
			if (tooth.fdi_ == middle_left_list[i])
			{
				if (tooth.marks_.empty())
				{
					continue;
				}

				middle_left_node = tooth.marks_[2];
				found_time += 1;
				found = true;
				break;
			}
		}
		if (found)
		{
			break;
		}
	}

	Point3m right_endpoint, left_endpoint;
	int right_endpoint_index, left_endpoint_index;
	int symmetry_index = 7;
	for (int i = 7; i >= 1; --i)
	{
		int found_time = 0;
		for (auto& tooth : teeth_list_)
		{
			if (tooth.fdi_ % 10 == i)
			{
				++found_time;
			}
		}

		if (found_time == 2)
		{
			symmetry_index = i;
			break;
		}
	}
	if (this->b_upper_)
	{
		right_endpoint_index = 10 + symmetry_index;
		left_endpoint_index = 20 + symmetry_index;
	}
	else
	{
		right_endpoint_index = 30 + symmetry_index;
		left_endpoint_index = 40 + symmetry_index;
	}
	for (auto& tooth : teeth_list_)
	{
		if (tooth.fdi_ == right_endpoint_index)
		{
			if (tooth.marks_.empty())
			{
				continue;
			}

			int tooth_sign = tooth.fdi_ % 10;
			if (tooth_sign == 5 || tooth_sign == 4)
			{
				right_endpoint = tooth.marks_[0];
			}
			else
			{
				right_endpoint = tooth.marks_[1];
			}
		}

		if (tooth.fdi_ == left_endpoint_index)
		{
			if (tooth.marks_.empty())
			{
				continue;
			}

			int tooth_sign = tooth.fdi_ % 10;
			if (tooth_sign == 5 || tooth_sign == 4)
			{
				left_endpoint = tooth.marks_[0];
			}
			else
			{
				left_endpoint = tooth.marks_[1];
			}
		}
	}

	Point3m middle_node = (middle_left_node + middle_right_node) * 0.5f;
	Point3m center = (right_endpoint + left_endpoint) * 0.5f;
	if (found_time == 4 && middle_node != center)
	{
		Point3m axis_x = (right_endpoint - center).Normalize();
		Point3m axis_y = (middle_node - center).Normalize();
		Point3m axis_z = (axis_x ^ axis_y).Normalize();
		axis_x = (axis_y ^ axis_z).Normalize();

		Point3m last_point;
		if (left_last_index % 10 > right_last_index % 10)
		{
			last_point = left_node;
		}
		else
		{
			last_point = right_node;
		}
		center = axis_y * ((last_point - center)* axis_y) + center;

		this->base_plane_.center = center;
		this->base_plane_.axisXV = axis_x;
		this->base_plane_.axisYV = axis_y;
		this->base_plane_.axisZV = axis_z;
		base_plane_ready_ = true;
	}
	else
	{
		base_plane_ready_ = false;
	}

	SAFE_DELETE_ARRAY(right_list);
	SAFE_DELETE_ARRAY(left_list);
	SAFE_DELETE_ARRAY(middle_right_list);
	SAFE_DELETE_ARRAY(middle_left_list);
}

void DentalFeatures::constructSpeePlane()
{
	Point3m right_node(0, 0, 0), left_node(0, 0, 0), middle_left_node(0, 0, 0), middle_right_node(0, 0, 0);

	int* right_list = nullptr, * left_list = nullptr, * middle_right_list = nullptr, * middle_left_list = nullptr;
	if (this->b_upper_)
	{
		right_list = new int[7]{ 16,17,15,14,13,12,11 };
		left_list = new int[7]{ 26,27,25,24,23,22,21 };
		middle_right_list = new int[7]{ 11,12,13,14,15,16,17 };
		middle_left_list = new int[7]{ 21,22,23,24,25,26,27 };
	}
	else
	{
		right_list = new int[7]{ 37,36,35,34,33,32,31 };
		left_list = new int[7]{ 47,46,45,44,43,42,41 };
		middle_right_list = new int[7]{ 31,32,33,34,35,36,37 };
		middle_left_list = new int[7]{ 41,42,43,44,45,46,47 };
	}

	int found_time = 0;
	for (int i = 0; i < 7; ++i)
	{
		bool found = false;
		for (auto& tooth : teeth_list_)
		{
			if (tooth.fdi_ == right_list[i])
			{
				if (tooth.marks_.empty())
				{
					continue;
				}

				int tooth_sign = tooth.fdi_ % 10;
				if (tooth_sign == 5 || tooth_sign == 4)
				{
					right_node = tooth.marks_[0];
				}
				else
				{
					right_node = tooth.marks_[1];
				}
				found_time += 1;
				found = true;
				break;
			}
		}
		if (found)
		{
			break;
		}
	}

	for (int i = 0; i < 7; ++i)
	{
		bool found = false;
		for (auto& tooth : teeth_list_)
		{
			if (tooth.fdi_ == left_list[i])
			{
				if (tooth.marks_.empty())
				{
					continue;
				}

				int tooth_sign = tooth.fdi_ % 10;
				if (tooth_sign == 5 || tooth_sign == 4)
				{
					left_node = tooth.marks_[0];
				}
				else
				{
					left_node = tooth.marks_[1];
				}
				found_time += 1;
				found = true;
				break;
			}
		}
		if (found)
		{
			break;
		}
	}

	for (int i = 0; i < 7; ++i)
	{
		bool found = false;
		for (auto& tooth : teeth_list_)
		{
			if (tooth.fdi_ == middle_right_list[i])
			{
				if (tooth.marks_.empty())
				{
					continue;
				}

				middle_right_node = tooth.marks_[2];
				found_time += 1;
				found = true;
				break;
			}
		}
		if (found)
		{
			break;
		}
	}

	for (int i = 0; i < 7; ++i)
	{
		bool found = false;
		for (auto& tooth : teeth_list_)
		{
			if (tooth.fdi_ == middle_left_list[i])
			{
				if (tooth.marks_.empty())
				{
					continue;
				}

				middle_left_node = tooth.marks_[2];
				found_time += 1;
				found = true;
				break;
			}
		}
		if (found)
		{
			break;
		}
	}

	Point3m middle_node = (middle_left_node + middle_right_node) * 0.5f;
	Point3m center = (right_node + left_node) * 0.5f;
	if (found_time == 4 && middle_node != center)
	{
		Point3m axisXV = (right_node - center).Normalize();
		Point3m axisYV = (middle_node - center).Normalize();
		Point3m axisZV = (base_plane_.axisXV ^ base_plane_.axisYV).Normalize();
		Point3m plane_center = center;
		float radius1 = sqrtf((middle_node - plane_center) * (middle_node - plane_center));
		float radius2 = sqrtf((middle_left_node - plane_center) * (middle_left_node - plane_center));
		float radius3 = sqrtf((middle_right_node - plane_center) * (middle_right_node - plane_center));
		float radius = radius2 < radius1 ? radius2 : radius1;
		radius = radius3 < radius ? radius3 : radius;
		radius *= 1.2f;
		this->spee_plane_.initFace2(plane_center, axisZV, axisXV, axisYV, radius);
	}

	SAFE_DELETE_ARRAY(right_list);
	SAFE_DELETE_ARRAY(left_list);
	SAFE_DELETE_ARRAY(middle_right_list);
	SAFE_DELETE_ARRAY(middle_left_list);
}

void DentalFeatures::setDentalArch(float _length, vector<Point3m>& _arch)
{
	this->fDentalArchLength_ = _length;
	this->dentalArch_ = _arch;
}

bool DentalFeatures::eachToothMarksReady()
{

	for (auto& tooth : this->teeth_list_)
	{
		if (tooth.marks_.size() < 3)
		{
			return false;
		}
	}
	return true;
}

bool DentalFeatures::getPreferViewDirect(Point3m& _left_direct, Point3m& _right_direct)
{
	if (this->b_upper_)
	{
		return false;
	}
	if (!this->base_plane_ready_)
	{
		return false;
	}

	bool left_ready, right_ready;
	int fdi_a, fdi_b;
	bool found_a, found_b;
	Point3m node_a, node_b, node_vec;

	left_ready = false;
	fdi_a = 34;
	found_a = false;
	for (auto& tooth : teeth_list_)
	{
		if (tooth.fdi_ == fdi_a && !tooth.marks_.empty())
		{
			found_a = true;
			node_a = tooth.marks_[0];
			break;
		}
	}
	fdi_b = 36;
	found_b = false;
	for (auto& tooth : teeth_list_)
	{
		if (tooth.fdi_ == fdi_b && !tooth.marks_.empty())
		{
			found_b = true;
			node_b = tooth.marks_[0];
			break;
		}
	}
	if (found_a && found_b)
	{
		left_ready = true;
		node_vec = (node_b - node_a).Normalize();
		_left_direct = (node_vec ^ base_plane_.axisZV).Normalize();
	}

	right_ready = false;
	fdi_a = 44;
	found_a = false;
	for (auto& tooth : teeth_list_)
	{
		if (tooth.fdi_ == fdi_a && !tooth.marks_.empty())
		{
			found_a = true;
			node_a = tooth.marks_[0];
			break;
		}
	}
	fdi_b = 46;
	found_b = false;
	for (auto& tooth : teeth_list_)
	{
		if (tooth.fdi_ == fdi_b && !tooth.marks_.empty())
		{
			found_b = true;
			node_b = tooth.marks_[0];
			break;
		}
	}
	if (found_a && found_b)
	{
		right_ready = true;
		node_vec = (node_a - node_b).Normalize();
		_right_direct = (node_vec ^ base_plane_.axisZV).Normalize();
	}

	return right_ready & left_ready;
}
