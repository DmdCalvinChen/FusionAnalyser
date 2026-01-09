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

#ifndef COMMON_MASS_POINT_H
#define COMMON_MASS_POINT_H
#include "common/ml_mesh_type.h"

#include "common_ext_global.h"

using namespace std;
using namespace vcg;

class COMMON_EXT_EXPORT  MassPoint
{
public:
	MassPoint();
	MassPoint(int _vertIndex);
	~MassPoint();

	void getAdjacentMeshVertIndexes(vector<int> _adjVertIndexes);
	void setBalanceState(bool _state);
public:
	bool bIsBalanced = true;
	int iVertInMeshIndex = -1;
	vector<int> adj_VertInMeshIndexList;
	vector<float> adj_EdgeLength;
};

#endif // !COMMON_MASS_POINT_H
