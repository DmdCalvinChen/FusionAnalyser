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

#include "cloudsurfacereconstruction.h"

#include "common/config.h"

CloudSurfaceReconstruction::CloudSurfaceReconstruction(vector<CloudNode> *_nodes)
{
	this->p_nodes_ = _nodes;
	if (_nodes != nullptr && !_nodes->empty())
	{
		initalKdTree();
	}
}

CloudSurfaceReconstruction::~CloudSurfaceReconstruction()
{
	SAFE_DELETE(kt_);
}

void CloudSurfaceReconstruction::initalKdTree()
{
	if (p_nodes_ == nullptr)
	{
		return;
	}
	this->kdtree_points_.clear();
	for (int i = 0; i < p_nodes_->size(); ++i)
	{
		this->kdtree_points_.push_back(p_nodes_->at(i).pos());
	}
	if (this->kt_ != nullptr)
	{
		delete this->kt_;
		this->kt_ = nullptr;
	}
	this->kt_ = new KdTree<float>(vcg::ConstDataWrapper<Point3m>(this->kdtree_points_.data(), this->kdtree_points_.size()));
}

vector<int> CloudSurfaceReconstruction::collectNearsetVertIndexs(Point3m &_p, int &_size)
{
	if (kt_ == nullptr || p_nodes_ == nullptr)
	{
		return vector<int>();
	}

	typename KdTree<float>::PriorityQueue pq;
	kt_->doQueryK(_p, _size + 1, pq);

	vector<int> klist;
	for (int j = 0; j < _size + 1; ++j)
	{
		int index = pq.getIndex(j);
		klist.push_back(index);
	}
	return klist;
}

void CloudSurfaceReconstruction::addNode(Point3m &_p, Point3m &_normal)
{
	int index = this->p_nodes_->size();
	CloudNode n(index, _p, _normal);
	n.setColor(Point3m(255, 255, 0));
	this->p_nodes_->push_back(n);
}
