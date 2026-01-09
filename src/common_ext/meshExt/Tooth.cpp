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

#include "Tooth.h"
#include <QCoreApplication>
#include "common/config.h"

Tooth::Tooth()
{
}

Tooth::Tooth(const Tooth &stooth)
{
	if (stooth.p_mesh_ == NULL)
	{
		return;
	}
	this->inital(stooth.p_mesh_);
	this->iLabel = stooth.iLabel;
	this->toothIndex = stooth.toothIndex;
	this->toothFaceIndex = stooth.toothFaceIndex;
	this->boundaryIndex = stooth.boundaryIndex;
	vcg::tri::UpdateBounding<CMeshO>::Box(*stooth.p_mesh_);
	this->transformMatrix_.SetIdentity();
	this->localAxis = stooth.localAxis;
	this->strFDI = stooth.strFDI;
	this->moveVector = Point3m(0, 0, 0);
	this->rotateVector = Point3m(0, 0, 0);
	this->fCtrlRadius_ = TOOTH_BASE_CTRL_RADIUS;
	this->fToothWidth = stooth.fToothWidth;
	this->fSliceCutValue = stooth.fSliceCutValue;
	this->bHavePickedCtrlPTs = stooth.bHavePickedCtrlPTs;
	this->InitalToothBoundbox();
	this->realTimeMesh = NULL;
	this->AimHexaVec = stooth.AimHexaVec;
	this->desireVertA = stooth.desireVertA;
	this->desireVertB = stooth.desireVertB;
	this->desiredXAxis = stooth.desiredXAxis;
	this->realTimeAxis = stooth.realTimeAxis;
	this->localBoundbox = stooth.localBoundbox;
	this->locationVertA = stooth.locationVertA;
	this->locationVertB = stooth.locationVertB;
	this->frozenBoundaryVerts = stooth.frozenBoundaryVerts;
	this->displayedX = stooth.displayedX;
	this->displayedY = stooth.displayedY;
	this->displayedZ = stooth.displayedZ;
	this->displayedA = stooth.displayedA;
	this->displayedB = stooth.displayedB;
	this->displayedG = stooth.displayedG;
	this->convexFaces = stooth.convexFaces;
	this->convexVerts = stooth.convexVerts;
	this->lateralOrigin_Left = stooth.lateralOrigin_Left;
	this->lateralOrigin_Right = stooth.lateralOrigin_Right;
	this->source_LateralOrigin_Left = stooth.source_LateralOrigin_Left;
	this->source_LateralOrigin_Right = stooth.source_LateralOrigin_Right;
	this->crownOrigin = stooth.crownOrigin;
	this->rootOrigin = stooth.rootOrigin;
	this->source_CrownOrigin = stooth.source_CrownOrigin;
	this->source_RootOrigin = stooth.source_RootOrigin;
	this->defaultMarkerPointIndexes = stooth.defaultMarkerPointIndexes;
	this->labialSideOrigin = stooth.labialSideOrigin;
	this->root_vertindex_interval_ = stooth.root_vertindex_interval_;
	this->bFixed_ = stooth.bFixed_;
	this->width_definition_point_a_ = stooth.width_definition_point_a_;
	this->width_definition_point_b_ = stooth.width_definition_point_b_;
	iMixedNumberLeft = stooth.iMixedNumberLeft;
	iMixedNumberRight = stooth.iMixedNumberRight;
	fToothWidth = stooth.fToothWidth;
	fSliceCutValue = stooth.fSliceCutValue;
	fCtrlRadius_ = stooth.fCtrlRadius_;

	fOverlapDegreeXMinusDirection = stooth.fOverlapDegreeXMinusDirection;
	fOverlapDegreeXPlusDirection = stooth.fOverlapDegreeXPlusDirection;

	front_CtrlPTs = stooth.front_CtrlPTs;
	back_CtrlPTs = stooth.back_CtrlPTs;
	intersections_ = stooth.intersections_;

	front_CutMark_ = stooth.front_CutMark_;
	back_CutMark_ = stooth.back_CutMark_;

	bHasLeftNeighboor = stooth.bHasLeftNeighboor;
	bHasRightNeighboor = stooth.bHasRightNeighboor;
	//segmentPicedInfo = stooth.segmentPicedInfo;
	LipsSurfaceMinAngle = stooth.LipsSurfaceMinAngle;
	LipsSurfaceMaxAngle = stooth.LipsSurfaceMaxAngle;
	bPullOut = stooth.bPullOut;

	TransHexaVec = stooth.TransHexaVec;
	CollideTab = stooth.CollideTab;

	this->circleYOZ = stooth.circleYOZ;
	this->circleZOX = stooth.circleZOX;
	this->circleXOY = stooth.circleXOY;
	this->feature_marks_ = stooth.feature_marks_;
}

Tooth::~Tooth()
{
	if (compare_obj_)
	{
		delete compare_obj_;
		compare_obj_ = nullptr;
	}
    SAFE_DELETE(rigid);
}

void Tooth::InitalTooth(CMeshO *_mesh, int _lable, bool belongToUpperDental, Axis toothSys, QString fdiString, bool _b_filterToothMesh /*= true*/)
{
	this->inital(_mesh);
	this->iLabel = _lable;
	if (_b_filterToothMesh)
	{
		generateToothSourceMesh();
	}
	else
	{
		for (int i = 0; i < p_mesh_->vert.size(); i++)
		{
			this->toothIndex.push_back(i);
		}
		for (int i = 0; i < p_mesh_->face.size(); i++)
		{
			this->toothFaceIndex.push_back(i);
		}
	}
	this->boundaryIndex.clear();
	this->convexFaces.clear();
	this->convexVerts.clear();
	vcg::tri::UpdateBounding<CMeshO>::Box(*this->p_mesh_);
	this->transformMatrix_.SetIdentity();
	this->localAxis = toothSys;
	this->strFDI = fdiString;
	this->realTimeAxis = this->localAxis;
	this->moveVector = Point3m(0, 0, 0);
	this->rotateVector = Point3m(0, 0, 0);
	this->fCtrlRadius_ = TOOTH_BASE_CTRL_RADIUS;
	this->fToothWidth = 0;
	this->fSliceCutValue = 0;
	this->bHavePickedCtrlPTs = false;
	this->InitalToothBoundbox();
	this->initialWidthDefinitionPoint();
	this->realTimeMesh = nullptr;
	this->fOverlapDegreeXMinusDirection = 0;
	this->fOverlapDegreeXPlusDirection = 0;
	this->root_vertindex_interval_ = Point2i(-1, -1);
	updateToothCtrlSys();
}

void Tooth::InitalFeatureMarks(vector<Point3m> _marks)
{
	this->feature_marks_ = _marks;
}

void Tooth::freshToothWithRepairedMesh(CMeshO *pToothMesh, const std::vector<int> &indexs, const std::vector<int> &indexsFace, CMeshO *pDentalMesh)
{
	this->inital(pToothMesh);
	this->toothIndex = indexs;
	this->toothFaceIndex = indexsFace;
	vcg::tri::UpdateBounding<CMeshO>::Box(*this->p_mesh_);
	this->InitalToothBoundbox();
	if (this->realTimeMesh != NULL)
	{
		delete this->realTimeMesh;
		this->realTimeMesh = NULL;
	}

	this->freshToothBoundaryOnRepairedMesh(pDentalMesh);
}

void Tooth::InitalToothBoundbox()
{
	vector<Point3m> vertlist;
	for (int index = 0; index < p_mesh_->vert.size();  ++index)
	{
		vertlist.push_back(p_mesh_->vert[index].P());
	}
	this->localBoundbox.fLength -= this->fSliceCutValue;
	this->localBoundbox.InitBoundBox(this->localAxis, vertlist);
	vector<Point3m>().swap(vertlist);
}

void Tooth::InitalToothBoundbox(CMeshO *curmesh)
{
	vector<Point3m> vertlist;
	for (int i = 0; i < this->p_mesh_->vert.size(); ++i)
	{
		vertlist.push_back(p_mesh_->vert[i].P());
	}
	this->localBoundbox.fLength -= this->fSliceCutValue;
	this->localBoundbox.InitBoundBox(this->localAxis, vertlist);
	vertlist.clear();
}

void Tooth::getItsCollideTable(Tooth &neighbor, int stageNum)
{
	CMeshO sourceA, sourceB;
	vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(sourceA, *this->p_mesh_);
	vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(sourceB, *neighbor.p_mesh_);

	for (int i = 0; i <= stageNum; i++)
	{
		doHexagonTransform(this->TransHexaVec[i]);
		getRootOfBBTree();

		for (int j = 0; j <= stageNum; j++)
		{
			bool bCollideCurrent = false;

			// Transform this tooth and its neighbor tooth
			neighbor.doHexagonTransform(neighbor.TransHexaVec[j]);

			// Perform collision detection
			neighbor.getRootOfBBTree();

			int resultLR[3];
			iMixedNumberRight = 0;
			this->BBTreeRoot->getAllMixedLeaf(*neighbor.BBTreeRoot, iMixedNumberRight, resultLR);
			if (iMixedNumberRight > 0)
				bCollideCurrent = true;
			else
				bCollideCurrent = false;

			if (bCollideCurrent)
				this->CollideTab.push_back(1);
			else
				this->CollideTab.push_back(0);

			vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(*neighbor.p_mesh_, sourceB);
		}
		vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(*this->p_mesh_, sourceA);
	}
	this->clearAllLeavesBuffer();
	neighbor.clearAllLeavesBuffer();
	sourceA.Clear();
	sourceB.Clear();
}

void Tooth::doHexagonTransform(HexaVec V)
{
	vcg::Matrix44f tempTransMatrix = computeTransformMatrix(V);
	for (int i = 0; i < this->toothIndex.size(); i++)
	{
		int tempIndex = toothIndex[i];
		p_mesh_->vert[tempIndex].P() = tempTransMatrix * p_mesh_->vert[tempIndex].P();
	}
}

vcg::Matrix44f Tooth::computeTransformMatrix(HexaVec transHexaVec)
{
	vcg::Matrix44f transMat;
	vcg::Matrix44f transform, transformInverse;
	transform.SetIdentity();

	transform.SetTranslate(this->localAxis.centerPoint);
	transformInverse.SetTranslate(-this->localAxis.centerPoint);

	vcg::Matrix44f transformValue, rotMatGlobal, rotMatLocal;
	transMat.SetIdentity();
	rotMatGlobal.SetIdentity();
	rotMatLocal.SetIdentity();
	Point3m trans;

	trans = this->localAxis.axisXVector * transHexaVec.fVX;
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	trans = this->localAxis.axisYVector * transHexaVec.fVY;
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	trans = this->localAxis.axisZVector * transHexaVec.fVZ;
	transformValue.SetTranslate(trans);
	transMat *= transformValue;

	transformValue.SetRotateDeg(transHexaVec.fA, this->localAxis.axisXVector);
	rotMatLocal *= transformValue;
	transformValue.SetRotateDeg(transHexaVec.fB, this->localAxis.axisYVector);
	rotMatLocal *= transformValue;
	transformValue.SetRotateDeg(transHexaVec.fG, this->localAxis.axisZVector);
	rotMatLocal *= transformValue;

	transform *= transMat * rotMatLocal;
	transform *= transformInverse;

	return transform;
}

void Tooth::getArrangeMarkedPoint()
{
	int sign = this->strFDI.toInt();
	int dentalSign, toothSign;
	dentalSign = sign / 10;
	if (dentalSign != 1 && dentalSign != 2)//Not belong to UpperDental, return;
	{
		return;
	}
	toothSign = sign % 10;
	switch (toothSign)
	{
	case 1:
	case 2:
	case 3:
		getArrangeMarkedPoint_AnteriorRegion();
		break;
	default:
		getArrangeMarkedPoint_PosteriorRegion();
		break;
	}
	return;
}

void Tooth::getArrangeMarkedPoint_AnteriorRegion()
{
	Point3m vertCusp, vertFoot;
	this->getLocationPoints();
	vertCusp = (this->p_mesh_->vert[this->locationVertA].P() + this->p_mesh_->vert[this->locationVertB].P()) / 2.0f;

	float tempValue, minValue = FLT_MAX;
	Point3m tempV;
	for (auto v : frozenBoundaryVerts)
	{
		tempV = (v - this->localAxis.centerPoint).Normalize();
		if (tempV * this->localAxis.axisZVector < 0)
		{
			tempValue = abs(tempV * this->localAxis.axisXVector);
			if (tempValue < minValue)
			{
				minValue = tempValue;
				vertFoot = v;
			}
		}
	}

	int result = -1;
	Point3m aimVert = vertCusp + (vertFoot - vertCusp) * 0.4f;
	float tempDistance, minDistance = FLT_MAX;
	if (0 <= root_vertindex_interval_.X() && root_vertindex_interval_.X() < root_vertindex_interval_.Y())
	{
		for (int i = 0; i < p_mesh_->vert.size(); ++i)
		{
			if (root_vertindex_interval_.X() <= i && i <= root_vertindex_interval_.Y())
			{
				continue;
			}

			int index = i;
			tempV = p_mesh_->vert[index].P() - aimVert;
			tempDistance = tempV * tempV;
			if (tempDistance < minDistance)
			{
				minDistance = tempDistance;
				result = index;
			}
		}
	}
	else
	{
		int limited = this->toothIndex.size() - this->boundaryIndex.size() * 40;
		for (int j = 0; j < limited - 1; ++j)
		{
			int index = j;
			tempV = p_mesh_->vert[index].P() - aimVert;
			tempDistance = tempV * tempV;
			if (tempDistance < minDistance)
			{
				minDistance = tempDistance;
				result = index;
			}
		}
	}

	this->localArrageMarkedVert = result;
	this->arrageMarkedVert = this->toothIndex[result];
}

void Tooth::getArrangeMarkedPoint_PosteriorRegion()
{
	Point3m axisY = this->localAxis.axisYVector;
	Point3m tempV;

	int result = -1;
	float tempValue, minValue = FLT_MAX;
	if (0 <= root_vertindex_interval_.X() && root_vertindex_interval_.X() < root_vertindex_interval_.Y())
	{
		for (int i = 0; i < p_mesh_->vert.size(); ++i)
		{
			if (root_vertindex_interval_.X() <= i && i <= root_vertindex_interval_.Y())
			{
				continue;
			}

			int index = i;
			tempV = (p_mesh_->vert[index].P() - this->localAxis.centerPoint).Normalize();
			tempValue = 1.0f - tempV * axisY;
			if (tempValue < minValue)
			{
				minValue = tempValue;
				result = index;
			}
		}
	}
	else
	{
		int limited = this->toothIndex.size() - this->boundaryIndex.size() * 40;
		for (int j = 0; j < limited - 1; ++j)
		{
			int index = j;
			tempV = (p_mesh_->vert[index].P() - this->localAxis.centerPoint).Normalize();
			tempValue = 1.0f - tempV * axisY;
			if (tempValue < minValue)
			{
				minValue = tempValue;
				result = index;
			}
		}
	}
	this->localArrageMarkedVert = result;
	this->arrageMarkedVert = this->toothIndex[result];
}

void Tooth::getArrangeMarkOnDesireXAxis()
{
	if (this->localArrageMarkedVert < 0)
	{
		return;
	}

	Point3m arrageMarkPos = this->p_mesh_->vert[this->localArrageMarkedVert].P();
	Point3m tempV = arrageMarkPos - this->localAxis.centerPoint;
	float projValue = tempV * this->localAxis.axisXVector;

	Point3m desiredXV = (this->desireVertA - this->desireVertB).Normalize();
	Point3m desiredCenter = (this->desireVertA + this->desireVertB) / 2.0f;
	this->arrageMarkOnDesireXAxis = desiredCenter + desiredXV * projValue;
}

void Tooth::getLocationPoints()
{
	this->locationVertA = getDirectionalPeakIndex(localAxis.axisXVector, localAxis.axisYVector, localAxis.axisZVector);
	this->locationVertB = getDirectionalPeakIndex(-localAxis.axisXVector, localAxis.axisYVector, localAxis.axisZVector);
}

int Tooth::getDirectionalPeakIndex(Point3m _x, Point3m _y, Point3m _z, bool proj_length)
{
	Point3m origin = this->localAxis.centerPoint;

	Point3m vec;
	vector<Point4m> record;
	float max_x = 0, max_y = 0, max_z = 0;
	for (int i = 0; i < this->p_mesh_->vert.size(); ++i)
	{
		if (proj_length)
		{
			vec = p_mesh_->vert[i].P() - origin;
		}
		else
		{
			vec = (p_mesh_->vert[i].P() - origin).Normalize();
		}

		float x_value = vec * _x;
		if (x_value < 0)
		{
			x_value = -1000;
		}
		if (x_value > max_x)
		{
			max_x = x_value;
		}

		float y_value = vec * _y;
		if (y_value < 0)
		{
			y_value = -1000;
		}
		if (y_value > max_y)
		{
			max_y = y_value;
		}

		float z_value = vec * _z;
		if (z_value < 0)
		{
			z_value = -1000;
		}
		if (z_value > max_z)
		{
			max_z = z_value;
		}

		record.push_back(Point4m(x_value, y_value, z_value, i));
	}

	float t, max_t = 0;
	int peak_index = 0;
	for (auto p : record)
	{
		t = 0;
		if (max_x != 0)
		{
			t += p.X() / max_x;
		}
		if (max_y != 0)
		{
			t += p.Y() / max_y;
		}
		if (max_z != 0)
		{
			t += p.Z() / max_z;
		}

		if (t > max_t)
		{
			max_t = t;
			peak_index = p.W();
		}
	}
	vector<Point4m>().swap(record);
	return peak_index;
}

Point3m Tooth::getDirectionalPeak(Point3m _x, Point3m _y, Point3m _z, bool proj_length /*= true*/)
{
	Point3m peak(0, 0, 0);
	int index = getDirectionalPeakIndex(_x, _y, _z, proj_length);
	peak = p_mesh_->vert[index].P();
	return peak;
}

void Tooth::getCtrlPTs()
{
	if (this->boundaryIndex.empty())
	{
		return;
	}

	vector<Point3m>().swap(this->front_CtrlPTs);
	vector<Point3m>().swap(this->back_CtrlPTs);
	this->front_CtrlPTs = this->realtimeBoundary;
	return;
}

void Tooth::adjLinkFrontArea(Tooth &neighborTooth)
{
	Point3m tempVector;
	Point3m judgeVector = ((neighborTooth.localAxis.axisYVector + this->localAxis.axisYVector) / 2.0f).Normalize();
	Point3m tempA, tempB;
	tempA = *neighborTooth.front_CtrlPTs.begin();
	tempB = *--this->front_CtrlPTs.end();
	tempVector = (tempA - tempB).Normalize();
	float value = tempVector * judgeVector;
	if (value > 0.3f)
	{
		Tooth &changeTooth = neighborTooth;
		Point3m planePos, planeNormal;
		planePos = tempB;
		planeNormal = judgeVector;
		vector<int> checkList;
		for (int i = 0; i < neighborTooth.front_CtrlPTs.size() / 2; ++i)
		{
			checkList.push_back(i);
		}
		adjToothsLinkedFrontArea(changeTooth, planePos, planeNormal, checkList);
	}
	else if (value < -0.3f)
	{
		Tooth &changeTooth = *this;
		Point3m planePos, planeNormal;
		planePos = tempA;
		planeNormal = judgeVector;
		vector<int> checkList;
		for (int i = this->front_CtrlPTs.size() - 1; i > this->front_CtrlPTs.size() / 2; --i)
		{
			checkList.push_back(i);
		}
		adjToothsLinkedFrontArea(changeTooth, planePos, planeNormal, checkList);
	}
}

void Tooth::adjLinkBackArea(Tooth &neighborTooth)
{
	Point3m tempVector;
	Point3m judgeVector = ((neighborTooth.localAxis.axisYVector + this->localAxis.axisYVector) / 2.0f).Normalize();
	Point3m tempA, tempB;
	tempA = *--neighborTooth.back_CtrlPTs.end();
	tempB = *this->back_CtrlPTs.begin();
	tempVector = (tempA - tempB).Normalize();
	float value = tempVector * judgeVector;
	if (value > 0.3f)
	{
		Tooth &changeTooth = neighborTooth;
		Point3m planePos, planeNormal;
		planePos = tempB;
		planeNormal = judgeVector;
		vector<int> checkList;
		for (int i = neighborTooth.back_CtrlPTs.size() - 1; i > neighborTooth.back_CtrlPTs.size() / 2; --i)
		{
			checkList.push_back(i);
		}
		adjToothsLinkedBackArea(changeTooth, planePos, planeNormal, checkList);
	}
	else if (value < -0.3f)
	{
		Tooth &changeTooth = *this;
		Point3m planePos, planeNormal;
		planePos = tempA;
		planeNormal = judgeVector;
		vector<int> checkList;
		for (int i = 0; i < this->back_CtrlPTs.size() / 2; ++i)
		{
			checkList.push_back(i);
		}
		adjToothsLinkedBackArea(changeTooth, planePos, planeNormal, checkList);
	}
}

void Tooth::adjToothsLinkedFrontArea(Tooth &changeTooth, Point3m planePos, Point3m planeNormal, vector<int> checkList)
{
	if (checkList.empty())
	{
		return;
	}
	bool saveFrontVert;
	float a = checkList[checkList.size() - 1];
	float b = checkList[0];
	if (a > b)
	{
		saveFrontVert = true;
	}
	else
	{
		saveFrontVert = false;
	}

	int inext;
	float tempValue1, tempValue2;
	vector<int> delVerts;
	bool needDelOperation = false;
	for (auto i : checkList)
	{
		if (saveFrontVert)
		{
			inext = i + 1;
		}
		else
		{
			inext = i - 1;
		}
		tempValue1 = (changeTooth.front_CtrlPTs[i] - planePos) * planeNormal;
		tempValue2 = (changeTooth.front_CtrlPTs[inext] - planePos) * planeNormal;

		delVerts.push_back(i);

		if (tempValue1 * tempValue2 < 0)
		{
			needDelOperation = true;
			break;
		}
	}

	if (needDelOperation)
	{
		vector<bool> saveItList;
		for (auto vert : changeTooth.front_CtrlPTs)
		{
			saveItList.push_back(true);
		}
		for (auto index : delVerts)
		{
			saveItList[index] = false;
		}

		vector<Point3m> tempList;
		for (int i = 0; i < changeTooth.front_CtrlPTs.size(); ++i)
		{
			if (saveItList[i])
			{
				tempList.push_back(changeTooth.front_CtrlPTs[i]);
			}
		}
		changeTooth.front_CtrlPTs.clear();
		changeTooth.front_CtrlPTs = tempList;

		tempList.clear();
		saveItList.clear();
	}
	delVerts.clear();
}

void Tooth::adjToothsLinkedBackArea(Tooth &changeTooth, Point3m planePos, Point3m planeNormal, vector<int> checkList)
{
	if (checkList.empty())
	{
		return;
	}
	bool saveFrontVert;
	float a = checkList[checkList.size() - 1];
	float b = checkList[0];
	if (a > b)
	{
		saveFrontVert = true;
	}
	else
	{
		saveFrontVert = false;
	}

	int inext;
	float tempValue1, tempValue2;
	vector<int> delVerts;
	bool needDelOperation = false;
	for (auto i : checkList)
	{
		if (saveFrontVert)
		{
			inext = i + 1;
		}
		else
		{
			inext = i - 1;
		}
		tempValue1 = (changeTooth.back_CtrlPTs[i] - planePos) * planeNormal;
		tempValue2 = (changeTooth.back_CtrlPTs[inext] - planePos) * planeNormal;

		delVerts.push_back(i);

		if (tempValue1 * tempValue2 < 0)
		{
			needDelOperation = true;
			break;
		}
	}

	if (needDelOperation)
	{
		vector<bool> saveItList;
		for (auto vert : changeTooth.back_CtrlPTs)
		{
			saveItList.push_back(true);
		}
		for (auto index : delVerts)
		{
			saveItList[index] = false;
		}

		vector<Point3m> tempList;
		for (int i = 0; i < changeTooth.back_CtrlPTs.size(); ++i)
		{
			if (saveItList[i])
			{
				tempList.push_back(changeTooth.back_CtrlPTs[i]);
			}
		}
		changeTooth.back_CtrlPTs.clear();
		changeTooth.back_CtrlPTs = tempList;

		tempList.clear();
		saveItList.clear();
	}
	delVerts.clear();
}

void Tooth::pickCtrlPTsUniformly(int num)
{
	vcg::tri::Allocator<CMeshO>::CompactEveryVector(*this->p_mesh_);
	// Update topology
    if(!tri::HasFFAdjacency(*this->p_mesh_))
        this->p_mesh_->face.EnableFFAdjacency();
    assert(tri::HasFFAdjacency(*this->p_mesh_) == true);

    if(!tri::HasVFAdjacency(*this->p_mesh_))
    {
        this->p_mesh_->vert.EnableVFAdjacency();
        this->p_mesh_->face.EnableVFAdjacency();
    }
    assert(tri::HasVFAdjacency(*this->p_mesh_) == true);
	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*this->p_mesh_);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(*this->p_mesh_);
	if (num % 2 == 0)
		return;

	int segmentNum;
	float fStepLength;
	float lengthSummary;
	vector<float> segmentLengthList;
	vector<Point3m> newlist;

	segmentNum = this->front_CtrlPTs.size() - 1;
	for (int i = 0; i < segmentNum; i++)
	{
		Point3m tempEdge = front_CtrlPTs[i + 1] - front_CtrlPTs[i];
		segmentLengthList.push_back(sqrtf(tempEdge * tempEdge));
	}
	lengthSummary = 0;
	for (auto length : segmentLengthList)
	{
		lengthSummary += length;
	}
	fStepLength = lengthSummary / (float)(num - 1.0f);

	newlist.push_back(*this->front_CtrlPTs.begin());
	{
		DefiniteIntersection intersection(*this->front_CtrlPTs.begin());
		this->intersections_.push_back(intersection);
	}
	for (int i = 1; i < (num - 1); ++i)
	{
		int j = 0;
		float tempJourney = fStepLength * i;
		float journeyCounter = 0, returnJourney;

		for (; j < segmentNum; ++j)
		{
			journeyCounter += segmentLengthList[j];
			if (journeyCounter >= tempJourney)
			{
				returnJourney = journeyCounter - tempJourney;
				break;
			}
		}

		Point3m newVert = this->front_CtrlPTs[j + 1] + (this->front_CtrlPTs[j] - this->front_CtrlPTs[j + 1]).Normalize() * returnJourney;
		{
			DefiniteIntersection intersection(newVert, FEdge(this->front_CtrlPTs[j], this->front_CtrlPTs[j + 1]));
			this->intersections_.push_back(intersection);
		}
		newlist.push_back(newVert);
	}
	newlist.push_back(*(--this->front_CtrlPTs.end()));
	{
		DefiniteIntersection intersection(*(--this->front_CtrlPTs.end()));
		this->intersections_.push_back(intersection);
	}
	this->front_CtrlPTs.clear();
	this->front_CtrlPTs = newlist;
	newlist.clear();
	segmentLengthList.clear();

	segmentNum = this->back_CtrlPTs.size() - 1;
	for (int i = 0; i < segmentNum; i++)
	{
		Point3m tempEdge = back_CtrlPTs[i + 1] - back_CtrlPTs[i];
		segmentLengthList.push_back(sqrtf(tempEdge.X() * tempEdge.X() + tempEdge.Y() * tempEdge.Y() + tempEdge.Z() * tempEdge.Z()));
	}
	lengthSummary = 0;
	for (auto length : segmentLengthList)
	{
		lengthSummary += length;
	}
	fStepLength = lengthSummary / (float)(num - 1.0f);

	//填装
	newlist.push_back(*this->back_CtrlPTs.begin());
	{
		DefiniteIntersection intersection(*this->back_CtrlPTs.begin());
		this->intersections_.push_back(intersection);
	}
	for (int i = 1; i < (num - 1); ++i)
	{
		int j = 0;
		float tempJourney = fStepLength * i;
		float journeyCounter = 0, returnJourney;

		for (; j < segmentNum; ++j)
		{
			journeyCounter += segmentLengthList[j];
			if (journeyCounter >= tempJourney)
			{
				returnJourney = journeyCounter - tempJourney;
				break;
			}
		}

		Point3m newVert = this->back_CtrlPTs[j + 1] + (this->back_CtrlPTs[j] - this->back_CtrlPTs[j + 1]).Normalize() * returnJourney;
		{
			DefiniteIntersection intersection(newVert, FEdge(this->back_CtrlPTs[j], this->back_CtrlPTs[j + 1]));
			this->intersections_.push_back(intersection);
		}
		newlist.push_back(newVert);
	}
	newlist.push_back(*(--this->back_CtrlPTs.end()));
	{
		DefiniteIntersection intersection(*(--this->back_CtrlPTs.end()));
		this->intersections_.push_back(intersection);
	}
	this->back_CtrlPTs.clear();
	this->back_CtrlPTs = newlist;

	newlist.clear();
	segmentLengthList.clear();
	this->bHavePickedCtrlPTs = true;
}

void Tooth::pickCtrlPTsLowestCtrlPTWay(int pickNumLimit, vector<Point3m> &gumFrontCtrlPTs, vector<Point3m> &gumBackCtrlPTs, Point3m occlusalPlaneNormalV)
{
	if (gumFrontCtrlPTs.empty() || gumBackCtrlPTs.empty())
	{
		return;
	}

	Point3m originPoint = this->realTimeAxis.centerPoint;
	float fMinProjValue_Front = (gumFrontCtrlPTs[0] - originPoint) * occlusalPlaneNormalV;
	int iPickFrontIndex = 0;
	for (int i = 1; i < gumFrontCtrlPTs.size(); ++i)
	{
		float tempProjValue = (gumFrontCtrlPTs[i] - originPoint) * occlusalPlaneNormalV;
		if (tempProjValue < fMinProjValue_Front)
		{
			fMinProjValue_Front = tempProjValue;
			iPickFrontIndex = i;
		}
	}
	if (this->front_CutLineCtrlPTs.size() == pickNumLimit)
	{
		this->front_CutMark_ = gumFrontCtrlPTs[iPickFrontIndex];
	}

	float fMinProjValue_Back = (gumBackCtrlPTs[0] - originPoint) * occlusalPlaneNormalV;
	int iPickBackIndex = 0;
	for (int i = 1; i < gumBackCtrlPTs.size(); ++i)
	{
		float tempProjValue = (gumBackCtrlPTs[i] - originPoint) * occlusalPlaneNormalV;
		if (tempProjValue < fMinProjValue_Back)
		{
			fMinProjValue_Back = tempProjValue;
			iPickBackIndex = i;
		}
	}
	if (this->back_CutLineCtrlPTs.size() == pickNumLimit)
	{
		this->back_CutMark_ = gumBackCtrlPTs[iPickBackIndex];
	}
}

void Tooth::pickCtrlPTsTangentWay(Tooth &oppoTooth, int pickNumLimit, Point3m occlusalPlaneNormalV)
{
	Axis projAxis;
	Point3m ctrlVert_Self, ctrlVert_Oppo;
	projAxis.centerPoint = (this->realTimeAxis.centerPoint + oppoTooth.realTimeAxis.centerPoint) / 2.0f;
	projAxis.axisYVector = occlusalPlaneNormalV;
	projAxis.axisXVector = ((this->realTimeAxis.axisXVector + oppoTooth.realTimeAxis.axisXVector) / 2.0f).Normalize();
	projAxis.axisZVector = (projAxis.axisXVector ^ projAxis.axisYVector).Normalize();
	FEdge tangentLineFront = pickOneSideCtrlPTsTangentWay(this->front_CtrlPTs, oppoTooth.front_CtrlPTs, projAxis);

	if ((tangentLineFront.vertA - this->front_CutLineCtrlPTs[0]) * (this->front_CutLineCtrlPTs[pickNumLimit - 1] - this->front_CutLineCtrlPTs[0]) > 0)
	{
		if (this->front_CutLineCtrlPTs.size() == pickNumLimit)
		{
			this->front_CutLineCtrlPTs[pickNumLimit / 2] = tangentLineFront.vertA;
		}
	}

	projAxis.axisXVector *= -1;
	projAxis.axisZVector *= -1;
	FEdge tangentLineBack = pickOneSideCtrlPTsTangentWay(this->back_CtrlPTs, oppoTooth.back_CtrlPTs, projAxis);
	if ((tangentLineBack.vertA - this->back_CutLineCtrlPTs[0]) * (this->back_CutLineCtrlPTs[pickNumLimit - 1] - this->back_CutLineCtrlPTs[0]) > 0)
	{
		if (this->back_CutLineCtrlPTs.size() == pickNumLimit)
		{
			this->back_CutLineCtrlPTs[pickNumLimit / 2] = tangentLineBack.vertA;
		}
	}
}

FEdge Tooth::pickOneSideCtrlPTsTangentWay(vector<Point3m> &vertSetA, vector<Point3m> &vertSetB, Axis projAxis)
{
	FEdge result;
	vector<FPoint> vertSet;
	Point3m vert, edge;
	for (int i = 0; i < vertSetA.size(); ++i)
	{
		edge = vertSetA[i] - projAxis.centerPoint;
		vert = Point3m(edge * projAxis.axisXVector, edge * projAxis.axisYVector, 0);
		vertSet.push_back(FPoint(vert, i, true));
	}
	for (int i = 0; i < vertSetB.size(); ++i)
	{
		edge = vertSetB[i] - projAxis.centerPoint;
		vert = Point3m(edge * projAxis.axisXVector, edge * projAxis.axisYVector, 0);
		vertSet.push_back(FPoint(vert, i, false));
	}

	CommonFuncSet funcSet;
	Axis axis2D(Point3m(0, 0, 0), Point3m(1, 0, 0), Point3m(0, 1, 0), Point3m(0, 0, 1));
	pair<FPoint, FPoint> tangentline = funcSet.findCommonTangentLineFrom(vertSet, axis2D);
	result.vertA = vertSetA[tangentline.first.n];
	result.vertB = vertSetB[tangentline.second.n];

	vector<FPoint>().swap(vertSet);
	return result;
}

void Tooth::updateRealTimeTooth(vcg::Matrix44f transformMatrix)
{
	if (this->realTimeMesh == NULL)
	{
		this->realTimeMesh = new CMeshO();
	}
	else
	{
		this->realTimeMesh->Clear();
	}

	vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(*this->realTimeMesh, *this->p_mesh_);
	for (int i = 0; i < realTimeMesh->vert.size(); ++i)
	{
		if (!realTimeMesh->vert[i].IsD())
		{
			this->realTimeMesh->vert[i].P() = transformMatrix * this->realTimeMesh->vert[i].P();
		}
	}
	for (int i = 0; i < this->intersections_.size(); ++i)
	{
		this->intersections_[i].transformation(transformMatrix);
	}

	vcg::tri::Allocator<CMeshO>::CompactEveryVector(*this->realTimeMesh);
	// Update topology
    if(!tri::HasFFAdjacency(*realTimeMesh))
        realTimeMesh->face.EnableFFAdjacency();
    assert(tri::HasFFAdjacency(*realTimeMesh) == true);

    if (!tri::HasVFAdjacency(*realTimeMesh))
    {
        realTimeMesh->vert.EnableVFAdjacency();
        realTimeMesh->face.EnableVFAdjacency();
    }
    assert(tri::HasVFAdjacency(*realTimeMesh) == true);

	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*realTimeMesh);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(*realTimeMesh);
	// Update normals
	vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(*realTimeMesh);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*realTimeMesh);
	realTimeAxis = updateRealTimeAxisSys(transformMatrix);
}

void Tooth::updateRealTimeSpecialOrigins(vcg::Matrix44f transformMatrix)
{
	this->lateralOrigin_Left = transformMatrix * this->source_LateralOrigin_Left;
	this->lateralOrigin_Right = transformMatrix * this->source_LateralOrigin_Right;
	this->rootOrigin = transformMatrix * this->source_RootOrigin;

	// Transform localAxis to get current realTimeAxis
	Point3m center, axisXVert, axisYVert, axisZVert;
	center = this->localRootAxis.centerPoint;
	axisXVert = center + this->localRootAxis.axisXVector;
	axisYVert = center + this->localRootAxis.axisYVector;
	axisZVert = center + this->localRootAxis.axisZVector;

	center = transformMatrix * center;
	axisXVert = transformMatrix * axisXVert;
	axisYVert = transformMatrix * axisYVert;
	axisZVert = transformMatrix * axisZVert;

	this->realTimeRootAxis.centerPoint = center;
	this->realTimeRootAxis.axisXVector = (axisXVert - center).Normalize();
	this->realTimeRootAxis.axisYVector = (axisYVert - center).Normalize();
	this->realTimeRootAxis.axisZVector = (axisZVert - center).Normalize();
}

float Tooth::computeAngle(Point3m e, Point3m axisX, Point3m axisY)
{
	float angle;

	float cosValue = e.Normalize() * axisX.Normalize();
	if (abs(cosValue) > 1.0f && (abs(cosValue) - 1.0f) < 0.0001f)
	{
		if (cosValue >= 0)
		{
			cosValue = 1.0f;
		}
		else
		{
			cosValue = -1.0f;
		}
	}
	angle = acosf(cosValue) * 180.0f / PI;

	if (e.Normalize() * axisY.Normalize() < 0)
	{
		angle *= -1;
		angle = 360.0f + angle;
	}

	return angle;
}

void Tooth::computeOneOfLipsSurfaceAngles(Tooth &neighborTooth)
{
	Point3m edge;
	edge = getProjPointOnPlane(neighborTooth.realTimeAxis.centerPoint, -realTimeAxis.axisYVector, this->realTimeAxis.centerPoint, realTimeAxis.axisYVector) - this->realTimeAxis.centerPoint;
	this->LipsSurfaceMinAngle = this->computeAngle(edge, this->realTimeAxis.axisXVector, this->realTimeAxis.axisZVector);
	edge = getProjPointOnPlane(this->realTimeAxis.centerPoint, -neighborTooth.realTimeAxis.axisYVector, neighborTooth.realTimeAxis.centerPoint, neighborTooth.realTimeAxis.axisYVector) - neighborTooth.realTimeAxis.centerPoint;
	neighborTooth.LipsSurfaceMaxAngle = this->computeAngle(edge, neighborTooth.realTimeAxis.axisXVector, neighborTooth.realTimeAxis.axisZVector);
	return;
}

void Tooth::computeAxisFromSourceAxis()
{
	Point3m newCenter, newAxisX, newAxisY, newAxisZ;
	newCenter = this->localAxis.centerPoint;
	newAxisX = newCenter + this->localAxis.axisXVector;
	newAxisY = newCenter + this->localAxis.axisYVector;
	newAxisZ = newCenter + this->localAxis.axisZVector;

	vcg::Matrix44f transMat;
	vcg::Matrix44f transform, transformInverse;
	transform.SetIdentity();
	transform.SetTranslate(newCenter);
	transformInverse.SetTranslate(-newCenter);

	vcg::Matrix44f transformValue, rotMatGlobal, rotMatLocal;
	transMat.SetIdentity();
	rotMatGlobal.SetIdentity();
	rotMatLocal.SetIdentity();
	Point3f trans;

	trans = this->localAxis.axisXVector * this->moveVector.X();
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	trans = this->localAxis.axisYVector * this->moveVector.Y();
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	trans = this->localAxis.axisZVector * this->moveVector.Z();
	transformValue.SetTranslate(trans);
	transMat *= transformValue;

	transformValue.SetRotateDeg(this->rotateVector.X(), this->localAxis.axisXVector);
	rotMatLocal *= transformValue;
	transformValue.SetRotateDeg(this->rotateVector.Y(), this->localAxis.axisYVector);
	rotMatLocal *= transformValue;
	transformValue.SetRotateDeg(this->rotateVector.Z(), this->localAxis.axisZVector);
	rotMatLocal *= transformValue;

	transform *= rotMatLocal * transMat;
	transform *= transformInverse;

	//更新所选牙齿的点坐标
	newCenter = transform * newCenter;
	newAxisX = transform * newAxisX;
	newAxisY = transform * newAxisY;
	newAxisZ = transform * newAxisZ;
	this->realTimeAxis.axisXVector = (newAxisX - newCenter).Normalize();
	this->realTimeAxis.axisYVector = (newAxisY - newCenter).Normalize();
	this->realTimeAxis.axisZVector = (newAxisZ - newCenter).Normalize();
	this->realTimeAxis.centerPoint = newCenter;
	this->moveVector = Point3m(0, 0, 0);
	this->rotateVector = Point3m(0, 0, 0);
}

HexaVec Tooth::computeTransformVectors_Root(Point3m &move, Point3m &rotate)
{
	this->localRootAxis.computeTransformVectors(realTimeRootAxis, move, rotate);
	return HexaVec(move.X(), move.Y(), move.Z(), rotate.X(), rotate.Y(), rotate.Z());
}

void Tooth::getRidOfCrevicePoints3(bool bNotChangeMin, bool bNotChangeMax, float fIndex, float fBackIndex)
{
	if (abs(this->LipsSurfaceMinAngle - this->LipsSurfaceMaxAngle) < 10.0f)
	{
		this->LipsSurfaceMinAngle = 0;
		this->LipsSurfaceMaxAngle = 180.0f;
	}

	float frontArea, backArea;
	if (this->LipsSurfaceMinAngle > this->LipsSurfaceMaxAngle)
	{
		frontArea = this->LipsSurfaceMaxAngle - this->LipsSurfaceMinAngle + 360.0f;
	}
	else
	{
		frontArea = this->LipsSurfaceMaxAngle - this->LipsSurfaceMinAngle;
	}
	backArea = 360 - frontArea;

	float frontDeflection, backDeflection;
	frontDeflection = frontArea * fIndex;
	backDeflection = backArea * fBackIndex;

	float backLimMin, backLimMax, frontLimMin, frontLimMax;
	frontLimMin = this->LipsSurfaceMinAngle + frontDeflection;
	if (frontLimMin > 360)
	{
		frontLimMin -= 360;
	}
	frontLimMax = this->LipsSurfaceMaxAngle - frontDeflection;
	if (frontLimMax < 0)
	{
		frontLimMax += 360;
	}
	backLimMin = this->LipsSurfaceMaxAngle + backDeflection;
	if (backLimMin > 360)
	{
		backLimMin -= 360;
	}
	backLimMax = this->LipsSurfaceMinAngle - backDeflection;
	if (backLimMax < 0)
	{
		backLimMax += 360;
	}
	if (bNotChangeMin)
	{
		frontLimMin = 0;
		backLimMax = 360;
	}
	if (bNotChangeMax)
	{
		frontLimMax = 180;
		backLimMin = 180;
	}
	/////////////////////////////////////////////////////////////////////////////////////

	vector<Point3m> frontPTs, backPTs;
	vector<float> dotMulitFront, dotMulitBack;

	float minAngle = 0, maxAngle;
	maxAngle = this->LipsSurfaceMaxAngle - this->LipsSurfaceMinAngle;
	if (maxAngle < 0)
	{
		maxAngle += 360;
	}

	float nearDegreeMin = FLT_MAX, nearDegreeMax = FLT_MAX;
	float tempAngle;
	Point3m tempEdge;
	for (auto vert : this->front_CtrlPTs)
	{
		Point3m projVert = getProjPointOnPlane(vert, -realTimeAxis.axisYVector, realTimeAxis.centerPoint, realTimeAxis.axisYVector);
		tempEdge = (projVert - realTimeAxis.centerPoint).Normalize();
		tempAngle = this->computeAngle(tempEdge, this->realTimeAxis.axisXVector, this->realTimeAxis.axisZVector);

		if (frontLimMin <= frontLimMax)
		{
			if (frontLimMin <= tempAngle && tempAngle < frontLimMax)
			{
				frontPTs.push_back(vert);
				dotMulitFront.push_back(tempAngle);
			}
		}
		else
		{
			if ((frontLimMin <= tempAngle && tempAngle <= 360) || (0 <= tempAngle && tempAngle <= frontLimMax))
			{
				frontPTs.push_back(vert);
				dotMulitFront.push_back(tempAngle);
			}
		}

		if (backLimMin <= backLimMax)
		{
			if (backLimMin <= tempAngle && tempAngle < backLimMax)
			{
				backPTs.push_back(vert);
				dotMulitBack.push_back(tempAngle);
			}
		}
		else
		{
			if ((backLimMax <= tempAngle && tempAngle <= 360) || (0 <= tempAngle && tempAngle <= backLimMin))
			{
				backPTs.push_back(vert);
				dotMulitBack.push_back(tempAngle);
			}
		}

		float curDegreeMin, curDegreeMax;
		curDegreeMin = abs(tempAngle - this->LipsSurfaceMinAngle);
		curDegreeMax = abs(tempAngle - this->LipsSurfaceMaxAngle);
		if (curDegreeMin < nearDegreeMin)
		{
			nearDegreeMin = curDegreeMin;
			this->Prm = vert;
		}
		if (curDegreeMax < nearDegreeMax)
		{
			nearDegreeMax = curDegreeMin;
			this->Plm = vert;
		}
	}

	int i, j;
	float temp;
	Point3m tempPoint;
	vector<Point3f> projVerts;
	projVerts.clear();
	for (i = dotMulitFront.size() - 1; 0 < i; i--) {
		for (j = 0; j < i; j++) {
			if (dotMulitFront[j] > dotMulitFront[j + 1]) {
				temp = dotMulitFront[j];
				dotMulitFront[j] = dotMulitFront[j + 1];
				dotMulitFront[j + 1] = temp;

				tempPoint = frontPTs[j];
				frontPTs[j] = frontPTs[j + 1];
				frontPTs[j + 1] = tempPoint;
			}
		}
	}
	vector<float>().swap(dotMulitFront);

	projVerts.clear();
	for (i = dotMulitBack.size() - 1; 0 < i; i--) {
		for (j = 0; j < i; j++) {
			if (dotMulitBack[j] > dotMulitBack[j + 1]) {
				temp = dotMulitBack[j];
				dotMulitBack[j] = dotMulitBack[j + 1];
				dotMulitBack[j + 1] = temp;

				tempPoint = backPTs[j];
				backPTs[j] = backPTs[j + 1];
				backPTs[j + 1] = tempPoint;
			}
		}
	}
	vector<float>().swap(dotMulitBack);

	this->front_CtrlPTs = frontPTs;
	this->back_CtrlPTs = backPTs;

	vector<Point3m>().swap(frontPTs);
	vector<Point3m>().swap(backPTs);
	return;
}

void Tooth::freshToothBoundaryOnRepairedMesh(CMeshO *dentalMesh)
{
	this->boundaryIndex.clear();

	//更新当前义齿的toothIndex和boundaryIndex
	tri::RequireCompactness(*dentalMesh);
	VertexConstDataWrapper<CMeshO> ww(*dentalMesh);
	KdTree<float> kt(ww);
	typename KdTree<float>::PriorityQueue pq;
	int neighbor = 2;

	int idA, idB;
	float distanceA, distanceB;
	for (auto vPos : this->frozenBoundaryVerts)
	{
		kt.doQueryK(vPos, neighbor, pq);

		idA = pq.getIndex(0);
		idB = pq.getIndex(1);
		distanceA = (dentalMesh->vert[idA].P() - vPos) * (dentalMesh->vert[idA].P() - vPos);
		distanceB = (dentalMesh->vert[idB].P() - vPos) * (dentalMesh->vert[idB].P() - vPos);

		this->boundaryIndex.push_back(distanceA < distanceB ? idA : idB);
	}

	this->collectFrozenBoundaryVerts(dentalMesh);
}

void Tooth::collectFrozenBoundaryVerts(CMeshO *dentalMesh)
{
	if (this->p_mesh_ == nullptr)
	{
		return;
	}
	this->frozenBoundaryVerts.clear();
	for (auto i : this->boundaryIndex)
	{
		this->frozenBoundaryVerts.push_back(dentalMesh->vert[i].P());
	}
}

void Tooth::updateCurrentSignedVerts(Point3m v)
{
	this->curSideVert_L += v;
	this->curSideVert_R += v;
	this->curFeatureVert += v;
}

void Tooth::getCurrentSideVerts(CMeshO *curMesh, Axis curAxis)
{
	float max, min, tempValue;
	Point3m edge, axisX, axisY, axisZ, center;
	axisX = curAxis.axisXVector;
	axisY = curAxis.axisYVector;
	axisZ = curAxis.axisZVector;
	center =curAxis.centerPoint;

	Point3m tempEdge;
	vector<int> filterList;
	for (int i = 0; i < curMesh->vert.size(); ++i)
	{
		tempEdge = (transformMatrix_ * curMesh->vert[i].P() - center).Normalize();
		tempValue = tempEdge * axisZ;
		if (abs(tempValue) < 0.07f)//8度范围
		{
			filterList.push_back(i);
		}
	}

	if (filterList.empty())
	{
		//filterList = this->toothIndex;
		for (int i = 0; i < curMesh->vert.size(); ++i)
		{
			filterList.push_back(i);
		}
	}
	edge = curMesh->vert[filterList[0]].P() - center;

	max = edge * axisX;
	min = max;
	for (auto i : filterList)
	{
		edge = curMesh->vert[i].P() - center;
		tempValue = edge * axisX;
		if (tempValue > max)
		{
			max = tempValue;
			this->sideVert_R = curMesh->vert[i].P();
		}
		if (tempValue <= min)
		{
			min = tempValue;
			this->sideVert_L = curMesh->vert[i].P();
		}
	}
	this->curSideVert_R = this->sideVert_R;
	this->curSideVert_L = this->sideVert_L;
	filterList.clear();
}

void Tooth::computeToothDifferentialVector(Tooth *toothLast, Tooth *toothNext, CustomPlane curPlane)
{
	Point3m spaceVector_Last(0, 0, 0), spaceVector_Next(0, 0, 0);
	if (toothLast != NULL)
	{
		spaceVector_Last = this->computeInterdentalSpaceVector(toothLast, true, curPlane);
	}
	if (toothNext != NULL)
	{
		spaceVector_Next = this->computeInterdentalSpaceVector(toothNext, false, curPlane);
	}

	this->tempSpaceVector_Last = spaceVector_Last;
	this->tempSpaceVector_Next = spaceVector_Next;
	this->differentialVector = Point3m(0, 0, 0);
	this->differentialVector = spaceVector_Last + spaceVector_Next;
	this->fDifferentialVectorLength = sqrtf(differentialVector * differentialVector);
}

Point3m Tooth::computeInterdentalSpaceVector(Tooth *tooth, bool bIsLastOne, CustomPlane curPlane)
{
	Point3m vertA, vertB;
	if (bIsLastOne)
	{
		vertA = this->getProjPointOnPlane(this->curSideVert_R, -curPlane.axisZV, curPlane.center, curPlane.axisZV);
		vertB = this->getProjPointOnPlane(tooth->curSideVert_L, -curPlane.axisZV, curPlane.center, curPlane.axisZV);
	}
	else
	{
		vertA = this->getProjPointOnPlane(this->curSideVert_L, -curPlane.axisZV, curPlane.center, curPlane.axisZV);
		vertB = this->getProjPointOnPlane(tooth->curSideVert_R, -curPlane.axisZV, curPlane.center, curPlane.axisZV);
	}
	Point3m result = this->realTimeAxis.axisXVector * ((vertB - vertA) * this->realTimeAxis.axisXVector);
	//Point3m result = vertB - vertA;
	return result;
}

void Tooth::computeEachOtherDiffVector(Tooth *toothOther, bool bIsLastOne, CustomPlane curPlane)
{
	Point3m vertA, vertB;
	if (bIsLastOne)//toothOther is toothThis Last One Tooth
	{
		vertA = this->curSideVert_R;
		vertB = toothOther->curSideVert_L;
	}
	else
	{
		vertA = this->curSideVert_L;
		vertB = toothOther->curSideVert_R;
	}
	vertA = this->getProjPointOnPlane(vertA, -curPlane.axisZV, curPlane.center, curPlane.axisZV);
	vertB = this->getProjPointOnPlane(vertB, -curPlane.axisZV, curPlane.center, curPlane.axisZV);

	Point3m commonEdge, selfDirect, otherDirect;
	float projSelf, projOhter, lengthCommon;
	commonEdge = vertB - vertA;
	selfDirect = (vertB - vertA).Normalize();
	otherDirect = selfDirect * -1.0f;

	projSelf = abs(commonEdge * this->realTimeAxis.axisXVector);
	projOhter = abs(commonEdge * toothOther->realTimeAxis.axisXVector);
	lengthCommon = projSelf < projOhter ? projSelf : projOhter;

	Point3m selfVector, otherVector;
	selfVector = selfDirect * lengthCommon;
	otherVector = otherDirect * lengthCommon;

	if (bIsLastOne)//toothOther is toothThis Last One Tooth
	{
		this->tempSpaceVector_Last = this->realTimeAxis.axisXVector * (selfVector * this->realTimeAxis.axisXVector);
		this->tempSpaceVector_Next = Point3m(0, 0, 0);
		this->differentialVector = Point3m(0, 0, 0);
		this->differentialVector = tempSpaceVector_Last + tempSpaceVector_Next;
		this->fDifferentialVectorLength = sqrtf(differentialVector * differentialVector);

		toothOther->tempSpaceVector_Next = toothOther->realTimeAxis.axisXVector * (otherVector * toothOther->realTimeAxis.axisXVector);
		toothOther->tempSpaceVector_Last = Point3m(0, 0, 0);
		toothOther->differentialVector = Point3m(0, 0, 0);
		toothOther->differentialVector = toothOther->tempSpaceVector_Last + toothOther->tempSpaceVector_Next;
		toothOther->fDifferentialVectorLength = sqrtf(toothOther->differentialVector * toothOther->differentialVector);
	}
	else
	{
		this->tempSpaceVector_Next = this->realTimeAxis.axisXVector * (selfVector * this->realTimeAxis.axisXVector);
		this->tempSpaceVector_Last = Point3m(0, 0, 0);
		this->differentialVector = Point3m(0, 0, 0);
		this->differentialVector = tempSpaceVector_Last + tempSpaceVector_Next;
		this->fDifferentialVectorLength = sqrtf(differentialVector * differentialVector);

		toothOther->tempSpaceVector_Last = toothOther->realTimeAxis.axisXVector * (otherVector * toothOther->realTimeAxis.axisXVector);
		toothOther->tempSpaceVector_Next = Point3m(0, 0, 0);
		toothOther->differentialVector = Point3m(0, 0, 0);
		toothOther->differentialVector = toothOther->tempSpaceVector_Last + toothOther->tempSpaceVector_Next;
		toothOther->fDifferentialVectorLength = sqrtf(toothOther->differentialVector * toothOther->differentialVector);
	}
}

void Tooth::updateDisplayedParameters()
{
	int iFDI = this->strFDI.toInt();
	int iSign = iFDI / 10;
	this->displayedX = this->getDisplayedX(iSign, AimHexaVec.fVX);
	this->displayedY = this->getDisplayedY(iSign, AimHexaVec.fVY);
	this->displayedZ = this->getDisplayedZ(iSign, AimHexaVec.fVZ);
	this->displayedA = this->getDisplayedA(iSign, AimHexaVec.fA);
	this->displayedB = this->getDisplayedB(iSign, AimHexaVec.fB);
	this->displayedG = this->getDisplayedG(iSign, AimHexaVec.fG);

	this->displayedX_Root = this->getDisplayedRootX(iSign, AimHexaVec_Root.fVX);
	this->displayedY_Root = this->getDisplayedRootY(iSign, AimHexaVec_Root.fVY);
	this->displayedZ_Root = this->getDisplayedRootZ(iSign, AimHexaVec_Root.fVZ);
	this->displayedA_Root = this->getDisplayedRootA(iSign, AimHexaVec_Root.fA);
	this->displayedB_Root = this->getDisplayedRootB(iSign, AimHexaVec_Root.fB);
	this->displayedG_Root = this->getDisplayedRootG(iSign, AimHexaVec_Root.fG);
}

QString Tooth::getDisplayedX(int Sign, float value)
{
	QString result;
	value = keepNDecimalPlace(2, value);
	//B远中，A近中
	QString B = QString("D");
	QString A = QString("M");
	switch (Sign)
	{
	case 1:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 2:
		if (value > 0)
		{
			result = QString::number(value) + A;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + B;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 3:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 4:
		if (value > 0)
		{
			result = QString::number(value) + A;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + B;
		}
		else
		{
			result = QString::number(0);
		}
		break;
	}
	return result;
}

QString Tooth::getDisplayedY(int Sign, float value)
{
	QString result;
	value = keepNDecimalPlace(2, value);
	//B升高，A压低
	QString B = QString("E");
	QString A = QString("I");
	switch (Sign)
	{
	case 1:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 2:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 3:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 4:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;
	}
	return result;
}

QString Tooth::getDisplayedZ(int Sign, float value)
{
	QString result;
	value = keepNDecimalPlace(2, value);
	//B唇侧，A舌侧
	QString B = QString("B");
	QString A = QString("L");
	switch (Sign)
	{
	case 1:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 2:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 3:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 4:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;
	}
	return result;
}

QString Tooth::getDisplayedA(int Sign, float value)
{
	QString result;
	value = keepNDecimalPlace(2, value);
	//B唇侧，A舌侧
	QString B = QString("B");
	QString A = QString("L");
	switch (Sign)
	{
	case 1:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 2:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 3:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 4:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;
	}
	return result;
}

QString Tooth::getDisplayedB(int Sign, float value)
{
	QString result;
	value = keepNDecimalPlace(2, value);
	//B远中，A近中
	QString B = QString("D");
	QString A = QString("M");
	switch (Sign)
	{
	case 1:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 2:
		if (value > 0)
		{
			result = QString::number(value) + A;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + B;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 3:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 4:
		if (value > 0)
		{
			result = QString::number(value) + A;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + B;
		}
		else
		{
			result = QString::number(0);
		}
		break;
	}
	return result;
}

QString Tooth::getDisplayedG(int Sign, float value)
{
	QString result;
	value = keepNDecimalPlace(2, value);
	//B远中，A近中
	QString B = QString("D");
	QString A = QString("M");
	switch (Sign)
	{
	case 1:
		if (value > 0)
		{
			result = QString::number(value) + A;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + B;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 2:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 3:
		if (value > 0)
		{
			result = QString::number(value) + A;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + B;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 4:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;
	}
	return result;
}

QString Tooth::getDisplayedRootX(int Sign, float value)
{
	QString result;
	value = keepNDecimalPlace(2, value);
	//B远中，A近中
	QString B = QString("D");
	QString A = QString("M");
	switch (Sign)
	{
	case 1:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 2:
		if (value > 0)
		{
			result = QString::number(value) + A;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + B;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 3:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 4:
		if (value > 0)
		{
			result = QString::number(value) + A;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + B;
		}
		else
		{
			result = QString::number(0);
		}
		break;
	}
	return result;
}
QString Tooth::getDisplayedRootY(int Sign, float value)
{
	QString result;
	value = keepNDecimalPlace(2, value);
	//B升高，A压低
	QString B = QString("E");
	QString A = QString("I");
	switch (Sign)
	{
	case 1:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 2:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 3:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 4:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;
	}
	return result;
}
QString Tooth::getDisplayedRootZ(int Sign, float value)
{
	QString result;
	value = keepNDecimalPlace(2, value);
	//B唇侧，A舌侧
	QString B = QString("B");
	QString A = QString("L");
	switch (Sign)
	{
	case 1:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 2:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 3:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 4:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;
	}
	return result;
}
QString Tooth::getDisplayedRootA(int Sign, float value)
{
	QString result;
	value = keepNDecimalPlace(2, value);
	//B舌侧，A唇侧
	QString B = QString("L");
	QString A = QString("B");
	switch (Sign)
	{
	case 1:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 2:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 3:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 4:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;
	}
	return result;
}
QString Tooth::getDisplayedRootB(int Sign, float value)
{
	QString result;
	value = keepNDecimalPlace(2, value);
	//B远中，A近中
	QString B = QString("D");
	QString A = QString("M");
	switch (Sign)
	{
	case 1:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 2:
		if (value > 0)
		{
			result = QString::number(value) + A;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + B;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 3:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 4:
		if (value > 0)
		{
			result = QString::number(value) + A;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + B;
		}
		else
		{
			result = QString::number(0);
		}
		break;
	}
	return result;
}
QString Tooth::getDisplayedRootG(int Sign, float value)
{
	QString result;
	value = keepNDecimalPlace(2, value);
	//B近中，A远中
	QString B = QString("M");
	QString A = QString("D");
	switch (Sign)
	{
	case 1:
		if (value > 0)
		{
			result = QString::number(value) + A;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + B;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 2:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 3:
		if (value > 0)
		{
			result = QString::number(value) + A;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + B;
		}
		else
		{
			result = QString::number(0);
		}
		break;

	case 4:
		if (value > 0)
		{
			result = QString::number(value) + B;
		}
		else if (value < 0)
		{
			result = QString::number(abs(value)) + A;
		}
		else
		{
			result = QString::number(0);
		}
		break;
	}
	return result;
}

float Tooth::keepNDecimalPlace(int n, float value)
{
	float result;
	float bang = powf(10.0f, n);
	value *= bang;
	int tempValue = (value);
	result = tempValue / bang;
	return result;
}

void Tooth::adjToothAimParameter(TrigerType type, float stepValue)
{
    int iFDI = this->strFDI.toInt();
    int iSign = iFDI / 10;

    float *value = nullptr;
    if (iSign <= 0 || iSign > 4)
        return;
    switch (type)
    {
    case XPLUS:
    case XMINS:
        value = &AimHexaVec.fVX;
        break;
    case YPLUS:
    case YMINS:
        value = &AimHexaVec.fVY;
        break;
    case ZPLUS:
    case ZMINS:
        value = &AimHexaVec.fVZ;
        break;
    case APLUS:
    case AMINS:
        value = &AimHexaVec.fA;
        break;
    case BPLUS:
    case BMINS:
        value = &AimHexaVec.fB;
        break;
    case GPLUS:
    case GMINS:
        value = &AimHexaVec.fG;
        break;
    }
    switch (type)
    {
    case XPLUS:
    case BPLUS:
    case GPLUS:
        if (iSign == 1 || iSign == 3)
            stepValue *= -1.f;
        break;
    case XMINS:
    case BMINS:
    case GMINS:
        if (iSign == 2 || iSign == 4)
            stepValue *= -1.f;
        break;
    case YMINS:
    case ZMINS:
    case AMINS:
        stepValue *= -1.f;
        break;

    default:
        break;
    }
    if(value)
    {
        *value += stepValue;
    }
}

void Tooth::adjToothAimParameter(InputType type, float setValue)
{
	int iFDI = this->strFDI.toInt();
	int iSign = iFDI / 10;

	switch (type)
	{
	case INPUT_X:
		//this->adjToothAimParameter_X(setValue);
        adjToothAimParameter(setValue, this->AimHexaVec.fVX);
		break;

	case INPUT_Y:
		//this->adjToothAimParameter_Y(setValue);
        adjToothAimParameter(setValue, AimHexaVec.fVY);
		break;

	case INPUT_Z:
		//this->adjToothAimParameter_Z(setValue);
        adjToothAimParameter(setValue, AimHexaVec.fVZ);
		break;

	case INPUT_A:
		//this->adjToothAimParameter_A(setValue);
        adjToothAimParameter(setValue, AimHexaVec.fA);
		break;

	case INPUT_B:
		//this->adjToothAimParameter_B(setValue);
        adjToothAimParameter(setValue, AimHexaVec.fB);
		break;

	case INPUT_G:
		//this->adjToothAimParameter_G(setValue);
        adjToothAimParameter(setValue, AimHexaVec.fG);
		break;
	}
}
void Tooth::adjToothAimParameter(float value, float &parameter)
{
    parameter = parameter > 0 ? value : (value * -1.0f);
}
//		setValue = abs(setValue);
//
//
//
//
//	else
//	{
//		setValue = abs(setValue);
//
//
//
//
//	else
//	{
//		setValue = abs(setValue);
//
//
bool Tooth::stressTransfer()
{
	if (this->rigid == NULL)
	{
		return true;
	}
	if (this->rigid->activeRigid != NULL)
	{
		return true;
	}

	this->rigid->stressTransferVector = Point3m(0, 0, 0);
	this->rigid->currentConvex = this->convexVerts;
	this->rigid->currentConvexCenter = this->realTimeAxis.centerPoint;

	bool resistanceHappend = false;
	for (int i = 0; i < this->rigid->passiveRigids.size(); ++i)
	{
		if (this->rigid->passiveRigids[i]->updateCurrentConvex())
		{
			resistanceHappend = true;
		}
	}
	return resistanceHappend;
}

void Tooth::updateRigidbody()
{
	if (this->rigid != NULL)
	{
		delete this->rigid;
		this->rigid = NULL;
	}

    if (rigid == nullptr)
    {
        rigid = new Rigidbody();
    }
	Point3m axisXVert, axisYVert, axisZVert;
	axisXVert = this->realTimeAxis.centerPoint + this->realTimeAxis.axisXVector;
	axisYVert = this->realTimeAxis.centerPoint + this->realTimeAxis.axisYVector;
	axisZVert = this->realTimeAxis.centerPoint + this->realTimeAxis.axisZVector;
    rigid->initData(convexVerts, realTimeAxis.centerPoint);

	rigid->setFixed(this->fixed());
}

bool Tooth::isInFrontOfTheTooth(Tooth *otherTooth, Point3m cameraPosition, Point3m viewDirect, bool bCameraFront)
{
	Point3m selfNode, otherNode;
	if (bCameraFront)
	{
		selfNode = (this->localBoundbox.upLoop[2] + this->localBoundbox.upLoop[3]) / 2.0f;
		otherNode = (otherTooth->localBoundbox.upLoop[2] + otherTooth->localBoundbox.upLoop[3]) / 2.0f;
		cameraPosition = this->realTimeAxis.centerPoint + this->realTimeAxis.axisZVector * 3.0f;
		viewDirect = -this->realTimeAxis.axisZVector;
	}
	else
	{
		selfNode = (this->localBoundbox.upLoop[0] + this->localBoundbox.upLoop[1]) / 2.0f;
		otherNode = (otherTooth->localBoundbox.upLoop[0] + otherTooth->localBoundbox.upLoop[1]) / 2.0f;
		cameraPosition = this->realTimeAxis.centerPoint - this->realTimeAxis.axisZVector * 10.0f;
		viewDirect = this->realTimeAxis.axisZVector;
	}

	if ((abs((selfNode - cameraPosition) * viewDirect)) <= (abs((otherNode - cameraPosition) * viewDirect)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Tooth::depthRealTimeIntersectionTestWith(CMeshO *aimMesh, Axis &virtualEye, vector<pair<int, Point3m>> &result)
{
	if (aimMesh == nullptr)
	{
		vector<pair<int, Point3m>>().swap(result);
		return;
	}

	vcg::tri::UpdateBounding<CMeshO>::Box(*aimMesh);
	//选取并投影牙齿操作面片与点
	bool bVisible = true;
	vector<int> selfFaceList, otherFaceList, selfVertList, otherVertList;
	vector<FFace> proj2DFacesSelf, proj2DFacesOther;
	vector<Point2m> proj2DVertsSelf, proj2DVertsOther;
	Point2i maxProjVertSelf, minProjVertSelf, maxProjVertOther, minProjVertOther;
	Point3m vertLimitOriginSelf, vertLimitOriginOther;
	vertLimitOriginOther = aimMesh->bbox.Center();
	vertLimitOriginSelf = this->realTimeAxis.centerPoint;
	this->collectTooth2DProjFaces(this->realTimeMesh, bVisible, virtualEye, vertLimitOriginSelf, selfFaceList, selfVertList, proj2DFacesSelf, proj2DVertsSelf, maxProjVertSelf, minProjVertSelf);
	this->collectTooth2DProjFaces(aimMesh, !bVisible, virtualEye, vertLimitOriginOther, otherFaceList, otherVertList, proj2DFacesOther, proj2DVertsOther, maxProjVertOther, minProjVertOther);

	//根据投影区域，确定深度填充区域大小
	int depthAreaWidth, depthAreaHeight;
	int maxX, minX, maxY, minY;
	maxX = maxProjVertSelf.X() > maxProjVertOther.X() ? maxProjVertSelf.X() : maxProjVertOther.X();
	maxY = maxProjVertSelf.Y() > maxProjVertOther.Y() ? maxProjVertSelf.Y() : maxProjVertOther.Y();
	minX = minProjVertSelf.X() < minProjVertOther.X() ? minProjVertSelf.X() : minProjVertOther.X();
	minY = minProjVertSelf.Y() < minProjVertOther.Y() ? minProjVertSelf.Y() : minProjVertOther.Y();
	depthAreaHeight = (maxY - minY) + 4;
	depthAreaWidth = (maxX - minX) + 4;
	Point2i matrixOrigin = Point2i(-depthAreaWidth / 2, depthAreaHeight / 2);

	for (int i = 0; i < proj2DFacesSelf.size(); ++i)
	{
		proj2DFacesSelf[i].v1.X() -= matrixOrigin.X();
		proj2DFacesSelf[i].v1.Y() = matrixOrigin.Y() - proj2DFacesSelf[i].v1.Y();
		proj2DFacesSelf[i].v2.X() -= matrixOrigin.X();
		proj2DFacesSelf[i].v2.Y() = matrixOrigin.Y() - proj2DFacesSelf[i].v2.Y();
		proj2DFacesSelf[i].v3.X() -= matrixOrigin.X();
		proj2DFacesSelf[i].v3.Y() = matrixOrigin.Y() - proj2DFacesSelf[i].v3.Y();
	}
	for (int i = 0; i < proj2DFacesOther.size(); ++i)
	{
		proj2DFacesOther[i].v1.X() -= matrixOrigin.X();
		proj2DFacesOther[i].v1.Y() = matrixOrigin.Y() - proj2DFacesOther[i].v1.Y();
		proj2DFacesOther[i].v2.X() -= matrixOrigin.X();
		proj2DFacesOther[i].v2.Y() = matrixOrigin.Y() - proj2DFacesOther[i].v2.Y();
		proj2DFacesOther[i].v3.X() -= matrixOrigin.X();
		proj2DFacesOther[i].v3.Y() = matrixOrigin.Y() - proj2DFacesOther[i].v3.Y();
	}
	//开始深度插值
	float **selfDepthMap = nullptr, **otherDepthMap = nullptr;
	selfDepthMap = new float*[depthAreaWidth];
	for (int i = 0; i < depthAreaWidth; ++i)
	{
		selfDepthMap[i] = new float[depthAreaHeight];
		for (int j = 0; j < depthAreaHeight; ++j)
		{
			selfDepthMap[i][j] = -1;
		}
	}
	otherDepthMap = new float*[depthAreaWidth];
	for (int i = 0; i < depthAreaWidth; ++i)
	{
		otherDepthMap[i] = new float[depthAreaHeight];
		for (int j = 0; j < depthAreaHeight; ++j)
		{
			otherDepthMap[i][j] = -1;
		}
	}
	coloredMapWithDepth(selfDepthMap, &proj2DFacesSelf, depthAreaWidth, depthAreaHeight);
	coloredMapWithDepth(otherDepthMap, &proj2DFacesOther, depthAreaWidth, depthAreaHeight);

	if (selfDepthMap != nullptr || otherDepthMap != nullptr)
	{
		//对比网格顶点在底片上投影深度值和在另一底片上深度值是否满足交叠条件
		for (int i = 0; i < proj2DVertsSelf.size(); ++i)
		{
			int x, y;
			x = proj2DVertsSelf[i].X() - matrixOrigin.X();
			y = matrixOrigin.Y() - proj2DVertsSelf[i].Y();
			if (0 <= x && x < depthAreaWidth && 0 <= y && y < depthAreaHeight)
			{
				float selfDepth = selfDepthMap[x][y];
				float otherDepth = otherDepthMap[x][y];
				if (selfDepth != -1 && otherDepth != -1)
				{
					if (selfDepth <= otherDepth)
					{
						Point3m adjMoveVector(0, 0, 0);
						adjMoveVector = virtualEye.axisZVector * (otherDepth - selfDepth);
						pair<int, Point3m> record;
						record.first = selfVertList[i];
						record.second = this->realTimeMesh->vert[selfVertList[i]].P() + adjMoveVector;
						result.push_back(record);
					}
				}
			}
		}
	}

	if (selfDepthMap != nullptr)
	{
		for (int i = 0; i < depthAreaWidth; ++i)
		{
			delete[]selfDepthMap[i];
		}
		delete[]selfDepthMap;
	}
	if (otherDepthMap != nullptr)
	{
		for (int i = 0; i < depthAreaWidth; ++i)
		{
			delete[]otherDepthMap[i];
		}
		delete[]otherDepthMap;
	}

	return;
}

vector<Point3m> Tooth::getOverlapAreaVertsWithOtherTooth(Tooth *otherTooth, CMeshO *curMesh)
{
	vector<Point3m> result;
	//建立虚拟视觉坐标系
	Axis virtualEye;
	virtualEye.centerPoint = this->realTimeAxis.centerPoint;
	virtualEye.axisZVector = (otherTooth->realTimeAxis.centerPoint - this->realTimeAxis.centerPoint).Normalize();
	virtualEye.axisYVector = getProjPointOnPlane(virtualEye.centerPoint + this->realTimeAxis.axisYVector, -virtualEye.axisZVector, virtualEye.centerPoint, virtualEye.axisZVector) - virtualEye.centerPoint;
	virtualEye.axisYVector = virtualEye.axisYVector.Normalize();
	virtualEye.axisXVector = (virtualEye.axisZVector ^ virtualEye.axisYVector).Normalize();
	//选取并投影牙齿操作面片与点
	bool bVisible = true;
	vector<int> selfFaceList, otherFaceList, selfVertList, otherVertList;
	vector<FFace> proj2DFacesSelf, proj2DFacesOther;
	vector<Point2m> proj2DVertsSelf, proj2DVertsOther;
	Point2i maxProjVertSelf, minProjVertSelf, maxProjVertOther, minProjVertOther;
	Point3m vertLimitOriginSelf, vertLimitOriginOther;
	if (this->strFDI.toInt() / 10 == 1 || this->strFDI.toInt() / 10 == 2)
	{
		vertLimitOriginSelf = (this->Plb + this->Plf) / 2.0f;
		vertLimitOriginOther = (otherTooth->Prb + otherTooth->Prf) / 2.0f;
	}
	else
	{
		vertLimitOriginSelf = (this->Prb + this->Prf) / 2.0f;
		vertLimitOriginOther = (otherTooth->Plb + otherTooth->Plf) / 2.0f;
	}
	this->collectTooth2DProjFaces(curMesh, this, !bVisible, virtualEye, vertLimitOriginSelf, selfFaceList, selfVertList, proj2DFacesSelf, proj2DVertsSelf, maxProjVertSelf, minProjVertSelf);
	this->collectTooth2DProjFaces(curMesh, otherTooth, bVisible, virtualEye, vertLimitOriginOther, otherFaceList, otherVertList, proj2DFacesOther, proj2DVertsOther, maxProjVertOther, minProjVertOther);
	//根据投影区域，确定深度填充区域大小
	int depthAreaWidth, depthAreaHeight;
	int maxX, minX, maxY, minY;
	maxX = maxProjVertSelf.X() > maxProjVertOther.X() ? maxProjVertSelf.X() : maxProjVertOther.X();
	maxY = maxProjVertSelf.Y() > maxProjVertOther.Y() ? maxProjVertSelf.Y() : maxProjVertOther.Y();
	minX = minProjVertSelf.X() < minProjVertOther.X() ? minProjVertSelf.X() : minProjVertOther.X();
	minY = minProjVertSelf.Y() < minProjVertOther.Y() ? minProjVertSelf.Y() : minProjVertOther.Y();
	depthAreaHeight = (maxY - minY) + 4;
	depthAreaWidth = (maxX - minX) + 4;
	Point2i matrixOrigin = Point2i(-depthAreaWidth / 2, depthAreaHeight / 2);
	float **selfDepthMap, **otherDepthMap;
	selfDepthMap = new float*[depthAreaWidth];
	for (int i = 0; i < depthAreaWidth; ++i)
	{
		selfDepthMap[i] = new float[depthAreaHeight];
		for (int j = 0; j < depthAreaHeight; ++j)
		{
			selfDepthMap[i][j] = -1;
		}
	}
	otherDepthMap = new float*[depthAreaWidth];
	for (int i = 0; i < depthAreaWidth; ++i)
	{
		otherDepthMap[i] = new float[depthAreaHeight];
		for (int j = 0; j < depthAreaHeight; ++j)
		{
			otherDepthMap[i][j] = -1;
		}
	}
	for (int i = 0; i < proj2DFacesSelf.size(); ++i)
	{
		proj2DFacesSelf[i].v1.X() -= matrixOrigin.X();
		proj2DFacesSelf[i].v1.Y() = matrixOrigin.Y() - proj2DFacesSelf[i].v1.Y();
		proj2DFacesSelf[i].v2.X() -= matrixOrigin.X();
		proj2DFacesSelf[i].v2.Y() = matrixOrigin.Y() - proj2DFacesSelf[i].v2.Y();
		proj2DFacesSelf[i].v3.X() -= matrixOrigin.X();
		proj2DFacesSelf[i].v3.Y() = matrixOrigin.Y() - proj2DFacesSelf[i].v3.Y();
	}
	for (int i = 0; i < proj2DFacesOther.size(); ++i)
	{
		proj2DFacesOther[i].v1.X() -= matrixOrigin.X();
		proj2DFacesOther[i].v1.Y() = matrixOrigin.Y() - proj2DFacesOther[i].v1.Y();
		proj2DFacesOther[i].v2.X() -= matrixOrigin.X();
		proj2DFacesOther[i].v2.Y() = matrixOrigin.Y() - proj2DFacesOther[i].v2.Y();
		proj2DFacesOther[i].v3.X() -= matrixOrigin.X();
		proj2DFacesOther[i].v3.Y() = matrixOrigin.Y() - proj2DFacesOther[i].v3.Y();
	}
	//开始深度插值
	coloredMapWithDepth(selfDepthMap, &proj2DFacesSelf, depthAreaWidth, depthAreaHeight);
	coloredMapWithDepth(otherDepthMap, &proj2DFacesOther, depthAreaWidth, depthAreaHeight);
	//对比网格顶点在底片上投影深度值和在另一底片上深度值是否满足交叠条件
	for (int i = 0; i < proj2DVertsSelf.size(); ++i)
	{
		int x, y;
		x = proj2DVertsSelf[i].X() - matrixOrigin.X();
		y = matrixOrigin.Y() - proj2DVertsSelf[i].Y();
		if (0 <= x && x < depthAreaWidth && 0 <= y && y < depthAreaHeight)
		{
			float selfDepth = selfDepthMap[x][y];
			float otherDepth = otherDepthMap[x][y];
			if (selfDepth != -1 && otherDepth != -1 && selfDepth >= otherDepth)
			{
				result.push_back(curMesh->vert[selfVertList[i]].P());
			}
		}
	}

	for (int i = 0; i < proj2DVertsOther.size(); ++i)
	{
		int x, y;
		x = proj2DVertsOther[i].X() - matrixOrigin.X();
		y = matrixOrigin.Y() - proj2DVertsOther[i].Y();
		if (0 <= x && x < depthAreaWidth && 0 <= y && y < depthAreaHeight)
		{
			float selfDepth = selfDepthMap[x][y];
			float otherDepth = otherDepthMap[x][y];
			if (selfDepth != -1 && otherDepth != -1 && selfDepth >= otherDepth)
			{
				result.push_back(curMesh->vert[otherVertList[i]].P());
			}
		}
	}

	for (int i = 0; i < depthAreaWidth; ++i)
	{
		delete[]selfDepthMap[i];
	}
	delete[]selfDepthMap;
	for (int i = 0; i < depthAreaWidth; ++i)
	{
		delete[]otherDepthMap[i];
	}
	delete[]otherDepthMap;

	return result;
}

vector<Point3m> Tooth::getOverlapAreaVertsWithOtherToothSourceMesh(Tooth *otherTooth)
{
	//建立虚拟视觉坐标系
	Axis virtualEye;
	virtualEye.centerPoint = this->localAxis.centerPoint;
	virtualEye.axisZVector = (otherTooth->localAxis.centerPoint - this->localAxis.centerPoint).Normalize();
	virtualEye.axisYVector = getProjPointOnPlane(virtualEye.centerPoint + this->localAxis.axisYVector, -virtualEye.axisZVector, virtualEye.centerPoint, virtualEye.axisZVector) - virtualEye.centerPoint;
	virtualEye.axisYVector = virtualEye.axisYVector.Normalize();
	virtualEye.axisXVector = (virtualEye.axisZVector ^ virtualEye.axisYVector).Normalize();

	//选取并投影牙齿操作面片与点
	bool bVisible = true;
	vector<int> selfFaceList, otherFaceList, selfVertList, otherVertList;
	vector<FFace> proj2DFacesSelf, proj2DFacesOther;
	vector<Point2m> proj2DVertsSelf, proj2DVertsOther;
	Point2i maxProjVertSelf, minProjVertSelf, maxProjVertOther, minProjVertOther;
	Point3m vertLimitOriginSelf, vertLimitOriginOther;
	if (this->strFDI.toInt() / 10 == 1 || this->strFDI.toInt() / 10 == 2)
	{
		vertLimitOriginSelf = (this->Plb + this->Plf) / 2.0f;
		vertLimitOriginOther = (otherTooth->Prb + otherTooth->Prf) / 2.0f;
	}
	else
	{
		vertLimitOriginSelf = (this->Prb + this->Prf) / 2.0f;
		vertLimitOriginOther = (otherTooth->Plb + otherTooth->Plf) / 2.0f;
	}
	this->collectTooth2DProjFaces(this->p_mesh_, this, !bVisible, virtualEye, vertLimitOriginSelf, selfFaceList, selfVertList, proj2DFacesSelf, proj2DVertsSelf, maxProjVertSelf, minProjVertSelf);
	this->collectTooth2DProjFaces(otherTooth->p_mesh_, otherTooth, bVisible, virtualEye, vertLimitOriginOther, otherFaceList, otherVertList, proj2DFacesOther, proj2DVertsOther, maxProjVertOther, minProjVertOther);

	//根据投影区域，确定深度填充区域大小
	int depthAreaWidth, depthAreaHeight;
	int maxX, minX, maxY, minY;
	maxX = maxProjVertSelf.X() > maxProjVertOther.X() ? maxProjVertSelf.X() : maxProjVertOther.X();
	maxY = maxProjVertSelf.Y() > maxProjVertOther.Y() ? maxProjVertSelf.Y() : maxProjVertOther.Y();
	minX = minProjVertSelf.X() < minProjVertOther.X() ? minProjVertSelf.X() : minProjVertOther.X();
	minY = minProjVertSelf.Y() < minProjVertOther.Y() ? minProjVertSelf.Y() : minProjVertOther.Y();
	depthAreaHeight = (maxY - minY) + 4;
	depthAreaWidth = (maxX - minX) + 4;
	Point2i matrixOrigin = Point2i(-depthAreaWidth / 2, depthAreaHeight / 2);
	float **selfDepthMap, **otherDepthMap;
	selfDepthMap = new float*[depthAreaWidth];
	for (int i = 0; i < depthAreaWidth; ++i)
	{
		selfDepthMap[i] = new float[depthAreaHeight];
		for (int j = 0; j < depthAreaHeight; ++j)
		{
			selfDepthMap[i][j] = -1;
		}
	}
	otherDepthMap = new float*[depthAreaWidth];
	for (int i = 0; i < depthAreaWidth; ++i)
	{
		otherDepthMap[i] = new float[depthAreaHeight];
		for (int j = 0; j < depthAreaHeight; ++j)
		{
			otherDepthMap[i][j] = -1;
		}
	}
	for (int i = 0; i < proj2DFacesSelf.size(); ++i)
	{
		proj2DFacesSelf[i].v1.X() -= matrixOrigin.X();
		proj2DFacesSelf[i].v1.Y() = matrixOrigin.Y() - proj2DFacesSelf[i].v1.Y();
		proj2DFacesSelf[i].v2.X() -= matrixOrigin.X();
		proj2DFacesSelf[i].v2.Y() = matrixOrigin.Y() - proj2DFacesSelf[i].v2.Y();
		proj2DFacesSelf[i].v3.X() -= matrixOrigin.X();
		proj2DFacesSelf[i].v3.Y() = matrixOrigin.Y() - proj2DFacesSelf[i].v3.Y();
	}
	for (int i = 0; i < proj2DFacesOther.size(); ++i)
	{
		proj2DFacesOther[i].v1.X() -= matrixOrigin.X();
		proj2DFacesOther[i].v1.Y() = matrixOrigin.Y() - proj2DFacesOther[i].v1.Y();
		proj2DFacesOther[i].v2.X() -= matrixOrigin.X();
		proj2DFacesOther[i].v2.Y() = matrixOrigin.Y() - proj2DFacesOther[i].v2.Y();
		proj2DFacesOther[i].v3.X() -= matrixOrigin.X();
		proj2DFacesOther[i].v3.Y() = matrixOrigin.Y() - proj2DFacesOther[i].v3.Y();
	}

	//开始深度插值
	coloredMapWithDepth(selfDepthMap, &proj2DFacesSelf, depthAreaWidth, depthAreaHeight);
	coloredMapWithDepth(otherDepthMap, &proj2DFacesOther, depthAreaWidth, depthAreaHeight);

	//对比网格顶点在底片上投影深度值和在另一底片上深度值是否满足交叠条件
	vector<Point3m> result;
	for (int i = 0; i < proj2DVertsSelf.size(); ++i)
	{
		int x, y;
		x = proj2DVertsSelf[i].X() - matrixOrigin.X();
		y = matrixOrigin.Y() - proj2DVertsSelf[i].Y();
		if (0 <= x && x < depthAreaWidth && 0 <= y && y < depthAreaHeight)
		{
			float selfDepth = selfDepthMap[x][y];
			float otherDepth = otherDepthMap[x][y];
			if (selfDepth != -1 && otherDepth != -1 && selfDepth >= otherDepth)
			{
				result.push_back(p_mesh_->vert[selfVertList[i]].P());
			}
		}
	}

	for (int i = 0; i < proj2DVertsOther.size(); ++i)
	{
		int x, y;
		x = proj2DVertsOther[i].X() - matrixOrigin.X();
		y = matrixOrigin.Y() - proj2DVertsOther[i].Y();
		if (0 <= x && x < depthAreaWidth && 0 <= y && y < depthAreaHeight)
		{
			float selfDepth = selfDepthMap[x][y];
			float otherDepth = otherDepthMap[x][y];
			if (selfDepth != -1 && otherDepth != -1 && selfDepth >= otherDepth)
			{
				result.push_back(otherTooth->p_mesh_->vert[otherVertList[i]].P());
			}
		}
	}

	for (int i = 0; i < depthAreaWidth; ++i)
	{
		delete[]selfDepthMap[i];
	}
	delete[]selfDepthMap;
	for (int i = 0; i < depthAreaWidth; ++i)
	{
		delete[]otherDepthMap[i];
	}
	delete[]otherDepthMap;

	return result;
}

void Tooth::collectTooth2DProjFaces(CMeshO *curMesh, bool bVisible, Axis eye, Point3m limitOrigin, vector<int> &faceList, vector<int> &vertList, vector<FFace> &proj2DFaces, vector<Point2m> &proj2DVerts,
	Point2i &maxProjVert, Point2i &minProjVert)
{
	Point3m baryCenter, edgeVector;
	//collect useful faces Indexes
	faceList.clear();
	for (int i = 0; i < curMesh->face.size(); ++i)
	{
		baryCenter = (curMesh->face[i].V(0)->P() + curMesh->face[i].V(1)->P() + curMesh->face[i].V(2)->P()) / 3.0f;

		if (bVisible && (baryCenter - limitOrigin) * eye.axisZVector <= 0)
		{
			faceList.push_back(i);
		}
		else if (!bVisible && (baryCenter - limitOrigin) * eye.axisZVector >= 0)
		{
			faceList.push_back(i);
		}
	}
	vertList.clear();
	for (int i = 0; i < curMesh->vert.size(); ++i)
	{
		if (bVisible && (curMesh->vert[i].P() - limitOrigin) * eye.axisZVector <= 0)
		{
			vertList.push_back(i);
		}
		else if (!bVisible && (curMesh->vert[i].P() - limitOrigin) * eye.axisZVector >= 0)
		{
			vertList.push_back(i);
		}
	}

	float fUnit = 100.0f;
	proj2DFaces.clear();
	for (auto i : faceList)
	{
		Point3m tempVert;
		Point3m projV0, projV1, projV2;
		int x, y;
		float distance;
		distance = (curMesh->face[i].V(0)->P() - eye.centerPoint) * eye.axisZVector;
		tempVert = getProjPointOnPlane(curMesh->face[i].V(0)->P(), -eye.axisZVector, eye.centerPoint, eye.axisZVector);
		x = (int)(((tempVert - eye.centerPoint) * eye.axisXVector) * fUnit);
		y = (int)(((tempVert - eye.centerPoint) * eye.axisYVector) * fUnit);
		projV0 = Point3m(x, y, distance);

		distance = (curMesh->face[i].V(1)->P() - eye.centerPoint) * eye.axisZVector;
		tempVert = getProjPointOnPlane(curMesh->face[i].V(1)->P(), -eye.axisZVector, eye.centerPoint, eye.axisZVector);
		x = (int)(((tempVert - eye.centerPoint) * eye.axisXVector) * fUnit);
		y = (int)(((tempVert - eye.centerPoint) * eye.axisYVector) * fUnit);
		projV1 = Point3m(x, y, distance);

		distance = (curMesh->face[i].V(2)->P() - eye.centerPoint) * eye.axisZVector;
		tempVert = getProjPointOnPlane(curMesh->face[i].V(2)->P(), -eye.axisZVector, eye.centerPoint, eye.axisZVector);
		x = (int)(((tempVert - eye.centerPoint) * eye.axisXVector) * fUnit);
		y = (int)(((tempVert - eye.centerPoint) * eye.axisYVector) * fUnit);
		projV2 = Point3m(x, y, distance);

		proj2DFaces.push_back(FFace(projV0, projV1, projV2));
	}

	int maxX = INT_MIN, minX = INT_MAX, maxY = INT_MIN, minY = INT_MAX;
	proj2DVerts.clear();
	for (auto i : vertList)
	{
		Point3m tempVert = getProjPointOnPlane(curMesh->vert[i].P(), -eye.axisZVector, eye.centerPoint, eye.axisZVector);
		int x = (int)(((tempVert - eye.centerPoint) * eye.axisXVector) * fUnit);
		int y = (int)(((tempVert - eye.centerPoint) * eye.axisYVector) * fUnit);
		proj2DVerts.push_back(Point2m(x, y));

		if (x > maxX)
		{
			maxX = x;
		}
		if (x < minX)
		{
			minX = x;
		}

		if (y > maxY)
		{
			maxY = y;
		}
		if (y < minY)
		{
			minY = y;
		}
	}

	maxProjVert = Point2i(maxX, maxY);
	minProjVert = Point2i(minX, minY);

	return;
}

void Tooth::collectTooth2DProjFaces(CMeshO *curMesh, Tooth *tooth, bool bVisible, Axis eye, Point3m limitOrigin, vector<int> &faceList, vector<int> &vertList, vector<FFace> &proj2DFaces, vector<Point2m> &proj2DVerts, Point2i &maxProjVert, Point2i &minProjVert)
{
	Point3m baryCenter, edgeVector;
	//collect useful faces Indexes
	faceList.clear();
	for (auto i : tooth->toothFaceIndex)
	{
		baryCenter = (curMesh->face[i].V(0)->P() + curMesh->face[i].V(1)->P() + curMesh->face[i].V(2)->P()) / 3.0f;

		if (bVisible && (baryCenter - limitOrigin) * eye.axisZVector <= 0)
		{
			faceList.push_back(i);
		}
		else if (!bVisible && (baryCenter - limitOrigin) * eye.axisZVector >= 0)
		{
			faceList.push_back(i);
		}
	}
	vertList.clear();
	for (auto i : tooth->toothIndex)
	{
		if (bVisible && (curMesh->vert[i].P() - limitOrigin) * eye.axisZVector <= 0)
		{
			vertList.push_back(i);
		}
		else if (!bVisible && (curMesh->vert[i].P() - limitOrigin) * eye.axisZVector >= 0)
		{
			vertList.push_back(i);
		}
	}

	float fUnit = 100.0f;
	proj2DFaces.clear();
	for (auto i : faceList)
	{
		Point3m tempVert;
		Point3m projV0, projV1, projV2;
		int x, y;
		float distance;
		distance = (curMesh->face[i].V(0)->P() - eye.centerPoint) * eye.axisZVector;
		tempVert = getProjPointOnPlane(curMesh->face[i].V(0)->P(), -eye.axisZVector, eye.centerPoint, eye.axisZVector);
		x = (int)(((tempVert - eye.centerPoint) * eye.axisXVector) * fUnit);
		y = (int)(((tempVert - eye.centerPoint) * eye.axisYVector) * fUnit);
		projV0 = Point3m(x, y, distance);

		distance = (curMesh->face[i].V(1)->P() - eye.centerPoint) * eye.axisZVector;
		tempVert = getProjPointOnPlane(curMesh->face[i].V(1)->P(), -eye.axisZVector, eye.centerPoint, eye.axisZVector);
		x = (int)(((tempVert - eye.centerPoint) * eye.axisXVector) * fUnit);
		y = (int)(((tempVert - eye.centerPoint) * eye.axisYVector) * fUnit);
		projV1 = Point3m(x, y, distance);

		distance = (curMesh->face[i].V(2)->P() - eye.centerPoint) * eye.axisZVector;
		tempVert = getProjPointOnPlane(curMesh->face[i].V(2)->P(), -eye.axisZVector, eye.centerPoint, eye.axisZVector);
		x = (int)(((tempVert - eye.centerPoint) * eye.axisXVector) * fUnit);
		y = (int)(((tempVert - eye.centerPoint) * eye.axisYVector) * fUnit);
		projV2 = Point3m(x, y, distance);

		proj2DFaces.push_back(FFace(projV0, projV1, projV2));
	}

	int maxX = INT_MIN, minX = INT_MAX, maxY = INT_MIN, minY = INT_MAX;
	proj2DVerts.clear();
	for (auto i : vertList)
	{
		Point3m tempVert = getProjPointOnPlane(curMesh->vert[i].P(), -eye.axisZVector, eye.centerPoint, eye.axisZVector);
		int x = (int)(((tempVert - eye.centerPoint) * eye.axisXVector) * fUnit);
		int y = (int)(((tempVert - eye.centerPoint) * eye.axisYVector) * fUnit);
		proj2DVerts.push_back(Point2m(x, y));

		if (x > maxX)
		{
			maxX = x;
		}
		if (x < minX)
		{
			minX = x;
		}

		if (y > maxY)
		{
			maxY = y;
		}
		if (y < minY)
		{
			minY = y;
		}
	}

	maxProjVert = Point2i(maxX, maxY);
	minProjVert = Point2i(minX, minY);

	return;
}

void Tooth::coloredMapWithDepth(float **map, vector<FFace> *faceList, int areaWidth, int areaHeight)
{
	/*if (areaWidth == 0 || areaHeight == 0 || faceList->empty())
	{
		return;
	}

	map = new float*[areaWidth];
	for (int i = 0; i < areaWidth; ++i)
	{
		map[i] = new float[areaHeight];
		for (int j = 0; j < areaHeight; ++j)
		{
			map[i][j] = -1;
		}
	}*/

	for (int i = 0; i < faceList->size(); ++i)
	{
		coloredMapWithSingleTriangle(map, &*faceList->begin() + i, areaWidth, areaHeight);
	}
	return;
}

void Tooth::coloredMapWithSingleTriangle(float **map, FFace *face, int areaWidth, int areaHeight)
{
	int maxX, maxY;
	int minX, minY;
	Point3m p0, p1, p2;
	p0 = face->v1;
	p1 = face->v2;
	p2 = face->v3;

	maxX = (p0.X()>p1.X()) ? p0.X() : p1.X();
	maxX = (p2.X()>maxX) ? p2.X() : maxX;

	maxY = (p0.Y()>p1.Y()) ? p0.Y() : p1.Y();
	maxY = (p2.Y()>maxY) ? p2.Y() : maxY;

	minX = (p0.X()<p1.X()) ? p0.X() : p1.X();
	minX = (p2.X()<minX) ? p2.X() : minX;

	minY = (p0.Y()<p1.Y()) ? p0.Y() : p1.Y();
	minY = (p2.Y()<minY) ? p2.Y() : minY;

	double k0, k1, k2;
	k0 = 1 / (double)((p0.Y() - p1.Y())*p2.X() + (p1.X() - p0.X())*p2.Y() + p0.X()*p1.Y() - p1.X()*p0.Y());
	k1 = 1 / (double)((p1.Y() - p2.Y())*p0.X() + (p2.X() - p1.X())*p0.Y() + p1.X()*p2.Y() - p2.X()*p1.Y());
	k2 = 1 / (double)((p2.Y() - p0.Y())*p1.X() + (p0.X() - p2.X())*p1.Y() + p2.X()*p0.Y() - p0.X()*p2.Y());

	int I, J;

	for (int ii = minX; ii <= maxX; ii++)
	{
		if (0 <= ii && ii < areaWidth)
		{
			for (int jj = minY; jj <= maxY; jj++)
			{
				if (0 <= jj && jj < areaHeight)
				{
					double a, b, c;
					bool canDo = getABC(ii, jj, p0, p1, p2, k0, k1, k2, a, b, c);
					if (canDo)
					{
						float depth = (a*p0.Z() + b*p1.Z() + c*p2.Z());
						map[ii][jj] = depth;
					}
				}
			}
		}
	}
	return;
}

bool Tooth::getABC(int i, int j, Point3m p0, Point3m p1, Point3m p2, double k0, double k1, double k2, double &a, double &b, double &c)
{
	a = ((p1.Y() - p2.Y())*i + (p2.X() - p1.X())*j + p1.X()*p2.Y() - p2.X()*p1.Y())*k1;
	b = ((p2.Y() - p0.Y())*i + (p0.X() - p2.X())*j + p2.X()*p0.Y() - p0.X()*p2.Y())*k2;
	c = ((p0.Y() - p1.Y())*i + (p1.X() - p0.X())*j + p0.X()*p1.Y() - p1.X()*p0.Y())*k0;

	if (a >= 0 && b >= 0 && c >= 0)
		return true;
	else
		return false;
}

void Tooth::getSpecialOrigins()
{
	this->lateralOrigin_Left = getLateralOrigin(this->localAxis.axisXVector, this->localAxis.centerPoint, this->convexVerts);
	this->source_LateralOrigin_Left = this->lateralOrigin_Left;
	this->lateralOrigin_Right = getLateralOrigin(-this->localAxis.axisXVector, this->localAxis.centerPoint, this->convexVerts);
	this->source_LateralOrigin_Right = this->lateralOrigin_Right;

	getVirtualRootOrigin();
}

Point3m Tooth::getLateralOrigin(Point3m direct, Point3m origin, const vector<Point3m>& verts)
{
	int aimIndex = -1;
	Point3m edgeV;
	float maxValue = 0, curValue;
	for (int i = 0; i < verts.size(); ++i)
	{
		edgeV = (verts.at(i) - origin).Normalize();
		curValue = edgeV * direct;
		if (curValue > maxValue)
		{
			maxValue = curValue;
			aimIndex = i;
		}
	}

	if (aimIndex >= 0)
	{
		return verts.at(aimIndex);
	}
	else
	{
		float halfWidth = this->localBoundbox.fWidth / 2.0f;
		return origin + direct * halfWidth;
	}
}

void Tooth::getVirtualRootOrigin()
{
	float fVirtualRootLength = 15.0f;
	float fExtendLength = fVirtualRootLength - this->localBoundbox.fHeight / 2.0f;
	Point3m rootOrigin = this->localAxis.centerPoint - this->localAxis.axisYVector * fExtendLength;
	this->localRootAxis.axisXVector = this->localAxis.axisXVector;
	this->localRootAxis.axisYVector = this->localAxis.axisYVector;
	this->localRootAxis.axisZVector = this->localAxis.axisZVector;
	this->localRootAxis.centerPoint = rootOrigin;
	this->source_RootOrigin = rootOrigin;

	this->rootOrigin = this->source_RootOrigin;
	this->realTimeRootAxis = this->localRootAxis;
}

void Tooth::adjArrageCoorSys()
{
	int iFDI = this->strFDI.toInt();
	int iDental, iTooth;
	iDental = iFDI / 10;
	iTooth = iFDI % 10;
	if (iDental != 1 && iDental != 2)
	{
		return;
	}

	float adjDegree_X(0.0), adjDegree_Z(0.0);

	switch (iTooth)
	{
	case 1:
	case 2:
		adjDegree_X = 10.0f;
		adjDegree_Z = 0;
		break;
	case 3:
	case 4:
		adjDegree_X = 5.0f;
		adjDegree_Z = 0;
		break;
	case 5:
	case 6:
	case 7:
	case 8:
		adjDegree_X = 0;
		adjDegree_Z = 0;
		break;
	}

	this->arrageCoorSys.rotate(adjDegree_X, this->arrageCoorSys.axisXVector);
	this->arrageCoorSys.rotate(adjDegree_Z, this->arrageCoorSys.axisZVector);
}

void Tooth::computeDefaultMarkerPointIndexes()
{
	//1.取牙齿下边界中点，截断外包围盒投影面。
    Point3m middlePoint;
    if(front_CtrlPTs.size() > 3)
    {
        middlePoint = this->front_CtrlPTs[2];
    }
	if (localBoundbox.upLoop.size() < 3)
		return;
	Point3m frontUpRightVert = this->localBoundbox.upLoop[2];
	Point3m frontUpLeftVert = this->localBoundbox.upLoop[3];
	Point3m upFrontMiddlePoint = (frontUpRightVert + frontUpLeftVert) / 2.0f;
	float projValue = abs((middlePoint - upFrontMiddlePoint) * this->localAxis.axisYVector);

	Point3m frontDownRightVert = frontUpRightVert - this->localAxis.axisYVector * projValue;
	Point3m frontDownLeftVert = frontUpLeftVert - this->localAxis.axisYVector * projValue;
	float halfWidth, halfHeight;
	halfWidth = this->localBoundbox.fLength / 2.0f;
	halfHeight = projValue / 2.0f;
	Point3m projPlaneCenter = (frontUpLeftVert + frontDownRightVert) / 2.0f;
	this->labialSideOrigin = projPlaneCenter;
	//2.将外包围盒投影面上的矩形进行适量等比缩放。得到理想投影点。
	float fWidthScale = 0.8f, fHeightScale = 0.9f;
	halfWidth *= fWidthScale;
	halfHeight *= fHeightScale;
	vector<Point3m> ideaProjVerts;

	int tooth_sign = this->strFDI.toInt() % 10;
	if (tooth_sign == 4 || tooth_sign == 5)
	{
		float special_reduction = halfHeight;
		ideaProjVerts.push_back(projPlaneCenter - this->localAxis.axisXVector * halfWidth + this->localAxis.axisYVector * (halfHeight - special_reduction));
		ideaProjVerts.push_back(projPlaneCenter + this->localAxis.axisYVector * (halfHeight - special_reduction));
		ideaProjVerts.push_back(projPlaneCenter + this->localAxis.axisXVector * halfWidth + this->localAxis.axisYVector * (halfHeight - special_reduction));
	}
	else
	{
		ideaProjVerts.push_back(projPlaneCenter - this->localAxis.axisXVector * halfWidth + this->localAxis.axisYVector * halfHeight);
		ideaProjVerts.push_back(projPlaneCenter + this->localAxis.axisYVector * halfHeight);
		ideaProjVerts.push_back(projPlaneCenter + this->localAxis.axisXVector * halfWidth + this->localAxis.axisYVector * halfHeight);
	}

	ideaProjVerts.push_back(projPlaneCenter - this->localAxis.axisXVector * halfWidth);
	ideaProjVerts.push_back(projPlaneCenter);
	ideaProjVerts.push_back(projPlaneCenter + this->localAxis.axisXVector * halfWidth);

	ideaProjVerts.push_back(projPlaneCenter - this->localAxis.axisXVector * halfWidth - this->localAxis.axisYVector * halfHeight);
	ideaProjVerts.push_back(projPlaneCenter - this->localAxis.axisYVector * halfHeight);
	ideaProjVerts.push_back(projPlaneCenter + this->localAxis.axisXVector * halfWidth - this->localAxis.axisYVector * halfHeight);
	//3.寻找理想投影点在牙齿网格上最近的点作为默认编辑点。记录其索引。
	vector<int>().swap(this->defaultMarkerPointIndexes);

	tri::RequireCompactness(*this->p_mesh_);
	VertexConstDataWrapper<CMeshO> ww(*this->p_mesh_);
	KdTree<float> kt(ww);
	typename KdTree<float>::PriorityQueue pq;
	int neighbor = 2;

	int idA, idB, index;
	float distanceA, distanceB;
	for (auto vPos : ideaProjVerts)
	{
		kt.doQueryK(vPos, neighbor, pq);

		idA = pq.getIndex(0);
		idB = pq.getIndex(1);
		distanceA = (this->p_mesh_->vert[idA].P() - vPos) * (this->p_mesh_->vert[idA].P() - vPos);
		distanceB = (this->p_mesh_->vert[idB].P() - vPos) * (this->p_mesh_->vert[idB].P() - vPos);
		index = distanceA < distanceB ? idA : idB;

		this->defaultMarkerPointIndexes.push_back(index);
	}
	return;
}

Point3m Tooth::selectNearestDefaultMarkerPoint(Point3m v)
{
	if (this->realTimeMesh == nullptr)
	{
		return v;
	}

	float minDistance = FLT_MAX;
	int i = 0, minIndex = -1;
	vector<int>::iterator iter = this->defaultMarkerPointIndexes.begin();
	for (; iter != this->defaultMarkerPointIndexes.end(); ++iter, ++i)
	{
		Point3m markerPoint = this->realTimeMesh->vert[*iter].P();
		Point3m edge = markerPoint - v;
		float distanceSquare = edge * edge;
		if (distanceSquare < minDistance)
		{
			minDistance = distanceSquare;
			minIndex = i;
		}
	}
	if (minIndex < 0)
	{
		return v;
	}
	return this->realTimeMesh->vert[this->defaultMarkerPointIndexes[minIndex]].P();
}

void Tooth::swelledOutToothOn(float _height, float _reduction)
{
	if (this->defaultMarkerPointIndexes.empty())
	{
		return;
	}
	if (this->swelledOutMesh_ != nullptr)
	{
		return;
	}

	Axis swellAxis(this->labialSideOrigin, this->localAxis.axisXVector, this->localAxis.axisYVector, this->localAxis.axisZVector);
	//1.在localAxis坐标轴下，对curMesh进行区域划分。得出位于唇侧的牙齿索引点列。
	float fMaxRadius = 2.0f;

	int tooth_sign = this->strFDI.toInt() % 10;
	if (tooth_sign == 4 || tooth_sign == 5)
	{
		fMaxRadius = 1.8f;
	}

	swelledOutMesh_ = new CMeshO;
	vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(*swelledOutMesh_, *p_mesh_);
	this->updateModel(swelledOutMesh_);
	vector<pair<int, Point2m>> pickedVerts = this->getSwelledAreaVerts(swelledOutMesh_, swellAxis, fMaxRadius, _reduction);
	//2.对pickedVertIndexes中的点索引进行高斯突出
	//this->swellOutVerts(pickedVerts, swelledOutMesh_, swellAxis, fMaxRadius);
	//this->swellOutVerts2(pickedVerts, swelledOutMesh_, swellAxis, _height);
	this->generateSwellOutAccessory(swellAxis, _height);
	return;
}

void Tooth::deleteSwelledOutTooth()
{
	if (swelledOutMesh_ == nullptr)
	{
		return;
	}
	delete swelledOutMesh_;
	swelledOutMesh_ = nullptr;
}

CMeshO *Tooth::getRealtimeMesh()
{
	CMeshO *temp_mesh = new CMeshO;

	if (swelledOutMesh_ != nullptr)
	{
		vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(*temp_mesh, *swelledOutMesh_);
	}
	else
	{
		vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(*temp_mesh, *p_mesh_);
	}

	for (int i = 0; i < temp_mesh->vert.size(); ++i)
	{
		temp_mesh->vert[i].P() = p_mesh_->Tr * temp_mesh->vert[i].P();
	}
	this->updateModel(temp_mesh);
	return temp_mesh;
}

vector<std::pair<int, Point2m>> Tooth::getSwelledAreaVerts(CMeshO *curMesh, Axis &swellAxis, float &maxRadius, float& _reduction)
{
	//1.计算区域半径
	float fMaxRadiusSquare = 0.0f;
	vector<int>::iterator ite = this->defaultMarkerPointIndexes.begin();
	for (; ite != this->defaultMarkerPointIndexes.end(); ++ite)
	{
		int index = *ite;
		Point3m vert = curMesh->vert[index].P();
		float tempRadius = (vert - swellAxis.centerPoint) * (vert - swellAxis.centerPoint);
		if (tempRadius > fMaxRadiusSquare)
		{
			fMaxRadiusSquare = tempRadius;
		}
	}
	maxRadius = sqrtf(fMaxRadiusSquare);

	//2.剔除牙根和舌侧点
	vector<int> tempArea;
	if (root_vertindex_interval_.X() >= 0 && root_vertindex_interval_.Y() > root_vertindex_interval_.X())
	{
		for (int i = 0; i < p_mesh_->vert.size(); ++i)
		{
			if (root_vertindex_interval_.X() <= i && i <= root_vertindex_interval_.Y())
			{
				continue;
			}
			tempArea.push_back(i);
		}
	}
	else
	{
		int limited = curMesh->vert.size() - this->boundaryIndex.size() * 40;
		for (int i = 0; i < limited; ++i)
		{
			tempArea.push_back(i);
		}
	}

	Point3m eliminateCenter = curMesh->vert[defaultMarkerPointIndexes[this->defaultMarkerPointIndexes.size() / 2]].P();
	eliminateCenter = eliminateCenter * 0.3f + this->localAxis.centerPoint * 0.7f;
	ite = tempArea.begin();
	while (ite != tempArea.end())
	{
		Point3m vert = curMesh->vert[*ite].P();
		if ((vert - eliminateCenter) * this->localAxis.axisZVector > 0)
		{
			++ite;
		}
		else
		{
			tempArea.erase(ite);
		}
	}

	//3.将可见的点保留
	Point3m tempCamPos = swellAxis.centerPoint + swellAxis.axisZVector * 3.0f;
	ite = tempArea.begin();
	while (ite != tempArea.end())
	{
		Point3m vertCamera = curMesh->vert[*ite].P() - tempCamPos;
		Point3m vertNormal = curMesh->vert[*ite].N();
		if (vertNormal * vertCamera < 0)
		{
			++ite;
		}
		else
		{
			tempArea.erase(ite);
		}
	}

	//4.将tempArea中投影距离处于半径范围之内的点的索引作为结果收集返回
	int tooth_sign = this->strFDI.toInt() % 10;
	if (tooth_sign == 4 || tooth_sign == 5)
	{
		swellAxis.centerPoint = swellAxis.centerPoint + swellAxis.axisYVector * 1.0f;
	}
	else
	{
		swellAxis.centerPoint = swellAxis.centerPoint + swellAxis.axisYVector * 1.5f;
	}

	vector<pair<int, Point2m>> result;
	ite = tempArea.begin();
	while (ite != tempArea.end())
	{
		Point3m edge = curMesh->vert[*ite].P() - swellAxis.centerPoint;
		float x = edge * swellAxis.axisXVector;
		float y = edge * swellAxis.axisYVector;
		if (sqrtf(x * x + y * y) < maxRadius)
		{
			result.push_back(pair<int, Point2m>(*ite, Point2m(x, y)));
		}
		++ite;
	}

	//5.拟合美白区域轮廓
	eliminateOuterCircleVerts(result, curMesh, swellAxis, _reduction);

	return result;
}

void Tooth::swellOutVerts(vector<pair<int, Point2m>> &pickedVertRecords, CMeshO *curMesh, Axis &swellAxis, float &maxRadius)
{
	int vertIndex;
	Point2m vertPos2D;
	float fA = 2.0f;
	//Point2m alpha2D(maxRadius * 0.316223f, maxRadius * 0.316223f), center2D(0, 0);
	Point2m alpha2D(maxRadius * 0.5f, maxRadius * 0.5f), center2D(0, 0);
	for (int i = 0; i < pickedVertRecords.size(); ++i)
	{
		vertIndex = pickedVertRecords[i].first;
		vertPos2D = pickedVertRecords[i].second;
		float adjValue = swellOutByGaussianFunc(vertPos2D, center2D, alpha2D, fA);
		curMesh->vert[vertIndex].P() += swellAxis.axisZVector * adjValue;
	}
	this->updateModel(curMesh);
}

void Tooth::swellOutVerts2(vector<pair<int, Point2m>>& pickedVertRecords, CMeshO* curMesh, Axis& swellAxis, float& _value)
{
	int vertIndex;
	for (int i = 0; i < pickedVertRecords.size(); ++i)
	{
		vertIndex = pickedVertRecords[i].first;
		curMesh->vert[vertIndex].P() += swellAxis.axisZVector * _value;
	}
	this->updateModel(curMesh);
}

void Tooth::generateSwellOutAccessory(Axis& _axis, float& _value)
{
	if (whitening_boundary_.empty())
	{
		return;
	}

	Point3m p, pa, pb, pc, pd;
	Point3m center_out(0,0,0), center_inside(0, 0, 0);
	vector<Point3m> ring_out, ring_inside;
	for (auto& vert : whitening_boundary_)
	{
		p = vert + _axis.axisZVector * _value;
		ring_out.push_back(p);

		p = vert + _axis.axisZVector * -1.0f;
		ring_inside.push_back(p);
	}
	center_out = _axis.centerPoint + _axis.axisZVector * _value;
	center_inside = _axis.centerPoint + _axis.axisZVector * -1.0f;

	this->whitening_accessory_.clear();
	for (int i = 0; i < ring_out.size(); ++i)
	{
		if (i == ring_out.size() - 1)
		{
			pa = ring_out[i];
			pb = ring_out[0];
		}
		else
		{
			pa = ring_out[i];
			pb = ring_out[i + 1];
		}

		whitening_accessory_.push_back(FFace(center_out, pa, pb));
	}

	for (int i = 0; i < ring_out.size(); ++i)
	{
		if (i == ring_out.size() - 1)
		{
			pa = ring_out[i];
			pb = ring_out[0];
			pc = ring_inside[i];
			pd = ring_inside[0];
		}
		else
		{
			pa = ring_out[i];
			pb = ring_out[i + 1];
			pc = ring_inside[i];
			pd = ring_inside[i + 1];
		}

		whitening_accessory_.push_back(FFace(pa, pc, pd));
		whitening_accessory_.push_back(FFace(pb, pa, pd));
	}

	for (int i = 0; i < ring_inside.size(); ++i)
	{
		if (i == ring_inside.size() - 1)
		{
			pa = ring_inside[i];
			pb = ring_inside[0];
		}
		else
		{
			pa = ring_inside[i];
			pb = ring_inside[i + 1];
		}

		whitening_accessory_.push_back(FFace(center_inside, pb, pa));
	}
}

float Tooth::swellOutByGaussianFunc(Point2m pos2D, Point2m center2D, Point2m alpha2D, float A)
{
	float value;
	float subVaule = (powf(pos2D.X() - center2D.X(), 2) / (2.0f * powf(alpha2D.X(), 2))) + (powf(pos2D.Y() - center2D.Y(), 2) / (2.0f * powf(alpha2D.Y(), 2)));
	value = A * expf(-subVaule);
	return value;
}

void Tooth::generateToothSourceMesh()
{
	if (this->p_mesh_ == nullptr)
	{
		return;
	}

	//根据标记网格signedMesh和sign对进行tempTooth网格面进行筛选
	for (int i = 0; i < p_mesh_->face.size(); i++)
	{
		if ((int)p_mesh_->face[i].V(0)->Q() != iLabel || (int)p_mesh_->face[i].V(1)->Q() != iLabel || (int)p_mesh_->face[i].V(2)->Q() != iLabel)
		{
			vcg::tri::Allocator<CMeshO>::DeleteFace(*p_mesh_, p_mesh_->face[i]);
		}
		else
		{
			this->toothFaceIndex.push_back(i);
		}
	}
	//根据标记网格pMesh和iLabel对进行tempTooth网格点进行筛选
	for (int i = 0; i < p_mesh_->vert.size(); i++)
	{
		if ((int)p_mesh_->vert[i].Q() != iLabel)
		{
			vcg::tri::Allocator<CMeshO>::DeleteVertex(*p_mesh_, p_mesh_->vert[i]);
		}
		else
		{
			this->toothIndex.push_back(i);
		}
	}
	this->updateModel(p_mesh_);
}

void Tooth::initializeCompareObject(CMeshO *_pmesh, MeshModel* _pmodel)
{
	for (int i = 0; i < _pmesh->vert.size(); ++i)
	{
		_pmesh->vert[i].Q() = QVALUE_COMPAREDOBJECT;
		_pmesh->vert[i].C() = CCOLOR_COMPARED_OGJECT;
	}
	this->compare_obj_ = new AbsMesh(_pmesh, _pmodel);
}

void Tooth::initializeCompareObject2(CMeshO* _pmesh, MeshModel* _pmodel)
{
	for (int i = 0; i < _pmesh->vert.size(); ++i)
	{
		_pmesh->vert[i].Q() = this->iLabel;
		_pmesh->vert[i].C() = CCOLOR_TOOTH;
	}
	this->compare_obj2_ = new AbsMesh(_pmesh, _pmodel);
}

void Tooth::setRootInterval(Point2i _interval)
{
	this->root_vertindex_interval_ = _interval;
}

void Tooth::setFixed(bool _state)
{
	this->bFixed_ = _state;
	if (_state)
	{
		for (int i = 0; i < this->p_mesh_->vert.size(); ++i)
		{
			p_mesh_->vert[i].C() = CCOLOR_TOOTH_FIXED;
		}
	}
	else
	{
		for (int i = 0; i < this->p_mesh_->vert.size(); ++i)
		{
			p_mesh_->vert[i].C() = CCOLOR_TOOTH;
		}
	}
}
