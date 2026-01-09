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

#ifndef COMMON_DEFINITE_INTERSECTION_
#define COMMON_DEFINITE_INTERSECTION_

#include "common/ml_mesh_type.h"
#include "util/assist_geometry.h"

#include "common_ext_global.h"

using namespace std;
using namespace vcg;

class COMMON_EXT_EXPORT DefiniteIntersection
{
public:
	DefiniteIntersection();
	DefiniteIntersection(Point3f _intersection);
	DefiniteIntersection(Point3f _intersection, FEdge edge);
	DefiniteIntersection(Point3f _intersection, CMeshO *pMesh);
	DefiniteIntersection(Point3f _intersection, FEdge edge, CMeshO *pMesh);
	~DefiniteIntersection();

	void collectInformation(Point3f vert);
	void collectInformation(FEdge edge);
	template<typename T> bool exitInVector(T t, vector<T> list);
	void getMeshAndUpdate(CMeshO* pMesh);
	void transformation(vcg::Matrix44f transform);
public:
	Point3f intersection;
	vector<FEdge> edgeList;
	vector<int> faceList;
	CMeshO *meshPointer = NULL;
private:
	bool ensureEdge;
};

#endif // !COMMON_DEFINITE_INTERSECTION_
