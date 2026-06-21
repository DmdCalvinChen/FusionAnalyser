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
#include "util/oriented_bounding_box.h"
// VirtualGingiva removed - file deleted
// CureInfom removed - treatment functionality not used in FusionAnalyser
// DentalAnimate removed - files deleted
#include "common_base/SignalManager.h"
//#include "common_base/util/uitools.h"

// CureInfom-related functions removed - treatment functionality not used in FusionAnalyser

bool Dental::getCtrlPTsOnEachToothNewestWay(const vector<int> &_tooth_list)
{
	return true;
}

bool Dental::getCtrlPTsOnEachToothNewestWay(int iTooth)
{
	return false;
}

bool Dental::getFillingbodyCtrlPTsOnEachTooth(const vector<int> &_tooth_list)
{
	return true;
}

bool Dental::getCutLineCtrlPTsOnEachToothTangentWay(CMeshO *curMesh)
{
	return true;
}

bool Dental::getCutLineCtrlPTsOnEachToothNewestWay(CMeshO *curMesh)
{
	return true;
}

void Dental::setRenderObject(DentalRenderObject _object)
{
	this->render_obj_ = _object;
	switch (render_obj_)
	{
	case  VIRTUAL_JAW_ARRANGE:
		this->setVisible(true);
		for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
		{
			if (bToothExist[i])
			{
				teeth[i].setVisible(true);
			}

		}
		if (basalLabel_ != nullptr)
		{
			basalLabel_->setVisible(false);
		}
		if (export_result_ != nullptr)
		{
			export_result_->setVisible(false);
		}
		break;

	case  REAL_ROOT_JAWBONE:
		this->setVisible(false);
		for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
		{
			if (bToothExist[i])
			{
				teeth[i].setVisible(false);
			}

		}
		if (basalLabel_ != nullptr)
		{
			basalLabel_->setVisible(false);
		}
		if (export_result_ != nullptr)
		{
			export_result_->setVisible(false);
		}
		break;
	case  BASAL_LABEL:
		this->setVisible(false);
		if (basalLabel_ != nullptr)
		{
			basalLabel_->setRealTimeAxis(basalLabel_->localAxis);
			basalLabel_->setVisible(true);
		}
		if (export_result_ != nullptr)
		{
			export_result_->setVisible(false);
		}
		break;
	case  EXPORTABLE_RESULT:
		this->setVisible(false);
		if (basalLabel_ != nullptr)
		{
			basalLabel_->setRealTimeAxis(this->fixtureCSYS);
			basalLabel_->setVisible(true);
		}
		if (export_result_ != nullptr)
		{
			export_result_->setVisible(true);
		}
		break;
	default:
		break;
	}

	setCompareModelVisible(bCompareModelVisible_);
}

void Dental::setCompareModelVisible(bool _visible)
{
	this->bCompareModelVisible_ = _visible;
	if (bCompareModelVisible_)
	{
		switch (render_obj_)
		{
		case  VIRTUAL_JAW_ARRANGE:
			for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
			{
				if (bToothExist[i] && teeth[i].compare_obj_ != nullptr)
				{
					teeth[i].compare_obj_->setVisible(true);
				}
			}
			break;

		case REAL_ROOT_JAWBONE:
			for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
			{
				if (bToothExist[i] && teeth[i].compare_obj_ != nullptr)
				{
					teeth[i].compare_obj_->setVisible(false);
				}
			}
			break;

		default:
			break;
		}
	}
	else
	{
		for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
		{
			if (bToothExist[i] && teeth[i].compare_obj_ != nullptr)
			{
				teeth[i].compare_obj_->setVisible(false);
			}
		}
	}
}

int Dental::pickTooth(int _mouseX, int _mouseY, vector<CFaceO*>& _vf, int &_iface)
{
	for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (!bToothExist[i])
		{
			continue;
		}
		if (teeth[i].bPulled_out_)
		{
			continue;
		}
		if (teeth[i].bArtificial_)
		{
			continue;
		}
		if (teeth[i].pickedUp(_mouseX, _mouseY, _vf, _iface))
		{
			return i;
		}
	}
	return -1;
}

int Dental::getIntersectedToothIndexByBoundBoxWay(AbsMesh *_obj)
{
	if (_obj->BBTreeRoot != nullptr)
	{
		delete _obj->BBTreeRoot;
	}
	CMeshO *temp_mesh = _obj->getLocationMesh();
	_obj->getRootOfBBTree(temp_mesh);
	delete temp_mesh;
	temp_mesh = nullptr;

	int **cMA;
	int rowNum, columnNum;
	for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (bToothExist[i])
		{
			if (teeth[i].BBTreeRoot == nullptr)
			{
				teeth[i].getRootOfBBTree();
			}

			cMA = teeth[i].getCollideObjectMatrix(_obj->BBTreeRoot, rowNum, columnNum);
			if (rowNum > 0)
			{
				for (int j = 0; j < rowNum; ++j)
				{
					delete []cMA[j];
				}
				delete[]cMA;
				return i;
			}
		}
	}
	return -1;
}

void Dental::setExportableMeshReady(bool _ready)
{
	this->bExportResultReady = _ready;
}

int Dental::getToothIndexAccording(int _i_fdi)
{
	switch (_i_fdi)
	{
	case 18:
	case 48:
		return 1;

	case 17:
	case 47:
		return 2;

	case 16:
	case 46:
		return 3;

	case 15:
	case 45:
		return 4;

	case 14:
	case 44:
		return 5;

	case 13:
	case 43:
		return 6;

	case 12:
	case 42:
		return 7;

	case 11:
	case 41:
		return 8;

		////////////////////////////////////

	case 21:
	case 31:
		return 9;

	case 22:
	case 32:
		return 10;

	case 23:
	case 33:
		return 11;

	case 24:
	case 34:
		return 12;

	case 25:
	case 35:
		return 13;

	case 26:
	case 36:
		return 14;

	case 27:
	case 37:
		return 15;

	case 28:
	case 38:
		return 16;
	}
}

void Dental::autoGenerateOcclusalPlane()
{

}

void Dental::autoSmoothToEliminateOverlap(int _toothA, int _toothB)
{
	if (!bToothExist[_toothA] || !bToothExist[_toothB])
	{
		return;
	}
	FEdge collide_result = computeAdjacentToothCreviceEdge(_toothA, _toothB);
	if (collide_result.fLength <= 0 || collide_result.fLength > 5.0f)
	{
		return;
	}

	Point3m area_center = (collide_result.vertA + collide_result.vertB) / 2.0f;
	vector<vector<int>> gradation_a = searchGradationRingWithCenter(area_center, _toothA, 10);
	vector<vector<int>> gradation_b = searchGradationRingWithCenter(area_center, _toothB, 10);

	float adj_value = collide_result.fLength * 0.5f;
	smoothToothMeshToEliminateOverlap(_toothA, gradation_a, adj_value);
	smoothToothMeshToEliminateOverlap(_toothB, gradation_b, adj_value);
}

vector<vector<int>> Dental::searchGradationRingWithCenter(Point3m _search_center, int _iTooth, int _ring_num)
{
	int diffusion_center_index = computeColestVertIndexOnToothConvexHull(_search_center, _iTooth);
	vector<vector<int>> gradation;
	vector<int> last_ring, new_ring;

	CMeshO *cur_mesh = this->teeth[_iTooth].p_mesh_;
	int size = cur_mesh->vert.size();

	bool* checked_list = new bool[size];
	memset(checked_list, 0, size);
	if (checked_list == nullptr)
	{
		return gradation;
	}

	for (int i = 0; i < _ring_num; ++i)
	{
		if (i == 0)
		{
			new_ring.push_back(diffusion_center_index);
			checked_list[diffusion_center_index] = true;

			gradation.push_back(new_ring);
		}
		else
		{
			last_ring = new_ring;
			new_ring.clear();

			for (auto &icur : last_ring)
			{
				vector<int> neighbor_verts = UtilityTools::getInstance()->oneRingNeighborhoodVV(icur, cur_mesh);
				for (auto& index : neighbor_verts)
				{
					if (!checked_list[index])
					{
						new_ring.push_back(index);
						checked_list[index] = true;
					}
				}
			}
			gradation.push_back(new_ring);
		}
	}

	delete []checked_list;
	checked_list = nullptr;
	return gradation;
}

void Dental::smoothToothMeshToEliminateOverlap(int _iTooth, vector<vector<int>>& _gradation, float _max_adj_value)
{

}

void Dental::loadSourceMesh(QString& _file_path, MeshDocument* _md, vcg::Matrix44f _matrix)
{
	if (_file_path.isEmpty())
	{
		return;
	}

	if (this->compared_source_mesh_)
	{
		CMeshO new_mesh;
		int err;
		int loadmask;
		err = vcg::tri::io::ImporterSTL<CMeshO>::Open(new_mesh, (_file_path.toStdString()).c_str(), loadmask);
		if (err != 0)
		{
			const char* errmsg = vcg::tri::io::ImporterSTL<CMeshO>::ErrorMsg(err);
		}
		new_mesh.Tr = vcg::Matrix44f::Identity();
		this->updateModel(&new_mesh);
		vcg::tri::UpdateBounding<CMeshO>::Box(new_mesh);

		vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(*compared_source_mesh_->p_mesh_, new_mesh);
	}
	else
	{
		MeshModel* new_compare_model = _md->addNewMesh("", COMPAREOBJ_LABEL, false);
		if (bUpperDental)
		{
			new_compare_model->upperOrLowerToothModelMark = UpperSourceMesh;
		}
		else
		{
			new_compare_model->upperOrLowerToothModelMark = LowerSourceMesh;
		}

		int err;
		int loadmask;

#if PLATFORM_IOS
    string file_name = _file_path.toStdString();
#else
    string file_name = _file_path.toLocal8Bit().constData();
#endif

		err = vcg::tri::io::ImporterSTL<CMeshO>::Open(new_compare_model->cm, (file_name).c_str(), loadmask);
		if (err != 0)
		{
			const char* errmsg = vcg::tri::io::ImporterSTL<CMeshO>::ErrorMsg(err);
		}
		new_compare_model->cm.Tr = _matrix;
		this->updateModel(&new_compare_model->cm);
		vcg::tri::UpdateBounding<CMeshO>::Box(new_compare_model->cm);

		this->compared_source_mesh_ = new AbsMesh(new_compare_model);
	}
}
