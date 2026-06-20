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

#include "cloudkdtree.h"
#include "common_ext/util/utility_tools.h"
#include "common_base/SignalManager.h"
#include "qfile.h"
#include <qdir.h>
#include <qdatetime.h>
#include <thread>
#include <future>

static vcg::Matrix44f adj_matrix_ = vcg::Matrix44f::Identity();
static CameraState camera_state;
static float fixed_scale = 0.014f;
static int ANIMATION_TIME_LEN = 50;

float CloudOctree::min_scree_radius = MIN_SCREEN_RADIUS;
bool CloudOctree::true_color_mode_ = false;

Point3m dental_upper_screen_fixed_t(0.1f, 0.7f, 0.5f);
Point3m dental_lower_screen_fixed_t(0.1f, 0.3f, 0.5f);

bool cmp(pair<double, int> _a, pair<double, int> _b)
{
	return _a.first > _b.first;
}

bool cmp_min(pair<float, int> _a, pair<float, int> _b)
{
	return _a.first < _b.first;
}

PiecewiseCloud::PiecewiseCloud(const vector<CloudNode>& _nodes, CloudOctree* _tree, int _index, float* _m)
{
	this->nodes_ = _nodes;
	this->sub_tree_ = _tree;
	this->index_ = _index;

	this->matrix_ = new float[16];
	if (_m != nullptr)
	{
		for (int i = 0; i < 16; ++i)
		{
			matrix_[i] = _m[i];
		}
	}
	else
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				if (i == j)
				{
					matrix_[i * 4 + j] = 1;
				}
				else
				{
					matrix_[i * 4 + j] = 0;
				}
			}
		}
	}

	_tree->setTransformMatrix(this->matrix_);
}

PiecewiseCloud::PiecewiseCloud(const PiecewiseCloud& rhs)
{
	nodes_ = rhs.nodes_;
	this->index_ = rhs.index_;
	this->matrix_ = new float[16];
	for (int i = 0; i < 16; i++)
	{
		this->matrix_[i] = rhs.matrix_[i];
	}
	this->sub_tree_ = rhs.sub_tree_;
	this->picked_ = rhs.picked_;
	if (sub_tree_ != nullptr)
	{
		sub_tree_->setTransformMatrix(this->matrix_);
	}
}

PiecewiseCloud::~PiecewiseCloud()
{
	delete[] matrix_;
	matrix_ = nullptr;
}

bool PiecewiseCloud::modifyMatrix(const std::vector<float>& _m)
{
	if (this->matrix_ == nullptr || _m.size() != 16)
	{
		return false;
	}

	bool changed = false;
	for (int i = 0; i < 16; ++i)
	{
		if (matrix_[i] != _m[i])
		{
			changed = true;
			break;
		}
	}

	if (changed)
	{
		for (int i = 0; i < 16; ++i)
		{
			matrix_[i] = _m[i];
		}
	}
	return changed;
}

void PiecewiseCloud::deletePickedVerts()
{
	vector<CloudNode>::iterator ite = nodes_.begin();
	while (ite != nodes_.end())
	{
		if ((*ite).tempSign() > 0)
		{
			nodes_.erase(ite);
		}
		else
		{
			++ite;
		}
	}
	for (int i = 0; i < nodes_.size(); ++i)
	{
		nodes_[i].setIndex(i);
	}

	vcg::Box3f input_box;
	for (auto& node : this->nodes_)
	{
		input_box.Add(node.pos());
	}

	ObjectName obj_name;
	if (this->sub_tree_ != nullptr)
	{
		obj_name = sub_tree_->objectName();
		delete this->sub_tree_;
		this->sub_tree_ = nullptr;
	}
	sub_tree_ = new CloudOctree(nodes_, input_box, nullptr);
	sub_tree_->setObjectName(obj_name);
	sub_tree_->initalLayerBall(nodes_);
	sub_tree_->setNodeCloud(nodes_);
	sub_tree_->setTransformMatrix(this->matrix_);
}

void PiecewiseCloud::fusionProcessing(vector<CloudNode>& _cur_nodes, float* _cur_matrix)
{
	vcg::Matrix44f matrix_a(_cur_matrix);
	vector<CloudNode> summary;
	Point3m pos, normal, color, nor_node;
	Point4m pos4D, normal4D;

	int index = 0;
	for (int i = 0; i < _cur_nodes.size(); ++i)
	{
		pos = _cur_nodes[i].pos();
		normal = _cur_nodes[i].normal();
		color = _cur_nodes[i].color();

		pos4D = Point4m(pos.X(), pos.Y(), pos.Z(), 1);
		pos4D = matrix_a * pos4D;
		pos = Point3m(pos4D.X(), pos4D.Y(), pos4D.Z());
		summary.push_back(CloudNode(index, pos, normal, color));
		++index;
	}
	BilateralFilterCSR csr(&summary);

	vector<CloudNode>::iterator ite = _cur_nodes.begin();
	while (ite != _cur_nodes.end())
	{
		if (isOutlierWith(&*ite, _cur_matrix, &csr))
		{
			_cur_nodes.erase(ite);
		}
		else
		{
			++ite;
		}
	}

	for (int i = 0; i < _cur_nodes.size(); ++i)
	{
		fusionAction(&_cur_nodes[i], _cur_matrix, &csr, _cur_nodes.size());
	}
	vector<CloudNode>().swap(summary);
}

void PiecewiseCloud::fusionWith(vector<CloudNode>& _cur_nodes, float* _cur_matrix)
{
	vcg::Matrix44f matrix_a(_cur_matrix), matrix_b(this->matrix_);
	vector<CloudNode> summary;
	Point3m pos, normal, color, nor_node;
	Point4m pos4D, normal4D;

	int index = 0;
	for (int i = 0; i < _cur_nodes.size(); ++i)
	{
		pos = _cur_nodes[i].pos();
		normal = _cur_nodes[i].normal();
		color = _cur_nodes[i].color();

		pos4D = Point4m(pos.X(), pos.Y(), pos.Z(), 1);
		pos4D = matrix_a * pos4D;
		pos = Point3m(pos4D.X(), pos4D.Y(), pos4D.Z());

		summary.push_back(CloudNode(index, pos, normal, color));
		++index;
	}
	for (int i = 0; i < this->nodes_.size(); ++i)
	{
		pos = this->nodes_[i].pos();
		normal = this->nodes_[i].normal();
		color = this->nodes_[i].color();

		pos4D = Point4m(pos.X(), pos.Y(), pos.Z(), 1);
		pos4D = matrix_b * pos4D;
		pos = Point3m(pos4D.X(), pos4D.Y(), pos4D.Z());

		summary.push_back(CloudNode(index, pos, normal, color));
		++index;
	}
	BilateralFilterCSR csr(&summary);

	for (int i = 0; i < _cur_nodes.size(); ++i)
	{
		fusionAction(&_cur_nodes[i], _cur_matrix, &csr, _cur_nodes.size());
	}
	vector<CloudNode>().swap(summary);
}

bool PiecewiseCloud::isOutlierWith(CloudNode* _node, float* _matrix, BilateralFilterCSR* _csr)
{
	vcg::Matrix44f matrix_a(_matrix);
	vcg::Matrix44f matrix_b;
	matrix_b = matrix_a;
	matrix_b = matrix_b.transpose();
	Point3m pos, normal, color, nor_node;
	Point4m pos4D, normal4D;

	pos = _node->pos();
	normal = _node->normal();
	pos4D = Point4m(pos.X(), pos.Y(), pos.Z(), 1);
	normal4D = Point4m(normal.X(), normal.Y(), normal.Z(), 1);

	pos4D = matrix_a * pos4D;
	normal4D = matrix_b * normal4D;

	pos = Point3m(pos4D.X(), pos4D.Y(), pos4D.Z());
	normal = Point3m(normal4D.X(), normal4D.Y(), normal4D.Z());
	normal = normal.Normalize();

	return _csr->isOutlier(pos, normal);
}

void PiecewiseCloud::fusionAction(CloudNode* _node, float* _matrix, BilateralFilterCSR* _csr, int _s)
{
	vcg::Matrix44f matrix_a(_matrix);
	vcg::Matrix44f matrix_b;
	matrix_b = matrix_a;
	matrix_b = matrix_b.transpose();
	Point3m pos, normal, color, nor_node;
	Point4m pos4D, normal4D;

	pos = _node->pos();
	normal = _node->normal();
	pos4D = Point4m(pos.X(), pos.Y(), pos.Z(), 1);
	normal4D = Point4m(normal.X(), normal.Y(), normal.Z(), 1);

	pos4D = matrix_a * pos4D;
	normal4D = matrix_b * normal4D;

	pos = Point3m(pos4D.X(), pos4D.Y(), pos4D.Z());
	normal = Point3m(normal4D.X(), normal4D.Y(), normal4D.Z());
	normal = normal.Normalize();

	float adj_value = _csr->getFunctionValue2(pos, normal, color);
	_node->setPos(_node->pos() - _node->normal() * adj_value);
	_node->setColor(color);
}

CloudOctree::CloudOctree()
{
	for (int i = 0; i < 8; ++i)
	{
		leafs[i] = nullptr;
	}
}

CloudOctree::CloudOctree(CloudOctree *_father):CloudOctree()
{
	father_ = _father;
}

CloudOctree::CloudOctree(vector<CloudNode> &_nodes, vcg::Box3f &_box, CloudOctree *_father) : CloudOctree(_father)
{
	if (_nodes.size() <= MIN_CONTAIN_NUM)
	{
		for (auto &node : _nodes)
		{
			contain_.push_back(node.index());
		}
	}
	else
	{
		float x0, x1, x2;
		x0 = _box.min.X();
		x2 = _box.max.X();
		x1 = (x0 + x2) / 2;
		float y0, y1, y2;
		y0 = _box.min.Y();
		y2 = _box.max.Y();
		y1 = (y0 + y2) / 2;
		float z0, z1, z2;
		z0 = _box.min.Z();
		z2 = _box.max.Z();
		z1 = (z0 + z2) / 2;

		vector<vcg::Box3f> sub_boxes;
		sub_boxes.push_back(vcg::Box3f(Point3f(x0, y1, z0), Point3f(x1,y2,z1)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x1, y1, z0), Point3f(x2, y2, z1)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x1, y1, z1), Point3f(x2, y2, z2)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x0, y1, z1), Point3f(x1, y2, z2)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x0, y0, z0), Point3f(x1, y1, z1)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x1, y0, z0), Point3f(x2, y1, z1)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x1, y0, z1), Point3f(x2, y1, z2)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x0, y0, z1), Point3f(x1, y1, z2)));

		vector<vector<CloudNode>> sub_nodes(8);
		for (auto &node : _nodes)
		{
			for (int i = 0; i < sub_boxes.size(); ++i)
			{
				if (sub_boxes[i].IsIn(node.pos()))
				{
					sub_nodes[i].push_back(node);
					break;
				}
			}
		}

		vector<OctreeLeaf> curNeedSplitSonNodes;
		for (int i = 0; i < 8; ++i)
		{
			if (!sub_nodes[i].empty())
			{
				curNeedSplitSonNodes.push_back(OctreeLeaf(this, sub_nodes[i], sub_boxes[i], i));
			}
		}

		vector<OctreeLeaf> newSonNodes;
		while (!curNeedSplitSonNodes.empty())
		{
			for (int i = 0; i < curNeedSplitSonNodes.size(); ++i)
			{
				vector<OctreeLeaf> tempSonNodes = generateLeaf(curNeedSplitSonNodes[i]);
				if (!tempSonNodes.empty())
				{
					newSonNodes.insert(newSonNodes.end(), tempSonNodes.begin(), tempSonNodes.end());
					vector<OctreeLeaf>().swap(tempSonNodes);
				}
			}
			curNeedSplitSonNodes = newSonNodes;
			vector<OctreeLeaf>().swap(newSonNodes);
		}
	}
}

CloudOctree::CloudOctree(vector<CloudOctree*> _nodetrees, vector<PiecewiseCloud> _input_record, vcg::Box3f &_box, CloudOctree *_father) : CloudOctree(_father)
{
	vector<CloudNode>().swap(node_cloud_);
	for (int i = 0; i < _nodetrees.size(); ++i)
	{
		for (int j = 0; j < _nodetrees[i]->node_cloud_.size(); ++j)
		{
			_nodetrees[i]->node_cloud_[j].nodeBall()->setDepthZ(-1);
		}
		node_cloud_.insert(node_cloud_.end(), _nodetrees[i]->node_cloud_.begin(), _nodetrees[i]->node_cloud_.end());
	}

	float x0, x1, x2;
	x0 = _box.min.X();
	x2 = _box.max.X();
	x1 = (x0 + x2) / 2;
	float y0, y1, y2;
	y0 = _box.min.Y();
	y2 = _box.max.Y();
	y1 = (y0 + y2) / 2;
	float z0, z1, z2;
	z0 = _box.min.Z();
	z2 = _box.max.Z();
	z1 = (z0 + z2) / 2;

	vector<vcg::Box3f> sub_boxes;
	sub_boxes.push_back(vcg::Box3f(Point3f(x0, y1, z0), Point3f(x1, y2, z1)));
	sub_boxes.push_back(vcg::Box3f(Point3f(x1, y1, z0), Point3f(x2, y2, z1)));
	sub_boxes.push_back(vcg::Box3f(Point3f(x1, y1, z1), Point3f(x2, y2, z2)));
	sub_boxes.push_back(vcg::Box3f(Point3f(x0, y1, z1), Point3f(x1, y2, z2)));
	sub_boxes.push_back(vcg::Box3f(Point3f(x0, y0, z0), Point3f(x1, y1, z1)));
	sub_boxes.push_back(vcg::Box3f(Point3f(x1, y0, z0), Point3f(x2, y1, z1)));
	sub_boxes.push_back(vcg::Box3f(Point3f(x1, y0, z1), Point3f(x2, y1, z2)));
	sub_boxes.push_back(vcg::Box3f(Point3f(x0, y0, z1), Point3f(x1, y1, z2)));

	vector<vector<CloudOctree*>> sub_nodetrees(8);
	for (auto tree : _nodetrees)
	{
		for (int i = 0; i < sub_boxes.size(); ++i)
		{
			if (sub_boxes[i].IsIn(tree->ball_->p_))
			{
				sub_nodetrees[i].push_back(tree);
				break;
			}
		}
	}

	vector<OctreeSubTreeLeaf> curNeedSplitSonNodes;
	for (int i = 0; i < 8; ++i)
	{
		if (!sub_nodetrees[i].empty())
		{
			curNeedSplitSonNodes.push_back(OctreeSubTreeLeaf(this, sub_nodetrees[i], sub_boxes[i], i));
		}
	}

	vector<OctreeSubTreeLeaf> newSonNodes;
	while (!curNeedSplitSonNodes.empty())
	{
		for (int i = 0; i < curNeedSplitSonNodes.size(); ++i)
		{
			vector<OctreeSubTreeLeaf> tempSonNodes = generateSubTreeLeaf(curNeedSplitSonNodes[i]);
			if (!tempSonNodes.empty())
			{
				newSonNodes.insert(newSonNodes.end(), tempSonNodes.begin(), tempSonNodes.end());
				vector<OctreeSubTreeLeaf>().swap(tempSonNodes);
			}
		}
		curNeedSplitSonNodes = newSonNodes;
		vector<OctreeSubTreeLeaf>().swap(newSonNodes);
	}

	this->subtree_list_ = _nodetrees;
	this->input_record_ = _input_record;
}

void CloudOctree::setNodeCloud(vector<CloudNode>& _nodes)
{
	this->node_cloud_ = _nodes;
}

vector<OctreeLeaf> CloudOctree::generateLeaf(OctreeLeaf &_node)
{
	vector<OctreeLeaf> split_result;

	if (_node.nodes_.size() <= MIN_CONTAIN_NUM)
	{
		_node.becomeTrueLeafNode();
		return split_result;
	}
	else
	{
		float x0, x1, x2;
		x0 = _node.box_.min.X();
		x2 = _node.box_.max.X();
		x1 = (x0 + x2) / 2;
		float y0, y1, y2;
		y0 = _node.box_.min.Y();
		y2 = _node.box_.max.Y();
		y1 = (y0 + y2) / 2;
		float z0, z1, z2;
		z0 = _node.box_.min.Z();
		z2 = _node.box_.max.Z();
		z1 = (z0 + z2) / 2;

		vector<vcg::Box3f> sub_boxes;
		sub_boxes.push_back(vcg::Box3f(Point3f(x0, y1, z0), Point3f(x1, y2, z1)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x1, y1, z0), Point3f(x2, y2, z1)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x1, y1, z1), Point3f(x2, y2, z2)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x0, y1, z1), Point3f(x1, y2, z2)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x0, y0, z0), Point3f(x1, y1, z1)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x1, y0, z0), Point3f(x2, y1, z1)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x1, y0, z1), Point3f(x2, y1, z2)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x0, y0, z1), Point3f(x1, y1, z2)));

		vector<vector<CloudNode>> sub_nodes(8);
		for (auto &node : _node.nodes_)
		{
			for (int i = 0; i < sub_boxes.size(); ++i)
			{
				if (sub_boxes[i].IsIn(node.pos()))
				{
					sub_nodes[i].push_back(node);
					break;
				}
			}
		}

		int temp_counter = 0;
		int temp_index = -1;
		for (int i = 0; i < 8; ++i)
		{
			if (!sub_nodes[i].empty())
			{
				temp_index = i;
				temp_counter += 1;
			}
		}
		if (temp_counter == 1)
		{
			CheckAndEliminateDuplication(sub_nodes[temp_index]);
		}

		for (int i = 0; i < 8; ++i)
		{
			if (!sub_nodes[i].empty())
			{
				split_result.push_back(OctreeLeaf(_node.self_, sub_nodes[i], sub_boxes[i], i));
			}
		}
		return split_result;
	}
}

vector<OctreeSubTreeLeaf>  CloudOctree::generateSubTreeLeaf(OctreeSubTreeLeaf &_node)
{
	vector<OctreeSubTreeLeaf> split_result;

	if (_node.nodes_.size() == 1)
	{
		_node.becomeTrueSubTreeLeafNode();
		return split_result;
	}
	else
	{
		float x0, x1, x2;
		x0 = _node.box_.min.X();
		x2 = _node.box_.max.X();
		x1 = (x0 + x2) / 2;
		float y0, y1, y2;
		y0 = _node.box_.min.Y();
		y2 = _node.box_.max.Y();
		y1 = (y0 + y2) / 2;
		float z0, z1, z2;
		z0 = _node.box_.min.Z();
		z2 = _node.box_.max.Z();
		z1 = (z0 + z2) / 2;

		vector<vcg::Box3f> sub_boxes;
		sub_boxes.push_back(vcg::Box3f(Point3f(x0, y1, z0), Point3f(x1, y2, z1)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x1, y1, z0), Point3f(x2, y2, z1)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x1, y1, z1), Point3f(x2, y2, z2)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x0, y1, z1), Point3f(x1, y2, z2)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x0, y0, z0), Point3f(x1, y1, z1)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x1, y0, z0), Point3f(x2, y1, z1)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x1, y0, z1), Point3f(x2, y1, z2)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x0, y0, z1), Point3f(x1, y1, z2)));

		vector<vector<CloudOctree*>> sub_nodes(8);
		for (auto node : _node.nodes_)
		{
			for (int i = 0; i < sub_boxes.size(); ++i)
			{
				if (sub_boxes[i].IsIn(node->ball_->p_))
				{
					sub_nodes[i].push_back(node);
					break;
				}
			}
		}

		for (int i = 0; i < 8; ++i)
		{
			if (!sub_nodes[i].empty())
			{
				split_result.push_back(OctreeSubTreeLeaf(_node.self_, sub_nodes[i], sub_boxes[i], i));
			}
		}
		return split_result;
	}
}

void CloudOctree::getSubTreesTogether(vector<CloudOctree*> _subtrees, vector<PiecewiseCloud> _input_records)
{
	vcg::Box3f box;
	for (auto tree : _subtrees)
	{
		box.Add(tree->ball_->p_);
	}
	bool first_time_record = this->first_time_draw;
	new (this)CloudOctree(_subtrees, _input_records, box, nullptr);
	this->initalLayerBall2();
	this->first_time_draw = first_time_record;
}

void CloudOctree::CheckAndEliminateDuplication(vector<CloudNode> &_nodes)
{
	vector<CloudNode> vList;
	for (auto &v : _nodes)
	{
		if (vList.empty())
		{
			vList.push_back(v);
		}
		else
		{
			bool duplicate = false;
			for (auto &s : vList)
			{
				if (v.pos() == s.pos())
				{
					duplicate = true;
					break;
				}
			}
			if (!duplicate)
			{
				vList.push_back(v);
			}
		}
	}
	_nodes = vList;
	vector<CloudNode>().swap(vList);
}

void CloudOctree::setLeafPointer(int _index, CloudOctree* &_leaf)
{
	this->leafs[_index] = _leaf;
}

void CloudOctree::setLeafNodeContainList(vector<CloudNode> _nodes)
{
	contain_.clear();
	for (auto node : _nodes)
	{
		contain_.push_back(node.index());
	}
}

void CloudOctree::generateBranch(CloudOctree*& _btree, vector<CloudNode> &_nodes, vcg::Box3f &_box, CloudOctree *_father)
{
	_btree = new CloudOctree;
	_btree->father_ = _father;
	for (int i = 0; i < 8; ++i)
	{
		_btree->leafs[i] = nullptr;
	}

	if (_nodes.size() <= MIN_CONTAIN_NUM)
	{
		for (auto &node : _nodes)
		{
			_btree->contain_.push_back(node.index());
		}
	}
	else
	{
		float x0, x1, x2;
		x0 = _box.min.X();
		x2 = _box.max.X();
		x1 = (x0 + x2) / 2;
		float y0, y1, y2;
		y0 = _box.min.Y();
		y2 = _box.max.Y();
		y1 = (y0 + y2) / 2;
		float z0, z1, z2;
		z0 = _box.min.Z();
		z2 = _box.max.Z();
		z1 = (z0 + z2) / 2;

		vector<vcg::Box3f> sub_boxes;
		sub_boxes.push_back(vcg::Box3f(Point3f(x0, y1, z0), Point3f(x1, y2, z1)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x1, y1, z0), Point3f(x2, y2, z1)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x1, y1, z1), Point3f(x2, y2, z2)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x0, y1, z1), Point3f(x1, y2, z2)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x0, y0, z0), Point3f(x1, y1, z1)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x1, y0, z0), Point3f(x2, y1, z1)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x1, y0, z1), Point3f(x2, y1, z2)));
		sub_boxes.push_back(vcg::Box3f(Point3f(x0, y0, z1), Point3f(x1, y1, z2)));

		vector<vector<CloudNode>> sub_nodes(8);
		for (auto &node : _nodes)
		{
			for (int i = 0; i < sub_boxes.size(); ++i)
			{
				if (sub_boxes[i].IsIn(node.pos()))
				{
					sub_nodes[i].push_back(node);
					break;
				}
			}
		}

		for (int i = 0; i < 8; ++i)
		{
			if (!sub_nodes[i].empty())
			{
				generateBranch(_btree->leafs[i], sub_nodes[i], sub_boxes[i], _btree);
			}
		}
	}
}

CloudOctree::~CloudOctree()
{
	for (int i = 0; i < 8; ++i)
	{
		if (leafs[i] != nullptr)
		{
			delete leafs[i];
			leafs[i] = nullptr;
		}
	}

	if (!contain_.empty())
	{
		vector<int>().swap(contain_);
		for (int i = 0; i < leaf_balls_.size(); ++i)
		{
			delete leaf_balls_[i];
			leaf_balls_[i] = nullptr;
		}
		vector<HBall*>().swap(leaf_balls_);
	}
	if (ball_ != nullptr)
	{
		delete ball_;
		ball_ = nullptr;
	}
	if (fitting_mesh_ != nullptr)
	{
		delete fitting_mesh_;
		fitting_mesh_ = nullptr;
	}
}

bool CloudOctree::findNearestLeafPoint(int &_ia, float &_aim, vector<CloudNode> &_nodes)
{
	if (this->contain_.empty())
	{
		return false;
	}

	Point3m pos, normal;
	pos = _nodes[_ia].pos();
	normal = _nodes[_ia].normal();

	vector<pair<float, int>> record_list;
	for (auto &index : contain_)
	{
		if (index != _ia)
		{
			Point3m proj_vert = UtilityTools::getInstance()->getProjPointOnPlane(_nodes[index].pos(), pos, -normal);
			Point3m edge = proj_vert - pos;
			record_list.push_back(make_pair(edge * edge, index));
		}
	}

	if (record_list.empty())
	{
		return false;
	}
	else
	{
		sort(record_list.begin(), record_list.end(), cmp_min);
		if (record_list.size() > 6)
		{
			_aim = sqrtf(record_list.at(5).first) * 0.5f;
		}
		else
		{
			_aim = sqrtf(record_list.back().first) * 0.5f;
		}
		vector<pair<float, int>>().swap(record_list);
		return true;
	}
}

void CloudOctree::initalLayerBall(vector<CloudNode>& _nodes)
{
	if (!contain_.empty())
	{
		Point3m tan_vec;

		for (auto index : contain_)
		{
			float radius;
			if (!this->findNearestLeafPoint(index, radius, _nodes))
			{
				radius = MAX_BALL_RADIUS;
			}
			if (radius < MIN_BALL_RADIUS)
			{
				radius = MIN_BALL_RADIUS;
			}
			if (radius > MAX_BALL_RADIUS)
			{
				radius = MAX_BALL_RADIUS;
			}

			Point3m default_color = DEFAULT_COLOR;
			if (this->object_name_ == OCCLUSION_CLOUD)
			{
				default_color = DEFAULT_COLOR2;
			}

			HBall *node_ball = new HBall(_nodes[index].pos(), _nodes[index].normal(), tan_vec, radius, _nodes[index].color(), default_color, true_color_mode_);
			leaf_balls_.push_back(node_ball);
			_nodes[index].setLeafBallPointer(node_ball);
		}
		ball_ = new HBall(leaf_balls_, true_color_mode_);
	}
	else
	{
		for (int i = 0; i < 8; ++i)
		{
		if (leafs[i] != nullptr)
		{
			leafs[i]->initalLayerBall(_nodes);
		}
		}

		vector<HBall*> son_balls;
		for (int i = 0; i < 8; ++i)
		{
			if (leafs[i] != nullptr)
			{
				son_balls.push_back(leafs[i]->ball_);
			}
		}
		ball_ = new HBall(son_balls, true_color_mode_);
	}
}

void CloudOctree::initalLayerBall2()
{
	if (ball_ == nullptr)
	{
		for (int i = 0; i < 8; ++i)
		{
			if (leafs[i] != nullptr)
			{
				leafs[i]->initalLayerBall2();
			}
		}

		vector<HBall*> son_balls;
		for (int i = 0; i < 8; ++i)
		{
			if (leafs[i] != nullptr)
			{
				son_balls.push_back(leafs[i]->ball_);
			}
		}
		ball_ = new HBall(son_balls, true_color_mode_);
	}
}

int CloudOctree::getRenderElementSize()
{
	return render_elements_size_;
}

CMeshO* CloudOctree::getDisplayFittingMesh()
{
	if (this->fitting_mesh_ == nullptr)
	{
		this->fitting_mesh_ = new CMeshO;
	}
	return fitting_mesh_;
}

void CloudOctree::updateAxis()
{
	vcg::Box3f box;
	for (int i = 0; i < subtree_list_.size(); ++i)
	{
		Point4m p(subtree_list_[i]->ballCenter().X(), subtree_list_[i]->ballCenter().Y(), subtree_list_[i]->ballCenter().Z(), 1);
		vcg::Matrix44f matrix(subtree_list_[i]->matrix());
		p = matrix * p;
		box.Add(Point3m(p.X(), p.Y(), p.Z()));
	}
	this->axis_ = Axis(box.Center(), Point3m(0, 1, 0), Point3m(-1, 0, 0), Point3m(0, 0, 1));
}

Matrix44f CloudOctree::updateFixedScreenTransformMatrix(int _w, int _h, const Point3m& _view_direct, const Point3m& _right_direct)
{
	Point3m fixed_t;
	if (this->object_name_ == DENTAL_UPPER)
	{
		fixed_t = dental_upper_screen_fixed_t;
	}
	if (this->object_name_ == DENTAL_LOWER)
	{
		fixed_t = dental_lower_screen_fixed_t;
	}

	int x = (int)(_w * fixed_t.X());
	int y = (int)(_h * fixed_t.Y());
	CMeshO* pCmesh = nullptr;
	Point3d pos = UtilityTools::getInstance()->projectPoint(Point3f(x, y, fixed_t.Z()), pCmesh);
	Point3m fixed_3d_pos(pos.X(), pos.Y(), pos.Z());
	Point3m fixed_axis_y = ((-_view_direct) ^ _right_direct).Normalize();
	Axis fixed_axis(fixed_3d_pos, _right_direct, fixed_axis_y, -_view_direct);
	Point3m move, rotate;
	this->axis_.computeTransformVectors(fixed_axis, move, rotate);

	if (this->fixed_on_screen_changed_)
	{
		HexaVec new_aim_vec(move, rotate);
		setTransferHexavec(new_aim_vec, fixed_scale);
		return Matrix44f::Identity();
	}
	else
	{
		float sca_t = fixed_scale / cur_scale_;
		Point3m zoom(sca_t, sca_t, sca_t);
		this->cur_vec_ = HexaVec(move, rotate, zoom);
		Matrix44f matrix = this->axis_.constructTransformMatrix(cur_vec_);
		return matrix;
	}
}

void CloudOctree::allocateVAOVBOSpace()
{
	static GLenum ERR = glewInit();
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// Bind vertex array object, then bind and set vertex buffer, then configure vertex attributes
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * MAX_POINT_NUM * NODE_ELEMENT_LEN, NULL, GL_DYNAMIC_DRAW);
	old_length = 0;
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool CloudOctree::bindVAOVBO()
{
#if PRINT_DEBUG_INFO_O
	QDateTime dateTime = QDateTime::currentDateTime();
	int start_msec = 0, start_sec = 0, start_min = 0;
	start_msec = dateTime.time().msec();
	start_sec = dateTime.time().second();
	start_min = dateTime.time().minute();
#endif // PRINT_DEBUG_INFO_O

	if (update_render_vertices_list_.empty())
	{
		return false;
	}

	int temp_begin_pos = update_render_vertices_list_.front().first.first;
	int temp_end_pos = update_render_vertices_list_.front().first.second;
	float* temp_new_vertices = update_render_vertices_list_.front().second;

	if (temp_new_vertices == nullptr)
	{
		update_render_vertices_list_.erase(update_render_vertices_list_.begin());
		render_elements_size_ = 0;
		draw_time_ = 0;
		return false;
	}

	glBufferSubData(GL_ARRAY_BUFFER, temp_begin_pos, temp_end_pos, temp_new_vertices);
	if (temp_new_vertices != nullptr)
	{
		delete[]temp_new_vertices;
		temp_new_vertices = nullptr;
	}
	update_render_vertices_list_.erase(update_render_vertices_list_.begin());

	render_elements_size_ = temp_begin_pos / sizeof(float) + temp_end_pos / sizeof(float);
	draw_time_ = render_elements_size_ / NODE_ELEMENT_LEN;

#if PRINT_DEBUG_INFO_O
	dateTime = QDateTime::currentDateTime();
	int end_msec = 0, end_sec = 0, end_min = 0;
	end_msec = dateTime.time().msec();
	end_sec = dateTime.time().second();
	end_min = dateTime.time().minute();
	int intervel_msec = end_msec - start_msec;
	int intervel_sec = end_sec - start_sec;
	int intervel_min = end_min - start_min;
	int draw_intervel = intervel_min * 60000 + intervel_sec * 1000 + intervel_msec;
	qDebug() << "XXXXXXXXXXXXXXXXXXXXXXXX" << new_length << "XXXXXXXXXXXXXXXXXXXXXXXX" << draw_intervel;
#endif // PRINT_DEBUG_INFO_O
	return true;
}

bool CloudOctree::updateWholeTree()
{
	int limit = MAX_POINT_NUM * NODE_ELEMENT_LEN;
	vector<float> render_elements;
	for (int i = 0; i < this->subtree_list_.size(); ++i)
	{
		if (subtree_list_[i] == nullptr)
		{
			continue;
		}

		int old_len = render_elements.size();
		if (old_len >= limit)
		{
			break;
		}

		vector<float> sub_elements;
		drawTreeNode(subtree_list_[i], camera_state, sub_elements);
		if (old_len + sub_elements.size() > limit)
		{
			int over_num = (old_len + sub_elements.size()) / NODE_ELEMENT_LEN;
			over_num = over_num - MAX_POINT_NUM;
			for (int i = 0; i < over_num * NODE_ELEMENT_LEN; ++i)
			{
				sub_elements.pop_back();
			}
		}

		render_elements.insert(render_elements.end(), sub_elements.begin(), sub_elements.end());
	}

	if (render_elements.empty())
	{
		old_length = 0;
		new_length = 0;
		update_render_vertices_list_.push_back(make_pair(make_pair(old_length, new_length), nullptr));
	}
	else
	{
		float* update_vertices = new float[render_elements.size()];
		for (int i = render_elements.size() - 1, j = 0; i >= 0; --i, ++j)
		{
			update_vertices[j] = render_elements[i];
		}
		old_length = 0;
		new_length = sizeof(float) * render_elements.size();
		update_render_vertices_list_.push_back(make_pair(make_pair(old_length, new_length), update_vertices));
	}

	b_updating_whole_tree_ = false;
	vector<float>().swap(render_elements);
	return true;
}

void CloudOctree::drawScreenParts(int _w, int _h, QPainter* _painter)
{
	if (b_fixed_on_screen_)
	{
		drawAligningFrame(_w, _h, _painter);
	}
}

void CloudOctree::draw(int _w, int _h, QPainter *_painter)
{
	if (this->subtree_list_.empty())
	{
		return;
	}
	if (first_time_draw)
	{
		allocateVAOVBOSpace();
		first_time_draw = false;
	}
	if (VAO == 0 || VBO == 0)
	{
		return;
	}

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();

	vcg::Matrix44f fixed_matrix = local_matrix_;

	cur_scale_ = updateCurScaleLength();
	if (this->b_fixed_on_screen_ && this->timer_transfer_ == nullptr)
	{
		Point3m view_direct, right_direct, camera_pos;
		UtilityTools::getInstance()->getCurrentCameraState2(nullptr, _w, _h, view_direct, right_direct, camera_pos);
		fixed_matrix = updateFixedScreenTransformMatrix(_w, _h, view_direct, right_direct);
		fixed_on_screen_changed_ = false;
	}

	if (b_transfer_timer_slot_)
	{
		float sca_t = 1;
		if (animation_aim_scale_ > 0)
		{
			sca_t = animation_aim_scale_ / cur_scale_;
		}
		this->animation_timer_vec_.fZX = sca_t;
		this->animation_timer_vec_.fZY = sca_t;
		this->animation_timer_vec_.fZZ = sca_t;
		fixed_matrix = this->axis_.constructTransformMatrix(animation_timer_vec_);

		b_transfer_timer_slot_ = false;
	}

	glMultMatrix(fixed_matrix);
	cur_fixed_scale_ = updateCurScaleLength();

	Point3m view_direct, right_direct, camera_pos;
	UtilityTools::getInstance()->getCurrentCameraState2(nullptr, _w, _h, view_direct, right_direct, camera_pos);
	camera_state.update(_w, _h, view_direct, right_direct, camera_pos);

	static GLenum ERR = glewInit();
	static QString exePath = QCoreApplication::applicationDirPath();
	static QString vertPath = exePath + QString("/shader/cloud_vert_shader.txt");
	static QString facePath = exePath + QString("/shader/cloud_face_shader.txt");
	static CloudShader *shader_ = new CloudShader(vertPath.toLocal8Bit(), facePath.toLocal8Bit());

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	if (b_need_update_record_trees_ && !first_time_draw && !b_updating_whole_tree_)
	{
		b_updating_whole_tree_ = true;
		handler = std::async(std::launch::async, &CloudOctree::updateWholeTree, this);
		b_need_update_record_trees_ = false;
	}
	if (!update_render_vertices_list_.empty())
	{
		bindVAOVBO();
	}

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, NODE_ELEMENT_LEN * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, NODE_ELEMENT_LEN * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// Color attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, NODE_ELEMENT_LEN * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	// Radius attribute
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, NODE_ELEMENT_LEN * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// Matrix attributes
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, NODE_ELEMENT_LEN * sizeof(float), (void*)(10 * sizeof(float)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, NODE_ELEMENT_LEN * sizeof(float), (void*)(14 * sizeof(float)));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, NODE_ELEMENT_LEN * sizeof(float), (void*)(18 * sizeof(float)));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, NODE_ELEMENT_LEN * sizeof(float), (void*)(22 * sizeof(float)));
	glEnableVertexAttribArray(7);
	// Depth attribute
	glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, NODE_ELEMENT_LEN * sizeof(float), (void*)(26 * sizeof(float)));
	glEnableVertexAttribArray(8);

	float sub_t = 0.4f;
	float ambient_t = 0.1f;
	float diffuse_t = 1.0f;
	float specular_t = 1.0f;
	shader_->use();
	shader_->setMat4("model", camera_state.mm_);
	shader_->setMat4("projection", camera_state.pm_);
	shader_->setVec3("viewPos", camera_state.camera_pos_);
	shader_->setVec3("rightHandDirect", camera_state.right_vec_);
	shader_->setFloat("screenWidth", camera_state.screen_width_);
	shader_->setFloat("screenHeight", camera_state.screen_height_);
	shader_->setBool("lightEnable", true_color_mode_);

	shader_->setVec3("lightDirect[0].direction", 0, 0, 1);
	shader_->setVec3("lightDirect[0].lightcolor", 1, 1, 1);
	shader_->setFloat("lightDirect[0].t", sub_t);
	shader_->setFloat("lightDirect[0].ambientParam", ambient_t);
	shader_->setFloat("lightDirect[0].diffuseParam", diffuse_t);
	shader_->setFloat("lightDirect[0].specularParam", specular_t);

	shader_->setVec3("lightDirect[1].direction", 0, -1, 0);
	shader_->setVec3("lightDirect[1].lightcolor", 1, 1, 1);
	shader_->setFloat("lightDirect[1].t", sub_t);
	shader_->setFloat("lightDirect[1].ambientParam", ambient_t);
	shader_->setFloat("lightDirect[1].diffuseParam", diffuse_t);
	shader_->setFloat("lightDirect[1].specularParam", specular_t);

	shader_->setVec3("lightDirect[2].direction", 1, 0, 0);
	shader_->setVec3("lightDirect[2].lightcolor", 1, 1, 1);
	shader_->setFloat("lightDirect[2].t", sub_t);
	shader_->setFloat("lightDirect[2].ambientParam", ambient_t);
	shader_->setFloat("lightDirect[2].diffuseParam", diffuse_t);
	shader_->setFloat("lightDirect[2].specularParam", specular_t);

	shader_->setVec3("lightDirect[3].direction", -1, 0, 0);
	shader_->setVec3("lightDirect[3].lightcolor", 1, 1, 1);
	shader_->setFloat("lightDirect[3].t", sub_t);
	shader_->setFloat("lightDirect[3].ambientParam", ambient_t);
	shader_->setFloat("lightDirect[3].diffuseParam", diffuse_t);
	shader_->setFloat("lightDirect[3].specularParam", specular_t);

	shader_->setInt("timerCounter1", timer1_counter_);
	shader_->setInt("timerCounter2", timer2_counter_);
	shader_->setInt("timerCounter3", timer3_counter_);
	shader_->setVec3("viewPos", camera_state.camera_pos_);

	glBindVertexArray(VAO);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_POINTS, 0, draw_time_);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glPopMatrix();
	glPopAttrib();
	glFlush();
	glFinish();
}

void CloudOctree::drawAligningFrame(int _w, int _h, QPainter* _p)
{
	_p->save();
	_p->beginNativePainting();
	QPen pen;
	QColor pen_color(100, 100, 100, 255);
	pen.setStyle(Qt::SolidLine);
	pen.setWidth(3);
	pen.setColor(pen_color);
	pen.setCapStyle(Qt::FlatCap);
	pen.setJoinStyle(Qt::MiterJoin);
	_p->setPen(pen);

	Point3m fixed_t;
	if (this->object_name_ == DENTAL_UPPER)
	{
		fixed_t = dental_upper_screen_fixed_t;
	}
	if (this->object_name_ == DENTAL_LOWER)
	{
		fixed_t = dental_lower_screen_fixed_t;
	}
	int center_x = (int)(_w * fixed_t.X());
	int center_y = (int)(_h * (1.0f - fixed_t.Y()));

	Point2m t_list[3];
	t_list[0] = Point2m(0.5f, 1);
	t_list[1] = Point2m(1, 1);
	t_list[2] = Point2m(1, 0.5f);

	float radius_t = 0.15f;
	float radius = _h * radius_t;

	QPoint plist[3];
	for (int i = 0; i < 3; ++i)
	{
		float x = center_x + t_list[i].X() * radius;
		float y = center_y + t_list[i].Y() * radius;
		plist[i] = QPoint(x, y);
	}
	_p->drawPolyline(plist, 3);

	for (int i = 0; i < 3; ++i)
	{
		float x = center_x + -t_list[i].X() * radius;
		float y = center_y + t_list[i].Y() * radius;
		plist[i] = QPoint(x, y);
	}
	_p->drawPolyline(plist, 3);

	for (int i = 0; i < 3; ++i)
	{
		float x = center_x + -t_list[i].X() * radius;
		float y = center_y + -t_list[i].Y() * radius;
		plist[i] = QPoint(x, y);
	}
	_p->drawPolyline(plist, 3);

	for (int i = 0; i < 3; ++i)
	{
		float x = center_x + t_list[i].X() * radius;
		float y = center_y + -t_list[i].Y() * radius;
		plist[i] = QPoint(x, y);
	}
	_p->drawPolyline(plist, 3);

	_p->endNativePainting();
	_p->restore();
}

void CloudOctree::appendRenderElementsWith(CloudOctree* _tree)
{
	int limit = MAX_POINT_NUM * NODE_ELEMENT_LEN;
	static int last_render_elements_size = 0;
	last_render_elements_size = render_elements_size_;
	int old_len = render_elements_size_;
	if (old_len >= limit)
	{
		return;
	}
	old_length = sizeof(float) * old_len;

	vector<float> sub_elements;
	drawTreeNode(_tree, camera_state, sub_elements);
	if (old_len + sub_elements.size() > limit)
	{
		int over_num = (old_len + sub_elements.size()) / NODE_ELEMENT_LEN;
		over_num = over_num - MAX_POINT_NUM;
		for (int i = 0; i < over_num * NODE_ELEMENT_LEN; ++i)
		{
			sub_elements.pop_back();
		}
	}

	new_length = sizeof(float) * sub_elements.size();
	float *update_new_vertices = new float[new_length];
	for (int i = sub_elements.size() - 1, j = 0; i >= 0; --i, ++j)
	{
		update_new_vertices[j] = sub_elements[i];
	}
	update_render_vertices_list_.push_back(make_pair(make_pair(old_length, new_length), update_new_vertices));
	vector<float>().swap(sub_elements);
}

void CloudOctree::drawTreeNode(CloudOctree *_tree, CameraState &_cam, vector<float>& _elements)
{
	if (!_tree->contain_.empty())//leaf node
	{
		drawLeafNodes(_tree, _cam, _elements);
		return;
	}
	else
	{
		drawSonNodes(_tree, _cam, _elements);
		return;
	}
}

void CloudOctree::getBallDepth(HBall *_ball, CameraState &_cam)
{
	vcg::Matrix44f local_matrix(this->matrix_);
	Point4f center = Point4f(_ball->p_.X(), _ball->p_.Y(), _ball->p_.Z(), 1);
	Point4f center_clip = _cam.project_ *_cam.model_ * local_matrix * center;
	_ball->setNdcPos(center_clip);
	_ball->setDepthZ(center_clip.Z());
}

void CloudOctree::getBallProjectRadius(HBall *_ball, CameraState &_cam)
{
#if PERSPECTIVE_STATE
	Point4f center = Point4f(_ball->p_.X(), _ball->p_.Y(), _ball->p_.Z(), 1);
	Point4f center_eye = _cam.model_ * center;
	Point4f center_clip = _cam.project_ * center_eye;
	float proj_radius = (-_cam.n_ * _ball->r_) / center_eye.Z();
#else
	vcg::Matrix44f local_matrix;
	if (this->matrix_ != nullptr)
	{
		local_matrix = vcg::Matrix44f(this->matrix_);
	}
	else
	{
		local_matrix = vcg::Matrix44f::Identity();
	}
	Point4f center = local_matrix * Point4f(_ball->p_.X(), _ball->p_.Y(), _ball->p_.Z(), 1);
	Point4f center_clip = _cam.project_ * _cam.model_ *  center;
	_ball->setNdcPos(center_clip);
	_ball->setDepthZ(center_clip.Z());

	Point3m rnode = Point3m(center.X(), center.Y(), center.Z());
	rnode = rnode + _cam.right_vec_ * _ball->r_;
	Point4m rnode_clip = _cam.project_ * _cam.model_ * Point4m(rnode.X(), rnode.Y(), rnode.Z(), 1);

	Point2f center_screen(_cam.screen_width_ * 0.5f * (_ball->ndcPos().X() + 1), _cam.screen_height_ * 0.5f * (1 - _ball->ndcPos().Y()));
	Point2f rnode_screen(_cam.screen_width_ * 0.5f * (rnode_clip.X() + 1), _cam.screen_height_ * 0.5f * (1 - rnode_clip.Y()));
	Point2f edge = center_screen - rnode_screen;
	float proj_radius = sqrtf(edge * edge) * 1.4f;
#endif
	_ball->setScreenPos(center_screen);
	_ball->setProjectRadius(proj_radius);
}

bool CloudOctree::judgeVisible(CloudOctree *_tree, CameraState &_cam)
{
	if (-1 <= _tree->ball_->ndcPos().X() && _tree->ball_->ndcPos().X() <= 1 &&
		 -1 <= _tree->ball_->ndcPos().Y() && _tree->ball_->ndcPos().Y() <= 1 &&
		 -1 <= _tree->ball_->ndcPos().Z() && _tree->ball_->ndcPos().Z() <= 1)
	{
		return true;
	}
	else
	{
		if (_tree->ball_->projRadius() > MAX_SCREEN_RADIUS)
		{
			return true;
		}
		_tree->ball_->setScreenPos(Point2f(_cam.screen_width_ * 0.5f * (_tree->ball_->ndcPos().X() + 1), _cam.screen_height_ * 0.5f * (1 - _tree->ball_->ndcPos().Y())));
		vcg::Box2f proj_box(Point2f(_tree->ball_->projPos().X(), _tree->ball_->projPos().Y()), _tree->ball_->projRadius() * 1.41415f);
		vcg::Box2f near_box(Point2f(0, 0), Point2f(_cam.screen_width_, _cam.screen_height_));
		if (near_box.Collide(proj_box))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

bool CloudOctree::cullBackFace(HBall *_ball, CameraState &_cam)
{
	return false;
	if (_ball->projRadius() > MAX_SCREEN_RADIUS)
	{
		return false;
	}

	if (_ball->n_  * _cam.view_vec_ >= 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CloudOctree::drawLeafNodes(CloudOctree *_tree, CameraState &_cam, vector<float>& _elements)
{
	vector<pair<double, int>> leaf_list;
	for (int i = 0; i < _tree->leaf_balls_.size(); ++i)
	{
		if (!cullBackFace(_tree->leaf_balls_[i], _cam))
		{
			leaf_list.push_back(make_pair(_tree->leaf_balls_[i]->depthZ(), i));
		}
	}

	for (auto &record : leaf_list)
	{
		_tree->leaf_balls_[record.second]->draw(_elements, _cam, _tree->matrix_);
	}
}

void CloudOctree::drawCurrentNode(CloudOctree *_tree, CameraState &_cam, vector<float>& _elements)
{
	_tree->ball_->draw(_elements, _cam, _tree->matrix_);
}

void CloudOctree::drawSonNodes(CloudOctree *_tree, CameraState &_cam, vector<float>& _elements)
{
	vector<pair<double, int>> son_list;
	for (int i = 0; i < 8; ++i)
	{
		if (_tree->leafs[i] != nullptr)
		{
			son_list.push_back(make_pair(_tree->leafs[i]->ball_->depthZ(), i));
		}
	}

	for (auto &record : son_list)
	{
		drawTreeNode(_tree->leafs[record.second], _cam, _elements);
	}
}

void CloudOctree::copyFromTree(CloudOctree* _tree)
{
	for (int i = 0; i < 8; ++i)
	{
		leafs[i] = _tree->leafs[i];
	}
	ball_ = _tree->ball_;
	matrix_ = _tree->matrix_;
	leaf_balls_ = _tree->leaf_balls_;
	node_cloud_ = _tree->node_cloud_;
}

Axis CloudOctree::alignModelMatrixTo(CloudOctree *_tree, vcg::Box3f &_box, int _w, int _h)
{
	Point3m view_direct, right_direct, camera_pos;
	UtilityTools::getInstance()->getCurrentCameraState2(nullptr, _w, _h, view_direct, right_direct, camera_pos);
	camera_state.update(_w, _h, view_direct, right_direct, camera_pos);

	Axis cur_axis = constructLocalAxis(_tree, _box, this->local_matrix_);
		return cur_axis;
}

Axis CloudOctree::constructLocalAxis(CloudOctree *_tree, vcg::Box3f &_box, const Matrix44f& _father_matrix)
{
	Point3m origin;
	if (_tree->ball_ != nullptr)
	{
		origin = _tree->ball_->p_;
	}
	else
	{
		origin = _box.Center();
	}
	Point3m xnode = origin + Point3m(1, 0, 0);
	Point3m ynode = origin + Point3m(0, 1, 0);
	Point3m znode = origin + Point3m(0, 0, 1);

	vcg::Matrix44f m = _father_matrix * vcg::Matrix44f(_tree->matrix_);
	Point3m t_origin = m * origin;
	Point3m t_xnode = m * xnode;
	Point3m t_ynode = m * ynode;
	Point3m t_znode = m * znode;

	origin = t_origin;
	xnode = (t_xnode - t_origin).Normalize();
	ynode = (t_ynode - t_origin).Normalize();
	znode = (t_znode - t_origin).Normalize();

	return Axis(origin, xnode, ynode, znode);
}

Axis CloudOctree::constructAimViewAxis(CloudOctree *_tree, CameraState &_cam, Axis &_temp_axis)
{
	Point3m axis_z = -_cam.view_vec_;
	Point3m plane_pos = _temp_axis.centerPoint;
	Point3m origin = UtilityTools::getInstance()->getProjPointOnPlane(_cam.camera_pos_, plane_pos, -axis_z);
	Point3m edge = origin - _cam.camera_pos_;
	float distance = sqrtf(edge * edge);
	Point3m aim_camera_pos = _temp_axis.centerPoint + _temp_axis.axisZVector * distance;
	return Axis(aim_camera_pos, _temp_axis.axisXVector, _temp_axis.axisYVector, _temp_axis.axisZVector);
}

void CloudOctree::setTransformMatrix(float *_m)
{
	this->matrix_ = _m;
	for (int i = 0; i < 8; ++i)
	{
		if (this->leafs[i] != nullptr)
		{
			this->leafs[i]->setTransformMatrix(_m);
		}
	}
}

void CloudOctree::startMoving()
{
	min_scree_radius = 10;
}

void CloudOctree::stopMoving()
{
	min_scree_radius = MIN_SCREEN_RADIUS;
}

bool CloudOctree::intersectWithLeafBall(HBall &_ball)
{
	if (ball_->intersectWithBall(_ball, this->matrix_))
	{
		if (!contain_.empty())
		{
			return true;
		}

		for (int i = 0; i < 8; ++i)
		{
			if (this->leafs[i] != nullptr)
			{
				if (this->leafs[i]->intersectWithLeafBall(_ball))
				{
					return true;
				}
			}
		}
		return false;
	}
	else
	{
		return false;
	}
}

vector<int> CloudOctree::collectIntersectNodes(HBall& _ball)
{
	vector<int> inside_nodes_list;
	if (ball_->intersectWithBall(_ball, this->matrix_))
	{
		if (!leaf_balls_.empty() && !contain_.empty() && leaf_balls_.size() == contain_.size())
		{
			int i = 0;
			Point3m pos, edge;
			for (auto &leaf_ball : leaf_balls_)
			{
				pos = leaf_ball->p_;
				edge = pos - _ball.p_;
				if (edge * edge <= _ball.r_ * _ball.r_)
				{
					inside_nodes_list.push_back(contain_[i]);
				}
				++i;
			}
			return inside_nodes_list;
		}

		for (int i = 0; i < 8; ++i)
		{
			if (this->leafs[i] != nullptr)
			{
				vector<int> sub_inside_list = this->leafs[i]->collectIntersectNodes(_ball);
				inside_nodes_list.insert(inside_nodes_list.end(), sub_inside_list.begin(), sub_inside_list.end());
				vector<int>().swap(sub_inside_list);
			}
		}
		return inside_nodes_list;
	}
	else
	{
		return inside_nodes_list;
	}
}

vector<int> CloudOctree::collectIntersectNodes(HBall& _ball, vector<CloudNode>& _nodes)
{
	vector<int> inside_nodes_list;
	if (ball_->intersectWithBall(_ball, this->matrix_))
	{
		if (!contain_.empty())
		{
			Point3m pos, edge;
			for (auto index : contain_)
			{
				pos = _nodes[index].pos();
				edge = pos - _ball.p_;
				if (edge * edge <= _ball.r_ * _ball.r_)
				{
					inside_nodes_list.push_back(index);
				}
			}
			return inside_nodes_list;
		}

		for (int i = 0; i < 8; ++i)
		{
			if (this->leafs[i] != nullptr)
			{
				vector<int> sub_inside_list = this->leafs[i]->collectIntersectNodes(_ball, _nodes);
				inside_nodes_list.insert(inside_nodes_list.end(), sub_inside_list.begin(), sub_inside_list.end());
				vector<int>().swap(sub_inside_list);
			}
		}
		return inside_nodes_list;
	}
	else
	{
		return inside_nodes_list;
	}
}

bool CloudOctree::pickLeafNodesWhichInside(vector<Point2f>& _loop, vector<CloudNode>& _nodes)
{
	vcg::Box2f loop_box2d;
	for (auto vert : _loop)
	{
		loop_box2d.Add(vert);
	}
	bool picked = pickLeafNodesOnTree(_loop, loop_box2d, _nodes);
	return picked;
}

bool CloudOctree::pickLeafNodesOnTree(vector<Point2f>& _loop, vcg::Box2f& _loop_box, vector<CloudNode>& _nodes)
{
	bool picked = false;
	if (!this->contain_.empty())
	{
		for (int i = 0; i < leaf_balls_.size(); ++i)
		{
			getBallProjectRadius(leaf_balls_[i], camera_state);
			if (!UtilityTools::getInstance()->vertOutOfPolygon(leaf_balls_[i]->projPos(), _loop))
			{
				leaf_balls_[i]->color_ = Point3m(255, 0, 0);
				_nodes[contain_[i]].setTempSign(1);
				picked = true;
			}
			else
			{
				if (true_color_mode_)
				{
					leaf_balls_[i]->color_ = _nodes[contain_[i]].color();
				}
				else
				{
					leaf_balls_[i]->color_ = leaf_balls_[i]->default_color_;
				}

				_nodes[contain_[i]].setTempSign(0);
			}
		}
	}
	else
	{
		getBallProjectRadius(this->ball_, camera_state);
		vcg::Box2f cur_box(ball_->projPos(), ball_->projRadius());
		if (_loop_box.Collide(cur_box))
		{
			for (int i = 0; i < 8; ++i)
			{
				if (leafs[i] != nullptr)
				{
					if (leafs[i]->pickLeafNodesOnTree(_loop, _loop_box, _nodes))
					{
						picked = true;
					}
				}
			}
		}
		else
		{
			unPickTree(_nodes);
		}
	}
	return picked;
}

void CloudOctree::unPickTree(vector<CloudNode>& _nodes)
{
	if (!this->contain_.empty())
	{
		for (int i = 0; i < leaf_balls_.size(); ++i)
		{
			if (true_color_mode_)
			{
				leaf_balls_[i]->color_ = _nodes[contain_[i]].color();
			}
			else
			{
				leaf_balls_[i]->color_ = leaf_balls_[i]->default_color_;
			}
			_nodes[contain_[i]].setTempSign(0);
		}
	}
	else
	{
		for (int i = 0; i < 8; ++i)
		{
			if (leafs[i] != nullptr)
			{
				leafs[i]->unPickTree(_nodes);
			}
		}
	}
}

float CloudOctree::sectionDistance(vcg::Matrix44f _matrix, Point3m _pos, Point3m _normal)
{
	_pos = _pos + _normal * 0.1f;
	float min_distance = 0;
	if (!this->contain_.empty())
	{
		for (int i = 0; i < leaf_balls_.size(); ++i)
		{
			float distance = getBallSectionDistance(leaf_balls_[i], _matrix, _pos, _normal);
			if (distance < min_distance)
			{
				min_distance = distance;
			}
		}
		return min_distance;
	}
	else
	{
		float distance = getBallSectionDistance(this->ball_, _matrix, _pos, _normal);
		if (distance < 0)
		{
			for (int i = 0; i < 8; ++i)
			{
				if (leafs[i] != nullptr)
				{
					float temp_distance = leafs[i]->sectionDistance(_matrix, _pos, _normal);
					if (temp_distance < min_distance)
					{
						min_distance = temp_distance;
					}
				}
			}
			return min_distance;
		}
		return 1;
	}
}

float CloudOctree::getBallSectionDistance(HBall* _ball, vcg::Matrix44f& _matrix, Point3m& _pos, Point3m& _normal)
{
	Point3m ball_radius_node = _matrix * (_ball->p_ + Point3m(_ball->r_, 0, 0));
	Point3m ball_center = _matrix * _ball->p_;
	Point3m vec = ball_center - _pos;
	float proj_distance = vec * _normal;

	if (proj_distance < 0)
	{
		return proj_distance;
	}
	else
	{
		float scale_radius = (ball_radius_node - ball_center).Norm();
		return proj_distance - scale_radius;
	}
}

void CloudOctree::setColorMode(bool _mode)
{
	true_color_mode_ = _mode;
}

void CloudOctree::setLeafsUseTrueColor(bool _yes)
{
	if (!this->contain_.empty())
	{
		for (int i = 0; i < leaf_balls_.size(); ++i)
		{
			leaf_balls_[i]->useTrueColor(_yes);
		}
		return;
	}

	for (int i = 0; i < 8; ++i)
	{
		if (leafs[i] != nullptr)
		{
			leafs[i]->setLeafsUseTrueColor(_yes);
		}
	}
}

void CloudOctree::updateWholeTreeData()
{
	b_need_update_record_trees_ = true;
}

int CloudOctree::findNearestNodeWith(Point3m _pos)
{
	if (this->kdtree_ == nullptr)
	{
		this->kdtree_points_.clear();
		for (auto& node : node_cloud_)
		{
			this->kdtree_points_.push_back(node.pos());
		}
		this->kdtree_ = new KdTree<float>(vcg::ConstDataWrapper<Point3m>(this->kdtree_points_.data(), this->kdtree_points_.size()));
	}

	typename KdTree<float>::PriorityQueue pq;
	kdtree_->doQueryK(_pos, 5 + 1, pq);

	vector<int> klist;
	for (int j = 0; j < 5+ 1; ++j)
	{
		int index = pq.getIndex(j);
		return index;
	}
	return 0;
}

void CloudOctree::spreadAnimation(int _layer_num)
{
	if (timer1_ != nullptr)
	{
		timer1_->stop();
		delete timer1_;
		timer1_ = nullptr;
	}

	int time_interval = 5;
	layer_num_ = _layer_num;
	timer_add_step_ = timer_counter_sum_ / _layer_num;

	timer1_counter_ = 0;
	timer2_counter_ = 0;
	timer3_counter_ = 0;

	timer3_ = new QTimer();
	timer2_ = new QTimer(timer3_);
	timer1_ = new QTimer(timer3_);
	connect(timer1_, SIGNAL(timeout()), this, SLOT(timer1Slot()));
	connect(timer2_, SIGNAL(timeout()), this, SLOT(timer2Slot()));
	connect(timer3_, SIGNAL(timeout()), this, SLOT(timer3Slot()));
	timer1_->start(time_interval);
}

float CloudOctree::updateCurScaleLength()
{
	float mm[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mm);
	vcg::Matrix44f model = vcg::Matrix44f(mm).transpose();
	Point3m point1 = model * this->axis_.centerPoint;
	Point3m point2 = model * (this->axis_.centerPoint + this->axis_.axisXVector);
	float scale = sqrtf((point1 - point2) * (point1 - point2));
	return scale;
}

void CloudOctree::clearAllInputData()
{
	for (int i = 0; i < subtree_list_.size(); ++i)
	{
		if (subtree_list_[i] != nullptr)
		{
			delete subtree_list_[i];
			subtree_list_[i] = nullptr;
		}
	}
	vector<vector<int>>().swap(hole_list_);
	vector<CloudNode>().swap(node_cloud_);
	vector<CloudOctree*>().swap(subtree_list_);
	vector<PiecewiseCloud>().swap(input_record_);
	if (fitting_mesh_ != nullptr)
	{
		delete fitting_mesh_;
		fitting_mesh_ = nullptr;
	}

	updateWholeTreeData();
}

void CloudOctree::setTransferHexavec(HexaVec _vec, float _aim_fixed_scale)
{
	if (timer_transfer_ != nullptr)
	{
		timer_transfer_->stop();
		disconnect(timer_transfer_, SIGNAL(timeout()), this, SLOT(timerTransferSlot()));
		delete timer_transfer_;
		timer_transfer_ = nullptr;
	}

	if (_aim_fixed_scale > 0)
	{
		fixed_scale_a_ = cur_fixed_scale_;
		fixed_scale_b_ = _aim_fixed_scale;
		b_transfer_scale_ = true;
	}
	else
	{
		fixed_scale_a_ = -1;
		fixed_scale_b_ = -1;
		b_transfer_scale_ = false;
	}

	emit PSIGNALMANAGER->updateTrackBallEnableSignal(false);
	transfer_aim_vec_ = _vec - cur_vec_;
	timer_transfer_counter_ = 0;
	timer_transfer_ = new QTimer();
	connect(timer_transfer_, SIGNAL(timeout()), this, SLOT(timerTransferSlot()));
	timer_transfer_->start(10);
}

void CloudOctree::timer1Slot()
{
	timer1_counter_ += timer_add_step_;

	if (timer2_ != nullptr && timer1_counter_ == timer_add_step_ * 3)
	{
		timer2_->start(5);
	}

	if (timer1_counter_ >= timer_counter_sum_)
	{
		timer1_->stop();
	}
	emit PSIGNALMANAGER->updateSceneSignal();
}

void CloudOctree::timer2Slot()
{
	timer2_counter_ += timer_add_step_;

	if (timer3_ != nullptr && timer2_counter_ == timer_add_step_ * 3)
	{
		timer3_->start(5);
	}

	if (timer2_counter_ >= timer_counter_sum_)
	{
		timer2_->stop();
	}
	emit PSIGNALMANAGER->updateSceneSignal();
}

void CloudOctree::timer3Slot()
{
	timer3_counter_ += timer_add_step_;

	if (timer3_counter_ >= timer_counter_sum_)
	{
		timer3_->stop();
	}
	emit PSIGNALMANAGER->updateSceneSignal();
}

void CloudOctree::timerTransferSlot()
{
	timer_transfer_counter_ += 1;

	if (timer_transfer_counter_ >= ANIMATION_TIME_LEN)
	{
		timer_transfer_counter_ = 0;
		timer_transfer_->stop();
		disconnect(timer_transfer_, SIGNAL(timeout()), this, SLOT(timerTransferSlot()));
		delete timer_transfer_;
		timer_transfer_ = nullptr;

		this->cur_vec_ = cur_vec_ + transfer_aim_vec_;
		animation_timer_vec_ = this->cur_vec_;
		animation_aim_scale_ = fixed_scale_b_;
		this->b_transfer_timer_slot_ = true;
		emit PSIGNALMANAGER->updateTrackBallEnableSignal(true);
		return;
	}

	float t = (timer_transfer_counter_ / (float)(ANIMATION_TIME_LEN)) * PI * 0.5f;
	t = sin(t);

	float sca_t = -1;
	if (b_transfer_scale_)
	{
		sca_t = (1 - t) * fixed_scale_a_ + t * fixed_scale_b_;
	}
	float x = cur_vec_.fVX + transfer_aim_vec_.fVX * t;
	float y = cur_vec_.fVY + transfer_aim_vec_.fVY * t;
	float z = cur_vec_.fVZ + transfer_aim_vec_.fVZ * t;
	float a = cur_vec_.fA + transfer_aim_vec_.fA * t;
	float b = cur_vec_.fB + transfer_aim_vec_.fB * t;
	float g = cur_vec_.fG + transfer_aim_vec_.fG * t;
	animation_timer_vec_ = HexaVec(x, y, z, a, b, g);
	animation_aim_scale_ = sca_t;
	this->b_transfer_timer_slot_ = true;
	emit PSIGNALMANAGER->updateSceneSignal();
}

void CloudOctree::setDisplayFittingMesh(bool _state)
{
	this->b_display_fitting_mesh_ = _state;
}
