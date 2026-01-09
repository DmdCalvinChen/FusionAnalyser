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

#ifndef COMMON_DELAUNAYMACHINE_H_
#define COMMON_DELAUNAYMACHINE_H_

#include <math.h>
#include "common/ml_mesh_type.h"
#include "util/assist_geometry.h"
#include "machine/combededgemachine.h"

#include "common_ext_global.h"

using namespace std;
using namespace vcg;

enum SITUATION {
LEFT,
OUTSIDE,
INSIDE
};

enum SPACEATTRIBUTE {
	OUTOFDENTAL,
	ONDENTAL,
	WITHINDENTAL
};

struct COMMON_EXT_EXPORT OnEdgePoint
{
public:
	OnEdgePoint() {}

	OnEdgePoint(Point3m _p, FEdge _edge)
	{
		this->p = _p;
		this->edge = _edge;
	}
public:
	Point3m p;
	FEdge edge;
};

class COMMON_EXT_EXPORT DelaunayMachine
{
public:
	DelaunayMachine();
	~DelaunayMachine();

public:
	vector<Point3f> vertexs, sortedVertexs;
	vector<FFace> triangles;
	vector<FFace> tempTriangles;
	vector<FEdge> edgeList;
	vector<FEdge> ReEdgeList;
	vector<OnEdgePoint> onEdgePList;
	FFace STriangle;
	FFace sourceSTriangle;
	Point3f circleCenter;
	int edgeNum = 0;
	bool bCanDrawSTriangle = false;
	float allowedDeviation = 0.0f;

	vector<TraceablePoint> vertexs_trace;
	vector<FEdge> traceEdgeTab;

	void InitDelaunayMachine(vector<Point3f> _vertexs);
	void InitDelaunayMachine(vector<Point3f> _vertexs, FFace defaultSuperTriangle);
	void InitDelaunayMachine(vector<TraceablePoint> _vertexs, FFace defaultSuperTriangle);
	void DelaunayMachineTurnOn();
	void DelaunayMachineTurnOnModel2();
	void DelaunayMachineTurnOnModel3();
	void DelaunayMachineTurnOnModel3Traceable(vector<vector<FFace>> &splitCollect);
	vector<FFace> DelaunayMachineTurnOnModel3Newest(const vector<Point3m> &vertices);
	void DelaunayMachineTurnOnModeTraceable(vector<vector<FFace>> &splitCollect);
	void ClearMachine();

	vector<FFace> DelaunayMachineArbitraryTurnOn2(vector<Point3m> _vertexs, vector<FEdge> _edgelist, FFace superFace);
	vector<vector<FFace>> DelaunayMachineArbitraryTurnOn2Traceable(vector<Point3m> _vertexs, vector<FEdge> _edgelist, FFace superFace);
	vector<FFace> DelaunayMachineArbitraryTurnOn2(vector<TraceablePoint> _vertexs, FFace defaultSuperTriangle);

	bool DelaunayMachineArbitraryTurnOnNewest(const vector<Point3m> &_vertexs, vector<FEdge>& _edgelist,
        const FFace &superFace, const Point3m &judgeNormal, vector<FFace> &result);

	void sortVertex(vector<Point3f> &vertexs);
	void sortVertex(vector<TraceablePoint> &vertexs);
	void getSuperTriangle(vector<Point3f> v);
	FFace getAndReturnSuperTriangle(vector<Point3f> v);
	void GetMaximumIn(vector<Point3f> vertexs, float &maxX, float &minX, float &maxY, float &minY);
	void getCircleCenterAndRadius(Point3f v1, Point3f v2, Point3f v3, Point3f &ccenter, float &fRadius);
	void getCircleCenterAndRadius(FFace face, Point3f &ccenter, float &fRadius);
	SITUATION WhichPosSituationIs(Point3f p, Point3f center, float riduas);
	SITUATION WhichPosSituationIs(Point3f p, Point3f center, float riduas, FFace triangle, bool &c1, bool &c2, bool &c3);
	SITUATION WhichPosSituationIsWithTraceableEdge(TraceablePoint p, Point3f center, float riduas, FFace triangle);
	void AddNewTempTriangles(Point3f p);
	void AddNewTempTriangles2(Point3f p);
	void AddNewTempTrianglesWithTraceableInfomation(TraceablePoint p);
	void CombineTwoTriangleVector();
	void RipeConnectionWithSuperTriangle(FFace superTri);
	bool IsConnectWithST(FFace f, FFace superTri);
	void SeachAndDeleteReplicateEdge(vector<FEdge> &edgeList);
	void addEdgeToConstructSplitPolygon(FEdge newEdge, vector<FEdge> &edgeList);
	bool tracePointOnEdge(TraceablePoint p, FEdge edge, bool &bColline);
	bool tracePointOnEdge2(TraceablePoint p, FEdge edge);

	void adjustByComplusiveBorder2(vector<FEdge> border, vector<Point3m> seedlist, FFace superTriangle);
	void combedSingleBoundary(FEdge border, vector<Point3m> &nodelist, vector<FEdge> &edgelist, vector<FFace> &facelist);
	void analysisAndFillThreeVector(vector<Point3m> &nodelist, vector<FEdge> &edgelist, vector<FFace> &facelist);
	void analysisAndFillThreeVector(vector<Point3m> &nodelist, vector<FEdge> &edgelist, vector<FFace> &facelist, vector<Point3m> seedlist, FFace superTriangle);
	vector<int> pickOutAllCoverFacesWith(const FEdge &border, const vector<Point3f> &nodelist, const vector<FEdge> &edgelist, const vector<FFace> &facelist);
	bool belongToSameBorder(FEdge aimEdge, FEdge objEdge);
	bool checkCoverArea(const vector<int> &coverArea, const FEdge &border, const vector<FFace> &facelist);

	bool getRidOfOutsideBoundaryTriangles(const vector<FFace> &splitFaces, const vector<FEdge> &_edgelist, const Point3m &judgeNormal, vector<FFace> &result);
	bool ifThisFaceOutofBoundary(const FFace &face, const vector<FEdge> &orderedBoundary, const Point3m &judgeNormal);
	bool ifThisFaceOutofBoundaryByItersectionMethod(const FFace &face, const vector<FEdge> &orderedBoundary, const Point3m &judgeNormal);
	void computeCoordinateIn2D(Point3f E3, float &fu, float &fv, Point3f axisX, Point3f axisY, Point3f U, Point3f V);
	void coordinateIn2D(Point3f e, float &fu, float &fv, Point3f origin, Point3f axisX, Point3f axisY);

	void localOptimization(vector<FFace> &faces);
	bool subLocalOptimization(vector<FFace> &faces);
	vector<pair<int, int>> checkBadTriangleCoupleFrom(vector<FFace> &faces);
	void optimizeAction(vector<pair<int, int>> badTriCP, vector<FFace> &faces);
	vector<pair<pair<int, int>, FEdge>> collectCommonEdgeInfo(vector<FFace> &faces);
	vector<pair<int, int>> extractBadTriangleCouple(vector<pair<pair<int, int>, FEdge>> &infoList, vector<FFace> &faces);
	bool isBadTriangleCouple(FFace &faceA, FFace &faceB, FEdge &commonEdge);
public:
	// Test/debug members
	vector<vector<FFace>> embedFacesCollect;
	vector<vector<FFace>> embedFacesSteps;
	vector<Point3m> embedVerts;

	vector<vector<FFace>> translateSteps(vector<vector<FFace>> faceMatrix, FFace superTriangle, vector<Point3f> projVertexes, vector<Point3m> _vertexs);
	vector<Point3m> translateVerts(vector<Point3m> vertlist, vector<Point3f> projVertexes, vector<Point3m> _vertexs);
};

#endif

