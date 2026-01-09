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

#ifndef COMBEDEDGEMACHINE_H
#define COMBEDEDGEMACHINE_H

#include <QObject>
#include <qdebug.h>
#include <qmessagebox.h>
#include "common/ml_mesh_type.h"
#include "util/assist_geometry.h"
#include "common_ext_global.h"

using namespace std;
using namespace vcg;

class COMMON_EXT_EXPORT CombedEdgeMachine
{
public:
	CombedEdgeMachine();
	CombedEdgeMachine(vector<Point3m> _nodelist, vector<FEdge> _edgelist, vector<FFace> _facelist);
	~CombedEdgeMachine();

public:
	void initalMachine(FEdge aim, vector<int> indexList);
	void initalMachine(FEdge edge, vector<int> indexList, vector<Point3m> _nodelist, vector<FEdge> _edgelist, vector<FFace> _facelist);
	void updateCombedRequirement();
	void combedEdgeTurnOn();
	void clearAll();
	FEdge pickOutAimDiagonal();
    bool exchangeDiagonal(FEdge aimDiagonal, vector<int> &arealist);
	FPoint getOppositeVertToEdge(FEdge aimDiagonal, int face);
	int getEdgeIndexInList(vector<FEdge> list, FEdge aim);
	int getEdgeIndexInList(set<int> indexlist, Point2i aim);
	void findAssociatedEdgesWith(int iVert, FEdge exceptEdge, int faceA, int faceB, FEdge &newEdge1, FEdge &newEdge2);
	bool ensureFaceIndex(FFace newFace, int &faceIndex, int &anOtherfaceIndex, int faceA, int faceB);
	SPACE_ENUM ensureEdgeSpaceAttribute(FEdge edge);
	SPACE_ENUM ensurePointSpaceAttribute(FPoint point);
	bool isSalientAngleInQuadrilateral(FEdge edge, int angleVertIndex);

	Point3m standNormalV;
	int aimDiagonalIndex;

	vector<int> combedAreaIndex;
	vector<FPoint> tempPointlist;

	vector<FEdge> edgeCollect;
	vector<FEdge> unBorderlist;

	vector<vector<FFace>> embedSteps;
	int ifuck = 0;

public:
	void combedEdgeByCheckForbiddenEdgeCP();
	void analysisCurrentState();
	void updateAreaBoundaryAndInsideEdgeIndexSet(set<int> edgeIndexes);
	bool isAreaBoundary(int edgeIndex);
	void updateForbiddenEdgeCPSet();
	void sortInsideEdgeIndexes();
	int pickOneOpeDiagonal();
	pair<int, int> collectEdgeCPLinkedWith(int iVert, int iEdge);
	bool ifExistInForbiddenEdgeCPSet(const pair<int, int> &cp);
	void exchangeDiagonal(int iEdge);
	int getDiffVertIndexIn(int iFace, int iVertA, int iVertB);
	void getEachEdgeIndexInFace(FFace &face, int newFaceIndex, pair<int, int> range, int iAvoidEdge);
	void correctionEdgesAdjacentFaceIndex(int iEdge, int newFaceIndex, pair<int, int> range, int iAvoidEdge);
	bool checkAndUpdateDynamicArea();
	bool canGetRidOfFromDynamicArea(int iFace);
	bool getConcaveEdge(FEdge &_sub_border);

public:
	set<int> boundaryIndexes, insideEdgeIndexes;
	vector<int> sortedInsideEdgeIndexes;
	vector<int> localEdgeIndexes;
	vector<pair<int, int>> forbiddenEdgeCPs;
	vector<FPoint> QL, QR;
	vector<int> dynamicAreaList, concaveVertIndexList;
	vector<FFace> area;

	FEdge aimEdge;
	vector<Point3m> nodelist;
	vector<FEdge> edgelist;
	vector<FFace> facelist;
	int iTime = 0;
};

#endif // !COMBEDEDGEMACHINE_H

