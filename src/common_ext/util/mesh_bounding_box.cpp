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

#include "mesh_bounding_box.h"

MeshBoundingBox::MeshBoundingBox(){}
MeshBoundingBox::~MeshBoundingBox(){}

void MeshBoundingBox::initBoundBox(int face_sign, bool bIs_Root, CFaceO face, MeshBoundingBox* leftSubTree, MeshBoundingBox* rightSubTree)
{
    this->iFaceSign = face_sign;
    this->bIsRoot = bIs_Root;
    face.GetBBox(this->bbox);
    this->pLBox = leftSubTree;
    this->pRBox = rightSubTree;
}

void MeshBoundingBox::initBoundBox(bool bIs_Root, MeshBoundingBox* boxA, MeshBoundingBox* boxB)
{
	this->iFaceSign = -1;
	this->bIsRoot = bIs_Root;
	this->bbox.Add(boxA->bbox);
	this->bbox.Add(boxB->bbox);
	this->pLBox = boxA;
	this->pRBox = boxB;
}

void MeshBoundingBox::initBoundBox(bool bIs_Root, vcg::tri::UpdateTopology<CMeshO>::PEdge edge, int index)
{
	this->iFaceSign = index;
	this->bIsRoot = bIs_Root;
	this->bbox.Set(edge.v[0]->P());
	this->bbox.Add(edge.v[1]->P());
	this->pLBox = NULL;
	this->pRBox = NULL;
}

void MeshBoundingBox::initBoundBox(Point3m boxCenter, float edgeLength)
{
	this->iFaceSign = -1;
	this->bIsRoot = true;
	this->bbox = vcg::Box3f(boxCenter, edgeLength);
	this->pLBox = NULL;
	this->pRBox = NULL;
}

MeshBoundingBox* MeshBoundingBox::autoBuildTree(int leafNum, MeshBoundingBox* leafList, MeshBoundingBox *&nodeList)
{
	int nodeNum;
	int tempNodeNum;
	int tempOldNodeNum;
	MeshBoundingBox *root;

	//获得将要构建的树的节点数目nodeNum，并对nodeList进行空间分配
	nodeNum = leafNum;
	tempNodeNum = leafNum;
	while (tempNodeNum > 1)
	{
		if (tempNodeNum % 2 == 0)//if the number of node is even num
		{
			tempNodeNum /= 2;
			nodeNum += tempNodeNum;
		}
		else                     //if the number of node is odd num
		{
			nodeNum += (tempNodeNum - 1) / 2;
			tempNodeNum = 1 + (tempNodeNum - 1) / 2;
		}
	}
	nodeList = new MeshBoundingBox[nodeNum];

	//开始自底向上地构建树，所有节点操作均在nodeList上进行
	root = new MeshBoundingBox();
	tempNodeNum = leafNum;
	for (int i = 0; i < leafNum; i++)
		nodeList[i] = leafList[i];

	int idnode, idact;// The index of nodeList and actionSign
	idnode = tempNodeNum;
	idact = 0;
	tempOldNodeNum = tempNodeNum;
	while (tempNodeNum > 1)
	{
		if (tempNodeNum % 2 == 0)//if the number of node is even num
		{
			tempNodeNum /= 2;

			if (tempNodeNum == 1)
			{
				nodeList[idnode].initBoundBox(true, &nodeList[idact], &nodeList[idact + 1]);
				root->initBoundBox(true, &nodeList[idact], &nodeList[idact + 1]);

				break;
			}
			else
			{
				for (; idact < (tempOldNodeNum - 1); idact += 2, idnode++)
				{
					nodeList[idnode].initBoundBox(false, &nodeList[idact], &nodeList[idact + 1]);
				}

				tempOldNodeNum += tempNodeNum;
				idnode = tempOldNodeNum;
			}
		}//end of if
		else                     //if the number of node is odd num
		{
			tempNodeNum = (tempNodeNum - 1) / 2;

			for (; idact < (tempOldNodeNum - 1); idact += 2, idnode++)
			{
				nodeList[idnode].initBoundBox(false, &nodeList[idact], &nodeList[idact + 1]);
			}

			tempOldNodeNum += tempNodeNum;
			idnode = tempOldNodeNum;
			tempNodeNum += 1;
		}//end of else
	}//end of while

	return root;
}

bool MeshBoundingBox::ifIntersectWith(const MeshBoundingBox &box)
{
	return this->bbox.Collide(box.bbox);
}

int MeshBoundingBox::getAllMixedLeaf(const MeshBoundingBox &aimBox, int &counter, int mixedList[])
{
	if (this->ifIntersectWith(aimBox)) //如果当前BoundBox与aimBox相交
	{
		if (this->iFaceSign >= 0)      //如果当前BoundBox是叶节点，计数并退出
		{
			mixedList[counter] = this->iFaceSign;
			counter++;
			return counter;
		}
		else                           //如果当前BoundBox不是叶节点，继续判断其左右子树
		{
			this->pLBox->getAllMixedLeaf(aimBox, counter, mixedList);
			this->pRBox->getAllMixedLeaf(aimBox, counter, mixedList);
		}
	}
	else                               //如果当前BoundBox没有与aimBox相交， 退出
		return counter;
}

void MeshBoundingBox::getAllMixedLeaf(const MeshBoundingBox &aimBox, vector<int> &mixedList)
{
	if (this->ifIntersectWith(aimBox)) //如果当前BoundBox与aimBox相交
	{
		if (this->iFaceSign >= 0)      //如果当前BoundBox是叶节点，计数并退出
		{
			mixedList.push_back(this->iFaceSign);

			return;
		}
		else                           //如果当前BoundBox不是叶节点，继续判断其左右子树
		{
			this->pLBox->getAllMixedLeaf(aimBox, mixedList);
			this->pRBox->getAllMixedLeaf(aimBox, mixedList);
		}
	}
	else                               //如果当前BoundBox没有与aimBox相交， 退出
		return;
}

void MeshBoundingBox::getAllMixedLeafVector(const MeshBoundingBox &aimBox, vector<int> &result)
{
	if (this->ifIntersectWith(aimBox)) //如果当前BoundBox与aimBox相交
	{
		if (this->iFaceSign >= 0)      //如果当前BoundBox是叶节点，计数并退出
		{
			result.push_back(iFaceSign);
			return;
		}
		else                           //如果当前BoundBox不是叶节点，继续判断其左右子树
		{
			this->pRBox->getAllMixedLeafVector(aimBox, result);
			this->pLBox->getAllMixedLeafVector(aimBox, result);
		}
	}
	else
	{
		return;
	}
}
