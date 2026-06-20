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

#include "gjkmachine.h"

GJKMachine::GJKMachine()
{
}

GJKMachine::~GJKMachine()
{
}

bool GJKMachine::test(Point3m _centerA, vector<Point3m> _A, Point3m _centerB, vector<Point3m> _B)
{
	this->centerA = _centerA;
	this->centerB = _centerB;
	this->A = _A;
	this->B = _B;

	this->simplicianVertexes.clear();

	int itimer = 0;
	EvolveResult result;
	result = EvolveResult::STILLEVOLVE;
	while (result == EvolveResult::STILLEVOLVE && itimer <= 2000)
	{
		result = evolveSimplex2();
		++itimer;
	}

	this->fPiercingDistance = 0.0f;
	this->fCreviceDistance = 0.0f;
	if (result != EvolveResult::STILLEVOLVE)
	{
		//if (pointInTetrahedron(Point3m(0, 0, 0), this->simplicianVertexes))
		if(result == EvolveResult::FOUNDINTERSCTION)
		{
			bool findPiercingDistance;

			intersectHappend = true;
			showEdge = epaFilterCloestEdge(this->simplicianVertexes, findPiercingDistance, fPiercingDistance);

			if (!findPiercingDistance)
			{
				Point3m approximateVector = showEdge.vertA - showEdge.vertB;
				this->fPiercingDistance = sqrtf(approximateVector * approximateVector);
			}
		}
		else
		{
			intersectHappend = false;
			showEdge = selectAimEdge();

			Point3m approximateVector = showEdge.vertA - showEdge.vertB;
			//this->fCreviceDistance = sqrtf(approximateVector * approximateVector);

			this->fCreviceDistance = sqrtf(curDirection * curDirection);
		}
	}

	return result == EvolveResult::FOUNDINTERSCTION;
}

bool GJKMachine::test(Point3m _centerA, vector<Point3m> _A, Point3m _centerB, vector<Point3m> _B, Point3m &transferVector)
{
	this->centerA = _centerA;
	this->centerB = _centerB;
	this->A = _A;
	this->B = _B;

	this->simplicianVertexes.clear();

	EvolveResult result;
	result = EvolveResult::STILLEVOLVE;
	while (result == EvolveResult::STILLEVOLVE)
	{
		result = evolveSimplex2();
	}

	this->fPiercingDistance = 0.0f;
	this->fCreviceDistance = 0.0f;
	if (result != EvolveResult::STILLEVOLVE)
	{
		if (result == EvolveResult::FOUNDINTERSCTION)
		{
			bool findPiercingDistance;

			intersectHappend = true;
			showEdge = epaFilterCloestEdge(this->simplicianVertexes, findPiercingDistance, fPiercingDistance);
			fPiercingDistance *= -1.0f;

			if (findPiercingDistance && fPiercingDistance < MIN_CONTACT_PIERCING_DEPTH)
			{
				transferVector = -this->piercingVector * (MIN_CONTACT_PIERCING_DEPTH - fPiercingDistance);
				return true;
			}
			else
			{
				transferVector = Point3m(0, 0, 0);
				return false;
			}
		}
		else
		{
			intersectHappend = false;
			transferVector = Point3m(0, 0, 0);
			return false;
		}
	}
	return false;
}

bool GJKMachine::test(Point3m _centerA, vector<Point3m> _A, Point3m _centerB, vector<Point3m> _B, Point3m &transferVector, float fCreviceLimit)
{
	this->centerA = _centerA;
	this->centerB = _centerB;
	this->A = _A;
	this->B = _B;

	this->simplicianVertexes.clear();

	EvolveResult result;
	result = EvolveResult::STILLEVOLVE;
	while (result == EvolveResult::STILLEVOLVE)
	{
		result = evolveSimplex2();
	}

	bool bFrozenCrevice;
	if (abs(fCreviceLimit) >= EFFECTIVE_CREVICE_WIDTH)
	{
		bFrozenCrevice = false;
	}
	else
	{
		bFrozenCrevice = true;
	}

	this->fPiercingDistance = 0.0f;
	this->fCreviceDistance = 0.0f;
	if (result != EvolveResult::STILLEVOLVE)
	{
		if (result == EvolveResult::FOUNDINTERSCTION)
		{
			bool findPiercingDistance;

			intersectHappend = true;
			showEdge = epaFilterCloestEdge(this->simplicianVertexes, findPiercingDistance, fPiercingDistance);
			fPiercingDistance *= -1.0f;

			if (bFrozenCrevice)
			{
				if (findPiercingDistance)
				{
					transferVector = -this->piercingVector * (fCreviceLimit - fPiercingDistance);
					return true;
				}
				else
				{
					transferVector = Point3m(0, 0, 0);
					return false;
				}
			}
			else
			{
				if (findPiercingDistance && fPiercingDistance < MIN_CONTACT_PIERCING_DEPTH)
				{
					transferVector = -this->piercingVector * (MIN_CONTACT_PIERCING_DEPTH - fPiercingDistance);
					return true;
				}
				else
				{
					transferVector = Point3m(0, 0, 0);
					return false;
				}
			}

		}
		else
		{
			if (bFrozenCrevice)
			{
				this->fCreviceDistance = sqrtf(curDirection * curDirection);
				curDirection = curDirection.Normalize();
				transferVector = -curDirection * (fCreviceLimit - fCreviceDistance);
				return true;
			}
			else
			{
				intersectHappend = false;
				transferVector = Point3m(0, 0, 0);
				return false;
			}
		}
	}
	return false;
}

bool GJKMachine::testSingleStep()
{
	EvolveResult result;
	result = EvolveResult::STILLEVOLVE;
	result = evolveSimplex2();

	if (result != EvolveResult::STILLEVOLVE)
	{
		if (pointInTetrahedron(Point3m(0, 0, 0), this->simplicianVertexes))
		{
			intersectHappend = true;
			showEdge = selectAimEdge();
		}
		else
		{
			intersectHappend = false;
			showEdge = selectAimEdge();
		}
	}

	return result == EvolveResult::FOUNDINTERSCTION;
}

EvolveResult GJKMachine::evolveSimplex()
{
	Point3m ab, ac, ao, da, db, dc, d0, norabd, norbcd, norcad;
	//ݵǰζǷжԭλùϵԼˢ
	switch (this->simplicianVertexes.size())
	{
	case 0:
		this->curDirection = this->centerB - this->centerA;
		//this->curDirection = this->B[0] - this->A[0];
		break;

	case 1:
		this->curDirection *= -1.0f;
		break;

	case 2:
		ab = simplicianVertexes[1].diff - simplicianVertexes[0].diff;
		ao = simplicianVertexes[0].diff * -1;
		this->curDirection = (ab^ao)^ab;
		break;

	case 3:
		ab = simplicianVertexes[1].diff - simplicianVertexes[0].diff;
		ac = simplicianVertexes[2].diff - simplicianVertexes[0].diff;
		ao = simplicianVertexes[0].diff * -1;
		this->curDirection = ab^ac;
		if (curDirection * ao < 0)
		{
			this->curDirection *= -1.0f;
		}
		break;

	case 4:
		da = simplicianVertexes[0].diff - simplicianVertexes[3].diff;
		db = simplicianVertexes[1].diff - simplicianVertexes[3].diff;
		dc = simplicianVertexes[2].diff - simplicianVertexes[3].diff;
		d0 = simplicianVertexes[3].diff * -1.0f;

		norabd = da ^ db;
		norbcd = db ^ dc;
		norcad = dc ^ da;

		if (norabd * d0 > 0)
		{
			simplicianVertexes.erase(simplicianVertexes.begin() + 2);
			this->curDirection = norabd;
		}
		else if (norbcd * d0 > 0)
		{
			simplicianVertexes.erase(simplicianVertexes.begin() + 0);
			this->curDirection = norbcd;
		}
		else if (norcad * d0 > 0)
		{
			simplicianVertexes.erase(simplicianVertexes.begin() + 1);
			this->curDirection = norabd;
		}
		else
		{
			return EvolveResult::FOUNDINTERSCTION;
		}

		break;
	}

	if (!addSupport(this->curDirection))
	{
		return EvolveResult::NOINTERSECTION;
	}
	else
	{
		return EvolveResult::STILLEVOLVE;
	}
}

EvolveResult GJKMachine::evolveSimplex2()
{
	switch (this->simplicianVertexes.size())
	{
	case 0:
		this->simplicianVertexes.push_back(MinkowskiDiff(this->A[0], this->B[0]));
		this->k = 0;
		this->oldDiff = 0;
		this->noChangeTime = 0;
		break;

	case 1:
		this->simplicianVertexes.push_back(this->newVert);
		break;

	case 2:
		this->simplicianVertexes.push_back(this->newVert);
		break;

	case 3:
		this->simplicianVertexes.push_back(this->newVert);
		break;

	case 4:
		this->simplicianVertexes.clear();
		this->simplicianVertexes = this->baseTriangleVerts;
		this->simplicianVertexes.push_back(this->newVert);
		break;
	}

	this->curDirection = this->getNearestVertToOriginFrom(this->simplicianVertexes);
	this->curDirection *= -1.0f;
	this->k += 1;

	float curDistance = curDirection * curDirection;

	float curDiff = abs(this->gk(curDirection));
	if (curDiff != oldDiff)
	{
		oldDiff = curDiff;
		noChangeTime = 0;
	}
	else
	{
		noChangeTime++;
	}
	if (noChangeTime >= 10)
	{
		return EvolveResult::FOUNDINTERSCTION;
	}
	if (curDiff < GJK_EPSILON)
	{
		return EvolveResult::NOINTERSECTION;
	}
	else
	{
		return EvolveResult::STILLEVOLVE;
	}
}

bool GJKMachine::addSupport(Point3m direction)
{
	MinkowskiDiff newVert = computeSupport(direction);
	this->simplicianVertexes.push_back(newVert);
	if (newVert.diff * direction > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

MinkowskiDiff GJKMachine::computeSupport(Point3m direction)
{
	Point3m tempA = support(this->A, direction, this->centerA);
	Point3m tempB = support(this->B, -direction, this->centerB);
	return MinkowskiDiff(tempA, tempB);
}

Point3m GJKMachine::support(const vector<Point3m> &vertList, Point3m direction, Point3m center)
{
	float maxProj;
	Point3m wantedVert;

	center = Point3m(0, 0, 0);
	wantedVert = vertList[0] - center;
	maxProj = wantedVert * direction;

	float tempProj;
	for (int i = 1; i < vertList.size(); ++i)
	{
		tempProj = (vertList[i] - center) * direction;
		if (tempProj > maxProj)
		{
			maxProj = tempProj;
			wantedVert = vertList[i];
		}
	}

	return wantedVert;
}

FEdge GJKMachine::selectAimEdge()
{
	int minIndex;
	float minDistance;

	for (int i = 0; i < this->simplicianVertexes.size(); ++i)
	{
		simplicianVertexes[i].computeDistance();
	}

	minIndex = 0;
	minDistance = simplicianVertexes[minIndex].fDistance;
	for (int i = 0; i < this->simplicianVertexes.size(); ++i)
	{
		if (minDistance > simplicianVertexes[i].fDistance)
		{
			minIndex = i;
			minDistance = simplicianVertexes[i].fDistance;
		}
	}

	return FEdge(simplicianVertexes[minIndex].sourceA, simplicianVertexes[minIndex].sourceB);
}

FEdge GJKMachine::epaFilterCloestEdge(vector<MinkowskiDiff> initalVertsList, bool &findSuccess, float &distance)
{
	if (initalVertsList.size() != 4)
	{
		return FEdge(Point3m(0, 0, 0), Point3m(0, 0, 0));
	}

	this->mkTriangles.clear();
	FEdge result;
	//1.ʼMinkowskiTriangle
	int j, k;
	for (int i = 0; i < 4; ++i)
	{
		j = i + 1;
		if (j >= 4)
		{
			j = 0;
		}

		k = j + 1;
		if (k >= 4)
		{
			k = 0;
		}

		this->mkTriangles.push_back(
			MinkowskiTriangle(
				  i,
			      initalVertsList[i],
				  initalVertsList[j],
				  initalVertsList[k]
			));
	}

	//2.ʼmkTrianglesѰ
	int curIndex;
	MinkowskiTriangle *curOpeTriangle;
	int iCounter = 0, iTIME = 100;
	MinkowskiDiff newestVert;

	findSuccess = false;
	distance = 0;
	do {
		////->浽ԭľ
		{
			for (int i = 0; i < mkTriangles.size(); ++i)
			{
				if (!mkTriangles[i].ensureDistanceToOrigin)
				{
					mkTriangles[i].getDistanceToOrigin();
				}
			}
		}
		////->ɸѡԭΪcurOpeTriangle
		{
			curIndex = 0;
			float minDistance = mkTriangles[curIndex].distanceToOrigin;
			for (int i = 0; i < mkTriangles.size(); ++i)
			{
				if (minDistance > mkTriangles[i].distanceToOrigin)
				{
					minDistance = mkTriangles[i].distanceToOrigin;
					curIndex = i;
				}
			}
			curOpeTriangle = &mkTriangles[curIndex];
		}
		////->curOpeTrianglenormalΪµsupport
		{
			MinkowskiDiff tempNewVert = computeSupport(-curOpeTriangle->normal);

			this->curDirection = curOpeTriangle->normal;
			this->newVert = tempNewVert;

			float newDistance = abs(tempNewVert.diff * curOpeTriangle->normal);
			if (abs(newDistance - curOpeTriangle->distanceToOrigin) < GJK_EPSILON)
			{
				////->Ϊҵ
				result = FEdge(tempNewVert.sourceA, tempNewVert.sourceB);
				findSuccess = true;

				Point3m edge = curOpeTriangle->triangle[0].diff;
				distance = abs(edge * curOpeTriangle->normal);
				piercingVector = curOpeTriangle->normal;
				//result = FEdge(Point3m(0, 0, 0), curOpeTriangle->normal * distance);

				return result;
			}
			else
			{
				newestVert = tempNewVert;
				this->reconstructMinkowskiTrianglesSet(tempNewVert, curOpeTriangle);
			}
		}

		iCounter += 1;
	} while (iCounter < iTIME);

	return FEdge(newestVert.sourceA, newestVert.sourceB);
}

FEdge GJKMachine::testEpaSingleStep()
{
	FEdge result;
	//2.ʼmkTrianglesѰ
	int curIndex;
	MinkowskiTriangle *curOpeTriangle;
	int iCounter = 0, iTIME = 1;
	MinkowskiDiff newestVert;

	do {
		////->浽ԭľ
		{
			for (int i = 0; i < mkTriangles.size(); ++i)
			{
				if (!mkTriangles[i].ensureDistanceToOrigin)
				{
					mkTriangles[i].getDistanceToOrigin();
				}
			}
		}
		////->ɸѡԭΪcurOpeTriangle
		{
			curIndex = 0;
			float minDistance = mkTriangles[curIndex].distanceToOrigin;
			for (int i = 0; i < mkTriangles.size(); ++i)
			{
				if (minDistance > mkTriangles[i].distanceToOrigin)
				{
					minDistance = mkTriangles[i].distanceToOrigin;
					curIndex = i;
				}
			}
			this->curSTIndex = curIndex;
			curOpeTriangle = &mkTriangles[curIndex];
		}
		////->curOpeTrianglenormalΪµsupport
		{
			MinkowskiDiff tempNewVert = computeSupport(-curOpeTriangle->normal);
			this->curDirection = curOpeTriangle->normal;
			this->newVert = tempNewVert;
			float newDistance = abs(tempNewVert.diff * curOpeTriangle->normal);
			if (abs(newDistance - curOpeTriangle->distanceToOrigin) < GJK_EPSILON)
			{
				////->Ϊҵ
				result = FEdge(tempNewVert.sourceA, tempNewVert.sourceB);
				return result;
			}
			else
			{
				newestVert = tempNewVert;
				this->reconstructMinkowskiTrianglesSet(tempNewVert, curOpeTriangle);
			}
		}

		iCounter += 1;
	} while (iCounter < iTIME);

	//return FEdge(Point3m(10, 0, 0), Point3m(0, 0, 0));
	return FEdge(newestVert.sourceA, newestVert.sourceB);
}

void GJKMachine::reconstructMinkowskiTrianglesSet(MinkowskiDiff newVert, MinkowskiTriangle *delTriangle)
{
	////////////////////////////
	vector<MinkowskiEdge> lst_edges;

	auto lam_addEdge = [&](const MinkowskiDiff &a, const MinkowskiDiff &b)->void {
		for (auto it = lst_edges.begin(); it != lst_edges.end(); it++) {
			if ((it->vb.diff == b.diff && it->va.diff == a.diff) || (it->vb.diff == a.diff && it->va.diff == b.diff)) {
				//opposite edge found, remove it and do not add new one
				lst_edges.erase(it);
				return;
			}
		}
		lst_edges.emplace_back(a, b);
	};

	for (auto it = mkTriangles.begin(); it != mkTriangles.end();) {
		// can this face be 'seen' by entry_cur_support?
		if ((it->normal * (newVert.diff - it->triangle[0].diff)) > 0) {
			lam_addEdge(it->triangle[0], it->triangle[1]);
			lam_addEdge(it->triangle[1], it->triangle[2]);
			lam_addEdge(it->triangle[2], it->triangle[0]);
			it = mkTriangles.erase(it);
			continue;
		}
		it++;
	}

	// create new triangles from the edges in the edge list
	for (auto it = lst_edges.begin(); it != lst_edges.end(); it++) {
		mkTriangles.emplace_back(MinkowskiTriangle(0,  newVert, it->va, it->vb));
	}

	lst_edges.clear();
}

vector<Point3m> GJKMachine::getAllMKVerts()
{
	vector<Point3m> result;

	for (auto vertA : this->A)
	{
		for (auto vertB : this->B)
		{
			result.push_back(vertB - vertA);
		}
	}

	return result;
}

Point3m GJKMachine::getNearestVertToOriginFrom(vector<MinkowskiDiff> list)
{
	float projV;
	Point3m edge1, edge2;
	switch (list.size())
	{
	case 1:
		return list[0].diff;
		break;

	case 2:
		return this->getNearestVertToOriginFrom3DEdge(list[0].diff, list[1].diff);
		break;

	case 3:
		return this->getNearestVertToOriginFrom3DTriangle(list[0].diff, list[1].diff, list[2].diff);
		//return this->getOriginProjVertOnPlane(list[0].diff, list[1].diff, list[2].diff);
		break;

	case 4:
		//return this->closestVertInTetrahedron(list);
		return this->getNearestVertToOriginFrom3DTetrehedron(list[0].diff, list[1].diff, list[2].diff, list[3].diff);
		break;

	default:
		break;
	}
}

Point3m GJKMachine::getNearestVertToOriginFrom3DEdge(Point3m v1, Point3m v2)
{
	Point3m edge1 = (v2 - v1).Normalize();
	Point3m edge2 = -v1;
	float projV = edge1 * edge2;

	if (0 < projV && projV < sqrt((v2 - v1) * (v2 - v1)))//ֱϵͶӰ㣬ͶӰv1v2֮
	{
		return (v1 + edge1 * projV);
	}
	else
	{
		if (v1 * v1 > v2 * v2)//ȡϽĶ
		{
			return v2;
		}
		else
		{
			return v1;
		}
	}
}

Point3m GJKMachine::getNearestVertToOriginFrom3DTriangle(Point3m v1, Point3m v2, Point3m v3)
{
	Point3m planeNormal;
	planeNormal = (v2 - v1) ^ (v3 - v1);
	if (planeNormal * -v1 < 0)
	{
		planeNormal *= -1.0f;
	}

	Point3m P = this->getProjPointOnPlane(Point3m(0, 0, 0), -planeNormal, v1, planeNormal);
	if (this->pointInTriangle(v1, v2, v3, P))
	{
		return P;
	}
	else
	{
		vector<Point3m> edgeP;
		edgeP.push_back(this->getNearestVertToOriginFrom3DEdge(v1, v2));
		edgeP.push_back(this->getNearestVertToOriginFrom3DEdge(v2, v3));
		edgeP.push_back(this->getNearestVertToOriginFrom3DEdge(v3, v1));

		int minIndex = 0;
		float minDistance = edgeP[0] * edgeP[0];

		float curDistance;
		for (int i = 1; i < 3; ++i)
		{
			curDistance = edgeP[i] * edgeP[i];
			if (curDistance < minDistance)
			{
				minDistance = curDistance;
				minIndex = i;
			}
		}

		return edgeP[minIndex];
	}
}

Point3m GJKMachine::getNearestVertToOriginFrom3DTetrehedron(Point3m v1, Point3m v2, Point3m v3, Point3m v4)
{
	vector<Point3m> edgeP;
	edgeP.push_back(this->getNearestVertToOriginFrom3DTriangle(v1, v2, v4));
	edgeP.push_back(this->getNearestVertToOriginFrom3DTriangle(v2, v3, v4));
	edgeP.push_back(this->getNearestVertToOriginFrom3DTriangle(v3, v1, v4));
	edgeP.push_back(this->getNearestVertToOriginFrom3DTriangle(v1, v3, v2));

	int minIndex = 0;
	float minDistance = edgeP[0] * edgeP[0];

	float curDistance;
	for (int i = 1; i < 4; ++i)
	{
		curDistance = edgeP[i] * edgeP[i];
		if (curDistance < minDistance)
		{
			minDistance = curDistance;
			minIndex = i;
		}
	}

	baseTriangleVerts.clear();
	switch (minIndex)
	{
	case 0:
		baseTriangleVerts.push_back(this->simplicianVertexes[0]);
		baseTriangleVerts.push_back(this->simplicianVertexes[1]);
		baseTriangleVerts.push_back(this->simplicianVertexes[3]);
		break;

	case 1:
		baseTriangleVerts.push_back(this->simplicianVertexes[1]);
		baseTriangleVerts.push_back(this->simplicianVertexes[2]);
		baseTriangleVerts.push_back(this->simplicianVertexes[3]);
		break;

	case 2:
		baseTriangleVerts.push_back(this->simplicianVertexes[2]);
		baseTriangleVerts.push_back(this->simplicianVertexes[0]);
		baseTriangleVerts.push_back(this->simplicianVertexes[3]);
		break;

	case 3:
		baseTriangleVerts.push_back(this->simplicianVertexes[0]);
		baseTriangleVerts.push_back(this->simplicianVertexes[2]);
		baseTriangleVerts.push_back(this->simplicianVertexes[1]);
		break;
	default:
		break;
	}

	return edgeP[minIndex];
}

bool GJKMachine::pointInTriangle(Point3m A, Point3m B, Point3m C, Point3m P)
{
	Point3m v0 = C - A;
	Point3m v1 = B - A;
	Point3m v2 = P - A;

	float dot00 = v0 * v0;
	float dot01 = v0 * v1;
	float dot02 = v0 * v2;
	float dot11 = v1 * v1;
	float dot12 = v1 * v2;

	float inverDeno = 1 / (dot00 * dot11 - dot01 * dot01);

	float u = (dot11 * dot02 - dot01 * dot12) * inverDeno;
	if (u < 0 || u > 1) // if u out of range, return directly
	{
		return false;
	}

	float v = (dot00 * dot12 - dot01 * dot02) * inverDeno;
	if (v < 0 || v > 1) // if v out of range, return directly
	{
		return false;
	}

	return u + v <= 1;
}

float GJKMachine::gk(Point3m vk)
{
	float result;
	result = vk * vk + hk(-vk);
	return result;
}

float GJKMachine::hk(Point3m direct)
{
	Point3m sk1, sk2, sk;
	float hk;

	sk1 = support(this->A, direct, this->centerA);
	sk2 = support(this->B, -direct, this->centerB);
	sk = sk2 - sk1;

	newVert = MinkowskiDiff(sk1, sk2);

	hk = -sk * direct;
	return hk;
}

Point3m GJKMachine::getOriginProjVertOnPlane(Point3m v1, Point3m v2, Point3m v3)
{
	Point3m planeNormal;
	planeNormal = (v2 - v1) ^ (v3 - v1);
	if (planeNormal * -v1 < 0)
	{
		planeNormal *= -1.0f;
	}

	return this->getProjPointOnPlane(Point3m(0, 0, 0), -planeNormal, v1, planeNormal);
}

Point3m GJKMachine::getProjPointOnPlane(Point3m p, Point3m D, Point3f pos, Point3m N)
{
	Point3m result;

	float up, down;
	up = (pos - p) * N;
	down = D * N;
	float t = up / down;

	result = p + D * t;
	return result;
}

Point3m GJKMachine::closestVertInTetrahedron(vector<MinkowskiDiff> list)
{
	vector<Point3m> projVertList;
	projVertList.push_back(getOriginProjVertOnPlane(list[0].diff, list[1].diff, list[3].diff));
	projVertList.push_back(getOriginProjVertOnPlane(list[1].diff, list[2].diff, list[3].diff));
	projVertList.push_back(getOriginProjVertOnPlane(list[2].diff, list[0].diff, list[3].diff));
	projVertList.push_back(getOriginProjVertOnPlane(list[0].diff, list[2].diff, list[1].diff));

	int minIndex = 0;
	float minValue = projVertList[0] * projVertList[0];
	float curValue;
	for (int i = 1; i < 4; ++i)
	{
		curValue = projVertList[i] * projVertList[i];
		if (minValue > curValue)
		{
			minValue = curValue;
			minIndex = i;
		}
	}

	baseTriangleVerts.clear();
	switch (minIndex)
	{
	case 0:
		baseTriangleVerts.push_back(list[0]);
		baseTriangleVerts.push_back(list[1]);
		baseTriangleVerts.push_back(list[3]);
		break;

	case 1:
		baseTriangleVerts.push_back(list[1]);
		baseTriangleVerts.push_back(list[2]);
		baseTriangleVerts.push_back(list[3]);
		break;

	case 2:
		baseTriangleVerts.push_back(list[2]);
		baseTriangleVerts.push_back(list[0]);
		baseTriangleVerts.push_back(list[3]);
		break;

	case 3:
		baseTriangleVerts.push_back(list[0]);
		baseTriangleVerts.push_back(list[2]);
		baseTriangleVerts.push_back(list[1]);
		break;
	default:
		break;
	}

	return projVertList[minIndex];
}

bool GJKMachine::pointInTetrahedron(Point3m v, vector<MinkowskiDiff> list)
{
	if (list.size() < 4)
	{
		return false;
	}

	Point3m p, direct, pos, planeNormal, projVert;

	p = v;

	pos = list[0].diff;
	planeNormal = (list[1].diff - list[0].diff) ^ (list[3].diff - list[0].diff);
	direct = -planeNormal;
	projVert = this->getProjPointOnPlane(p, direct, pos, planeNormal);
	if (!pointInTriangle(list[0].diff, list[1].diff, list[3].diff, projVert))
	{
		return false;
	}

	pos = list[1].diff;
	planeNormal = (list[2].diff - list[1].diff) ^ (list[3].diff - list[1].diff);
	direct = -planeNormal;
	projVert = this->getProjPointOnPlane(p, direct, pos, planeNormal);
	if (!pointInTriangle(list[1].diff, list[2].diff, list[3].diff, projVert))
	{
		return false;
	}

	pos = list[2].diff;
	planeNormal = (list[0].diff - list[2].diff) ^ (list[3].diff - list[2].diff);
	direct = -planeNormal;
	projVert = this->getProjPointOnPlane(p, direct, pos, planeNormal);
	if (!pointInTriangle(list[2].diff, list[0].diff, list[3].diff, projVert))
	{
		return false;
	}

	pos = list[0].diff;
	planeNormal = (list[1].diff - list[0].diff) ^ (list[2].diff - list[0].diff);
	direct = -planeNormal;
	projVert = this->getProjPointOnPlane(p, direct, pos, planeNormal);
	if (!pointInTriangle(list[0].diff, list[1].diff, list[2].diff, projVert))
	{
		return false;
	}

	return true;
}
