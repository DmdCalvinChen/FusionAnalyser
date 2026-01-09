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

#include "AbsMesh.h"
#include "common/config.h"
#include "common/meshmodel.h"
#include "util/utility_tools.h"
#include "common_base/SignalManager.h"

AbsMesh::AbsMesh()
{
	BBTreeRoot = NULL;
	BBLeafList = NULL;
	BBNodeList = NULL;
}

AbsMesh::AbsMesh(CMeshO *_pmesh, MeshModel* _pmodel)
{
	this->p_mesh_ = _pmesh;
	this->p_model_ = _pmodel;
	move_vec_ = Point3m(0, 0, 0);
	rotate_vec_ = Point3m(0, 0, 0);
	this->transformMatrix_.SetIdentity();
}

AbsMesh::AbsMesh(MeshModel *pmodel)
{
    assert(pmodel);
    p_model_ = pmodel;
    p_mesh_ = &pmodel->cm;
	BBTreeRoot = NULL;
	BBLeafList = NULL;
	BBNodeList = NULL;
	this->moveVector = Point3m(0, 0, 0);
	this->rotateVector = Point3m(0, 0, 0);
	move_vec_ = Point3m(0, 0, 0);
	rotate_vec_ = Point3m(0, 0, 0);
	this->transformMatrix_.SetIdentity();
}

AbsMesh::~AbsMesh()
{
    SAFE_DELETE_ARRAY(BBNodeList);
    SAFE_DELETE_ARRAY(BBLeafList);
    SAFE_DELETE_ARRAY(leafIndexList);
    SAFE_DELETE(BBTreeRoot);
}

void AbsMesh::inital(CMeshO *_mesh)
{
	p_mesh_ = _mesh;
	BBTreeRoot = NULL;
	BBLeafList = NULL;
	BBNodeList = NULL;
	this->moveVector = Point3m(0, 0, 0);
	this->rotateVector = Point3m(0, 0, 0);
	move_vec_ = Point3m(0, 0, 0);
	rotate_vec_ = Point3m(0, 0, 0);
	this->transformMatrix_.SetIdentity();
}

void AbsMesh::getRootOfBBTree(CMeshO *pTempMesh)
{
	// Clear all non-leaf and leaf nodes
    SAFE_DELETE_ARRAY(BBNodeList);
    SAFE_DELETE_ARRAY(BBLeafList);
    SAFE_DELETE_ARRAY(leafIndexList);

    CMeshO * cmesh = nullptr;
    if (pTempMesh == nullptr)
    {
        cmesh = p_mesh_;
    }
    else
    {
        cmesh = pTempMesh;
    }
	// Initialize leaf nodes
    int fnNum = cmesh->FN();
	leafIndexList = new int[fnNum];
	int I = 0;
	for (int i = 0; i < cmesh->face.size(); i++)
	{
		if (!cmesh->face[i].IsD())
		{
			leafIndexList[I] = i;
			I++;
		}
	}
	this->BBLeafList = new MeshBoundingBox[cmesh->FN()];
	for (int i = 0; i < fnNum; i++)
	{
		int tempIndex;
		tempIndex = leafIndexList[i];
		this->bulidLeafBBNode(tempIndex, this->BBLeafList[i], cmesh);
	}

	// Build bounding box tree bottom-up
	this->BBTreeRoot = MeshBoundingBox::autoBuildTree(fnNum, this->BBLeafList, this->BBNodeList);
}

void AbsMesh::bulidLeafBBNode(int FIndex, MeshBoundingBox &BBox, AbsMesh* souce)
{
	BBox.initBoundBox(FIndex, false, souce->p_mesh_->face.at(FIndex), NULL, NULL);
}

void AbsMesh::bulidLeafBBNode(int FIndex, MeshBoundingBox &BBox, CMeshO* souce)
{
	Point3m a = souce->face.at(FIndex).V(0)->P();
	BBox.initBoundBox(FIndex, false, souce->face.at(FIndex), NULL, NULL);
}

int** AbsMesh::getCollideObjectMatrix(MeshBoundingBox &aimBox, int &rowNum, int &columnNum)
{
	int** COM; // Collide Object Matrix

	// Get all leaf nodes from this bounding box
	int cia[30000]; // Column Index Array
	int ria[100]; // Row Index Array
	int counterA = 0;
	this->BBTreeRoot->getAllMixedLeaf(aimBox, counterA, cia);

	rowNum = counterA;
	columnNum = 6;
	// Initialize COM Matrix
	COM = new int*[counterA];
	for (int i = 0; i < counterA; i++)
		COM[i] = new int[columnNum];
	for (int i = 0; i < counterA; i++)
		for (int j = 0; j < columnNum; j++)
		{
			if (j == 0)
				COM[i][j] = cia[i];
			else
				COM[i][j] = -1;
		}

	// Get each leaf node from aimBox
	int counterB = 0;
	for (int i = 0; i < counterA; i++)
	{
		// Initialize ria
		for (int j = 0; j < 100; j++)
			ria[j] = -1;

		int ii;
		for (int j = 0; j < p_mesh_->FN(); j++)//求出cia[i]所代表的待定相交面在A树种的叶子编号
		{
			if (cia[i] == leafIndexList[j])
			{
				ii = j;
				break;
			}
		}
		aimBox.getAllMixedLeaf(this->BBLeafList[ii], counterB, ria);
		if (counterB > 0)
		{
			for (int j = 0; j < 5; j++)
			{
				COM[i][j + 1] = ria[j];
			}

			counterB = 0;
		}
		else
		{
			COM[i][0] = -1;
			counterB = 0;
		}
	}
	return COM;
}

int** AbsMesh::getCollideObjectMatrix(MeshBoundingBox *aimBox, int &rowNum, int &columnNum)
{
	int** COM = NULL; // Collide Object Matrix

	// Get all leaf nodes from this bounding box
	int cia[30000]; // Column Index Array
	int ria[100]; // Row Index Array
	int counterA = 0;
	this->BBTreeRoot->getAllMixedLeaf(*aimBox, counterA, cia);

	rowNum = counterA;
	columnNum = 20;
	// Initialize COM Matrix
	COM = new int*[counterA];
	for (int i = 0; i < counterA; i++)
	{
		COM[i] = NULL;
		COM[i] = new int[columnNum];
	}

	for (int i = 0; i < counterA; i++)
		for (int j = 0; j < columnNum; j++)
		{
			if (j == 0)
				COM[i][j] = cia[i];
			else
				COM[i][j] = -1;
		}

	// Get each leaf node from aimBox
	int counterB = 0;
	for (int i = 0; i < counterA; i++)
	{
		// Initialize ria
		for (int j = 0; j < 100; j++)
			ria[j] = -1;

		int ii;
		for (int j = 0; j < p_mesh_->FN(); j++) // Find leaf index of face represented by cia[i] in tree A
		{
			if (cia[i] == leafIndexList[j])
			{
				ii = j;
				break;
			}
		}
		aimBox->getAllMixedLeaf(this->BBLeafList[ii], counterB, ria);
		if (counterB > 0)
		{
			for (int j = 0; j < columnNum - 1; j++)
			{
				COM[i][j + 1] = ria[j];
			}

			counterB = 0;
		}
		else
		{
			COM[i][0] = -1;
			rowNum--;
			counterB = 0;
		}
	}

	int** CM = NULL;
	CM = new int*[rowNum];
	for (int i = 0; i < rowNum; i++)
	{
		CM[i] = new int[columnNum];
	}

	int m = 0;
	// Remove all rows starting with -1
	for (int i = 0; i < counterA; i++)
	{
		if (COM[i][0] != -1)
		{
			for (int j = 0; j < columnNum; j++)
			{
				CM[m][j] = COM[i][j];
			}
			m++;
		}
	}

	for (int i = 0; i < counterA; i++)
	{
		if (COM[i] != NULL)
		{
			delete[]COM[i];
			COM[i] = NULL;
		}
	}
	if (COM != NULL)
	{
		delete[]COM;
		COM = NULL;
	}

	return CM;
}

int** AbsMesh::getCollideObjectMatrix(MeshBoundingBox *aimBox, int &rowNum, int &columnNum, vector<int> &waitingFace)
{
	vector<int> leafList;
	this->BBTreeRoot->getAllMixedLeaf(*aimBox, leafList);
	int counterA = leafList.size();
	int *cia = new int[counterA];
	for (int i = 0; i < counterA; i++)
	{
		cia[i] = leafList[i];
	}
	leafList.clear();

	// Initialize COM Matrix
	columnNum = 20;
	rowNum = counterA;
	int** COM = new int*[counterA];
	for (int i = 0; i < counterA; i++)
	{
		COM[i] = new int[columnNum];
	}

	for (int i = 0; i < counterA; i++)
	{
		for (int j = 0; j < columnNum; j++)
		{
			if (j == 0)
			{
				waitingFace.push_back(cia[i]);
				COM[i][j] = cia[i];
			}
			else
			{
				COM[i][j] = -1;
			}
		}
	}

	// Get each leaf node from aimBox
	int counterB = 0;
	int ria[100]; // Row Index Array
	for (int i = 0; i < counterA; i++)
	{
		// Initialize ria
		for (int j = 0; j < 100; j++)
			ria[j] = -1;

		int ii;
		for (int j = 0; j < p_mesh_->FN(); j++) // Find leaf index of face represented by cia[i] in tree A
		{
			if (cia[i] == leafIndexList[j])
			{
				ii = j;
				break;
			}
		}
		aimBox->getAllMixedLeaf(this->BBLeafList[ii], counterB, ria);
		if (counterB > 0)
		{
			for (int j = 0; j < columnNum - 1; j++)
			{
				COM[i][j + 1] = ria[j];
			}

			counterB = 0;
		}
		else
		{
			COM[i][0] = -1;
			rowNum--;
			counterB = 0;
		}
	}

	int** CM = new int*[rowNum];
	for (int i = 0; i < rowNum; i++)
	{
		CM[i] = new int[columnNum];
	}

	int m = 0;
	// Remove all rows starting with -1
	for (int i = 0; i < counterA; i++)
	{
		if (COM[i][0] != -1)
		{
			for (int j = 0; j < columnNum; j++)
			{
				CM[m][j] = COM[i][j];
			}
			m++;
		}
	}

	for (int i = 0; i < counterA; i++)
	{
		if (COM[i] != NULL)
		{
			delete[]COM[i];
			COM[i] = NULL;
		}
	}
	if (COM != NULL)
	{
		delete[]COM;
		COM = NULL;
	}

	return CM;
}

void AbsMesh::getCollideObjectMatrix(MeshBoundingBox *aimBox, vector<Point2m> &result)
{
	result.clear();

	// Get all leaf nodes from this bounding box
	vector<int> leafA;
	this->BBTreeRoot->getAllMixedLeafVector(*aimBox, leafA);

	// Get each leaf node from aimBox
	int counterB = 0;
	for (int i = 0; i < leafA.size(); i++)
	{
		int ii;
		for (int j = 0; j < p_mesh_->FN(); j++) // Find leaf index of face represented by leafA[i] in tree A
		{
			if (leafA[i] == leafIndexList[j])
			{
				ii = j;
				break;
			}
		}

		vector<int> leafB;
		aimBox->getAllMixedLeafVector(this->BBLeafList[ii], leafB);
		if (leafB.size() > 0)
		{
			for (int j = 0; j < leafB.size(); j++)
			{
				result.push_back(Point2m(leafA[i], leafB[j]));
			}

			leafB.clear();
		}
	}

	leafA.clear();
	return;
}

void AbsMesh::getCollideObjectMatrixRough(MeshBoundingBox *aimBox, vector<int> &result)
{
	// Get all leaf nodes from this bounding box
	vector<int> leafA;
	this->BBTreeRoot->getAllMixedLeafVector(*aimBox, leafA);

	result = leafA;
	leafA.clear();
	return;
}

void AbsMesh::rotateArbitraryLine(double pOut[16], Point3m v1, Point3m v2, float theta)
{
	float a = v1.X();
	float b = v1.Y();
	float c = v1.Z();

	Point3m p = v2 - v1;
	p = p.Normalize();
	float u = p.X();
	float v = p.Y();
	float w = p.Z();

	float uu = u * u;
	float uv = u * v;
	float uw = u * w;
	float vv = v * v;
	float vw = v * w;
	float ww = w * w;
	float au = a * u;
	float av = a * v;
	float aw = a * w;
	float bu = b * u;
	float bv = b * v;
	float bw = b * w;
	float cu = c * u;
	float cv = c * v;
	float cw = c * w;

	float costheta = cosf(theta);
	float sintheta = sinf(theta);

	pOut[0] = uu + (vv + ww) * costheta;
	pOut[1] = uv * (1 - costheta) + w * sintheta;
	pOut[2] = uw * (1 - costheta) - v * sintheta;
	pOut[3] = 0;

	pOut[4] = uv * (1 - costheta) - w * sintheta;
	pOut[5] = vv + (uu + ww) * costheta;
	pOut[6] = vw * (1 - costheta) + u * sintheta;
	pOut[7] = 0;

	pOut[8] = uw * (1 - costheta) + v * sintheta;
	pOut[9] = vw * (1 - costheta) - u * sintheta;
	pOut[10] = ww + (uu + vv) * costheta;
	pOut[11] = 0;

	pOut[12] = (a * (vv + ww) - u * (bv + cw)) * (1 - costheta) + (bw - cv) * sintheta;
	pOut[13] = (b * (uu + ww) - v * (au + cw)) * (1 - costheta) + (cu - aw) * sintheta;
	pOut[14] = (c * (uu + vv) - w * (au + bv)) * (1 - costheta) + (av - bu) * sintheta;
	pOut[15] = 1;
}

void AbsMesh::rotateArbitraryAxis(double pOut[16], Point3m axis, float theta)
{
	axis = axis.Normalize();
	float u = axis.X();
	float v = axis.Y();
	float w = axis.Z();

	pOut[0] = cosf(theta) + (u * u) * (1 - cosf(theta));
	pOut[1] = u * v * (1 - cosf(theta)) + w * sinf(theta);
	pOut[2] = u * w * (1 - cosf(theta)) - v * sinf(theta);
	pOut[3] = 0;

	pOut[4] = u * v * (1 - cosf(theta)) - w * sinf(theta);
	pOut[5] = cosf(theta) + v * v * (1 - cosf(theta));
	pOut[6] = w * v * (1 - cosf(theta)) + u * sinf(theta);
	pOut[7] = 0;

	pOut[8] = u * w * (1 - cosf(theta)) + v * sinf(theta);
	pOut[9] = v * w * (1 - cosf(theta)) - u * sinf(theta);
	pOut[10] = cosf(theta) + w * w * (1 - cosf(theta));
	pOut[11] = 0;

	pOut[12] = 0;
	pOut[13] = 0;
	pOut[14] = 0;
	pOut[15] = 1;
}

void AbsMesh::rotateOnePoint(Point3m origionCopy, Point3m axis, float angle, Point3m &P)
{
	vcg::Matrix44f transform, transformInverse;
	transform.SetIdentity();
	transformInverse.SetIdentity();
	transform.SetTranslate(origionCopy);
	transformInverse.SetTranslate(-origionCopy);

	vcg::Matrix44f transformValue, rotMatLocal;
	rotMatLocal.SetIdentity();
	vcg::Point3f trans;

	transformValue.SetRotateDeg(angle, axis);
	rotMatLocal *= transformValue;

	transform *= rotMatLocal;
	transform *= transformInverse;

	P = transform * P;
}

void AbsMesh::clearAllLeavesBuffer()
{
	// Clear all non-leaf and leaf nodes
	if (this->BBNodeList != NULL)
	{
		delete[] this->BBNodeList;
		this->BBNodeList = NULL;
	}
	if (this->BBLeafList != NULL)
	{
		delete[] this->BBLeafList;
		this->BBLeafList = NULL;
	}
	if (this->leafIndexList != NULL)
	{
		delete[] leafIndexList;
		this->leafIndexList = NULL;
	}
}

vector<vector<FFace>> AbsMesh::getSplitResultOnTriangle(int faceIndex)
{
	vector<vector<FFace>> splitStepList;
	for (auto splitFace : this->machine.splitTrianglesA)
	{
		if (splitFace.index == faceIndex)
		{
			splitFace.collectEdgeCombed();
			vector<FEdge> tempEdgeList;
			splitStepList = machine.splitMachine.DelaunayMachineArbitraryTurnOn2Traceable(splitFace.splitPoints, splitFace.splitEdges,
				FFace(splitFace.pMesh->face[splitFace.index].V(0)->P(), splitFace.pMesh->face[splitFace.index].V(1)->P(), splitFace.pMesh->face[splitFace.index].V(2)->P()));

			return splitStepList;
		}
	}
	return splitStepList;
}

vector<FFace> AbsMesh::isItTriangleOutofModel(int faceIndex, bool& bOutSide, AbsMesh aimMesh)
{
	vector<FFace> result;

	bOutSide = machine.judgeOnFaceSpace(faceIndex, this->p_mesh_, aimMesh.p_mesh_, result);

	return result;
}

Point3m AbsMesh::getProjPointOnPlane(Point3m p, Point3m D, Point3f pos, Point3m N)
{
	Point3m result;

	float up, down;
	up = (pos - p) * N;
	down = D * N;
	float t = up / down;

	result = p + D * t;
	return result;
}

vector<FEdge> AbsMesh::collectEachVertexClosestVert(CMeshO *curMesh, CMeshO *aimMesh)
{
	vector<FEdge> result;

	tri::RequireCompactness(*aimMesh);
	VertexConstDataWrapper<CMeshO> ww(*aimMesh);
	KdTree<float> kt(ww);
	typename KdTree<float>::PriorityQueue pq;
	int neighbor = 2;

	int idA, idB, index;
	float distanceA, distanceB;
	for (int i = 0; i < curMesh->vert.size(); ++i)
	{
		Point3m vPos = curMesh->vert[i].P();

		kt.doQueryK(vPos, neighbor, pq);

		idA = pq.getIndex(0);
		idB = pq.getIndex(1);
		distanceA = (aimMesh->vert[idA].P() - vPos) * (aimMesh->vert[idA].P() - vPos);
		distanceB = (aimMesh->vert[idB].P() - vPos) * (aimMesh->vert[idB].P() - vPos);
		index = distanceA < distanceB ? idA : idB;

		result.push_back(FEdge(vPos, aimMesh->vert[index].P()));
	}
	return result;
}

void AbsMesh::updateModel(CMeshO *curMesh)
{
	vcg::tri::Allocator<CMeshO>::CompactEveryVector(*curMesh);

	// Update topology
	if (!tri::HasFFAdjacency(*curMesh))
		curMesh->face.EnableFFAdjacency();
	assert(tri::HasFFAdjacency(*curMesh) == true);

	if (!tri::HasVFAdjacency(*curMesh))
	{
		curMesh->vert.EnableVFAdjacency();
		curMesh->face.EnableVFAdjacency();
	}
	assert(tri::HasVFAdjacency(*curMesh) == true);
	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*curMesh);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(*curMesh);

	// Update normals
	vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(*curMesh);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*curMesh);
}

void AbsMesh::meshDeformer(CMeshO *curMesh, Axis curSys, Axis aimSys)
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

	for (int i = 0; i < curMesh->vert.size(); ++i)
	{
		curMesh->vert[i].P() = transform * curMesh->vert[i].P();
	}
	this->updateModel(curMesh);
}

void AbsMesh::zoom(CMeshO *curMesh, Axis curSys, Point3m zoomVector)
{
	vcg::Matrix44f transMat;
	vcg::Matrix44f transform, transformInverse;
	transform.SetIdentity();
	transform.SetTranslate(curSys.centerPoint);
	transformInverse.SetTranslate(-curSys.centerPoint);

	vcg::Matrix44f transformValue, rotMatLocal, zoomMatLocal;
	transMat.SetIdentity();
	rotMatLocal.SetIdentity();
	zoomMatLocal.SetIdentity();
	Point3f trans;

	transformValue = this->setZoomDeg(zoomVector.X(), curSys.axisXVector);
	zoomMatLocal *= transformValue;
	transformValue = this->setZoomDeg(zoomVector.Y(), curSys.axisYVector);
	zoomMatLocal *= transformValue;
	transformValue = this->setZoomDeg(zoomVector.Z(), curSys.axisZVector);
	zoomMatLocal *= transformValue;

	transform *= rotMatLocal * zoomMatLocal * transMat;
	transform *= transformInverse;

	for (int i = 0; i < curMesh->vert.size(); i++)
	{
		if (!curMesh->vert[i].IsD())
		{
			curMesh->vert[i].P() = transform * curMesh->vert[i].P();
		}
	}
	this->updateModel(curMesh);
}

vcg::Matrix44f AbsMesh::setZoomDeg(float value, Point3m axis)
{
	float rotateValue;
	Point3m A, B, rotateAxis;
	A = axis.Normalize();
	B = Point3m(1, 0, 0);
	rotateAxis = (B^A).Normalize();
	float dotMulit = A * B;
	if (abs(dotMulit) > 1)
	{
		if (dotMulit > 0)
		{
			dotMulit = 1.0f;
		}
		else
		{
			dotMulit = -1.0f;
		}
	}
	rotateValue = acosf(dotMulit);
	rotateValue = (rotateValue * 180.0f) / PI;

	vcg::Matrix44f rotMatLocal, rotMatReturn, zoomLocal;
	rotMatLocal.SetRotateDeg(rotateValue, rotateAxis);
	rotMatReturn.SetRotateDeg(-rotateValue, rotateAxis);
	zoomLocal.SetScale(value, 1, 1);
	return rotMatLocal * zoomLocal * rotMatReturn;
}

void AbsMesh::addChild(AbsMesh *_node)
{
    _node->setParent(this);
	_node->p_father_trans_matrix_ = &p_model_->cm.Tr;
	_node->updateModelRenderingMatrix();
	this->children_.push_back(_node);
}

void AbsMesh::removeChild(AbsMesh *_node)
{
	vector<AbsMesh*>::iterator ite = children_.begin();
	while (ite != children_.end())
	{
		if ((*ite)->p_model_->id() == _node->p_model_->id())
		{
			children_.erase(ite);
			break;
		}
		++ite;
	}
}

void AbsMesh::unbundingFromParent()
{
	if (p_parent_ != nullptr)
	{
		p_parent_->removeChild(this);
	}
}

void AbsMesh::updateModelRenderingMatrix()
{
    if (p_father_trans_matrix_)
    {
		p_model_->cm.Tr = (*p_father_trans_matrix_) * transformMatrix_;
    }
	else
	{
		p_model_->cm.Tr = transformMatrix_;
	}

	if (!children_.empty())
	{
		for (int i = 0; i < children_.size(); ++i)
		{
			children_[i]->updateModelRenderingMatrix();
		}
	}
}

Axis AbsMesh::updateRealTimeAxisSys(vcg::Matrix44f transformMatrix)
{
	// Transform localAxis to get current realTimeAxis
	Point3m center, axisXVert, axisYVert, axisZVert;
	center = this->localAxis.centerPoint;
	axisXVert = center + this->localAxis.axisXVector;
	axisYVert = center + this->localAxis.axisYVector;
	axisZVert = center + this->localAxis.axisZVector;

	center = transformMatrix * center;
	axisXVert = ((transformMatrix * axisXVert) - center).Normalize();
	axisYVert = ((transformMatrix * axisYVert) - center).Normalize();
	axisZVert = ((transformMatrix * axisZVert) - center).Normalize();

	return Axis(center, axisXVert, axisYVert, axisZVert);
}

vcg::Matrix44f AbsMesh::getTransformMatrix(HexaVec _transHexaVec, Axis _axis)
{
	vcg::Matrix44f transMat;
	vcg::Matrix44f transform, transformInverse;
	transform.SetTranslate(_axis.centerPoint);
	transformInverse.SetTranslate(-_axis.centerPoint);

	vcg::Matrix44f transformValue, rotMatGlobal, rotMatLocal;
	transMat.SetIdentity();
	rotMatGlobal.SetIdentity();
	rotMatLocal.SetIdentity();
	Point3f trans;

	trans = _axis.axisXVector * _transHexaVec.fVX;
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	trans = _axis.axisYVector * _transHexaVec.fVY;
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	trans = _axis.axisZVector * _transHexaVec.fVZ;
	transformValue.SetTranslate(trans);
	transMat *= transformValue;

	transformValue.SetRotateDeg(_transHexaVec.fA, _axis.axisXVector);
	rotMatLocal *= transformValue;
	transformValue.SetRotateDeg(_transHexaVec.fB, _axis.axisYVector);
	rotMatLocal *= transformValue;
	transformValue.SetRotateDeg(_transHexaVec.fG, _axis.axisZVector);
	rotMatLocal *= transformValue;

	Matrix44f zoom_matrix;
	zoom_matrix.SetIdentity();
	if (_transHexaVec.fZX != 1 ||
		_transHexaVec.fZY != 1 ||
		_transHexaVec.fZZ != 1)
	{
		zoom_matrix = getZoomMatrixAccordZoomVec(_transHexaVec.fZX, _transHexaVec.fZY, _transHexaVec.fZZ);
	}

	transform *= transMat * rotMatLocal * zoom_matrix;
	transform *= transformInverse;

	return transform;
}

vcg::Matrix44f AbsMesh::getZoomMatrixAccordZoomVec(const float &_x, const float &_y, const float &_z)
{
	Matrix44f zoomMatLocal;
	zoomMatLocal.SetIdentity();
	zoomMatLocal *= this->setZoomDeg(_x, localAxis.axisXVector);
	zoomMatLocal *= this->setZoomDeg(_y, localAxis.axisYVector);
	zoomMatLocal *= this->setZoomDeg(_z, localAxis.axisZVector);
	return zoomMatLocal;
}

void AbsMesh::setModelTransparency(int _value, vector<int> _invalid_list)
{
	if (!_invalid_list.empty())
	{
		for (auto mark : _invalid_list)
		{
			if (mark == this->p_model_->upperOrLowerToothModelMark)
			{
				return;
			}
		}
	}

	for (int i = 0; i < p_mesh_->vert.size(); ++i)
	{
		p_mesh_->vert[i].C().W() = _value;
	}
	if (!children_.empty())
	{
		for (int i = 0; i < children_.size(); ++i)
		{
			children_[i]->setModelTransparency(_value, _invalid_list);
		}
	}
}

Axis AbsMesh::getActualRealtimeAxisNow()
{
	if (this->p_father_trans_matrix_ != nullptr)
	{
		Point3m center, axisX, axisY, axisZ;
		center = this->realTimeAxis.centerPoint;
		axisX = center + this->realTimeAxis.axisXVector;
		axisY = center + this->realTimeAxis.axisYVector;
		axisZ = center + this->realTimeAxis.axisZVector;

		center = (*p_father_trans_matrix_) * center;
		axisX = (*p_father_trans_matrix_) * axisX;
		axisY = (*p_father_trans_matrix_) * axisY;
		axisZ = (*p_father_trans_matrix_) * axisZ;

		axisX = (axisX - center).Normalize();
		axisY = (axisY - center).Normalize();
		axisZ = (axisZ - center).Normalize();
		return Axis(center, axisX, axisY, axisZ);
	}
	else
	{
		return this->realTimeAxis;
	}
}

CMeshO *AbsMesh::getRealtimeMesh()
{
	CMeshO *temp_mesh = new CMeshO;
	vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(*temp_mesh, *p_mesh_);
	for (int i = 0; i < temp_mesh->vert.size(); ++i)
	{
		temp_mesh->vert[i].P() = p_mesh_->Tr * temp_mesh->vert[i].P();
	}
	this->updateModel(temp_mesh);
	return temp_mesh;
}

CMeshO *AbsMesh::getLocationMesh()
{
	CMeshO *temp_mesh = new CMeshO;
	vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(*temp_mesh, *p_mesh_);
	for (int i = 0; i < temp_mesh->vert.size(); ++i)
	{
		temp_mesh->vert[i].P() = transformMatrix_ * temp_mesh->vert[i].P();
	}
	this->updateModel(temp_mesh);
	return temp_mesh;
}

bool AbsMesh::pickedUp(int _mouseX, int _mouseY, vector<CFaceO*>& _vf, int &_iface)
{
	return UtilityTools::getInstance()->getPickedFaceIndex(_mouseX, _mouseY, _vf, p_mesh_, _iface);
}

void AbsMesh::setLocalAxis(Axis _local_axis)
{
	this->localAxis = _local_axis;
	this->updateModelRenderingMatrix();
	this->realTimeAxis = this->updateRealTimeAxisSys(this->transformMatrix_);
	this->afterChangeModelMatrix();
}

void AbsMesh::setRealTimeAxis(Axis _real_time_axis)
{
	this->realTimeAxis = _real_time_axis;
	this->updateCtrlSys();
	Point3m zoom_value = Point3m(trans_vec_.fZX, trans_vec_.fZY, trans_vec_.fZZ);
	Point3m move, rotate;
	HexaVec temp_vec = computeTransformVectors(move, rotate);
	temp_vec.setZoomPart(zoom_value);
	trans_vec_ = temp_vec;
	this->transformMatrix_ = getTransformMatrix(temp_vec, localAxis);
	updateModelRenderingMatrix();
}

void AbsMesh::setTransformMatrix(Matrix44f _matrix)
{
	this->transformMatrix_ = _matrix;
	updateModelRenderingMatrix();
}

HexaVec AbsMesh::computeTransformVectors(Point3m &move, Point3m &rotate)
{
	this->localAxis.computeTransformVectors(realTimeAxis, move, rotate);
	return HexaVec(move.X(), move.Y(), move.Z(), rotate.X(), rotate.Y(), rotate.Z());
}

void AbsMesh::setAdjustDirect(AbsAdjDirect _direct)
{
	this->cur_adj_direct_ = _direct;
}

void AbsMesh::setAdjustMode(AbsAdjMode _mode)
{
	this->cur_adj_mode_ = _mode;
}

bool AbsMesh::pickCtrlSys(int x, int y)
{
	Matrix44f adj_matrix;
	if (this->p_father_trans_matrix_ != nullptr)
	{
		adj_matrix = *p_father_trans_matrix_;
	}
	else
	{
		adj_matrix = Matrix44f::Identity();
	}
	glPushMatrix();
	glMultMatrix(adj_matrix);

	bool picked = false;
	int adjParameter;
	picked = UtilityTools::getInstance()->pickConeHandle(x, y, adjParameter, moveCtrlNodesPair_, CONE_WIDTH);
	if (picked)
	{
		this->setAdjustMode(AbsAdjMode::ABS_ADJ_TRANSLATE);
		if (adjParameter == 0)
		{
			setAdjustDirect(AbsAdjDirect::X_PLUS_PART);
		}
		if (adjParameter == 1)
		{
			setAdjustDirect(AbsAdjDirect::X_MINUS_PART);
		}
		if (adjParameter == 2)
		{
			setAdjustDirect(AbsAdjDirect::Y_PLUS_PART);
		}
		if (adjParameter == 3)
		{
			setAdjustDirect(AbsAdjDirect::Y_MINUS_PART);
		}
		if (adjParameter == 4)
		{
			setAdjustDirect(AbsAdjDirect::Z_PLUS_PART);
		}
		if (adjParameter == 5)
		{
			setAdjustDirect(AbsAdjDirect::Z_MINUS_PART);
		}
		glPopMatrix();
		return true;
	}

	picked = UtilityTools::getInstance()->pickOnCircle(x, y, realTimeAxis.centerPoint, realTimeAxis.axisXVector, fCtrlRadius_, rotateCircleHandle_);
	if (picked)
	{
		this->setAdjustMode(AbsAdjMode::ABS_ADJ_ROTATE);
		setAdjustDirect(AbsAdjDirect::X_PLUS_PART);
		glPopMatrix();
		return true;
	}

	picked = UtilityTools::getInstance()->pickOnCircle(x, y, realTimeAxis.centerPoint, realTimeAxis.axisYVector, fCtrlRadius_, rotateCircleHandle_);
	if (picked)
	{
		this->setAdjustMode(AbsAdjMode::ABS_ADJ_ROTATE);
		setAdjustDirect(AbsAdjDirect::Y_PLUS_PART);
		glPopMatrix();
		return true;
	}

	picked = UtilityTools::getInstance()->pickOnCircle(x, y, realTimeAxis.centerPoint, realTimeAxis.axisZVector, fCtrlRadius_, rotateCircleHandle_);
	if (picked)
	{
		this->setAdjustMode(AbsAdjMode::ABS_ADJ_ROTATE);
		setAdjustDirect(AbsAdjDirect::Z_PLUS_PART);
		glPopMatrix();
		return true;
	}

	this->setAdjustMode(AbsAdjMode::ABS_ADJ_NONE);
	glPopMatrix();
	return false;
}

void AbsMesh::drawCtrlSys()
{
	glPushMatrix();
	if (p_father_trans_matrix_ != nullptr)
	{
		glMultMatrix(*p_father_trans_matrix_);
	}

	float fXLineWidth, fYLineWidth, fZLineWidth;
	Point3m x_color(1, 0, 0), y_color(0, 0, 1), z_color(1, 1, 0);
	// Draw handles
	{
		glPushMatrix();
		glEnable(GL_LIGHTING);
		glColor3f(x_color.X(), x_color.Y(), x_color.Z());
		vcg::Add_Ons::glCone<vcg::Add_Ons::DMSolid>(moveCtrlNodesPair_[X_PLUS_PART].first, moveCtrlNodesPair_[X_PLUS_PART].second, CONE_WIDTH);
		vcg::Add_Ons::glCone<vcg::Add_Ons::DMSolid>(moveCtrlNodesPair_[X_MINUS_PART].first, moveCtrlNodesPair_[X_MINUS_PART].second, CONE_WIDTH);
		glColor3f(y_color.X(), y_color.Y(), y_color.Z());
		vcg::Add_Ons::glCone<vcg::Add_Ons::DMSolid>(moveCtrlNodesPair_[Y_PLUS_PART].first, moveCtrlNodesPair_[Y_PLUS_PART].second, CONE_WIDTH);
		vcg::Add_Ons::glCone<vcg::Add_Ons::DMSolid>(moveCtrlNodesPair_[Y_MINUS_PART].first, moveCtrlNodesPair_[Y_MINUS_PART].second, CONE_WIDTH);
		glColor3f(z_color.X(), z_color.Y(), z_color.Z());
		vcg::Add_Ons::glCone<vcg::Add_Ons::DMSolid>(moveCtrlNodesPair_[Z_PLUS_PART].first, moveCtrlNodesPair_[Z_PLUS_PART].second, CONE_WIDTH);
		vcg::Add_Ons::glCone<vcg::Add_Ons::DMSolid>(moveCtrlNodesPair_[Z_MINUS_PART].first, moveCtrlNodesPair_[Z_MINUS_PART].second, CONE_WIDTH);
		glDisable(GL_LIGHTING);
		glPopMatrix();
	}
	// Draw straight lines
	{
		fXLineWidth = 2.0f;
		fYLineWidth = 2.0f;
		fZLineWidth = 2.0f;
		if (bFineTuneing && cur_adj_mode_ == ABS_ADJ_TRANSLATE)
		{
			switch (cur_adj_direct_)
			{
			case X_PLUS_PART:
			case X_MINUS_PART:
				fXLineWidth = 6.0f;
				break;
			case Y_PLUS_PART:
			case Y_MINUS_PART:
				fYLineWidth = 6.0f;
				break;
			case Z_PLUS_PART:
			case Z_MINUS_PART:
				fZLineWidth = 6.0f;
				break;
			default:
				break;
			}
		}
		paintStraightLine(moveCtrlNodesPair_[X_PLUS_PART].first, moveCtrlNodesPair_[X_MINUS_PART].first, x_color, fXLineWidth);
		paintStraightLine(moveCtrlNodesPair_[Y_PLUS_PART].first, moveCtrlNodesPair_[Y_MINUS_PART].first, y_color, fYLineWidth);
		paintStraightLine(moveCtrlNodesPair_[Z_PLUS_PART].first, moveCtrlNodesPair_[Z_MINUS_PART].first, z_color, fZLineWidth);
	}
	// Draw circles
	{
		fXLineWidth = 2.0f;
		fYLineWidth = 2.0f;
		fZLineWidth = 2.0f;
		if (bFineTuneing && cur_adj_mode_ == ABS_ADJ_ROTATE)
		{
			switch (cur_adj_direct_)
			{
			case X_PLUS_PART:
			case X_MINUS_PART:
				fXLineWidth = 6.0f;
				break;
			case Y_PLUS_PART:
			case Y_MINUS_PART:
				fYLineWidth = 6.0f;
				break;
			case Z_PLUS_PART:
			case Z_MINUS_PART:
				fZLineWidth = 6.0f;
				break;
			default:
				break;
			}
		}
		paintCircleLines(circleYOZ, x_color, fXLineWidth);
		paintCircleLines(circleZOX, y_color, fYLineWidth);
		paintCircleLines(circleXOY, z_color, fZLineWidth);
	}
	glPopMatrix();
}

void AbsMesh::paintStraightLine(const Point3f &_v1, const Point3f &_v2, const Point3m &_color, const float &_width)
{
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);  // Antialias the lines
	glLineWidth(_width);
	glColor3f(_color.X(), _color.Y(), _color.Z());
	glBegin(GL_LINES);
	glVertex3f(_v1.X(), _v1.Y(), _v1.Z());
	glVertex3f(_v2.X(), _v2.Y(), _v2.Z());
	glEnd();
	glLineWidth(CONE_WIDTH);
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

void AbsMesh::paintCircleLines(const vector<Point3m> &_circle, const Point3m &_color, const float &_width)
{
	Point3m vert;
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);  // Antialias the lines
	glColor3f(_color.X(), _color.Y(), _color.Z());
	glLineWidth(_width);
	glBegin(GL_LINE_STRIP);
	for (auto v : _circle)
	{
		glVertex3f(v.X(), v.Y(), v.Z());
	}
	vert = _circle.front();
	glVertex3f(vert.X(), vert.Y(), vert.Z());
	glEnd();
	glLineWidth(CONE_WIDTH);
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

void AbsMesh::updateCtrlSys()
{
	float maxZoomComponent = trans_vec_.fZX;
	maxZoomComponent = maxZoomComponent > trans_vec_.fZY ? maxZoomComponent : trans_vec_.fZY;
	maxZoomComponent = maxZoomComponent > trans_vec_.fZZ ? maxZoomComponent : trans_vec_.fZZ;
	this->fCtrlRadius_ = fBaseCtrlRadius_ * maxZoomComponent;

	updateHandles();
	updateRotateCircle();
}

void AbsMesh::updateHandles()
{
	vector<Point3m>().swap(this->zoomCtrlNodes_);
	zoomCtrlNodes_.push_back(realTimeAxis.centerPoint + realTimeAxis.axisXVector * (baseLengthVec_.X() * trans_vec_.fZX + ZOOM_HANDLE_EXTEND_LEN));
	zoomCtrlNodes_.push_back(realTimeAxis.centerPoint - realTimeAxis.axisXVector * (baseLengthVec_.X() * trans_vec_.fZX + ZOOM_HANDLE_EXTEND_LEN));
	zoomCtrlNodes_.push_back(realTimeAxis.centerPoint + realTimeAxis.axisYVector * (baseLengthVec_.Y() * trans_vec_.fZY + ZOOM_HANDLE_EXTEND_LEN));
	zoomCtrlNodes_.push_back(realTimeAxis.centerPoint - realTimeAxis.axisYVector * (baseLengthVec_.Y() * trans_vec_.fZY + ZOOM_HANDLE_EXTEND_LEN));
	zoomCtrlNodes_.push_back(realTimeAxis.centerPoint + realTimeAxis.axisZVector * (baseLengthVec_.Z() * trans_vec_.fZZ + ZOOM_HANDLE_EXTEND_LEN));
	zoomCtrlNodes_.push_back(realTimeAxis.centerPoint - realTimeAxis.axisZVector * (baseLengthVec_.Z() * trans_vec_.fZZ + ZOOM_HANDLE_EXTEND_LEN));

	vector<pair<Point3m, Point3m>>().swap(moveCtrlNodesPair_);
	float beginPos = 1.4f, endPos = 1.6f;
	moveCtrlNodesPair_.push_back(make_pair(
		realTimeAxis.centerPoint + realTimeAxis.axisXVector * fCtrlRadius_ * beginPos,
		realTimeAxis.centerPoint + realTimeAxis.axisXVector * fCtrlRadius_ * endPos));
	moveCtrlNodesPair_.push_back(make_pair(
		realTimeAxis.centerPoint - realTimeAxis.axisXVector * fCtrlRadius_ * beginPos,
		realTimeAxis.centerPoint - realTimeAxis.axisXVector * fCtrlRadius_ * endPos));
	moveCtrlNodesPair_.push_back(make_pair(
		realTimeAxis.centerPoint + realTimeAxis.axisYVector * fCtrlRadius_ * beginPos,
		realTimeAxis.centerPoint + realTimeAxis.axisYVector * fCtrlRadius_ * endPos));
	moveCtrlNodesPair_.push_back(make_pair(
		realTimeAxis.centerPoint - realTimeAxis.axisYVector * fCtrlRadius_ * beginPos,
		realTimeAxis.centerPoint - realTimeAxis.axisYVector * fCtrlRadius_ * endPos));
	moveCtrlNodesPair_.push_back(make_pair(
		realTimeAxis.centerPoint + realTimeAxis.axisZVector * fCtrlRadius_ * beginPos,
		realTimeAxis.centerPoint + realTimeAxis.axisZVector * fCtrlRadius_ * endPos));
	moveCtrlNodesPair_.push_back(make_pair(
		realTimeAxis.centerPoint - realTimeAxis.axisZVector * fCtrlRadius_ * beginPos,
		realTimeAxis.centerPoint - realTimeAxis.axisZVector * fCtrlRadius_ * endPos));
}

void AbsMesh::updateRotateCircle()
{
	Point3m p;
	float costhita = 0.0f;
	float sinthita = 0.0f;
	std::vector<Point3m>().swap(this->circleXOY);
	std::vector<Point3m>().swap(this->circleYOZ);
	std::vector<Point3m>().swap(this->circleZOX);
	for (int i = 0; i < CIRCLE_DIVIDE_NUMBER; ++i)
	{
		costhita = cos(STEP_RADS * i);
		sinthita = sin(STEP_RADS * i);
		p = realTimeAxis.centerPoint + (realTimeAxis.axisYVector * costhita + realTimeAxis.axisZVector * sinthita) * fCtrlRadius_;
		this->circleYOZ.push_back(p);
		p = realTimeAxis.centerPoint + (realTimeAxis.axisZVector * costhita + realTimeAxis.axisXVector * sinthita) * fCtrlRadius_;
		this->circleZOX.push_back(p);
		p = realTimeAxis.centerPoint + (realTimeAxis.axisXVector * costhita + realTimeAxis.axisYVector * sinthita) * fCtrlRadius_;
		this->circleXOY.push_back(p);
	}
}

void AbsMesh::setVisible(bool _visible)
{
	if (p_model_ == nullptr)
	{
		return;
	}
	this->p_model_->visible = _visible;
	if (!children_.empty())
	{
		for (auto &child : children_)
		{
			child->setVisible(_visible);
		}
	}
}

