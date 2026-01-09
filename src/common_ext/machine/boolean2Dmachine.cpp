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

#include "boolean2Dmachine.h"

Boolean2DMachine::Boolean2DMachine()
{
}

Boolean2DMachine::Boolean2DMachine(CMeshO *_meshA, CMeshO *_meshB, vector<Point2m> cplist)
{
	this->sourceFaceIndexs_A.clear();
	this->sourceFaceIndexs_B.clear();

	this->meshA = _meshA;
	this->meshB = _meshB;
	for (auto cp : cplist)
	{
		this->sourceFaceIndexs_A.push_back(cp.X());
		this->sourceFaceIndexs_B.push_back(cp.Y());
	}
}

Boolean2DMachine::~Boolean2DMachine()
{
}

void Boolean2DMachine::initial(CMeshO *_meshA, CMeshO *_meshB, vector<Point2m> cplist)
{
	this->sourceFaceIndexs_A.clear();
	this->sourceFaceIndexs_B.clear();
	this->mustExitEdges.clear();

	this->meshA = _meshA;
	this->meshB = _meshB;
	bool exitedA, exitedB;
	for (auto cp : cplist)
	{
		exitedA = false;
		for (auto index : this->sourceFaceIndexs_A)
		{
			if (index == cp.X())
			{
				exitedA = true;
				break;
			}
		}
		if (!exitedA)
		{
			this->sourceFaceIndexs_A.push_back(cp.X());
		}

		exitedB = false;
		for (auto index : this->sourceFaceIndexs_B)
		{
			if (index == cp.Y())
			{
				exitedB = true;
				break;
			}
		}
		if (!exitedB)
		{
			this->sourceFaceIndexs_B.push_back(cp.Y());
		}
	}
}

void Boolean2DMachine::initial(CMeshO *_meshA, CMeshO *_meshB, vector<int> AFaceIndexs, vector<int> BFaceIndexs)
{
	this->sourceFaceIndexs_A.clear();
	this->sourceFaceIndexs_B.clear();
	this->mustExitEdges.clear();

	this->meshA = _meshA;
	this->meshB = _meshB;
	this->sourceFaceIndexs_A = AFaceIndexs;
	this->sourceFaceIndexs_B = BFaceIndexs;
}

bool Boolean2DMachine::boolean2DCompute(int iOperateKind, vector<FFace> &result)
{
    result.clear();
	//获得操作二维平面信息，建立在其上的坐标系
	this->construct2DCoordinateSystem();

	//分别提取出A和B面集的轮廓组
	this->contourA = extractOrderedContourLoopsFrom(this->meshA, this->sourceFaceIndexs_A);
	this->contourB = extractOrderedContourLoopsFrom(this->meshB, this->sourceFaceIndexs_B);

	////获得属于A和B的轮廓曲线之后，求A组和B组的交点拆分曲线
	this->splitAndSignContourLoop(contourA, contourB);

	////依据内外包含属性拆分了曲线之后，根据二维布尔运算类型进行最终轮廓确定。
	this->getFinalWantedContourCircles(iOperateKind);

	////根据外轮廓进行剖分和剔除得到最终的面结果
	this->generateAimFaces(result);

	return true;
}

void Boolean2DMachine::construct2DCoordinateSystem()
{
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*this->meshA);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*this->meshB);
	this->coordinateAxisZ = meshA->face[sourceFaceIndexs_A[0]].N();
	this->coordinateCenterPos = meshA->face[sourceFaceIndexs_A[0]].V(0)->P();
	this->coordinateAxisX = (meshA->face[sourceFaceIndexs_A[0]].V(1)->P() - coordinateCenterPos).Normalize();
	this->coordinateAxisY = (coordinateAxisX ^ coordinateAxisZ).Normalize();
}

vector<WordCircle> Boolean2DMachine::extractOrderedContourLoopsFrom(CMeshO *mesh, vector<int> faceIndexs)
{
	int delVertNum = vcg::tri::Clean<CMeshO>::RemoveDegenerateVertex(*mesh);
	int delFaceNum = vcg::tri::Clean<CMeshO>::RemoveDegenerateFace(*mesh);
	tri::Clean<CMeshO>::RemoveDuplicateVertex(*mesh);// delete Redundant vertex data
	tri::Clean<CMeshO>::RemoveUnreferencedVertex(*mesh);

	//将边界边从面片集中提取出来
	bool exited;
	vector<FEdge> edges;
	vector<FEdge> deleteEdges;
	for (auto index : faceIndexs)
	{
		FEdge tempEdge1(mesh->face[index].V(0)->P(), mesh->face[index].V(1)->P(), mesh->face[index].V(0) - &mesh->vert[0], mesh->face[index].V(1) - &mesh->vert[0]);
		FEdge tempEdge2(mesh->face[index].V(1)->P(), mesh->face[index].V(2)->P(), mesh->face[index].V(1) - &mesh->vert[0], mesh->face[index].V(2) - &mesh->vert[0]);
		FEdge tempEdge3(mesh->face[index].V(2)->P(), mesh->face[index].V(0)->P(), mesh->face[index].V(2) - &mesh->vert[0], mesh->face[index].V(0) - &mesh->vert[0]);

		exited = false;
		for (auto edge : edges)
		{
			if (tempEdge1 == edge)
			{
				exited = true;
				deleteEdges.push_back(tempEdge1);
				break;
			}
		}
		if (!exited)
		{
			edges.push_back(tempEdge1);
		}

		exited = false;
		for (auto edge : edges)
		{
			if (tempEdge2 == edge)
			{
				exited = true;
				deleteEdges.push_back(tempEdge2);
				break;
			}
		}
		if (!exited)
		{
			edges.push_back(tempEdge2);
		}

		exited = false;
		for (auto edge : edges)
		{
			if (tempEdge3 == edge)
			{
				exited = true;
				deleteEdges.push_back(tempEdge3);
				break;
			}
		}
		if (!exited)
		{
			edges.push_back(tempEdge3);
		}
	}

	vector<FEdge> tempList;
	bool needDeleteIt;
	for (auto edge : edges)
	{
		needDeleteIt = false;
		for (auto deleteEdge : deleteEdges)
		{
			if (edge == deleteEdge)
			{
				needDeleteIt = true;
				break;
			}
		}

		if (!needDeleteIt)
		{
			tempList.push_back(edge);
		}
	}
	deleteEdges.clear();
	edges.clear();
	edges = tempList;
	tempList.clear();

	//将离散的边集edges组合成为有序的曲线
	vector<WordCircle> result = getContourCircles(edges);
	//识别内外轮廓
	this->recognizeOutsideOrInside(result);
	//根据内外调节循环方向
	this->adjustLoopDirect(result);

	return result;
}

vector<WordCircle> Boolean2DMachine::getContourCircles(vector<FEdge> edgelist)
{
	vector<WordEdge> edges;
	for (auto temp : edgelist)
	{
		edges.push_back(WordEdge(temp.vertA, temp.vertB, temp.indexA, temp.indexB));
	}

	vector<WordCircle> contourCircles;
	contourCircles.clear();
	for (auto edge : edges)
	{
		if (contourCircles.size() == 0)
		{
			WordCircle curLoop;
			curLoop.edgeList.push_back(edge);
			curLoop.headIndex = edge.indexA;
			curLoop.endIndex = edge.indexB;
			contourCircles.push_back(curLoop);
		}
		else
		{
			bool notFindLinker = true;
			for (int i = 0; i < contourCircles.size(); i++)
			{
				if (contourCircles[i].headIndex != contourCircles[i].endIndex)
				{
					if (contourCircles[i].endIndex == edge.indexA)
					{
						notFindLinker = false;
						contourCircles[i].edgeList.push_back(edge);
						contourCircles[i].endIndex = edge.indexB;
						break;
					}
					else if (contourCircles[i].endIndex == edge.indexB)
					{
						notFindLinker = false;
						edge.exchange();
						contourCircles[i].edgeList.push_back(edge);
						contourCircles[i].endIndex = edge.indexB;
						break;
					}
					else if (contourCircles[i].headIndex == edge.indexB)
					{
						notFindLinker = false;
						vector<WordEdge> tempList;
						tempList = contourCircles[i].edgeList;
						contourCircles[i].edgeList.clear();
						contourCircles[i].edgeList.push_back(edge);
						for (auto sonEdge : tempList)
						{
							contourCircles[i].edgeList.push_back(sonEdge);
						}
						tempList.clear();
						contourCircles[i].headIndex = edge.indexA;
						break;
					}
					else if (contourCircles[i].headIndex == edge.indexA)
					{
						notFindLinker = false;
						edge.exchange();
						vector<WordEdge> tempList;
						tempList = contourCircles[i].edgeList;
						contourCircles[i].edgeList.clear();
						contourCircles[i].edgeList.push_back(edge);
						for (auto sonEdge : tempList)
						{
							contourCircles[i].edgeList.push_back(sonEdge);
						}
						tempList.clear();
						contourCircles[i].headIndex = edge.indexA;
						break;
					}
				}
			}

			if (notFindLinker)
			{
				WordCircle curLoop;
				curLoop.edgeList.push_back(edge);
				curLoop.headIndex = edge.indexA;
				curLoop.endIndex = edge.indexB;
				contourCircles.push_back(curLoop);
			}
		}
	}

	int ITime = 0;
	bool continueLink = false;
	do {
		continueLink = false;
		for (int i = 0; i < contourCircles.size(); i++)
		{
			if (contourCircles[i].headIndex != contourCircles[i].endIndex)
			{
				if (ITime < 100)
				{
					continueLink = true;
					for (int j = 0; j < contourCircles.size(); j++)
					{
						if (i != j)
						{
							if (contourCircles[j].headIndex == contourCircles[i].endIndex)
							{
								contourCircles[i].pushCircleBack(contourCircles[j]);
							}
							else if (contourCircles[j].endIndex == contourCircles[i].endIndex)
							{
								contourCircles[j].putUpsideDown();
								contourCircles[i].pushCircleBack(contourCircles[j]);
							}
							else if (contourCircles[j].endIndex == contourCircles[i].headIndex)
							{
								contourCircles[i].pushCircleFront(contourCircles[j]);
							}
							else if (contourCircles[j].headIndex == contourCircles[i].headIndex)
							{
								contourCircles[j].putUpsideDown();
								contourCircles[i].pushCircleFront(contourCircles[j]);
							}
						}
					}
				}
				else
				{
					Point3f A, B;
					A = contourCircles[i].edgeList[contourCircles[i].edgeList.size() - 1].vertB;
					B = contourCircles[i].edgeList[0].vertA;
					contourCircles[i].edgeList.push_back(WordEdge(A, B, contourCircles[i].endIndex, contourCircles[i].headIndex));
					contourCircles[i].endIndex = contourCircles[i].headIndex;
					edges.push_back(WordEdge(A, B, contourCircles[i].endIndex, contourCircles[i].headIndex));
				}
			}
		}

		ITime++;
	} while (continueLink);

	vector<WordCircle> tempCircles;
	for (auto c : contourCircles)
	{
		if (c.headIndex != -1)
		{
			tempCircles.push_back(c);
		}
	}
	contourCircles.clear();
	contourCircles = tempCircles;
	tempCircles.clear();

	return contourCircles;
}

void Boolean2DMachine::recognizeOutsideOrInside(vector<WordCircle> &contourCircles)
{
	for (int i = 0; i < contourCircles.size(); i++)
	{
		contourCircles[i].initalBoundingBox();
	}

	for (int i = 0; i < contourCircles.size(); i++)
	{
		Point3f A, B;
		A = pickOneVertOutofAllBox(contourCircles[i], i, contourCircles);
		B = A + Point3f(1, 0, 0);

		int IntersectNum;
		IntersectNum = getIntersectNumWithOtherCircle(i, A, B, contourCircles);

		if (IntersectNum % 2 == 0)
		{
			contourCircles[i].bOutsideCircle = true;
		}
		else
		{
			contourCircles[i].bOutsideCircle = false;
		}
	}
}

Point3f Boolean2DMachine::pickOneVertOutofAllBox(WordCircle circle, int I, vector<WordCircle> contourCircles)
{
	for (auto edge : circle.edgeList)
	{
		bool outOfAllBox = true;

		for (int i = 0; i < contourCircles.size(); i++)
		{
			if (i != I)
			{
				if (contourCircles[i].thisVertInsideBoundBox(edge.vertA))
				{
					outOfAllBox = false;
					break;
				}
			}
		}

		if (outOfAllBox)
		{
			return edge.vertA;
		}
	}

	return circle.edgeList[0].vertA;
}

int Boolean2DMachine::getIntersectNumWithOtherCircle(int I, Point3f A, Point3f B, vector<WordCircle> contourCircles)
{
	int result = 0;

	for (int i = 0; i < contourCircles.size(); i++)
	{
		if (i != I)
		{
			result += computSumIntersectNumWithCircle(A, B, contourCircles[i]);
		}
	}

	return result;
}

int Boolean2DMachine::computSumIntersectNumWithCircle(Point3f A, Point3f B, WordCircle circle)
{
	int result = 0;

	vector<Point3m> mixCollect;
	for (auto edge : circle.boundingBoxEdgeList)
	{
		if ((edge.vertA.Y() - A.Y()) * (edge.vertB.Y() - A.Y()) < 0)
		{
			float a, b;
			if ((edge.vertB.X() - edge.vertA.X()) != 0)
			{
				a = (edge.vertB.Y() - edge.vertA.Y()) / (edge.vertB.X() - edge.vertA.X());
				b = edge.vertA.Y() - a * edge.vertA.X();

				float intersectX = (A.Y() - b) / a;
				if (intersectX >= A.X())
				{
					Point3m tempMix(intersectX, A.Y(), 0);
					bool isNewMix = true;
					for (auto vert : mixCollect)
					{
						if (tempMix == vert)
						{
							isNewMix = false;
							break;
						}
					}

					if (isNewMix)
					{
						mixCollect.push_back(tempMix);
						result++;
					}
				}
			}
			else
			{
				if (edge.vertA.X() > A.X())
				{
					Point3m tempMix(edge.vertA.X(), A.Y(), 0);
					bool isNewMix = true;
					for (auto vert : mixCollect)
					{
						if (tempMix == vert)
						{
							isNewMix = false;
							break;
						}
					}

					if (isNewMix)
					{
						mixCollect.push_back(tempMix);
						result++;
					}
				}
			}
		}
		else if ((edge.vertA.Y() - A.Y()) * (edge.vertB.Y() - A.Y()) == 0)
		{
			if (edge.vertA.Y() - edge.vertB.Y() != 0)
			{
				if (edge.vertA.Y() - A.Y() == 0)
				{
					if (edge.vertA.X() > A.X())
					{
						Point3m tempMix(edge.vertA.X(), edge.vertA.Y(), 0);
						bool isNewMix = true;
						for (auto vert : mixCollect)
						{
							if (tempMix == vert)
							{
								isNewMix = false;
								break;
							}
						}

						if (isNewMix)
						{
							mixCollect.push_back(tempMix);
							result++;
						}
					}
				}
				else if (edge.vertB.Y() - A.Y() == 0)
				{
					if (edge.vertB.X() > A.X())
					{
						Point3m tempMix(edge.vertB.X(), edge.vertB.Y(), 0);
						bool isNewMix = true;
						for (auto vert : mixCollect)
						{
							if (tempMix == vert)
							{
								isNewMix = false;
								break;
							}
						}

						if (isNewMix)
						{
							mixCollect.push_back(tempMix);
							result++;
						}
					}
				}
			}
		}
	}

	mixCollect.clear();
	return result;
}

void Boolean2DMachine::adjustLoopDirect(vector<WordCircle> &contourCircles)
{
	for (int i = 0; i < contourCircles.size(); i++)
	{
		contourCircles[i].recognizeClockWise(this->coordinateAxisZ);

		if (contourCircles[i].bOutsideCircle && contourCircles[i].bIsClockWise)
		{
			contourCircles[i].putUpsideDown();
			contourCircles[i].bIsClockWise = false;
		}
		else if (!contourCircles[i].bOutsideCircle && !contourCircles[i].bIsClockWise)
		{
			contourCircles[i].putUpsideDown();
			contourCircles[i].bIsClockWise = true;
		}
	}
}

void Boolean2DMachine::splitAndSignContourLoop(vector<WordCircle> &A, vector<WordCircle> &B)
{
	//求得交点，并判断该交点的出入属性。补充到各自的WordCircle对象中
	for (int i = 0; i < A.size(); i++)
	{
		for (int ii = 0; ii < A[i].edgeList.size(); ++ii)
		{
			computeAllIntersectionWith(A[i].edgeList[ii], B);
		}
	}

	//求得各边上的交叉点并判断了该点在相应曲线上的出入属性，开始重构各个轮廓曲线
	for (int i = 0; i < A.size(); i++)
	{
		A[i].reconstructItself();
	}
	for (int i = 0; i < B.size(); i++)
	{
		B[i].reconstructItself();
	}

	//依据重构的轮廓，其上包含对各点是否为出点或入点或普通点的属性标记。据此标记进行分类
	this->getOutAndInsideEdgeFromCircles(A, AoutsideB, AinsideB);
	this->getOutAndInsideEdgeFromCircles(B, BoutsideA, BinsideA);
}

void Boolean2DMachine::computeAllIntersectionWith(WordEdge & edge, vector<WordCircle> &AimCircle)
{
	Point3m directionA, directionB;
	edge.getVector();
	directionA = edge.vector;
	for (int i = 0; i < AimCircle.size(); i++)
	{
		for (int j = 0; j < AimCircle[i].edgeList.size(); ++j)
		{
			Point3m intersection;
			if (edge.intersectWith(AimCircle[i].edgeList[j], this->coordinateAxisZ, intersection))
			{
				AimCircle[i].edgeList[j].getVector();
				directionB = AimCircle[i].edgeList[j].vector;

				//如若两边存在交点，则判断此交点对于相应轮廓线的出点入点属性
				if ((directionA ^ directionB) * this->coordinateAxisZ < 0)
				{
					edge.splitVerts.push_back(RichIntersection(intersection, true));
					AimCircle[i].edgeList[j].splitVerts.push_back(RichIntersection(intersection, false));
				}
				else
				{
					edge.splitVerts.push_back(RichIntersection(intersection, false));
					AimCircle[i].edgeList[j].splitVerts.push_back(RichIntersection(intersection, true));
				}
			}
		}
	}

	return;
}

void Boolean2DMachine::getOutAndInsideEdgeFromCircles(vector<WordCircle> circles, vector<WordCircle> &outsideCircles, vector<WordCircle> &insideCircles)
{
	outsideCircles.clear();
	insideCircles.clear();
	for (int i = 0; i < circles.size(); ++i)
	{
		this->getOutAndInsideEdgeFromOneCircle(circles[i], outsideCircles, insideCircles);
	}
	return;
}

void Boolean2DMachine::getOutAndInsideEdgeFromOneCircle(WordCircle circle, vector<WordCircle> &outsideCircles, vector<WordCircle> &insideCircles)
{
	//寻找第一个edge的起点是入点的边对应的索引，以此索引为开始和结束循环的依据
	int judgeIndex = -1;
	for (int i = 0; i < circle.edgeList.size(); ++i)
	{
		if (circle.edgeList[i].attributeA == WordEdge::ENTERPOINT)
		{
			judgeIndex = i;
			break;
		}
	}

	if (judgeIndex < 0)
	{
		outsideCircles.push_back(circle);
		return;
	}

	int cursorI = judgeIndex;
	bool PutInInsideBox = true;
	vector<WordEdge> tempEdges;
	do {
		tempEdges.push_back(circle.edgeList[cursorI]);

		if (PutInInsideBox)
		{
			if (circle.edgeList[cursorI].attributeB == WordEdge::OUTPOINT)
			{
				WordCircle newInsideEdges(tempEdges);
				insideCircles.push_back(newInsideEdges);
				tempEdges.clear();
				PutInInsideBox = false;
			}
		}
		else
		{
			if (circle.edgeList[cursorI].attributeB == WordEdge::ENTERPOINT)
			{
				WordCircle newOutsideEdges(tempEdges);
				outsideCircles.push_back(newOutsideEdges);
				tempEdges.clear();
				PutInInsideBox = true;
			}
		}

		cursorI += 1;
		if (cursorI >= circle.edgeList.size())
		{
			cursorI = 0;
		}
	} while (cursorI != judgeIndex);

	tempEdges.clear();
}

void Boolean2DMachine::getFinalWantedContourCircles(int iKind)
{
	this->FinalContourCircles.clear();

	switch (iKind)
	{
	case A_UNION_B:
		this->FinalContourCircles = autoCombineAndLinkCircles(this->AoutsideB, this->BoutsideA);
		break;

	case A_INTERSECT_B:
		this->FinalContourCircles = autoCombineAndLinkCircles(this->AinsideB, this->BinsideA);
		break;

	case A_MINUS_B:
		for (int i = 0; i < this->BinsideA.size(); ++i)
		{
			this->BinsideA[i].putUpsideDown();
		}
		this->FinalContourCircles = autoCombineAndLinkCircles(this->AoutsideB, this->BinsideA);
		break;

	case B_MINUS_A:
		for (int i = 0; i < this->AinsideB.size(); ++i)
		{
			this->AinsideB[i].putUpsideDown();
		}
		this->FinalContourCircles = autoCombineAndLinkCircles(this->BoutsideA, this->AinsideB);
		break;
	default:
		break;
	}

	this->AinsideB.clear();
	this->AoutsideB.clear();
	this->BinsideA.clear();
	this->BoutsideA.clear();
}

vector<WordCircle> Boolean2DMachine::autoCombineAndLinkCircles(vector<WordCircle> A, vector<WordCircle> B)
{
	vector<WordCircle> result;

	result = A;
	for (int i = 0; i < B.size(); ++i)
	{
		result.push_back(B[i]);
	}

	int ITime = 0;
	bool continueLink = false;
	do {
		continueLink = false;
		for (int i = 0; i < result.size(); i++)
		{
			Point3m headVert, tailVert;
			headVert = result[i].edgeList.begin()->vertA;
			tailVert = result[i].edgeList.end()->vertB;
			if (headVert != tailVert)
			{
				if (ITime < 100)
				{
					continueLink = true;
					for (int j = 0; j < result.size(); j++)
					{
						if (i != j)
						{
							Point3m JheadVert, IheadVert, JtailVert, ItailVert;
							JheadVert = result[j].edgeList.begin()->vertA;
							IheadVert = result[i].edgeList.begin()->vertA;
							JtailVert = result[j].edgeList.end()->vertB;
							ItailVert = result[i].edgeList.end()->vertB;

							if (JheadVert == ItailVert)
							{
								result[i].pushCircleBack(result[j]);
							}
							else if (JtailVert == ItailVert)
							{
								result[j].putUpsideDown();
								result[i].pushCircleBack(result[j]);
							}
							else if (JtailVert == IheadVert)
							{
								result[i].pushCircleFront(result[j]);
							}
							else if (JheadVert == IheadVert)
							{
								result[j].putUpsideDown();
								result[i].pushCircleFront(result[j]);
							}
						}
					}
				}
				/*else
				{
					Point3f A, B;
					A = result[i].edgeList[result[i].edgeList.size() - 1].vertB;
					B = result[i].edgeList[0].vertA;
					result[i].edgeList.push_back(WordEdge(A, B, result[i].endIndex, result[i].headIndex));
					result[i].endIndex = result[i].headIndex;
				}*/
			}
		}

		ITime++;
	} while (continueLink);

	return result;
}

bool  Boolean2DMachine::generateAimFaces(vector<FFace>& result)
{
    result.clear();
	DelaunayMachine facesGenerator;

	//获得当前平面包围内多边形轮廓的SuperTriangle
	vector<Point3f> contourVerts, contourVerts2D;
	for (auto circle : this->FinalContourCircles)
	{
		for (auto edge : circle.edgeList)
		{
			contourVerts.push_back(edge.vertA);
		}
	}
	for (auto vert : contourVerts)
	{
		Point3m edge = vert - this->coordinateCenterPos;
		float x = edge * this->coordinateAxisX;
		float y = edge * this->coordinateAxisY;
		contourVerts2D.push_back(Point3f(x, y, 0));
	}
	FFace superTriangle, superTriangle2D;
	superTriangle2D = this->getSuperTriangle(contourVerts2D);
	Point3m v1_3D, v2_3D, v3_3D;
	v1_3D = this->coordinateCenterPos + coordinateAxisX * superTriangle2D.v1.X() + coordinateAxisY * superTriangle2D.v1.Y();
	v2_3D = this->coordinateCenterPos + coordinateAxisX * superTriangle2D.v2.X() + coordinateAxisY * superTriangle2D.v2.Y();
	v3_3D = this->coordinateCenterPos + coordinateAxisX * superTriangle2D.v3.X() + coordinateAxisY * superTriangle2D.v3.Y();
	superTriangle = FFace(v1_3D, v2_3D, v3_3D);

	//收集限制边集
	for (auto circle : this->FinalContourCircles)
	{
		for (auto edge : circle.edgeList)
		{
			mustExitEdges.push_back(FEdge(edge.vertA, edge.vertB));
		}
	}
	facesGenerator.DelaunayMachineArbitraryTurnOnNewest(contourVerts, mustExitEdges, superTriangle, this->coordinateAxisZ, result);

	///////////////////////////////////////////
	///////////////////////////////////////////

    return true;
}

FFace Boolean2DMachine::getSuperTriangle(vector<Point3f> v)
{
	float maxX, minX, maxY, minY;
	GetMaximumIn(v, maxX, minX, maxY, minY);

	Point3f v1((maxX + minX) / 2.0f, 2 * maxY - minY, 0);
	Point3f v2(maxX + (maxX - minX) / 2.0f + 8.0f, minY - 1.0f, 0);
	Point3f v3(maxX + minX - v2.X(), v2.Y(), 0);

	FFace SuperTriangle(v1, v2, v3);
	return SuperTriangle;
}

void Boolean2DMachine::GetMaximumIn(vector<Point3f> vertexs, float &maxX, float &minX, float &maxY, float &minY)
{
	minX = vertexs[0].X();
	maxX = vertexs[0].X();
	for (int i = 1; i < vertexs.size(); i++)
	{
		if (minX > vertexs[i].X())
			minX = vertexs[i].X();
		if (maxX < vertexs[i].X())
			maxX = vertexs[i].X();
	}

	minY = vertexs[0].Y();
	maxY = vertexs[0].Y();
	for (int i = 1; i < vertexs.size(); i++)
	{
		if (minY > vertexs[i].Y())
			minY = vertexs[i].Y();
		if (maxY < vertexs[i].Y())
			maxY = vertexs[i].Y();
	}
}
