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

#ifndef _COMMON_UNBOUNDBOX
#define  _COMMON_UNBOUNDBOX

#include <iostream>
#include "util/assist_geometry.h"
#include "common_ext_global.h"

using namespace std;
class COMMON_EXT_EXPORT CustomBoundingBox
{
public:
	CustomBoundingBox();
	~CustomBoundingBox();
public:
	Axis localCoordinateSys;
	Point3f maxVert, minVert;
	float fLength, fWidth, fHeight;
	vector<Point3f> constUpLoop, constDownLoop;
	vector<Point3f> upLoop, downLoop;
	// Maximum and minimum values of BORDER
	float Max_X, Min_X;
	float Max_Y, Min_Y;
	float Max_Z, Min_Z;
	// Left and right LEAF of current NODE
	CustomBoundingBox* pLBox;
	CustomBoundingBox* pRBox;
	// Sign of LEAF NODE: IsLeafNode ? FaceIndex : -1
	int iFaceSign;
	// Sign of ROOT NODE: IsRootNode ? true : false
	bool bIsRoot;
	// Bounding box adjustment length value
	float adjValue = 0;
	CMeshO *pMesh;

	void InitBoundBox(int face_sign, bool bIs_Root, float maxX, float minX, float maxY, float minY, float maxZ, float minZ, CustomBoundingBox* leftSubTree, CustomBoundingBox* rightSubTree);
	void InitBoundBox(bool bIs_Root, CustomBoundingBox* boxA, CustomBoundingBox* boxB);
	void InitBoundBox(Axis sys, const vector<Point3m> &vertslist);
	static CustomBoundingBox* AutoBuildTree(int leafNum, CustomBoundingBox* leafList, CustomBoundingBox *&nodeList);
	bool IfIntersectWith(CustomBoundingBox box);
	int GetAllMixedLeaf(CustomBoundingBox aimBox, int &counter, int mixedList[]);
	void GetAllMixedLeafVector(CustomBoundingBox aimBox, vector<int> &result);
	void DestoryBBTree(CustomBoundingBox* tree);

};

#endif // !_COMMON_UNBOUNDBOX
