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

#ifndef COMMON_BOOLEANMACHINE_H
#define COMMON_BOOLEANMACHINE_H

#include "common/ml_mesh_type.h"
#include "machine/delaunaymachine.h"
#include "util/mesh_bounding_box.h"
#include "machine/boolean2Dmachine.h"
#include "util/definite_intersection.h"
#include "machine/triangleintersectmachine.h"

#include "common_ext_global.h"

using namespace std;

const int AunitB = 1;
const int AminsB = 2;
const int BminsA = 3;
const int AintersectB = 4;

struct COMMON_EXT_EXPORT TrackableVert
{
public:
	TrackableVert() {}
	TrackableVert(Point3m _vert, int _iEdge, CMeshO *_source)
	{
		vert = _vert;
		iEdge = _iEdge;
		sourceModel = _source;
	}

public:
	Point3m vert;
	int iEdge;
	float fDistance;
	CMeshO *sourceModel;
};

struct COMMON_EXT_EXPORT SplitTriangle {
public:
	int index;
	CMeshO *pMesh;
	Point3m v1, v2, v3;
	vector<Point3f> splitPoints;
	vector<TrackableVert> splitTrackPoints;
	vector<FEdge> splitEdges;
	vector<FFace> splitResults;

	SplitTriangle() {}
	SplitTriangle(int _index, CMeshO *_pmesh, vector<Point3f> points, FEdge edge)
	{
		this->index = _index;
		this->pMesh = _pmesh;
		//splitPoints = points;
		for (int i = 0; i < points.size(); i++)
			splitPoints.push_back(points[i]);
		this->splitEdges.push_back(edge);
	}

	SplitTriangle(int _index, CMeshO *_pmesh, vector<Point3f> points, vector<TrackableVert> trackVerts, FEdge edge)
	{
		this->index = _index;
		this->pMesh = _pmesh;
		//splitPoints = points;
		for (int i = 0; i < points.size(); i++)
		{
			splitPoints.push_back(points[i]);
		}
		for (int i = 0; i < trackVerts.size(); i++)
		{
			splitTrackPoints.push_back(trackVerts[i]);
		}
		if (points.size() == 2)
		{
			this->splitEdges.push_back(edge);
		}
	}

	void Inital(int _index, CMeshO *_pmesh, vector<Point3f> points)
	{
		this->index = _index;
		this->pMesh = _pmesh;
		for (int i = 0; i < points.size(); i++)
			splitPoints.push_back(points[i]);
	}

	void collectEdgeCombed()
	{
		Point3m tempv1, tempv2, tempv3;
		tempv1 = this->pMesh->face[this->index].V(0)->P();
		tempv2 = this->pMesh->face[this->index].V(1)->P();
		tempv3 = this->pMesh->face[this->index].V(2)->P();

		vector<TrackableVert> tempSplitNodes;
		for (auto node : this->splitTrackPoints)
		{
			if (node.iEdge == 0)
			{
				tempSplitNodes.push_back(node);
			}
		}
		if (tempSplitNodes.size() > 0)
		{
			this->splitThreeEdges(0, tempSplitNodes);
		}
		else
		{
			this->splitEdges.push_back(FEdge(tempv1, tempv2));
		}
		tempSplitNodes.clear();

		for (auto node : this->splitTrackPoints)
		{
			if (node.iEdge == 1)
			{
				tempSplitNodes.push_back(node);
			}
		}
		if (tempSplitNodes.size() > 0)
		{
			this->splitThreeEdges(1, tempSplitNodes);
		}
		else
		{
			this->splitEdges.push_back(FEdge(tempv2, tempv3));
		}
		tempSplitNodes.clear();

		for (auto node : this->splitTrackPoints)
		{
			if (node.iEdge == 2)
			{
				tempSplitNodes.push_back(node);
			}
		}
		if (!tempSplitNodes.empty())
		{
			this->splitThreeEdges(2, tempSplitNodes);
		}
		else
		{
			this->splitEdges.push_back(FEdge(tempv3, tempv1));
		}
		tempSplitNodes.clear();
	}

	void splitThreeEdges(int iSign, vector<TrackableVert> nodes)
	{
		FEdge border;
		Point3m  standardVector;
		Point3m tempv1, tempv2, tempv3;
		tempv1 = this->pMesh->face[this->index].V(0)->P();
		tempv2 = this->pMesh->face[this->index].V(1)->P();
		tempv3 = this->pMesh->face[this->index].V(2)->P();
		switch (iSign)
		{
		case 0:
			border = FEdge(tempv1, tempv2);
			standardVector = tempv2 - tempv1;
			break;

		case 1:
			border = FEdge(tempv2, tempv3);
			standardVector = tempv3 - tempv2;
			break;

		case 2:
			border = FEdge(tempv3, tempv1);
			standardVector = tempv1 - tempv3;
			break;

		default:
			break;
		}

		for (int i = 0; i < nodes.size(); i++)
		{
			Point3m tempVector;
			tempVector = nodes[i].vert - border.vertA;
			nodes[i].fDistance = tempVector * standardVector;
		}

		int i, j;
		TrackableVert temp;
		for (i = nodes.size() - 1; 0 < i; i--)
		{
			for (j = 0; j < i; j++) {
				if (nodes[j].fDistance > nodes[j + 1].fDistance)
				{
					temp = nodes[j];
					nodes[j] = nodes[j + 1];
					nodes[j + 1] = temp;
				}
			}
		}

		this->splitEdges.push_back(FEdge(border.vertA, nodes[0].vert));
		for (int i = 0; i < nodes.size() - 1; ++i)
		{
			this->splitEdges.push_back(FEdge(nodes[i].vert, nodes[i + 1].vert));
		}
		this->splitEdges.push_back(FEdge(nodes[nodes.size() - 1].vert, border.vertB));
	}
};

struct COMMON_EXT_EXPORT IntersectTestRecord
{
public:
	IntersectTestRecord() {}
	IntersectTestRecord(CMeshO *_mesh, int _face, FEdge _edge, Point3m _intersection)
	{
		this->mesh = _mesh;
		this->iFace = _face;
		this->edge = _edge;
		this->intersection = _intersection;
	}
	IntersectTestRecord(CMeshO *_mesh, int _face, FEdge _edge)
	{
		this->mesh = _mesh;
		this->iFace = _face;
		this->edge = _edge;
		this->intersection = Point3m(0, 0, 0);
	}

	bool sameAs(IntersectTestRecord b)
	{
		if (this->iFace == b.iFace && this->mesh == b.mesh)
		{
			if (this->edge.vertA == b.edge.vertA && this->edge.vertB == b.edge.vertB)
			{
				return true;
			}
			else if (this->edge.vertA == b.edge.vertB && this->edge.vertB == b.edge.vertA)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	int findSameEdgeSegment(IntersectTestRecord b)
	{
		if (this->iFace == b.iFace && this->mesh == b.mesh)
		{
			for (int i = 0; i < this->edgeList.size(); ++i)
			{
				if (this->edgeList[i].vertA == b.edge.vertA && this->edgeList[i].vertB == b.edge.vertB)
				{
					return i;
				}
				else if (this->edgeList[i].vertA == b.edge.vertB && this->edgeList[i].vertB == b.edge.vertA)
				{
					return i;
				}
			}

			return -1;
		}
		else
		{
			return -1;
		}
	}
public:
	CMeshO *mesh;
	int iFace;
	FEdge edge;
	Point3m intersection;

	vector<FEdge> edgeList;
	vector<Point3m> intersectionList;
};

class COMMON_EXT_EXPORT BooleanMachine
{
public:
	BooleanMachine();
	~BooleanMachine();

	enum { AnB, AuB, AminusB, BminusA };

public:
	void BooleanCompute(CMeshO *_a, CMeshO *_b, int iComputeKind, int** _collideMatrix, int _rowNumber, int _columnNumber);///布尔运算起始函数
	void BooleanCompute(CMeshO *_a, CMeshO *_b, int iComputeKind, vector<Point2m> cp);///布尔运算起始函数2
	vector<Point2i> intersectEdgesCompute(CMeshO *_a, CMeshO *_b, int** _collideMatrix, int _rowNumber, int _columnNumber);
	bool TwoTriangleInsertCompute(int indexA, int indexB, CMeshO* meshA, CMeshO* meshB, vector<Point3f> &intersectVerts, vector<TrackableVert> &intersectTrackVerts, FEdge &intersectEdge);
	bool TwoTriangleInsertCompute2(int indexA, int indexB, CMeshO* meshA, CMeshO* meshB, vector<Point3f> &intersectVerts, vector<TrackableVert> &intersectTrackVerts, FEdge &intersectEdge);
	bool TwoTriangleInsertCompute3(int indexA, int indexB, CMeshO* meshA, CMeshO* meshB, vector<Point3f> &intersectVerts, vector<TrackableVert> &intersectTrackVerts, FEdge &intersectEdge);
	bool TwoTriangleInsertComputeProj(int indexA, int indexB, CMeshO* meshA, CMeshO* meshB, vector<Point3f> &intersectVerts, vector<TrackableVert> &intersectTrackVerts, FEdge &intersectEdge);
	void AnalysisCollideMatrix(int** &cM, int &rowNum, int columnNum, CMeshO* A, CMeshO* B, vector<SplitTriangle> &splitFacesA, vector<SplitTriangle> &splitFacesB);
	void AddSplitTriangleToVector(int index, CMeshO *pMesh, vector<Point3f> newSplitPoints, vector<TrackableVert> intersectTrackVerts, FEdge newSplitEdge, vector<SplitTriangle> &splitTriangles);
	vector<FFace> SplitEachTriangleInVector2(vector<SplitTriangle> &faceList);
	vector<FFace> SplitOneTriangleFace2(SplitTriangle face);
	void ClassifyFacesSpace(CMeshO* pMesh, CMeshO* pAimMesh, vector<int> &IN_NS, vector<int> &OUT_NS, vector<SplitTriangle> ignoreFace);
	void ClassifyFacesSpace(CMeshO* pMesh, CMeshO* pAimMesh, vector<int> &IN_NS, vector<int> &OUT_NS, vector<int> ignoreIndexList);
	void ClassifyWaitingFacesSpace(vector<int> waitingface, CMeshO* pMesh, CMeshO* pAimMesh, vector<int> &IN_NS, vector<int> &OUT_NS, vector<SplitTriangle> ignoreFace);
	bool judgeOnFaceSpace(int faceIndex, CMeshO* pMesh, CMeshO* pAimMesh, vector<FFace>& filterFaceList);
	vector<int> classifyAccordingTopologyStruct(vector<int> faceList, vector<FFace> boundaryFaceList, CMeshO* pMesh);
	void ClassifyIntersectedFaces(vector<FFace> splitFaces, CMeshO *AimMesh, vector<Point3f> splitPoints, vector<FFace> &IN_S, vector<FFace> &OUT_S);
	bool ClassifyIntersectedFace(FFace splitFace, CMeshO *AimMesh);
	void WhatBooleanModelYouWant(int iComputeKind);
	void WhatBooleanModelYouWant2(int iComputeKind);
	void WhatBooleanModelYouWant3(int iComputeKind);
	void ClearAllVectorObject();
	bool ComputeCrossPoint(Point3f startP, Point3f endP, int iFaceIndex, CMeshO *Mesh, Point3f &CrossPoint);
	bool ComputeCrossPointEasy(Point3f startP, Point3f endP, int iFaceIndex, CMeshO *Mesh, Point3f &CrossPoint);
	int HowManyTimesIntersected(Point3f P, Point3f DirectV, CMeshO *AimMesh);
	int HowManyTimesIntersected(Point3f P, Point3f DirectV, vector<int> indexList, CMeshO *AimMesh);
	bool IfThisRadialIntersectWith(Point3f startP, Point3f DirectV, int iFaceIndex, CMeshO *Mesh);
	vector<int>  filterPossiblyIntersectFaces(Point3m P, Point3m edge1, Point3m edge2, vector<int> faceList, CMeshO *AimMesh);

	void generateAddingFaces(int iKind);
	void generateAddingFaces2(int iKind);
	void deleteFacesInList(vector<int> deleteFaceList);
	void deleteFacesInList(vector<int> deleteFaceList, CMeshO *curMesh);
	void addAllFaces();
	vector<int> addAllFaces(vector<FFace> faceList, CMeshO *curMesh);//返回新添加面的索引队列
	void generateNewVertexsAndNewFaces(vector<FFace> &faceList);
	CVertexO* addOrFindThisPointsPointer(Point3f p);
	CVertexO* addOrFindThisPointsPointer(Point3f p, CMeshO *curMesh);

	template<typename T> bool exitInVector(T t, vector<T> list);
	template<typename T> void putInVector(T t, vector<T> &list);
	void getIgnoreSplitFaceList(vector<Point2m> igonreCP);

	Point3m getLocalSysPoint(Point3m vert, Point3m axisX, Point3m axisY, Point3m axisZ, Point3m center);
	Point3m releaseLocalSysPoint(Point3m vert, Point3m axisX, Point3m axisY, Point3m axisZ, Point3m center);
	bool getAHProjIntersection(Point3m &p0, Point3m &p1, FEdge limitEdge, FFace projTriangle, FFace localTriangle);
	bool getAVProjIntersection(Point3m &p0, Point3m &p1, FEdge edge, FFace localTriangle);
	bool computeZeroPointOnLineSegment(float &t, float minXLimit, float maxXLimit, FEdge edge);

public:
	CMeshO *A, *B;          //所要进行布尔运算的两个mesh模型
	int **cMA, **cMB;       //A和B进行包围盒碰撞检测后记录的碰撞面结果（碰撞矩阵）。
	vector<SplitTriangle> splitTrianglesA, splitTrianglesB;//A和B待分割面记录。记录了待分割面所在网格，序号，内部分割点
	vector<Point3f> SPoints; //分割点序列
	int rowNumA, columnNumA, rowNumB, columnNumB;

	//关于模型布尔运算
	DelaunayMachine splitMachine;
	vector<FFace> splitFacesA, splitFacesB; //相交面重新分割之后产生的剖分结果
	vector<int> AinB, AoutB, BinA, BoutA;   //记录了四种情况各自所属的面的编号
	vector<FFace> AinB_Split, AoutB_Split, BinA_Split, BoutA_Split;
	vector<IntersectTestRecord>  interRecords;
	vector<IntersectTestRecord>  interRecordsA, interRecordsB;

	vector<FFace> addingFaces;
	vector<int> ignoreSplitFace_A, ignoreSplitFace_B;
	vector<FFace> booleanComputeResult_2D;

	Boolean2DMachine machine2D;
	vector<FFace> BO2Dresult;

//使用构建交线循环圈的方案进行三维布尔运算剔除
public:
	void BooleanComputeSortedLoop(CMeshO *_a, CMeshO *_b, int iComputeKind, vector<Point2m> cp);///布尔运算起始函数3,构建有序边界环进行面片筛选
	void BooleanComputeSortedLoopNewest(CMeshO *_a, CMeshO *_b, int iComputeKind, vector<Point2m> cp);
	vector<vector<FFace>> computeIntersectionFaces(CMeshO *_a, CMeshO *_b, vector<Point2m> cp);//计算量模型相交面集，不改变原始模型
	vector<int> updateModelAfterSplit(vector<SplitTriangle> splitFaces, vector<FFace> addFaces, CMeshO *curMesh, vector<int> join2DBooleanOperateFaces);//返回新添加面的索引队列
	vector<WordCircle> constructIntersectionLoop(vector<SplitTriangle> triangles);//根据剖分面集中记录的交界边，构建有序交线环
	vector<WordCircle> getOppositeOrderLoops(vector<WordCircle> loops, CMeshO *curMesh);
	void distributeVertsSpace(vector<FEdge> &edges);
	vector<WordCircle> getContourCircles(vector<FEdge> edgelist);
	void adjustLoopToAntiClockwise(WordCircle &loop, Point3f tempCenter, CMeshO *curMesh, CMeshO *aimMesh);
	void distinguishOutsideAndInsideFaces(vector<WordCircle> loops, CMeshO *curMesh, vector<int> boundFaces, vector<int> &outsideList, vector<int> &insideList);
	void distinguishOutsideAndInsideFaces(WordCircle loop, CMeshO *curMesh, vector<int> boundFaces, vector<int> &outsideList, vector<int> &insideList);
	vector<int> searchAllTrianglesConjugateLoop(WordCircle loop, vector<int> boundFaces, CMeshO *curMesh);
	vector<int> searchConnectedFacesByTopology(CMeshO* curMesh, vector<int> startFaceIndexes, vector<WordCircle> boundaryLoops, vector<int> &insideList);
	int getConnectedFaceByEdge(CMeshO* curMesh, int faceIndex, int edgeSign, Point2i edgeIndexes, const vector<Point2i> boundays, const vector<Point2i> checkedEdges);
	bool findNearIntersectPoint(Point3m &vert, const vector<Point3m> intersectVerts);
	bool pointInsideTriangle(Point3m p, int faceIndex, CMeshO *curMesh);
	float computeDet2D(Point2f a, Point2f b, Point2f c);
	void computeCoordinateIn2D(Point3f E3, float &fu, float &fv, Point3f axisX, Point3f axisY, Point3f U, Point3f V);
	void initalIntersectInfos(const vector<DefiniteIntersection> interVertsA, const vector<DefiniteIntersection> interVertsB);
	vector<IntersectTestRecord> collectIntersectInfos(DefiniteIntersection vertA, DefiniteIntersection vertB);

	int quickFindAimRecord(IntersectTestRecord curR, vector<IntersectTestRecord> recordList);
	void quickInsertAimRecord(IntersectTestRecord curR, vector<IntersectTestRecord> &recordList);
	void quickGetIntersectionResult(IntersectTestRecord newrecord, vector<IntersectTestRecord> &recordList, bool &bIntersected, Point3m &intersectionP,
		vector<Point3f> &intersectVerts, vector<TrackableVert> &intersectTrackVerts, Point3m startP, Point3m endP, int curFaceIndex, CMeshO* curMesh, CMeshO* anotherMesh);

	vector<WordCircle> interLoopA, interLoopB;//intersection edge Loop
	vector<int> outsideA, insideA, outsideB, insideB;
	CMeshO *sourceAA = NULL, *sourceBB = NULL;
};

#endif // !COMMON_BOOLEANMACHINE_H
