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

#ifndef  CLOUD_SURFACE_RECONSTRUCTION_H
#define CLOUD_SURFACE_RECONSTRUCTION_H

#include "cloudnode.h"
#include "common_ext_global.h"

class CloudSurfaceReconstruction
{
public:
	CloudSurfaceReconstruction(vector<CloudNode> *_nodes);
	~CloudSurfaceReconstruction();
	virtual float getFunctionValue(Point3m &_p, Point3m &_n, Point3m &_color, bool &_too_faraway, float _thickness) = 0;
	void initalKdTree();
	vector<int> collectNearsetVertIndexs(Point3m &_p, int &_size);
	void addNode(Point3m &_p, Point3m &_normal);
protected:
	vector<CloudNode> *p_nodes_ = nullptr;
	KdTree<float> *kt_ = nullptr;
};

#endif // ! CLOUD_SURFACE_RECONSTRUCTION_H
