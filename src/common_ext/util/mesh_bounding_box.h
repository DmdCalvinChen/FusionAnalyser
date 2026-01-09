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

#ifndef COMMON_MESH_BOUNDING_BOXBOX_H_
#define COMMON_MESH_BOUNDING_BOXBOX_H_

#include <iostream>
#include <vector>
#include <vcg/complex/complex.h>
#include <wrap/io_trimesh/import.h>
#include <wrap/io_trimesh/export_ply.h>

#include "common/ml_mesh_type.h"

#include "common_ext_global.h"

using namespace std;
/**MeshBoundingBox:包围盒二叉树节点类
 *
 * 包含包围盒树节点需要的成员变量
 * 例如树的左分支，右分支，树根，包围盒大小
 * 提供了构建单一包围盒树节点的方法
 * 提供了包围盒之间相交检测，融合等方法
 * 实现了自动构造包围盒二叉树的方法
 */
class COMMON_EXT_EXPORT MeshBoundingBox
{
public:
	MeshBoundingBox();
	~MeshBoundingBox();

public:
	vcg::Box3f bbox;
	MeshBoundingBox* pLBox;
	MeshBoundingBox* pRBox;
	//The Sign of LEAF NODE: IsLeafNode ? FaceIndex : -1
	int iFaceSign;
	//The Sigh of ROOT NODE: IsRootNode ? true : false
	bool bIsRoot;

	void initBoundBox(int face_sign, bool bIs_Root, CFaceO face, MeshBoundingBox* leftSubTree, MeshBoundingBox* rightSubTree);
	void initBoundBox(bool bIs_Root, MeshBoundingBox* boxA, MeshBoundingBox* boxB);
	void initBoundBox(Point3m boxCenter, float edgeLength);
	void initBoundBox(bool bIs_Root, vcg::tri::UpdateTopology<CMeshO>::PEdge edge, int index);
	static MeshBoundingBox* autoBuildTree(int leafNum, MeshBoundingBox* leafList, MeshBoundingBox *&nodeList);
	bool ifIntersectWith(const MeshBoundingBox &box);
	int getAllMixedLeaf(const MeshBoundingBox &aimBox, int &counter, int mixedList[]);
	void getAllMixedLeaf(const MeshBoundingBox &aimBox, vector<int> &mixedList);
	void getAllMixedLeafVector(const MeshBoundingBox &aimBox, vector<int> &result);
};

#endif // COMMON_MESH_BOUNDING_BOXBOX_H_

