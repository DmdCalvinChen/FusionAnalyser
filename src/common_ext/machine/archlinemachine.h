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

#ifndef COMMON_ARCHLINEMACHINE
#define COMMON_ARCHLINEMACHINE

#include "common/ml_mesh_type.h"
#include "util/assist_geometry.h"
#include "util/custom_plane.h"

#include "common_ext_global.h"

using namespace std;

class COMMON_EXT_EXPORT ArchLineMachine
{
public:
	ArchLineMachine();
	ArchLineMachine(CustomPlane _plane, vector<Point3m> _inputNodes, bool _upperDental);
	~ArchLineMachine();

public:
	Point3m dimensionReduction(Point3m p);
	Point3m dimensionIncrese(Point3m p);
	void GenerateDentalArch();
	void GetDentalArchLine(int i1, int i2, int i3, vector<Point3m> &DA, float &Length);
	void ResortCtrlPoints();

	bool bUpperDental;
	CustomPlane curPlane;
	float fDentalArchLength = 0;
	vector<Point3m> inputNodes;
	vector<Point3m> plane2DNodes, plane3DNodes;
	vector<Point3m> archLineNodes;
};

#endif // !COMMON_ARCHLINEMACHINE
