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

#ifndef BILATERALFILTER_CSR_H
#define BILATERALFILTER_CSR_H

#define K_SIZE 10
#define SIGMA_C 10
#define SIGMA_S 10

#include "cloudsurfacereconstruction.h"
#include "common_ext/common_ext_global.h"

class COMMON_EXT_EXPORT BilateralFilterCSR :
	public CloudSurfaceReconstruction
{
public:
	BilateralFilterCSR(vector<CloudNode> *_nodes);
	~BilateralFilterCSR();
public:
	float getDistanceValue(Point3m& _p);
	float getFunctionValue(Point3m &_p, Point3m &_fit_n, Point3m& _color, bool& _too_faraway, float _thickness);
	float getFunctionValue2(Point3m &_p, Point3m _n, Point3m& _color);
	float getFunctionValue3(Point3m _p, Point3m _n, vector<Point3m> &_neigh_verts);
	bool isOutlier(Point3m& _p, Point3m _n);
	bool isOutlier2(Point3m& _p, const vector<int> &_k_list, float _range);
	void sourceDenoising(int _time);
	void nodeDenoising(CloudNode &_node);
	Point3m fittingNormal(Point3m& _p, vector<int>& _K);
	Point3m fittingColor(vector<int> &_K);
	std::pair<float, float> computeOnceElement(Point3m &_p, Point3m &_near_p, Point3m &_normal);
	float Wn(float &_r);
	float Wc(float &_r);
	float Ws(float &_r);
};

#endif // !BILATERALFILTER_CSR_H
