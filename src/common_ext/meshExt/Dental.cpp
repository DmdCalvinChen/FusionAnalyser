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

#include "Dental.h"
#include "common/meshmodel.h"
#include "common_ext/data/fusionaligndata.h"

Dental::Dental()
{
	bDentalIsReady = false;
	bBasalPlaneIsReady = false;
	bCureModelGenerated = false;
	for (int i = 0; i < STANDARD_TOOTH_SUM; i++)
	{
		bToothExist[i] = false;
	}
}

Dental::~Dental()
{

}

void Dental::loadDental(CMeshO* _sourceMesh, bool _bIsItUpperDental, QString _label_seg_mark, QString _full_path_filename_seg, SegmentedStatusInfo _savedInfo, MeshDocument* _md
	, Dental *_ref_dental)
{
	this->bUpperDental = _bIsItUpperDental;
	/// read toothIndex:统计牙颌上的分割标记，保存在toothIndex数组中
	for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (_savedInfo.bIsLossTooth[i] <= 0)
		{
			bToothExist[i] = true;
		}
		else
		{
			bToothExist[i] = false;
		}
	}

	/// Operate the itooth-th ToothModel
	int j = 0;
	for (int i = 0; i < STANDARD_TOOTH_SUM; i++)
	{
		if (bToothExist[i])
		{
			Axis toothCoorSys = _savedInfo.axisTooth[j];
			vector<Point3m> cur_width_def_verts;
			if (!_savedInfo.width_def_verts_list_.empty())
			{
				cur_width_def_verts = _savedInfo.width_def_verts_list_[j];
			}
			this->getATooth(teeth[i], i, _sourceMesh, tempmesh[i], toothCoorSys, _savedInfo.toothFDIInfo[j].labelFDI, cur_width_def_verts, _md);
			if (!_savedInfo.root_interval_list_.empty())
			{
				teeth[i].setRootInterval(_savedInfo.root_interval_list_[j]);
			}
			if (!_savedInfo.eachToothFeatureMarks.empty())
			{
				teeth[i].InitalFeatureMarks(_savedInfo.eachToothFeatureMarks[i]);
			}
			++j;
		}
	}
	updateTeethWidthDefVertsList();
	this->getTeethNum();
	// getLocalGumBoundaryIndexList removed - gingiva functionality not used in FusionAnalyser
	//初始化边界点索引数组，分析Tooth获取边界点
	this->bDentalIsReady = true;
	collectSortIndexList();

	if (_savedInfo.occlusalPlane != NULL)
	{
		this->basePlane = *_savedInfo.occlusalPlane;
		this->bBasalPlaneIsReady = true;
		initalDentalWidthAndDepth(_savedInfo.fPlaneWidth, _savedInfo.fPlaneDepth);
		//initalDentalWidthAndDepth(66.0f, 45.0f);
	}

	if (_ref_dental != nullptr)
	{
		this->basePlane = _ref_dental->basePlane;
		this->bBasalPlaneIsReady = true;
		initalDentalWidthAndDepth(66.0f, 45.0f);
	}

	//构建凸体包围盒
	constructDentalConvexHull(_savedInfo, _ref_dental);

	this->moveLinkRecords.clear();
	this->rotateLinkRecords.clear();
	this->synLinkCPRecords.clear();
	this->moveLinkRecords.resize(0);
	this->rotateLinkRecords.resize(0);
	this->synLinkCPRecords.resize(0);
	if (!_savedInfo.moveRecords.empty())
	{
		this->moveLinkRecords = _savedInfo.moveRecords;
	}
	if (!_savedInfo.rotateRecords.empty())
	{
		this->rotateLinkRecords = _savedInfo.rotateRecords;
	}
	if (!_savedInfo.synCPs.empty())
	{
		this->synLinkCPRecords = _savedInfo.synCPs;
	}
	//生成初始牙弓曲线
	constructDentalArchLine(&_savedInfo);

	int toothIndex;
	for (int i = 0; i < toothSortIndexList.size(); ++i)
	{
		toothIndex = toothSortIndexList[i];
		this->teeth[toothIndex].getSpecialOrigins();
		this->teeth[toothIndex].updateSpecialRotateCircle();
	}

	//获得目标牙弓曲线控制点集
	if (!_savedInfo.archCtrlNodes.empty())
	{
		this->archBezierCtrlNodes2D = _savedInfo.archCtrlNodes;
	}
	else
	{
		this->archBezierCtrlNodes2D.clear();
	}

	analysisExportableOriginalMesh(_md, _full_path_filename_seg);

	QString strRealRootsLabel;

	CMeshO* gumMesh = nullptr;
	MeshModel* new_model = _md->addNewMesh("", DENTAL_LABEL);
	//new_model->segmentedStatusInfo = _savedInfo;
	new_model->_labelSegmentMark = _label_seg_mark;
	new_model->_fullPathFileNameSegment = _full_path_filename_seg;
	if (bUpperDental)
	{
		new_model->upperOrLowerToothModelMark = UpperSingleDental;
	}
	else
	{
		new_model->upperOrLowerToothModelMark = LowerSingleDental;
	}
	if (gumMesh != nullptr)
	{
		vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(new_model->cm, *gumMesh);
		SAFE_DELETE(gumMesh);
	}
	else
	{
		UiUtilityTools::getInstance()->showInfoMessageBox(QObject::tr("error"), QObject::tr("There is an error in the tooth node, please resegment it"));
	}
	this->inital(&new_model->cm);
	this->p_model_ = new_model;

	MeshModel* new_compare_model = _md->addNewMesh("", COMPAREOBJ_LABEL, false);
	if (bUpperDental)
	{
		new_compare_model->upperOrLowerToothModelMark = UpperSingleComparedGum;
	}
	else
	{
		new_compare_model->upperOrLowerToothModelMark = LowerSingleComparedGum;
	}
	vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(new_compare_model->cm, *this->p_mesh_);
	this->initializeCompareObject(&new_compare_model->cm, new_compare_model);

	if (this->bBasalPlaneIsReady)
	{
		this->localAxis = Axis(basePlane.center, basePlane.axisXV, basePlane.axisYV, basePlane.axisZV);
		if (_savedInfo.dentalInitialCSys != nullptr)
		{
			this->setRealTimeAxis(*_savedInfo.dentalInitialCSys);
		}
		else
		{
			this->setRealTimeAxis(this->localAxis);
		}
	}

	for (int i = 1; i < STANDARD_TOOTH_SUM; i++)
	{
		if (bToothExist[i])
		{
			this->addChild(&teeth[i]);
			if (teeth[i].compare_obj_ != nullptr)
			{
				teeth[i].compare_obj_->p_father_trans_matrix_ = &this->p_model_->cm.Tr;
			}
		}
	}
	this->setRenderObject(VIRTUAL_JAW_ARRANGE);
	setCompareModelVisible(false);

	if (this->basalLabel_ == nullptr)
	{
		MeshModel* new_model = _md->addNewMesh("", BASALLABEL_LABEL);
		if (this->bUpperDental)
		{
			new_model->upperOrLowerToothModelMark = UpperSingleBasalLabel;
		}
		else
		{
			new_model->upperOrLowerToothModelMark = LowerSingleBasalLabel;
		}
		this->basalLabel_ = new AbsMesh(new_model);
	}

	if (this->export_result_ == nullptr)
	{
		MeshModel* new_model = _md->addNewMesh("", EXPORTABLEMESH_LABEL);
		if (this->bUpperDental)
		{
			new_model->upperOrLowerToothModelMark = UpperSingleExportableMesh;
		}
		else
		{
			new_model->upperOrLowerToothModelMark = LowerSingleExportableMesh;
		}
		this->export_result_ = new AbsMesh(new_model);
		this->setExportableMeshReady(false);
	}
}

void Dental::initializeCompareObject(CMeshO* _pmesh, MeshModel* _pmodel)
{

}

void Dental::restoreFixtureCSYS()
{
	Point3m aimAxisX = -basePlane.axisXV.Normalize();
	Point3m aimAxisY = basePlane.axisYV.Normalize();
	Point3m aimAxisZ = -basePlane.axisZV.Normalize();

	//float forward_Adjvalue = this->fDepth - fFixtureLimitRadius;
	float forward_Adjvalue = 0;
	Point3m aimCenter = basePlane.center - aimAxisZ * fHeightOfGumBase + aimAxisY * forward_Adjvalue + aimAxisX * 0; // was: HORIZONTAL_ADJUST_VALUE
	fixtureCSYS = Axis(aimCenter, aimAxisX, aimAxisY, aimAxisZ);
	return;
}

void Dental::sortArray(int array[], int length)
{
	int i, j, temp;
	for (i = length - 1; 0 < i; i--) {
		for (j = 0; j < i; j++) {
			if (array[j] > array[j + 1]) {
				temp = array[j];
				array[j] = array[j + 1];
				array[j + 1] = temp;
			}
		}
	}
}

void Dental::getAdjoingFaceAndPoints()
{
	// update topology
	assert(tri::HasFFAdjacency(*p_mesh_) == false);
	p_mesh_->face.EnableFFAdjacency();
	assert(tri::HasFFAdjacency(*p_mesh_) == true);

	assert(tri::HasVFAdjacency(*p_mesh_) == false);
	p_mesh_->vert.EnableVFAdjacency();
	p_mesh_->face.EnableVFAdjacency();
	assert(tri::HasVFAdjacency(*p_mesh_) == true);
	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*p_mesh_);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(*p_mesh_);

	for (int i = 0; i < p_mesh_->VN(); i++)
	{
		oneRingNeighborhoodVF(i);
	}
}

void Dental::oneRingNeighborhoodVF(int index)
{
	vcg::face::VFIterator<CFaceO> vfi(&p_mesh_->vert[index]);
	for (; !vfi.End(); ++vfi)
	{
		int indexF = vfi.F() - &p_mesh_->face[0];

		if (verList[index].iAdjFNum < MaxAdjFNum)
		{
			this->verList[index].adj_triangle[verList[index].iAdjFNum] = indexF;
		}
		verList[index].iAdjFNum++;

		int tempVI[3];
		tempVI[0] = p_mesh_->face[indexF].V(0) - &p_mesh_->vert[0];
		tempVI[1] = p_mesh_->face[indexF].V(1) - &p_mesh_->vert[0];
		tempVI[2] = p_mesh_->face[indexF].V(2) - &p_mesh_->vert[0];

		//if the i-th tempVI is new vertex index Of the verList[i](meshPoint3f)
		for (int i = 0; i < 3; i++)
		{
			if (tempVI[i] != index)
			{
				bool bIsNewAdjVertex = true;
				int  iCurrentAdjVNum = verList[index].iAdjPNum;
				if (iCurrentAdjVNum >= 20)
					break;
				for (int j = 0; j < iCurrentAdjVNum; j++)
				{
					if (tempVI[i] == verList[index].adj_vertex[j])
					{
						bIsNewAdjVertex = false;
						break;
					}
				}

				if (bIsNewAdjVertex)
				{
					verList[index].adj_vertex[iCurrentAdjVNum] = tempVI[i];
					verList[index].iAdjPNum++;
				}
			}
		}
	}
}

vector<int> Dental::getVertsOneNeighbourhoodVertIndexes(int index)
{
	vector<int> result;

	int tempVI[3];
	vcg::face::VFIterator<CFaceO> vfi(&p_mesh_->vert[index]);
	for (; !vfi.End(); ++vfi)
	{
		int indexF = vfi.F() - &p_mesh_->face[0];

		tempVI[0] = p_mesh_->face[indexF].V(0) - &p_mesh_->vert[0];
		tempVI[1] = p_mesh_->face[indexF].V(1) - &p_mesh_->vert[0];
		tempVI[2] = p_mesh_->face[indexF].V(2) - &p_mesh_->vert[0];

		for (int i = 0; i < 3; i++)
		{
			if (tempVI[i] != index)
			{
				if (this->exitInVector(tempVI[i], result))
				{
					result.push_back(tempVI[i]);
				}
			}
		}
	}

	return result;
}

void Dental::getATooth(Tooth& Teeth, int sign, CMeshO* signedMesh, CMeshO& tempmesh, Axis toothSys, QString strFDI, vector<Point3m> _width_def_verts_list, MeshDocument* _md)
{
	MeshModel* new_model = _md->addNewMesh("", TOOTHRD_LABEL);
	if (bUpperDental)
	{
		new_model->upperOrLowerToothModelMark = UpperSingleTooth;
	}
	else
	{
		new_model->upperOrLowerToothModelMark = LowerSingleTooth;
	}
	vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(new_model->cm, *signedMesh);

	Teeth.InitalTooth(&new_model->cm, sign, this->bUpperDental, toothSys, strFDI);
	if (!_width_def_verts_list.empty())
	{
		Teeth.setWidthDefinitionPoint(_width_def_verts_list[0], _width_def_verts_list[1], _width_def_verts_list[2]);
	}
	Teeth.p_model_ = new_model;

	MeshModel* new_compare_model = _md->addNewMesh("", TOOTHRD_LABEL, false);
	if (bUpperDental)
	{
		new_compare_model->upperOrLowerToothModelMark = UpperSingleComparedTooth;
	}
	else
	{
		new_compare_model->upperOrLowerToothModelMark = LowerSingleComparedTooth;
	}
	vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(new_compare_model->cm, *Teeth.p_mesh_);
	Teeth.initializeCompareObject(&new_compare_model->cm, new_compare_model);

	MeshModel* new_compare_model2 = _md->addNewMesh("", COMPAREOBJ_LABEL, false);
	if (bUpperDental)
	{
		new_compare_model2->upperOrLowerToothModelMark = UpperSingleComparedTooth;
	}
	else
	{
		new_compare_model2->upperOrLowerToothModelMark = LowerSingleComparedTooth;
	}
	vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(new_compare_model2->cm, *Teeth.p_mesh_);
	Teeth.initializeCompareObject2(&new_compare_model2->cm, new_compare_model2);
}

void Dental::getBoundaryOfTooth(int iToothIndex, int*& boundary, int& cursorI, CMeshO* signedMesh)
{
	int a = teeth[iToothIndex].p_mesh_->VN();
	int b = teeth[iToothIndex].p_mesh_->FN();

	for (int i = 0; i < p_mesh_->vert.size(); i++)
	{
		if (!teeth[iToothIndex].p_mesh_->vert[i].IsD())
		{
			int ii = &p_mesh_->vert[i] - &p_mesh_->vert[0];//对应在原模型中的第几个点

			bool bIsABorderPoint = false;
			for (int j = 0; j < MaxAdjPNum; j++)
			{
				if (verList[ii].adj_vertex[j] != -1)
				{
					int tempPI; // temp Point Index
					int  NPT; // Neighboor Point Type
					tempPI = verList[ii].adj_vertex[j];//一个领接点的索引
					NPT = (int)signedMesh->vert[tempPI].Q();

					if (NPT != iToothIndex && NPT == 0)
					{
						bIsABorderPoint = true;
						break;
					}
				}
			}

			if (bIsABorderPoint)
			{
				boundary[cursorI] = ii;
				this->teeth[iToothIndex].boundaryIndex.push_back(ii);
				cursorI++;
			}
		}
	}
}

void Dental::getBoundaryOfTooth(Tooth& tooth)
{
	for (auto index : tooth.toothIndex)
	{
		int a, b;
		a = verList[index].iAdjFNum;
		b = verList[index].iAdjPNum;
		if (a != b)
		{
			tooth.boundaryIndex.push_back(index);
		}
	}
}

void Dental::layeredModel(int* Boundary, int boundSize)
{
	int layer;
	int* Bound;
	int  Size;
	int* lastBound;
	int* newBound;
	int  lastSize;
	int  newSize;
	int  sum;

	layer = 0;
	Bound = Boundary;
	Size = boundSize;
	sum = boundSize;
	for (int i = 0; i < Size; i++)
		this->verList[Bound[i]].iLayer = layer;

	layer = 1;
	Bound = Boundary;
	Size = boundSize;
	sum = boundSize;

	do {
		if (Bound[0] < p_mesh_->VN() && Bound[0] >= 0)
		{
			int a = Bound[0];
			int max = this->verList[verList[a].iFactIndex].iAdjPNum;
			for (int i = 1; i < Size; i++)
			{
				if (this->verList[verList[Bound[i]].iFactIndex].iAdjPNum > max)
					max = this->verList[verList[Bound[i]].iFactIndex].iAdjPNum;
			}
			newBound = new int[max * Size];

			int counter = 0;
			for (int i = 0; i < Size; i++)
			{
				for (int j = 0; j < MaxAdjPNum; j++)
				{
					int currentV = this->verList[verList[Bound[i]].iFactIndex].adj_vertex[j];

					if (currentV != -1 && this->verList[currentV].iLayer == -1 && (int)p_mesh_->vert[currentV].Q() == 0)
					{
						verList[currentV].iLayer = layer;
						newBound[counter] = currentV;
						counter++;
					}
				}
			}

			layer++;
			Size = counter;
			Bound = newBound;
			sum += counter;
			this->LayerNumber = layer + 1;
		}
		else
		{
			this->LayerNumber = layer + 1; // 0, 1, 2 ... layer
			break;
		}

	} while (this->LayerNumber < LAYERNUM);
}

void Dental::getBaseEdjeLength()
{
	for (int i = 0; i < p_mesh_->VN(); i++)
	{
		if ((int)p_mesh_->vert[i].Q() == 0 && this->verList[i].iLayer != -1)
		{
			for (int j = 0; j < MaxAdjPNum; j++)
			{
				if (this->verList[i].adj_vertex[j] != -1 && this->verList[i].adj_vertex[j] < p_mesh_->VN())
				{
					float tempLength;
					CustomVector3D tempVec;
					tempVec.initVector(this->verList[i], this->verList[verList[i].adj_vertex[j]]);
					tempLength = tempVec.getLength();
					this->verList[i].adj_EdjeLength[j] = tempLength;
				}
			}
		}
	}
}

//牙龈网格变形起始函数
void Dental::gumDeformation(int iToothLabel, Dental& dental, CMeshO& signedMesh)
{
#ifndef USING_MASSPOINT_STRUCT
	bool bLayer1IsBalance = false;
	CustomVector3D moveVec;

	//将所有质点的平衡状态改为不平衡false
	for (int i = 0; i < p_mesh_->VN(); i++)
		if ((int)signedMesh.vert[i].Q() == 0) //牙龈网格点的标号是0，所以只将牙龈的平衡打破。牙齿网格点始终的平衡的状态
			this->verList[i].bIsBalanced = false;
		else
			this->verList[i].bIsBalanced = true;

	int tempNodeNum;
	int cLayer;// current Operate Layer

	cLayer = 0;
	tempNodeNum = this->tNN[cLayer];
	for (int i = 0; i < tempNodeNum; i++) // 首先对边界层，即第0层的各点直接移动到目标位置，之后边界点均达到平衡状态，位置不再改变
	{
		this->verList[this->LNM[cLayer][i]].bIsBalanced = true;
	}

	//循环反馈开始，从第一层开始直到第一层的所有质点都达到平衡状态。则认为所有点均达到了平衡状态。
	cLayer = 1;
	tempNodeNum = this->tNN[cLayer];
	int nodeNumOfL1;
	nodeNumOfL1 = this->tNN[1];
	do {
		//首先对该层的各个质点进行新一轮的位移计算，该计算只对bIsBalanced为false的质点进行。
		//计算结果是满足当前状态下的平衡移动向量moveVec。
		this->disturbLayer(cLayer);
		//当当前操作层不是1的时候，每一层的质点在平衡移动后将会打破之前所有处于暂时平衡状态的质点的平衡
		//此时需要从当前层开始向之前 的每一层进行扰动反馈
		this->destabilizationFeedback(cLayer);

		bool allBalance = true;
		for (int ii = 0; ii < nodeNumOfL1; ii++)
		{
			allBalance &= this->verList[this->LNM[1][ii]].bIsBalanced;
		}
		if (allBalance)
			bLayer1IsBalance = true;
		else
		{
			cLayer++;
			tempNodeNum = this->tNN[cLayer];
		}

	} while (!bLayer1IsBalance && cLayer < this->LayerNumber);

	if (bLayer1IsBalance)
	{
		for (int i = 0; i < p_mesh_->VN(); i++)
		{
			if ((int)signedMesh.vert[i].Q() == 0)
			{
				this->verList[i].bIsBalanced = true;
			}
		}

		this->getBaseEdjeLength();
	}
#else
	bool bLayer1IsBalance = false;
	Point3f moveVec;

	//将所有质点的平衡状态改为不平衡false
	for (int i = 0; i < this->gumMassPointes.size(); ++i)
	{
		this->gumMassPointes[i].setBalanceState(false);
	}

	int tempNodeNum;
	int cLayer;// current Operate Layer

	cLayer = 0;
	tempNodeNum = this->teeth[iToothLabel].massnode_LIM[cLayer].size();
	for (int i = 0; i < tempNodeNum; i++) // 首先对边界层，即第0层的各点直接移动到目标位置，之后边界点均达到平衡状态，位置不再改变
	{
		this->gumMassPointes[this->teeth[iToothLabel].massnode_LIM[cLayer][i]].bIsBalanced = true;
	}

	//循环反馈开始，从第一层开始直到第一层的所有质点都达到平衡状态。则认为所有点均达到了平衡状态。
	cLayer = 1;
	tempNodeNum = this->teeth[iToothLabel].massnode_LIM[cLayer].size();
	int nodeNumOfL1;
	nodeNumOfL1 = this->teeth[iToothLabel].massnode_LIM[1].size();//......
	do {
		//首先对该层的各个质点进行新一轮的位移计算，该计算只对bIsBalanced为false的质点进行。
		//计算结果是满足当前状态下的平衡移动向量moveVec。
		this->disturbLayer(cLayer);
		//当当前操作层不是1的时候，每一层的质点在平衡移动后将会打破之前所有处于暂时平衡状态的质点的平衡
		//此时需要从当前层开始向之前 的每一层进行扰动反馈
		this->destabilizationFeedback(cLayer);

		bool allBalance = true;
		for (int ii = 0; ii < nodeNumOfL1; ii++)
		{
			allBalance &= this->verList[this->LNM[1][ii]].bIsBalanced;
		}
		if (allBalance)
			bLayer1IsBalance = true;
		else
		{
			cLayer++;
			tempNodeNum = this->tNN[cLayer];
		}

	} while (!bLayer1IsBalance && cLayer < this->LayerNumber);

	if (bLayer1IsBalance)
	{
		for (int i = 0; i < p_mesh_->VN(); i++)
		{
			if ((int)signedMesh.vert[i].Q() == 0)
			{
				this->verList[i].bIsBalanced = true;
			}
		}

		this->getBaseEdjeLength();
	}
#endif // !USING_MASSPOINT_STRUCT
}

void Dental::getLayeredMatrix()
{
	tNN = new int[this->LayerNumber];
	this->LNM = new int* [this->LayerNumber];
	for (int i = 0; i < this->LayerNumber; i++) // Get The Node Number Of each Layer
	{
		int counter;
		counter = 0;
		for (int j = 0; j < p_mesh_->VN(); j++)
		{
			if (this->verList[j].iLayer == i)
				counter++;
		}
		tNN[i] = counter;
		this->LNM[i] = new int[counter];
	}

	int k = 0;
	for (int i = 0; i < this->LayerNumber; i++) // Get The Node Number Of each Layer
	{
		for (int j = 0; j < p_mesh_->VN(); j++)
		{
			if (this->verList[j].iLayer == i)
			{
				this->LNM[i][k] = j;
				k++;
			}

		}
		k = 0;
	}
}

void Dental::disturbLayer(int layer)
{
	int nodeNum;
	CustomVector3D moveVec;

	nodeNum = this->tNN[layer];
	for (int i = 0; i < nodeNum; i++)
	{
		if (!this->verList[this->LNM[layer][i]].bIsBalanced)
		{
			moveVec = this->getMoveVector(this->LNM[layer][i]);
			if (moveVec.getLength() <= 0.0f)
				this->verList[this->LNM[layer][i]].bIsBalanced = true;
			else
			{
				this->verList[this->LNM[layer][i]].movePoint3f(moveVec.x, moveVec.y, moveVec.z);

				p_mesh_->vert[LNM[layer][i]].P().X() += moveVec.x;
				p_mesh_->vert[LNM[layer][i]].P().Y() += moveVec.y;
				p_mesh_->vert[LNM[layer][i]].P().Z() += moveVec.z;
			}
		}
	}
}

CustomVector3D Dental::getMoveVector(int p)
{
	CustomVector3D resultVec;
	resultVec.initVector(0, 0, 0);

	int adjVNum = 0; // adjaction Vertex Number
	for (int i = 0; i < 20; i++)
	{
		if (this->verList[p].adj_vertex[i] != -1 && this->verList[p].adj_vertex[i] != p)
			adjVNum++;
	}

	int* adjVIndex = NULL;
	int* adjVSign = NULL;
	adjVIndex = new int[adjVNum];
	adjVSign = new int[adjVNum];
	int k = 0;
	for (int i = 0; i < 20; i++)
	{
		if (this->verList[p].adj_vertex[i] != -1 && this->verList[p].adj_vertex[i] != p)
		{
			adjVIndex[k] = this->verList[p].adj_vertex[i];
			adjVSign[k] = i;
			k++;
		}
	}

	double tempLength;
	CustomVector3D tempVec;
	MeshVertex tempS, tempE; // temp StartPoint and EndPoint
	tempS = this->verList[p];
	for (int i = 0; i < adjVNum; i++)
	{
		double fZoom;
		tempE = this->verList[adjVIndex[i]];

		tempVec.initVector(tempS, tempE);
		tempLength = tempVec.getLength();

		if (tempLength != 0)
			fZoom = Kw * (this->verList[p].adj_EdjeLength[adjVSign[i]] - tempLength) / tempLength;
		else
			fZoom = 0.0;

		tempVec.x *= fZoom;
		tempVec.y *= fZoom;
		tempVec.z *= fZoom;

		resultVec = resultVec + tempVec;
	}

	resultVec.negate();
	resultVec.x /= kp;
	resultVec.y /= kp;
	resultVec.z /= kp;
	delete[] adjVIndex;
	delete[] adjVSign;
	adjVIndex = NULL;
	adjVSign = NULL;

	return resultVec;
}

void Dental::destabilizationFeedback(int layer)
{
	int clayer;
	int tempNodeNum;

	clayer = layer - 1;
	while (clayer >= 1)
	{
		this->disturbLayer(clayer);
		clayer--;
	}
}

void Dental::layerGumByTooth(int iToothIndex)
{
	int* currentBoundary;
	int currentBoundaryLength;
	currentBoundaryLength = this->teeth[iToothIndex].boundaryIndex.size();
	currentBoundary = new int[this->teeth[iToothIndex].boundaryIndex.size()];
	for (int i = 0; i < this->teeth[iToothIndex].boundaryIndex.size(); i++)
		currentBoundary[i] = this->teeth[iToothIndex].boundaryIndex[i];

	//还原初始化
	this->bDentalIsReady = false;
	for (int i = 0; i < this->teeth[0].toothIndex.size(); i++)
		this->verList[this->teeth[0].toothIndex[i]].iLayer = -1;
	delete[]tNN;
	tNN = NULL;
	delete[]LNM;
	LNM = NULL;

	layeredModel(currentBoundary, currentBoundaryLength);
	this->getBaseEdjeLength();
	//获得层次质点矩阵LNM，和各层质点数统计数组tNN
	this->getLayeredMatrix();
	this->bDentalIsReady = true;
	delete[]currentBoundary;
}

void Dental::deleteChewinggumFaces()
{
	if (chewinggumFaces.empty())
	{
		return;
	}

	for (int i = 0; i < chewinggumFaces.size(); i++)
	{
		vcg::tri::Allocator<CMeshO>::DeleteFace(*this->p_mesh_, p_mesh_->face[chewinggumFaces[i]]);
	}
	vcg::tri::Allocator<CMeshO>::CompactFaceVector(*p_mesh_);
	vcg::tri::Allocator<CMeshO>::CompactVertexVector(*p_mesh_);

	vector<int> chewinggumVerts;
	for (int i = 0; i < p_mesh_->vert.size(); ++i)
	{
		vcg::face::VFIterator<CFaceO> vfi(&p_mesh_->vert[i]); //initialize the iterator tohe first face
		if (vfi.End())
		{
			chewinggumVerts.push_back(i);
		}
	}
	for (auto index : chewinggumVerts)
	{
		vcg::tri::Allocator<CMeshO>::DeleteVertex(*this->p_mesh_, p_mesh_->vert[index]);
	}
	chewinggumVerts.clear();
	vcg::tri::Allocator<CMeshO>::CompactFaceVector(*p_mesh_);
	vcg::tri::Allocator<CMeshO>::CompactVertexVector(*p_mesh_);

	int delVertNum = vcg::tri::Clean<CMeshO>::RemoveDegenerateVertex(*p_mesh_);
	int delFaceNum = vcg::tri::Clean<CMeshO>::RemoveDegenerateFace(*p_mesh_);
	tri::Clean<CMeshO>::RemoveDuplicateVertex(*p_mesh_);// delete Redundant vertex data
	vcg::tri::Allocator<CMeshO>::CompactFaceVector(*p_mesh_);
	vcg::tri::Allocator<CMeshO>::CompactVertexVector(*p_mesh_);
	vcg::tri::UpdateBounding<CMeshO>::Box(*p_mesh_);

	// update topology
	assert(tri::HasPerVertexVFAdjacency(*p_mesh_) && tri::HasPerFaceVFAdjacency(*p_mesh_));
	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*p_mesh_);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(*p_mesh_);

	// update Normals
	vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(*p_mesh_);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*p_mesh_);

	this->chewinggumFaces.clear();
}

void Dental::deleteChewinggumFacesOnFrozenMesh()
{
	for (int i = 0; i < chewinggumFaces.size(); i++)
	{
		vcg::tri::Allocator<CMeshO>::DeleteFace(this->frozenMesh, frozenMesh.face[chewinggumFaces[i]]);
	}
	vcg::tri::Allocator<CMeshO>::CompactFaceVector(this->frozenMesh);
	vcg::tri::Allocator<CMeshO>::CompactVertexVector(this->frozenMesh);

	vector<int> chewinggumVerts;
	for (int i = 0; i < this->frozenMesh.vert.size(); ++i)
	{
		vcg::face::VFIterator<CFaceO> vfi(&this->frozenMesh.vert[i]); //initialize the iterator tohe first face
		if (vfi.End())
		{
			chewinggumVerts.push_back(i);
		}
	}
	for (auto index : chewinggumVerts)
	{
		vcg::tri::Allocator<CMeshO>::DeleteVertex(this->frozenMesh, this->frozenMesh.vert[index]);
	}
	chewinggumVerts.clear();
	vcg::tri::Allocator<CMeshO>::CompactFaceVector(this->frozenMesh);
	vcg::tri::Allocator<CMeshO>::CompactVertexVector(this->frozenMesh);

	int delVertNum = vcg::tri::Clean<CMeshO>::RemoveDegenerateVertex(this->frozenMesh);
	int delFaceNum = vcg::tri::Clean<CMeshO>::RemoveDegenerateFace(this->frozenMesh);
	tri::Clean<CMeshO>::RemoveDuplicateVertex(this->frozenMesh);// delete Redundant vertex data
	vcg::tri::Allocator<CMeshO>::CompactFaceVector(this->frozenMesh);
	vcg::tri::Allocator<CMeshO>::CompactVertexVector(this->frozenMesh);
	vcg::tri::UpdateBounding<CMeshO>::Box(this->frozenMesh);

	// update topology
	assert(tri::HasPerVertexVFAdjacency(this->frozenMesh) && tri::HasPerFaceVFAdjacency(this->frozenMesh));
	vcg::tri::UpdateTopology<CMeshO>::VertexFace(this->frozenMesh);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(this->frozenMesh);

	// update Normals
	vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(this->frozenMesh);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(this->frozenMesh);
}

vcg::Matrix44f Dental::computeTransformMatrix(int iSign, HexaVec transHexaVec)
{
	vcg::Matrix44f transMat;
	vcg::Matrix44f transform, transformInverse;
	transform.SetIdentity();
	if (iSign <= 0)
		return transMat;

	transform.SetTranslate(this->teeth[iSign].localAxis.centerPoint);
	transformInverse.SetTranslate(-this->teeth[iSign].localAxis.centerPoint);

	vcg::Matrix44f transformValue, rotMatGlobal, rotMatLocal;
	transMat.SetIdentity();
	rotMatGlobal.SetIdentity();
	rotMatLocal.SetIdentity();
	Point3f trans;

	trans = this->teeth[iSign].localAxis.axisXVector * transHexaVec.fVX;
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	trans = this->teeth[iSign].localAxis.axisYVector * transHexaVec.fVY;
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	trans = this->teeth[iSign].localAxis.axisZVector * transHexaVec.fVZ;
	transformValue.SetTranslate(trans);
	transMat *= transformValue;

	transformValue.SetRotateDeg(transHexaVec.fA, this->teeth[iSign].localAxis.axisXVector);
	rotMatLocal *= transformValue;
	transformValue.SetRotateDeg(transHexaVec.fB, this->teeth[iSign].localAxis.axisYVector);
	rotMatLocal *= transformValue;
	transformValue.SetRotateDeg(transHexaVec.fG, this->teeth[iSign].localAxis.axisZVector);
	rotMatLocal *= transformValue;

	transform *= transMat * rotMatLocal;
	transform *= transformInverse;

	return transform;
}

void Dental::collectGumMassPoints()
{
	vector<MassPoint>().swap(gumMassPointes);
	vector<int>().swap(gumVertIndexes);

	for (int i = 0; i < p_mesh_->VN(); ++i)
	{
		if (p_mesh_->vert[i].Q() == 0)
		{
			this->gumMassPointes.push_back(MassPoint(i));
			gumVertIndexes.push_back(i);
		}
	}

	for (int i = 0; i < gumVertIndexes.size(); ++i)
	{
		this->gumMassPointes[i].getAdjacentMeshVertIndexes(getVertsOneNeighbourhoodVertIndexes(gumVertIndexes[i]));
	}
}

// CureInfom-related functions removed - treatment functionality not used in FusionAnalyser

void Dental::initalBaseLabelPosition(Point3m pos, Point3m axisX, Point3m axisY, Point3m axisZ)
{
	this->labelPos = pos;
	this->labelAxisX = axisX;
	this->labelAxisY = axisY;
	this->labelAxisZ = axisZ;
}

void Dental::initalDentalWidthAndDepth(float width, float depth)
{
	if(fabs(width) > 0.001f)
		this->fWidth = width;
	if(fabs(depth) > 0.001f)
		this->fDepth = depth;
}

void Dental::initalBaseSelectedRing()
{
	if (!this->bBasalPlaneIsReady)
	{
		return;
	}
	vector<Point3m> cirOut, cirInner;

	float fDepth = 15.0f;
	Point3m baseCenter = this->basePlane.center + this->basePlane.axisZV * fDepth;
	Point3m baseAxisX = this->basePlane.axisXV;
	Point3m baseAxisY = this->basePlane.axisYV;

	vcg::tri::UpdateBounding<CMeshO>::Box(*this->p_mesh_);

	int edgeNum = 60;
	float stepRads = (2.0f * PI) / (float)edgeNum;

	Point3m p;
	float costhita = 0.0f;
	float sinthita = 0.0f;
	float circleRadius = this->p_mesh_->bbox.DimX() * 1.0f;
	for (int i = 0; i < edgeNum; ++i)
	{
		costhita = cos(stepRads * i);
		sinthita = sin(stepRads * i);
		p = baseCenter + (baseAxisX * costhita + baseAxisY * sinthita) * circleRadius;
		cirOut.push_back(p);
	}
	costhita = 0.0f;
	sinthita = 0.0f;
	circleRadius = this->p_mesh_->bbox.DimX() * 0.8f;
	for (int i = 0; i < edgeNum; ++i)
	{
		costhita = cos(stepRads * i);
		sinthita = sin(stepRads * i);
		p = baseCenter + (baseAxisX * costhita + baseAxisY * sinthita) * circleRadius;
		cirInner.push_back(p);
	}

	this->baseRing.clear();
	for (int i = 0; i < cirOut.size(); ++i)
	{
		if (i == cirOut.size() - 1)
		{
			this->baseRing.push_back(FFace(cirOut[i], cirOut[0], cirInner[i]));
			this->baseRing.push_back(FFace(cirOut[0], cirInner[0], cirInner[i]));
		}
		else
		{
			this->baseRing.push_back(FFace(cirOut[i], cirOut[i + 1], cirInner[i]));
			this->baseRing.push_back(FFace(cirOut[i + 1], cirInner[i + 1], cirInner[i]));
		}
	}

	cirOut.clear();
	cirInner.clear();
}

void  Dental::initalBaseSelectedRing(CustomPlane plane)
{
	vector<Point3m> cirOut, cirInner;

	float fDepth = 15.0f;
	Point3m baseCenter = plane.center + plane.axisZV * fDepth;
	Point3m baseAxisX = plane.axisXV;
	Point3m baseAxisY = plane.axisYV;

	vcg::tri::UpdateBounding<CMeshO>::Box(*this->p_mesh_);

	int edgeNum = 60;
	float stepRads = (2.0f * PI) / (float)edgeNum;

	Point3m p;
	float costhita = 0.0f;
	float sinthita = 0.0f;
	float circleRadius = this->p_mesh_->bbox.DimX() * 1.0f;
	for (int i = 0; i < edgeNum; ++i)
	{
		costhita = cos(stepRads * i);
		sinthita = sin(stepRads * i);
		p = baseCenter + (baseAxisX * costhita + baseAxisY * sinthita) * circleRadius;
		cirOut.push_back(p);
	}
	costhita = 0.0f;
	sinthita = 0.0f;
	circleRadius = this->p_mesh_->bbox.DimX() * 0.8f;
	for (int i = 0; i < edgeNum; ++i)
	{
		costhita = cos(stepRads * i);
		sinthita = sin(stepRads * i);
		p = baseCenter + (baseAxisX * costhita + baseAxisY * sinthita) * circleRadius;
		cirInner.push_back(p);
	}

	this->baseRing.clear();
	for (int i = 0; i < cirOut.size(); ++i)
	{
		if (i == cirOut.size() - 1)
		{
			this->baseRing.push_back(FFace(cirOut[i], cirOut[0], cirInner[i]));
			this->baseRing.push_back(FFace(cirOut[0], cirInner[0], cirInner[i]));
		}
		else
		{
			this->baseRing.push_back(FFace(cirOut[i], cirOut[i + 1], cirInner[i]));
			this->baseRing.push_back(FFace(cirOut[i + 1], cirInner[i + 1], cirInner[i]));
		}
	}

	cirOut.clear();
	cirInner.clear();
}

// AccessoryTool removed - file deleted
// CureInfom removed - passiveStageList no longer exists

// collectPassiveCureInfomations - commented out due to sonCureInfo removal
// This function depends on the sub-stage functionality which has been removed
// 	// AccessoryTool removed - file deleted

// CureInfom removed - collectTemplateCureInfomations no longer exists

void Dental::makeTeethCatchingEye(vector<int> teethIndexList)
{
	setModelTransparency(100);
	for (auto index : teethIndexList)
	{
		if (bToothExist[index])
		{
			teeth[index].setModelTransparency(255);
		}
	}
}

void Dental::makeToothCatchingEye(int toothIndex)
{
	if (toothIndex == -1)
	{
		this->setModelTransparency(255);
	}
	else
	{
		setModelTransparency(100);
		if (bToothExist[toothIndex])
		{
			teeth[toothIndex].setModelTransparency(255);
		}
	}
}

void Dental::makeDentalNotCatchingEye()
{
	this->setModelTransparency(50);
	return;
}

void Dental::getTeethNum()
{
	this->iToothNum = 0;
	for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (this->bToothExist[i])
		{
			this->iToothNum += 1;
		}
	}
}

void Dental::freshDentationWithRepairedMesh(CMeshO* repairedMesh)
{
	//1.使用新修复过得repairedMesh来替换frozenMesh和pMesh指针
	vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(*this->p_mesh_, *repairedMesh);
	//this->inital(repairedMesh);
	this->frozenMesh.Clear();
	vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(this->frozenMesh, *repairedMesh);
	int vertexSummary = p_mesh_->VN();
	int faceSummary = p_mesh_->FN();
	//2.使用repairedMesh来更新牙齿对象的参数
	for (int ii = 1; ii < STANDARD_TOOTH_SUM; ++ii)
	{
		if (this->bToothExist[ii])
		{
			int sign = ii;

			vector<int> tempToothIndex;
			vector<int> tempToothFaceIndex;
			//复制源网格数据到tempTooth
			vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(tempmesh[ii], *p_mesh_);
			//根据标记网格signedMesh和sign对进行tempTooth网格面进行筛选
			for (int i = 0; i < faceSummary; i++)
			{
				if ((int)p_mesh_->face[i].V(0)->Q() != sign || (int)p_mesh_->face[i].V(1)->Q() != sign || (int)p_mesh_->face[i].V(2)->Q() != sign)
				{
					vcg::tri::Allocator<CMeshO>::DeleteFace(tempmesh[ii], tempmesh[ii].face[i]);
				}
				else
					tempToothFaceIndex.push_back(i);
			}
			//根据标记网格signedMesh和sign对进行tempTooth网格点进行筛选
			for (int i = 0; i < vertexSummary; i++)
			{
				if ((int)p_mesh_->vert[i].Q() != sign)
				{
					vcg::tri::Allocator<CMeshO>::DeleteVertex(tempmesh[ii], tempmesh[ii].vert[i]);
				}
				else
					tempToothIndex.push_back(i);
			}
			this->teeth[ii].freshToothWithRepairedMesh(&tempmesh[ii], tempToothIndex, tempToothFaceIndex, this->p_mesh_);
		}
	}
}

void Dental::updateTreatmentDragRecords(vector<FEdge> moveRecords, vector<FEdge> rotateRecords)
{
	this->moveLinkRecords = moveRecords;
	this->rotateLinkRecords = rotateRecords;
}

void Dental::getEachToothSideVerts()
{
	Point3m tempFeatureVert;
	for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (this->bToothExist[i])
		{
			this->teeth[i].getCurrentSideVerts(this->p_mesh_, teeth[i].realTimeAxis);
			int indexA = this->teeth[i].locationVertA;
			int indexB = this->teeth[i].locationVertB;
			tempFeatureVert = this->p_mesh_->vert[indexA].P();
			tempFeatureVert += this->p_mesh_->vert[indexB].P();
			tempFeatureVert /= 2.0f;
			this->teeth[i].featureVert = tempFeatureVert;
			this->teeth[i].curFeatureVert = tempFeatureVert;
		}
	}
}

void Dental::collectSortIndexList()
{
	this->toothSortIndexList.clear();
	for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (this->bToothExist[i])
		{
			this->toothSortIndexList.push_back(i);
		}
	}
}

void Dental::intialSortToothCreviceLimitList()
{
	this->toothCreviceLimit.clear();
	for (int i = 0; i < this->toothSortIndexList.size() - 1; ++i)
	{
		this->toothCreviceLimit.push_back(Point3m(i, i + 1, EFFECTIVE_CREVICE_WIDTH));
	}
}

float Dental::UpdateDifferentialVectorSummary()
{
	if (this->toothSortIndexList.empty())
	{
		return 0;
	}

	Tooth* toothLast = NULL, * toothNext = NULL;
	for (int i = 0; i < this->toothSortIndexList.size(); ++i)
	{
		if (i == 0)
		{
			toothLast = NULL;
		}
		else
		{
			toothLast = &this->teeth[toothSortIndexList[i - 1]];
		}

		if (i == toothSortIndexList.size() - 1)
		{
			toothNext = NULL;
		}
		else
		{
			toothNext = &this->teeth[toothSortIndexList[i + 1]];
		}

		this->teeth[toothSortIndexList[i]].computeToothDifferentialVector(toothLast, toothNext, this->basePlane);
	}

	float summary = 0;
	for (auto index : this->toothSortIndexList)
	{
		summary += this->teeth[index].fDifferentialVectorLength;
	}
	return summary;
}

void Dental::constructDentalArchLine(SegmentedStatusInfo* savedInfo)
{
	if (!this->bDentalIsReady)
	{
		return;
	}
	if (!this->bBasalPlaneIsReady)
	{
		return;
	}
	this->collectSortIndexList();
	//拾取牙齿上的牙尖标记点
	for (auto i : this->toothSortIndexList)
	{
		if (this->bToothExist[i])
		{
			this->teeth[i].getLocationPoints();
		}
	}
	//按照牙数选取牙弓构造点
	vector<Point3m> tempNodes;
	int pickedToothList[5];
	Point3m startNode, endNode;
	if (this->toothSortIndexList.size() % 2 == 0)
	{
		pickedToothList[0] = 0;
		pickedToothList[4] = toothSortIndexList.size() - 1;
		pickedToothList[2] = toothSortIndexList.size() / 2;
		pickedToothList[1] = (pickedToothList[0] + pickedToothList[2]) / 2;
		pickedToothList[3] = (pickedToothList[4] + pickedToothList[2]) / 2;
		for (int i = 0; i < 5; ++i)
		{
			if (i == 0)
			{
				startNode = savedInfo->posVertexPickedLocated[0].first;
			}
			if (i == 4)
			{
				endNode = (*--savedInfo->posVertexPickedLocated.end()).second;
			}

			//pickedToothList[i] = toothSortIndexList[pickedToothList[i]];
		}

		int toothIndex, vertAIndex, vertBIndex;
		for (int i = 0; i < 5; ++i)
		{
			if (i == 0)
			{
				tempNodes.push_back(startNode);
			}
			else if (i == 4)
			{
				tempNodes.push_back(endNode);
			}
			else if (i == 2)
			{
				tempNodes.push_back((savedInfo->posVertexPickedLocated[pickedToothList[i]].second + savedInfo->posVertexPickedLocated[pickedToothList[i] - 1].first) / 2.0f);
			}
			else
			{
				tempNodes.push_back((savedInfo->posVertexPickedLocated[pickedToothList[i]].first + savedInfo->posVertexPickedLocated[pickedToothList[i]].second) / 2.0f);
			}
		}
	}
	else
	{
		pickedToothList[0] = 0;
		pickedToothList[4] = toothSortIndexList.size() - 1;
		pickedToothList[2] = (toothSortIndexList.size() - 1) / 2;
		pickedToothList[1] = (pickedToothList[0] + pickedToothList[2]) / 2;
		pickedToothList[3] = (pickedToothList[4] + pickedToothList[2]) / 2;
		for (int i = 0; i < 5; ++i)
		{
			if (i == 0)
			{
				startNode = savedInfo->posVertexPickedLocated[0].first;
			}
			if (i == 4)
			{
				endNode = (*--savedInfo->posVertexPickedLocated.end()).second;
			}
		}
		int toothIndex, vertAIndex, vertBIndex;
		for (int i = 0; i < 5; ++i)
		{
			if (i == 0)
			{
				tempNodes.push_back(startNode);
			}
			else if (i == 4)
			{
				tempNodes.push_back(endNode);
			}
			else
			{
				tempNodes.push_back((savedInfo->posVertexPickedLocated[pickedToothList[i]].first + savedInfo->posVertexPickedLocated[pickedToothList[i]].second) / 2.0f);
			}
		}
	}
	//调用牙弓生成函数，生成初始牙弓线
	ArchLineMachine archLineMachine(this->basePlane, tempNodes, this->bUpperDental);
	archLineMachine.GenerateDentalArch();

	this->dentalArch = archLineMachine.archLineNodes;
	this->fDentalArchLength = archLineMachine.fDentalArchLength;
	this->bArchIsReady = true;
}

// SingleToothTreatmentInfomation removed - treatment functionality not used in FusionAnalyser
// 	////使用dental的frozenMesh和LocalAxis来确定各个牙齿的侧面标识点
// 	//使用当前存储的治疗周期模型得到碰撞表

// SingleToothTreatmentInfomation removed - treatment functionality not used in FusionAnalyser
// 	//各颗牙齿依据CureInfom获得当前curSideVerts
// 	//依据curSideVert来计算间隙..

// SingleToothTreatmentInfomation removed - treatment functionality not used in FusionAnalyser
// 		else

// SingleToothTreatmentInfomation removed - treatment functionality not used in FusionAnalyser
// 		else

void Dental::constructDentalConvexHull(const SegmentedStatusInfo& savedInfo, Dental *_ref_dental)
{
	if (!savedInfo.eachToothConvexVerts.empty())
	{
		if (savedInfo.eachToothConvexVerts.size() == this->toothSortIndexList.size())
		{
			int toothIndex;
			for (int i = 0; i < toothSortIndexList.size(); ++i)
			{
				toothIndex = toothSortIndexList[i];

				this->teeth[toothIndex].frozenConvexVerts = savedInfo.eachToothConvexVerts[i];
				this->teeth[toothIndex].convexVerts = savedInfo.eachToothConvexVerts[i];
				this->teeth[toothIndex].frozenConvexHullCenter = teeth[toothIndex].localAxis.centerPoint;
				this->teeth[toothIndex].convexHullCenter = teeth[toothIndex].localAxis.centerPoint;
			}
		}
	}
	else
	{
		constructEachToothConvexHull(_ref_dental);
	}

	this->convexHullGenerated = true;
}

void Dental::constructEachToothConvexHull(Dental *_ref_dental)
{
	if (_ref_dental != nullptr)
	{
		if (this->bUpperDental == _ref_dental->bUpperDental)
		{
			for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
			{
				if (this->bToothExist[i] && _ref_dental->bToothExist[i])
				{
					this->teeth[i].frozenConvexVerts = _ref_dental->teeth[i].frozenConvexVerts;
					this->teeth[i].convexVerts = _ref_dental->teeth[i].convexVerts;
					this->teeth[i].convexFaces = _ref_dental->teeth[i].convexFaces;
					this->teeth[i].frozenConvexHullCenter = _ref_dental->teeth[i].frozenConvexHullCenter;
					this->teeth[i].convexHullCenter = _ref_dental->teeth[i].convexHullCenter;

					this->teeth[i].InitalFeatureMarks(_ref_dental->teeth[i].feature_marks_);
				}
			}
		}
		return;
	}

	int counter = 0;
	for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (this->bToothExist[i])
		{
			constructOneToothConvexHull(i);
		}
	}
}

bool Dental::constructOneToothConvexHull(int i)
{
	updateModel(teeth[i].p_mesh_);
	vector<Point3m> verts;
	if (teeth[i].root_vertindex_interval_.X() >= 0 && teeth[i].root_vertindex_interval_.Y() > teeth[i].root_vertindex_interval_.X())
	{
		for (int i = 0; i < teeth[i].p_mesh_->vert.size(); ++i)
		{
			if (teeth[i].root_vertindex_interval_.X() <= i && i <= teeth[i].root_vertindex_interval_.Y())
			{
				continue;
			}
			verts.push_back(teeth[i].p_mesh_->vert[i].P());
		}
	}
	else
	{
		int limited = teeth[i].p_mesh_->vert.size() - this->teeth[i].boundaryIndex.size() * 40;
		for (int j = 0; j < limited - 1; ++j)
		{
			verts.push_back(teeth[i].p_mesh_->vert[j].P());
		}
	}

	ConvexHullMachine convexHull;
	convexHull.inital(verts, this->teeth[i].realTimeAxis);

	if(convexHull.convexVerts.size() > 0)
	{
		this->teeth[i].frozenConvexVerts = convexHull.convexVerts;
		this->teeth[i].convexVerts = convexHull.convexVerts;
		this->teeth[i].convexFaces = convexHull.convexFaces;
		this->teeth[i].frozenConvexHullCenter = teeth[i].localAxis.centerPoint;
		this->teeth[i].convexHullCenter = teeth[i].localAxis.centerPoint;

		vector<Point3m>().swap(verts);
		return true;
	}
	return false;
}

void Dental::computeEachToothCreviceEdges(const vector<int> &_tooth_list)
{

}

// CureInfom-related functions removed - treatment functionality not used in FusionAnalyser

bool Dental::getTwoTeethCreviceEdge(int iToothA, int iToothB, FEdge& showEdge)
{
	Point3m toothACenter, toothBCenter;
	vector<Point3m> toothA, toothB;
	toothACenter = teeth[iToothA].realTimeAxis.centerPoint;
	toothA = teeth[iToothA].convexVerts;
	toothBCenter = teeth[iToothB].realTimeAxis.centerPoint;
	toothB = teeth[iToothB].convexVerts;

	GJKMachine gjkTestMachine;
	if (!toothA.empty() && !toothB.empty())
	{
		bool intersectHappend = gjkTestMachine.test(toothACenter, toothA, toothBCenter, toothB);

		if (intersectHappend)
		{
			gjkTestMachine.showEdge.fLength = gjkTestMachine.fPiercingDistance;
		}
		else
		{
			gjkTestMachine.showEdge.fLength = -gjkTestMachine.fCreviceDistance;
		}
		gjkTestMachine.showEdge.indexA = iToothA;
		gjkTestMachine.showEdge.indexB = iToothB;

		showEdge = gjkTestMachine.showEdge;
		return true;
	}
	return false;
}

int Dental::computeColestVertIndexOnToothConvexHull(Point3m v, int iTooth)
{
	if (teeth[iTooth].realTimeMesh != nullptr)
	{
		delete teeth[iTooth].realTimeMesh;
		teeth[iTooth].realTimeMesh = nullptr;
	}
	teeth[iTooth].realTimeMesh = teeth[iTooth].getRealtimeMesh();

	vector<FEdge> result;
	tri::RequireCompactness(*teeth[iTooth].realTimeMesh);
	VertexConstDataWrapper<CMeshO> ww(*teeth[iTooth].realTimeMesh);
	KdTree<float> kt(ww);
	typename KdTree<float>::PriorityQueue pq;
	int neighbor = 2;

	int idA, idB, index;
	float distanceA, distanceB;
	kt.doQueryK(v, neighbor, pq);

	idA = pq.getIndex(0);
	idB = pq.getIndex(1);
	distanceA = (teeth[iTooth].realTimeMesh->vert[idA].P() - v) * (teeth[iTooth].realTimeMesh->vert[idA].P() - v);
	distanceB = (teeth[iTooth].realTimeMesh->vert[idB].P() - v) * (teeth[iTooth].realTimeMesh->vert[idB].P() - v);
	index = distanceA < distanceB ? idA : idB;
	return index;
}

Point3m Dental::computeColestVertOnToothConvexHull(Point3m v, int iTooth)
{
	int index = computeColestVertIndexOnToothConvexHull(v, iTooth);
	return  teeth[iTooth].realTimeMesh->vert[index].P();
}
bool Dental::computeIntersectedToothCPwithAnohterDental(vector<FEdge>& result, Dental* aimDental)
{
	result.clear();
	for (int i = 0; i < toothSortIndexList.size(); ++i)
	{
		vector<FEdge> temp_result;
		singleToothCollideWithAnohterDental(toothSortIndexList[i], aimDental, temp_result);
		result.insert(result.end(), temp_result.begin(), temp_result.end());
		vector<FEdge>().swap(temp_result);
	}
	return true;
}

bool Dental::singleToothCollideWithAnohterDental(int _i_tooth, Dental* aimDental, vector<FEdge> &result)
{
	result.clear();
	int iToothA, iToothB;
	Point3m toothACenter, toothBCenter;
	vector<Point3m> toothA, toothB;

	GJKMachine gjkTestMachine;
	iToothA = _i_tooth;
	for (int j = 0; j < aimDental->toothSortIndexList.size(); ++j)
	{
		iToothB = toothSortIndexList[j];

		toothACenter = teeth[iToothA].realTimeAxis.centerPoint;
		toothA = teeth[iToothA].convexVerts;
		toothBCenter = aimDental->teeth[iToothB].realTimeAxis.centerPoint;
		toothB = aimDental->teeth[iToothB].convexVerts;

		if (!toothA.empty() && !toothB.empty())
		{
			bool intersectHappend = gjkTestMachine.test(toothACenter, toothA, toothBCenter, toothB);

			if (intersectHappend)
			{
				gjkTestMachine.showEdge.indexA = iToothA;
				gjkTestMachine.showEdge.indexB = iToothB;
				gjkTestMachine.showEdge.fLength = gjkTestMachine.fPiercingDistance;
				result.push_back(gjkTestMachine.showEdge);
			}
		}
	}

	return true;
}

bool Dental::singleToothMarkedBallCollideWithAnohterDental(int _i_tooth, Dental* aimDental, vector<FEdge>& result)
{
	result.clear();
	int iToothA, iToothB;
	Point3m toothACenter, toothBCenter;
	vector<Point3m> toothA, toothB;

	GJKMachine gjkTestMachine;
	iToothA = _i_tooth;
	for (int j = 0; j < aimDental->toothSortIndexList.size(); ++j)
	{
		iToothB = toothSortIndexList[j];

		toothACenter = teeth[iToothA].p_mesh_->vert[teeth[iToothA].localArrageMarkedVert].P();
		toothACenter = teeth[iToothA].p_model_->cm.Tr * toothACenter;
		toothA = constructBall(toothACenter);
		toothBCenter = aimDental->teeth[iToothB].realTimeAxis.centerPoint;
		toothB = aimDental->teeth[iToothB].convexVerts;

		if (!toothA.empty() && !toothB.empty())
		{
			bool intersectHappend = gjkTestMachine.test(toothACenter, toothA, toothBCenter, toothB);

			if (intersectHappend)
			{
				gjkTestMachine.showEdge.indexA = iToothA;
				gjkTestMachine.showEdge.indexB = iToothB;
				gjkTestMachine.showEdge.fLength = gjkTestMachine.fPiercingDistance;
				result.push_back(gjkTestMachine.showEdge);
			}
		}
	}

	return true;
}

vector<Point3m> Dental::constructBall(Point3m _center)
{
	float radius = 1.0f;

	vector<Point3m> result;
	int time = 12;
	float x, y, z, step_degree = 180 / (float)(time);
	for (int i = 1; i < time; ++i)
	{
		z = radius * cos(i * step_degree);
		for (int j = 1; j < time; ++j)
		{
			y = radius * cos(j * step_degree);
			for (int k = 1; k < time; ++k)
			{
				x = radius * cos(k * step_degree);

				result.push_back(_center + Point3m(x, y, z));
			}
		}
	}
	return result;
}

FEdge Dental::computeAdjacentToothCreviceEdge(int iToothA, int iToothB)
{
	Point3m toothACenter, toothBCenter;
	vector<Point3m> toothA, toothB;

	GJKMachine gjkTestMachine;

	toothACenter = teeth[iToothA].realTimeAxis.centerPoint;
	toothA = teeth[iToothA].convexVerts;
	toothBCenter = teeth[iToothB].realTimeAxis.centerPoint;
	toothB = teeth[iToothB].convexVerts;

	if (!toothA.empty() && !toothB.empty())
	{
		bool intersectHappend = gjkTestMachine.test(toothACenter, toothA, toothBCenter, toothB);

		if (intersectHappend)
		{
			gjkTestMachine.showEdge.fLength = gjkTestMachine.fPiercingDistance;
		}
		else
		{
			gjkTestMachine.showEdge.fLength = -gjkTestMachine.fCreviceDistance;
		}
		gjkTestMachine.showEdge.indexA = iToothA;
		gjkTestMachine.showEdge.indexB = iToothB;
		return gjkTestMachine.showEdge;
	}

	return FEdge(Point3m(0, 0, 0), Point3m(0, 0, 0));
}

bool Dental::constructRigidbodySys(int startToothIndex)
{
	int curPos = -1;
	for (int i = 0; i < this->toothSortIndexList.size(); ++i)
	{
		if (toothSortIndexList[i] == startToothIndex)
		{
			curPos = i;
			break;
		}
	}
	if (curPos < 0)
	{
		return false;
	}

	for (auto index : this->toothSortIndexList)
	{
		this->teeth[index].updateRigidbody();
	}

	if (curPos == 0)
	{
		this->constructLinkageRigidbody(curPos, NULL, 1);
	}
	else if (curPos == this->toothSortIndexList.size() - 1)
	{
		this->constructLinkageRigidbody(curPos, NULL, -1);
	}
	else
	{
		this->constructLinkageRigidbody(curPos, NULL, 1);
		this->constructLinkageRigidbody(curPos, NULL, -1);
	}
	return true;
}

void Dental::constructLinkageRigidbody(int sortListIndex, Rigidbody* active, int step)
{
	int neighIndex = sortListIndex + step;
	int toothIndex = this->toothSortIndexList[sortListIndex];
	if (0 <= neighIndex && neighIndex < this->toothSortIndexList.size())
	{
		float fCreviceLimit = EFFECTIVE_CREVICE_WIDTH;
		int neighToothIndex = this->toothSortIndexList[neighIndex];

		for (auto record : this->toothCreviceLimit)
		{
			if ((record.X() == sortListIndex && record.Y() == neighIndex) || (record.Y() == sortListIndex && record.X() == neighIndex))
			{
				fCreviceLimit = record.Z();
				break;
			}
		}

		this->teeth[neighToothIndex].rigid->fFrozenDistance = fCreviceLimit;
		constructLinkageRigidbody(neighIndex, this->teeth[toothIndex].rigid, step);
		this->teeth[toothIndex].rigid->passiveRigids.push_back(this->teeth[neighToothIndex].rigid);
	}
	else
	{
		this->teeth[toothIndex].rigid->passiveRigids.clear();
	}
	this->teeth[toothIndex].rigid->activeRigid = active;
	return;
}

bool Dental::insertKeyStageWithSubStage(int iIndex, int& tempNewKeyStageIndex, vector<FEdge>& oldMoveRecords, vector<FEdge>& oldRotateRecords, MeshDocument* _md)
{
	return false;
}

bool Dental::recoverKeyStageWith(int iKeyStage, vector<FEdge> moveRecords, vector<FEdge> rotateRecords)
{

	return true;
}

vector<FEdge> Dental::splitLinkRecordsWith(int iIndex, vector<FEdge> records)
{
	vector<FEdge> result;

	int min, max, y;
	Point3m a, b;
	for (auto record : records)
	{
		if (record.vertA.X() == record.vertB.X())
		{
			if (record.vertA.X() != iIndex)
			{
				result.push_back(record);
			}
			continue;
		}
		else if (record.vertA.X() < record.vertB.X())
		{
			min = record.vertA.X();
			max = record.vertB.X();
			y = record.vertA.Y();
		}
		else if (record.vertB.X() < record.vertA.X())
		{
			min = record.vertB.X();
			max = record.vertA.X();
			y = record.vertA.Y();
		}

		if (min == iIndex)
		{
			a = Point3m(min + 1, y, 0);
			b = Point3m(max, y, 0);
			result.push_back(FEdge(a, b));
		}
		else if (max == iIndex)
		{
			a = Point3m(min, y, 0);
			b = Point3m(max - 1, y, 0);
			result.push_back(FEdge(a, b));
		}
		else if (min < iIndex && iIndex < max)
		{
			a = Point3m(min, y, 0);
			b = Point3m(iIndex - 1, y, 0);
			result.push_back(FEdge(a, b));

			a = Point3m(iIndex + 1, y, 0);
			b = Point3m(max, y, 0);
			result.push_back(FEdge(a, b));
		}
		else
		{
			result.push_back(record);
		}
	}

	return result;
}

void Dental::generateProfileSurfaceVertsIndex(vector<int>& transVertList, vector<vector<int>>& frontVertToothMatrix, vector<vector<int>>& backVertToothMatrix)
{
	//依据ontop_Front_CtrlPTs和projOnBspline_Front生成Front的曲线节点，组成侧面点集
	int iToothCtrlNum = 5, iLinkCtrlNum = 5;
	int iSingalToothBoundaryLen = iToothCtrlNum + iLinkCtrlNum;
	int ontopCtrlPTsSize = this->toothSortIndexList.size() * iToothCtrlNum + (this->toothSortIndexList.size() - 1) * iLinkCtrlNum;

	for (int i = 0; i < ontopCtrlPTsSize; i++)
	{
		int iToothIndex = i / iSingalToothBoundaryLen;
		//将新生成的点加入到点表中
		for (int j = 0; j < (iToothCtrlNum + iLinkCtrlNum + 1); ++j)
		{
			transVertList.push_back(1);
			frontVertToothMatrix[iToothIndex].push_back(transVertList.size() - 1);
		}
	}

	for (int i = 0; i < ontopCtrlPTsSize; i++)
	{
		int iToothIndex = (this->toothSortIndexList.size() - 1) - (i / iSingalToothBoundaryLen);

		//将新生成的点加入到点表中
		for (int j = 0; j < (iToothCtrlNum + iLinkCtrlNum + 1); ++j)
		{
			transVertList.push_back(1);
			backVertToothMatrix[iToothIndex].push_back(transVertList.size() - 1);
		}
	}
}

void Dental::generateTopSurfaceVertsIndex(vector<int>& transVertList, vector<vector<int>>& topVertToothMatrix)
{
	int iToothCtrlNum = 5, iLinkCtrlNum = 5;
	int iSingalToothBoundaryLen = iToothCtrlNum + iLinkCtrlNum;
	int ontopCtrlPTsSize = this->toothSortIndexList.size() * iToothCtrlNum + (this->toothSortIndexList.size() - 1) * iLinkCtrlNum;
	int segnum = 10;

	//ontopCtrlPTsSize - 1

	for (int i = 1; i < ontopCtrlPTsSize - 1; ++i)
	{
		int iToothIndex = i / iSingalToothBoundaryLen;

		for (int j = 0; j < segnum - 2; ++j)
		{
			transVertList.push_back(1);
			topVertToothMatrix[iToothIndex].push_back(transVertList.size() - 1);
		}
	}
}

// SingleToothTreatmentInfomation removed - treatment functionality not used in FusionAnalyser
// 	//搜集所有子步骤
// 	else
// 		else

// SingleToothTreatmentInfomation removed - treatment functionality not used in FusionAnalyser
// 			else
// 		else
// 			else
// 		else

// SingleToothTreatmentInfomation removed - treatment functionality not used in FusionAnalyser
// 	vector<QString>& normalInfo, vector<QString>& pressureInfo, vector<QString>& towHookInfo, vector<QString>& towBtnInfo, bool& bLeftHook, bool& normalAttachExist)
// 		else
// 			else
// 		else
// 			else
// 			else
// 			else
// 		else

int Dental::getTreatmentSubStageStepSummary()
{
	if (!this->bCureModelGenerated)
	{
		return 0;
	}
	// CureInfom removed - stageInfoList no longer exists

	// sonCureInfo removed - treatment sub-stage functionality removed
	// Return 0 since sub-stages no longer exist
	return 0;
	// Original implementation:
	// int result = 0;
	// for (int i = 0; i < this->stageInfoList->size(); ++i)
	// {
	// 	result += this->stageInfoList->at(i).sonCureInfo.size();
	// }
	// return result;
}

void Dental::clearAllLabelMesh()
{
	this->bLabelMeshReady = false;
	// CureInfom removed - stageInfoList no longer exists
		// 	// reloadBaseBraceMesh removed - BaseBrace functionality removed
	// CureInfom removed - templateStageList and passiveStageList no longer exist
}

void Dental::clearLabelMeshAndContent()
{
	this->bLabelContentReady = false;
	vector<QString>().swap(labelContentList);
	this->clearAllLabelMesh();
}

void Dental::updateFixtureCircle()
{
	Point3m p;
	float circleRadius = this->fFixtureLimitRadius;
	float costhita = 0.0f;
	float sinthita = 0.0f;
	std::vector<Point3m>().swap(this->circle_Fixture);
	for (int i = 0; i < CIRCLE_DIVIDE_NUMBER; ++i)
	{
		costhita = cos(STEP_RADS * i);
		sinthita = sin(STEP_RADS * i);
		p = this->fixtureCSYS.centerPoint + (this->fixtureCSYS.axisXVector * costhita + this->fixtureCSYS.axisYVector * sinthita) * circleRadius;
		this->circle_Fixture.push_back(p);
	}
	return;
}

void Dental::getEachToothDefaultMarkerVert(const vector<int> &_tooth_list)
{
	for (auto index : _tooth_list)
	{
		this->teeth[index].computeDefaultMarkerPointIndexes();
	}
}

bool Dental::deleteSwelledTeeth()
{
	for (auto index : this->toothSortIndexList)
	{
		int sign = this->teeth[index].strFDI.toInt();
		sign %= 10;
		if (sign != 6 && sign != 7 && sign != 8)
		{
			this->teeth[index].deleteSwelledOutTooth();
		}
	}
	return true;
}

bool Dental::generateSwelledFrozenMesh(float _height, float _reduction)
{
	//对swelledMesh网格每一个牙执行膨胀操作
	for (auto index : this->toothSortIndexList)
	{
		int sign = this->teeth[index].strFDI.toInt();
		sign %= 10;
		if (sign != 6 && sign != 7 && sign != 8)
		{
			this->teeth[index].swelledOutToothOn(_height, _reduction);
		}
	}
	return true;
}

// CureInfom-related functions removed - treatment functionality not used in FusionAnalyser
/*
void Dental::updatePerfectSoleplateHeight(CureInfom *_info, vector<int> &_tooth_list)
{
	float result = 0;
	for (auto i_tooth : _tooth_list)
	{
		float tooth_perfect_height = 0;
		for (auto vert : teeth[i_tooth].front_CtrlPTs)
		{
			Point3m tempVec = vert - basePlane.center;
			float curRootDistance = tempVec * basePlane.axisZV;
			if (curRootDistance > tooth_perfect_height)
			{
				tooth_perfect_height = curRootDistance;
			}
		}
		for (auto vert : teeth[i_tooth].back_CtrlPTs)
		{
			Point3m tempVec = vert - basePlane.center;
			float curRootDistance = tempVec * basePlane.axisZV;
			if (curRootDistance > tooth_perfect_height)
			{
				tooth_perfect_height = curRootDistance;
			}
		}

		if (tooth_perfect_height > result)
		{
			result = tooth_perfect_height;
		}
	}

	result += 5.0f;
	_info->gum_soleplate_height_ = result;
	this->fHeightOfGumBase = result;
}
*/

void Dental::analysisExportableOriginalMesh(MeshDocument* _md, QString _fileName)
{
	MeshModel* new_model = _md->addNewMesh("", EXPORTABLE_ORIGINAL_LABEL);
	if (bUpperDental)
	{
		new_model->upperOrLowerToothModelMark = UpperSourceMesh;
	}
	else
	{
		new_model->upperOrLowerToothModelMark = LowerSourceMesh;
	}

	int load_mask;
	QString fileName = QDir::currentPath() + QString("/") + _fileName.left(_fileName.lastIndexOf("_")) + QString(".stl");
	int err = vcg::tri::io::ImporterSTL<CMeshO>::Open(new_model->cm, (fileName.toLocal8Bit()), load_mask);
	if (err != 0)
	{
		const char* errmsg = vcg::tri::io::ImporterSTL<CMeshO>::ErrorMsg(err);
		_md->delMesh(new_model);
		return;
	}
	tri::Clean<CMeshO>::RemoveDuplicateVertex(new_model->cm);
	tri::Allocator<CMeshO>::CompactEveryVector(new_model->cm);
	for (int i = 0; i < new_model->cm.vert.size(); ++i)
	{
		new_model->cm.vert[i].C() = CCOLOR_TOOTH;
		new_model->cm.vert[i].Q() = 0;
	}
	generateExportableOriginalMesh(&new_model->cm);

	if (original_mesh_ != nullptr)
	{
		SAFE_DELETE(original_mesh_);
	}
	original_mesh_ = new AbsMesh(new_model);
	original_mesh_->setVisible(false);
}

void Dental::generateExportableOriginalMesh(CMeshO* _mesh)
{
	qDebug() << "Before delete origin mesh vert size : " << _mesh->vert.size() << endl;
	qDebug() << "Before delete origin mesh face size : " << _mesh->face.size() << endl;

	Point3m plane_pos = fixtureCSYS.centerPoint;
	Point3m plane_normal = fixtureCSYS.axisZVector;
	plane_normal = plane_normal.Normalize();

	Point3m p, vec, proj_p;
	int del_vert_num = 0;
	for (int i = 0; i < _mesh->vert.size(); ++i)
	{
		p = _mesh->vert[i].P();
		proj_p = UtilityTools::getInstance()->getProjPointOnPlane(p, plane_pos, -plane_normal);
		vec = proj_p - p;
		float value = vec * plane_normal;

		if (value >= -1.0f)
		{
			vcg::tri::Allocator<CMeshO>::DeleteVertex(*_mesh, _mesh->vert[i]);
			++del_vert_num;
		}
	}

	int del_face_num = 0;
	vector<int> delete_list;
	for (int i = 0; i < _mesh->face.size(); ++i)
	{
		if (_mesh->face[i].V(0)->IsD() ||
			_mesh->face[i].V(1)->IsD() ||
			_mesh->face[i].V(2)->IsD())
		{
			vcg::tri::Allocator<CMeshO>::DeleteFace(*_mesh, _mesh->face[i]);
			++del_face_num;
		}
	}

	qDebug() << "origin mesh delete vert num : " << del_vert_num << endl;
	qDebug() << "origin mesh delete face num : " << del_face_num << endl;

	this->updateModel(_mesh);

	qDebug() << "After delete origin mesh vert size : " << _mesh->vert.size() << endl;
	qDebug() << "After delete origin mesh face size : " << _mesh->face.size() << endl;
	return;
}

bool Dental::gotFeatureMarks()
{
	bool got_it = true;
	for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (bToothExist[i])
		{
			if (teeth[i].feature_marks_.empty())
			{
				got_it = false;
				break;
			}
		}
	}
	return got_it;
}

void Dental::setToothWidthDefVerts(int _itooth, Point3m _a, Point3m _b, Point3m _c)
{
	teeth[_itooth].setWidthDefinitionPoint(_a, _b, _c);
	updateTeethWidthDefVertsList();
}

void Dental::updateTeethWidthDefVertsList()
{
	vector<vector<Point3m>>().swap(teeth_width_def_verts_list_);
	for (int i = 0; i < STANDARD_TOOTH_SUM; i++)
	{
		if (bToothExist[i])
		{
			vector<Point3m> verts_list;
			verts_list.push_back(teeth[i].width_definition_point_a_);
			verts_list.push_back(teeth[i].width_definition_point_b_);
			verts_list.push_back(teeth[i].width_definition_point_c_);
			teeth_width_def_verts_list_.push_back(verts_list);
		}
	}
}
