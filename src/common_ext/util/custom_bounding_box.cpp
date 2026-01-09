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

#include "custom_bounding_box.h"

CustomBoundingBox::CustomBoundingBox()
{
	this->pLBox = NULL;
	this->pRBox = NULL;
	maxVert = Point3m(0, 0, 0);
	minVert = Point3m(0, 0, 0);
	this->adjValue = 0;
}

CustomBoundingBox::~CustomBoundingBox()
{
}

void CustomBoundingBox::InitBoundBox(int face_sign, bool bIs_Root, float maxX, float minX, float maxY, float minY, float maxZ, float minZ,
								CustomBoundingBox* leftSubTree, CustomBoundingBox* rightSubTree)
{
	this->iFaceSign = face_sign;
	this->bIsRoot = bIs_Root;
	this->Max_X = maxX;
	this->Min_X = minX;
	this->Max_Y = maxY;
	this->Min_Y = minY;
	this->Max_Z = maxZ;
	this->Min_Z = minZ;
	this->pLBox = NULL;
	this->pRBox = NULL;
}

void CustomBoundingBox::InitBoundBox(bool bIs_Root, CustomBoundingBox* boxA, CustomBoundingBox* boxB)
{
	this->iFaceSign = -1;
	this->bIsRoot = bIs_Root;
	this->Max_X = boxA->Max_X > boxB->Max_X ? boxA->Max_X : boxB->Max_X;
	this->Min_X = boxA->Min_X < boxB->Min_X ? boxA->Min_X : boxB->Min_X;
	this->Max_Y = boxA->Max_Y > boxB->Max_Y ? boxA->Max_Y : boxB->Max_Y;
	this->Min_Y = boxA->Min_Y < boxB->Min_Y ? boxA->Min_Y : boxB->Min_Y;
	this->Max_Z = boxA->Max_Z > boxB->Max_Z ? boxA->Max_Z : boxB->Max_Z;
	this->Min_Z = boxA->Min_Z < boxB->Min_Z ? boxA->Min_Z : boxB->Min_Z;
	this->pLBox = NULL;
	this->pRBox = NULL;
	this->pLBox = boxA;
	this->pRBox = boxB;
}

void CustomBoundingBox::InitBoundBox(Axis sys, const vector<Point3m> &vertslist)
{
	if (vertslist.empty())  return;
	this->upLoop.clear();
	this->downLoop.clear();
	this->constUpLoop.clear();
	this->constDownLoop.clear();
	this->pLBox = NULL;
	this->pRBox = NULL;
	this->iFaceSign = -1;
	this->bIsRoot = false;

	vector<Point3m> localVerts;
	for (auto vert : vertslist)
	{
		Point3m tempVert;
		float tempX, tempY, tempZ;
		tempVert = vert - sys.centerPoint;
		tempX = tempVert * sys.axisXVector;
		tempY = tempVert * sys.axisYVector;
		tempZ = tempVert * sys.axisZVector;
		localVerts.push_back(Point3m(tempX, tempY, tempZ));
	}

	float maxX, maxY, maxZ, minX, minY, minZ;
	maxX = localVerts[0].X();
	minX = localVerts[0].X();
	maxY = localVerts[0].Y();
	minY = localVerts[0].Y();
	maxZ = localVerts[0].Z();
	minZ = localVerts[0].Z();
	for (auto vert : localVerts)
	{
		if (vert.X() > maxX)
		{
			maxX = vert.X();
		}
		if (vert.Y() > maxY)
		{
			maxY = vert.Y();
		}
		if (vert.Z() > maxZ)
		{
			maxZ = vert.Z();
		}

		if (vert.X() < minX)
		{
			minX = vert.X();
		}
		if (vert.Y() < minY)
		{
			minY = vert.Y();
		}
		if (vert.Z() < minZ)
		{
			minZ = vert.Z();
		}
	}
	maxX = maxX + this->adjValue;
	minX = minX - this->adjValue;
	this->fHeight = maxY - minY;
	this->fWidth = maxZ - minZ;
	this->fLength = maxX - minX;
	this->maxVert = Point3f(maxX, maxY, maxZ);
	this->minVert = Point3f(minX, minY, minZ);

	upLoop.push_back(maxVert - Point3m(1, 0, 0) * fLength - Point3m(0, 0, 1) * fWidth);
	upLoop.push_back(maxVert - Point3m(0, 0, 1) * fWidth);
	upLoop.push_back(maxVert);
	upLoop.push_back(maxVert - Point3m(1, 0, 0) * fLength);

	downLoop.push_back(minVert);
	downLoop.push_back(minVert + Point3m(1, 0, 0) * fLength);
	downLoop.push_back(minVert + Point3m(1, 0, 0) * fLength + Point3m(0, 0, 1) * fWidth);
	downLoop.push_back(minVert + Point3m(0, 0, 1) * fWidth);

	this->maxVert = sys.centerPoint + sys.axisXVector * maxVert.X() + sys.axisYVector * maxVert.Y() + sys.axisZVector * maxVert.Z();
	this->minVert = sys.centerPoint + sys.axisXVector * minVert.X() + sys.axisYVector * minVert.Y() + sys.axisZVector * minVert.Z();
	vector<Point3m> tempList;
	for (auto vert : upLoop)
	{
		tempList.push_back(sys.centerPoint + sys.axisXVector * vert.X() + sys.axisYVector * vert.Y() + sys.axisZVector * vert.Z());
	}
	upLoop.clear();
	upLoop = tempList;
	this->constUpLoop = tempList;
	tempList.clear();
	for (auto vert : downLoop)
	{
		tempList.push_back(sys.centerPoint + sys.axisXVector * vert.X() + sys.axisYVector * vert.Y() + sys.axisZVector * vert.Z());
	}
	downLoop.clear();
	downLoop = tempList;
	this->constDownLoop = tempList;
	tempList.clear();
}

CustomBoundingBox* CustomBoundingBox::AutoBuildTree(int leafNum, CustomBoundingBox* leafList, CustomBoundingBox *&nodeList)
{
	int nodeNum;
	int tempNodeNum;
	int tempOldNodeNum;
	CustomBoundingBox *root;

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
	nodeList = new CustomBoundingBox[nodeNum];

	//开始自底向上地构建树，所有节点操作均在nodeList上进行
	root = new CustomBoundingBox();
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
				nodeList[idnode].InitBoundBox(true, &nodeList[idact], &nodeList[idact + 1]);
				root->InitBoundBox(true, &nodeList[idact], &nodeList[idact + 1]);

				break;
			}
			else
			{
				for (; idact < (tempOldNodeNum - 1); idact += 2, idnode++)
				{
					nodeList[idnode].InitBoundBox(false, &nodeList[idact], &nodeList[idact + 1]);
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
				nodeList[idnode].InitBoundBox(false, &nodeList[idact], &nodeList[idact + 1]);
			}

			tempOldNodeNum += tempNodeNum;
			idnode = tempOldNodeNum;
			tempNodeNum += 1;
		}//end of else
	}//end of while

	return root;
}

bool CustomBoundingBox::IfIntersectWith(CustomBoundingBox box)
{
	bool mixedInXY, mixedInXZ, mixedInYZ;
	bool mixedInDirect1, mixedInDirect2;
	float minA, maxA, minB, maxB;

	//判断是否在XY平面上重叠
	//Xdirection
	minA = this->Min_X; maxA = this->Max_X;
	minB = box.Min_X;   maxB = box.Max_X;
	mixedInDirect1 = (maxA - minB) > 0 ?  (maxA - minB) > ( (maxA - minA) + (maxB - minB) ) ? false : true  : false;
	//Ydirection
	minA = this->Min_Y; maxA = this->Max_Y;
	minB = box.Min_Y;   maxB = box.Max_Y;
	mixedInDirect2 = (maxA - minB) > 0 ? (maxA - minB) > ((maxA - minA) + (maxB - minB)) ? false : true : false;
	if (mixedInDirect1 && mixedInDirect2)
		mixedInXY = true;
	else
		mixedInXY = false;

	//判断是否在XZ平面上重叠
	//Xdirection
	minA = this->Min_X; maxA = this->Max_X;
	minB = box.Min_X;   maxB = box.Max_X;
	mixedInDirect1 = (maxA - minB) > 0 ? (maxA - minB) > ((maxA - minA) + (maxB - minB)) ? false : true : false;
	//Zdirection
	minA = this->Min_Z; maxA = this->Max_Z;
	minB = box.Min_Z;   maxB = box.Max_Z;
	mixedInDirect2 = (maxA - minB) > 0 ? (maxA - minB) > ((maxA - minA) + (maxB - minB)) ? false : true : false;
	if (mixedInDirect1 && mixedInDirect2)
		mixedInXZ = true;
	else
		mixedInXZ = false;

	//判断是否在ZY平面上重叠
	//Zdirection
	minA = this->Min_Z; maxA = this->Max_Z;
	minB = box.Min_Z;   maxB = box.Max_Z;
	mixedInDirect1 = (maxA - minB) > 0 ? (maxA - minB) > ((maxA - minA) + (maxB - minB)) ? false : true : false;
	//Ydirection
	minA = this->Min_Y; maxA = this->Max_Y;
	minB = box.Min_Y;   maxB = box.Max_Y;
	mixedInDirect2 = (maxA - minB) > 0 ? (maxA - minB) > ((maxA - minA) + (maxB - minB)) ? false : true : false;
	if (mixedInDirect1 && mixedInDirect2)
		mixedInYZ = true;
	else
		mixedInYZ = false;

	if (mixedInXY && mixedInXZ && mixedInYZ)
		return true;
	else
		return false;
}

int CustomBoundingBox::GetAllMixedLeaf(CustomBoundingBox aimBox, int &counter, int mixedList[])
{
	if (this->IfIntersectWith(aimBox)) //如果当前BoundBox与aimBox相交
	{
		if (this->iFaceSign >= 0)      //如果当前BoundBox是叶节点，计数并退出
		{
			mixedList[counter] = this->iFaceSign;
			counter++;
			return counter;
		}
		else                           //如果当前BoundBox不是叶节点，继续判断其左右子树
		{
			this->pLBox->GetAllMixedLeaf(aimBox, counter, mixedList);
			this->pRBox->GetAllMixedLeaf(aimBox, counter, mixedList);
		}
	}
	else                               //如果当前BoundBox没有与aimBox相交， 退出
		return counter;
}

void CustomBoundingBox::GetAllMixedLeafVector(CustomBoundingBox aimBox, vector<int> &result)
{
	if (this->IfIntersectWith(aimBox)) //如果当前BoundBox与aimBox相交
	{
		if (this->iFaceSign >= 0)      //如果当前BoundBox是叶节点，计数并退出
		{
			result.push_back(iFaceSign);
			return;
		}
		else                           //如果当前BoundBox不是叶节点，继续判断其左右子树
		{
			this->pRBox->GetAllMixedLeafVector(aimBox, result);
			this->pLBox->GetAllMixedLeafVector(aimBox, result);
		}
	}
	else
	{
		return;
	}
}

void CustomBoundingBox::DestoryBBTree(CustomBoundingBox* tree)
{
	if (tree)
	{
			DestoryBBTree(tree->pLBox);

			DestoryBBTree(tree->pRBox);

			delete tree;
			tree = NULL;
	}
}
