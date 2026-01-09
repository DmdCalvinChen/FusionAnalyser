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

#include "booleanmachine.h"

using namespace vcg;

BooleanMachine::BooleanMachine()
{
	this->sourceAA = NULL;
	this->sourceBB = NULL;
}

BooleanMachine::~BooleanMachine()
{
}

void BooleanMachine::ClearAllVectorObject()
{
	SPoints.clear();
	splitFacesA.clear(); splitFacesB.clear();
	splitTrianglesA.clear(); splitTrianglesB.clear();
	AinB.clear(); AoutB.clear();
	BinA.clear(); BoutA.clear();
	AinB_Split.clear(); AoutB_Split.clear(); BinA_Split.clear(); BoutA_Split.clear();
	interRecords.clear();
	interRecordsA.clear();
	interRecordsB.clear();
	BO2Dresult.clear();
}

void BooleanMachine::BooleanCompute(CMeshO *_a, CMeshO *_b, int iComputeKind, int** _collideMatrix, int _rowNumber, int _columnNumber)
{
	ClearAllVectorObject();

	A = _a;
	B = _b;
	cMA = _collideMatrix;
	rowNumA = _rowNumber;
	columnNumA = _columnNumber;

	if (rowNumA > 0)
	{
		//对碰撞矩阵中记录的各个面进行“面面相交检测”,并提取构建A和B各自的待剖分面队列
		AnalysisCollideMatrix(cMA, rowNumA, columnNumA, A, B, splitTrianglesA, splitTrianglesB);
		//将A和B得到的剖分面队列进行剖分，得到各自的剖分结果splitFaces
		splitFacesA = SplitEachTriangleInVector2(splitTrianglesA);
		splitFacesB = SplitEachTriangleInVector2(splitTrianglesB);
		// Classify spatial position of split faces
		ClassifyIntersectedFaces(splitFacesA, B, SPoints, AinB_Split, AoutB_Split);
		ClassifyIntersectedFaces(splitFacesB, A, SPoints, BinA_Split, BoutA_Split);
	}
	// Classify non-intersected faces (excluding triangles in splitTriangles)
	ClassifyFacesSpace(B, A, BinA, BoutA, splitTrianglesB);
	ClassifyFacesSpace(A, B, AinB, AoutB, splitTrianglesA);

	// Apply boolean operation based on user selection
	WhatBooleanModelYouWant(iComputeKind);

	for (int i = 0; i < rowNumA; i++)
	{
		if (cMA[i] != NULL)
		{
			delete[] cMA[i];
			cMA[i] = NULL;
		}
	}
	if (cMA != NULL)
	{
		delete[]cMA;
		cMA = NULL;
	}
}

void BooleanMachine::BooleanCompute(CMeshO *_a, CMeshO *_b, int iComputeKind, vector<Point2m> Couple)
{
	ClearAllVectorObject();
	A = _a;
	B = _b;

	if (!this->ignoreSplitFace_A.empty() && !this->ignoreSplitFace_B.empty())
	{
		int boolean2DOperateKind;
		switch (iComputeKind)
		{
		case AunitB:
			boolean2DOperateKind = Boolean2DMachine::A_UNION_B;
			break;

		case AminsB:
			boolean2DOperateKind = Boolean2DMachine::A_MINUS_B;
			break;

		case BminsA:
			boolean2DOperateKind = Boolean2DMachine::B_MINUS_A;
			break;

		case AintersectB:
			boolean2DOperateKind = Boolean2DMachine::A_INTERSECT_B;
			break;

		default:
			break;
		}
		machine2D.initial(A, B, this->ignoreSplitFace_A, this->ignoreSplitFace_B);
		machine2D.boolean2DCompute(boolean2DOperateKind, BO2Dresult);
	}

	//计算获得sub-face和sub-block
	{
		//非共面三角形对进行三维布尔运算
		{
			if (Couple.size() > 0)
			{
				for (auto cp : Couple)
				{
					vector<Point3f> tempVector;
					vector<TrackableVert> tempIntersectionTrack;
					FEdge tempEdge;
					tempVector.clear();
					if (TwoTriangleInsertCompute(cp.X(), cp.Y(), A, B, tempVector, tempIntersectionTrack, tempEdge)) //若两个面确实相交
					{
						//在A待分面容器中记录当前A的待分面
						if (!exitInVector<int>(cp.X(), this->ignoreSplitFace_A))
						{
							AddSplitTriangleToVector(cp.X(), A, tempVector, tempIntersectionTrack, tempEdge, splitTrianglesA);
						}

						//在B待分面容器中记录当前B的待分面
						if (!exitInVector<int>(cp.Y(), this->ignoreSplitFace_B))
						{
							AddSplitTriangleToVector(cp.Y(), B, tempVector, tempIntersectionTrack, tempEdge, splitTrianglesB);
						}
					}
				}
				//将A和B得到的剖分面队列进行剖分，得到各自的剖分结果splitFaces
				splitFacesA = SplitEachTriangleInVector2(splitTrianglesA);
				splitFacesB = SplitEachTriangleInVector2(splitTrianglesB);
				//对A，B相交面剖分得到的splitFaces的所有三角形进行空间位置划分
				ClassifyIntersectedFaces(splitFacesA, B, SPoints, AinB_Split, AoutB_Split);
				ClassifyIntersectedFaces(splitFacesB, A, SPoints, BinA_Split, BoutA_Split);
			}
			//对A，B非相交面进行空间位置判断。要求这些三角形不包含在splitTriangles中
			for (int i = 0; i < splitTrianglesA.size(); ++i)
			{
				this->putInVector<int>(splitTrianglesA[i].index, this->ignoreSplitFace_A);
			}
			for (int i = 0; i < splitTrianglesB.size(); ++i)
			{
				this->putInVector<int>(splitTrianglesB[i].index, this->ignoreSplitFace_B);
			}
			ClassifyFacesSpace(B, A, BinA, BoutA, this->ignoreSplitFace_B);
			ClassifyFacesSpace(A, B, AinB, AoutB, this->ignoreSplitFace_A);
		}
	}

	//提取出四个面集合之后，需要根据用户的选择进行相应的删除和添加
	WhatBooleanModelYouWant2(iComputeKind);
}

vector<Point2i> BooleanMachine::intersectEdgesCompute(CMeshO *_a, CMeshO *_b, int** _collideMatrix, int _rowNumber, int _columnNumber)
{
	//.
	ClearAllVectorObject();

	A = _a;
	B = _b;
	cMA = _collideMatrix;
	rowNumA = _rowNumber;
	columnNumA = _columnNumber;

	vector<Point2i> triangleCP;
	if (rowNumA > 0)
	{
		for (int i = 0; i < rowNumA; ++i)
		{
			for (int j = 1; j < columnNumA; ++j)
			{
				if (cMA[i][j] != -1)
				{
					triangleCP.push_back(Point2i(cMA[i][0], cMA[i][j]));
				}
			}
		}
		//对碰撞矩阵中记录的各个面进行“面面相交检测”,并提取构建A和B各自的待剖分面队列
		AnalysisCollideMatrix(cMA, rowNumA, columnNumA, A, B, splitTrianglesA, splitTrianglesB);
		splitFacesA = SplitEachTriangleInVector2(splitTrianglesA);
		ClassifyIntersectedFaces(splitFacesA, B, SPoints, AinB_Split, AoutB_Split);
	}

	for (int i = 0; i < rowNumA; i++)
	{
		if (cMA[i] != NULL)
		{
			delete[] cMA[i];
			cMA[i] = NULL;
		}
	}
	if (cMA != NULL)
	{
		delete[]cMA;
		cMA = NULL;
	}

	return triangleCP;
}

void BooleanMachine::AnalysisCollideMatrix(int** &cM, int &rowNum, int columnNum, CMeshO* A, CMeshO* B,
	                                       vector<SplitTriangle> &splitFaceA, vector<SplitTriangle> &splitFaceB)
{
	for (int i = 0; i < rowNum; i++)
	{
		for (int j = 1; j < columnNum; j++)
		{
			if (cM[i][j] != -1)
			{
				vector<Point3f> tempIntersection;
				vector<TrackableVert> tempIntersectionTrack;
				FEdge tempEdge;
				tempIntersection.clear();
				if (TwoTriangleInsertCompute(cM[i][0], cM[i][j], A, B, tempIntersection, tempIntersectionTrack, tempEdge)) //若两个面确实相交
				{
					//在A待分面容器中记录当前A的待分面
					AddSplitTriangleToVector(cM[i][0], A, tempIntersection, tempIntersectionTrack, tempEdge, splitFaceA);
					//在B待分面容器中记录当前B的待分面
					AddSplitTriangleToVector(cM[i][j], B, tempIntersection, tempIntersectionTrack, tempEdge, splitFaceB);
				}
			}
		}
	}
}

bool BooleanMachine::findNearIntersectPoint(Point3m &vert, const vector<Point3m> intersectVerts)
{
	for (auto v : intersectVerts)
	{
		if(vert == v)
		{
			vert = v;
			return true;
		}
	}
	return false;
}

bool BooleanMachine::TwoTriangleInsertCompute(int indexA, int indexB, CMeshO* meshA, CMeshO* meshB, vector<Point3f> &intersectVerts, vector<TrackableVert> &intersectTrackVerts, FEdge &intersectEdge)
{
	Point3m v10, v11, v12, v20, v21, v22;
	v10 = meshA->face[indexA].V(0)->P();
	v11 = meshA->face[indexA].V(1)->P();
	v12 = meshA->face[indexA].V(2)->P();
	v20 = meshB->face[indexB].V(0)->P();
	v21 = meshB->face[indexB].V(1)->P();
	v22 = meshB->face[indexB].V(2)->P();
	Point3m N1 = ((v10 - v11) ^ (v12 - v11)).Normalize();
	float d1 = -(N1 * v11);
	float dist20, dist21, dist22;
	dist20 = N1 * v20 + d1;
	dist21 = N1 * v21 + d1;
	dist22 = N1 * v22 + d1;
	if (dist20 > 0 && dist21 > 0 && dist22 > 0)
	{
		return false;
	}
	else if (dist20 < 0 && dist21 < 0 && dist22 < 0)
	{
		return false;
	}

	bool haveChecked = false;
	bool intersect01A, intersect12A, intersect20A;
	bool intersect01B, intersect12B, intersect20B;
	Point3f startP, endP;
	Point3f intersectV1A, intersectV2A, intersectV3A;
	Point3f intersectV1B, intersectV2B, intersectV3B;

	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*meshA);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*meshB);

	haveChecked = false;
	startP = meshB->face[indexB].V(0)->P();
	endP = meshB->face[indexB].V(1)->P();
	IntersectTestRecord record1(meshA, indexA, FEdge(startP, endP));
	for (auto record : interRecordsA)
	{
		if (record.sameAs(record1))
		{
			haveChecked = true;
			intersect01A = true;
			intersectV1A = record.intersection;
			if (!this->findNearIntersectPoint(record.intersection, intersectVerts))
			{
				intersectVerts.push_back(record.intersection);
			}
			intersectTrackVerts.push_back(TrackableVert(record.intersection, 0, meshB));
			SPoints.push_back(record.intersection);
			break;
		}
	}
	if (!haveChecked)
	{
		intersect01A = ComputeCrossPointEasy(startP, endP, indexA, meshA, intersectV1A);
		if (intersect01A)
		{
			if (!this->findNearIntersectPoint(intersectV1A, intersectVerts))
			{
				intersectVerts.push_back(intersectV1A);
			}
			intersectTrackVerts.push_back(TrackableVert(intersectV1A, 0, meshB));
			SPoints.push_back(intersectV1A);

			record1.intersection = intersectV1A;
			interRecordsA.push_back(record1);
		}
	}
	haveChecked = false;
	startP = meshB->face[indexB].V(1)->P();
	endP = meshB->face[indexB].V(2)->P();
	IntersectTestRecord record2(meshA, indexA, FEdge(startP, endP));
	for (auto record : interRecordsA)
	{
		if (record.sameAs(record2))
		{
			haveChecked = true;
			intersect12A = true;
			intersectV2A = record.intersection;
			if (!this->findNearIntersectPoint(record.intersection, intersectVerts))
			{
				intersectVerts.push_back(record.intersection);
			}
			intersectTrackVerts.push_back(TrackableVert(record.intersection, 1, meshB));
			SPoints.push_back(record.intersection);
			break;
		}
	}
	if (!haveChecked)
	{
		intersect12A = ComputeCrossPointEasy(startP, endP, indexA, meshA, intersectV2A);
		if (intersect12A)
		{
			if (!this->findNearIntersectPoint(intersectV2A, intersectVerts))
			{
				intersectVerts.push_back(intersectV2A);
			}
			intersectTrackVerts.push_back(TrackableVert(intersectV2A, 1, meshB));
			SPoints.push_back(intersectV2A);

			record2.intersection = intersectV2A;
			interRecordsA.push_back(record2);
		}
	}
	haveChecked = false;
	startP = meshB->face[indexB].V(2)->P();
	endP = meshB->face[indexB].V(0)->P();
	IntersectTestRecord record3(meshA, indexA, FEdge(startP, endP));
	for (auto record : interRecordsA)
	{
		if (record.sameAs(record3))
		{
			haveChecked = true;
			intersect20A = true;
			intersectV3A = record.intersection;
			if (!this->findNearIntersectPoint(record.intersection, intersectVerts))
			{
				intersectVerts.push_back(record.intersection);
			}
			intersectTrackVerts.push_back(TrackableVert(record.intersection, 2, meshB));
			SPoints.push_back(record.intersection);
			break;
		}
	}
	if (!haveChecked)
	{
		intersect20A = ComputeCrossPointEasy(startP, endP, indexA, meshA, intersectV3A);
		if (intersect20A)
		{
			if (!this->findNearIntersectPoint(intersectV3A, intersectVerts))
			{
				intersectVerts.push_back(intersectV3A);
			}
			intersectTrackVerts.push_back(TrackableVert(intersectV3A, 2, meshB));
			SPoints.push_back(intersectV3A);

			record3.intersection = intersectV3A;
			interRecordsA.push_back(record3);
		}
	}
	haveChecked = false;
	startP = meshA->face[indexA].V(0)->P();
	endP = meshA->face[indexA].V(1)->P();
	IntersectTestRecord record4(meshB, indexB, FEdge(startP, endP));
	for (auto record : interRecordsB)
	{
		if (record.sameAs(record4))
		{
			haveChecked = true;
			intersect01B = true;
			intersectV1B = record.intersection;
			if (!this->findNearIntersectPoint(record.intersection, intersectVerts))
			{
				intersectVerts.push_back(record.intersection);
			}
			intersectTrackVerts.push_back(TrackableVert(record.intersection, 0, meshA));
			SPoints.push_back(record.intersection);
			break;
		}
	}
	if (!haveChecked)
	{
		intersect01B = ComputeCrossPointEasy(startP, endP, indexB, meshB, intersectV1B);
		if (intersect01B)
		{
			if (!this->findNearIntersectPoint(intersectV1B, intersectVerts))
			{
				intersectVerts.push_back(intersectV1B);
			}
			intersectTrackVerts.push_back(TrackableVert(intersectV1B, 0, meshA));
			SPoints.push_back(intersectV1B);

			record4.intersection = intersectV1B;
			interRecordsB.push_back(record4);
		}
	}
	haveChecked = false;
	startP = meshA->face[indexA].V(1)->P();
	endP = meshA->face[indexA].V(2)->P();
	IntersectTestRecord record5(meshB, indexB, FEdge(startP, endP));
	for (auto record : interRecordsB)
	{
		if (record.sameAs(record5))
		{
			haveChecked = true;
			intersect12B = true;
			intersectV2B = record.intersection;
			if (!this->findNearIntersectPoint(record.intersection, intersectVerts))
			{
				intersectVerts.push_back(record.intersection);
			}
			intersectTrackVerts.push_back(TrackableVert(record.intersection, 1, meshA));
			SPoints.push_back(record.intersection);
			break;
		}
	}
	if (!haveChecked)
	{
		intersect12B = ComputeCrossPointEasy(startP, endP, indexB, meshB, intersectV2B);
		if (intersect12B)
		{
			if (!this->findNearIntersectPoint(intersectV2B, intersectVerts))
			{
				intersectVerts.push_back(intersectV2B);
			}
			intersectTrackVerts.push_back(TrackableVert(intersectV2B, 1, meshA));
			SPoints.push_back(intersectV2B);

			record5.intersection = intersectV2B;
			interRecordsB.push_back(record5);
		}
	}
	haveChecked = false;
	startP = meshA->face[indexA].V(2)->P();
	endP = meshA->face[indexA].V(0)->P();
	IntersectTestRecord record6(meshB, indexB, FEdge(startP, endP));
	for (auto record : interRecordsB)
	{
		if (record.sameAs(record6))
		{
			haveChecked = true;
			intersect20B = true;
			intersectV3B = record.intersection;
			if (!this->findNearIntersectPoint(record.intersection, intersectVerts))
			{
				intersectVerts.push_back(record.intersection);
			}
			intersectTrackVerts.push_back(TrackableVert(record.intersection, 2, meshA));
			SPoints.push_back(record.intersection);
			break;
		}
	}
	if (!haveChecked)
	{
		intersect20B = ComputeCrossPointEasy(startP, endP, indexB, meshB, intersectV3B);
		if (intersect20B)
		{
			if (!this->findNearIntersectPoint(intersectV3B, intersectVerts))
			{
				intersectVerts.push_back(intersectV3B);
			}
			intersectTrackVerts.push_back(TrackableVert(intersectV3B, 2, meshA));
			SPoints.push_back(intersectV3B);

			record6.intersection = intersectV3B;
			interRecordsB.push_back(record6);
		}
	}
	if (intersectVerts.size() == 2)
	{
		intersectEdge = FEdge(intersectVerts[0], intersectVerts[1]);
		return true;
	}
	else if (intersectVerts.size() == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool BooleanMachine::TwoTriangleInsertCompute3(int indexA, int indexB, CMeshO* meshA, CMeshO* meshB, vector<Point3f> &intersectVerts, vector<TrackableVert> &intersectTrackVerts, FEdge &intersectEdge)
{
	Point3m v10, v11, v12, v20, v21, v22;
	v10 = meshA->face[indexA].V(0)->P();
	v11 = meshA->face[indexA].V(1)->P();
	v12 = meshA->face[indexA].V(2)->P();
	v20 = meshB->face[indexB].V(0)->P();
	v21 = meshB->face[indexB].V(1)->P();
	v22 = meshB->face[indexB].V(2)->P();
	Point3m N1 = ((v10 - v11) ^ (v12 - v11)).Normalize();
	float d1 = -(N1 * v11);
	float dist20, dist21, dist22;
	dist20 = N1 * v20 + d1;
	dist21 = N1 * v21 + d1;
	dist22 = N1 * v22 + d1;
	if (dist20 > 0 && dist21 > 0 && dist22 > 0)
	{
		return false;
	}
	else if (dist20 < 0 && dist21 < 0 && dist22 < 0)
	{
		return false;
	}

	bool haveChecked = false;
	bool intersect01A, intersect12A, intersect20A;
	bool intersect01B, intersect12B, intersect20B;
	Point3f startP, endP;
	Point3f intersectV1A, intersectV2A, intersectV3A;
	Point3f intersectV1B, intersectV2B, intersectV3B;

	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*meshA);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*meshB);

	haveChecked = false;
	startP = meshB->face[indexB].V(0)->P();
	endP = meshB->face[indexB].V(1)->P();
	IntersectTestRecord record1(meshA, indexA, FEdge(startP, endP));
	this->quickGetIntersectionResult(record1, this->interRecordsA, intersect01A, intersectV1A,
		intersectVerts, intersectTrackVerts, startP, endP, indexA, meshA, meshB);

	haveChecked = false;
	startP = meshB->face[indexB].V(1)->P();
	endP = meshB->face[indexB].V(2)->P();
	IntersectTestRecord record2(meshA, indexA, FEdge(startP, endP));
	this->quickGetIntersectionResult(record2, this->interRecordsA, intersect12A, intersectV2A,
		intersectVerts, intersectTrackVerts, startP, endP, indexA, meshA, meshB);

	haveChecked = false;
	startP = meshB->face[indexB].V(2)->P();
	endP = meshB->face[indexB].V(0)->P();
	IntersectTestRecord record3(meshA, indexA, FEdge(startP, endP));
	this->quickGetIntersectionResult(record3, this->interRecordsA, intersect20A, intersectV3A,
		intersectVerts, intersectTrackVerts, startP, endP, indexA, meshA, meshB);

	//////////////////////////////////////

	haveChecked = false;
	startP = meshA->face[indexA].V(0)->P();
	endP = meshA->face[indexA].V(1)->P();
	IntersectTestRecord record4(meshB, indexB, FEdge(startP, endP));
	this->quickGetIntersectionResult(record4, this->interRecordsB, intersect01B, intersectV1B,
		intersectVerts, intersectTrackVerts, startP, endP, indexB, meshB, meshA);

	haveChecked = false;
	startP = meshA->face[indexA].V(1)->P();
	endP = meshA->face[indexA].V(2)->P();
	IntersectTestRecord record5(meshB, indexB, FEdge(startP, endP));
	this->quickGetIntersectionResult(record5, this->interRecordsB, intersect12B, intersectV2B,
		intersectVerts, intersectTrackVerts, startP, endP, indexB, meshB, meshA);

	haveChecked = false;
	startP = meshA->face[indexA].V(2)->P();
	endP = meshA->face[indexA].V(0)->P();
	IntersectTestRecord record6(meshB, indexB, FEdge(startP, endP));
	this->quickGetIntersectionResult(record6, this->interRecordsB, intersect20B, intersectV3B,
		intersectVerts, intersectTrackVerts, startP, endP, indexB, meshB, meshA);

	if (intersectVerts.size() == 2)
	{
		intersectEdge = FEdge(intersectVerts[0], intersectVerts[1]);
		return true;
	}
	else if (intersectVerts.size() == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void BooleanMachine::quickGetIntersectionResult(IntersectTestRecord newrecord, vector<IntersectTestRecord> &recordList, bool &bIntersected, Point3m &intersectionP,
	vector<Point3f> &intersectVerts, vector<TrackableVert> &intersectTrackVerts, Point3m startP, Point3m endP, int curFaceIndex, CMeshO* curMesh, CMeshO* anotherMesh)
{
	bool haveChecked = false;
	//使用二分法快速定位curMesh和curFaceIndex相同的IntersectTestRecord记录
	int curAimIndex = -1;
	curAimIndex = quickFindAimRecord(newrecord, recordList);
	if (curAimIndex >= 0)//若使用二分法找到对应的已经进行过交叉测试网格面
	{
		//则判断该面是否已经进行过相同边的交叉测试
		int result = recordList[curAimIndex].findSameEdgeSegment(newrecord);
		if (result >= 0)//找到同面同边的测试记录
		{
			haveChecked = true;
			bIntersected = true;
			intersectionP = recordList[curAimIndex].intersectionList[result];
			if (!this->findNearIntersectPoint(intersectionP, intersectVerts))
			{
				intersectVerts.push_back(intersectionP);
			}
			intersectTrackVerts.push_back(TrackableVert(intersectionP, 0, anotherMesh));
			SPoints.push_back(intersectionP);
		}
		else//只找到同面未找到同边
		{
			bIntersected = ComputeCrossPoint(startP, endP, curFaceIndex, curMesh, intersectionP);
			if (bIntersected)
			{
				if (!this->findNearIntersectPoint(intersectionP, intersectVerts))
				{
					intersectVerts.push_back(intersectionP);
				}
				intersectTrackVerts.push_back(TrackableVert(intersectionP, 0, anotherMesh));
				SPoints.push_back(intersectionP);

				newrecord.intersection = intersectionP;
				recordList.push_back(newrecord);
				recordList[curAimIndex].intersectionList.push_back(intersectionP);
				recordList[curAimIndex].edgeList.push_back(newrecord.edge);
			}
		}
	}
	else//未找到同面
	{
		bIntersected = ComputeCrossPoint(startP, endP, curFaceIndex, curMesh, intersectionP);
		if (bIntersected)
		{
			if (!this->findNearIntersectPoint(intersectionP, intersectVerts))
			{
				intersectVerts.push_back(intersectionP);
			}
			intersectTrackVerts.push_back(TrackableVert(intersectionP, 0, anotherMesh));
			SPoints.push_back(intersectionP);

			newrecord.intersection = intersectionP;
			newrecord.edgeList.push_back(newrecord.edge);
			newrecord.intersectionList.push_back(newrecord.intersection);
			this->quickInsertAimRecord(newrecord, recordList);
		}
	}
}

bool BooleanMachine::TwoTriangleInsertCompute2(int indexA, int indexB, CMeshO* meshA, CMeshO* meshB, vector<Point3f> &intersectVerts, vector<TrackableVert> &intersectTrackVerts, FEdge &intersectEdge)
{
	Point3m v10, v11, v12, v20, v21, v22;
	v10 = meshA->face[indexA].V(0)->P();
	v11 = meshA->face[indexA].V(1)->P();
	v12 = meshA->face[indexA].V(2)->P();
	v20 = meshB->face[indexB].V(0)->P();
	v21 = meshB->face[indexB].V(1)->P();
	v22 = meshB->face[indexB].V(2)->P();
	Point3m N1 = ((v10 - v11) ^ (v12 - v11)).Normalize();
	float d1 = -(N1 * v11);
	float dist20, dist21, dist22;
	dist20 = N1 * v20 + d1;
	dist21 = N1 * v21 + d1;
	dist22 = N1 * v22 + d1;

	if (dist20 > 0 && dist21 > 0 && dist22 > 0)
	{
		return false;
	}
	else if (dist20 < 0 && dist21 < 0 && dist22 < 0)
	{
		return false;
	}

	return false;
}

bool BooleanMachine::TwoTriangleInsertComputeProj(int indexA, int indexB, CMeshO* meshA, CMeshO* meshB, vector<Point3f> &intersectVerts, vector<TrackableVert> &intersectTrackVerts, FEdge &intersectEdge)
{
	//代替三角顶点，便于编写
	Point3m v10, v11, v12, v20, v21, v22;
	v10 = meshA->face[indexA].V(0)->P();
	v11 = meshA->face[indexA].V(1)->P();
	v12 = meshA->face[indexA].V(2)->P();
	v20 = meshB->face[indexB].V(0)->P();
	v21 = meshB->face[indexB].V(1)->P();
	v22 = meshB->face[indexB].V(2)->P();

	//选择边长最长的Triangle1边作为局部坐标系的axisX，Triangl1面法向量作为axisY，进而得出axisZ和center
	Point3m axisX, axisY, axisZ, center;
	Point3m edge1, edge2, edge3, maxLenEdge, maxLenEdgeStartVert;
	FEdge axisXEdge;
	edge1 = v11 - v10;
	edge2 = v12 - v11;
	edge3 = v10 - v12;
	float edgeLen1, edgeLen2, edgeLen3;
	edgeLen1 = edge1 * edge1;
	edgeLen2 = edge2 * edge2;
	edgeLen3 = edge3 * edge3;
	if (edgeLen1 > edgeLen2)
	{
		if (edgeLen1 > edgeLen3)
		{
			maxLenEdge = edge1;
			maxLenEdgeStartVert = v10;
			axisXEdge = FEdge(v10, v11);
		}
		else
		{
			maxLenEdge = edge3;
			maxLenEdgeStartVert = v12;
			axisXEdge = FEdge(v12, v10);
		}
	}
	else
	{
		if (edgeLen2 > edgeLen3)
		{
			maxLenEdge = edge2;
			maxLenEdgeStartVert = v11;
			axisXEdge = FEdge(v11, v12);
		}
		else
		{
			maxLenEdge = edge3;
			maxLenEdgeStartVert = v12;
			axisXEdge = FEdge(v12, v10);
		}
	}
	center = maxLenEdgeStartVert;
	axisX = (maxLenEdge).Normalize();
	axisY = meshA->face[indexA].N();
	axisZ = (axisX ^ axisY).Normalize();

	//将两个三角形各自的三个顶点转换为local坐标系描述的坐标
	Point3m localV10, localV11, localV12, localV20, localV21, localV22;
	localV10 = this->getLocalSysPoint(v10, axisX, axisY, axisZ, center);
	localV11 = this->getLocalSysPoint(v11, axisX, axisY, axisZ, center);
	localV12 = this->getLocalSysPoint(v12, axisX, axisY, axisZ, center);
	localV20 = this->getLocalSysPoint(v20, axisX, axisY, axisZ, center);
	localV21 = this->getLocalSysPoint(v21, axisX, axisY, axisZ, center);
	localV22 = this->getLocalSysPoint(v22, axisX, axisY, axisZ, center);

	//获得localTriangle1在X轴上的投影范围AHmin和AHmax
	float AHmin, AHmax;
	AHmin = localV10.X() < localV11.X() ? (localV10.X() < localV12.X() ? localV10.X() : localV12.X()) : (localV11.X() < localV12.X() ? localV11.X() : localV12.X());
	AHmax = localV10.X() > localV11.X() ? (localV10.X() > localV12.X() ? localV10.X() : localV12.X()) : (localV11.X() > localV12.X() ? localV11.X() : localV12.X());
	//获得localTriangle2三个顶点在H面(axisX, axisY)上的投影
	Point3m proj_H_V20, proj_H_V21, proj_H_V22;
	proj_H_V20 = Point3m(localV20.X(), localV20.Y(), 0);
	proj_H_V21 = Point3m(localV21.X(), localV21.Y(), 0);
	proj_H_V22 = Point3m(localV22.X(), localV22.Y(), 0);

	//测算proj_H_Triangle2与FEdge(Point3m(AHmin, 0, 0), Point3m(AHmax, 0, 0))的交叠区域
	//得出在localSys中描述的交叠线段p0, p1
	Point3m p0, p1;
	FFace proj_H_Triangle2(proj_H_V20, proj_H_V21, proj_H_V22);
	FFace localTriangle2(localV20, localV21, localV22);
	if (this->getAHProjIntersection(p0, p1, FEdge(Point3m(AHmin, 0, 0), Point3m(AHmax, 0, 0)), proj_H_Triangle2, localTriangle2))
	{
		//如若存在交叠在H面上的投影交叠，则p0,p1线段与localTriangle1在V面（axisX, axisZ）上的投影localTriangle1
		//所交叠的线段区域Q0Q1返回
		Point3m Q0, Q1;
		if (this->getAVProjIntersection(Q0, Q1, FEdge(p0, p1), FFace(localV10, localV11, localV12)))
		{
			Point3m intersection0, intersection1;
			intersection0 = this->releaseLocalSysPoint(Q0, axisX, axisZ, axisY, center);
			intersection1 = this->releaseLocalSysPoint(Q1, axisX, axisZ, axisY, center);

			if (!this->findNearIntersectPoint(intersection0, intersectVerts))
			{
				intersectVerts.push_back(intersection0);
			}
			if (!this->findNearIntersectPoint(intersection1, intersectVerts))
			{
				intersectVerts.push_back(intersection1);
			}
			intersectEdge = FEdge(intersection0, intersection1);
			//intersectEdge = FEdge(intersection1, intersection0);
			return true;
		}
	}
	return false;
}

float BooleanMachine::computeDet2D(Point2f a, Point2f b, Point2f c)
{
	return (c.X() - a.X()) * (a.Y() - b.Y()) + (a.Y() - c.Y()) * (a.X() - b.X());
}

bool BooleanMachine::pointInsideTriangle(Point3m p, int faceIndex, CMeshO *curMesh)
{
	Point3f v0, v1, v2;
	v0 = curMesh->face[faceIndex].V(0)->P();
	v1 = curMesh->face[faceIndex].V(1)->P();
	v2 = curMesh->face[faceIndex].V(2)->P();
	Point3f faceNormalV = curMesh->face[faceIndex].N();

	Point3f axisX = (v1 - v0).Normalize();
	Point3f axisZ = faceNormalV;
	Point3f axisY = (axisZ ^ axisX).Normalize();

	Point2f vertA, vertB, vertC, vertP;
	vertA.X() = 0;
	vertA.Y() = 0;
	vertB.X() = (v1 - v0) * axisX;
	vertB.Y() = (v1 - v0) * axisY;
	vertC.X() = (v2 - v0) * axisX;
	vertC.Y() = (v2 - v0) * axisY;
	vertP.X() = (p - v0) * axisX;
	vertP.Y() = (p - v0) * axisY;

	float det1, det2, det3;
	det1 = this->computeDet2D(vertA, vertB, vertP);
	det2 = this->computeDet2D(vertB, vertC, vertP);
	det3 = this->computeDet2D(vertC, vertA, vertP);

	det1 = abs(det1) < 5e-5 ? 0 : det1;
	det2 = abs(det2) < 5e-5 ? 0 : det2;
	det3 = abs(det3) < 5e-5 ? 0 : det3;

	if (det1 >= 0 && det2 >= 0 && det3 >= 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool BooleanMachine::ComputeCrossPoint(Point3f startP, Point3f endP, int iFaceIndex, CMeshO *Mesh, Point3f &CrossPoint)
{
	if (startP == endP)
		return false;

	Point3f v0, v1, v2;
	Point3f dir;
	dir = endP - startP;
	v0 = Mesh->face[iFaceIndex].V(0)->P();
	v1 = Mesh->face[iFaceIndex].V(1)->P();
	v2 = Mesh->face[iFaceIndex].V(2)->P();
	//else
	//    return false;

	Point3f axisX, axisY, axisZ;
	Point3f E1 = v1 - v0;
	Point3f E2 = v2 - v0;
	Point3f FaceNormalV = E1 ^ E2;
	FaceNormalV = FaceNormalV.Normalize();
	axisX = E1;
	axisX = axisX.Normalize();
	axisY = axisX ^ FaceNormalV;
	axisY = axisY.Normalize();

	float dotMulit1, dotMulit2;
	Point3f tempV1, tempV2;
	tempV1 = startP - v0;
	tempV2 = endP - v0;
	dotMulit1 = FaceNormalV * tempV1;
	dotMulit2 = FaceNormalV * tempV2;
	if (abs(dotMulit1) < 1e-6)
	{
		dotMulit1 = 0;
		CrossPoint = startP;

		//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx//
		Point3m tempEdge, tempAxisX, tempAxisY;
		Point3m projV0, projV1, projV2, projResult;
		tempAxisX = (Mesh->face[iFaceIndex].V(1)->P() - Mesh->face[iFaceIndex].V(0)->P()).Normalize();
		tempAxisY = (tempAxisX ^ FaceNormalV).Normalize();

		tempEdge = Mesh->face[iFaceIndex].V(1)->P() - Mesh->face[iFaceIndex].V(0)->P();
		projV1 = Point3m(tempEdge * tempAxisX, tempEdge * tempAxisY, 0);
		tempEdge = Mesh->face[iFaceIndex].V(2)->P() - Mesh->face[iFaceIndex].V(0)->P();
		projV2 = Point3m(tempEdge * tempAxisX, tempEdge * tempAxisY, 0);
		tempEdge = CrossPoint - Mesh->face[iFaceIndex].V(0)->P();
		projResult = Point3m(tempEdge * tempAxisX, tempEdge * tempAxisY, 0);
		projV0 = Point3m(0, 0, 0);

		FFace tempTriangle(projV0, projV1, projV2);
		bool intersectHappen = tempTriangle.ifVertInsideTriangleOn2DSpace(projResult);
		//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx//

		return intersectHappen;
	}
	else if (abs(dotMulit2) < 1e-6)
	{
		dotMulit2 = 0;
		CrossPoint = endP;

		//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx//
		Point3m tempEdge, tempAxisX, tempAxisY;
		Point3m projV0, projV1, projV2, projResult;
		tempAxisX = (Mesh->face[iFaceIndex].V(1)->P() - Mesh->face[iFaceIndex].V(0)->P()).Normalize();
		tempAxisY = (tempAxisX ^ FaceNormalV).Normalize();

		tempEdge = Mesh->face[iFaceIndex].V(1)->P() - Mesh->face[iFaceIndex].V(0)->P();
		projV1 = Point3m(tempEdge * tempAxisX, tempEdge * tempAxisY, 0);
		tempEdge = Mesh->face[iFaceIndex].V(2)->P() - Mesh->face[iFaceIndex].V(0)->P();
		projV2 = Point3m(tempEdge * tempAxisX, tempEdge * tempAxisY, 0);
		tempEdge = CrossPoint - Mesh->face[iFaceIndex].V(0)->P();
		projResult = Point3m(tempEdge * tempAxisX, tempEdge * tempAxisY, 0);
		projV0 = Point3m(0, 0, 0);

		FFace tempTriangle(projV0, projV1, projV2);
		bool intersectHappen = tempTriangle.ifVertInsideTriangleOn2DSpace(projResult);
		//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx//
		return intersectHappen;
	}
	else if (dotMulit1 * dotMulit2 > 0) //线段两端点在三角形所在平面的同一侧，则线段和三角面不可能相交
	{
		return false;
	}

	float t;
	Point3f node;//交点
	t = float((FaceNormalV.X() * (v0.X() - startP.X()) + FaceNormalV.Y() * (v0.Y() - startP.Y()) + FaceNormalV.Z() * (v0.Z() - startP.Z()))
		       / (dir.X() * FaceNormalV.X() + dir.Y() * FaceNormalV.Y() + dir.Z() * FaceNormalV.Z()));
	if (t < 0 || t > 1)//向量值在线段上则t值应在（0，1）区间内
	{
		return false;
	}
	else if (t > 0 && t < 1e-5)
	{
		//CrossPoint = startP;
		//return true;
		node = startP;
	}
	else if (t < 1 && abs(1 - t) < 1e-5)
	{
		//CrossPoint = endP;
		//return true;
		node = endP;
	}
	else
	{
		node = startP + dir * t;
	}

	float fu, fv;
	Point3f E3 = node - v0;
	this->computeCoordinateIn2D(E3, fu, fv, axisX, axisY, E1, E2);

 	float fUVdeviation = 1e-6;
	float fUVdeviationNegative = 1e-6;

	if (abs(fu) < fUVdeviation && fu > 0)
	{
		fu = 0;
	}
	if (abs(fv) < fUVdeviation && fv > 0)
	{
		fv = 0;
	}
	if (fu < 0)
	{
		if (abs(fu) < fUVdeviationNegative)
		{
			fu = 0;
		}
		else
		{
			return false;
		}
	}
	if (fv < 0)
	{
		if (abs(fv) < fUVdeviationNegative)
		{
			fv = 0;
		}
		else
		{
			return false;
		}
	}

	if (fu == 0 && fv == 0)
	{
		CrossPoint = v0;
		return true;
	}
	if (fu == 0 && abs(1 - fv) < fUVdeviation && fv > 0)
	{
		CrossPoint = v2;
		return true;
	}
	if (abs(1 - fu) < fUVdeviation && fv == 0 && fu > 0)
	{
		CrossPoint = v1;
		return true;
	}
	if (fu >= 0 && fv >= 0 && (fu + fv) <= 1)
	{
		CrossPoint = node;
		return true;
	}
	else
		return false;
}

bool BooleanMachine::ComputeCrossPointEasy(Point3f startP, Point3f endP, int iFaceIndex, CMeshO *Mesh, Point3f &CrossPoint)
{
	bool intersectHappen = false;

	Point3m result;
	Point3m pos, p, D, N;
	pos = Mesh->face[iFaceIndex].V(0)->P();
	N = Mesh->face[iFaceIndex].N();
	D = endP - startP;
	p = startP;

	float up, down;
	up = (pos - p) * N;
	down = D * N;
	float t = up / down;

	if(0<= t && t<= 1)
	{
	  result = p + D * t;
	  CrossPoint = result;
	}
	else
	{
		return false;
	}

	Point3m tempEdge, tempAxisX, tempAxisY;
	Point3m projV0, projV1, projV2, projResult;
	tempAxisX = (Mesh->face[iFaceIndex].V(1)->P() - Mesh->face[iFaceIndex].V(0)->P()).Normalize();
	tempAxisY = (tempAxisX ^ N).Normalize();

	tempEdge = Mesh->face[iFaceIndex].V(1)->P() - Mesh->face[iFaceIndex].V(0)->P();
	projV1 = Point3m(tempEdge * tempAxisX, tempEdge * tempAxisY, 0);
	tempEdge = Mesh->face[iFaceIndex].V(2)->P() - Mesh->face[iFaceIndex].V(0)->P();
	projV2 = Point3m(tempEdge * tempAxisX, tempEdge * tempAxisY, 0);
	tempEdge = result - Mesh->face[iFaceIndex].V(0)->P();
	projResult = Point3m(tempEdge * tempAxisX, tempEdge * tempAxisY, 0);
	projV0 = Point3m(0, 0, 0);

	FFace tempTriangle(projV0, projV1, projV2);
	intersectHappen = tempTriangle.ifVertInsideTriangleOn2DSpace(projResult);

	return intersectHappen;
}

void BooleanMachine::AddSplitTriangleToVector(int Index, CMeshO *PMesh, vector<Point3f> newSplitPoints, vector<TrackableVert> intersectTrackVerts, FEdge newSplitEdge,
	                                          vector<SplitTriangle> &splitTriangles)
{
	vector<TrackableVert> correctTrackVerts;
	for (auto vert : intersectTrackVerts)
	{
		if (vert.sourceModel == PMesh)
		{
			correctTrackVerts.push_back(vert);
		}
		else
		{
			vert.iEdge = -1;
			correctTrackVerts.push_back(vert);
		}
	}

	bool IsNewSplitTriangle = true;
	for (int i = 0; i < splitTriangles.size(); i++)
	{
		if (splitTriangles[i].index == Index) //当前的剖分面在剖分面容器中已经存在
		{
			IsNewSplitTriangle = false;
			for (int j = 0; j < newSplitPoints.size(); j++)
			{
				bool newVertex = true;
				for (auto v : splitTriangles[i].splitPoints)
				{
					if (v == newSplitPoints[j])
					{
						newVertex = false;
						break;
					}
				}

				if (newVertex)
				{
					splitTriangles[i].splitPoints.push_back(newSplitPoints[j]);
					if (!correctTrackVerts.empty())
					{
						splitTriangles[i].splitTrackPoints.push_back(correctTrackVerts[j]);
					}
				}
			}

			if (newSplitPoints.size() == 2)
			{
				bool newBorder = true;
				for (auto edge : splitTriangles[i].splitEdges)
				{
					if (edge == newSplitEdge)
					{
						newBorder = false;
						break;
					}
				}
				if (newBorder)
				{
					splitTriangles[i].splitEdges.push_back(newSplitEdge);
				}
			}
			break;
		}
	}

	if (IsNewSplitTriangle)
	{
		splitTriangles.push_back(SplitTriangle(Index, PMesh, newSplitPoints, correctTrackVerts, newSplitEdge));
	}
	correctTrackVerts.clear();
}

vector<FFace> BooleanMachine::SplitEachTriangleInVector2(vector<SplitTriangle> &faceList)
{
	vector<FFace> result;
	vector<FFace> PartialFaces1;
	for (int i = 0; i < faceList.size(); i++)
	{
		if (i == 19)
		{
			int stop = 1;
		}
		PartialFaces1.clear();
		PartialFaces1 = SplitOneTriangleFace2(faceList[i]);
		faceList[i].splitResults = PartialFaces1;

		for (int j = 0; j < PartialFaces1.size(); j++)
		{
			result.push_back(PartialFaces1[j]);
		}
	}

	return result;
}

vector<FFace> BooleanMachine::SplitOneTriangleFace2(SplitTriangle face)
{
	//去除重复的点，得到要进入剖分机器的顶点集vertexes
	vector<Point3f> vertexes;
	vertexes.clear();
	for (int i = 0; i < face.splitPoints.size(); i++)
	{
		bool IsNewVertex = true;
		for (int j = 0; j < vertexes.size(); j++)
		{
			if (face.splitPoints[i] == vertexes[j])
			{
				IsNewVertex = false;
				break;
			}
		}

		if (IsNewVertex)
			vertexes.push_back(face.splitPoints[i]);
	}

	face.collectEdgeCombed();
	vector<FEdge> tempEdges;
	vector<FFace> splitFaces = splitMachine.DelaunayMachineArbitraryTurnOn2(vertexes, face.splitEdges,
		FFace(face.pMesh->face[face.index].V(0)->P(), face.pMesh->face[face.index].V(1)->P(), face.pMesh->face[face.index].V(2)->P()));

	return splitFaces;
}

void BooleanMachine::ClassifyIntersectedFaces(vector<FFace> splitFaces, CMeshO *AimMesh, vector<Point3f> splitPoints,
	                                          vector<FFace> &IN_S, vector<FFace> &OUT_S)
{
	for (int i = 0; i < splitFaces.size(); i++)
	{
		if (ClassifyIntersectedFace(splitFaces[i], AimMesh))
		{
			OUT_S.push_back(splitFaces[i]);
		}
		else
		{
			IN_S.push_back(splitFaces[i]);
		}
	}
}

bool BooleanMachine::ClassifyIntersectedFace(FFace splitFace, CMeshO *AimMesh)
{
    return true;
}

vector<int>  BooleanMachine::filterPossiblyIntersectFaces(Point3m P, Point3m edge1, Point3m edge2, vector<int> faceList, CMeshO *AimMesh)
{
	vector<int> result;

	Point3m E, N;
	bool vert1OnOneSide = false, vert2OnOneSide = false, vert3OnOneSide = false;
	int spaticalSide1, spaticalSide2, spaticalSide3;
	N = (edge1 ^ edge2).Normalize();
	float tempDotMulit;
	for (auto index : faceList)
	{
		E = AimMesh->face[index].V(0)->P() - P;
		tempDotMulit = E * N;
		if (tempDotMulit > 0)
		{
			spaticalSide1 = 1;
		}
		else if (tempDotMulit == 0)
		{
			spaticalSide1 = 0;
		}
		else
		{
			spaticalSide1 = -1;
		}

		E = AimMesh->face[index].V(1)->P() - P;
		tempDotMulit = E * N;
		if (tempDotMulit > 0)
		{
			spaticalSide2 = 1;
		}
		else if (tempDotMulit == 0)
		{
			spaticalSide2 = 0;
		}
		else
		{
			spaticalSide2 = -1;
		}

		E = AimMesh->face[index].V(2)->P() - P;
		tempDotMulit = E * N;
		if (tempDotMulit > 0)
		{
			spaticalSide3 = 1;
		}
		else if (tempDotMulit == 0)
		{
			spaticalSide3 = 0;
		}
		else
		{
			spaticalSide3 = -1;
		}

		if (spaticalSide1 != spaticalSide2 || spaticalSide2 != spaticalSide3)
		{
			result.push_back(index);
		}
	}

	return result;
}

void BooleanMachine::ClassifyFacesSpace(CMeshO* pMesh, CMeshO* pAimMesh, vector<int> &IN_NS, vector<int> &OUT_NS,
	                                    vector<SplitTriangle> ignoreFace)
{
	for (int i = 0; i < pMesh->face.size(); i++)
	{
		bool IgnoreIt = false;
		for (int j = 0; j < ignoreFace.size(); j++)
		{
			if (i == ignoreFace[j].index)
			{
				IgnoreIt = true;
				break;
			}
		}

		if (!IgnoreIt)
		{
			Point3f P, N;
			P = (pMesh->face[i].V(0)->P() + pMesh->face[i].V(1)->P() + pMesh->face[i].V(2)->P()) / 3.0f;
			N = -pMesh->face[i].N();

			int intersectNum = HowManyTimesIntersected(P, N, pAimMesh);

			if (intersectNum % 2 == 0)
			{
				OUT_NS.push_back(i);
			}
			else
			{
				IN_NS.push_back(i);
			}
		}
	}
}

void BooleanMachine::ClassifyFacesSpace(CMeshO* pMesh, CMeshO* pAimMesh, vector<int> &IN_NS, vector<int> &OUT_NS,
	                                    vector<int> ignoreIndexList)
{
	for (int i = 0; i < pMesh->face.size(); i++)
	{
		bool IgnoreIt = false;
		for (auto ignoreIndex : ignoreIndexList)
		{
			if (i == ignoreIndex)
			{
				IgnoreIt = true;
				break;
			}
		}

		if (!IgnoreIt)
		{
			Point3f P, N;
			P = (pMesh->face[i].V(0)->P() + pMesh->face[i].V(1)->P() + pMesh->face[i].V(2)->P()) / 3.0f;
			N = -pMesh->face[i].N();

			int intersectNum = HowManyTimesIntersected(P, N, pAimMesh);

			if (intersectNum % 2 == 0)
			{
				OUT_NS.push_back(i);
			}
			else
			{
				IN_NS.push_back(i);
			}
		}
	}
}

void BooleanMachine::ClassifyWaitingFacesSpace(vector<int> waitingface, CMeshO* pMesh, CMeshO* pAimMesh, vector<int> &IN_NS, vector<int> &OUT_NS, vector<SplitTriangle> ignoreFace)
{
	vcg::tri::UpdateNormal<CMeshO>::PerVertex(*pMesh);
	vcg::tri::UpdateNormal<CMeshO>::PerFace(*pMesh);
	for (auto index : waitingface)
	{
		bool IgnoreIt = false;
		for (int j = 0; j < ignoreFace.size(); j++)
		{
			if (index == ignoreFace[j].index)
			{
				IgnoreIt = true;
				break;
			}
		}

		if (!IgnoreIt)
		{
			Point3f P, N;
			P = ((pMesh->face[index].V(0)->P() + pMesh->face[index].V(1)->P()) / 2.0f + pMesh->face[index].V(2)->P()) / 2.0f;
			N = -pMesh->face[index].N();

			Point3m N3 = pMesh->face[index].V(1)->P() - P;
			N3 = N3.Normalize();

			int intersectNum = HowManyTimesIntersected(P, N, pAimMesh);

			if (intersectNum % 2 == 0)
			{
				OUT_NS.push_back(index);
			}
			else
			{
				IN_NS.push_back(index);
			}
		}
	}
}

bool BooleanMachine::judgeOnFaceSpace(int index, CMeshO* pMesh, CMeshO* pAimMesh, vector<FFace>& filterFaceList)
{
	vcg::tri::UpdateNormal<CMeshO>::PerVertex(*pMesh);
	vcg::tri::UpdateNormal<CMeshO>::PerFace(*pMesh);

	Point3f P, N;
	P = ((pMesh->face[index].V(0)->P() + pMesh->face[index].V(1)->P()) / 2.0f + pMesh->face[index].V(2)->P()) / 2.0f;
	N = -pMesh->face[index].N();

	Point3m N2 = pMesh->face[index].V(0)->P() - P;
	N2 = N2.Normalize();
	vector<int> maybeFaces;
	for (int i = 0; i < pAimMesh->face.size(); ++i)
	{
		if (!pAimMesh->face[i].IsD())
		{
			maybeFaces.push_back(i);
		}
	}
	maybeFaces = filterPossiblyIntersectFaces(P, N, N2, maybeFaces, pAimMesh);

	Point3m N3 = pMesh->face[index].V(1)->P() - P;
	N3 = N3.Normalize();
	maybeFaces = filterPossiblyIntersectFaces(P, N, N3, maybeFaces, pAimMesh);

	int intersectNum = HowManyTimesIntersected(P, N, maybeFaces, pAimMesh);
	for (auto i : maybeFaces)
	{
		filterFaceList.push_back(FFace(pAimMesh->face[i].V(0)->P(), pAimMesh->face[i].V(1)->P(), pAimMesh->face[i].V(2)->P()));
	}
	maybeFaces.clear();

	if (intersectNum % 2 == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void BooleanMachine::WhatBooleanModelYouWant(int iComputeKind)
{
	//删除牙颌模型要删除的面，将要添加的面添加进去
	for (int i = 0; i < splitTrianglesA.size(); i++)
		vcg::tri::Allocator<CMeshO>::DeleteFace(*A, A->face[splitTrianglesA[i].index]);
	for (int i = 0; i < AinB.size(); i++)
		vcg::tri::Allocator<CMeshO>::DeleteFace(*A, A->face[AinB[i]]);
	vcg::tri::Allocator<CMeshO>::CompactFaceVector(*A);

	////将faceList中的所有面添加到网格中去
	for (int i = 0; i < AoutB_Split.size(); i++)
	{
		CMeshO::VertexPointer ivp[3];
		ivp[0] = &*vcg::tri::Allocator<CMeshO>::AddVertex(*A, AoutB_Split[i].v1);
		ivp[1] = &*vcg::tri::Allocator<CMeshO>::AddVertex(*A, AoutB_Split[i].v2);
		ivp[2] = &*vcg::tri::Allocator<CMeshO>::AddVertex(*A, AoutB_Split[i].v3);
		//保持原来的颜色，保持原来的三角形所在面的牙齿分割编号
		ivp[0]->C().X() = 1; ivp[1]->C().X() = 1; ivp[2]->C().X() = 1;
		ivp[0]->C().Y() = 1; ivp[1]->C().Y() = 1; ivp[2]->C().Y() = 1;
		ivp[0]->C().Z() = 1; ivp[1]->C().Z() = 1; ivp[2]->C().Z() = 1;
		vcg::tri::Allocator<CMeshO>::AddFace(*A, ivp[0], ivp[1], ivp[2]);
	}
	vcg::tri::Allocator<CMeshO>::CompactFaceVector(*A);

	if (iComputeKind == AunitB)
	{
		vector<FFace> BOUTA;
		for (int i = 0; i < BoutA_Split.size(); i++)
		{
			BOUTA.push_back(BoutA_Split[i]);
		}
		for (int i = 0; i < BoutA.size(); i++)
		{
			BOUTA.push_back(FFace(B->face[BoutA[i]].V(0)->P(), B->face[BoutA[i]].V(1)->P(), B->face[BoutA[i]].V(2)->P()));
		}
		for (int i = 0; i < BOUTA.size(); i++)
		{
			CMeshO::VertexPointer ivp[3];
			ivp[0] = &*vcg::tri::Allocator<CMeshO>::AddVertex(*A, BOUTA[i].v1);
			ivp[1] = &*vcg::tri::Allocator<CMeshO>::AddVertex(*A, BOUTA[i].v2);
			ivp[2] = &*vcg::tri::Allocator<CMeshO>::AddVertex(*A, BOUTA[i].v3);
			//保持原来的颜色，保持原来的三角形所在面的牙齿分割编号
			ivp[0]->C().X() = 1; ivp[1]->C().X() = 1; ivp[2]->C().X() = 1;
			ivp[0]->C().Y() = 1; ivp[1]->C().Y() = 1; ivp[2]->C().Y() = 1;
			ivp[0]->C().Z() = 1; ivp[1]->C().Z() = 1; ivp[2]->C().Z() = 1;
			vcg::tri::Allocator<CMeshO>::AddFace(*A, ivp[0], ivp[1], ivp[2]);
		}
		vcg::tri::Allocator<CMeshO>::CompactFaceVector(*A);
	}
	else if (iComputeKind == AminsB)
	{
		vector<FFace> BINA;
		for (int i = 0; i < BinA_Split.size(); i++)
		{
			BINA.push_back(FFace(BinA_Split[i].v1, BinA_Split[i].v3, BinA_Split[i].v2));
		}
		for (int i = 0; i < BinA.size(); i++)
		{
			BINA.push_back(FFace(B->face[BinA[i]].V(0)->P(), B->face[BinA[i]].V(2)->P(), B->face[BinA[i]].V(1)->P()));
		}
		for (int i = 0; i < BINA.size(); i++)
		{
			CMeshO::VertexPointer ivp[3];
			ivp[0] = &*vcg::tri::Allocator<CMeshO>::AddVertex(*A, BINA[i].v1);
			ivp[1] = &*vcg::tri::Allocator<CMeshO>::AddVertex(*A, BINA[i].v2);
			ivp[2] = &*vcg::tri::Allocator<CMeshO>::AddVertex(*A, BINA[i].v3);
			//保持原来的颜色，保持原来的三角形所在面的牙齿分割编号
			ivp[0]->C().X() = 1;
			ivp[1]->C().X() = 1;
			ivp[2]->C().X() = 1;
			vcg::tri::Allocator<CMeshO>::AddFace(*A, ivp[0], ivp[1], ivp[2]);
		}
		vcg::tri::Allocator<CMeshO>::CompactFaceVector(*A);
	}
}

void BooleanMachine::WhatBooleanModelYouWant2(int iComputeKind)
{
	generateAddingFaces(iComputeKind);

	vector<int> List;
	for (auto index : this->AinB)
	{
		List.push_back(index);
	}
	for (auto deleteFace : this->splitTrianglesA)
	{
		List.push_back(deleteFace.index);
	}
	for (auto index : this->ignoreSplitFace_A)
	{
		List.push_back(index);
	}

	deleteFacesInList(List);

	addAllFaces();
}

int BooleanMachine::HowManyTimesIntersected(Point3f P, Point3f DirectV, CMeshO *AimMesh)
{
	int result = 0;

	for (int i = 0; i < AimMesh->face.size(); i++)
	{
		if (IfThisRadialIntersectWith(P, DirectV, i, AimMesh))
		{
			result++;
		}
	}
	return result;
}

int BooleanMachine::HowManyTimesIntersected(Point3f P, Point3f DirectV, vector<int> indexList, CMeshO *AimMesh)
{
	int result = 0;

	for (auto index : indexList)
	{
		if (IfThisRadialIntersectWith(P, DirectV, index, AimMesh))
		{
			result++;
		}
	}

	return result;
}

bool BooleanMachine::IfThisRadialIntersectWith(Point3f startP, Point3f DirectV, int iFaceIndex, CMeshO *Mesh)
{
	Point3f v0, v1, v2;
	Point3f dir;
	dir = DirectV;
	v0 = Mesh->face[iFaceIndex].V(0)->P();
	v1 = Mesh->face[iFaceIndex].V(1)->P();
	v2 = Mesh->face[iFaceIndex].V(2)->P();

	Point3f axisX, axisY, axisZ;
	Point3f E1 = v1 - v0;
	Point3f E2 = v2 - v0;
	Point3f FaceNormalV = E1 ^ E2;
	axisX = E1;
	axisX = axisX.Normalize();
	axisY = axisX ^ FaceNormalV;
	axisY = axisY.Normalize();

	float t;
	Point3f node;//交点
	t = float((FaceNormalV.X() * (v0.X() - startP.X()) + FaceNormalV.Y() * (v0.Y() - startP.Y()) + FaceNormalV.Z() * (v0.Z() - startP.Z()))
		/ (dir.X() * FaceNormalV.X() + dir.Y() * FaceNormalV.Y() + dir.Z() * FaceNormalV.Z()));
	if (t <= 0)
		return false;

	node = startP + dir * t;
	float fu, fv;
	Point3f E3 = node - v0;
	this->computeCoordinateIn2D(E3, fu, fv, axisX, axisY, E1, E2);

	if (abs(fu) < 0.00001f)
	{
		fu = 0;
	}
	if (abs(fv) < 0.00001f)
	{
		fv = 0;
	}

	if (fu >= 0 && fv >= 0 && (fu + fv) <= 1)
		return true;
	else
		return false;
}

void BooleanMachine::computeCoordinateIn2D(Point3f E3, float &fu, float &fv, Point3f axisX, Point3f axisY, Point3f U, Point3f V)
{
	float fUx, fUy, fVx, fVy, fX, fY;
	fUx = U * axisX;
	fUy = U * axisY;
	fVx = V * axisX;
	fVy = V * axisY;
	fX = E3 * axisX;
	fY = E3 * axisY;

	fv = (fY * fUx - fX * fUy) / (fVy * fUx - fVx * fUy);
	fu = (fX + fY - fv * (fVx + fVy)) / (fUx + fUy);
	return;
}

void BooleanMachine::generateAddingFaces(int iKind)
{
	addingFaces.clear();

	for (auto face : AoutB_Split)
	{
		addingFaces.push_back(face);
	}

	switch (iKind)
	{
	case AunitB:
		for (auto face : BoutA_Split)
		{
			addingFaces.push_back(face);
		}
		for (auto i : BoutA)
		{
			addingFaces.push_back(FFace(B->face[i].V(0)->P(), B->face[i].V(1)->P(), B->face[i].V(2)->P()));
		}
		break;

	case AminsB:
		for (auto face : BinA_Split)
		{
			addingFaces.push_back(face);
		}
		for (auto i : BinA)
		{
			addingFaces.push_back(FFace(B->face[i].V(0)->P(), B->face[i].V(1)->P(), B->face[i].V(2)->P()));
		}
		break;

	default:
		break;
	}

	if (!this->BO2Dresult.empty())
	{
		for (auto face : this->BO2Dresult)
		{
			addingFaces.push_back(face);
		}
	}

	return;
}

void BooleanMachine::deleteFacesInList(vector<int> deleteFaceList)
{
	for (int i = 0; i < deleteFaceList.size(); i++)
	{
		if (!A->face[deleteFaceList[i]].IsD())
		{
			vcg::tri::Allocator<CMeshO>::DeleteFace(*A, A->face[deleteFaceList[i]]);
		}
	}

	vector<int> deleteVertexList;
	for (int i = 0; i < A->VN(); i++)
	{
		vcg::face::VFIterator<CFaceO> vfi(&A->vert[i]);
		int I = 0;
		for (; !vfi.End(); ++vfi)
		{
			I++;
		}
		if (I == 0)
		{
			deleteVertexList.push_back(i);
		}
	}
	for (int i = 0; i < deleteVertexList.size(); i++)
	{
		vcg::tri::Allocator<CMeshO>::DeleteVertex(*A, A->vert[deleteVertexList[i]]);
	}

	vcg::tri::Allocator<CMeshO>::CompactFaceVector(*A);
	vcg::tri::Allocator<CMeshO>::CompactVertexVector(*A);

	// update topology
	assert(tri::HasFFAdjacency(*A) == false);
	A->face.EnableFFAdjacency();
	assert(tri::HasFFAdjacency(*A) == true);

	assert(tri::HasVFAdjacency(*A) == false);
	A->vert.EnableVFAdjacency();
	A->face.EnableVFAdjacency();
	assert(tri::HasVFAdjacency(*A) == true);

	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*A);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(*A);

	// update Normals
	vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(*A);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*A);
}

void BooleanMachine::deleteFacesInList(vector<int> deleteFaceList, CMeshO *curMesh)
{
	for (int i = 0; i < deleteFaceList.size(); i++)
	{
		if (!curMesh->face[deleteFaceList[i]].IsD())
		{
			vcg::tri::Allocator<CMeshO>::DeleteFace(*curMesh, curMesh->face[deleteFaceList[i]]);
		}
	}

	vector<int> deleteVertexList;
	for (int i = 0; i < curMesh->VN(); i++)
	{
		vcg::face::VFIterator<CFaceO> vfi(&curMesh->vert[i]);
		int I = 0;
		for (; !vfi.End(); ++vfi)
		{
			I++;
		}
		if (I == 0)
		{
			deleteVertexList.push_back(i);
		}
	}
	for (int i = 0; i < deleteVertexList.size(); i++)
	{
		vcg::tri::Allocator<CMeshO>::DeleteVertex(*curMesh, curMesh->vert[deleteVertexList[i]]);
	}

	vcg::tri::Allocator<CMeshO>::CompactFaceVector(*curMesh);
	vcg::tri::Allocator<CMeshO>::CompactVertexVector(*curMesh);

	// update topology
	assert(tri::HasFFAdjacency(*curMesh) == false);
	curMesh->face.EnableFFAdjacency();
	assert(tri::HasFFAdjacency(*curMesh) == true);

	assert(tri::HasVFAdjacency(*curMesh) == false);
	curMesh->vert.EnableVFAdjacency();
	curMesh->face.EnableVFAdjacency();
	assert(tri::HasVFAdjacency(*curMesh) == true);

	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*curMesh);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(*curMesh);

	// update Normals
	vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(*curMesh);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*curMesh);
}

void BooleanMachine::addAllFaces()
{
	//generateNewVertexsAndNewFaces(addingFaces);

	for (auto face : addingFaces)
	{
		//vcg::tri::Allocator<CMeshO>::AddFace(*A, face.vertP1, face.vertP2, face.vertP3);
		vcg::tri::Allocator<CMeshO>::AddFace(*A, face.v1, face.v2, face.v3);
	}
	int delVertNum = vcg::tri::Clean<CMeshO>::RemoveDegenerateVertex(*A);
	int delFaceNum = vcg::tri::Clean<CMeshO>::RemoveDegenerateFace(*A);
	tri::Clean<CMeshO>::RemoveDuplicateVertex(*A);// delete Redundant vertex data
	tri::Clean<CMeshO>::RemoveUnreferencedVertex(*A);

	vcg::tri::Allocator<CMeshO>::CompactEveryVector(*A);

	// update topology
	// update Normals
	vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(*A);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*A);
}

vector<int> BooleanMachine::addAllFaces(vector<FFace> faceList, CMeshO *curMesh)
{
	vector<int> newFaceindexes;
	CMeshO::FaceIterator facePointer;
	for (auto face : faceList)
	{
		facePointer = vcg::tri::Allocator<CMeshO>::AddFace(*curMesh, face.v1, face.v2, face.v3);
		facePointer->V(0)->C() = vcg::Color4b(174, 174, 174, 255);
		facePointer->V(1)->C() = vcg::Color4b(174, 174, 174, 255);
		facePointer->V(2)->C() = vcg::Color4b(174, 174, 174, 255);
		newFaceindexes.push_back(facePointer - curMesh->face.begin());
	}
	int delVertNum = vcg::tri::Clean<CMeshO>::RemoveDegenerateVertex(*curMesh);
	int delFaceNum = vcg::tri::Clean<CMeshO>::RemoveDegenerateFace(*curMesh);
	tri::Clean<CMeshO>::RemoveDuplicateVertex(*curMesh);// delete Redundant vertex data
	tri::Clean<CMeshO>::RemoveUnreferencedVertex(*curMesh);

	vcg::tri::Allocator<CMeshO>::CompactEveryVector(*curMesh);

	 //update topology
	assert(tri::HasPerVertexVFAdjacency(*curMesh) && tri::HasPerFaceVFAdjacency(*curMesh));
	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*curMesh);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(*curMesh);

	// update Normals
	vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(*curMesh);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*curMesh);

	return newFaceindexes;
}

void BooleanMachine::generateNewVertexsAndNewFaces(vector<FFace> &faceList)
{
	Point3f currentP;
	CVertexO *vertPointer1, *vertPointer2, *vertPointer3;
	for (int i = 0; i < faceList.size(); i++)
	{
		currentP = faceList[i].v1;
		vertPointer1 = addOrFindThisPointsPointer(currentP);

		currentP = faceList[i].v2;
		vertPointer2 = addOrFindThisPointsPointer(currentP);

		currentP = faceList[i].v3;
		vertPointer3 = addOrFindThisPointsPointer(currentP);

		faceList[i].InitVertexsPointers(vertPointer1, vertPointer2, vertPointer3);
	}

	//vector<Point3f> unRepeatVertList;
}

CVertexO* BooleanMachine::addOrFindThisPointsPointer(Point3f p)
{
	bool isNewVert = true;
	for (int i = 0; i < A->vert.size(); i++)
	{
		if (A->vert[i].P() == p)
		{
			isNewVert = false;
			return &A->vert[i];
		}
	}

	if (isNewVert)
	{
		CMeshO::VertexPointer newVertPointer;
		newVertPointer = &*vcg::tri::Allocator<CMeshO>::AddVertex(*A, p);
		vcg::tri::Allocator<CMeshO>::CompactVertexVector(*A);
		return newVertPointer;
	}
}

CVertexO* BooleanMachine::addOrFindThisPointsPointer(Point3f p, CMeshO *curMesh)
{
	bool isNewVert = true;
	for (int i = 0; i < curMesh->vert.size(); i++)
	{
		if (curMesh->vert[i].P() == p)
		{
			isNewVert = false;
			return &curMesh->vert[i];
		}
	}

	if (isNewVert)
	{
		CMeshO::VertexPointer newVertPointer;
		newVertPointer = &*vcg::tri::Allocator<CMeshO>::AddVertex(*curMesh, p);
		vcg::tri::Allocator<CMeshO>::CompactVertexVector(*curMesh);
		return newVertPointer;
	}
}

vector<int> BooleanMachine::classifyAccordingTopologyStruct(vector<int> faceList, vector<FFace> boundaryFaceList, CMeshO* pMesh)
{
	vector<int> result, ignoreFaces, waitVert;
	vector<int> newIgnoreFaces, newWaitVert;

	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*pMesh);

	//获得无重的初始边界面索引和边界点索引
	ignoreFaces = faceList;
	vector<int> vertWanted;
	for (auto index : faceList)
	{
		int vert0Index = pMesh->face[index].V(0) - &pMesh->vert[0];
		int vert1Index = pMesh->face[index].V(1) - &pMesh->vert[0];
		int vert2Index = pMesh->face[index].V(2) - &pMesh->vert[0];

		if (!this->exitInVector(vert0Index, vertWanted))
		{
			vertWanted.push_back(vert0Index);
		}

		if (!this->exitInVector(vert1Index, vertWanted))
		{
			vertWanted.push_back(vert1Index);
		}

		if (!this->exitInVector(vert2Index, vertWanted))
		{
			vertWanted.push_back(vert2Index);
		}
	}
	for (auto face : boundaryFaceList)
	{
		for (auto vertIndex : vertWanted)
		{
			if (face.v1 == pMesh->vert[vertIndex].P() && !exitInVector(vertIndex, waitVert))
			{
				waitVert.push_back(vertIndex);
				break;
			}
		}

		for (auto vertIndex : vertWanted)
		{
			if (face.v2 == pMesh->vert[vertIndex].P() && !exitInVector(vertIndex, waitVert))
			{
				waitVert.push_back(vertIndex);
				break;
			}
		}

		for (auto vertIndex : vertWanted)
		{
			if (face.v3 == pMesh->vert[vertIndex].P() && !exitInVector(vertIndex, waitVert))
			{
				waitVert.push_back(vertIndex);
				break;
			}
		}
	}
	vertWanted.clear();

	int i = 0;
	do {
		newWaitVert.clear();
		newIgnoreFaces.clear();
		for (auto vertIndex : waitVert)
		{
			//将vertIndex所指向的一点的一领域面中，不存在于ignoreFaces和newIgnoreFaces中的面筛选出来
			vector<int> tempFaceList;
			vcg::face::VFIterator<CFaceO> vfi(&pMesh->vert[vertIndex]);
			for (; !vfi.End(); ++vfi)
			{
				int faceIndex = vfi.F() - &pMesh->face[0];
				bool exited1 = this->exitInVector(faceIndex, ignoreFaces);
				bool exited2 = this->exitInVector(faceIndex, newIgnoreFaces);
				if (!exited1 && !exited2)
				{
					tempFaceList.push_back(faceIndex);
				}
			}

			//对每个考虑的面的三个顶点，其索引既不属于已经检查过的点的加入到newWaitVert中
			for (auto index : tempFaceList)
			{
				int vert0Index = pMesh->face[index].V(0) - &pMesh->vert[0];
				int vert1Index = pMesh->face[index].V(1) - &pMesh->vert[0];
				int vert2Index = pMesh->face[index].V(2) - &pMesh->vert[0];

				if (!exitInVector(vert0Index, waitVert) && !exitInVector(vert0Index, newWaitVert))
				{
					newWaitVert.push_back(vert0Index);
				}
				if (!exitInVector(vert1Index, waitVert) && !exitInVector(vert1Index, newWaitVert))
				{
					newWaitVert.push_back(vert1Index);
				}
				if (!exitInVector(vert2Index, waitVert) && !exitInVector(vert2Index, newWaitVert))
				{
					newWaitVert.push_back(vert2Index);
				}
			}

			//将一点筛选出的tempList面加入到Result和newIgnoreFaces中
			for (auto index : tempFaceList)
			{
				result.push_back(index);
				newIgnoreFaces.push_back(index);
			}
		}

		waitVert.clear();
		waitVert = newWaitVert;
		for (auto index : newIgnoreFaces)
		{
			ignoreFaces.push_back(index);
		}

		i++;
	} while (!newIgnoreFaces.empty());
	//i < 6
	return result;
}

template<typename T> bool  BooleanMachine::exitInVector(T t, vector<T> list)
{
	for (auto temp : list)
	{
		if (t == temp)
		{
			return true;
		}
	}
	return false;
}

template<typename T> void BooleanMachine::putInVector(T t, vector<T> &list)
{
	bool exit = false;
	for (auto temp : list)
	{
		if (t == temp)
		{
			return;
		}
	}
	list.push_back(t);
	return;
}

void BooleanMachine::getIgnoreSplitFaceList(vector<Point2m> ignoreCP)
{
	this->ignoreSplitFace_A.clear();
	this->ignoreSplitFace_B.clear();

	bool exitedA, exitedB;
	for (auto cp : ignoreCP)
	{
		exitedA = false;
		for (auto index : this->ignoreSplitFace_A)
		{
			if (index == cp.X())
			{
				exitedA = true;
				break;
			}
		}
		if (!exitedA)
		{
			this->ignoreSplitFace_A.push_back(cp.X());
		}

		exitedB = false;
		for (auto index : this->ignoreSplitFace_B)
		{
			if (index == cp.Y())
			{
				exitedB = true;
				break;
			}
		}
		if (!exitedB)
		{
			this->ignoreSplitFace_B.push_back(cp.Y());
		}
	}
	ClearAllVectorObject();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BooleanMachine::BooleanComputeSortedLoop(CMeshO *_a, CMeshO *_b, int iComputeKind, vector<Point2m> Couple)
{

}

vector<WordCircle> BooleanMachine::getOppositeOrderLoops(vector<WordCircle> loops, CMeshO *curMesh)
{
	vector<WordCircle> result;

	result = loops;
	for (int i = 0; i < result.size(); ++i)
	{
		result[i].putUpsideDown();
	}

	for (int i = 0; i < result.size(); ++i)
	{
		for (int j = 0; j < result[i].edgeList.size(); ++j)
		{
			CVertexO *vertPointerA = this->addOrFindThisPointsPointer(result[i].edgeList[j].vertA, curMesh);
			result[i].edgeList[j].indexA = vertPointerA - &curMesh->vert[0];
			CVertexO *vertPointerB = this->addOrFindThisPointsPointer(result[i].edgeList[j].vertB, curMesh);
			result[i].edgeList[j].indexB = vertPointerB - &curMesh->vert[0];
		}
	}

	return  result;
}

void BooleanMachine::distinguishOutsideAndInsideFaces(vector<WordCircle> loops, CMeshO *curMesh, vector<int> boundFaces, vector<int> &outsideList, vector<int> &insideList)
{
	vector<int> outsideFaces;
	for (auto loop : loops)
	{
		vector<int> tempList = searchAllTrianglesConjugateLoop(loop, boundFaces, curMesh);
		for (auto index : tempList)
		{
			outsideFaces.push_back(index);
		}
	}

	outsideList = this->searchConnectedFacesByTopology(curMesh, outsideFaces, loops, insideList);
	return;
}

void BooleanMachine::distinguishOutsideAndInsideFaces(WordCircle loop, CMeshO *curMesh, vector<int> boundFaces, vector<int> &outsideList, vector<int> &insideList)
{
	vector<int> outsideFaces;
	vector<int> tempList = searchAllTrianglesConjugateLoop(loop, boundFaces, curMesh);
	for (auto index : tempList)
	{
		outsideFaces.push_back(index);
	}
	vector<WordCircle> tempLoops;
	tempLoops.push_back(loop);

	if (outsideFaces.size() == loop.edgeList.size())
	{
		outsideList = this->searchConnectedFacesByTopology(curMesh, outsideFaces, tempLoops, insideList);
	}
	else
	{
		outsideList = outsideFaces;
		insideList.clear();
	}

	return;
}

vector<int> BooleanMachine::searchAllTrianglesConjugateLoop(WordCircle loop, vector<int> boundFaces, CMeshO *curMesh)
{
	vector<int> result;
	WordEdge curEdge;
	int indexA, indexB;
	int iv0, iv1, iv2;
	Point3m testVert, tempVector, edgeVector;

	for (int j = 0; j < loop.edgeList.size(); ++j)
	{
		bool findWanted;
		curEdge = loop.edgeList[j];
		Point3i tempCP(-1, -1, j);

		findWanted = false;
		for (auto i : boundFaces)
		{
			iv0 = curMesh->face[i].V(0) - &curMesh->vert[0];
			iv1 = curMesh->face[i].V(1) - &curMesh->vert[0];
			iv2 = curMesh->face[i].V(2) - &curMesh->vert[0];

			if (curEdge.indexA == iv1 && curEdge.indexB == iv0)
			{
				findWanted = true;
				result.push_back(i);
				break;
			}
			else if (curEdge.indexA == iv2 && curEdge.indexB == iv1)
			{
				findWanted = true;
				result.push_back(i);
				break;
			}
			else if (curEdge.indexA == iv0 && curEdge.indexB == iv2)
			{
				findWanted = true;
				result.push_back(i);
				break;
			}
		}
		if (!findWanted)
		{
			int stop = 1;
		}
	}

	return result;

	/*vector<Point3i> cpList;
	for (int j = 0; j < loop.edgeList.size(); ++j)
	{
		bool findWanted;
		curEdge = loop.edgeList[j];
		Point3i tempCP(-1, -1, j);

		findWanted = false;
		for (auto i : boundFaces)
		{
			iv0 = curMesh->face[i].V(0) - &curMesh->vert[0];
			iv1 = curMesh->face[i].V(1) - &curMesh->vert[0];
			iv2 = curMesh->face[i].V(2) - &curMesh->vert[0];

			if (curEdge.indexA == iv0 && curEdge.indexB == iv1)
			{
				indexA = i;
				findWanted = true;
				break;
			}
			else if (curEdge.indexA == iv1 && curEdge.indexB == iv0)
			{
				indexA = i;
				findWanted = true;
				break;
			}

			if (curEdge.indexA == iv1 && curEdge.indexB == iv2)
			{
				indexA = i;
				findWanted = true;
				break;
			}
			else if (curEdge.indexA == iv2 && curEdge.indexB == iv1)
			{
				indexA = i;
				findWanted = true;
				break;
			}

			if (curEdge.indexA == iv2 && curEdge.indexB == iv0)
			{
				indexA = i;
				findWanted = true;
				break;
			}
			else if (curEdge.indexA == iv0 && curEdge.indexB == iv2)
			{
				indexA = i;
				findWanted = true;
				break;
			}
		}
		if (findWanted)
		{
			tempCP.X() = indexA;
		}

		findWanted = false;
		for (auto i : boundFaces)
		{
			iv0 = curMesh->face[i].V(0) - &curMesh->vert[0];
			iv1 = curMesh->face[i].V(1) - &curMesh->vert[0];
			iv2 = curMesh->face[i].V(2) - &curMesh->vert[0];

			if (curEdge.indexA == iv0 && curEdge.indexB == iv1 && i != indexA)
			{
				indexB = i;
				findWanted = true;
				break;
			}
			else if (curEdge.indexA == iv1 && curEdge.indexB == iv0  && i != indexA)
			{
				indexB = i;
				findWanted = true;
				break;
			}

			if (curEdge.indexA == iv1 && curEdge.indexB == iv2 && i != indexA)
			{
				indexB = i;
				findWanted = true;
				break;
			}
			else if (curEdge.indexA == iv2 && curEdge.indexB == iv1 && i != indexA)
			{
				indexB = i;
				findWanted = true;
				break;
			}

			if (curEdge.indexA == iv2 && curEdge.indexB == iv0 && i != indexA)
			{
				indexB = i;
				findWanted = true;
				break;
			}
			else if (curEdge.indexA == iv0 && curEdge.indexB == iv2 && i != indexA)
			{
				indexB = i;
				findWanted = true;
				break;
			}
		}
		if (findWanted)
		{
			tempCP.Y() = indexB;
		}
		cpList.push_back(tempCP);
	}

	//int I = 0;
	bool wanted;
	for (auto cp : cpList)
	{
		if (cp.X() != -1 && cp.Y() != -1)
		{
			int i = cp.X();
			bool sameDirection = false;
			if (curMesh->face[i].V(0)->P() == loop.edgeList[cp.Z()].vertA && curMesh->face[i].V(1)->P() == loop.edgeList[cp.Z()].vertB)
			{
				sameDirection = true;
			}
			else if (curMesh->face[i].V(1)->P() == loop.edgeList[cp.Z()].vertA && curMesh->face[i].V(2)->P() == loop.edgeList[cp.Z()].vertB)
			{
				sameDirection = true;
			}
			else if (curMesh->face[i].V(2)->P() == loop.edgeList[cp.Z()].vertA && curMesh->face[i].V(0)->P() == loop.edgeList[cp.Z()].vertB)
			{
				sameDirection = true;
			}
			if (sameDirection)
			{
				result.push_back(cp.X());
			}
			else
			{
				result.push_back(cp.Y());
			}
			//else
			//else
		}
		//++I;
	}
	return result;*/
}

vector<int> BooleanMachine::searchConnectedFacesByTopology(CMeshO* curMesh, vector<int> startFaceIndexes, vector<WordCircle> boundaryLoops, vector<int> &insideList)
{
	// update topology
	assert(tri::HasFFAdjacency(*curMesh) == false);
	curMesh->face.EnableFFAdjacency();
	assert(tri::HasFFAdjacency(*curMesh) == true);

	assert(tri::HasVFAdjacency(*curMesh) == false);
	curMesh->vert.EnableVFAdjacency();
	curMesh->face.EnableVFAdjacency();
	assert(tri::HasVFAdjacency(*curMesh) == true);
	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*curMesh);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(*curMesh);

	vector<int> result;
	result = startFaceIndexes;

	vector<int> newWantedFaces;
	vector<int> curSearchFaces;
	vector<Point2i> checkedEdges;
	vector<Point2i> boundaryEdges;
	for (auto loop : boundaryLoops)
	{
		for (auto edge : loop.edgeList)
		{
			boundaryEdges.push_back(Point2i(edge.indexB, edge.indexA));
		}
	}

	vector<bool> viewedFaces;
	for (int i = 0; i < curMesh->face.size(); ++i)
	{
		viewedFaces.push_back(false);
	}

	//初始化
	//int iTime = 0;
	bool continueSearch = false;
	int iv0, iv1, iv2, tempIndex;
	Point3f p0, p1, p2;
	curSearchFaces = startFaceIndexes;
	for (auto index : curSearchFaces)
	{
		viewedFaces[index] = true;
	}
	do {
		for (auto curFace : curSearchFaces)
		{
			////test...
			iv0 = curMesh->face[curFace].V(0) - &curMesh->vert[0];
			iv1 = curMesh->face[curFace].V(1) - &curMesh->vert[0];
			iv2 = curMesh->face[curFace].V(2) - &curMesh->vert[0];
			p0 = curMesh->face[curFace].V(0)->P();
			p1 = curMesh->face[curFace].V(1)->P();
			p2 = curMesh->face[curFace].V(2)->P();

			Point2i edge0(iv0, iv1);
			tempIndex = getConnectedFaceByEdge(curMesh, curFace, 0, edge0, boundaryEdges, checkedEdges);
			////test...
			if (tempIndex != -1 && tempIndex != curFace && !viewedFaces[tempIndex])
			{
				newWantedFaces.push_back(tempIndex);
				viewedFaces[tempIndex] = true;
			}

			Point2i edge1(iv1, iv2);
			tempIndex = getConnectedFaceByEdge(curMesh, curFace, 1, edge1, boundaryEdges, checkedEdges);
			////test...
			if (tempIndex != -1 && tempIndex != curFace && !viewedFaces[tempIndex])
			{
				newWantedFaces.push_back(tempIndex);
				viewedFaces[tempIndex] = true;
			}

			Point2i edge2(iv2, iv0);
			tempIndex = getConnectedFaceByEdge(curMesh, curFace, 2, edge2, boundaryEdges, checkedEdges);
			////test...
			if (tempIndex != -1 && tempIndex != curFace && !viewedFaces[tempIndex])
			{
				newWantedFaces.push_back(tempIndex);
				viewedFaces[tempIndex] = true;
			}

			/*checkedEdges.push_back(edge0);
			checkedEdges.push_back(edge1);
			checkedEdges.push_back(edge2);*/
		}

		if (!newWantedFaces.empty())
		{
			continueSearch = true;
			for (auto index : newWantedFaces)
			{
				result.push_back(index);
			}
			curSearchFaces.clear();
			curSearchFaces = newWantedFaces;
			newWantedFaces.clear();
		}
		else
		{
			curSearchFaces.clear();
			continueSearch = false;
		}
		//iTime++;
	}//while (iTime < 1);
	 while (continueSearch);

	for (int i = 0; i < curMesh->face.size(); ++i)
	{
		if (viewedFaces[i] == false)
		{
			insideList.push_back(i);
		}
	}

	viewedFaces.clear();
	return result;
}

int BooleanMachine::getConnectedFaceByEdge(CMeshO* curMesh, int faceIndex, int edgeSign, Point2i edgeIndexes, const vector<Point2i> boundays, const vector<Point2i> checkedEdges)
{
	bool curEdgeIsBorder, curEdgeHaveChecked;
	curEdgeIsBorder = this->exitInVector<Point2i>(edgeIndexes, boundays);
	if (curEdgeIsBorder)
	{
		return -1;
	}
	/*curEdgeHaveChecked = this->exitInVector<Point2i>(edgeIndexes, checkedEdges);
	if (curEdgeHaveChecked)
	{
		return -1;
	}*/

	int result;
	result = curMesh->face[faceIndex].FFp(edgeSign) - &curMesh->face[0];

	////test...
	return result;
}

void BooleanMachine::WhatBooleanModelYouWant3(int iComputeKind)
{
	CMeshO *curMesh, *needClearMesh;
	switch (iComputeKind)
	{
	case AunitB:
	case AminusB:
	case AintersectB:
		curMesh = this->A;
		needClearMesh = this->B;
		break;

	case BminsA:
		curMesh = this->B;
		needClearMesh = this->A;
		break;

	default:
		break;
	}

	this->generateAddingFaces2(iComputeKind);

	vector<int> deleteFaceIndexes;
	switch (iComputeKind)
	{
	case AunitB:
		deleteFaceIndexes = this->insideA;
		break;

	case AminusB:
		deleteFaceIndexes = this->insideA;
		break;

	case BminsA:
		deleteFaceIndexes = this->insideB;
		break;

	case AintersectB:
		deleteFaceIndexes = this->outsideA;
		break;

	default:
		break;
	}

	this->deleteFacesInList(deleteFaceIndexes, curMesh);
	this->addAllFaces(this->addingFaces, curMesh);
	needClearMesh->Clear();
}

void BooleanMachine::generateAddingFaces2(int iKind)
{
	this->addingFaces.clear();
	if (!this->BO2Dresult.empty())
	{
		this->addingFaces = this->BO2Dresult;
	}

	switch (iKind)
	{
	case AunitB:
		for (auto i : this->outsideB)
		{
			addingFaces.push_back(FFace(B->face[i].V(0)->P(), B->face[i].V(1)->P(), B->face[i].V(2)->P()));
		}
		break;

	case AminsB:
		for (auto i : this->insideB)
		{
			addingFaces.push_back(FFace(B->face[i].V(0)->P(), B->face[i].V(2)->P(), B->face[i].V(1)->P()));
		}
		break;

	case BminsA:
		for (auto i : this->insideA)
		{
			addingFaces.push_back(FFace(A->face[i].V(0)->P(), A->face[i].V(2)->P(), A->face[i].V(1)->P()));
		}
		break;

	case AintersectB:
		for (auto i : this->insideB)
		{
			addingFaces.push_back(FFace(B->face[i].V(0)->P(), B->face[i].V(1)->P(), B->face[i].V(2)->P()));
		}
		break;

	default:
		break;
	}

}

void BooleanMachine::initalIntersectInfos(const vector<DefiniteIntersection> interVertsA, const vector<DefiniteIntersection> interVertsB)
{
	this->interRecords.clear();
	DefiniteIntersection tempVertA, tempVertB;
	for (int i = 0; i < interVertsA.size(); ++i)
	{
		tempVertA = interVertsA[i];
		bool findCPVert = false;
		for (int j = 0; j < interVertsB.size(); ++j)
		{
			if (tempVertA.intersection == interVertsB[j].intersection)
			{
				findCPVert = true;
				tempVertB = interVertsB[j];
			}
		}
		if (!findCPVert)
		{
			continue;
		}

		vector<IntersectTestRecord> tempRecords;
		tempRecords = collectIntersectInfos(tempVertA, tempVertB);
		for (int j = 0; j < tempRecords.size(); ++j)
		{
			this->interRecords.push_back(tempRecords[j]);
		}
		tempRecords.clear();
	}
}

vector<IntersectTestRecord> BooleanMachine::collectIntersectInfos(DefiniteIntersection vertA, DefiniteIntersection vertB)
{
	vector<IntersectTestRecord> result;

	CMeshO *meshA, *meshB;
	meshA = vertA.meshPointer;
	meshB = vertB.meshPointer;
	Point3f intersection = vertA.intersection;

	for (int i = 0; i < vertA.edgeList.size(); ++i)
	{
		for (auto faceIndex : vertB.faceList)
		{
			result.push_back(IntersectTestRecord(meshB, faceIndex, vertA.edgeList[i], intersection));
		}
	}
	for (int i = 0; i < vertB.edgeList.size(); ++i)
	{
		for (auto faceIndex : vertA.faceList)
		{
			result.push_back(IntersectTestRecord(meshA, faceIndex, vertB.edgeList[i], intersection));
		}
	}

	return result;
}

Point3m BooleanMachine::getLocalSysPoint(Point3m vert, Point3m axisX, Point3m axisY, Point3m axisZ, Point3m center)
{
	Point3m edge;
	float x, y, z;
	edge = vert - center;
	x = edge * axisX;
	y = edge * axisY;
	z = edge * axisZ;
	return Point3m(x, y, z);
}

Point3m BooleanMachine::releaseLocalSysPoint(Point3m vert, Point3m axisX, Point3m axisY, Point3m axisZ, Point3m center)
{
	return center + axisX * vert.X() + axisY * vert.Y() + axisZ * vert.Z();
}

bool BooleanMachine::getAHProjIntersection(Point3m &p0, Point3m &p1, FEdge limitEdge, FFace projTriangle, FFace localTriangle)
{
	bool projEdge_1_Intersected = false, projEdge_2_Intersected = false, projEdge_3_Intersected = false;

	//对projTriangle的三边构成的线段与求y=0时的零点，当零点处于limitEdge范围且在segment范围之内时表示有交点产生
	//为了防止同点产生重复交叠，ti(i = 1, 2, 3)属于(0, 1]
	float t1 = -1, t2 = -1, t3 = -1;
	projEdge_1_Intersected = computeZeroPointOnLineSegment(t1, limitEdge.vertA.X(), limitEdge.vertB.X(), FEdge(projTriangle.v1, projTriangle.v2));
	projEdge_2_Intersected = computeZeroPointOnLineSegment(t2, limitEdge.vertA.X(), limitEdge.vertB.X(), FEdge(projTriangle.v2, projTriangle.v3));
	projEdge_3_Intersected = computeZeroPointOnLineSegment(t3, limitEdge.vertA.X(), limitEdge.vertB.X(), FEdge(projTriangle.v3, projTriangle.v1));

	//若三边均无交点则没有面交叉发生；返回false
	if (!projEdge_1_Intersected && !projEdge_2_Intersected && !projEdge_3_Intersected)
	{
		return false;
	}

	//若有且仅有两条边产生交点,则根据得到比例在localTriangle上复原交叉点
	if (projEdge_1_Intersected && projEdge_2_Intersected && !projEdge_3_Intersected)
	{
		p0 = localTriangle.v1 + (localTriangle.v2 - localTriangle.v1) * t1;
		p1 = localTriangle.v2 + (localTriangle.v3 - localTriangle.v2) * t2;
		return true;
	}
	else if (projEdge_1_Intersected && !projEdge_2_Intersected && projEdge_3_Intersected)
	{
		p0 = localTriangle.v1 + (localTriangle.v2 - localTriangle.v1) * t1;
		p1 = localTriangle.v3 + (localTriangle.v1 - localTriangle.v3) * t3;
		return true;
	}
	else if (!projEdge_1_Intersected && projEdge_2_Intersected && projEdge_3_Intersected)
	{
		p0 = localTriangle.v2 + (localTriangle.v3 - localTriangle.v2) * t2;
		p1 = localTriangle.v3 + (localTriangle.v1 - localTriangle.v3) * t3;
		return true;
	}

	//若有且仅有一条边产生交点，则在该边上复原localTriangle的一个交叉点。
	//在其他两条边上选取对应t值不为-1的边，使用该边的t值复原在localTriangle上的点，作为交叉点导出。
	if (projEdge_1_Intersected && !projEdge_2_Intersected && !projEdge_3_Intersected)
	{
		p0 = localTriangle.v1 + (localTriangle.v2 - localTriangle.v1) * t1;
		if (t2 != -1)
		{
			p1 = localTriangle.v2 + (localTriangle.v3 - localTriangle.v2) * t2;
			return true;
		}
		else if (t3 != -1)
		{
			p1 = localTriangle.v3 + (localTriangle.v1 - localTriangle.v3) * t3;
			return true;
		}
	}
	else if (!projEdge_1_Intersected && projEdge_2_Intersected && !projEdge_3_Intersected)
	{
		p0 = localTriangle.v2 + (localTriangle.v3 - localTriangle.v2) * t2;
		if (t1 != -1)
		{
			p1 = localTriangle.v1 + (localTriangle.v2 - localTriangle.v1) * t1;
			return true;
		}
		else if (t3 != -1)
		{
			p1 = localTriangle.v3 + (localTriangle.v1 - localTriangle.v3) * t3;
			return true;
		}
	}
	else if (!projEdge_1_Intersected && !projEdge_2_Intersected && projEdge_3_Intersected)
	{
		p0 = localTriangle.v3 + (localTriangle.v1 - localTriangle.v3) * t3;
		if (t1 != -1)
		{
			p1 = localTriangle.v1 + (localTriangle.v2 - localTriangle.v1) * t1;
			return true;
		}
		else if (t2 != -1)
		{
			p1 = localTriangle.v2 + (localTriangle.v3 - localTriangle.v2) * t2;
			return true;
		}
	}
	return false;
}

bool BooleanMachine::getAVProjIntersection(Point3m &p0, Point3m &p1, FEdge edge, FFace localTriangle)
{
	Point3m tempA, tempB;
	tempA = Point3m(edge.vertA.X(), edge.vertA.Z(), 0);
	tempB = Point3m(edge.vertB.X(), edge.vertB.Z(), 0);
	FEdge aimEdge(tempA, tempB);
	Point3m tempVert1, tempVert2, tempVert3;
	tempVert1 = Point3m(localTriangle.v1.X(), localTriangle.v1.Z(), 0);
	tempVert2 = Point3m(localTriangle.v2.X(), localTriangle.v2.Z(), 0);
	tempVert3 = Point3m(localTriangle.v3.X(), localTriangle.v3.Z(), 0);
	FFace aimTriangle(tempVert1, tempVert2, tempVert3);
	//测算edge与localTriangle三边的交点
	//记录交点个数和交点坐标
	vector<Point3m> intersections;
	aimTriangle.getIntersectionWithEdgeOn2DSpace(aimEdge, intersections);

	if (intersections.size() == 0)
	{
		if (aimTriangle.ifVertInsideTriangleOn2DSpace(aimEdge.vertA))
		{
			p0 = aimEdge.vertA;
			p1 = aimEdge.vertB;
			return true;
		}
		else
		{
			return false;
		}
	}

	if (intersections.size() == 1)
	{
		if (aimTriangle.ifVertInsideTriangleOn2DSpace(aimEdge.vertA))
		{
			p0 = aimEdge.vertA;
			p1 = intersections[0];
			return true;
		}
		else
		{
			p0 = aimEdge.vertB;
			p1 = intersections[0];
			return true;
		}
	}

	if (intersections.size() == 2)
	{
		p0 = intersections[0];
		p1 = intersections[1];
		return true;
	}

	return false;
}

bool BooleanMachine::computeZeroPointOnLineSegment(float &t, float minXLimit, float maxXLimit, FEdge edge)
{
	if (edge.vertA.Y() * edge.vertB.Y() > 0)
	{
		t = -1;
		return false;
	}
	float absYA, absYB;
	absYA = abs(edge.vertA.Y());
	absYB = abs(edge.vertB.Y());
	t = absYA / (absYA + absYB);

	float zeroX = edge.vertA.X() + (edge.vertB.X() - edge.vertA.X()) * t;

	if (minXLimit <= zeroX && zeroX <= maxXLimit)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int BooleanMachine::quickFindAimRecord(IntersectTestRecord curR, vector<IntersectTestRecord> recordList)
{
	if (recordList.empty())
	{
		return -1;
	}

	int front, end, mid, x, i, N;
	x = curR.iFace;
	N = recordList.size();

	front = 0;
	end = N - 1;
	mid = (front + end) / 2;
	while (front < end && recordList[mid].iFace != x)
	{
		if (recordList[mid].iFace < x)
		{
			front = mid + 1;
		}
		if (recordList[mid].iFace > x)
		{
			end = mid - 1;
		}
		mid = (front + end) / 2;
	}
	if (recordList[mid].iFace != x)
	{
		return -1;
	}
	else
	{
		return mid + 1;
	}

	return -1;
}

void BooleanMachine::quickInsertAimRecord(IntersectTestRecord curR, vector<IntersectTestRecord> &recordList)
{
	recordList.push_back(curR);

	int i, j, length;
	length = recordList.size();
	IntersectTestRecord temp;
	for (i = length - 1; 0 < i; i--) {
		for (j = 0; j < i; j++) {
			if (recordList[j].iFace > recordList[j + 1].iFace) {
				temp = recordList[j];
				recordList[j] = recordList[j + 1];
				recordList[j + 1] = temp;
			}
		}
	}
}
