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

#include "definite_intersection.h"
#include "vcg/complex/algorithms/update/topology.h"

DefiniteIntersection::DefiniteIntersection()
{
}

DefiniteIntersection::DefiniteIntersection(Point3f _intersection)
{
	this->intersection = _intersection;
	this->ensureEdge = false;
}

DefiniteIntersection::DefiniteIntersection(Point3f _intersection, FEdge edge)
{
	this->intersection = _intersection;
	this->edgeList.push_back(edge);
	this->ensureEdge = true;
}

DefiniteIntersection::DefiniteIntersection(Point3f _intersection, CMeshO *pMesh)
{
	this->intersection = _intersection;
	this->meshPointer = pMesh;
	this->collectInformation(intersection);
	this->ensureEdge = false;
}

DefiniteIntersection::DefiniteIntersection(Point3f _intersection, FEdge edge, CMeshO *pMesh)
{
	this->intersection = _intersection;
	this->meshPointer = pMesh;
	this->edgeList.push_back(edge);
	this->collectInformation(edge);
	this->ensureEdge = true;
}

DefiniteIntersection::~DefiniteIntersection()
{
}

void DefiniteIntersection::collectInformation(Point3f vert)
{
	if (this->meshPointer == NULL)
	{
		return;
	}

	CVertexO* pVert = NULL;
	for (int i = 0; i < meshPointer->vert.size(); ++i)
	{
		if (meshPointer->vert[i].P() == vert)
		{
			pVert = &meshPointer->vert[i];
			break;
		}
	}
	if (pVert == NULL)
	{
		return;
	}

	bool exited;
	vcg::face::VFIterator<CFaceO> vfi(pVert);
	for (; !vfi.End(); ++vfi)
	{
		int faceIndex = vfi.F() - &this->meshPointer->face[0];
		exited = this->exitInVector<int>(faceIndex, this->faceList);
		if (exited)
		{
			continue;
		}

		this->faceList.push_back(faceIndex);
		FEdge edge1, edge2;
		if (meshPointer->face[faceIndex].V(0) == pVert)
		{
			edge1 = FEdge(meshPointer->face[faceIndex].V(0)->P(), meshPointer->face[faceIndex].V(1)->P());
			edge2 = FEdge(meshPointer->face[faceIndex].V(2)->P(), meshPointer->face[faceIndex].V(0)->P());
		}
		else if (meshPointer->face[faceIndex].V(1) == pVert)
		{
			edge1 = FEdge(meshPointer->face[faceIndex].V(0)->P(), meshPointer->face[faceIndex].V(1)->P());
			edge2 = FEdge(meshPointer->face[faceIndex].V(1)->P(), meshPointer->face[faceIndex].V(2)->P());
		}
		else if (meshPointer->face[faceIndex].V(2) == pVert)
		{
			edge1 = FEdge(meshPointer->face[faceIndex].V(1)->P(), meshPointer->face[faceIndex].V(2)->P());
			edge2 = FEdge(meshPointer->face[faceIndex].V(2)->P(), meshPointer->face[faceIndex].V(0)->P());
		}

		exited = this->exitInVector<FEdge>(edge1, this->edgeList);
		if (!exited)
		{
			this->edgeList.push_back(edge1);
		}
		exited = this->exitInVector<FEdge>(edge2, this->edgeList);
		if (!exited)
		{
			this->edgeList.push_back(edge2);
		}
	}

	return;
}

void DefiniteIntersection::collectInformation(FEdge edge)
{
	CVertexO* pVert = NULL;
	for (int i = 0; i < meshPointer->vert.size(); ++i)
	{
		if (meshPointer->vert[i].P() == edge.vertA)
		{
			pVert = &meshPointer->vert[i];
			break;
		}
	}
	if (pVert == NULL)
	{
		return;
	}

	bool exited;
	vcg::face::VFIterator<CFaceO> vfi(pVert);
	for (; !vfi.End(); ++vfi)
	{
		int faceIndex = vfi.F() - &this->meshPointer->face[0];
		FEdge edge1, edge2, edge3;
		edge1 = FEdge(meshPointer->face[faceIndex].V(0)->P(), meshPointer->face[faceIndex].V(1)->P());
		edge2 = FEdge(meshPointer->face[faceIndex].V(1)->P(), meshPointer->face[faceIndex].V(2)->P());
		edge3 = FEdge(meshPointer->face[faceIndex].V(2)->P(), meshPointer->face[faceIndex].V(0)->P());
		if (edge1 == edge)
		{
			this->faceList.push_back(faceIndex);
		}
		else if (edge2 == edge)
		{
			this->faceList.push_back(faceIndex);
		}
		else if (edge3 == edge)
		{
			this->faceList.push_back(faceIndex);
		}
	}

	return;
}

template<typename T> bool  DefiniteIntersection::exitInVector(T t, vector<T> list)
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

void DefiniteIntersection::getMeshAndUpdate(CMeshO* pMesh)
{
	this->meshPointer = pMesh;
	this->faceList.clear();
	if (!ensureEdge)
	{
		this->edgeList.clear();
	}

	if (this->edgeList.empty())
	{
		this->collectInformation(this->intersection);
	}
	else
	{
		this->collectInformation(this->edgeList[0]);
	}
}

void DefiniteIntersection::transformation(vcg::Matrix44f transform)
{
	this->intersection = transform * intersection;
	for (int i = 0; i < this->edgeList.size(); ++i)
	{
		this->edgeList[i].vertA = transform * this->edgeList[i].vertA;
		this->edgeList[i].vertB = transform * this->edgeList[i].vertB;
	}
}
