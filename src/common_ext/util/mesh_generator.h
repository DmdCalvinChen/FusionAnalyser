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

#ifndef  COMMON_MESH_GENERATOR_
#define COMMON_MESH_GENERATOR_

#include "common/ml_mesh_type.h"
#include "common/meshmodel.h"
#include "util/assist_geometry.h"
#include "common_ext_global.h"
#include <vcg/complex/algorithms/update/curvature.h>

using namespace std;
using namespace vcg;

class COMMON_EXT_EXPORT  MeshGenerator
{
public:
public:
	MeshGenerator();
	~MeshGenerator();

public:
	CVertexO* addOrFindThisPointsPointer(Point3f p, int startPos, MeshModel *aimMode);
	void generateNewVertexsAndNewFaces(vector<FFace> &faceList, MeshModel *aimMode, int vertStartPos);
	void addNewMeshToAimMeshMode(std::vector<FFace> &addingFaceList, MeshModel *aimMode, int iQValue, vcg::Color4b wantColor);
	void addNewMeshToAimMeshMode(std::vector<FFace> &addingFaceList, CMeshO *aimMesh, int iQValue, vcg::Color4b wantColor, int default_start_index = -1);
	void meshAppend(CMeshO *_augend_mesh, CMeshO *_addend_mesh);

	CVertexO* addOrFindThisPointsPointer(Point3f p, int startPos, CMeshO* aimMesh);
	int addOrFindThisPointsPointer2(Point3f p, int startPos, CMeshO *aimMesh);
	void generateNewVertexsAndNewFaces(vector<FFace>& faceList, CMeshO* aimMesh, int vertStartPos);
	void generateNewVertexsAndNewFaces2(vector<FFace> &faceList, CMeshO *aimMesh, int vertStartPos);
	void addNewMeshToAimMesh(vector<FFace> addingFaceList, CMeshO *aimMesh, int iQValue, vcg::Color4b wantColor);
	void addNewMeshToAimMeshWithVertlist(const vector<Point3m> &_vertList, const vector<FFace> &_faceList, CMeshO *aimMesh, int iQValue, vcg::Color4b wantColor);
	void addAllFaces2(const vector<FPoint> &vertList, const vector<FFace> &faceList, CMeshO *mesh);
	void addAllFaces(vector<FFace> faceList, CMeshO *mesh);

    void collectNoRepeatVertexes(const std::vector<FFace> &facelist, std::vector<Point3f> &result);
	void matchingFaceWithVertexPointers(vector<FFace> &facelist, const std::vector<CMeshO::VertexPointer> &vertPointerList);

	vector<int> getOneRingNeighborVerts(int ivert, CMeshO *curMesh);
	vector<int> getOneRingNeighborFaces(int ivert, CMeshO *curMesh);
	void fixReversePatch(vector<int> _seeds, CMeshO *_curMesh);

	void updateModel(CMeshO *curMesh);
	void deleteFacesInList(vector<int> faceList, CMeshO *curMesh);
	void transformer(CMeshO *curMesh, Axis curSys, Axis aimSys);
	void meshDeformer(CMeshO *curMesh, Axis curSys, Axis aimSys);
	void meshDeformer(CMeshO *curMesh, const vcg::Matrix44f &_matrix);
};

#endif //  COMMON_MESH_GENERATOR_
