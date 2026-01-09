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

#include "dentalmanager.h"
#include	"../common_base/SignalManager.h"
#include "../common/meshmodel.h"
#include "common_base/util/uitools.h"
#define MIN_CREVICE_LENGTH 2.0f

DentalManager::DentalManager()
{
}

DentalManager::~DentalManager()
{
	this->cDental.bDentalIsReady = false;
}

void DentalManager::GetDental(CMeshO *_cm, bool _bUpperDental, QString _label_seg_mark, QString _full_path_filename_seg, const SegmentedStatusInfo &_segmentedStatusInfo, MeshDocument *_md, Dental *_ref_dental)
{
	this->asyncMonitor = 0x00000000;
	if (_segmentedStatusInfo.numToothSegmented == 0)
	{
		return;
	}

	this->neigh_state_ = _segmentedStatusInfo.recover_records_;
	this->cDental.loadDental(_cm, _bUpperDental, _label_seg_mark, _full_path_filename_seg, _segmentedStatusInfo, _md, _ref_dental);
	emit PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
	bGotDental = true;
}

void DentalManager::updateModel(CMeshO *curMesh)
{
	vcg::tri::Allocator<CMeshO>::CompactEveryVector(*curMesh);

	if (!tri::HasFFAdjacency(*curMesh))
	{
		curMesh->face.EnableFFAdjacency();
	}
	assert(tri::HasFFAdjacency(*curMesh) == true);

	if (!tri::HasVFAdjacency(*curMesh))
	{
		curMesh->vert.EnableVFAdjacency();
		curMesh->face.EnableVFAdjacency();
	}
	assert(tri::HasVFAdjacency(*curMesh) == true);
	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*curMesh);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(*curMesh);

	vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(*curMesh);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*curMesh);
}

void DentalManager::updateComparedDentalState(int _iNo)
{
	// CureInfom removed - treatment functionality not used in FusionAnalyser
	if (cDental.compare_obj_ != nullptr)
	{
		if(cDental.compare_obj_->p_mesh_->vert.size() > 0)
		{
			// DentalAnimate removed - files deleted
				emit PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
				emit PSIGNALMANAGER->updateMeshVertexInfoBufferCompare_PP();
			// }
		}
	}
}

void DentalManager::updateHistoricalComparedDentalState(int _iNo)
{
	// CureInfom removed - treatment functionality not used in FusionAnalyser
	if (cDental.compare_obj_ != nullptr)
	{
		// DentalAnimate removed - files deleted
		emit PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
		emit PSIGNALMANAGER->updateMeshVertexInfoBufferCompare_PP();
	}
}

void DentalManager::setVisible(bool _visible)
{
	bVisible_ = _visible;
}

bool DentalManager::isVisible()
{
	return bVisible_;
}

bool DentalManager::pickTooth(int _mouseX, int _mouseY, vector<CFaceO*>& _vf)
{
	int ifaceIndex = -1;
	int pickedToothIndex = this->cDental.pickTooth(_mouseX, _mouseY, _vf, ifaceIndex);

	if (pickedToothIndex > 0)
	{
		curPickedTooth_ = &this->cDental.teeth[pickedToothIndex];
		iCurrentSelectTooth_ = pickedToothIndex;
		return pickOnTooth(pickedToothIndex);
	}
	else
	{
		return pickOnTooth(-1);
	}
}

bool DentalManager::pickTooth(int _mouseX, int _mouseY, vector<CFaceO*>& _vf, int &pickedToothIndex, Point3m &pickedPos, Point3m &pickedFaceNormal, bool bDefaultPosEnable )
{
	int ifaceIndex = -1;
	pickedToothIndex = this->cDental.pickTooth(_mouseX, _mouseY, _vf, ifaceIndex);

	if (pickedToothIndex > 0 && ifaceIndex >= 0)
	{
		pickedPos = Point3m(0, 0, 0);
		pickedPos += cDental.teeth[pickedToothIndex].p_mesh_->face[ifaceIndex].V(0)->P();
		pickedPos += cDental.teeth[pickedToothIndex].p_mesh_->face[ifaceIndex].V(1)->P();
		pickedPos += cDental.teeth[pickedToothIndex].p_mesh_->face[ifaceIndex].V(2)->P();
		pickedPos /= 3.0f;
		pickedFaceNormal = cDental.teeth[pickedToothIndex].p_mesh_->face[ifaceIndex].N();
		return true;
	}
	return false;
}

bool DentalManager::pickOnTooth(int _index)
{
	if (_index < 0)
	{
		curPickedTooth_ = nullptr;
		iCurrentSelectTooth_ = -1;
		emit PSIGNALMANAGER->setToothAxisAdjCheckBoxEnableSignal(false);
		return false;
	}
	else
	{
		curPickedTooth_ = &this->cDental.teeth[_index];
		iCurrentSelectTooth_ = _index;
		emit PSIGNALMANAGER->setToothAxisAdjCheckBoxEnableSignal(true);
		return true;
	}
}

bool DentalManager::haveSelectedTooth()
{
	if (this->curPickedTooth_ != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void DentalManager::updateAfterAdjustSingleTooth(int _itooth)
{
	if (_itooth < 0 )
	{
		return;
	}
	vector<int> tempList;
	tempList.push_back(_itooth);
	updateAfterAdjustSomeTeeth(tempList);
	vector<int>().swap(tempList);
}

void DentalManager::updateAfterAdjustSomeTeeth(const vector<int> &_itoothlist)
{

}

void DentalManager::saveTeethAsToothModel()
{
	ToothModelManager model_manager;
	model_manager.dismantlingDental(&cDental);
}

void DentalManager::switchOverlapCompare(bool _visible)
{
	bCompareModelVisible_ = _visible;
	cDental.setCompareModelVisible(bCompareModelVisible_);
	emit PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
}

void DentalManager::setAnimationLoadComplete(bool _state)
{
	this->bAnimationLoadComplete_ = _state;
}

void DentalManager::setToothFixed(int _index)
{
	if (!this->cDental.bToothExist[_index])
	{
		return;
	}
	cDental.teeth[_index].setFixed(true);
	this->fixed_tooth_list_.push_back(_index);
}

void DentalManager::unfixSeletedFixedTooth()
{
	if (picked_fixed_tooth_index_ < 0)
	{
		return;
	}
	cDental.teeth[picked_fixed_tooth_index_].setFixed(false);
	vector<int>::iterator ite = fixed_tooth_list_.begin();
	while (ite != fixed_tooth_list_.end())
	{
		if (*ite == picked_fixed_tooth_index_)
		{
			fixed_tooth_list_.erase(ite);
		}
		else
		{
			++ite;
		}
	}
	picked_fixed_tooth_index_ = -1;
}

bool DentalManager::pickFixedToothAction(int _mouseX, int _mouseY)
{
	if (fixed_tooth_list_.empty())
	{
		return false;
	}

	int ifaceIndex = -1;
	vector<CFaceO*> vf;
	bool picked_fixed_tooth = false;
	picked_fixed_tooth_index_ = -1;
	for (auto index : fixed_tooth_list_)
	{
		if (cDental.teeth[index].pickedUp(_mouseX, _mouseY, vf, ifaceIndex))
		{
			picked_fixed_tooth_index_ = index;
			picked_fixed_tooth = true;
			break;
		}
	}
	return picked_fixed_tooth;
}
