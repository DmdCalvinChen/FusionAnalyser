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

#ifndef COMMON_CONVEXHULLMACHINE_H
#define COMMON_CONVEXHULLMACHINE_H

#define EPS 1.74532e-4

#include "common/ml_mesh_type.h"
#include "util/assist_geometry.h"
#include <qdebug.h>

#include "common_ext_global.h"

using namespace std;

enum VertState {UNSIGNED=0, INTERIOR, EXTERIOR, BOUNDARY};

struct COMMON_EXT_EXPORT ConvexFace
{
	ConvexFace(){}
	ConvexFace(int _iv1, int _iv2, int _iv3)
	{
		this->ia = _iv1;
		this->ib = _iv2;
		this->ic = _iv3;
		this->bOk = 1;
	}

public:
	bool bOk = 0;
	int ia = -1, ib = -1, ic = -1;
};

class COMMON_EXT_EXPORT ConvexHullMachine
{
public:
	ConvexHullMachine();
	ConvexHullMachine(const vector<Point3m> &_a, Axis _localAxis);
	~ConvexHullMachine();

	void inital(const vector<Point3m>& _a, Axis _localAxis);
	void initalConvexVerts(const vector<Point3m> &_a);

	void initalMachine();
	void creatConvexHull();
	void creatConvexHullSingleStep();
	double dblcmp(Point3m &p, ConvexFace &f);
	void dfs(int p, int now);
	void deal(int p, int a, int b);
	void collectVertsList();

	Point3m getFurtherestVert(Point3m direction, const vector<Point3m> &vertList, int &index);

public:
	////////////////////////////////////////////////
	int num;//当前凸包面数
	int n;//输入点数
	vector<ConvexFace> F;//当前凸体面集
	vector<Point3m> A;//模型点集
	Axis localAxis;//模型局部坐标系
	vector<vector<int>> g;

	vector<Point3m> convexVerts;//凸体点集
	vector<FFace> convexFaces;//凸体面集
	int iCurstep = 4;
};

#endif // !COMMON_CONVEXHULLMACHINE_H
