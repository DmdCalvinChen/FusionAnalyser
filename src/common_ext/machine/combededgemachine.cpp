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

#include "combededgemachine.h"

CombedEdgeMachine::CombedEdgeMachine()
{
	standNormalV = Point3m(0, 0, 1);
	this->ifuck = 0;
}

CombedEdgeMachine::CombedEdgeMachine(vector<Point3m> _nodelist, vector<FEdge> _edgelist, vector<FFace> _facelist)
{
	standNormalV = Point3m(0, 0, 1);
	this->nodelist = _nodelist;
	this->edgelist = _edgelist;
	this->facelist = _facelist;
	this->ifuck = 0;
}

CombedEdgeMachine::~CombedEdgeMachine()
{
}

void CombedEdgeMachine::initalMachine(FEdge edge, vector<int> indexList)
{
	standNormalV = Point3m(0, 0, 1);
	this->aimEdge = edge;
	this->combedAreaIndex = indexList;
	this->dynamicAreaList = indexList;
	this->embedSteps.clear();
}

void CombedEdgeMachine::initalMachine(FEdge edge, vector<int> indexList, vector<Point3m> _nodelist, vector<FEdge> _edgelist, vector<FFace> _facelist)
{
	standNormalV = Point3m(0, 0, 1);
	this->aimEdge = edge;
	this->combedAreaIndex = indexList;
	this->dynamicAreaList = indexList;
	this->nodelist = _nodelist;
	this->edgelist = _edgelist;
	this->facelist = _facelist;
	this->embedSteps.clear();
	this->iTime = 0;
}

void CombedEdgeMachine::updateCombedRequirement()
{
	vector<FFace>().swap(area);
	vector<FEdge>().swap(edgeCollect);
	vector<FEdge>().swap(unBorderlist);
	vector<FPoint>().swap(tempPointlist);
	vector<FPoint>().swap(QR);
	vector<FPoint>().swap(QL);
	for (auto index : dynamicAreaList)
	{
		this->area.push_back(facelist[index]);
	}
	// Get all boundary edges
	bool newEdge;
	Point3m vLeft, vRight;
	Point3m normalV;
	vector<FEdge> polyBoundary;
	for (auto face : this->area)
	{
		newEdge = true;
		for (auto edge : polyBoundary)
		{
			if (edge == face.edge1)
			{
				newEdge = false;
				break;
			}
		}
		if (newEdge)
		{
			polyBoundary.push_back(edgelist[face.edge1Index]);
		}

		newEdge = true;
		for (auto edge : polyBoundary)
		{
			if (edge == face.edge2)
			{
				newEdge = false;
				break;
			}
		}
		if (newEdge)
		{
			polyBoundary.push_back(edgelist[face.edge2Index]);
		}

		newEdge = true;
		for (auto edge : polyBoundary)
		{
			if (edge == face.edge3)
			{
				newEdge = false;
				break;
			}
		}
		if (newEdge)
		{
			polyBoundary.push_back(edgelist[face.edge3Index]);
		}
	}
	edgeCollect = polyBoundary; // Get unique edge list of triangles in current area

	vector<FEdge> tempEdgeList;
	for (auto edge : polyBoundary)
	{
		bool exit1 = false, exit2 = false;
		if (edge.faceA >= 0)
		{
			for (auto index : this->dynamicAreaList)
			{
				if (edge.faceA == index)
				{
					exit1 = true;
					break;
				}
			}
		}
		if (edge.faceB >= 0)
		{
			for (auto index : this->dynamicAreaList)
			{
				if (edge.faceB == index)
				{
					exit2 = true;
					break;
				}
			}
		}

		if (!exit1 || !exit2)
		{
			tempEdgeList.push_back(edge);
		}
		else
		{
			unBorderlist.push_back(edge);
		}
	}
	polyBoundary = tempEdgeList;
	tempEdgeList.clear();

	// Sort boundary edges counter-clockwise
	int AimEnd = this->aimEdge.indexA;
	int startIndex = this->aimEdge.indexA;
	int Time = polyBoundary.size();
	if (Time == 0)
	{
		return;
	}
	for (int I = 0; I < Time; I++)
	{
		int i = 0;
		for (auto edge : polyBoundary)
		{
			if (startIndex == edge.indexA)
			{
				tempEdgeList.push_back(edge);
				startIndex = edge.indexB;

				vector<FEdge> templist;
				for (int j = 0; j < polyBoundary.size(); j++)
				{
					if (j != i)
					{
						templist.push_back(polyBoundary[j]);
					}
				}
				polyBoundary.clear();
				polyBoundary = templist;
				templist.clear();

				break;
			}
			else if (startIndex == edge.indexB)
			{
				edge.exchangeItself();
				tempEdgeList.push_back(edge);
				startIndex = edge.indexB;

				vector<FEdge> templist;
				for (int j = 0; j < polyBoundary.size(); j++)
				{
					if (j != i)
					{
						templist.push_back(polyBoundary[j]);
					}
				}
				polyBoundary.clear();
				polyBoundary = templist;
				templist.clear();

				break;
			}
			else
			{
				i++;
			}
		}
	}
	polyBoundary.clear();
	polyBoundary = tempEdgeList;
	tempEdgeList.clear();

	vLeft = nodelist[polyBoundary[Time - 1].indexB] - nodelist[polyBoundary[Time - 1].indexA];
	vRight = nodelist[polyBoundary[0].indexB] - nodelist[polyBoundary[0].indexA];
	normalV = vLeft ^ vRight;
	if (normalV * standNormalV < 0)
	{
		for (int i = polyBoundary.size() - 1; i >= 0; i--)
		{
			tempEdgeList.push_back(polyBoundary[i]);
		}
		for (int i = 0; i < tempEdgeList.size(); i++)
		{
			tempEdgeList[i].exchangeItself();
		}
		polyBoundary.clear();
		polyBoundary = tempEdgeList;
		tempEdgeList.clear();
	}

	//获取区域所有无重FPoint-node
	for (auto triangle : this->area)
	{
		bool newNode;

		newNode = true;
		for (auto node : tempPointlist)
		{
			if (triangle.n1 == node.n)
			{
				newNode = false;
				break;
			}
		}
		if (newNode)
		{
			tempPointlist.push_back(FPoint(nodelist[triangle.n1], triangle.n1));
		}

		newNode = true;
		for (auto node : tempPointlist)
		{
			if (triangle.n2 == node.n)
			{
				newNode = false;
				break;
			}
		}
		if (newNode)
		{
			tempPointlist.push_back(FPoint(nodelist[triangle.n2], triangle.n2));
		}

		newNode = true;
		for (auto node : tempPointlist)
		{
			if (triangle.n3 == node.n)
			{
				newNode = false;
				break;
			}
		}
		if (newNode)
		{
			tempPointlist.push_back(FPoint(nodelist[triangle.n3], triangle.n3));
		}
	}
	//更新各点的凹凸性
	for (int i = 0; i < tempPointlist.size(); i++)
	{
		if (tempPointlist[i].n == aimEdge.indexA || tempPointlist[i].n == aimEdge.indexB)
		{
			tempPointlist[i].bSalient = true;
		}
		else
		{
			for (int j = 0; j < polyBoundary.size(); j++)
			{
				if (polyBoundary[j].indexA == tempPointlist[i].n)
				{//................................................................
					vLeft = nodelist[polyBoundary[j - 1].indexB] - nodelist[polyBoundary[j -1].indexA];
					vRight = nodelist[polyBoundary[j].indexB] - nodelist[polyBoundary[j].indexA];
					normalV = vLeft ^ vRight;
					if (normalV * standNormalV <= 0)
					{
						tempPointlist[i].bSalient = false;
					}
					else
					{
						tempPointlist[i].bSalient = true;
					}
				}
			}
		}
	}

	//更新了凹凸性的各顶点划分到QR，QL中
	for (int i = 0; i < tempPointlist.size(); i++)
	{
		if (tempPointlist[i].n != aimEdge.indexA && tempPointlist[i].n != aimEdge.indexB)
		{
			vLeft = tempPointlist[i].point - nodelist[aimEdge.indexA];
			vRight = nodelist[aimEdge.indexB] - nodelist[aimEdge.indexA];
			normalV = vLeft ^ vRight;
			if (normalV * standNormalV < 0)
			{
				this->QR.push_back(tempPointlist[i]);
			}
			else
			{
				this->QL.push_back(tempPointlist[i]);
			}
		}
	}
	//按照从aimDiagonal起始点到终点方向进行排序
	int i, j;
	float temp;
	FPoint tempFP;
	vector<float> dotMulitV;
	Point3m tempV;
	Point3m aimV = (nodelist[aimEdge.indexB] - nodelist[aimEdge.indexA]).Normalize();
	for (auto p : this->QR)
	{
		tempV = p.point - nodelist[aimEdge.indexA];
		dotMulitV.push_back(tempV * aimV);
	}
	for (i = dotMulitV.size() - 1; 0 < i; i--)
	{
		for (j = 0; j < i; j++)
		{
			if (dotMulitV[j] > dotMulitV[j + 1])
			{
				temp = dotMulitV[j];
				dotMulitV[j] = dotMulitV[j + 1];
				dotMulitV[j + 1] = temp;

				tempFP = this->QR[j];
				QR[j] = QR[j + 1];
				QR[j + 1] = tempFP;
			}
		}
	}
	dotMulitV.clear();

	for (auto p : this->QL)
	{
		tempV = p.point - nodelist[aimEdge.indexA];
		dotMulitV.push_back(tempV * aimV);
	}
	for (i = dotMulitV.size() - 1; 0 < i; i--) {
		for (j = 0; j < i; j++) {
			if (dotMulitV[j] > dotMulitV[j + 1]) {
				temp = dotMulitV[j];
				dotMulitV[j] = dotMulitV[j + 1];
				dotMulitV[j + 1] = temp;

				tempFP = this->QL[j];
				QL[j] = QL[j + 1];
				QL[j + 1] = tempFP;
			}
		}
	}
	dotMulitV.clear();
}

int CombedEdgeMachine::getEdgeIndexInList(vector<FEdge> list, FEdge aim)
{
	for (int i = 0; i < list.size(); i++)
	{
		if (aim.isSameAs(list[i]))
		{
			return i;
		}
	}

	return -1;
}

int CombedEdgeMachine::getEdgeIndexInList(set<int> indexlist, Point2i aim)
{
	for (auto index : indexlist)
	{
		if (edgelist[index] == aim)
		{
			return index;
		}
	}
	return -1;
}

FEdge CombedEdgeMachine::pickOutAimDiagonal()
{
	FPoint leftNode;
	FEdge aimDiagonal;
	bool findDiagonal = false;
	for (int i = 0; i < this->QL.size(); i++)
	{
		if (QL[i].bSalient)
		{
			leftNode = QL[i];
			for (auto vert : this->QR)
			{
				Point2i temp(leftNode.n, vert.n);
				bool find = false;
				//找到凸角对边，则将其作为aimDiagonal
				for (auto edge : edgeCollect)
				{
					if (edge == temp && vert.bSalient)
					{
						aimDiagonal = edge;
						find = true;
						break;
					}
				}
				//找到对边但另一角非凸角，则判断该对边上相邻三角组成的四边形
				//其中另一角是否为凸角.如果是凸角则亦可将其作为aimDiagonal
				if (!find)
				{
					for (auto edge : edgeCollect)
					{
						if (edge == temp && !vert.bSalient)
						{
							if (isSalientAngleInQuadrilateral(edge, temp.Y()))
							{
								aimDiagonal = edge;
								find = true;
								break;
							}
						}
					}
				}

				if (find)
				{
					findDiagonal = true;
					break;
				}
			}

			if (findDiagonal)
			{
				break;
			}
		}
	}

	if (findDiagonal)
	{
		aimDiagonalIndex = getEdgeIndexInList(edgelist, aimDiagonal);
		return aimDiagonal;
	}
	else
	{
		aimDiagonalIndex = -1;
		return FEdge(-1, -1);
	}
}

FPoint CombedEdgeMachine::getOppositeVertToEdge(FEdge aimDiagonal, int face)
{
	int n;

	if (facelist[face].n1 != aimDiagonal.indexA && facelist[face].n1 != aimDiagonal.indexB)
	{
		n = facelist[face].n1;
	}
	else if (facelist[face].n2 != aimDiagonal.indexA && facelist[face].n2 != aimDiagonal.indexB)
	{
		n = facelist[face].n2;
	}
	else
	{
		n = facelist[face].n3;
	}

	return FPoint(nodelist[n], n);
}

void CombedEdgeMachine::findAssociatedEdgesWith(int iVert, FEdge exceptEdge, int faceA, int faceB,
	                                            FEdge &newEdge1, FEdge &newEdge2)
{
	int I[3] = { facelist[faceA].edge1Index, facelist[faceA].edge2Index, facelist[faceA].edge3Index};
	for (int i = 0; i < 3; i++)
	{
		if (I[i] != aimDiagonalIndex)
		{
			if (edgelist[I[i]].indexA == iVert || edgelist[I[i]].indexB == iVert)
			{
				newEdge1 = edgelist[I[i]];
				break;
			}
		}
	}

	int J[3] = { facelist[faceB].edge1Index, facelist[faceB].edge2Index, facelist[faceB].edge3Index };
	for (int i = 0; i < 3; i++)
	{
		if (J[i] != aimDiagonalIndex)
		{
			if (edgelist[J[i]].indexA == iVert || edgelist[J[i]].indexB == iVert)
			{
				newEdge2 = edgelist[J[i]];
				break;
			}
		}
	}
}

bool CombedEdgeMachine::ensureFaceIndex(FFace newFace, int &faceIndex, int &anOtherfaceIndex, int faceA, int faceB)
{
	for (auto edge : this->unBorderlist)
	{
		if (edge.isSameAs(edgelist[newFace.edge1Index]))
		{
			if (edge.faceA == faceA)
			{
				faceIndex = faceA;
				anOtherfaceIndex = faceB;
				return true;
			}
			else if (edge.faceA == faceB)
			{
				faceIndex = faceB;
				anOtherfaceIndex = faceA;
				return true;
			}
			else if (edge.faceB == faceA)
			{
				faceIndex = faceA;
				anOtherfaceIndex = faceB;
				return true;
			}
			else if (edge.faceB == faceB)
			{
				faceIndex = faceB;
				anOtherfaceIndex = faceA;
				return true;
			}
		}

		if (edge.isSameAs(edgelist[newFace.edge2Index]))
		{
			if (edge.faceA == faceA)
			{
				faceIndex = faceA;
				anOtherfaceIndex = faceB;
				return true;
			}
			else if (edge.faceA == faceB)
			{
				faceIndex = faceB;
				anOtherfaceIndex = faceA;
				return true;
			}
			else if (edge.faceB == faceA)
			{
				faceIndex = faceA;
				anOtherfaceIndex = faceB;
				return true;
			}
			else if (edge.faceB == faceB)
			{
				faceIndex = faceB;
				anOtherfaceIndex = faceA;
				return true;
			}
		}
	}
	return false;
}

SPACE_ENUM CombedEdgeMachine::ensureEdgeSpaceAttribute(FEdge edge)
{
	SPACE_ENUM vert1Space, vert2Space;
	vert1Space = this->ensurePointSpaceAttribute(FPoint(nodelist[edge.indexA], edge.indexA));
	vert2Space = this->ensurePointSpaceAttribute(FPoint(nodelist[edge.indexB], edge.indexB));
	if (vert1Space == ON_MIXED && vert2Space != ON_MIXED)
	{
		return vert2Space;
	}
	else if (vert2Space == ON_MIXED && vert1Space != ON_MIXED)
	{
		return vert1Space;
	}
	else if (vert1Space != ON_MIXED && vert2Space != ON_MIXED)
	{
		if (vert1Space != vert2Space)
		{
			return ON_MIXED;
		}
		else
		{
			return vert1Space;
		}
	}
	else
	{
		return ON_MIXED;
	}
}

SPACE_ENUM CombedEdgeMachine::ensurePointSpaceAttribute(FPoint point)
{
	if (point.n == aimEdge.indexA || point.n == aimEdge.indexB)
	{
		return ON_MIXED;
	}

	for (auto vert : QL)
	{
		if (point == vert)
		{
			return ON_LEFT;
		}
	}

	for (auto vert : QR)
	{
		if (point == vert)
		{
			return ON_RIGHT;
		}
	}

	return ON_MIXED;
}

bool CombedEdgeMachine::exchangeDiagonal(FEdge aimDiagonal, vector<int> &arealist)
{
	int faceA, faceB;
	faceA = aimDiagonal.faceA;
	faceB = aimDiagonal.faceB;
	if (faceA == faceB)
	{
		return true;
	}
	if (faceB == -1 || faceA == -1)
	{
		return true;
	}

	FPoint newFaceVertA(nodelist[aimDiagonal.indexA], aimDiagonal.indexA);
	FPoint newFaceVertB(nodelist[aimDiagonal.indexB], aimDiagonal.indexB);

	FPoint newDiagonalVert1, newDiagonalVert2;
	newDiagonalVert1 = getOppositeVertToEdge(aimDiagonal, faceA);
	newDiagonalVert2 = getOppositeVertToEdge(aimDiagonal, faceB);

	FEdge newDiagonal;
	newDiagonal.InitEdge(newDiagonalVert1.n, newDiagonalVert2.n, faceA, faceB, nodelist);
	edgelist[aimDiagonalIndex] = newDiagonal;

	FEdge newEdgeA1, newEdgeA2, newEdgeB1, newEdgeB2;
	findAssociatedEdgesWith(newFaceVertA.n, aimDiagonal, faceA, faceB, newEdgeA1, newEdgeA2);
	findAssociatedEdgesWith(newFaceVertB.n, aimDiagonal, faceA, faceB, newEdgeB1, newEdgeB2);

	int a, b;
	a = getEdgeIndexInList(edgelist, newEdgeA1);
	b = getEdgeIndexInList(edgelist, newEdgeA2);
	FFace newFaceA(a, b, aimDiagonalIndex, newDiagonalVert1.n, newDiagonalVert2.n, newFaceVertA.n, edgelist, nodelist);
	a = getEdgeIndexInList(edgelist, newEdgeB1);
	b = getEdgeIndexInList(edgelist, newEdgeB2);
	FFace newFaceB(a, b, aimDiagonalIndex, newFaceVertB.n, newDiagonalVert1.n, newDiagonalVert2.n, edgelist, nodelist);

	int FaceAIndex, FaceBIndex;
	FaceAIndex = faceA;
	FaceBIndex = faceB;
	this->facelist[faceA] = newFaceA;
	this->facelist[faceB] = newFaceB;
	edgelist[this->facelist[faceA].edge1Index].adjustNeighborFaceIndex(newDiagonal, faceA, faceA, faceB);
	edgelist[this->facelist[faceA].edge2Index].adjustNeighborFaceIndex(newDiagonal, faceA, faceA, faceB);
	edgelist[this->facelist[faceB].edge1Index].adjustNeighborFaceIndex(newDiagonal, faceB, faceA, faceB);
	edgelist[this->facelist[faceB].edge2Index].adjustNeighborFaceIndex(newDiagonal, faceB, faceA, faceB);

	/*int FaceAIndex, FaceBIndex;
	if (ensureFaceIndex(newFaceA, FaceAIndex, FaceBIndex, faceA, faceB))
	{
		this->facelist[FaceAIndex] = newFaceA;
		this->facelist[FaceBIndex] = newFaceB;
	}
	else if (ensureFaceIndex(newFaceB, FaceBIndex, FaceAIndex, faceA, faceB))
	{
		this->facelist[FaceAIndex] = newFaceA;
		this->facelist[FaceBIndex] = newFaceB;
	}
	else
	{
		this->facelist[faceA] = newFaceA;
		this->facelist[faceB] = newFaceB;
	}*/

	if (newDiagonal.isSameAs(aimEdge))
	{
		return true;
	}
	else
	{
		//判断newDiagonal的区域范围
		SPACE_ENUM newDiagonalSpace = ensureEdgeSpaceAttribute(newDiagonal);
		if (newDiagonalSpace == ON_MIXED)
		{
			return false;
		}
		vector<int> ignoreInNextStep;
		SPACE_ENUM faceASpace = ensurePointSpaceAttribute(newFaceVertA);
		if (faceASpace == newDiagonalSpace)
		{
			ignoreInNextStep.push_back(FaceAIndex);
		}
		SPACE_ENUM faceBSpace = ensurePointSpaceAttribute(newFaceVertB);
		if (faceBSpace == newDiagonalSpace)
		{
			ignoreInNextStep.push_back(FaceBIndex);
		}

		vector<int> tempList;
		for (auto i : arealist)
		{
			bool pushIt = true;
			for (auto index : ignoreInNextStep)
			{
				if (index == i)
				{
					pushIt = false;
					break;
				}
			}
			if (pushIt)
			{
				tempList.push_back(i);
			}
		}
		arealist.clear();
		arealist = tempList;
		tempList.clear();

		if (arealist.size() == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

void CombedEdgeMachine::combedEdgeTurnOn()
{
	if (this->dynamicAreaList.size() <= 0)
	{
		return;
	}

	updateCombedRequirement();

	try
	{
		FEdge aimDiagonal = pickOutAimDiagonal();
		if (aimDiagonal.indexA == -1 && aimDiagonal.indexB == -1)
		{
			throw false;
			return;
		}

		bool canStop = exchangeDiagonal(aimDiagonal, dynamicAreaList);

		if (!canStop)
		{
			this->ifuck += 1;
			if (this->ifuck > 300)
			{
				int stop = 1;
			}
			combedEdgeTurnOn();
		}
		else
		{
			///////^^^
			//////////////////////////////
			//embedSteps.push_back(facelist);//
			//////////////////////////////
			return;
		}
	}
	catch (bool findAimDiagonal)
	{
		/*QMessageBox* mesg = new QMessageBox("Error", "Can not find aim diagonal !",
			                                 QMessageBox::Critical,
			                                 QMessageBox::Ok | QMessageBox::Default,
			                                 QMessageBox::Cancel | QMessageBox::Escape,
			                                 0);
		mesg->show();*/
		return;
	}
}

void CombedEdgeMachine::clearAll()
{
	nodelist.clear();
	edgelist.clear();
	facelist.clear();

	combedAreaIndex.clear();
	dynamicAreaList.clear();
	tempPointlist.clear();
	edgeCollect.clear();
	unBorderlist.clear();
	area.clear();
	QL.clear();
	QR.clear();

	//this->timer = 0;
}

bool CombedEdgeMachine::isSalientAngleInQuadrilateral(FEdge edge, int angleVertIndex)
{
	int faceAIndex, faceBIndex;
	faceAIndex = edge.faceA;
	faceBIndex = edge.faceB;

	FPoint testSeed1, testSeed2;
	testSeed1 = FPoint(nodelist[edge.indexA], edge.indexA);
	testSeed2 = FPoint(nodelist[edge.indexB], edge.indexB);
	FPoint diagonVert1, diagonVert2;
	diagonVert1 = getOppositeVertToEdge(edge, faceAIndex);
	diagonVert2 = getOppositeVertToEdge(edge, faceBIndex);

	Point3m diagonVector = diagonVert2.point - diagonVert1.point;
	Point3m testV1, testV2;
	testV1 = testSeed1.point - diagonVert1.point;
	testV2 = testSeed2.point - diagonVert1.point;

	testV1 = testV1 ^ diagonVector;
	testV2 = testV2 ^ diagonVector;
	if (testV1 * testV2 > 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}
void CombedEdgeMachine::combedEdgeByCheckForbiddenEdgeCP()
{
	if (iTime > 100)
	{
		int stop = 1;
		return;
	}
	qDebug() << "!!*************TIME : " << iTime << "*************!!" << endl;
	if (this->dynamicAreaList.size() <= 1)
	{
		return;
	}
	QString dynamicAreaStr("Dynamic Area : ");
	for (auto index : this->dynamicAreaList)
	{
		dynamicAreaStr += QString::number(index);
		dynamicAreaStr += " ";
	}
	qDebug() << dynamicAreaStr << endl;

	//ANALYSIS CURRENT STATE
	analysisCurrentState();

	//del it debug
	for (int i = 0; i < this->dynamicAreaList.size(); ++i)
	{
		if (canGetRidOfFromDynamicArea(dynamicAreaList[i]))
		{
			return;
		}
	}

	//PICK ONE OPERATE DIAGONAL
	int opeEdgeIndex = pickOneOpeDiagonal();
	qDebug() << "OpeEdgeIndex : " << opeEdgeIndex << endl;
	if (opeEdgeIndex < 0)
	{
		return;
	}
	//EXCHANGE DIAGONAL
	exchangeDiagonal(opeEdgeIndex);
	//CHECK AND UPDATE DYNAMIC_AREA_LIST
	bool combedComplete = checkAndUpdateDynamicArea();
	if (!combedComplete)
	{
		iTime += 1;
		combedEdgeByCheckForbiddenEdgeCP();
	}
	int a = 0;
	return;
}

void CombedEdgeMachine::analysisCurrentState()
{
	//收集动态区域对应的面集
	vector<FFace>().swap(area);
	for (auto index : this->dynamicAreaList)
	{
		this->area.push_back(facelist[index]);
	}
	//收集area包含的无重边索引集
	set<int> areaEdgeSet;
	for (int i = 0; i < this->area.size(); ++i)
	{
		areaEdgeSet.insert(area[i].edge1Index);
		areaEdgeSet.insert(area[i].edge2Index);
		areaEdgeSet.insert(area[i].edge3Index);
	}
	//刷新area边界边和交叉边索引集
	updateAreaBoundaryAndInsideEdgeIndexSet(areaEdgeSet);
	//刷新不可出现边对集
	updateForbiddenEdgeCPSet();
	//对insideEdgeIndexes进行aimEdge方向的排序
	sortInsideEdgeIndexes();
}

void CombedEdgeMachine::updateAreaBoundaryAndInsideEdgeIndexSet(set<int> edgeIndexes)
{
	set<int>().swap(boundaryIndexes);
	set<int>().swap(insideEdgeIndexes);
	vector<int>().swap(localEdgeIndexes);
	for (auto index : edgeIndexes)
	{
		if (0 <= index && index < this->edgelist.size())
		{
			if (isAreaBoundary(index))
			{
				boundaryIndexes.insert(index);
			}
			else
			{
				insideEdgeIndexes.insert(index);
			}
			localEdgeIndexes.push_back(index);
		}
	}
}

bool CombedEdgeMachine::isAreaBoundary(int edgeIndex)
{
	bool findA = false, findB = false;
	for (int i = 0; i < this->dynamicAreaList.size(); ++i)
	{
		if (this->edgelist[edgeIndex].faceA == dynamicAreaList[i])
		{
			findA = true;
			break;
		}
	}
	if (!findA)
	{
		return true;
	}

	for (int i = 0; i < this->dynamicAreaList.size(); ++i)
	{
		if (this->edgelist[edgeIndex].faceB == dynamicAreaList[i])
		{
			findB = true;
			break;
		}
	}
	if (!findB)
	{
		return true;
	}
	return false;
}

void CombedEdgeMachine::updateForbiddenEdgeCPSet()
{
	//对boundary的顶点进行整理FPoint集合
	int vertexIndex;
	set<int> vertexIndexes;
	for (auto edgeIndex : this->boundaryIndexes)
	{
		vertexIndex = this->edgelist[edgeIndex].indexA;
		vertexIndexes.insert(vertexIndex);
		vertexIndex = this->edgelist[edgeIndex].indexB;
		vertexIndexes.insert(vertexIndex);
	}
	vector<FPoint> vertexes;
	for (auto vertIndex : vertexIndexes)
	{
		vertexes.push_back(FPoint(this->nodelist[vertIndex], vertIndex, false));
	}

	//对vertexes进行逆时针排序，更新QL和QR
	vector<FPoint>().swap(QL);
	vector<FPoint>().swap(QR);
	Point3m tempAxisX, tempAxisY, tempOrigin;
	tempAxisY = (nodelist[aimEdge.indexB] - nodelist[aimEdge.indexA]).Normalize();
	tempAxisX = (tempAxisY ^ standNormalV).Normalize();
	tempOrigin = (nodelist[aimEdge.indexB] + nodelist[aimEdge.indexA]) / 2.0f;
	vector<pair<float, FPoint>> vertCircle;
	Point3m curVector, aimVector;
	aimVector = -tempAxisY;
	for (int i = 0; i < vertexes.size(); ++i)
	{
		curVector = (vertexes[i].point - tempOrigin).Normalize();
		float angle = acosf(aimVector * curVector);
		if (curVector * tempAxisX < 0)
		{
			angle = 2 * PI - angle;
			if (vertexes[i].n != aimEdge.indexA && vertexes[i].n != aimEdge.indexB)
			{
				QL.push_back(vertexes[i]);
			}
		}
		else
		{
			if (vertexes[i].n != aimEdge.indexA && vertexes[i].n != aimEdge.indexB)
			{
				QR.push_back(vertexes[i]);
			}
		}
		vertCircle.push_back(pair<float, FPoint>(angle, vertexes[i]));
	}
	pair<float, FPoint> temp;
	for (int i = vertCircle.size() - 1; 0 < i; i--)
	{
		for (int j = 0; j < i; j++)
		{
			if (vertCircle[j].first > vertCircle[j + 1].first)
			{
				temp = vertCircle[j];
				vertCircle[j] = vertCircle[j + 1];
				vertCircle[j + 1] = temp;
			}
		}
	}
	vector<FPoint>().swap(vertexes);
	for (int i = 0; i < vertCircle.size(); ++i)
	{
		vertexes.push_back(vertCircle[i].second);
	}
	vector<pair<float, FPoint>>().swap(vertCircle);

	//筛选ForbiddenEdgeCP
	vector<int>().swap(concaveVertIndexList);
	vector<pair<int, int>>().swap(forbiddenEdgeCPs);
	for (int i = 0; i < vertexes.size(); ++i)
	{
		if (vertexes[i].n == aimEdge.indexA || vertexes[i].n == aimEdge.indexB)
		{
			vertexes[i].bSalient = false;
			continue;
		}

		FPoint vertLast, vertNext;
		if (i == 0)
		{
			vertLast = *--vertexes.end();
		}
		else
		{
			vertLast = vertexes[i - 1];
		}
		if (i == vertexes.size() - 1)
		{
			vertNext = *vertexes.begin();
		}
		else
		{
			vertNext = vertexes[i + 1];
		}

		Point3m vectorCome, vectorGo;
		vectorCome = vertexes[i].point - vertLast.point;
		vectorGo = vertNext.point - vertexes[i].point;
		if ((vectorCome ^ vectorGo) * standNormalV < 0/* 0.000001f*/)//vectorCome与vectorGo平行或凹陷
		{
			vertexes[i].bSalient = false;
			concaveVertIndexList.push_back(vertexes[i].n);
			Point2i edgeCome(vertexes[i].n, vertLast.n);
			Point2i edgeGo(vertNext.n, vertexes[i].n);
			int indexEdgeCome = getEdgeIndexInList(this->boundaryIndexes, edgeCome);
			int indexEdgeGo = getEdgeIndexInList(this->boundaryIndexes, edgeGo);

			forbiddenEdgeCPs.push_back(pair<int, int>(indexEdgeCome, indexEdgeGo));
		}
		else
		{
			vertexes[i].bSalient = true;
		}
	}

	if (this->dynamicAreaList.size() == 2)
	{
		vector<pair<int, int>>().swap(forbiddenEdgeCPs);
		vector<int>().swap(concaveVertIndexList);
	}
}

void CombedEdgeMachine::sortInsideEdgeIndexes()
{
	if (this->insideEdgeIndexes.empty())
	{
		return;
	}
	Point3m tempAxisY, tempOrigin;
	tempAxisY = (nodelist[aimEdge.indexB] - nodelist[aimEdge.indexA]).Normalize();
	tempOrigin = nodelist[aimEdge.indexA];

	vector<pair<float, int>> distanceIndexRecords;
	for (auto index : insideEdgeIndexes)
	{
		Point3m node = (nodelist[edgelist[index].indexA] + nodelist[edgelist[index].indexB]) / 2.0f;
		float distance = (node - tempOrigin) * tempAxisY;
		distanceIndexRecords.push_back(pair<float, int>(distance, index));
	}
	//按照距离从小到大进行排序
	pair<float, int> temp;
	for (int i = distanceIndexRecords.size() - 1; 0 < i; i--)
	{
		for (int j = 0; j < i; j++)
		{
			if (distanceIndexRecords[j].first > distanceIndexRecords[j + 1].first)
			{
				temp = distanceIndexRecords[j];
				distanceIndexRecords[j] = distanceIndexRecords[j + 1];
				distanceIndexRecords[j + 1] = temp;
			}
		}
	}
	vector<int>().swap(sortedInsideEdgeIndexes);
	for (int i = 0; i < distanceIndexRecords.size(); ++i)
	{
		sortedInsideEdgeIndexes.push_back(distanceIndexRecords[i].second);
	}
	vector<pair<float, int>>().swap(distanceIndexRecords);
}

int CombedEdgeMachine::pickOneOpeDiagonal()
{
	//从insideEdgeIndex中依次挑选一条满足条件的边作为OperateDiagonal
	for (auto index : sortedInsideEdgeIndexes)
	{
		pair<int, int> edgeIndexCP_A = collectEdgeCPLinkedWith(edgelist[index].indexA, index);
		pair<int, int> edgeIndexCP_B = collectEdgeCPLinkedWith(edgelist[index].indexB, index);

		bool edgeAForbidden = false, edgeBForbidden = false;
		edgeAForbidden = ifExistInForbiddenEdgeCPSet(edgeIndexCP_A);
		edgeBForbidden = ifExistInForbiddenEdgeCPSet(edgeIndexCP_B);

		if (!edgeAForbidden && !edgeBForbidden)
		{
			return index;
		}
	}
	return -1;
}

pair<int, int> CombedEdgeMachine::collectEdgeCPLinkedWith(int iVert, int iEdge)
{
	int iFaceA = edgelist[iEdge].faceA;
	int iFaceB = edgelist[iEdge].faceB;
	vector<int> possibleEdgeIndexes;
	if (facelist[iFaceA].edge1Index != iEdge)
	{
		possibleEdgeIndexes.push_back(facelist[iFaceA].edge1Index);
	}
	if (facelist[iFaceA].edge2Index != iEdge)
	{
		possibleEdgeIndexes.push_back(facelist[iFaceA].edge2Index);
	}
	if (facelist[iFaceA].edge3Index != iEdge)
	{
		possibleEdgeIndexes.push_back(facelist[iFaceA].edge3Index);
	}
	if (facelist[iFaceB].edge1Index != iEdge)
	{
		possibleEdgeIndexes.push_back(facelist[iFaceB].edge1Index);
	}
	if (facelist[iFaceB].edge2Index != iEdge)
	{
		possibleEdgeIndexes.push_back(facelist[iFaceB].edge2Index);
	}
	if (facelist[iFaceB].edge3Index != iEdge)
	{
		possibleEdgeIndexes.push_back(facelist[iFaceB].edge3Index);
	}

	int iA = -1, iB = -1;
	for (auto index : possibleEdgeIndexes)
	{
		if (iA == -1)
		{
			if (edgelist[index].indexA == iVert)
			{
				iA = index;
			}
			else if (edgelist[index].indexB == iVert)
			{
				iA = index;
			}
		}
		else
		{
			if (edgelist[index].indexA == iVert)
			{
				iB = index;
				break;
			}
			else if (edgelist[index].indexB == iVert)
			{
				iB = index;
				break;
			}
		}
	}

	return pair<int, int>(iA, iB);
}

bool CombedEdgeMachine::ifExistInForbiddenEdgeCPSet(const pair<int, int> &cp)
{
	for (auto forbiddenCP : this->forbiddenEdgeCPs)
	{
		if (forbiddenCP.first == cp.first && forbiddenCP.second == cp.second)
		{
			return true;
		}
		else if (forbiddenCP.first == cp.second && forbiddenCP.second == cp.first)
		{
			return true;
		}
	}
	return false;
}

void CombedEdgeMachine::exchangeDiagonal(int iEdge)
{
	assert(iEdge >= 0);

	int iFaceA = edgelist[iEdge].faceA;
	int iFaceB = edgelist[iEdge].faceB;
	int iOldVertA = edgelist[iEdge].indexA;
	int iOldVertB = edgelist[iEdge].indexB;
	int iNewVertA = getDiffVertIndexIn(iFaceA, iOldVertA, iOldVertB);
	int iNewVertB = getDiffVertIndexIn(iFaceB, iOldVertA, iOldVertB);
	//跟换当前Edge的顶点索引
	edgelist[iEdge].indexA = iNewVertA;
	edgelist[iEdge].indexB = iNewVertB;

	FFace newFaceA(iNewVertA, iNewVertB, iOldVertA);
	FFace newFaceB(iNewVertB, iNewVertA, iOldVertB);
	getEachEdgeIndexInFace(newFaceA, iFaceA, pair<int, int>(iFaceA, iFaceB), iEdge);
	getEachEdgeIndexInFace(newFaceB, iFaceB, pair<int, int>(iFaceA, iFaceB), iEdge);
	//跟换当前所影响面片
	facelist[iFaceA] = newFaceA;
	facelist[iFaceB] = newFaceB;
}

int CombedEdgeMachine::getDiffVertIndexIn(int iFace, int iVertA, int iVertB)
{
	if (facelist[iFace].n1 != iVertA && facelist[iFace].n1 != iVertB)
	{
		return facelist[iFace].n1;
	}
	if (facelist[iFace].n2 != iVertA && facelist[iFace].n2 != iVertB)
	{
		return facelist[iFace].n2;
	}
	if (facelist[iFace].n3 != iVertA && facelist[iFace].n3 != iVertB)
	{
		return facelist[iFace].n3;
	}
	assert(0);
}

void CombedEdgeMachine::getEachEdgeIndexInFace(FFace &face, int newFaceIndex, pair<int, int> range, int iAvoidEdge)
{
	bool find1 = false, find2 = false, find3 = false;
	for (auto index : localEdgeIndexes)
	{
		if (edgelist[index] == face.edge1)
		{
			find1 = true;
			face.edge1Index = index;
			correctionEdgesAdjacentFaceIndex(index, newFaceIndex, range, iAvoidEdge);
			break;
		}
	}
	for (auto index : localEdgeIndexes)
	{
		if (edgelist[index] == face.edge2)
		{
			find2 = true;
			face.edge2Index = index;
			correctionEdgesAdjacentFaceIndex(index, newFaceIndex, range, iAvoidEdge);
			break;
		}
	}
	for (auto index : localEdgeIndexes)
	{
		if (edgelist[index] == face.edge3)
		{
			find3 = true;
			face.edge3Index = index;
			correctionEdgesAdjacentFaceIndex(index, newFaceIndex, range, iAvoidEdge);
			break;
		}
	}

	assert(find1 && find2 && find3);
}

void CombedEdgeMachine::correctionEdgesAdjacentFaceIndex(int iEdge, int newFaceIndex, pair<int, int> range, int iAvoidEdge)
{
	if (iEdge == iAvoidEdge)
	{
		return;
	}

	if (this->edgelist[iEdge].faceA == range.first || this->edgelist[iEdge].faceA == range.second)
	{
		this->edgelist[iEdge].faceA = newFaceIndex;
		return;
	}

	if (this->edgelist[iEdge].faceB == range.first || this->edgelist[iEdge].faceB == range.second)
	{
		this->edgelist[iEdge].faceB = newFaceIndex;
		return;
	}
}

bool CombedEdgeMachine::checkAndUpdateDynamicArea()
{
	//检查一次梳理调整combed之后，aimEdge是否出现在当前调整后的localEdgeIndexes指向的边里
	for (auto index : this->localEdgeIndexes)
	{
		if (edgelist[index] == Point2i(aimEdge.indexA, aimEdge.indexB))
		{
			return true;
		}
	}

	//没有找到aimEdge，对dynamicAreaIndex进行过滤，剔除没有与aimEdge交叉的三角面的索引
	vector<int>::iterator ite = this->dynamicAreaList.begin();
	while (ite != this->dynamicAreaList.end())
	{
		if (canGetRidOfFromDynamicArea(*ite))
		{
			this->dynamicAreaList.erase(ite);
		}
		else
		{
			++ite;
		}
	}
	return false;
}

bool CombedEdgeMachine::canGetRidOfFromDynamicArea(int iFace)
{
	vector<int> curVertIndexList;
	if (facelist[iFace].n1 != aimEdge.indexA && facelist[iFace].n1 != aimEdge.indexB)
	{
		curVertIndexList.push_back(facelist[iFace].n1);
	}
	if (facelist[iFace].n2 != aimEdge.indexA && facelist[iFace].n2 != aimEdge.indexB)
	{
		curVertIndexList.push_back(facelist[iFace].n2);
	}
	if (facelist[iFace].n3 != aimEdge.indexA && facelist[iFace].n3 != aimEdge.indexB)
	{
		curVertIndexList.push_back(facelist[iFace].n3);
	}

	vector<bool> bOnLeftSide;
	for (auto index : curVertIndexList)
	{
		bool onLeft = false;
		for (int i = 0; i < this->QL.size(); ++i)
		{
			if (QL[i].n == index)
			{
				onLeft = true;
				break;
			}
		}
		bOnLeftSide.push_back(onLeft);
	}

	assert(!bOnLeftSide.empty());

	bool canGetRidOf = true;
	for (int i = 0; i < bOnLeftSide.size() - 1; ++i)
	{
		if (bOnLeftSide[i] ^ bOnLeftSide[i + 1])
		{
			canGetRidOf = false;
			break;
		}
	}
	vector<int>().swap(curVertIndexList);
	vector<bool>().swap(bOnLeftSide);
	return canGetRidOf;
}

bool CombedEdgeMachine::getConcaveEdge(FEdge &_sub_border)
{
	analysisCurrentState();
	if (!concaveVertIndexList.empty())
	{
		int iS = aimEdge.indexA;
		int iE = aimEdge.indexB;
		for (auto index : concaveVertIndexList)
		{
			bool existedSameEdge = false;
			for (auto edge : edgelist)
			{
				if (edge == Point2i(iS, index))
				{
					existedSameEdge = true;
					break;
				}
			}
			if (!existedSameEdge)
			{
				_sub_border = FEdge(nodelist[iS], nodelist[index], iS, index);
				return true;
			}

			existedSameEdge = false;
			for (auto edge : edgelist)
			{
				if (edge == Point2i(iE, index))
				{
					existedSameEdge = true;
					break;
				}
			}
			if (!existedSameEdge)
			{
				_sub_border = FEdge(nodelist[iE], nodelist[index], iE, index);
				return true;
			}
		}
	}
	return false;
}
