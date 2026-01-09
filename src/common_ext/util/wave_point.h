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

#ifndef COMMON_WAVE_POINT_H
#define COMMON_WAVE_POINT_H

#define PI 3.1415926
#include "common/ml_mesh_type.h"

#include "common_ext_global.h"

using namespace std;
using namespace vcg;

const int JUST_LINK_IT = 1;
const int BISECT_DIVIDE = 2;
const int TRISECTION_DIVIDE = 3;

const int CAVE_ANGLE = 1;
const int PROTRUDE_ANGLE = 2;

class COMMON_EXT_EXPORT WavePoint
{
public:
	WavePoint();
	WavePoint(int _vi, int _vi_m, int _vi_p, CMeshO *_mesh);
	~WavePoint();

public:
	void inital(int _vi, int _vi_m, int _vi_p, CMeshO *_mesh);
	Point3m getBisectDividePoint();
	Point3m getBisectDividePoint(Point3m edgeIM, Point3m edgeIP, Point3m vertex);
	pair<Point3m, Point3m> getTrisectionDividePoint();
	void getAngleAndAverageLength();

public:
	CMeshO *pMesh;
	int Vi, Vi_mins, Vi_plus;
	Point3m Pi, Pi_mins, Pi_plus;
	float fAngle;
	float fAverageLength;
	Point3m normalVector;
	bool bIsCaveAngle = false;
	vector<int> shareFaces;
};

#endif // !COMMON_WAVE_POINT_H
