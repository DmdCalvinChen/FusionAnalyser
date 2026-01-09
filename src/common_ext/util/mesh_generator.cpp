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

#include "mesh_generator.h"

MeshGenerator::MeshGenerator()
{
}

MeshGenerator::~MeshGenerator()
{
}

void MeshGenerator::addNewMeshToAimMeshMode(std::vector<FFace> &addingFaceList, MeshModel *aimMode, int iQValue, vcg::Color4b wantColor)
{
    addNewMeshToAimMeshMode(addingFaceList, &aimMode->cm, iQValue, wantColor);
}

void MeshGenerator::addNewMeshToAimMeshMode(std::vector<FFace> &addingFaceList, CMeshO *aimMesh, int iQValue, vcg::Color4b wantColor, int default_start_index)
{
	//获得将要添加的面FFace队列
	int beginPos_vert;
	if (default_start_index < 0)
	{
		beginPos_vert = aimMesh->vert.size();
	}
	else
	{
		beginPos_vert = default_start_index;
	}

	//为每个新添加面的点在*aimMesh中分配分配CVertexO*指针
	this->generateNewVertexsAndNewFaces(addingFaceList, aimMesh, beginPos_vert);
	for (auto face : addingFaceList)
	{
		face.vertP1->Q() = iQValue;
		face.vertP2->Q() = iQValue;
		face.vertP3->Q() = iQValue;
		face.vertP1->C() = wantColor;
		face.vertP2->C() = wantColor;
		face.vertP3->C() = wantColor;
		vcg::tri::Allocator<CMeshO>::AddFace(*aimMesh, face.vertP1, face.vertP2, face.vertP3);
	}
	addingFaceList.clear();

	vcg::tri::Allocator<CMeshO>::CompactFaceVector(*aimMesh);

	// update topology
	assert(tri::HasPerVertexVFAdjacency(*aimMesh) && tri::HasPerFaceVFAdjacency(*aimMesh));
	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*aimMesh);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(*aimMesh);

	// update Normals
	vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(*aimMesh);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*aimMesh);
}

void MeshGenerator::generateNewVertexsAndNewFaces(vector<FFace> &faceList, MeshModel *aimMode, int vertStartPos)
{
	Point3f currentP;
	CVertexO *vertPointer1, *vertPointer2, *vertPointer3, *startVertPointer;
	int n1, n2, n3;
	for (int i = 0; i < faceList.size(); i++)
	{
		currentP = faceList[i].v1;
		vertPointer1 = addOrFindThisPointsPointer(currentP, vertStartPos, aimMode);
		startVertPointer = &aimMode->cm.vert[0];
		n1 = vertPointer1 - startVertPointer;

		currentP = faceList[i].v2;
		vertPointer2 = addOrFindThisPointsPointer(currentP, vertStartPos, aimMode);
		startVertPointer = &aimMode->cm.vert[0];
		n2 = vertPointer2 - startVertPointer;

		currentP = faceList[i].v3;
		vertPointer3 = addOrFindThisPointsPointer(currentP, vertStartPos, aimMode);
		startVertPointer = &aimMode->cm.vert[0];
		n3 = vertPointer3 - startVertPointer;

		vertPointer1 = &aimMode->cm.vert[n1];
		vertPointer2 = &aimMode->cm.vert[n2];
		vertPointer3 = &aimMode->cm.vert[n3];

		faceList[i].InitVertexsPointers(vertPointer1, vertPointer2, vertPointer3);
	}
}

CVertexO* MeshGenerator::addOrFindThisPointsPointer(Point3f p, int startPos, MeshModel *aimMode)
{
    return addOrFindThisPointsPointer(p, startPos, &(aimMode->cm));
}

CVertexO* MeshGenerator::addOrFindThisPointsPointer(Point3f p, int startPos, CMeshO *aimMesh)
{
	bool isNewVert = true;
	for (int i = startPos; i < aimMesh->VN(); i++)
	{
		if (aimMesh->vert[i].P() == p)
		{
			isNewVert = false;
			return &aimMesh->vert[i];
		}
	}

	if (isNewVert)
	{
		CMeshO::VertexPointer newVertPointer;
		newVertPointer = &*vcg::tri::Allocator<CMeshO>::AddVertex(*aimMesh, p);
		return newVertPointer;
	}
}

int MeshGenerator::addOrFindThisPointsPointer2(Point3f p, int startPos, CMeshO* aimMesh)
{
	for (int i = aimMesh->vert.size() - 1; i >= 0; --i)
	{
		if (aimMesh->vert[i].P() == p)
		{
			return i;
		}
	}

	CMeshO::VertexIterator vi = vcg::tri::Allocator<CMeshO>::AddVertices(*aimMesh, 1);
	vi->P() = p;
	return  aimMesh->vert.size() - 1;
}

void MeshGenerator::generateNewVertexsAndNewFaces(vector<FFace> &faceList, CMeshO *aimMesh, int vertStartPos)
{
	Point3f currentP;
	CVertexO *vertPointer1, *vertPointer2, *vertPointer3;
	for (int i = 0; i < faceList.size(); i++)
	{
		currentP = faceList[i].v1;
		vertPointer1 = addOrFindThisPointsPointer(currentP, vertStartPos, aimMesh);

		currentP = faceList[i].v2;
		vertPointer2 = addOrFindThisPointsPointer(currentP, vertStartPos, aimMesh);

		currentP = faceList[i].v3;
		vertPointer3 = addOrFindThisPointsPointer(currentP, vertStartPos, aimMesh);

		faceList[i].InitVertexsPointers(vertPointer1, vertPointer2, vertPointer3);
	}
}

void MeshGenerator::generateNewVertexsAndNewFaces2(vector<FFace>& faceList, CMeshO* aimMesh, int vertStartPos)
{
	Point3f currentP;
	int vertIndex1, vertIndex2, vertIndex3;
	for (int i = 0; i < faceList.size(); i++)
	{
		currentP = faceList[i].v1;
		vertIndex1 = addOrFindThisPointsPointer2(currentP, vertStartPos, aimMesh);

		currentP = faceList[i].v2;
		vertIndex2 = addOrFindThisPointsPointer2(currentP, vertStartPos, aimMesh);

		currentP = faceList[i].v3;
		vertIndex3 = addOrFindThisPointsPointer2(currentP, vertStartPos, aimMesh);

		faceList[i].n1 = vertIndex1;
		faceList[i].n2 = vertIndex2;
		faceList[i].n3 = vertIndex3;
	}
}

void MeshGenerator::addNewMeshToAimMesh(vector<FFace> addingFaceList, CMeshO *aimMesh, int iQValue, vcg::Color4b wantColor)
{
	aimMesh->Clear();

	vector<CMeshO::VertexPointer> ivp;
    vector<Point3f> vertList;
    this->collectNoRepeatVertexes(addingFaceList, vertList);
	CMeshO::VertexIterator vi = vcg::tri::Allocator<CMeshO>::AddVertices(*aimMesh, vertList.size());
	for (int i = 0; i < vertList.size(); ++i, ++vi)
	{
		ivp.push_back(&*vi);
		(*vi).P() = vertList[i];
		(*vi).C() = wantColor;
		(*vi).Q() = iQValue;
	}

	this->matchingFaceWithVertexPointers(addingFaceList, ivp);
	CMeshO::FaceIterator fi = vcg::tri::Allocator<CMeshO>::AddFaces(*aimMesh, addingFaceList.size());
	for (int i = 0; i < addingFaceList.size(); ++i, ++fi)
	{
		(*fi).V(0) = addingFaceList[i].vertP1;
		(*fi).V(1) = addingFaceList[i].vertP2;
		(*fi).V(2) = addingFaceList[i].vertP3;
	}

	vcg::tri::Allocator<CMeshO>::CompactEveryVector(*aimMesh);
	//assert(tri::HasFFAdjacency(*aimMesh) == false);
	if (!tri::HasFFAdjacency(*aimMesh))
		aimMesh->face.EnableFFAdjacency();
	assert(tri::HasFFAdjacency(*aimMesh) == true);

	//assert(tri::HasVFAdjacency(*aimMesh) == false);
	if (!tri::HasVFAdjacency(*aimMesh))
	{
		aimMesh->vert.EnableVFAdjacency();
		aimMesh->face.EnableVFAdjacency();
	}
	assert(tri::HasVFAdjacency(*aimMesh) == true);
	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*aimMesh);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(*aimMesh);

	// update Normals
	vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(*aimMesh);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*aimMesh);
}

void MeshGenerator::addNewMeshToAimMeshWithVertlist(const vector<Point3m> &_vertList, const vector<FFace> &_faceList, CMeshO *aimMesh, int iQValue, vcg::Color4b wantColor)
{
	aimMesh->Clear();

	vector<CMeshO::VertexPointer> ivp;
	CMeshO::VertexIterator vi = vcg::tri::Allocator<CMeshO>::AddVertices(*aimMesh, _vertList.size());
	for (int i = 0; i < _vertList.size(); ++i, ++vi)
	{
		ivp.push_back(&*vi);
		(*vi).P() = _vertList[i];
		(*vi).C() = wantColor;
		(*vi).Q() = iQValue;
	}

	CMeshO::FaceIterator fi = vcg::tri::Allocator<CMeshO>::AddFaces(*aimMesh, _faceList.size());
	for (int i = 0; i < _faceList.size(); ++i, ++fi)
	{
		(*fi).V(0) = ivp[_faceList[i].n1];
		(*fi).V(1) = ivp[_faceList[i].n2];
		(*fi).V(2) = ivp[_faceList[i].n3];
	}

	vcg::tri::Allocator<CMeshO>::CompactEveryVector(*aimMesh);
	//assert(tri::HasFFAdjacency(*aimMesh) == false);
	if (!tri::HasFFAdjacency(*aimMesh))
		aimMesh->face.EnableFFAdjacency();
	assert(tri::HasFFAdjacency(*aimMesh) == true);

	//assert(tri::HasVFAdjacency(*aimMesh) == false);
	if (!tri::HasVFAdjacency(*aimMesh))
	{
		aimMesh->vert.EnableVFAdjacency();
		aimMesh->face.EnableVFAdjacency();
	}
	assert(tri::HasVFAdjacency(*aimMesh) == true);
	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*aimMesh);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(*aimMesh);

	// update Normals
	vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(*aimMesh);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*aimMesh);
}

void MeshGenerator::addAllFaces2(const vector<FPoint> &vertList, const vector<FFace> &faceList, CMeshO *mesh)
{
	vcg::tri::Allocator<CMeshO>::AddVertices(*mesh, vertList.size());
	for (int i = 0; i < vertList.size(); ++i)
	{
		mesh->vert[i].P() = vertList[i].point;
	}
	vcg::tri::Allocator<CMeshO>::CompactVertexVector(*mesh);

	for (auto face : faceList)
	{
		if (face.n1 != face.n2 && face.n1 != face.n3 && face.n2 != face.n3)
		{
			CVertexO *vertPointer1, *vertPointer2, *vertPointer3;
			vertPointer1 = &mesh->vert[face.n1];
			vertPointer2 = &mesh->vert[face.n2];
			vertPointer3 = &mesh->vert[face.n3];
			if (vertPointer1->P() != face.v1)
			{
				bool stop = true;
			}
			if (vertPointer2->P() != face.v2)
			{
				bool stop = true;
			}
			if (vertPointer3->P() != face.v3)
			{
				bool stop = true;
			}
			vcg::tri::Allocator<CMeshO>::AddFace(*mesh, vertPointer1, vertPointer2, vertPointer3);
		}
		else
		{
			bool stop = true;
		}
	}
	updateModel(mesh);
}

void MeshGenerator::addAllFaces(vector<FFace> faceList, CMeshO *mesh)
{
	generateNewVertexsAndNewFaces2(faceList, mesh, 0);

	for (auto face : faceList)
	{
		vcg::tri::Allocator<CMeshO>::AddFace(*mesh, face.n1, face.n2, face.n3);
		//vcg::tri::Allocator<CMeshO>::AddFace(*mesh, face.v1, face.v2, face.v3);
		mesh->face.back().V(0)->C() = face.color1;
		mesh->face.back().V(1)->C() = face.color2;
		mesh->face.back().V(2)->C() = face.color3;
	}

	vcg::tri::Allocator<CMeshO>::CompactEveryVector(*mesh);

	//update topology
	assert(tri::HasFFAdjacency(*mesh) == false);
	mesh->face.EnableFFAdjacency();
	assert(tri::HasFFAdjacency(*mesh) == true);

	assert(tri::HasVFAdjacency(*mesh) == false);
	mesh->vert.EnableVFAdjacency();
	mesh->face.EnableVFAdjacency();
	assert(tri::HasVFAdjacency(*mesh) == true);
	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*mesh);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(*mesh);

	// update Normals
	vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(*mesh);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*mesh);
	vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalized(*mesh);
}

void MeshGenerator::collectNoRepeatVertexes(const std::vector<FFace> &facelist, std::vector<Point3f> &result)
{
    bool repeat = false;
    for (auto face : facelist)
    {
        repeat = false;
        for (auto vert : result)
        {
            if (vert == face.v1)
            {
                repeat = true;
                break;
            }
        }
        if (!repeat)
        {
            result.push_back(face.v1);
        }

        repeat = false;
        for (auto vert : result)
        {
            if (vert == face.v2)
            {
                repeat = true;
                break;
            }
        }
        if (!repeat)
        {
            result.push_back(face.v2);
        }

        repeat = false;
        for (auto vert : result)
        {
            if (vert == face.v3)
            {
                repeat = true;
                break;
            }
        }
        if (!repeat)
        {
            result.push_back(face.v3);
        }
    }
}

void MeshGenerator::matchingFaceWithVertexPointers(vector<FFace> &facelist, const vector<CMeshO::VertexPointer> &vertPointerList)
{
	for (int i = 0; i < facelist.size(); ++i)
	{
		for (int j = 0; j < vertPointerList.size(); ++j)
		{
			if (vertPointerList[j]->P() == facelist[i].v1)
			{
				facelist[i].vertP1 = vertPointerList[j];
				break;
			}
		}

		for (int j = 0; j < vertPointerList.size(); ++j)
		{
			if (vertPointerList[j]->P() == facelist[i].v2)
			{
				facelist[i].vertP2 = vertPointerList[j];
				break;
			}
		}

		for (int j = 0; j < vertPointerList.size(); ++j)
		{
			if (vertPointerList[j]->P() == facelist[i].v3)
			{
				facelist[i].vertP3 = vertPointerList[j];
				break;
			}
		}
	}
}

vector<int> MeshGenerator::getOneRingNeighborVerts(int ivert, CMeshO *curMesh)
{
	vector<int> result;

	vcg::face::VFIterator<CFaceO> vfi(&curMesh->vert[ivert]);
	for (; !vfi.End(); ++vfi)
	{
		int indexF = vfi.F() - &curMesh->face[0];

		int tempVI[3];
		tempVI[0] = curMesh->face[indexF].V(0) - &curMesh->vert[0];
		tempVI[1] = curMesh->face[indexF].V(1) - &curMesh->vert[0];
		tempVI[2] = curMesh->face[indexF].V(2) - &curMesh->vert[0];

		//if the i-th tempVI is new vertex index Of the verList[i](meshPoint3f)
		for (int i = 0; i < 3; i++)
		{
			if (tempVI[i] != ivert)
			{
				bool bIsNewAdjVertex = true;

				for (auto index : result)
				{
					if (tempVI[i] == index)
					{
						bIsNewAdjVertex = false;
						break;
					}
				}
				if (bIsNewAdjVertex)
				{
					result.push_back(tempVI[i]);
				}
			}
		}
	}

	return result;
}

vector<int> MeshGenerator::getOneRingNeighborFaces(int ivert, CMeshO *curMesh)
{
	vector<int> result;

	vcg::face::VFIterator<CFaceO> vfi(&curMesh->vert[ivert]);
	for (; !vfi.End(); ++vfi)
	{
		int indexF = vfi.F() - &curMesh->face[0];
		result.push_back(indexF);
	}

	return result;
}

void MeshGenerator::fixReversePatch(vector<int> _seeds, CMeshO *_curMesh)
{
	vector<int> state_list(_curMesh->FN(), 0);
	for (auto index : _seeds)
	{
		state_list[index] = 1;
	}

	Point3m self_edge_vec, other_edge_vec;
	vector<int> next_generation_seeds;
	while (!_seeds.empty())
	{
		for (auto index : _seeds)
		{
			int f0 = _curMesh->face[index].FFi(0);
			if (state_list[f0] == 0)
			{
				next_generation_seeds.push_back(f0);
				self_edge_vec = _curMesh->face[index].V(1)->P() - _curMesh->face[index].V(0)->P();
				if (_curMesh->face[f0].FFi(0) == index)
				{
					other_edge_vec = _curMesh->face[f0].V(1)->P() - _curMesh->face[f0].V(0)->P();
				}
				else if (_curMesh->face[f0].FFi(1) == index)
				{
					other_edge_vec = _curMesh->face[f0].V(2)->P() - _curMesh->face[f0].V(1)->P();
				}
				else if (_curMesh->face[f0].FFi(2) == index)
				{
					other_edge_vec = _curMesh->face[f0].V(0)->P() - _curMesh->face[f0].V(2)->P();
				}

				if (self_edge_vec * other_edge_vec > 0)
				{
					state_list[f0] = state_list[index] * -1;
				}
				else
				{
					state_list[f0] = state_list[index];
				}
			}

			int f1 = _curMesh->face[index].FFi(1);
			if (state_list[f1] == 0)
			{
				next_generation_seeds.push_back(f1);
				self_edge_vec = _curMesh->face[index].V(2)->P() - _curMesh->face[index].V(1)->P();
				if (_curMesh->face[f1].FFi(0) == index)
				{
					other_edge_vec = _curMesh->face[f1].V(1)->P() - _curMesh->face[f1].V(0)->P();
				}
				else if (_curMesh->face[f1].FFi(1) == index)
				{
					other_edge_vec = _curMesh->face[f1].V(2)->P() - _curMesh->face[f1].V(1)->P();
				}
				else if (_curMesh->face[f1].FFi(2) == index)
				{
					other_edge_vec = _curMesh->face[f1].V(0)->P() - _curMesh->face[f1].V(2)->P();
				}

				if (self_edge_vec * other_edge_vec > 0)
				{
					state_list[f1] = state_list[index] * -1;
				}
				else
				{
					state_list[f1] = state_list[index];
				}
			}

			int f2 = _curMesh->face[index].FFi(2);
			if (state_list[f2] == 0)
			{
				next_generation_seeds.push_back(f2);
				self_edge_vec = _curMesh->face[index].V(0)->P() - _curMesh->face[index].V(2)->P();
				if (_curMesh->face[f2].FFi(0) == index)
				{
					other_edge_vec = _curMesh->face[f2].V(1)->P() - _curMesh->face[f2].V(0)->P();
				}
				else if (_curMesh->face[f2].FFi(1) == index)
				{
					other_edge_vec = _curMesh->face[f2].V(2)->P() - _curMesh->face[f2].V(1)->P();
				}
				else if (_curMesh->face[f2].FFi(2) == index)
				{
					other_edge_vec = _curMesh->face[f2].V(0)->P() - _curMesh->face[f2].V(2)->P();
				}

				if (self_edge_vec * other_edge_vec > 0)
				{
					state_list[f2] = state_list[index] * -1;
				}
				else
				{
					state_list[f2] = state_list[index];
				}
			}
		}

		_seeds = next_generation_seeds;
		vector<int>().swap(next_generation_seeds);
	}

	for (int i = 0; i < state_list.size(); ++i)
	{
		if (state_list[i] < 0)
		{
			swap(_curMesh->face[i].V(1), _curMesh->face[i].V(2));
		}
	}
	updateModel(_curMesh);
}

void MeshGenerator::updateModel(CMeshO *curMesh)
{
	if (curMesh == nullptr) return;
	vcg::tri::Allocator<CMeshO>::CompactEveryVector(*curMesh);

	//update topology
	assert(tri::HasFFAdjacency(*curMesh) == false);
    if(!tri::HasFFAdjacency(*curMesh))
        curMesh->face.EnableFFAdjacency();
	assert(tri::HasFFAdjacency(*curMesh) == true);

	assert(tri::HasVFAdjacency(*curMesh) == false);
    if (!tri::HasVFAdjacency(*curMesh))
    {
        curMesh->vert.EnableVFAdjacency();
        curMesh->face.EnableVFAdjacency();
    }
	assert(tri::HasVFAdjacency(*curMesh) == true);
	curMesh->vert.EnableCurvature();

	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*curMesh);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(*curMesh);

	// update Normals
	vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(*curMesh);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*curMesh);
}

void MeshGenerator::deleteFacesInList(vector<int> deleteFaceList, CMeshO *curMesh)
{
	for (int i = 0; i < deleteFaceList.size(); i++)
	{
		vcg::tri::Allocator<CMeshO>::DeleteFace(*curMesh, curMesh->face[deleteFaceList[i]]);
	}

	// update topology
	assert(vcg::tri::HasPerVertexVFAdjacency(*curMesh) && vcg::tri::HasPerFaceVFAdjacency(*curMesh));
	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*curMesh);

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
	assert(tri::HasPerVertexVFAdjacency(*curMesh) && tri::HasPerFaceVFAdjacency(*curMesh));
	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*curMesh);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(*curMesh);

	// update Normals
	vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(*curMesh);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*curMesh);
}

void MeshGenerator::meshDeformer(CMeshO *curMesh, Axis curSys, Axis aimSys)
{
	Point3m moveVector(0, 0, 0), rotateVector(0, 0, 0);
	curSys.computeTransformVectors(aimSys, moveVector, rotateVector);

	vcg::Matrix44f transMat;
	vcg::Matrix44f transform, transformInverse;
	transform.SetIdentity();

	transform.SetTranslate(curSys.centerPoint);
	transformInverse.SetTranslate(-curSys.centerPoint);

	vcg::Matrix44f transformValue, rotMatGlobal, rotMatLocal;
	transMat.SetIdentity();
	rotMatGlobal.SetIdentity();
	rotMatLocal.SetIdentity();
	Point3f trans;

	trans = curSys.axisXVector * moveVector.X();
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	trans = curSys.axisYVector * moveVector.Y();
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	trans = curSys.axisZVector * moveVector.Z();
	transformValue.SetTranslate(trans);
	transMat *= transformValue;

	transformValue.SetRotateDeg(rotateVector.X(), curSys.axisXVector);
	rotMatLocal *= transformValue;
	transformValue.SetRotateDeg(rotateVector.Y(), curSys.axisYVector);
	rotMatLocal *= transformValue;
	transformValue.SetRotateDeg(rotateVector.Z(), curSys.axisZVector);
	rotMatLocal *= transformValue;

	transform *= transMat * rotMatLocal;
	transform *= transformInverse;

	meshDeformer(curMesh, transform);
}

void MeshGenerator::meshDeformer(CMeshO *curMesh, const vcg::Matrix44f &_matrix)
{
	for (int i = 0; i < curMesh->vert.size(); ++i)
	{
		curMesh->vert[i].P() = _matrix * curMesh->vert[i].P();
	}
	this->updateModel(curMesh);
}

void MeshGenerator::meshAppend(CMeshO *_augend_mesh, CMeshO *_addend_mesh)
{
	if (!_augend_mesh || !_addend_mesh)
	{
		return;
	}
	int startPos = _augend_mesh->vert.size();
	CMeshO::VertexIterator vi = vcg::tri::Allocator<CMeshO>::AddVertices(*_augend_mesh, _addend_mesh->vert.size());
	for (int i = 0; i < _addend_mesh->vert.size(); ++i, ++vi)
	{
		(*vi).P() = _addend_mesh->vert[i].P();
		(*vi).C() = _addend_mesh->vert[i].C();
		(*vi).Q() = _addend_mesh->vert[i].Q();
	}

	int iv0, iv1, iv2;
	CMeshO::FaceIterator fi = vcg::tri::Allocator<CMeshO>::AddFaces(*_augend_mesh, _addend_mesh->face.size());
	for (int i = 0; i < _addend_mesh->face.size(); ++i, ++fi)
	{
		iv0 = _addend_mesh->face[i].V(0) - &_addend_mesh->vert[0];
		iv1 = _addend_mesh->face[i].V(1) - &_addend_mesh->vert[0];
		iv2 = _addend_mesh->face[i].V(2) - &_addend_mesh->vert[0];
		(*fi).V(0) = &_augend_mesh->vert[startPos + iv0];
		(*fi).V(1) = &_augend_mesh->vert[startPos + iv1];
		(*fi).V(2) = &_augend_mesh->vert[startPos + iv2];
	}
	updateModel(_augend_mesh);
}
