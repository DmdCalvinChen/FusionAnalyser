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

#ifndef COMMON_GJKMACHINE_H
#define COMMON_GJKMACHINE_H

#define GJK_EPSILON 0.0001f
#define MIN_CONTACT_PIERCING_DEPTH -0.01f
#define MAX_PERMISSIBLE_PIERCING_DEPTH 0.5f
#define EFFECTIVE_CREVICE_WIDTH 100

#include "common/ml_mesh_type.h"
#include "util/assist_geometry.h"

#include "common_ext_global.h"

using namespace std;

enum EvolveResult { NOINTERSECTION = 0, STILLEVOLVE, FOUNDINTERSCTION };

struct COMMON_EXT_EXPORT MinkowskiDiff
{
	MinkowskiDiff(){}
	MinkowskiDiff(Point3m _A, Point3m _B)
	{
		this->sourceA = _A;
		this->sourceB = _B;
		this->diff = sourceB - sourceA;
	}

	MinkowskiDiff& operator=(const MinkowskiDiff& e)
	{
		this->sourceA = e.sourceA;
		this->sourceB = e.sourceB;
		this->diff = e.diff;
		return *this;
	}

	void computeDistance()
	{
		this->fDistance = diff * diff;
	}

public:
	Point3m sourceA, sourceB;
	Point3m diff;
	float fDistance;
};

struct COMMON_EXT_EXPORT MinkowskiEdge
{
	MinkowskiEdge() {}
	MinkowskiEdge(MinkowskiDiff _a, MinkowskiDiff _b)
	{
		this->va = _a;
		this->vb = _b;
	}
	MinkowskiEdge& operator=(const MinkowskiEdge& e)
	{
		this->va = e.va;
		this->vb = e.vb;
		return *this;
	}

public:
	MinkowskiDiff va, vb;
};

struct COMMON_EXT_EXPORT MinkowskiTriangle
{
public:
	MinkowskiTriangle(){}
	MinkowskiTriangle(int _index, MinkowskiDiff v1, MinkowskiDiff v2, MinkowskiDiff v3)
	{
		this->ensureDistanceToOrigin = false;
		this->index = _index;
		this->triangle.push_back(v1);
		this->triangle.push_back(v2);
		this->triangle.push_back(v3);
	}
	MinkowskiTriangle& operator=(const MinkowskiTriangle& e)
	{
		this->triangle = e.triangle;
		this->normal = e.normal;
		this->index = e.index;
		this->distanceToOrigin = e.distanceToOrigin;
		this->ensureDistanceToOrigin = e.ensureDistanceToOrigin;
		return *this;
	}

	void getDistanceToOrigin()
	{
		this->getOutwardNormalVector();
		distanceToOrigin = abs(triangle[0].diff * this->normal);
		this->ensureDistanceToOrigin = true;
	}

	void getOutwardNormalVector()
	{
		Point3m vector1, vector2;
		vector1 = triangle[1].diff - triangle[0].diff;
		vector2 = triangle[2].diff - triangle[0].diff;
		normal = vector1 ^ vector2;
		if (normal * triangle[0].diff < 0)
		{
			normal *= -1.0f;
		}
		normal = normal.Normalize();
	}

public:
	vector<MinkowskiDiff> triangle;
	Point3m normal;
	float distanceToOrigin;
	int index;
	bool ensureDistanceToOrigin = false;
};

class COMMON_EXT_EXPORT GJKMachine
{
public:
	GJKMachine();
	~GJKMachine();

	bool test(Point3m _centerA, vector<Point3m> _A, Point3m _centerB, vector<Point3m> _B);
	bool test(Point3m _centerA, vector<Point3m> _A, Point3m _centerB, vector<Point3m> _B, Point3m &transferVector);//检测是否需要产生应力形变
	bool test(Point3m _centerA, vector<Point3m> _A, Point3m _centerB, vector<Point3m> _B, Point3m &transferVector, float fCreviceLimit);//检测是否需要产生应力形变
	bool testSingleStep();
	FEdge testEpaSingleStep();
	EvolveResult evolveSimplex();
	EvolveResult evolveSimplex2();
	bool addSupport(Point3m direction);
	MinkowskiDiff computeSupport(Point3m direction);
	Point3m support(const vector<Point3m> &shape, Point3m direction, Point3m center);
	FEdge selectAimEdge();

	FEdge epaFilterCloestEdge(vector<MinkowskiDiff> initalVertsList, bool &findSuccess, float &distance);
	void reconstructMinkowskiTrianglesSet(MinkowskiDiff newVert, MinkowskiTriangle *delTriangle);

	vector<Point3m> getAllMKVerts();

	Point3m getNearestVertToOriginFrom(vector<MinkowskiDiff> list);
	float gk(Point3m vk);
	float hk(Point3m direct);

	Point3m getOriginProjVertOnPlane(Point3m v1, Point3m v2, Point3m v3);
	Point3m getProjPointOnPlane(Point3m p, Point3m D, Point3f pos, Point3m N);
	Point3m closestVertInTetrahedron(vector<MinkowskiDiff> list);

	Point3m getNearestVertToOriginFrom3DEdge(Point3m v1, Point3m v2);
	Point3m getNearestVertToOriginFrom3DTriangle(Point3m v1, Point3m v2, Point3m v3);
	Point3m getNearestVertToOriginFrom3DTetrehedron(Point3m v1, Point3m v2, Point3m v3, Point3m v4);
	bool pointInTriangle(Point3m v1, Point3m v2, Point3m v3, Point3m P);
	bool pointInTetrahedron(Point3m v, vector<MinkowskiDiff> list);
public:
	Point3m centerA, centerB;
	vector<Point3m> A, B;//模型A，模型B点集
	vector<MinkowskiDiff> simplicianVertexes;//单纯形顶点集合
	Point3m curDirection;//当前搜索方向
	FEdge showEdge;
	int k = 0;

	int curSTIndex = 0;
	vector<MinkowskiTriangle> mkTriangles;
	MinkowskiDiff newVert;
	vector<MinkowskiDiff> baseTriangleVerts;
	float oldDiff;
	int noChangeTime = 0;
	bool intersectHappend = false;

	float fPiercingDistance = 0.0f, fCreviceDistance = 0.0f;;
	Point3m piercingVector;
};

#endif // !COMMON_GJKMACHINE_H
