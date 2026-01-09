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

#ifndef COMMON_MESH_EDGE_H_
#define COMMON_MESH_EDGE_H_

#include "common/ml_mesh_type.h"

#include "common_ext_global.h"

using namespace std;
using namespace vcg;

class COMMON_EXT_EXPORT MeshEdge
{
public:
	MeshEdge();
	MeshEdge(int indexA, int indexB, int faceIndex, bool isBorder, CMeshO *mesh);
	~MeshEdge();

public:
	CMeshO *pMesh;
	int A_Index, B_Index;
	bool bIsBorder;
	vector<int> shareFaceIndex;

private:
	void initlaMeshEdge(int indexA, int indexB, int faceIndex, bool isBorder, CMeshO *mesh);
};

#endif // !COMMON_MESH_EDGE_H_
