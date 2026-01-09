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

#include "dentalanalysisdata.h"

MissingToothItem::MissingToothItem(DentalFeatures* _dental, vector<int>& _fdi_list, QString* _context)
    : EditItem(EditItemIndex(MISSING_TOOTH), _context)
{
	setFeatureMode(true);
	if (_dental->b_upper_)
	{
		_fdi_list = { 18, 17, 16, 15, 14, 13, 12, 11, 21, 22, 23, 24, 25, 26, 27, 28 };
	}
	else
	{
		_fdi_list = { 38, 37, 36, 35, 34, 33, 32, 31, 41, 42, 43, 44, 45, 46, 47, 48 };
	}

	for (auto &tooth : _dental->teeth_list_)
	{
		int fdi = tooth.fdi_;
		vector<int>::iterator ite = find(_fdi_list.begin(), _fdi_list.end(), fdi);
		if (ite != _fdi_list.end())
		{
			_fdi_list.erase(ite);
		}
	}
	this->setPrompt(QString("Missing tooth"));
}

TeethWidthItem::TeethWidthItem(DentalFeatures* _dental_feature, QString* _context, QString* _context_first_three, QString* _context_first_six, vector<FEdge>* _ctrl_edges, bool _six_to_six)
    : EditItem(EditItemIndex(TEETH_WIDTH), _context)
{
	six_to_six_ = _six_to_six;
	setFeatureMode(true);
	dental_feature_ = _dental_feature;
	ctrl_edges_ = _ctrl_edges;
	context_first_six_ = _context_first_six;
	context_first_three_ = _context_first_three;

	if (ctrl_edges_ != nullptr && !ctrl_edges_->empty() && ctrl_edges_->size() == dental_feature_->teeth_list_.size())
	{
		for (int i = 0; i < dental_feature_->teeth_list_.size(); ++i)
		{
			int fdi = dental_feature_->teeth_list_[i].fdi_;
			Point3m a = ctrl_edges_->at(i).vertA;
			Point3m b = ctrl_edges_->at(i).vertB;
			if (a == Point3m(0, 0, 0) && a == b){continue;}
			dental_feature_->teeth_list_[i].setAccompanyEdgePointer(&_ctrl_edges->at(i));
			dental_feature_->teeth_list_[i].setWidthEdge(a, b);
		}
	}

	initialUpdateFeatures();
	updateFeatures();
	this->setPrompt(QString("Tooth Width"));
}

void TeethWidthItem::initialUpdateFeatures()
{
	vector<TowableCtrlSystem>().swap(ctrl_obj_list_);
	if (dental_feature_ != nullptr)
	{
		for (int i = 0; i < dental_feature_->teeth_list_.size(); ++i)
		{
			int fdi = dental_feature_->teeth_list_[i].fdi_;

			Point3m a = dental_feature_->teeth_list_[i].width_edge_.vertA;
			Point3m b = dental_feature_->teeth_list_[i].width_edge_.vertB;
			if (a == Point3m(0, 0, 0) && a == b)
			{
				continue;
			}

			Point3m c = (a + b) * 0.5f;
			Point3m plane_normal = -dental_feature_->base_plane_.axisZV;

			vector<Point3m> vert_list;
			vert_list.push_back(a);
			vert_list.push_back(b);

			Point3m visual_direct = (a + b) / 2.0f;
			visual_direct = (visual_direct - dental_feature_->base_plane_.center).Normalize();
			visual_direct -= dental_feature_->base_plane_.axisZV;
			visual_direct /= 2.0f;
			visual_direct = visual_direct.Normalize();

			TowableCtrlSystem ctrl_sys(i, vert_list, c, plane_normal, true, false, true);

			ctrl_sys.setScreenWidthAndHeight(screen_width_, screen_height_);
			ctrl_sys.setVisualAngleDirect(visual_direct);
			this->ctrl_obj_list_.push_back(ctrl_sys);
		}
	}
}

void TeethWidthItem::updateFeatures()
{
	if (p_cur_obj_ != nullptr && dental_feature_ != nullptr)
	{
		p_cur_obj_->update();

		int itooth = p_cur_obj_->iNo_;
		Point3m a = p_cur_obj_->ctrl_nodes_[0]->p_;
		Point3m b = p_cur_obj_->ctrl_nodes_[1]->p_;
		dental_feature_->teeth_list_[itooth].setWidthEdge(a, b);
	}

	int range = six_to_six_ ? 5 : 7;
	float length = 0;
	for (auto &tooth : dental_feature_->teeth_list_)
	{
		if (tooth.fdi_ % 10 <= range)
		{
			length += tooth.f_tooth_width_;
		}
	}
	int len = (length * 100.0f) / (int)1;
	float len2 = len / 100.0f;
	setContext(QString::number(len2));

	float anterior_length = 0;
	for (auto& tooth : dental_feature_->teeth_list_)
	{
		int fdi = tooth.fdi_;
		int tooth_sign = fdi % 10;
		if (tooth_sign <= 3)
		{
			anterior_length += tooth.f_tooth_width_;
		}
	}
	len = (anterior_length * 100.0f) / (int)1;
	len2 = len / 100.0f;
	if (context_first_three_!= nullptr)
	{
		*context_first_three_ = QString::number(len2);
	}

	float sixTosix_length = anterior_length;
	for (auto& tooth : dental_feature_->teeth_list_)
	{
		int fdi = tooth.fdi_;
		int tooth_sign = fdi % 10;
		if (3 < tooth_sign && tooth_sign <= 6)
		{
			sixTosix_length += tooth.f_tooth_width_;
		}
	}
	len = (sixTosix_length * 100.0f) / (int)1;
	len2 = len / 100.0f;
	if (context_first_six_ != nullptr)
	{
		*context_first_six_ = QString::number(len2);
	}
}

void TeethWidthItem::updateCtrlParts()
{
	if (ctrl_edges_ != nullptr)
	{
		ctrl_edges_->clear();
		if (!ctrl_obj_list_.empty())
		{
			for (int i = 0; i < ctrl_obj_list_.size(); ++i)
			{
				ctrl_edges_->push_back(FEdge(ctrl_obj_list_[i].ctrl_nodes_[0]->p_, ctrl_obj_list_[i].ctrl_nodes_[1]->p_));
			}
		}
	}
}

CurrentArchLengthItem::CurrentArchLengthItem(DentalFeatures* _dental_features, QString* _context, vector<Point3m>* _ctrlnodes, vector<Point3m>* _5_5_nodes, vector<Point3m>* _7_7_nodes, QString* _context_complete, bool _six_to_six)
    : EditItem(EditItemIndex(CUR_LENGTH_OF_DENTAL_ARCH), _context)
{
	setFeatureMode(true);
	six_to_six_ = _six_to_six;
	dental_feature_ = _dental_features;
	ctrlnodes_ = _ctrlnodes;
	nodes_5_5_ = _5_5_nodes;
	nodes_7_7_ = _7_7_nodes;
	context_complete_ = _context_complete;
	vector<Point3m> verts, verts2;

	if (six_to_six_)
	{
		int* left_left_fdi_list = nullptr;
		int* left_fdi_list2 = nullptr;
		int* left_fdi_list = nullptr;
		int* left_middle_fdi_list = nullptr;
		int* right_right_fdi_list = nullptr;
		int* right_fdi_list = nullptr;
		int* right_fdi_list2 = nullptr;
		int* right_middle_fdi_list = nullptr;
		int fdi_list[6];
		if (dental_feature_->b_upper_)
		{
			left_left_fdi_list = new int[5]{ 15,14,13,12,11 };
			left_fdi_list2 = new int[5]{ 14,13,12,11,15 };
			left_fdi_list = new int[3]{ 12,11,13 };
			left_middle_fdi_list = new int[6]{ 11,12,13,14,15,16 };
			right_middle_fdi_list = new int[6]{ 21,22,23,24,25,26 };
			right_fdi_list = new int[3]{ 22,21,23, };
			right_fdi_list2 = new int[5]{ 24,23,22,21,25 };
			right_right_fdi_list = new int[5]{ 25,24,23,22,21 };
		}
		else
		{
			left_left_fdi_list = new int[5]{ 35,34,33,32,31 };
			left_fdi_list2 = new int[5]{ 34,33,32,31,35 };
			left_fdi_list = new int[3]{ 32,31,33 };
			left_middle_fdi_list = new int[6]{ 31,32,33,34,35,36 };
			right_middle_fdi_list = new int[6]{ 41,42,43,44,45,46 };
			right_fdi_list = new int[3]{ 42,41,43 };
			right_fdi_list2 = new int[5]{ 44,43,42,41,45 };
			right_right_fdi_list = new int[5]{ 45,44,43,42,41 };
		}

		for (int ii = 0; ii < 5; ++ii)
		{
			bool found = false;
			int cur_fdi = left_left_fdi_list[ii];
			for (auto& tooth : dental_feature_->teeth_list_)
			{
				if (tooth.fdi_ == cur_fdi)
				{
					Point3m vert;
					int tooth_sign = cur_fdi % 10;
					int dental_sign = cur_fdi / 10;

					if (tooth_sign == 7)
					{
						vert = tooth.marks_[10];
					}
					else if (tooth_sign == 6)
					{
						if (dental_sign == 1 || dental_sign == 2)
						{
							vert = tooth.marks_[10];
						}
						else
						{
							vert = tooth.marks_[12];
						}
					}
					else if (4 <= tooth_sign && tooth_sign <= 5)
					{
						vert = tooth.marks_[3];
					}
					else
					{
						vert = tooth.marks_[1];
					}
					verts.push_back(vert);
					found = true;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}

		for (int ii = 0; ii < 5; ++ii)
		{
			bool found = false;
			int cur_fdi = left_fdi_list2[ii];
			for (auto& tooth : dental_feature_->teeth_list_)
			{
				if (tooth.fdi_ == cur_fdi)
				{
					Point3m vert;
					int tooth_sign = cur_fdi % 10;
					int dental_sign = cur_fdi / 10;

					if (4 <= tooth_sign && tooth_sign <= 5)
					{
						if (dental_sign == 1 || dental_sign == 2)
						{
							vert = (tooth.marks_[2] + tooth.marks_[3]) * 0.5f;
						}
						else
						{
							vert = tooth.marks_[0];
						}
					}
					else
					{
						vert = tooth.marks_[2];
					}
					verts.push_back(vert);
					found = true;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}

		for (int ii = 0; ii < 3; ++ii)
		{
			bool found = false;
			int cur_fdi = left_fdi_list[ii];
			for (auto& tooth : dental_feature_->teeth_list_)
			{
				if (tooth.fdi_ == cur_fdi)
				{
					Point3m vert;
					int tooth_sign = cur_fdi % 10;
					int dental_sign = cur_fdi / 10;

					vert = tooth.marks_[2];
					verts.push_back(vert);
					found = true;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}

		for (int ii = 0; ii < 6; ++ii)
		{
			bool found = false;
			int cur_fdi = left_middle_fdi_list[ii];
			for (auto& tooth : dental_feature_->teeth_list_)
			{
				if (tooth.fdi_ == cur_fdi)
				{
					Point3m vert;
					int tooth_sign = cur_fdi % 10;
					int dental_sign = cur_fdi / 10;

					if (tooth_sign == 6)
					{
						if (dental_sign == 1 || dental_sign == 2)
						{
							vert = tooth.marks_[4];
						}
						else
						{
							vert = tooth.marks_[5];
						}
					}
					else
					{
						vert = tooth.marks_[0];
					}
					verts.push_back(vert);
					found = true;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}

		for (int ii = 0; ii < 6; ++ii)
		{
			bool found = false;
			int cur_fdi = right_middle_fdi_list[ii];
			for (auto& tooth : dental_feature_->teeth_list_)
			{
				if (tooth.fdi_ == cur_fdi)
				{
					Point3m vert;
					int tooth_sign = cur_fdi % 10;
					int dental_sign = cur_fdi / 10;

					if (tooth_sign == 6)
					{
						if (dental_sign == 1 || dental_sign == 2)
						{
							vert = tooth.marks_[4];
						}
						else
						{
							vert = tooth.marks_[5];
						}
					}
					else
					{
						vert = tooth.marks_[0];
					}
					verts.push_back(vert);
					found = true;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}

		for (int ii = 0; ii < 3; ++ii)
		{
			bool found = false;
			int cur_fdi = right_fdi_list[ii];
			for (auto& tooth : dental_feature_->teeth_list_)
			{
				if (tooth.fdi_ == cur_fdi)
				{
					Point3m vert;
					int tooth_sign = cur_fdi % 10;
					int dental_sign = cur_fdi / 10;

					vert = tooth.marks_[2];
					verts.push_back(vert);
					found = true;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}

		for (int ii = 0; ii < 5; ++ii)
		{
			bool found = false;
			int cur_fdi = right_fdi_list2[ii];
			for (auto& tooth : dental_feature_->teeth_list_)
			{
				if (tooth.fdi_ == cur_fdi)
				{
					Point3m vert;
					int tooth_sign = cur_fdi % 10;
					int dental_sign = cur_fdi / 10;

					if (4 <= tooth_sign && tooth_sign <= 5)
					{
						if (dental_sign == 1 || dental_sign == 2)
						{
							vert = (tooth.marks_[2] + tooth.marks_[3]) * 0.5f;
						}
						else
						{
							vert = tooth.marks_[0];
						}
					}
					else
					{
						vert = tooth.marks_[2];
					}
					verts.push_back(vert);
					found = true;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}

		for (int ii = 0; ii < 5; ++ii)
		{
			bool found = false;
			int cur_fdi = right_right_fdi_list[ii];
			for (auto& tooth : dental_feature_->teeth_list_)
			{
				if (tooth.fdi_ == cur_fdi)
				{
					Point3m vert;
					int tooth_sign = cur_fdi % 10;
					int dental_sign = cur_fdi / 10;

					if (tooth_sign == 7)
					{
						vert = tooth.marks_[10];
					}
					else if (tooth_sign == 6)
					{
						if (dental_sign == 1 || dental_sign == 2)
						{
							vert = tooth.marks_[10];
						}
						else
						{
							vert = tooth.marks_[12];
						}
					}
					else if (4 <= tooth_sign && tooth_sign <= 5)
					{
						vert = tooth.marks_[3];
					}
					else
					{
						vert = tooth.marks_[1];
					}
					verts.push_back(vert);
					found = true;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}

		SAFE_DELETE_ARRAY(left_left_fdi_list);
		SAFE_DELETE_ARRAY(left_fdi_list2);
		SAFE_DELETE_ARRAY(left_fdi_list);
		SAFE_DELETE_ARRAY(left_middle_fdi_list);
		SAFE_DELETE_ARRAY(right_right_fdi_list);
		SAFE_DELETE_ARRAY(right_fdi_list);
		SAFE_DELETE_ARRAY(right_fdi_list2);
		SAFE_DELETE_ARRAY(right_middle_fdi_list);
	}
	else
	{
		int* left_left_fdi_list = nullptr;
		int* left_fdi_list2 = nullptr;
		int* left_fdi_list = nullptr;
		int* left_middle_fdi_list = nullptr;
		int* right_right_fdi_list = nullptr;
		int* right_fdi_list = nullptr;
		int* right_fdi_list2 = nullptr;
		int* right_middle_fdi_list = nullptr;
		int fdi_list[6];
		if (dental_feature_->b_upper_)
		{
			left_left_fdi_list = new int[7]{ 17,16,15,14,13,12,11 };
			left_fdi_list2 = new int[5]{ 15,14,13,12,11 };
			left_fdi_list = new int[3]{ 13,12,11 };
			left_middle_fdi_list = new int[6]{ 11,12,13,14,15,16 };
			right_middle_fdi_list = new int[6]{ 21,22,23,24,25,26 };
			right_fdi_list = new int[3]{ 23,22,21 };
			right_fdi_list2 = new int[5]{ 25,24,23,22,21 };
			right_right_fdi_list = new int[7]{ 27,26,25,24,23,22,21 };
		}
		else
		{
			left_left_fdi_list = new int[7]{ 37,36,35,34,33,32,31 };
			left_fdi_list2 = new int[5]{ 35,34,33,32,31 };
			left_fdi_list = new int[3]{ 33,32,31 };
			left_middle_fdi_list = new int[6]{ 31,32,33,34,35,36 };
			right_middle_fdi_list = new int[6]{ 41,42,43,44,45,46 };
			right_fdi_list = new int[3]{ 43,42,41 };
			right_fdi_list2 = new int[5]{ 45,44,43,42,41 };
			right_right_fdi_list = new int[7]{ 47,46,45,44,43,42,41 };
		}

		for (int ii = 0; ii < 7; ++ii)
		{
			bool found = false;
			int cur_fdi = left_left_fdi_list[ii];
			for (auto& tooth : dental_feature_->teeth_list_)
			{
				if (tooth.fdi_ == cur_fdi)
				{
					Point3m vert;
					int tooth_sign = cur_fdi % 10;
					int dental_sign = cur_fdi / 10;

					if (tooth_sign == 7)
					{
						vert = tooth.marks_[10];
					}
					else if (tooth_sign == 6)
					{
						if (dental_sign == 1 || dental_sign == 2)
						{
							vert = tooth.marks_[10];
						}
						else
						{
							vert = tooth.marks_[12];
						}
					}
					else if (4 <= tooth_sign && tooth_sign <= 5)
					{
						if (dental_sign == 1 || dental_sign == 2)
						{
							vert = (tooth.marks_[2] + tooth.marks_[3]) * 0.5f;
						}
						else
						{
							vert = tooth.marks_[0];
						}
					}
					else
					{
						vert = tooth.marks_[1];
					}
					verts.push_back(vert);
					found = true;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}

		for (int ii = 0; ii < 5; ++ii)
		{
			bool found = false;
			int cur_fdi = left_fdi_list2[ii];
			for (auto& tooth : dental_feature_->teeth_list_)
			{
				if (tooth.fdi_ == cur_fdi)
				{
					Point3m vert;
					int tooth_sign = cur_fdi % 10;
					int dental_sign = cur_fdi / 10;

					if (4 <= tooth_sign && tooth_sign <= 5)
					{
						if (dental_sign == 1 || dental_sign == 2)
						{
							vert = (tooth.marks_[2] + tooth.marks_[3]) * 0.5f;
						}
						else
						{
							vert = tooth.marks_[0];
						}
					}
					else
					{
						vert = tooth.marks_[2];
					}
					verts.push_back(vert);
					found = true;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}

		for (int ii = 0; ii < 3; ++ii)
		{
			bool found = false;
			int cur_fdi = left_fdi_list[ii];
			for (auto& tooth : dental_feature_->teeth_list_)
			{
				if (tooth.fdi_ == cur_fdi)
				{
					Point3m vert;
					int tooth_sign = cur_fdi % 10;
					int dental_sign = cur_fdi / 10;

					vert = tooth.marks_[2];
					verts.push_back(vert);
					found = true;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}

		for (int ii = 0; ii < 6; ++ii)
		{
			bool found = false;
			int cur_fdi = left_middle_fdi_list[ii];
			for (auto& tooth : dental_feature_->teeth_list_)
			{
				if (tooth.fdi_ == cur_fdi)
				{
					Point3m vert;
					int tooth_sign = cur_fdi % 10;
					int dental_sign = cur_fdi / 10;

					if (tooth_sign == 6)
					{
						if (dental_sign == 1 || dental_sign == 2)
						{
							vert = tooth.marks_[4];
						}
						else
						{
							vert = tooth.marks_[5];
						}
					}
					else
					{
						vert = tooth.marks_[0];
					}
					verts.push_back(vert);
					found = true;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}

		for (int ii = 0; ii < 6; ++ii)
		{
			bool found = false;
			int cur_fdi = right_middle_fdi_list[ii];
			for (auto& tooth : dental_feature_->teeth_list_)
			{
				if (tooth.fdi_ == cur_fdi)
				{
					Point3m vert;
					int tooth_sign = cur_fdi % 10;
					int dental_sign = cur_fdi / 10;

					if (tooth_sign == 6)
					{
						if (dental_sign == 1 || dental_sign == 2)
						{
							vert = tooth.marks_[4];
						}
						else
						{
							vert = tooth.marks_[5];
						}
					}
					else
					{
						vert = tooth.marks_[0];
					}
					verts.push_back(vert);
					found = true;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}

		for (int ii = 0; ii < 3; ++ii)
		{
			bool found = false;
			int cur_fdi = right_fdi_list[ii];
			for (auto& tooth : dental_feature_->teeth_list_)
			{
				if (tooth.fdi_ == cur_fdi)
				{
					Point3m vert;
					int tooth_sign = cur_fdi % 10;
					int dental_sign = cur_fdi / 10;

					vert = tooth.marks_[2];
					verts.push_back(vert);
					found = true;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}

		for (int ii = 0; ii < 5; ++ii)
		{
			bool found = false;
			int cur_fdi = right_fdi_list2[ii];
			for (auto& tooth : dental_feature_->teeth_list_)
			{
				if (tooth.fdi_ == cur_fdi)
				{
					Point3m vert;
					int tooth_sign = cur_fdi % 10;
					int dental_sign = cur_fdi / 10;

					if (4 <= tooth_sign && tooth_sign <= 5)
					{
						if (dental_sign == 1 || dental_sign == 2)
						{
							vert = (tooth.marks_[2] + tooth.marks_[3]) * 0.5f;
						}
						else
						{
							vert = tooth.marks_[0];
						}
					}
					else
					{
						vert = tooth.marks_[2];
					}
					verts.push_back(vert);
					found = true;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}

		for (int ii = 0; ii < 7; ++ii)
		{
			bool found = false;
			int cur_fdi = right_right_fdi_list[ii];
			for (auto& tooth : dental_feature_->teeth_list_)
			{
				if (tooth.fdi_ == cur_fdi)
				{
					Point3m vert;
					int tooth_sign = cur_fdi % 10;
					int dental_sign = cur_fdi / 10;

					if (tooth_sign == 7)
					{
						vert = tooth.marks_[10];
					}
					else if (tooth_sign == 6)
					{
						if (dental_sign == 1 || dental_sign == 2)
						{
							vert = tooth.marks_[10];
						}
						else
						{
							vert = tooth.marks_[12];
						}
					}
					else if (4 <= tooth_sign && tooth_sign <= 5)
					{
						if (dental_sign == 1 || dental_sign == 2)
						{
							vert = (tooth.marks_[2] + tooth.marks_[3]) * 0.5f;
						}
						else
						{
							vert = tooth.marks_[0];
						}
					}
					else
					{
						vert = tooth.marks_[1];
					}
					verts.push_back(vert);
					found = true;
					break;
				}
			}
			if (found)
			{
				break;
			}
		}

		SAFE_DELETE_ARRAY(left_left_fdi_list);
		SAFE_DELETE_ARRAY(left_fdi_list2);
		SAFE_DELETE_ARRAY(left_fdi_list);
		SAFE_DELETE_ARRAY(left_middle_fdi_list);
		SAFE_DELETE_ARRAY(right_right_fdi_list);
		SAFE_DELETE_ARRAY(right_fdi_list);
		SAFE_DELETE_ARRAY(right_fdi_list2);
		SAFE_DELETE_ARRAY(right_middle_fdi_list);
	}

	if (verts.size() == 8)
	{
		verts2.push_back(verts[0]);
		verts2.push_back(verts[1]);
		verts2.push_back(verts[2]);
		verts2.push_back((verts[3] + verts[4]) * 0.5f);
		verts2.push_back(verts[5]);
		verts2.push_back(verts[6]);
		verts2.push_back(verts[7]);
		verts = verts2;
	}

	Point3m plane_pos = dental_feature_->base_plane_.center;
	Point3m plane_normal = dental_feature_->base_plane_.axisZV;
	vector<Point3m> proj_verts, proj_verts1, proj_verts2;
	for (int i = 0; i < verts.size(); ++i)
	{
		if (i != 1 && i != verts.size() - 2)
		{
			proj_verts1.push_back(UtilityTools::getInstance()->getProjPointOnPlane(verts[i], plane_pos, -plane_normal));
		}
	}
	for (int i = 0; i < verts.size(); ++i)
	{
		if (i != 2 && i != verts.size() - 3)
		{
			proj_verts2.push_back(UtilityTools::getInstance()->getProjPointOnPlane(verts[i], plane_pos, -plane_normal));
		}
	}

	if (_5_5_nodes != nullptr && !_5_5_nodes->empty() && six_to_six_)
	{
		verts.clear();
		for (int i = 0; i < _5_5_nodes->size(); ++i)
		{
			verts.push_back(_5_5_nodes->at(i));
		}
	}
	else if (_7_7_nodes != nullptr && !_7_7_nodes->empty() && !six_to_six_)
	{
		verts.clear();
		for (int i = 0; i < _7_7_nodes->size(); ++i)
		{
			verts.push_back(_7_7_nodes->at(i));
		}
	}
	else if (ctrlnodes_ != nullptr && !ctrlnodes_->empty())
	{
		verts.clear();
		for (int i = 0; i < ctrlnodes_->size(); ++i)
		{
			verts.push_back(ctrlnodes_->at(i));
		}
	}

	for (int i = 0; i < verts.size(); ++i)
	{
		proj_verts.push_back(UtilityTools::getInstance()->getProjPointOnPlane(verts[i], plane_pos, -plane_normal));
	}

	this->ctrl_obj_ = new TowableCtrlSystem(0, proj_verts, plane_pos, plane_normal, false, true);
	this->auxiliary_obj1_ = new TowableCtrlSystem(1, proj_verts1, plane_pos, plane_normal, false, true);
	this->auxiliary_obj1_->setAuxiliary(true);
	this->auxiliary_obj2_ = new TowableCtrlSystem(2, proj_verts2, plane_pos, plane_normal, false, true);
	this->auxiliary_obj2_->setAuxiliary(true);
	this->setPrompt(QString("Crowding"));
	updateFeatures();
}

void CurrentArchLengthItem::updateFeatures()
{
	if (ctrl_obj_)
	{
		ctrl_obj_->update();
	}
	int len = (ctrl_obj_->length_ * 100.0f) / (int)1;
	float len2 = len / 100.0f;
	setContext(QString::number(len2));

	vector<Point3m> arch_verts;
	for (auto& edge : ctrl_obj_->edges_)
	{
		arch_verts.push_back(edge.vertA);
	}
	arch_verts.push_back(ctrl_obj_->edges_.back().vertB);
	this->dental_feature_->setDentalArch(len2, arch_verts);

	float length = 0;
	int range = six_to_six_ ? 5 : 7;
	length = 0;
	for (auto& tooth : dental_feature_->teeth_list_)
	{
		int fdi = tooth.fdi_;
		if (fdi % 10 <= range)
		{
			length += tooth.f_tooth_width_;
		}
	}
	len = (length * 100) / (int)1;
	len2 = len / 100.0f;
	if (context_complete_ != nullptr)
	{
		*context_complete_ = QString::number(len2);
	}
}

bool CurrentArchLengthItem::availabilityJudgment()
{
	if (!dental_feature_)
	{
		return false;
	}

	if (dental_feature_->b_upper_)
	{
		bool left = false, right = false;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			int dental_sign = tooth.fdi_ / 10;
			if (dental_sign == 1)
			{
				right = true;
			}
			else if (dental_sign == 2)
			{
				left = true;
			}
		}
		return left & right;
	}
	else
	{
		bool left = false, right = false;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			int dental_sign = tooth.fdi_ / 10;
			if (dental_sign == 4)
			{
				right = true;
			}
			else if (dental_sign == 3)
			{
				left = true;
			}
		}
		return left & right;
	}
}

BoltonItem::BoltonItem(DentalFeatures* _dental_features, QString* _context_anterior, QString* _context, QString* _context_complete, EditItemIndex _item_index, int _tooth_sign, bool _six_to_six)
	: EditItem(_item_index, _context)
{
	six_to_six_ = _six_to_six;
	setFeatureMode(true);
	dental_feature_ = _dental_features;
	context_anterior_ = _context_anterior;
	context_complete_ = _context_complete;
	tooth_sign_range_ = _tooth_sign;
	initialUpdateFeatures();
	updateFeatures();
	this->setPrompt(QString("Bolton Ratio"));
}

void BoltonItem::initialUpdateFeatures()
{
	if (dental_feature_ == nullptr)
	{
		return;
	}
	vector<TowableCtrlSystem>().swap(ctrl_obj_list_);
	int i = 0;
	for (auto &tooth : dental_feature_->teeth_list_)
	{
		int fdi = tooth.fdi_;
		if (fdi % 10 <= tooth_sign_range_)
		{
			Point3m a, b, c, plane_normal;
			a = tooth.width_edge_.vertA;
			b = tooth.width_edge_.vertB;
			plane_normal = -dental_feature_->base_plane_.axisZV;

			vector<Point3m> vert_list;
			vert_list.push_back(a);
			vert_list.push_back(b);

			Point3m visual_direct = (a + b) / 2.0f;
			visual_direct = (visual_direct - dental_feature_->base_plane_.center).Normalize();
			visual_direct -= dental_feature_->base_plane_.axisZV;
			visual_direct /= 2.0f;
			visual_direct = visual_direct.Normalize();

			TowableCtrlSystem ctrl_sys(i, vert_list, c, plane_normal, true, false, true);

			ctrl_sys.setScreenWidthAndHeight(screen_width_, screen_height_);
			ctrl_sys.setVisualAngleDirect(visual_direct);

			this->ctrl_obj_list_.push_back(ctrl_sys);
		}
		++i;
	}
}

void BoltonItem::updateFeatures()
{
	if (p_cur_obj_ != nullptr)
	{
		p_cur_obj_->update();

		int itooth = p_cur_obj_->iNo_;
		if (p_cur_obj_->ctrl_nodes_.size() >= 2)
		{
			Point3m a = p_cur_obj_->ctrl_nodes_[0]->p_;
			Point3m b = p_cur_obj_->ctrl_nodes_[1]->p_;
			dental_feature_->teeth_list_[itooth].setWidthEdge(a, b);
		}
		else
		{
			qDebug() << "error:ctrl node defect" << endl;
		}
	}

	int left_list[3], right_list[3];
	if (dental_feature_->b_upper_)
	{
		int temp_left_arr[3] = { 21,22,23};
		 memcpy(left_list, temp_left_arr, sizeof(temp_left_arr));
		 int temp_right_arr[3] = { 11,12,13 };
		 memcpy(right_list, temp_right_arr, sizeof(temp_right_arr));
	}
	else
	{
		int temp_left_arr[3] = { 31,32,33 };
		memcpy(left_list, temp_left_arr, sizeof(temp_left_arr));
		int temp_right_arr[3] = { 41,42,43 };
		memcpy(right_list, temp_right_arr, sizeof(temp_right_arr));
	}

	float left_tooth_width_list[3], right_tooth_width_list[3];
	for (int i = 0; i < 3; ++i)
	{
		left_tooth_width_list[i] = 0;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			if (tooth.fdi_ == left_list[i])
			{
				left_tooth_width_list[i] = tooth.f_tooth_width_;
			}
		}

		right_tooth_width_list[i] = 0;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			if (tooth.fdi_ == right_list[i])
			{
				right_tooth_width_list[i] = tooth.f_tooth_width_;
			}
		}
	}

	float anterior_length = 0;
	for (int i = 0; i < 3; ++i)
	{
		anterior_length += left_tooth_width_list[i];
		anterior_length += right_tooth_width_list[i];
	}
	int len = (anterior_length * 100.0f) / (int)1;
	float len2 = len / 100.0f;
	if (context_anterior_ != nullptr)
	{
		*context_anterior_ = QString::number(len2);
	}

	float sixTosix_length = anterior_length;
	for (auto& tooth : dental_feature_->teeth_list_)
	{
		int fdi = tooth.fdi_;
		int tooth_sign = fdi % 10;
		if (3 < tooth_sign && tooth_sign <= 6)
		{
			sixTosix_length += tooth.f_tooth_width_;
		}
	}
	len = (sixTosix_length * 100.0f) / (int)1;
	len2 = len / 100.0f;
	setContext(QString::number(len2));

	float length = 0;
	int range = six_to_six_ ? 5 : 7;
	for (auto& tooth : dental_feature_->teeth_list_)
	{
		int fdi = tooth.fdi_;
		if (fdi % 10 <= range)
		{
			length += tooth.f_tooth_width_;
		}
	}
	len = (length * 100) / (int)1;
	len2 = len / 100.0f;
	if (context_complete_ != nullptr)
	{
		*context_complete_ = QString::number(len2);
	}
}

bool BoltonItem::availabilityJudgment()
{
	if (!dental_feature_)
	{
		return false;
	}
	return true;
}

SpeeCurveDepthItem::SpeeCurveDepthItem(DentalFeatures* _dental_features, float* _left_deepth, float* _right_deepth, QString* _context, vector<Point3m>* _ctrlnodes)
    :EditItem(EditItemIndex(SPEE_CURVE_DEPTH), _context)
{
	setFeatureMode(true);
	dental_feature_ = _dental_features;
	ctrlnodes_ = _ctrlnodes;
	left_deepth_ = _left_deepth;
	right_deepth_ = _right_deepth;
	spee_plane_ = dental_feature_->spee_plane_;

	vector<pair<Point3m, vector<CMeshO*>>> left_node_infos, right_node_infos;
	Point3m vert;
	int cur_fdi, tooth_sign;

	if (ctrlnodes_ != nullptr && !ctrlnodes_->empty() && ctrlnodes_->size() == 6)
	{
		for (int ii = 0; ii < 3; ++ii)
		{
			vector<CMeshO*> mesh_list;
			mesh_list.push_back(&dental_feature_->mesh_->original_mesh_->cm);
			vert = ctrlnodes_->at(ii);
			left_node_infos.push_back(make_pair(vert, mesh_list));
		}
	}
	else
	{
		vector<vector<int>> left_indexes_list;
		left_indexes_list.push_back(vector<int>{31, 32, 33, 34, 35, 36, 37});
		left_indexes_list.push_back(vector<int>{34, 35});
		left_indexes_list.push_back(vector<int>{35, 34});
		left_indexes_list.push_back(vector<int>{37, 36, 35, 34, 33, 32, 31});

		for (int ii = 0; ii < 4; ++ii)
		{
			vector<int> cur_search_list = left_indexes_list[ii];
			for (auto& fdi : cur_search_list)
			{
				cur_fdi = fdi;
				tooth_sign = cur_fdi % 10;

				bool found = false;
				for (auto& tooth : dental_feature_->teeth_list_)
				{
					if (tooth.fdi_ == cur_fdi)
					{
						if (tooth_sign == 1 || tooth_sign == 2 || tooth_sign == 3 || tooth_sign == 4 || tooth_sign == 5)
						{
							vert = tooth.marks_[0];
						}
						else if (tooth_sign == 6 || tooth_sign == 7)
						{
							vert = tooth.marks_[1];
						}

						vector<CMeshO*> mesh_list;
						mesh_list.push_back(&dental_feature_->mesh_->original_mesh_->cm);
						left_node_infos.push_back(make_pair(vert, mesh_list));
						found = true;
						break;
					}
				}
				if (found)
				{
					break;
				}
			}
		}

		if (left_node_infos.size() == 4)
		{
			Point3m p0 = left_node_infos.front().first;
			Point3m p3 = left_node_infos.back().first;
			Point3m pos = (p0 + p3) * 0.5f;
			Point3m vec_x = (p0 - p3).Normalize();
			Point3m vec_y = dental_feature_->base_plane_.axisXV;
			Point3m normal = (vec_x ^ vec_y).Normalize();

			float deepest_value = 0;
			int eliminate_index = 1;
			for (int i = 1; i <= 2; ++i)
			{
				FEdge deepest_edge;
				Point3m vert = left_node_infos.at(i).first;
				Point3m proj_vert = UtilityTools::getInstance()->getProjPointOnPlane(vert, pos, -normal);
				Point3m cur_vec = proj_vert - vert;
				float length = cur_vec * normal;
				if (abs(length) > deepest_value)
				{
					deepest_value = abs(length);
					if (i == 1)
					{
						eliminate_index = 2;
					}
					else
					{
						eliminate_index = 1;
					}
				}
			}
			left_node_infos.erase(left_node_infos.begin() + eliminate_index);
		 }
	}
	this->left_ope_obj_ = new TowableCtrlSystem(0, left_node_infos, true);
	this->left_ope_obj_->setSpaceConstraints(make_pair(true, make_pair(_dental_features->base_plane_.center, _dental_features->base_plane_.axisXV)));
	this->left_ope_obj_->setScreenWidthAndHeight(screen_width_, screen_height_);
	this->left_ope_obj_->setVisualAngleDirect(dental_feature_->base_plane_.axisXV);
	this->left_vec_ = dental_feature_->base_plane_.axisXV;

	if (ctrlnodes_ != nullptr && !ctrlnodes_->empty() && ctrlnodes_->size() == 6)
	{
		for (int ii = 0; ii < 3; ++ii)
		{
			vector<CMeshO*> mesh_list;
			mesh_list.push_back(&dental_feature_->mesh_->original_mesh_->cm);
			vert = ctrlnodes_->at(ii + 3);
			right_node_infos.push_back(make_pair(vert, mesh_list));
		}
	}
	else
	{
		vector<vector<int>> right_indexes_list;
		right_indexes_list.push_back(vector<int>{41, 42, 43, 44, 45, 46, 47});
		right_indexes_list.push_back(vector<int>{44, 45});
		right_indexes_list.push_back(vector<int>{45, 44});
		right_indexes_list.push_back(vector<int>{47, 46, 45, 44, 43, 42, 41});
		for (int ii = 0; ii < 4; ++ii)
		{
			vector<int> cur_search_list = right_indexes_list[ii];
			for (auto& fdi : cur_search_list)
			{
				cur_fdi = fdi;
				tooth_sign = cur_fdi % 10;

				bool found = false;
				for (auto& tooth : dental_feature_->teeth_list_)
				{
					if (tooth.fdi_ == cur_fdi)
					{
						if (tooth_sign == 1 || tooth_sign == 2 || tooth_sign == 3 || tooth_sign == 4 || tooth_sign == 5)
						{
							vert = tooth.marks_[0];
						}
						else if (tooth_sign == 6 || tooth_sign == 7)
						{
							vert = tooth.marks_[1];
						}

						vector<CMeshO*> mesh_list;
						mesh_list.push_back(&dental_feature_->mesh_->original_mesh_->cm);
						right_node_infos.push_back(make_pair(vert, mesh_list));
						found = true;
						break;
					}
				}
				if (found)
				{
					break;
				}
			}
		}

		if (right_node_infos.size() == 4)
		{
			Point3m p0 = right_node_infos.front().first;
			Point3m p3 = right_node_infos.back().first;
			Point3m pos = (p0 + p3) * 0.5f;
			Point3m vec_x = (p0 - p3).Normalize();
			Point3m vec_y = dental_feature_->base_plane_.axisXV;
			Point3m normal = (vec_x ^ vec_y).Normalize();

			float deepest_value = 0;
			int eliminate_index = 1;
			for (int i = 1; i <= 2; ++i)
			{
				FEdge deepest_edge;
				Point3m vert = right_node_infos.at(i).first;
				Point3m proj_vert = UtilityTools::getInstance()->getProjPointOnPlane(vert, pos, -normal);
				Point3m cur_vec = proj_vert - vert;
				float length = cur_vec * normal;
				if (abs(length) > deepest_value)
				{
					deepest_value = abs(length);
					if (i == 1)
					{
						eliminate_index = 2;
					}
					else
					{
						eliminate_index = 1;
					}
				}
			}
			right_node_infos.erase(right_node_infos.begin() + eliminate_index);
		}
	}
	this->right_ope_obj_ = new TowableCtrlSystem(0, right_node_infos, true);
	this->right_ope_obj_->setSpaceConstraints(make_pair(true, make_pair(_dental_features->base_plane_.center, -_dental_features->base_plane_.axisXV)));
	this->right_ope_obj_->setScreenWidthAndHeight(screen_width_, screen_height_);
	this->right_ope_obj_->setVisualAngleDirect(-dental_feature_->base_plane_.axisXV);
	this->right_vec_ = dental_feature_->base_plane_.axisXV;
	this->setPrompt(QString("Spee"));
	updateFeatures();
}

void SpeeCurveDepthItem::updateFeatures()
{
	update();
}

bool SpeeCurveDepthItem::availabilityJudgment()
{
	if (!dental_feature_)
	{
		return false;
	}

	if (!dental_feature_->b_upper_)
	{
		bool left = false, right = false;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			int dental_sign = tooth.fdi_ / 10;
			if (dental_sign == 4)
			{
				right = true;
			}
			else if (dental_sign == 3)
			{
				left = true;
			}
		}
		return left & right;
	}
	return false;
}

MolarRelationshipItem::MolarRelationshipItem(QString* _context, Point3m* _left_node, Point3m* _right_node, DentalFeatures* _dental_features, vector<FEdge>* _ctrledges)
    :EditItem(EditItemIndex(MOLAR_RELATIONSHIP), _context)
{
	setFeatureMode(true);
	dental_feature_ = _dental_features;
	left_node_ = _left_node;
	right_node_ = _right_node;
	ctrledges_ = _ctrledges;
	vector<Point3m> vert_list;
	vector<CMeshO*> mesh_list;
	if (_dental_features->b_upper_)
	{
		for (auto &tooth : _dental_features->teeth_list_)
		{
			Point3m peak_node;
			int fdi = tooth.fdi_;
			if (fdi == 26)
			{
				if (ctrledges_ && !ctrledges_->empty())
				{
					peak_node = ctrledges_->at(0).vertA;
				}
				else
				{
					peak_node = tooth.marks_[0];
				}

				vector<pair<Point3m, vector<CMeshO*>>> node_info;
				vector<CMeshO*> mesh_list;
				mesh_list.push_back(&_dental_features->mesh_->original_mesh_->cm);
				node_info.push_back(make_pair(peak_node, mesh_list));
				left_obj_ = new TowableCtrlSystem(0, node_info, false, true);
				left_obj_->setSpaceConstraints(make_pair(true, make_pair(_dental_features->base_plane_.center, -_dental_features->base_plane_.axisXV)));
				left_direct_ = _dental_features->base_plane_.axisZV;
				left_view_direct_ = _dental_features->base_plane_.axisXV;
			}
			else if (fdi == 16)
			{
				if (ctrledges_ && !ctrledges_->empty())
				{
					peak_node = ctrledges_->at(1).vertA;
				}
				else
				{
					peak_node = tooth.marks_[0];
				}

				vector<pair<Point3m, vector<CMeshO*>>> node_info;
				vector<CMeshO*> mesh_list;
				mesh_list.push_back(&_dental_features->mesh_->original_mesh_->cm);
				node_info.push_back(make_pair(peak_node, mesh_list));
				right_obj_ = new TowableCtrlSystem(1, node_info, false, true);
				right_obj_->setSpaceConstraints(make_pair(true, make_pair(_dental_features->base_plane_.center, _dental_features->base_plane_.axisXV)));
				right_direct_ = _dental_features->base_plane_.axisZV;
				left_view_direct_ = -_dental_features->base_plane_.axisXV;
			}
		}
	}
	else
	{
		for (auto &tooth : _dental_features->teeth_list_)
		{
			Point3m peak_node;
			int fdi = tooth.fdi_;
			if (fdi == 36)
			{
				if (ctrledges_ && !ctrledges_->empty())
				{
					peak_node = ctrledges_->at(0).vertA;
				}
				else
				{
					peak_node = tooth.marks_[5];
				}

				vector<pair<Point3m, vector<CMeshO*>>> node_info;
				vector<CMeshO*> mesh_list;
				mesh_list.push_back(&_dental_features->mesh_->original_mesh_->cm);
				node_info.push_back(make_pair(peak_node, mesh_list));
				left_obj_ = new TowableCtrlSystem(0, node_info, false, true);
				left_obj_->setSpaceConstraints(make_pair(true, make_pair(_dental_features->base_plane_.center, _dental_features->base_plane_.axisXV)));
				left_direct_ = _dental_features->base_plane_.axisZV;
				left_view_direct_ = _dental_features->base_plane_.axisXV;
			}
			else if (fdi == 46)
			{
				if (ctrledges_ && !ctrledges_->empty())
				{
					peak_node = ctrledges_->at(1).vertA;
				}
				else
				{
					peak_node = tooth.marks_[5];
				}

				vector<pair<Point3m, vector<CMeshO*>>> node_info;
				vector<CMeshO*> mesh_list;
				mesh_list.push_back(&_dental_features->mesh_->original_mesh_->cm);
				node_info.push_back(make_pair(peak_node, mesh_list));
				right_obj_ = new TowableCtrlSystem(1, node_info, false, true);
				right_obj_->setSpaceConstraints(make_pair(true, make_pair(_dental_features->base_plane_.center, -_dental_features->base_plane_.axisXV)));
				right_direct_ = _dental_features->base_plane_.axisZV;
				right_view_direct_ = -_dental_features->base_plane_.axisXV;
			}
		}
	}

	this->setPrompt(QString("Molar REL"));
	updateFeatures();
}

void MolarRelationshipItem::updateFeatures()
{
	update();
}

bool MolarRelationshipItem::availabilityJudgment()
{
	if (!dental_feature_)
	{
		return false;
	}

	if (dental_feature_->b_upper_)
	{
		bool left = false, right = false;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			int dental_sign = tooth.fdi_ / 10;
			int tooth_sign = tooth.fdi_ % 10;
			if (dental_sign == 1 && tooth_sign == 6)
			{
				right = true;
			}
			else if (dental_sign == 2 && tooth_sign == 6)
			{
				left = true;
			}
		}
		return left & right;
	}
	else
	{
		bool left = false, right = false;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			int dental_sign = tooth.fdi_ / 10;
			int tooth_sign = tooth.fdi_ % 10;
			if (dental_sign == 4 && tooth_sign == 6)
			{
				right = true;
			}
			else if (dental_sign == 3 && tooth_sign == 6)
			{
				left = true;
			}
		}
		return left & right;
	}
}

MidlineRelationshipItem::MidlineRelationshipItem(QString* _context, DentalFeatures* _dental_features, Axis* _plane, vector<FEdge>* _ctrledges)
    : EditItem(EditItemIndex(MIDLINE_RELATIONSHIP), _context)
{
	setFeatureMode(true);
	dental_feature_ = _dental_features;
	first_initial_ = true;
	ctrledges_ = _ctrledges;
	plane_ = _plane;
	Point3m plane_pos, proj_plane_pos, plane_normal, extand_vec, proj_ha;
	vector<Point3m> vert_list;
	if (_dental_features->b_upper_)
	{
		int find_time = 0;
		plane_pos = Point3m(0, 0, 0);
		vector<vector<int>> sort_list;
		sort_list.push_back(vector<int>{11, 12, 13, 14, 15, 16, 17});
		sort_list.push_back(vector<int>{21, 22, 23, 24, 25, 26, 27});
		for (int i = 0; i < 2; ++i)
		{
			for (auto& aim_fdi : sort_list[i])
			{
				bool found = false;
				for (auto& tooth : _dental_features->teeth_list_)
				{
					int fdi = tooth.fdi_;
					if (fdi == aim_fdi)
					{
						Point3m vert;
						if (tooth.axis_ready_)
						{
							vert = tooth.axis_.centerPoint;
							vert += tooth.axis_.axisZVector;
						}
						else
						{
							vert = tooth.marks_[0];
						}

						plane_pos += vert;
						find_time += 1;
						found = true;
						break;
					}
				}
				if (found)
				{
					break;
				}
			}
		}
		vector<vector<int>>().swap(sort_list);
		if (find_time > 0)
		{
			if (find_time == 2)
			{
				plane_pos *= 0.5f;
			}

			proj_ha = _dental_features->base_plane_.center + _dental_features->base_plane_.axisZV * 10.0f;
			proj_plane_pos = UtilityTools::getInstance()->getProjPointOnPlane(plane_pos, proj_ha, -_dental_features->base_plane_.axisZV);
			occlusal_plane_normal = _dental_features->base_plane_.axisZV;

			if (ctrledges_ && !ctrledges_->empty())
			{
				vert_list.push_back(ctrledges_->at(0).vertA);
				vert_list.push_back(ctrledges_->at(0).vertB);
			}
			else
			{
				vert_list.push_back(plane_pos);
				vert_list.push_back(proj_plane_pos);
			}

			obj_ = new TowableCtrlSystem(0, vert_list, plane_pos, _dental_features->base_plane_.axisYV);
			obj_->initialTowableCtrlEdges();
		}
	}
	else
	{
		int find_time = 0;
		plane_pos = Point3m(0, 0, 0);
		vector<vector<int>> sort_list;
		sort_list.push_back(vector<int>{31, 32, 33, 34, 35, 36, 37});
		sort_list.push_back(vector<int>{41, 42, 43, 44, 25, 46, 47});
		for (int i = 0; i < 2; ++i)
		{
			for (auto& aim_fdi : sort_list[i])
			{
				bool found = false;
				for (auto& tooth : _dental_features->teeth_list_)
				{
					int fdi = tooth.fdi_;
					if (fdi == aim_fdi)
					{
						Point3m vert;
						if (tooth.axis_ready_)
						{
							vert = tooth.axis_.centerPoint;
							vert += tooth.axis_.axisZVector;
						}
						else
						{
							vert = tooth.marks_[0];
						}

						plane_pos += vert;
						find_time += 1;
						found = true;
						break;
					}
				}
				if (found)
				{
					break;
				}
			}
		}

		if (find_time > 0)
		{
			if (find_time == 2)
			{
				plane_pos *= 0.5f;
			}
			proj_ha = _dental_features->base_plane_.center + _dental_features->base_plane_.axisZV * 10.0f;
			proj_plane_pos = UtilityTools::getInstance()->getProjPointOnPlane(plane_pos, proj_ha, -_dental_features->base_plane_.axisZV);
			occlusal_plane_normal = _dental_features->base_plane_.axisZV;

			if (ctrledges_ && !ctrledges_->empty())
			{
				vert_list.push_back(ctrledges_->at(0).vertA);
				vert_list.push_back(ctrledges_->at(0).vertB);
			}
			else
			{
				vert_list.push_back(plane_pos);
				vert_list.push_back(proj_plane_pos);
			}

			obj_ = new TowableCtrlSystem(1, vert_list, plane_pos, _dental_features->base_plane_.axisYV);
			obj_->initialTowableCtrlEdges();
		}
	}
	first_initial_direct_ = -_dental_features->base_plane_.axisYV;
	this->setPrompt(QString("Centerline REL"));
	updateFeatures();
}

void MidlineRelationshipItem::updateFeatures()
{
	update();
}

bool MidlineRelationshipItem::availabilityJudgment()
{
	if (!dental_feature_)
	{
		return false;
	}

	if (dental_feature_->b_upper_)
	{
		bool left = false, right = false;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			int dental_sign = tooth.fdi_ / 10;
			if (dental_sign == 1)
			{
				right = true;
			}
			else if (dental_sign == 2)
			{
				left = true;
			}
		}
		return left & right;
	}
	else
	{
		bool left = false, right = false;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			int dental_sign = tooth.fdi_ / 10;
			if (dental_sign == 4)
			{
				right = true;
			}
			else if (dental_sign == 3)
			{
				left = true;
			}
		}
		return left & right;
	}
}

ArchWidthItem::ArchWidthItem(DentalFeatures* _dental_features, float* _front_value, float* _middle_value, float* _rear_value, float* _basalbone_value, QString* _context, vector<FEdge>* _ctrledges)
    :EditItem(EditItemIndex(ARCH_WIDTH), _context)
{
	setFeatureMode(true);
	dental_feature_ = _dental_features;
	ctrledges_ = _ctrledges;
	front_value_ = _front_value;
	middle_value_ = _middle_value;
	rear_value_ = _rear_value;
	basalbone_value_ = _basalbone_value;

	int fdi, tooth_sign, dental_sign;
	Point3m vertA, vertB;
	CMeshO* meshA = nullptr, * meshB = nullptr;
	bool find_nodeA, find_nodeB;

	find_nodeA = false;  find_nodeB = false;
	meshA = nullptr; meshB = nullptr;
	for (auto &tooth : dental_feature_->teeth_list_)
	{
		fdi = tooth.fdi_;
		tooth_sign = fdi % 10;
		if (tooth_sign == 3)
		{
			Point3m vert;
			vert = tooth.marks_[2];
			if (!find_nodeA)
			{
				vertA = vert;
				meshA = &dental_feature_->mesh_->original_mesh_->cm;
				find_nodeA = true;
			}
			else if (!find_nodeB)
			{
				vertB = vert;
				meshB = &dental_feature_->mesh_->original_mesh_->cm;
				find_nodeB = true;
			}
		}
	}
	if (ctrledges_ && !ctrledges_->empty())
	{
		vertA = ctrledges_->at(0).vertA;
		vertB = ctrledges_->at(0).vertB;
	}
	if (find_nodeA && find_nodeB && meshA && meshB)
	{
		this->front_obj_ = new TowableCtrlSystem(0, vertA, meshA, vertB, meshB);
	}
	else
	{
		this->front_obj_ = nullptr;
	}

	find_nodeA = false;  find_nodeB = false;
	meshA = nullptr; meshB = nullptr;
	for (auto &tooth : dental_feature_->teeth_list_)
	{
		fdi = tooth.fdi_;
		tooth_sign = fdi % 10;
		if (tooth_sign == 4)
		{
			Point3m vert;
			vert = (tooth.marks_[2] + tooth.marks_[3]) * 0.5f;

			if (!find_nodeA)
			{
				vertA = vert;
				meshA = &dental_feature_->mesh_->original_mesh_->cm;
				find_nodeA = true;
			}
			else if (!find_nodeB)
			{
				vertB = vert;
				meshB = &dental_feature_->mesh_->original_mesh_->cm;
				find_nodeB = true;
			}
		}
	}
	if (ctrledges_ && !ctrledges_->empty())
	{
		vertA = ctrledges_->at(1).vertA;
		vertB = ctrledges_->at(1).vertB;
	}
	if (find_nodeA && find_nodeB && meshA && meshB)
	{
		this->middle_obj_ = new TowableCtrlSystem(1, vertA, meshA, vertB, meshB);
	}
	else
	{
		this->middle_obj_ = nullptr;
	}

	find_nodeA = false;  find_nodeB = false;
	meshA = nullptr; meshB = nullptr;
	for (auto& tooth : dental_feature_->teeth_list_)
	{
		fdi = tooth.fdi_;
		tooth_sign = fdi % 10;
		dental_sign = fdi / 10;
		if (tooth_sign == 6)
		{
			Point3m vert;
			if (dental_sign == 1 || dental_sign == 2)
			{
				vert = tooth.marks_[6];
			}
			else
			{
				vert = tooth.marks_[8];
			}

			if (!find_nodeA)
			{
				vertA = vert;
				meshA = &dental_feature_->mesh_->original_mesh_->cm;
				find_nodeA = true;
			}
			else if (!find_nodeB)
			{
				vertB = vert;
				meshB = &dental_feature_->mesh_->original_mesh_->cm;
				find_nodeB = true;
			}
		}
	}
	if (ctrledges_ && !ctrledges_->empty())
	{
		vertA = ctrledges_->at(2).vertA;
		vertB = ctrledges_->at(2).vertB;
	}
	if (find_nodeA && find_nodeB && meshA && meshB)
	{
		this->rear_obj_ = new TowableCtrlSystem(1, vertA, meshA, vertB, meshB);
	}
	else
	{
		this->rear_obj_ = nullptr;
	}

	this->setPrompt(QString("Arch Width"));
	update();
}

void ArchWidthItem::updateFeatures()
{
	update();
}

bool ArchWidthItem::availabilityJudgment()
{
	if (!dental_feature_)
	{
		return false;
	}

	if (dental_feature_->b_upper_)
	{
		int right_num = 0, left_num = 0;
		bool left = false, right = false;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			int dental_sign = tooth.fdi_ / 10;
			int tooth_sign = tooth.fdi_ % 10;
			if (dental_sign == 1 && (tooth_sign == 3 || tooth_sign == 4 || tooth_sign == 6))
			{
				right_num += 1;
			}
			else if (dental_sign == 2 && (tooth_sign == 3 || tooth_sign == 4 || tooth_sign == 6))
			{
				left_num += 1;
			}
		}
		if (right_num > 0)
		{
			right = true;
		}
		if (left_num > 0)
		{
			left = true;
		}
		return left & right;
	}
	else
	{
		int right_num = 0, left_num = 0;
		bool left = false, right = false;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			int dental_sign = tooth.fdi_ / 10;
			int tooth_sign = tooth.fdi_ % 10;
			if (dental_sign == 4 && (tooth_sign == 3 || tooth_sign == 4 || tooth_sign == 6))
			{
				right_num += 1;
			}
			else if (dental_sign == 3 && (tooth_sign == 3 || tooth_sign == 4 || tooth_sign == 6))
			{
				left_num += 1;
			}
		}
		if (right_num > 0)
		{
			right = true;
		}
		if (left_num > 0)
		{
			left = true;
		}
		return left & right;
	}
	return false;
}

ArchLengthItem::ArchLengthItem(DentalFeatures* _dental_features, float* _front_value, float* _middle_value, float* _rear_value, float* _whole_value, QString* _context, vector<FEdge>* _ctrledges)
    :EditItem(EditItemIndex(ARCH_LENGTH), _context)
{
	setFeatureMode(true);
	dental_feature_ = _dental_features;
	ctrledges_ = _ctrledges;
	front_value_ = _front_value;
	middle_value_ = _middle_value;
	rear_value_ = _rear_value;
	whole_value_ = _whole_value;

	int fdi, tooth_sign, dental_sign;
	Point3m vertA, vertB;
	CMeshO* meshA = nullptr, * meshB = nullptr;
	bool find_nodeA, find_nodeB;
	Point3m plane_pos = dental_feature_->base_plane_.center;
	Point3m plane_normal = dental_feature_->base_plane_.axisZV;

	for (int i = 1; i <= 5; ++i)
	{
		find_nodeA = false;  find_nodeB = false;
		meshA = nullptr; meshB = nullptr;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			fdi = tooth.fdi_;
			tooth_sign = fdi % 10;
			if (tooth_sign == i)
			{
				Point3m vert;
				vert = tooth.marks_[0];
				if (!find_nodeA)
				{
					vertA = vert;
					meshA = &dental_feature_->mesh_->original_mesh_->cm;
					find_nodeA = true;
				}
				else if (!find_nodeB)
				{
					vertB = vert;
					meshB = &dental_feature_->mesh_->original_mesh_->cm;
					find_nodeB = true;
				}
			}
		}
		if (find_nodeA && find_nodeB && meshA && meshB)
		{
			Point3m vert = (vertA + vertB) * 0.5f;
			this->anterior_tangent_vert = UtilityTools::getInstance()->getProjPointOnPlane(vert, plane_pos, plane_normal);
			break;
		}
	}

	{
		find_nodeA = false;  find_nodeB = false;
		meshA = nullptr; meshB = nullptr;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			fdi = tooth.fdi_;
			tooth_sign = fdi % 10;
			if (tooth_sign == 3)
			{
				Point3m vert;
				vert = tooth.marks_[2];
				if (!find_nodeA)
				{
					vertA = UtilityTools::getInstance()->getProjPointOnPlane(vert, plane_pos, plane_normal);
					meshA = &dental_feature_->mesh_->original_mesh_->cm;
					find_nodeA = true;
				}
				else if (!find_nodeB)
				{
					vertB = UtilityTools::getInstance()->getProjPointOnPlane(vert, plane_pos, plane_normal);
					meshB = &dental_feature_->mesh_->original_mesh_->cm;
					find_nodeB = true;
				}
			}
		}
		if (find_nodeA && find_nodeB && meshA && meshB)
		{
			vector<Point3m> vert_list;
			if (ctrledges_ && !ctrledges_->empty())
			{
				vertA = ctrledges_->at(0).vertA;
				vertB = ctrledges_->at(0).vertB;
			}

			if (vertA != Point3m(0, 0, 0) && vertB != Point3m(0, 0, 0))
			{
				vert_list.push_back(vertA);
				vert_list.push_back(vertB);
				this->front_obj_ = new TowableCtrlSystem(0, vert_list, plane_pos, plane_normal);
				this->addToObjectList(front_obj_);
			}
			else
			{
				front_obj_ = nullptr;
				this->addToObjectList(front_obj_);
			}
		}
		else
		{
			front_obj_ = nullptr;
			this->addToObjectList(front_obj_);
		}
	}

	{
		find_nodeA = false;  find_nodeB = false;
		meshA = nullptr; meshB = nullptr;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			fdi = tooth.fdi_;
			tooth_sign = fdi % 10;
			if (tooth_sign == 6)
			{
				Point3m vert;
				if (dental_feature_->b_upper_)
				{
					vert = tooth.marks_[9];
				}
				else
				{
					vert = tooth.marks_[11];
				}

				if (!find_nodeA)
				{
					vertA = UtilityTools::getInstance()->getProjPointOnPlane(vert, plane_pos, plane_normal);
					meshA = &dental_feature_->mesh_->original_mesh_->cm;
					find_nodeA = true;
				}
				else if (!find_nodeB)
				{
					vertB = UtilityTools::getInstance()->getProjPointOnPlane(vert, plane_pos, plane_normal);
					meshB = &dental_feature_->mesh_->original_mesh_->cm;
					find_nodeB = true;
				}
			}
		}
		if (find_nodeA && find_nodeB && meshA && meshB)
		{
			vector<Point3m> vert_list;
			if (ctrledges_ && !ctrledges_->empty())
			{
				vertA = ctrledges_->at(1).vertA;
				vertB = ctrledges_->at(1).vertB;
			}

			if (vertA != Point3m(0, 0, 0) && vertB != Point3m(0, 0, 0))
			{
				vert_list.push_back(vertA);
				vert_list.push_back(vertB);
				this->middle_obj_ = new TowableCtrlSystem(1, vert_list, plane_pos, plane_normal);
				this->middle_obj_->setVisible(false);
				this->addToObjectList(middle_obj_);
			}
			else
			{
				this->middle_obj_ = nullptr;
				this->addToObjectList(middle_obj_);
			}
		}
		else
		{
			this->middle_obj_ = nullptr;
			this->addToObjectList(middle_obj_);
		}
	}

	{
		int seven_tooth_appear_time = 0;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			if (tooth.fdi_ % 10 == 7)
			{
				seven_tooth_appear_time += 1;
			}
		}

		Point3m vert;
		find_nodeA = false;  find_nodeB = false;
		meshA = nullptr; meshB = nullptr;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			fdi = tooth.fdi_;
			tooth_sign = fdi % 10;
			if (seven_tooth_appear_time == 2)
			{
				if (tooth_sign == 7)
				{
					vert = tooth.marks_[10];

					if (!find_nodeA)
					{
						vertA = UtilityTools::getInstance()->getProjPointOnPlane(vert, plane_pos, plane_normal);
						meshA = &dental_feature_->mesh_->original_mesh_->cm;
						find_nodeA = true;
					}
					else if (!find_nodeB)
					{
						vertB = UtilityTools::getInstance()->getProjPointOnPlane(vert, plane_pos, plane_normal);
						meshB = &dental_feature_->mesh_->original_mesh_->cm;
						find_nodeB = true;
					}
				}
			}
			else
			{
				if (tooth_sign == 6)
				{
					if (fdi == 16 || fdi == 26)
					{
						vert = tooth.marks_[10];
					}
					else
					{
						vert = tooth.marks_[12];
					}

					if (!find_nodeA)
					{
						vertA = UtilityTools::getInstance()->getProjPointOnPlane(vert, plane_pos, plane_normal);
						meshA = &dental_feature_->mesh_->original_mesh_->cm;
						find_nodeA = true;
					}
					else if (!find_nodeB)
					{
						vertB = UtilityTools::getInstance()->getProjPointOnPlane(vert, plane_pos, plane_normal);
						meshB = &dental_feature_->mesh_->original_mesh_->cm;
						find_nodeB = true;
					}
				}
			}
		}
		if (find_nodeA && find_nodeB && meshA && meshB)
		{
			vector<Point3m> vert_list;
			if (ctrledges_ && !ctrledges_->empty())
			{
				vertA = ctrledges_->at(2).vertA;
				vertB = ctrledges_->at(2).vertB;
			}

			if (vertA != Point3m(0, 0, 0) && vertB != Point3m(0, 0, 0))
			{
				vert_list.push_back(vertA);
				vert_list.push_back(vertB);
				this->rear_obj_ = new TowableCtrlSystem(2, vert_list, plane_pos, plane_normal);
				this->rear_obj_->setVisible(false);
				this->addToObjectList(rear_obj_);
			}
			else
			{
				this->rear_obj_ = nullptr;
				this->addToObjectList(rear_obj_);
			}
		}
		else
		{
			this->rear_obj_ = nullptr;
			this->addToObjectList(rear_obj_);
		}
	}

	setPrompt(QString("Arch Length"));
	update();
}

void ArchLengthItem::updateFeatures()
{
	update();
}

bool ArchLengthItem::availabilityJudgment()
{
	if (!dental_feature_)
	{
		return false;
	}
	int six_appear_num = 0, seven_appear_num = 0;
	for (auto& tooth : dental_feature_->teeth_list_)
	{
		int tooth_sign = tooth.fdi_ % 10;
		if (tooth_sign == 6)
		{
			six_appear_num += 1;
		}
		else if (tooth_sign == 7)
		{
			seven_appear_num += 1;
		}
	}
	return (six_appear_num == 2) | (seven_appear_num == 2);
}

GnathotectumHeightItem::GnathotectumHeightItem(DentalFeatures* _dental_features, float* _p_value, QString* _context, vector<FEdge>* _ctrledges)
    :EditItem(EditItemIndex(GNATHOTECTUM_HEIGHT), _context)
{
	setFeatureMode(true);
	dental_feature_ = _dental_features;
	ctrledges_ = _ctrledges;
	p_height_value_ = _p_value;

	int fdi, tooth_sign;
	Point3m vertA, vertB;
	CMeshO* meshA = nullptr, * meshB = nullptr;
	bool find_nodeA, find_nodeB;

	if (dental_feature_ != nullptr)
	{
		gum_mesh_ = &dental_feature_->mesh_->original_mesh_->cm;
	}

	find_nodeA = false;  find_nodeB = false;
	meshA = nullptr; meshB = nullptr;
	for (auto &tooth : dental_feature_->teeth_list_)
	{
		fdi = tooth.fdi_;
		tooth_sign = fdi % 10;
		if (tooth_sign == 6)
		{
			Point3m vert;
			vert = tooth.marks_[6];
			if (!find_nodeA)
			{
				vertA = vert;
				meshA = &dental_feature_->mesh_->original_mesh_->cm;
				find_nodeA = true;
			}
			else if (!find_nodeB)
			{
				vertB = vert;
				meshB = &dental_feature_->mesh_->original_mesh_->cm;
				find_nodeB = true;
			}
		}
	}

	auto initialGumOpeObj = [=](Point3m a, Point3m b)
	{
		Point3m mid_point = (a + b) * 0.5f;
		std::pair<bool, Point3m> intersection_test = UtilityTools::getInstance()->intersectMesh(mid_point, dental_feature_->base_plane_.axisZV, gum_mesh_);
		if (intersection_test.first)
		{
			vector<std::pair<Point3m, vector<CMeshO*>>> node_list;
			vector<CMeshO*> mesh_list;
			mesh_list.push_back(gum_mesh_);
			node_list.push_back(make_pair(intersection_test.second, mesh_list));
			this->gum_ope_obj_ = new TowableCtrlSystem(1, node_list);
		}
	};

	if (ctrledges_ && !ctrledges_->empty() && meshA && meshB && gum_mesh_)
	{
		vertA = ctrledges_->at(0).vertA;
		vertB = ctrledges_->at(0).vertB;

		this->tooth_ope_obj_ = new TowableCtrlSystem(0, vertA, meshA, vertB, meshB);

		if (ctrledges_->size() >= 2)
		{
			vector<std::pair<Point3m, vector<CMeshO*>>> node_list;
			vector<CMeshO*> mesh_list;
			mesh_list.push_back(gum_mesh_);
			node_list.push_back(make_pair(ctrledges_->at(1).vertA, mesh_list));
			this->gum_ope_obj_ = new TowableCtrlSystem(1, node_list);
		}
		else
		{
			initialGumOpeObj(vertA, vertB);
		}
	}
	else if (find_nodeA && find_nodeB && meshA && meshB && gum_mesh_)
	{
		this->tooth_ope_obj_ = new TowableCtrlSystem(0, vertA, meshA, vertB, meshB);

		initialGumOpeObj(vertA, vertB);
	}
	setPrompt(QString("Palatal height"));
	updateFeatures();
}

void GnathotectumHeightItem::updateFeatures()
{
	update();
}

bool GnathotectumHeightItem::availabilityJudgment()
{
	if (!dental_feature_)
	{
		return false;
	}

	if (dental_feature_->b_upper_)
	{
		bool left = false, right = false;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			int dental_sign = tooth.fdi_ / 10;
			int tooth_sign = tooth.fdi_ % 10;
			if (dental_sign == 1 && (tooth_sign == 6))
			{
				right = true;
			}
			else if (dental_sign == 2 && (tooth_sign == 6))
			{
				left = true;
			}
		}
		return left & right;
	}
	return false;
}

BasalBoneArchItem::BasalBoneArchItem(DentalFeatures* _dental_features, float* _basalbone_len_value, float* _basalbone_wid_value, QString* _context, vector<FEdge>* _ctrledges)
    :EditItem(EditItemIndex(BASAL_BONE_ARCH), _context)
{
	setFeatureMode(true);
	dental_feature_ = _dental_features;
	ctrledges_ = _ctrledges;
	basalbone_len_value_ = _basalbone_len_value;
	basalbone_wid_value_ = _basalbone_wid_value;

	int fdi, tooth_sign, dental_sign;
	Point3m vertA, vertB;
	CMeshO* meshA = nullptr, * meshB = nullptr;
	bool find_nodeA, find_nodeB;

	find_nodeA = false;  find_nodeB = false;
	meshA = nullptr; meshB = nullptr;
	for (auto& tooth : dental_feature_->teeth_list_)
	{
		fdi = tooth.fdi_;
		tooth_sign = fdi % 10;
		if (tooth_sign == 4)
		{
			Point3m vert;
			vert = tooth.marks_[2];

			if (!find_nodeA)
			{
				vertA = vert;
				meshA = &dental_feature_->mesh_->original_mesh_->cm;
				find_nodeA = true;
			}
			else if (!find_nodeB)
			{
				vertB = vert;
				meshB = &dental_feature_->mesh_->original_mesh_->cm;
				find_nodeB = true;
			}
		}
	}
	if (find_nodeA && find_nodeB && meshA && meshB)
	{
		if (ctrledges_ && !ctrledges_->empty() && meshA && meshB)
		{
			vertA = ctrledges_->at(0).vertA;
			vertB = ctrledges_->at(0).vertB;
		}
		this->basalbone_wid_obj_ = new TowableCtrlSystem(3, vertA, meshA, vertB, meshB);
		if (basalbone_wid_obj_)basalbone_wid_obj_->setVisible(true);
		this->addToObjectList(basalbone_wid_obj_);
	}
	else
	{
		this->basalbone_wid_obj_ = nullptr;
		this->addToObjectList(basalbone_wid_obj_);
	}

	Point3m plane_pos = dental_feature_->base_plane_.center;
	Point3m plane_normal = dental_feature_->base_plane_.axisZV;
	for (int i = 1; i <= 5; ++i)
	{
		find_nodeA = false;  find_nodeB = false;
		meshA = nullptr; meshB = nullptr;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			fdi = tooth.fdi_;
			tooth_sign = fdi % 10;
			if (tooth_sign == i)
			{
				Point3m vert;
				vert = tooth.marks_[0];

				if (!find_nodeA)
				{
					vertA = vert;
					meshA = &dental_feature_->mesh_->original_mesh_->cm;
					find_nodeA = true;
				}
				else if (!find_nodeB)
				{
					vertB = vert;
					meshB = &dental_feature_->mesh_->original_mesh_->cm;
					find_nodeB = true;
				}
			}
		}
		if (find_nodeA && find_nodeB && meshA && meshB)
		{
			Point3m vert = (vertA + vertB) * 0.5f;
			this->anterior_tangent_vert = UtilityTools::getInstance()->getProjPointOnPlane(vert, plane_pos, plane_normal);
			break;
		}
	}

	if (&dental_feature_->mesh_->original_mesh_->cm != nullptr)
	{
		find_nodeA = false;  find_nodeB = false;
		meshA = nullptr; meshB = nullptr;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			fdi = tooth.fdi_;
			tooth_sign = fdi % 10;
			dental_sign = fdi / 10;

			if (tooth_sign == 6)
			{
				Point3m vert;
				if (dental_sign == 1 || dental_sign == 2)
				{
					vert = tooth.marks_[10];
				}
				else
				{
					vert = tooth.marks_[12];
				}

				if (!find_nodeA)
				{
					vertA = vert;
					find_nodeA = true;
				}
				else if (!find_nodeB)
				{
					vertB = vert;
					find_nodeB = true;
				}
			}
		}

		if (find_nodeA && find_nodeB)
		{
			gum_vert_ = anterior_tangent_vert;
			if (ctrledges_ && !ctrledges_->empty())
			{
				if (ctrledges_->size() == 3)
				{
					gum_vert_ = ctrledges_->at(2).vertA;
				}
				else
				{
					gum_vert_ = anterior_tangent_vert;
				}
				vertA = ctrledges_->at(1).vertA;
				vertB = ctrledges_->at(1).vertB;
			}
			Point3m normal = dental_feature_->base_plane_.axisZV;
			proj_gum_vert_ = UtilityTools::getInstance()->getProjPointOnPlane(gum_vert_, anterior_tangent_vert, -normal);
			vector<CMeshO*> mesh_list;
			mesh_list.push_back(&dental_feature_->mesh_->original_mesh_->cm);
			vector<pair<Point3m, vector<CMeshO*>>> mesh_pair_list;
			mesh_pair_list.push_back(make_pair(gum_vert_, mesh_list));
			this->gum_vert_obj_ = new TowableCtrlSystem(4, mesh_pair_list);

			Point3m plane_pos = (vertA + vertB) / 2.0f;
			Point3m plane_normal = dental_feature_->base_plane_.axisZV;
			vector<Point3m> vert_list;
			vert_list.push_back(vertA);
			vert_list.push_back(vertB);
			this->basalbone_len_obj_ = new TowableCtrlSystem(3, vert_list, plane_pos, plane_normal);
			this->basalbone_len_obj_->setVisible(false);
			this->addToObjectList(basalbone_len_obj_);
		}
		else
		{
			this->gum_vert_obj_ = nullptr;
			this->basalbone_len_obj_ = nullptr;
			this->addToObjectList(basalbone_len_obj_);
		}
	}

	if (dental_feature_->b_upper_)
	{
		if (basalbone_wid_obj_ != nullptr)
		{
			first_enable_item_index_ = Interactive2DLabel::BASALBONE_WID_SHOW_ONLY_UPPER;
		}
		else if (basalbone_len_obj_ != nullptr)
		{
			first_enable_item_index_ = Interactive2DLabel::BASALBONE_LEN_SHOW_ONLY_UPPER;
		}
		else
		{
			first_enable_item_index_ = Interactive2DLabel::DEFAULT_VIEW;
		}
	}
	else
	{
		if (basalbone_wid_obj_ != nullptr)
		{
			first_enable_item_index_ = Interactive2DLabel::BASALBONE_WID_SHOW_ONLY_LOWER;
		}
		else if (basalbone_len_obj_ != nullptr)
		{
			first_enable_item_index_ = Interactive2DLabel::BASALBONE_LEN_SHOW_ONLY_LOWER;
		}
		else
		{
			first_enable_item_index_ = Interactive2DLabel::DEFAULT_VIEW;
		}
	}

	this->setPrompt(QString("Basal bone"));
	update();
}

void BasalBoneArchItem::updateFeatures()
{
	update();
}

bool BasalBoneArchItem::availabilityJudgment()
{
	if (!dental_feature_)
	{
		return false;
	}

	if (dental_feature_->b_upper_)
	{
		int right_num = 0, left_num = 0;
		bool left = false, right = false;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			int dental_sign = tooth.fdi_ / 10;
			int tooth_sign = tooth.fdi_ % 10;
			if (dental_sign == 1 && (tooth_sign == 4 || tooth_sign == 6))
			{
				right_num += 1;
			}
			else if (dental_sign == 2 && (tooth_sign == 4 || tooth_sign == 6))
			{
				left_num += 1;
			}
		}
		if (right_num > 0)
		{
			right = true;
		}
		if (left_num > 0)
		{
			left = true;
		}
		return left & right;
	}
	else
	{
		int right_num = 0, left_num = 0;
		bool left = false, right = false;
		for (auto& tooth : dental_feature_->teeth_list_)
		{
			int dental_sign = tooth.fdi_ / 10;
			int tooth_sign = tooth.fdi_ % 10;
			if (dental_sign == 4 && (tooth_sign == 4 || tooth_sign == 6))
			{
				right_num += 1;
			}
			else if (dental_sign == 3 && (tooth_sign == 4 || tooth_sign == 6))
			{
				left_num += 1;
			}
		}
		if (right_num > 0)
		{
			right = true;
		}
		if (left_num > 0)
		{
			left = true;
		}
		return left & right;
	}
	return false;
}

void DentalAnalysisData::initialAnalysisData(DentalFeatures* _upper_dental, DentalFeatures* _lower_dental)
{
	if (_upper_dental != nullptr || _lower_dental != nullptr)
	{
		upper_dental_features_ = _upper_dental;
		lower_dental_features_ = _lower_dental;
		vector<EditItemIndex>().swap(avaliable_item_index_list_);

		for (auto& item : upper_item_list_)
		{
			SAFE_DELETE(item);
		}
		upper_item_list_.clear();
		for (auto& item : lower_item_list_)
		{
			SAFE_DELETE(item);
		}
		lower_item_list_.clear();

        switchEditItemFeatureMode(EditItemIndex(NONE));
        switchEditItemFeatureMode(EditItemIndex(MISSING_TOOTH));
        switchEditItemFeatureMode(EditItemIndex(TEETH_WIDTH));
        switchEditItemFeatureMode(EditItemIndex(CUR_LENGTH_OF_DENTAL_ARCH));
        switchEditItemFeatureMode(EditItemIndex(BOLTON_COMPLETELY));
        switchEditItemFeatureMode(EditItemIndex(SPEE_CURVE_DEPTH));
        switchEditItemFeatureMode(EditItemIndex(MOLAR_RELATIONSHIP));
        switchEditItemFeatureMode(EditItemIndex(MIDLINE_RELATIONSHIP));
        switchEditItemFeatureMode(EditItemIndex(ARCH_WIDTH));
        switchEditItemFeatureMode(EditItemIndex(ARCH_LENGTH));
        switchEditItemFeatureMode(EditItemIndex(GNATHOTECTUM_HEIGHT));
        switchEditItemFeatureMode(EditItemIndex(OVERLAY_ANALYSIS));
        switchEditItemFeatureMode(EditItemIndex(OCCLUSION_COLORING));
        switchEditItemFeatureMode(EditItemIndex(BASAL_BONE_ARCH));
        switchEditItemFeatureMode(EditItemIndex(NONE));

		updateSpeedDeepth();
		updateSeverityOfCrowding();
		updateBoltonAnterior();
		updateBoltonCompletely();
		updateMolarRelationship();
		udpateMidlineRelationship();
		updateTeethWidthList();
		setOverjetRank(overjet_rank_);
		setOverbiteRank(overbite_rank_);

		emit PSIGNALMANAGER->updateAvaliableItemActionsSignal(avaliable_item_index_list_);
	}
}

void DentalAnalysisData::switchEditItemFeatureMode(EditItemIndex _item, bool _b_upper)
{

	cur_upper_item_ = nullptr;
	cur_lower_item_ = nullptr;

	cur_mode_ = _item;
	switch (_item)
	{
	case MISSING_TOOTH:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->initialUpdateFeatures();
		}
		else if(upper_dental_features_)
		{
			this->cur_upper_item_ = new MissingToothItem(upper_dental_features_, this->missing_tooth_upper_, nullptr);
			upper_item_list_.push_back(this->cur_upper_item_);
		}

		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->initialUpdateFeatures();
		}
		else if (lower_dental_features_)
		{
			this->cur_lower_item_ = new MissingToothItem(lower_dental_features_, this->missing_tooth_lower_, nullptr);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		break;

	case TEETH_WIDTH:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->initialUpdateFeatures();
		}
		else  if (upper_dental_features_)
		{
			this->cur_upper_item_ = new TeethWidthItem(upper_dental_features_, &this->space_requared_upper_, &this->sum_anterior_width_upper_, &this->sum_whole_width_upper_, &this->tooth_width_edges_upper_, crowding_analysis_6to6_mode_);
			this->cur_upper_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			upper_item_list_.push_back(this->cur_upper_item_);
		}

		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->initialUpdateFeatures();
		}
		else if (lower_dental_features_)
		{
			this->cur_lower_item_ = new TeethWidthItem(lower_dental_features_, &this->space_requared_lower_, &this->sum_anterior_width_lower_, &this->sum_whole_width_lower_, &this->tooth_width_edges_lower_, crowding_analysis_6to6_mode_);
			this->cur_lower_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		break;

	case CUR_LENGTH_OF_DENTAL_ARCH:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->initialUpdateFeatures();
		}
		else if (upper_dental_features_)
		{
			this->cur_upper_item_ = new CurrentArchLengthItem(this->upper_dental_features_, &this->cur_length_arch_upper_, &cur_length_arch_ctrlnodes_upper_, &crowding_arch_5_5_upper_, &crowding_arch_7_7_upper_, &this->space_requared_upper_, crowding_analysis_6to6_mode_);
			upper_item_list_.push_back(this->cur_upper_item_);
		}

		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->initialUpdateFeatures();
		}
		else if (lower_dental_features_)
		{
			this->cur_lower_item_ = new CurrentArchLengthItem(this->lower_dental_features_, &this->cur_length_arch_lower_, &cur_length_arch_ctrlnodes_lower_, &crowding_arch_5_5_lower_, &crowding_arch_7_7_lower_, &this->space_requared_lower_, crowding_analysis_6to6_mode_);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		if (this->crowding_analysis_6to6_mode_)
		{
			corwding_compute_mode_motion_ = Interactive2DLabel::CROWDING_SWITCH_TO_5_5;
		}
		else
		{
			corwding_compute_mode_motion_ = Interactive2DLabel::CROWDING_SWITCH_TO_7_7;
		}
		break;

	case BOLTON_COMPLETELY:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->initialUpdateFeatures();
		}
		else if (upper_dental_features_)
		{
            this->cur_upper_item_ = new BoltonItem(upper_dental_features_, &this->sum_anterior_width_upper_, &this->sum_whole_width_upper_, &this->space_requared_upper_, EditItemIndex(BOLTON_COMPLETELY), 6, crowding_analysis_6to6_mode_);
			this->cur_upper_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			upper_item_list_.push_back(this->cur_upper_item_);
		}

		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->initialUpdateFeatures();
		}
		else if (lower_dental_features_)
		{
            this->cur_lower_item_ = new BoltonItem(lower_dental_features_, &this->sum_anterior_width_lower_, &this->sum_whole_width_lower_, &this->space_requared_lower_, EditItemIndex(BOLTON_COMPLETELY), 6, crowding_analysis_6to6_mode_);
			this->cur_lower_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		break;

	case SPEE_CURVE_DEPTH:
		this->cur_upper_item_ = nullptr;

		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->initialUpdateFeatures();
		}
		else if (lower_dental_features_)
		{
			this->cur_lower_item_ = new SpeeCurveDepthItem(lower_dental_features_, &this->left_spee_depth_, &this->right_spee_deepth_, nullptr, &spee_ctrlnodes_);
			this->cur_lower_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		_b_upper = false;
		break;

	case MOLAR_RELATIONSHIP:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			this->cur_upper_item_->initialUpdateFeatures();
		}
		else if (upper_dental_features_)
		{
			this->cur_upper_item_ = new MolarRelationshipItem(nullptr, &left_molar_node_upper_, &right_molar_node_upper_, upper_dental_features_, &molar_ctrledges_upper_);
			this->cur_upper_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			upper_item_list_.push_back(this->cur_upper_item_);
		}

		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			this->cur_lower_item_->initialUpdateFeatures();
		}
		else if (lower_dental_features_)
		{
			this->cur_lower_item_ = new MolarRelationshipItem(nullptr, &left_molar_node_lower_, &right_molar_node_lower_, lower_dental_features_, &molar_ctrledges_lower_);
			this->cur_lower_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		break;

	case	MIDLINE_RELATIONSHIP:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			this->cur_upper_item_->initialUpdateFeatures();
		}
		else if (upper_dental_features_)
		{
			this->cur_upper_item_ = new MidlineRelationshipItem(nullptr, upper_dental_features_, &upper_plane_, &midline_ctrledges_upper_);
			this->cur_upper_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			upper_item_list_.push_back(this->cur_upper_item_);
		}

		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			this->cur_lower_item_->initialUpdateFeatures();
		}
		else if (lower_dental_features_)
		{
			this->cur_lower_item_ = new MidlineRelationshipItem(nullptr, lower_dental_features_, &lower_plane_, &midline_ctrledges_lower_);
			this->cur_lower_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		_b_upper = true;
		break;

	case ARCH_WIDTH:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->initialUpdateFeatures();
		}
		else  if (upper_dental_features_)
		{
			this->cur_upper_item_ = new ArchWidthItem(upper_dental_features_, &arch_width_front_upper_, &arch_width_middle_upper_, &arch_width_rear_upper_, &arch_width_basalbone_upper_,
				nullptr, &arch_width_ctrledges_upper_);
			upper_item_list_.push_back(this->cur_upper_item_);
		}

		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->initialUpdateFeatures();
		}
		else if (lower_dental_features_)
		{
			this->cur_lower_item_ = new ArchWidthItem(lower_dental_features_, &arch_width_front_lower_, &arch_width_middle_lower_, &arch_width_rear_lower_, &arch_width_basalbone_lower_,
				nullptr, &arch_width_ctrledges_lower_);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		break;

	case ARCH_LENGTH:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->initialUpdateFeatures();
		}
		else if (upper_dental_features_)
		{
			this->cur_upper_item_ = new ArchLengthItem(upper_dental_features_, &arch_len_front_upper_, &arch_len_middle_upper_, &arch_len_rear_upper_, &arch_len_whole_upper_,
				nullptr, &arch_len_ctrledges_upper_);
			upper_item_list_.push_back(this->cur_upper_item_);
		}

		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->initialUpdateFeatures();
		}
		else if (lower_dental_features_)
		{
			this->cur_lower_item_ = new ArchLengthItem(lower_dental_features_, &arch_len_front_lower_, &arch_len_middle_lower_, &arch_len_rear_lower_, &arch_len_whole_lower_,
				nullptr, &arch_len_ctrledges_lower_);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		break;

	case GNATHOTECTUM_HEIGHT:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->initialUpdateFeatures();
		}
		else if (upper_dental_features_)
		{
			this->cur_upper_item_ = new  GnathotectumHeightItem(upper_dental_features_, &gnathotectum_height_value_upper_, &gnathotectum_height_upper_, &gna_height_ctrledges_upper_);
			upper_item_list_.push_back(this->cur_upper_item_);
		}

		this->cur_lower_item_ = nullptr;
		break;

	case BASAL_BONE_ARCH:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->initialUpdateFeatures();
		}
		else if (upper_dental_features_)
		{
			this->cur_upper_item_ = new BasalBoneArchItem(upper_dental_features_, &arch_len_basalbone_upper_, &arch_width_basalbone_upper_,
				nullptr, &basal_bone_arch_ctrledges_upper_);
			upper_item_list_.push_back(this->cur_upper_item_);
		}
		if (cur_upper_item_)
		{
			upper_basal_bone_motion_ = cur_upper_item_->first_enable_item_index_;
		}

		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->initialUpdateFeatures();
		}
		else if (lower_dental_features_)
		{
			this->cur_lower_item_ = new BasalBoneArchItem(lower_dental_features_, &arch_len_basalbone_lower_, &arch_width_basalbone_lower_,
				nullptr, &basal_bone_arch_ctrledges_lower_);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		if (cur_lower_item_)
		{
			lower_basal_bone_motion_ = cur_lower_item_->first_enable_item_index_;
		}
		break;

	default:
		break;
	}

	if (cur_upper_item_)
	{
		cur_upper_item_->updateCtrlParts();
	}
	if (cur_lower_item_)
	{
		cur_lower_item_->updateCtrlParts();
	}

	if (cur_upper_item_ != nullptr)
	{
		this->cur_ope_item_ = cur_upper_item_;
		cur_ope_item_->setSpecialSign(true);
	}
	else if(cur_lower_item_ != nullptr)
	{
		this->cur_ope_item_ = cur_lower_item_;
		cur_ope_item_->setSpecialSign(false);
	}
	else
	{
		cur_ope_item_ = nullptr;
	}

	bool is_avaliable_item = false;
	switch (_item)
	{
	case MISSING_TOOTH:
		if (upper_dental_features_ != nullptr || lower_dental_features_ != nullptr)
		{
			is_avaliable_item = true;
		}
		break;

	case TEETH_WIDTH:
		if (upper_dental_features_ != nullptr || lower_dental_features_ != nullptr)
		{
			is_avaliable_item = true;
		}
		break;

	case CUR_LENGTH_OF_DENTAL_ARCH:
		if (upper_dental_features_ != nullptr || lower_dental_features_ != nullptr)
		{
			is_avaliable_item = true;
			if (cur_upper_item_ != nullptr)
			{
				is_avaliable_item &= cur_upper_item_->availabilityJudgment();
			}
			if (cur_lower_item_ != nullptr)
			{
				is_avaliable_item &= cur_lower_item_->availabilityJudgment();
			}
		}
		if (!is_avaliable_item)
		{
			confirm_crowding_ = false;
		}
		break;

	case BOLTON_COMPLETELY:
		if (upper_dental_features_ != nullptr && lower_dental_features_ != nullptr)
		{
			is_avaliable_item = true;
			if (cur_upper_item_ != nullptr)
			{
				is_avaliable_item &= cur_upper_item_->availabilityJudgment();
			}
			if (cur_lower_item_ != nullptr)
			{
				is_avaliable_item &= cur_lower_item_->availabilityJudgment();
			}
		}
		if (!is_avaliable_item)
		{
			confirm_bolton_ = false;
		}
		break;

	case SPEE_CURVE_DEPTH:
		if (lower_dental_features_ != nullptr)
		{
			is_avaliable_item = true;
			if (cur_lower_item_ != nullptr)
			{
				is_avaliable_item &= cur_lower_item_->availabilityJudgment();
			}
		}
		if (!is_avaliable_item)
		{
			confirm_spee_ = false;
		}
		break;

	case MOLAR_RELATIONSHIP:
		if (upper_dental_features_ != nullptr && lower_dental_features_ != nullptr)
		{
			is_avaliable_item = true;
			if (cur_upper_item_ != nullptr)
			{
				is_avaliable_item &= cur_upper_item_->availabilityJudgment();
			}
			if (cur_lower_item_ != nullptr)
			{
				is_avaliable_item &= cur_lower_item_->availabilityJudgment();
			}
		}
		if (!is_avaliable_item)
		{
			confirm_molar_ = false;
		}
		break;

	case	MIDLINE_RELATIONSHIP:
		if (upper_dental_features_ != nullptr && lower_dental_features_ != nullptr)
		{
			is_avaliable_item = true;
			if (cur_upper_item_ != nullptr)
			{
				is_avaliable_item &= cur_upper_item_->availabilityJudgment();
			}
			if (cur_lower_item_ != nullptr)
			{
				is_avaliable_item &= cur_lower_item_->availabilityJudgment();
			}
		}
		if (!is_avaliable_item)
		{
			confirm_anterior_parameter_ = false;
		}
		break;

	case ARCH_WIDTH:
		if (upper_dental_features_ != nullptr || lower_dental_features_ != nullptr)
		{
			is_avaliable_item = true;
			if (cur_upper_item_ != nullptr)
			{
				is_avaliable_item &= cur_upper_item_->availabilityJudgment();
			}
			if (cur_lower_item_ != nullptr)
			{
				is_avaliable_item &= cur_lower_item_->availabilityJudgment();
			}
		}
		if (!is_avaliable_item)
		{
			confirm_arch_width_ = false;
		}
		break;

	case ARCH_LENGTH:
		if (upper_dental_features_ != nullptr || lower_dental_features_ != nullptr)
		{
			is_avaliable_item = true;
			if (cur_upper_item_ != nullptr)
			{
				is_avaliable_item &= cur_upper_item_->availabilityJudgment();
			}
			if (cur_lower_item_ != nullptr)
			{
				is_avaliable_item &= cur_lower_item_->availabilityJudgment();
			}
		}
		if (!is_avaliable_item)
		{
			confirm_arch_length_ = false;
		}
		break;

	case GNATHOTECTUM_HEIGHT:
		if (upper_dental_features_ != nullptr)
		{
			is_avaliable_item = true;
			if (cur_upper_item_ != nullptr)
			{
				is_avaliable_item &= cur_upper_item_->availabilityJudgment();
			}
		}
		if (!is_avaliable_item)
		{
			confirm_gnathotectum_height_ = false;
		}
		break;

	case OVERLAY_ANALYSIS:
		if (upper_dental_features_ != nullptr && lower_dental_features_ != nullptr)
		{
			int found_time = 0;
			for (auto& tooth : upper_dental_features_->teeth_list_)
			{
				if (tooth.fdi_ == 11 || tooth.fdi_ == 21)
				{
					++found_time;
				}
			}
			if (found_time == 2)
			{
				is_avaliable_item = true;
			}
		}
		if (!is_avaliable_item)
		{
			confirm_overlay_ = false;
		}
		break;

	case OCCLUSION_COLORING:
		is_avaliable_item = true;
		break;

	case BASAL_BONE_ARCH:
		if (upper_dental_features_ != nullptr || lower_dental_features_ != nullptr)
		{
			is_avaliable_item = true;
			if (cur_upper_item_ != nullptr)
			{
				is_avaliable_item &= cur_upper_item_->availabilityJudgment();
			}
			if (cur_lower_item_ != nullptr)
			{
				is_avaliable_item &= cur_lower_item_->availabilityJudgment();
			}
		}
		if (!is_avaliable_item)
		{
			confirm_basal_bone_arch_ = false;
		}
		break;

	default:
		break;
	}
	if (is_avaliable_item)
	{
		if (find(this->avaliable_item_index_list_.begin(), this->avaliable_item_index_list_.end(), _item) == this->avaliable_item_index_list_.end())
		{
			avaliable_item_index_list_.push_back(_item);
		}
	}

	update();
}
