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

#include "separationmanager.h"
#include "common_base/SignalManager.h"
#include "common_ext/util/mesh_generator.h"
#include <common/config.h>

ToothDataPackage::ToothDataPackage()
{
}

ToothDataPackage::ToothDataPackage(CMeshO *_mesh, int _sign, Axis _axis, QString _fdi, Point2i _root_interval, vector<Point3m> _boundary_verts, vector<Point3m> _marks)
{
	this->mesh_ = _mesh;
	sign_ = _sign;
	local_axis_ = _axis;
	fdi_ = _fdi;
	boundary_verts_ = _boundary_verts;
	boundary_num_ = _boundary_verts.size();
	root_interval_ = _root_interval;
	feature_marks_ = _marks;
}

ToothDataPackage::~ToothDataPackage()
{

}

SeparationManager::SeparationManager()
{
}

SeparationManager::SeparationManager(MeshModel* _source, bool _bUpper)
{
	init(_source, _bUpper);
}

SeparationManager::~SeparationManager()
{

	if (this->temp_dental_ != nullptr)
	{
		SAFE_DELETE(temp_dental_);
	}
	original_mesh_ = nullptr;
}

void SeparationManager::init(MeshModel* _source, bool _bUpper)
{
	this->original_mesh_ = _source;
	this->original_mesh_->setCullMode(CULL_NONE);

	this->b_upper_part_ = _bUpper;
	setRenderMode(ORIGINAL_MESH);
}

void SeparationManager::distributionTeethResources(vector<ToothDataPackage> _teeth_mesh, CustomPlane *_p_plane, MeshDocument *_md)
{
	if (this == nullptr)
		return;

	if (_teeth_mesh.empty())
	{
		return;
	}

	deleteDental();
	this->tooth_index_list.clear();
	temp_dental_ = new Dental();
	temp_dental_->bUpperDental = this->b_upper_part_;
	for (auto package : _teeth_mesh)
	{
		MeshModel* new_model = _md->addNewMesh("", TOOTHRD_LABEL, false, false);
		if (b_upper_part_)
		{
			new_model->upperOrLowerToothModelMark = UpperSingleTooth;
		}
		else
		{
			new_model->upperOrLowerToothModelMark = LowerSingleTooth;
		}
		vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(new_model->cm, *package.mesh_);

		int i_tooth = package.sign_ + 1;
		this->tooth_index_list.push_back(i_tooth);
		temp_dental_->bToothExist[i_tooth] = true;
		temp_dental_->teeth[i_tooth].updateModel(&new_model->cm);
		temp_dental_->teeth[i_tooth].InitalTooth(&new_model->cm, i_tooth, b_upper_part_, package.local_axis_, package.fdi_, false);
		temp_dental_->teeth[i_tooth].p_model_ = new_model;
		temp_dental_->teeth[i_tooth].setRootInterval(package.root_interval_);
		temp_dental_->teeth[i_tooth].realtimeBoundary = package.boundary_verts_;
		temp_dental_->teeth[i_tooth].boundaryIndex.clear();
		temp_dental_->teeth[i_tooth].InitalFeatureMarks(package.feature_marks_);

		for (auto vert : package.boundary_verts_)
		{
			for (auto vert_index : temp_dental_->teeth[i_tooth].toothIndex)
			{
				if (vert == temp_dental_->teeth[i_tooth].p_mesh_->vert[vert_index].P())
				{
					temp_dental_->teeth[i_tooth].boundaryIndex.push_back(vert_index);
				}
			}
		}
	}

	int task_index = 0;
	float f_sub_step = 45.0f / (float)(_teeth_mesh.size());
	for (auto package : _teeth_mesh)
	{
		int i_tooth = package.sign_ + 1;
		analysisToothConvexhull(&temp_dental_->teeth[i_tooth], package.boundary_num_ * 40);
		++task_index;

		float progress_value = 50 + f_sub_step * task_index;
		emit PSIGNALMANAGER->setProgressBarSignal(progress_value, QStringLiteral("分析..."));
		emit PSIGNALMANAGER->updateProgressGuiSignal(QStringLiteral("分析..."), progress_value, 100);
	}

	int ia, ib;
	for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (temp_dental_->bToothExist[i])
		{
			ia = i;
			for (int j = ia + 1; j < STANDARD_TOOTH_SUM; ++j)
			{
				if (temp_dental_->bToothExist[j])
				{
					ib = j;
					int fdi = temp_dental_->teeth[i].strFDI.toInt();
					temp_dental_->autoSmoothToEliminateOverlap(ia, ib);
				}
			}
		}
	}
	for (auto package : _teeth_mesh)
	{
		int i_tooth = package.sign_ + 1;
		analysisToothConvexhull(&temp_dental_->teeth[i_tooth], package.boundary_num_ * 40);
	}

	if (_p_plane)
	{
		temp_dental_->basePlane = *_p_plane;
	}
	else
	{

	}

	temp_dental_->basePlane.axisXV = -temp_dental_->basePlane.axisXV;
	temp_dental_->basePlane.axisZV = -temp_dental_->basePlane.axisZV;

	temp_dental_->computeEachToothCreviceEdges(this->tooth_index_list);
	setRenderMode(SEPARATION_TEETH);
	pickOnTooth(-1);
}

void SeparationManager::updateTeethConvexhull(std::set<int> &_index_list)
{
	float f_step = 100.0f / float(_index_list.size());
	int index = 0;
	emit PSIGNALMANAGER->setProgressBarSignal(0, QString(QObject::tr("Tooth reanalysis in progress...")));
	for (auto i : _index_list)
	{
		analysisToothConvexhull(&temp_dental_->teeth[i], temp_dental_->teeth[i].boundaryIndex.size() * 40);
		++index;
		emit PSIGNALMANAGER->setProgressBarSignal(index * f_step, QString(QObject::tr("Tooth reanalysis in progress...")));
	}
	temp_dental_->computeEachToothCreviceEdges(this->tooth_index_list);
	_index_list.clear();
}

void SeparationManager::analysisToothConvexhull(Tooth *_tooth, int _i_end_limit)
{
	vector<Point3m> verts;
	if (_tooth->root_vertindex_interval_.X() >= 0 && _tooth->root_vertindex_interval_.Y() > _tooth->root_vertindex_interval_.X())
	{
		for (int i = 0; i < _tooth->p_mesh_->vert.size(); ++i)
		{
			if (_tooth->root_vertindex_interval_.X() <= i && i <= _tooth->root_vertindex_interval_.Y())
			{
				continue;
			}
			verts.push_back(_tooth->p_mesh_->vert[i].P());
		}
	}
	else
	{
		int limited = _tooth->p_mesh_->vert.size() - _i_end_limit;
		for (int j = 0; j < limited - 1; ++j)
		{
			verts.push_back(_tooth->p_mesh_->vert[j].P());
		}
	}

	ConvexHullMachine convexHull;
	convexHull.inital(verts, _tooth->localAxis);

	_tooth->frozenConvexVerts = convexHull.convexVerts;
	_tooth->convexVerts = convexHull.convexVerts;
	_tooth->convexFaces = convexHull.convexFaces;
	_tooth->frozenConvexHullCenter = _tooth->localAxis.centerPoint;
	_tooth->convexHullCenter = _tooth->localAxis.centerPoint;

	vector<Point3m>().swap(verts);
}

void SeparationManager::setRenderMode(RenderObjMode _mode)
{
	switch (_mode)
	{
	case ORIGINAL_MESH:
		this->render_mode_ = _mode;
		if (original_mesh_ != nullptr)
		{
			original_mesh_->visible = true;
		}
		if (temp_dental_ != nullptr)
		{
			for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
			{
				if (temp_dental_->bToothExist[i])
				{
					temp_dental_->teeth[i].setVisible(false);
				}
			}
		}
		break;
	case SEPARATION_TEETH:
		this->render_mode_ = _mode;
		if (original_mesh_ != nullptr)
		{
			original_mesh_->visible = false;
		}
		if (temp_dental_ != nullptr)
		{
			for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
			{
				if (temp_dental_->bToothExist[i])
				{
					temp_dental_->teeth[i].setVisible(true);
				}
			}
		}
		break;
	case NONE_MODEL:
		if (original_mesh_ != nullptr)
		{
			original_mesh_->visible = false;
		}
		if (temp_dental_ != nullptr)
		{
			for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
			{
				if (temp_dental_->bToothExist[i])
				{
					temp_dental_->teeth[i].setVisible(false);
				}
			}
		}
		break;
	default:
		break;
	}
}

void SeparationManager::setVisible(bool _visible)
{
	this->b_visible_ = _visible;
	if (_visible)
	{
		this->setRenderMode(render_mode_);
	}
	else
	{
		this->setRenderMode(NONE_MODEL);
	}
}

void SeparationManager::switchVisible()
{
	setVisible(!b_visible_);
}

bool SeparationManager::pickToothByMouse(int _mouseX, int _mouseY, vector<CFaceO*>& _vf)
{
	if (temp_dental_ == nullptr)
	{
		return false;
	}

	int i_face;
	for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (temp_dental_->bToothExist[i])
		{
			if (temp_dental_->teeth[i].pickedUp(_mouseX, _mouseY, _vf, i_face))
			{
				return pickOnTooth(i);
			}
		}
	}
	return pickOnTooth(-1);
}

bool SeparationManager::pickOnTooth(int _index)
{
	if (temp_dental_ == nullptr)
	{
		return false;
	}

	index_tooth_picked_ = _index;

	setTeethTransparency(_index, alpha_transparency_mesh_);

	if (_index < 0)
	{
		cur_picked_tooth_ = nullptr;
		for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
		{
			if (temp_dental_->bToothExist[i])
			{
				temp_dental_->teeth[i].p_model_->setCullMode(CULL_BACK);
			}
		}
		return false;
	}
	else
	{
		cur_picked_tooth_ = &temp_dental_->teeth[_index];
		for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
		{
			if (temp_dental_->bToothExist[i])
			{
				if (i == _index)
				{
					temp_dental_->teeth[i].p_model_->setCullMode(CULL_BACK);
				}
				else
				{
					temp_dental_->teeth[i].p_model_->setCullMode(CULL_FRONT);
				}
			}
		}

		return true;
	}
}

void SeparationManager::setTeethTransparency(int _index, int _value_transparency)
{
	if (temp_dental_ == nullptr)
	{
		return;
	}

	for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (temp_dental_->bToothExist[i])
		{
			if (i == _index)
			{
				temp_dental_->teeth[i].setModelTransparency(255);
			}
			else
			{
				temp_dental_->teeth[i].setModelTransparency(_value_transparency);
			}
		}
	}
	emit PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
}

void SeparationManager::updateTeethTransparency(bool _update_all_mesh, int _value_transparency )
{
	if (!_update_all_mesh)
	{
		alpha_transparency_mesh_ = _value_transparency;
		setTeethTransparency(index_tooth_picked_, _value_transparency);
	}
	else
	{
		if (temp_dental_ == nullptr)
		{
			return;
		}

		for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
		{
			if (temp_dental_->bToothExist[i])
			{
				temp_dental_->teeth[i].setModelTransparency(_value_transparency);
			}
		}
		emit PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
	}
}

void SeparationManager::setCTMeshModel(MeshModel* _ct_model)
{
	if (_ct_model != nullptr)
	{
		this->ct_mesh_ = _ct_model;
	}
}

void SeparationManager::setJawMeshModel(MeshModel* model)
{
	if (model != nullptr)
	{
		this->jaw_ct_mesh_ = model;
	}
}

void SeparationManager::sortObject(Point3m _camera_pos, vector<MeshModel*> &_model_list)
{
	if (this->temp_dental_ != nullptr)
	{
		// CureInfom removed - sortObject no longer takes CureInfom* parameter
		temp_dental_->sortObject(_camera_pos, _model_list);
	}
	if (this->original_mesh_ != nullptr)
	{
		_model_list.push_back(original_mesh_);
	}
	if (this->ct_mesh_ != nullptr)
	{
		_model_list.push_back(ct_mesh_);
	}
	if (this->jaw_ct_mesh_ != nullptr)
	{
		_model_list.push_back(jaw_ct_mesh_);
	}
}

void SeparationManager::intersectWithSelfToothDepthDiffMode(vector<OrientedBoundingBox>& result)
{

	if (this->temp_dental_ == nullptr)
	{
		return ;
	}

	temp_dental_->collectSortIndexList();
	if (temp_dental_->toothSortIndexList.size() >= 2)
	{
		for (int i = 1; i < temp_dental_->toothSortIndexList.size(); ++i)
		{
			int iToothA = temp_dental_->toothSortIndexList[i - 1];
			int iToothB = temp_dental_->toothSortIndexList[i];
			vector<Point3f> overlapAreaVerts = temp_dental_->teeth[iToothA].getOverlapAreaVertsWithOtherToothSourceMesh(&temp_dental_->teeth[iToothB]);

			if (!overlapAreaVerts.empty())
			{
				OrientedBoundingBox tempOB;
				tempOB.constructOBB_BoundBox(overlapAreaVerts);
				tempOB.getCollideIndexCP(Point2i(iToothA, iToothB));
				result.push_back(tempOB);
			}
		}
	}
	return ;
}

void SeparationManager::optimizeToothSideMeshAndDeform()
{
	if (temp_dental_ == nullptr)
	{
		return;
	}

	std::vector<OrientedBoundingBox> intersectOBBoxes;
	temp_dental_->collectSortIndexList();
	temp_dental_->getFillingbodyCtrlPTsOnEachTooth(temp_dental_->toothSortIndexList);

	intersectWithSelfToothDepthDiffMode(intersectOBBoxes);

	for (auto box : intersectOBBoxes)
	{
		int i_tooth_A, i_tooth_B;
		i_tooth_A = box.collideObjIndexCP.X();
		i_tooth_B = box.collideObjIndexCP.Y();
		bool sameDirect;
		sameDirect = true;

		std::vector<int> aimAdjVertsList;
		std::vector<Point3m> afterAdjVertsPos;

		aimAdjVertsList.clear();
		aimAdjVertsList = box.getAimToothVertsIndexList(
			temp_dental_->teeth[i_tooth_A].p_mesh_, temp_dental_->teeth[i_tooth_A].toothIndex);
		afterAdjVertsPos = box.adjAimVertsByGaussianWay(
			aimAdjVertsList, temp_dental_->teeth[i_tooth_A].p_mesh_, sameDirect);
		std::vector<int>().swap(aimAdjVertsList);

		aimAdjVertsList = box.getAimToothVertsIndexList(
			temp_dental_->teeth[i_tooth_B].p_mesh_, temp_dental_->teeth[i_tooth_B].toothIndex);
		afterAdjVertsPos = box.adjAimVertsByGaussianWay(
			aimAdjVertsList, temp_dental_->teeth[i_tooth_B].p_mesh_, !sameDirect);
		std::vector<int>().swap(aimAdjVertsList);
	}
	std::vector<OrientedBoundingBox>().swap(intersectOBBoxes);
}

void SeparationManager::deleteDental()
{
	if (temp_dental_ == nullptr)
	{
		return;
	}
	cur_picked_tooth_ = nullptr;

	for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (temp_dental_->bToothExist[i])
		{
			emit PSIGNALMANAGER->deleteMeshModelSignal(temp_dental_->teeth[i].p_model_->id());
		}
	}
	delete temp_dental_;
	temp_dental_ = nullptr;
}

void SeparationManager::loadSourceMesh(QString& _file_path, MeshDocument* _md, vcg::Matrix44f _matrix)
{
	if (_file_path.isEmpty())
	{
		return;
	}

	MeshGenerator mesh_maker;
	if (this->compared_source_mesh_)
	{
		CMeshO new_mesh;
		int err;
		int loadmask;
#if PLATFORM_IOS
    string file_name = _file_path.toStdString();
#else
    string file_name = _file_path.toLocal8Bit();
#endif

		err = vcg::tri::io::ImporterSTL<CMeshO>::Open(new_mesh, (file_name).c_str(), loadmask);
		if (err != 0)
		{
			const char* errmsg = vcg::tri::io::ImporterSTL<CMeshO>::ErrorMsg(err);
		}
		new_mesh.Tr = vcg::Matrix44f::Identity();

		mesh_maker.updateModel(&new_mesh);
		vcg::tri::UpdateBounding<CMeshO>::Box(new_mesh);

		vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(compared_source_mesh_->cm, new_mesh);
	}
	else
	{
		MeshModel* new_compare_model = _md->addNewMesh("", COMPAREOBJ_LABEL, false);
		if (this->b_upper_part_)
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
    string file_name = _file_path.toLocal8Bit();
#endif
		err = vcg::tri::io::ImporterSTL<CMeshO>::Open(new_compare_model->cm, (file_name).c_str(), loadmask);
		if (err != 0)
		{
			const char* errmsg = vcg::tri::io::ImporterSTL<CMeshO>::ErrorMsg(err);
		}
		new_compare_model->cm.Tr = _matrix;
		mesh_maker.updateModel(&new_compare_model->cm);
		vcg::tri::UpdateBounding<CMeshO>::Box(new_compare_model->cm);

		this->compared_source_mesh_ = new_compare_model;
	}
}

