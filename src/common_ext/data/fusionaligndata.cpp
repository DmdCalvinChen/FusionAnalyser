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

#include "fusionaligndata.h"
#include <algorithm>
#include "dentalanalysisdata.h"

FusionAlignData::Garbo garbo;
FusionAlignData* FusionAlignData::p_instance_ = nullptr;
FusionAlignData* FusionAlignData::p_cur_proj_data_ = nullptr;

FusionAppData::Garbo appGarbo_;

FusionAppData::FusionAppData(QObject* parent )
{

}

FusionAppData::~FusionAppData()
{

}

eLanguageType FusionAppData::getAppSettingLanguage()
{
	QSettings settings;

	int value = -1;
	if (settings.value(LanguageTypeKey).isValid())
		value = settings.value(LanguageTypeKey).toInt();

	if (value == 1)
	{

		app_language_type_ = E_ENGLISH;
	}
	else if (value == 0)
	{
		app_language_type_ = E_CHINESE;

	}
	else
	{
		settings.setValue("Language", "chinese");

		app_language_type_ = E_CHINESE;
	}

	return app_language_type_;
}
eLanguageType FusionAppData::getAppLanguage()
{

	if (app_language_type_ != E_LAN_NONE)
		return app_language_type_;
}

void FusionAppData::setAppLanguage(eLanguageType type)
{

	if (type != E_CHINESE && type != E_ENGLISH)
		return;

	QSettings settings;
	settings.setValue(LanguageTypeKey, type);
}

FusionAppData* FusionAppData::p_instance_ = nullptr;

FusionAlignData::FusionAlignData(QObject * parent) : QObject(parent)
{
	// Separation functionality removed - not used in FusionAnalyser
	connect(SignalManager::getInstance(), &SignalManager::setCurrentTreatmentStepSignal, this, &FusionAlignData::onSetCurrentTreatmentStepSlot);
}

FusionAlignData::~FusionAlignData()
{
	deleteMemory();
}

void FusionAlignData::deleteMemory()
{
	SAFE_DELETE(upper_dental_);
	SAFE_DELETE(lower_dental_);
	cur_dental_ = nullptr;

	// Separation functionality removed - not used in FusionAnalyser

	SAFE_DELETE(upper_dental_features_);
	SAFE_DELETE(lower_dental_features_ );


	SAFE_DELETE(dental_analysis_);

	emit PSIGNALMANAGER->setViewDirectConsoleVisibleSignal(false);
}

void FusionAlignData::deleteUpperDentalData()
{
	SAFE_DELETE(upper_dental_);
	// Separation functionality removed - not used in FusionAnalyser
	SAFE_DELETE(upper_dental_features_);
	upper_compared_source_mesh_ = nullptr;

	if (dental_analysis_ != nullptr)
	{
		dental_analysis_->initialAnalysisData(upper_dental_features_, lower_dental_features_);
	}
	if (upper_dental_features_ == nullptr && lower_dental_features_ == nullptr)
	{
		SAFE_DELETE(dental_analysis_);
	}
	setUpperManagerSelected(false);
}

void FusionAlignData::deleteLowerDentalData()
{
	SAFE_DELETE(lower_dental_);
	// Separation functionality removed - not used in FusionAnalyser
	SAFE_DELETE(lower_dental_features_);
	lower_compared_source_mesh_ = nullptr;

	if (dental_analysis_ != nullptr)
	{
		dental_analysis_->initialAnalysisData(upper_dental_features_, lower_dental_features_);
	}
	if (upper_dental_features_ == nullptr && lower_dental_features_ == nullptr)
	{
		SAFE_DELETE(dental_analysis_);
	}
	setUpperManagerSelected(true);
}

void FusionAlignData::initialDentalAnalysisDataFeatureMode(SegmentedStatusInfo& _info, bool _mark_points_modified, int _screen_width, int _screen_height, bool _read_record )
{
	QString patient_name_record, doctor_name_record;
	CutFace* left_overlay_cut_face_record = nullptr, *right_overlay_cut_face_record = nullptr;
	FEdge left_overlay_measure_edge_record, right_overlay_measure_edge_record;
	DentalAnalysisData* dental_analysis_record = nullptr;
	if (dental_analysis_ != nullptr)
	{
		patient_name_record = dental_analysis_->patient_name_;
		doctor_name_record = dental_analysis_->doctor_name_;
		if (dental_analysis_->left_overlay_cut_face_)
		{
			left_overlay_cut_face_record = new CutFace(*dental_analysis_->left_overlay_cut_face_);
			left_overlay_measure_edge_record = dental_analysis_->left_overlay_measure_edge_;
		}
		if (dental_analysis_->right_overlay_cut_face_)
		{
			right_overlay_cut_face_record = new CutFace(*dental_analysis_->right_overlay_cut_face_);
			right_overlay_measure_edge_record = dental_analysis_->right_overlay_measure_edge_;
		}
		if (!_read_record && !_mark_points_modified)
		{
			dental_analysis_record = new DentalAnalysisData(*dental_analysis_);
		}

		delete dental_analysis_;
		dental_analysis_ = nullptr;
	}
	dental_analysis_ = new DentalAnalysisData();
	dental_analysis_->setScreenWidthAndHeight(_screen_width, _screen_height);

	if (_read_record)
	{
		dental_analysis_->tooth_width_edges_upper_ = _info.tooth_width_edges_upper_;
		dental_analysis_->tooth_width_edges_lower_ = _info.tooth_width_edges_lower_;
		dental_analysis_->cur_length_arch_ctrlnodes_upper_ = _info.cur_length_arch_ctrlnodes_upper_;
		dental_analysis_->crowding_arch_5_5_upper_ = _info.crowding_arch_5_5_upper_;
		dental_analysis_->crowding_arch_7_7_upper_ = _info.crowding_arch_7_7_upper_;
		dental_analysis_->cur_length_arch_ctrlnodes_lower_ = _info.cur_length_arch_ctrlnodes_lower_;
		dental_analysis_->crowding_arch_5_5_lower_ = _info.crowding_arch_5_5_lower_;
		dental_analysis_->crowding_arch_7_7_lower_ = _info.crowding_arch_7_7_lower_;
		dental_analysis_->spee_ctrlnodes_ = _info.spee_ctrlnodes_;
		dental_analysis_->molar_ctrledges_upper_ = _info.molar_ctrledges_upper_;
		dental_analysis_->molar_ctrledges_lower_ = _info.molar_ctrledges_lower_;
		dental_analysis_->midline_ctrledges_upper_ = _info.midline_ctrledges_upper_;
		dental_analysis_->midline_ctrledges_lower_ = _info.midline_ctrledges_lower_;
		dental_analysis_->arch_width_ctrledges_upper_ = _info.arch_width_ctrledges_upper_;
		dental_analysis_->arch_width_ctrledges_lower_ = _info.arch_width_ctrledges_lower_;
		dental_analysis_->arch_len_ctrledges_upper_ = _info.arch_len_ctrledges_upper_;
		dental_analysis_->arch_len_ctrledges_lower_ = _info.arch_len_ctrledges_lower_;
		dental_analysis_->basal_bone_arch_ctrledges_upper_ = _info.basal_bone_arch_ctrledges_upper_;
		dental_analysis_->basal_bone_arch_ctrledges_lower_ = _info.basal_bone_arch_ctrledges_lower_;
		dental_analysis_->gna_height_ctrledges_upper_ = _info.gna_height_ctrledges_upper_;
		dental_analysis_->left_overlay_cut_face_ = _info.left_overlay_cut_face_;
		dental_analysis_->right_overlay_cut_face_ = _info.right_overlay_cut_face_;
		dental_analysis_->left_overlay_measure_edge_ = _info.left_overlay_measure_edge_;
		dental_analysis_->right_overlay_measure_edge_ = _info.right_overlay_measure_edge_;
		dental_analysis_->overbite_value_ = _info.overbite_value_;
		dental_analysis_->overbite_rank_ = _info.overbite_rank_;
		dental_analysis_->overjet_value_ = _info.overjet_value_;
		dental_analysis_->overjet_rank_ = _info.overjet_rank_;
		dental_analysis_->confirm_missing_fdi_ = _info.confirm_missing_fdi_;
		dental_analysis_->confirm_tooth_width_ = _info.confirm_tooth_width_;
		dental_analysis_->confirm_anterior_parameter_ = _info.confirm_anterior_parameter_;
		dental_analysis_->confirm_arch_length_ = _info.confirm_arch_length_;
		dental_analysis_->confirm_arch_width_ = _info.confirm_arch_width_;
		dental_analysis_->confirm_bolton_ = _info.confirm_bolton_;
		dental_analysis_->confirm_crowding_ = _info.confirm_crowding_;
		dental_analysis_->confirm_gnathotectum_height_ = _info.confirm_gnathotectum_height_;
		dental_analysis_->confirm_basal_bone_arch_ = _info.confirm_basal_bone_arch_;
		dental_analysis_->confirm_overlay_ = _info.confirm_overlay_;
		dental_analysis_->confirm_molar_ = _info.confirm_molar_;
		dental_analysis_->confirm_spee_ = _info.confirm_spee_;
		dental_analysis_->browsed_item_prompt_ = _info.browsed_item_prompt_;
		dental_analysis_->remark_ = _info.remark_;
		dental_analysis_->patient_name_ = _info.patient_name_;
		dental_analysis_->doctor_name_ = _info.doctor_name_;

		SAFE_DELETE(left_overlay_cut_face_record);
		SAFE_DELETE(right_overlay_cut_face_record);
	}
	else
	{
		dental_analysis_->patient_name_ = patient_name_record;
		dental_analysis_->doctor_name_ = doctor_name_record;

		if (!_mark_points_modified)
		{
			if (dental_analysis_record != nullptr)
			{
				*dental_analysis_ = *dental_analysis_record;
				SAFE_DELETE(dental_analysis_record);
			}
			if (left_overlay_cut_face_record != nullptr)
			{
				dental_analysis_->left_overlay_cut_face_ = left_overlay_cut_face_record;
				dental_analysis_->left_overlay_measure_edge_ = left_overlay_measure_edge_record;
			}
			if (right_overlay_cut_face_record != nullptr)
			{
				dental_analysis_->right_overlay_cut_face_ = right_overlay_cut_face_record;
				dental_analysis_->right_overlay_measure_edge_ = right_overlay_measure_edge_record;
			}
		}
		else
		{
			SAFE_DELETE(left_overlay_cut_face_record);
			SAFE_DELETE(right_overlay_cut_face_record);
		}
	}

	dental_analysis_->initialAnalysisData(upper_dental_features_, lower_dental_features_);
	emit PSIGNALMANAGER->updateDentalAnalysisDataReportSignal();
}

void FusionAlignData::initialDentalAnalysisData(SegmentedStatusInfo& _info)
{
	if (dental_analysis_ != nullptr)
	{
		delete dental_analysis_;
		dental_analysis_ = nullptr;
	}

	dental_analysis_ = new DentalAnalysisData();
	dental_analysis_->cur_length_arch_ctrlnodes_upper_ = _info.cur_length_arch_ctrlnodes_upper_;
	dental_analysis_->crowding_arch_5_5_upper_ = _info.crowding_arch_5_5_upper_;
	dental_analysis_->crowding_arch_7_7_upper_ = _info.crowding_arch_7_7_upper_;
	dental_analysis_->cur_length_arch_ctrlnodes_lower_ = _info.cur_length_arch_ctrlnodes_lower_;
	dental_analysis_->crowding_arch_5_5_lower_ = _info.crowding_arch_5_5_lower_;
	dental_analysis_->crowding_arch_7_7_lower_ = _info.crowding_arch_7_7_lower_;
	dental_analysis_->spee_ctrlnodes_ = _info.spee_ctrlnodes_;
	dental_analysis_->molar_ctrledges_upper_ = _info.molar_ctrledges_upper_;
	dental_analysis_->molar_ctrledges_lower_ = _info.molar_ctrledges_lower_;
	dental_analysis_->midline_ctrledges_upper_ = _info.midline_ctrledges_upper_;
	dental_analysis_->midline_ctrledges_lower_ = _info.midline_ctrledges_lower_;
	dental_analysis_->arch_width_ctrledges_upper_ = _info.arch_width_ctrledges_upper_;
	dental_analysis_->arch_width_ctrledges_lower_ = _info.arch_width_ctrledges_lower_;
	dental_analysis_->arch_len_ctrledges_upper_ = _info.arch_len_ctrledges_upper_;
	dental_analysis_->arch_len_ctrledges_lower_ = _info.arch_len_ctrledges_lower_;
	dental_analysis_->overbite_value_ = _info.overbite_value_;
	dental_analysis_->overbite_rank_ = _info.overbite_rank_;
	dental_analysis_->overjet_value_ = _info.overjet_value_;
	dental_analysis_->overjet_rank_ = _info.overjet_rank_;

	dental_analysis_->confirm_missing_fdi_ = _info.confirm_missing_fdi_;
	dental_analysis_->confirm_tooth_width_ = _info.confirm_tooth_width_;
	dental_analysis_->confirm_anterior_parameter_ = _info.confirm_anterior_parameter_;
	dental_analysis_->confirm_arch_length_ = _info.confirm_arch_length_;
	dental_analysis_->confirm_arch_width_ = _info.confirm_arch_width_;
	dental_analysis_->confirm_bolton_ = _info.confirm_bolton_;
	dental_analysis_->confirm_crowding_ = _info.confirm_crowding_;
	dental_analysis_->confirm_gnathotectum_height_ = _info.confirm_gnathotectum_height_;
	dental_analysis_->confirm_basal_bone_arch_ = _info.confirm_basal_bone_arch_;
	dental_analysis_->confirm_overlay_ = _info.confirm_overlay_;
	dental_analysis_->confirm_molar_ = _info.confirm_molar_;
	dental_analysis_->confirm_spee_ = _info.confirm_spee_;
	dental_analysis_->remark_ = _info.remark_;
	dental_analysis_->patient_name_ = _info.patient_name_;
	dental_analysis_->doctor_name_ = _info.doctor_name_;
	dental_analysis_->initialAnalysisData(upperManager(), lowerManager());
	emit PSIGNALMANAGER->updateDentalAnalysisDataReportSignal();
}

void FusionAlignData::createUpperDentalManager()
{
	if (upper_dental_ == nullptr)
	{
		upper_dental_ = new DentalManager;
	}
}

void FusionAlignData::createLowerDentalManager()
{
	if (lower_dental_ == nullptr)
	{
		lower_dental_ = new DentalManager;
	}
}

DentalManager* FusionAlignData::upperManager()
{

	return upper_dental_;
}

DentalManager* FusionAlignData::lowerManager()
{

	return lower_dental_;
}

DentalManager* FusionAlignData::curManager()
{
	return cur_dental_;
}

DentalManager* FusionAlignData::anotherManager()
{
	if (upperManager())
	{
		if (cur_dental_ != upperManager())
		{
			return upperManager();
		}
	}
	if (lowerManager())
	{
		if (cur_dental_ != lowerManager())
		{
			return lowerManager();
		}
	}
	return nullptr;
}

DentalAnalysisData* FusionAlignData::analysisData()
{
	return dental_analysis_;
}

void FusionAlignData::setDentalSummary(int _sum)
{
	this->dental_summary_ = _sum;
}

void FusionAlignData::clearData()
{
	deleteMemory();
	SAFE_DELETE(p_mesh_doc_);
	b_have_occlusal_plane_ = false;
	view_plane_ = CustomPlane();
	export_files_.clear();
}

void FusionAlignData::clearSolutionData()
{

    align_projs_.clear();
    proj_stage_ = E_ProjStageNone;
    export_files_.clear();
}

void FusionAlignData::setUpperManagerSelected(bool _yes)
{
	bool changed_select_jaw = false;

	if (_yes && upper_dental_ != nullptr && cur_dental_ != upper_dental_)
	{
		changed_select_jaw = true;
		cur_dental_ = upper_dental_;
		emit PSIGNALMANAGER->setMeshDocCurrentMesh(cur_dental_->cDental.p_model_->id());
		this->bUpperDentalSelected = true;
	}
	else if (!_yes && lower_dental_ != nullptr && cur_dental_ != lower_dental_)
	{
		changed_select_jaw = true;
		cur_dental_ = lower_dental_;
		emit PSIGNALMANAGER->setMeshDocCurrentMesh(cur_dental_->cDental.p_model_->id());
		this->bUpperDentalSelected = false;
	}

	if (changed_select_jaw)
	{
		emit haveChangedSelectedJaw();
	}
}

void FusionAlignData::setUpperDentalObjectsVisible(bool _visible)
{
	if (upperManager())
	{
		upperManager()->setVisible(_visible);
	}

	if (upper_dental_features_)
	{
		upper_dental_features_->setVisible(_visible);
	}
	if (upper_compared_source_mesh_)
	{
		upper_compared_source_mesh_->setVisible(_visible);
	}
}

void FusionAlignData::setLowerDentalObjectsVisible(bool _visible)
{
	if (lowerManager())
	{
		lowerManager()->setVisible(_visible);
	}

	if (lower_dental_features_)
	{
		lower_dental_features_->setVisible(_visible);
	}
	// Separation functionality removed - not used in FusionAnalyser

	if (lower_compared_source_mesh_)
	{
		lower_compared_source_mesh_->setVisible(_visible);
	}
}

void FusionAlignData::onSetCurrentTreatmentStepSlot(int stage, bool isCompare)
{
	// CureInfom removed - treatment functionality not used in FusionAnalyser
	int i_total_stage_num = 0; // Set to 0 since stage functionality is removed
	assert(i_total_stage_num >= 0);

    if (isCompare && stage >= 0 && stage < i_total_stage_num)
    {
		emit PSIGNALMANAGER->updateTreatmentStepSignal(stage, isCompare);
    }
    else if (stage >= 0 && stage < i_total_stage_num)
    {
		emit PSIGNALMANAGER->updateTreatmentStepSignal(stage, isCompare);
    }
    else
    {
        assert(0);
    }
}

void FusionAlignData::unFocusFramesSlot(int _direction)
{
	if (_direction <= 6)
	{
		if (dental_analysis_ != nullptr)
		{
			this->dental_analysis_->unFocusAllFrames();
		}
	}
}

bool  FusionAlignData::getAlignProjectByName(QString alignProjectName, sAlignProj *&proj)
{

    for (auto &projs : align_projs_)
    {
        for (auto &projItem : projs)
        {
            if (projItem.scheme_name_ == alignProjectName || projItem.file_name_ == alignProjectName)
            {
                proj = &projItem;
                return true;
            }
        }
    }
    return false;
}

sAlignProj& FusionAlignData::getAlignProjectByName(QString alignProjName)
{
    for (auto &projs : align_projs_)
    {
        for (auto &projItem : projs)
        {
            if (projItem.scheme_name_ == alignProjName)
                return  projItem;
        }
    }
}

vector<MeshModel*> FusionAlignData::sortComparableMeshModelPointers(Point3m _camera_pos)
{
	vector<MeshModel*> result;
	if (upperManager() && lowerManager())
	{
		Point3m upper_central_point = upperManager()->cDental.p_mesh_->bbox.Center();
		Point3m lower_central_point = lowerManager()->cDental.p_mesh_->bbox.Center();
		Point3m upper_edge = upper_central_point - _camera_pos;
		Point3m lower_edge = lower_central_point - _camera_pos;
		float distance_u = upper_edge * upper_edge;
		float distance_l = lower_edge * lower_edge;

		if (distance_u <= distance_l)
		{
			// CureInfom removed - sortComparableObject no longer available

			if (lowerManager()->bVisible_)
			{
				lowerManager()->cDental.sortObject(_camera_pos, result);
			}

		}
		else
		{
			if (lowerManager()->bVisible_)
			{
				lowerManager()->cDental.sortObject(_camera_pos, result);
			}

			if (upperManager()->bVisible_)
			{
				upperManager()->cDental.sortObject(_camera_pos, result);
			}

		}
	}
	else
	{
		// CureInfom removed - sortComparableObject no longer available
		if (upperManager() && upperManager()->bVisible_)
		{
			upperManager()->cDental.sortObject(_camera_pos, result);
		}

		if (lowerManager() && lowerManager()->bVisible_)
		{
			lowerManager()->cDental.sortObject(_camera_pos, result);
		}

	}
	return result;
}

vector<MeshModel*> FusionAlignData::sortComparableSourceMeshPointers(Point3m _camera_pos)
{
	vector<MeshModel*> result;
	if (upperManager() && lowerManager())
	{
		Point3m upper_central_point = upperManager()->cDental.p_mesh_->bbox.Center();
		Point3m lower_central_point = lowerManager()->cDental.p_mesh_->bbox.Center();
		Point3m upper_edge = upper_central_point - _camera_pos;
		Point3m lower_edge = lower_central_point - _camera_pos;
		float distance_u = upper_edge * upper_edge;
		float distance_l = lower_edge * lower_edge;

		if (distance_u <= distance_l)
		{
			if (upperManager()->bVisible_ && upperManager()->cDental.compared_source_mesh_)
			{
				result.push_back(upperManager()->cDental.compared_source_mesh_->p_model_);
			}

			if (lowerManager()->bVisible_ && lowerManager()->cDental.compared_source_mesh_)
			{
				result.push_back(lowerManager()->cDental.compared_source_mesh_->p_model_);
			}
		}
		else
		{
			if (lowerManager()->bVisible_ && lowerManager()->cDental.compared_source_mesh_)
			{
				result.push_back(lowerManager()->cDental.compared_source_mesh_->p_model_);
			}

			if (upperManager()->bVisible_ && upperManager()->cDental.compared_source_mesh_)
			{
				result.push_back(upperManager()->cDental.compared_source_mesh_->p_model_);
			}
		}
	}
	else
	{
		if (upperManager() && upperManager()->bVisible_ && upperManager()->cDental.compared_source_mesh_)
		{
			result.push_back(upperManager()->cDental.compared_source_mesh_->p_model_);
		}

		if (lowerManager() && lowerManager()->bVisible_ && lowerManager()->cDental.compared_source_mesh_)
		{
			result.push_back(lowerManager()->cDental.compared_source_mesh_->p_model_);
		}
	}
	return result;
}

vector<MeshModel*> FusionAlignData::sortUpperMeshModelPointers(Point3m _camera_pos)
{
	vector<MeshModel*> result;
	if (upperManager())
	{
		// CureInfom removed - sortObject no longer takes CureInfom* parameter
		upperManager()->cDental.sortObject(_camera_pos, result);
	}
	// Separation functionality removed - not used in FusionAnalyser
	return result;
}

vector<MeshModel*> FusionAlignData::sortLowerMeshModelPointers(Point3m _camera_pos)
{
	vector<MeshModel*> result;
	if (lowerManager())
	{
		// CureInfom removed - sortObject no longer takes CureInfom* parameter
		lowerManager()->cDental.sortObject(_camera_pos, result);

	}
	// Separation functionality removed - not used in FusionAnalyser
	return result;
}

vector<MeshModel*> FusionAlignData::sortMeshModelPointers(Point3m _camera_pos)
{
	vector<MeshModel*> result;
	if (upperManager() && lowerManager())
	{
		Point3m upper_central_point = upperManager()->cDental.p_mesh_->bbox.Center();
		Point3m lower_central_point = lowerManager()->cDental.p_mesh_->bbox.Center();
		Point3m upper_edge = upper_central_point - _camera_pos;
		Point3m lower_edge = lower_central_point - _camera_pos;
		float distance_u = upper_edge * upper_edge;
		float distance_l = lower_edge * lower_edge;

		if (distance_u <= distance_l)
		{
			if (upperManager()->bVisible_)
			{
				// CureInfom removed - sortObject no longer takes CureInfom* parameter
				upperManager()->cDental.sortObject(_camera_pos, result);
			}

			if (lowerManager()->bVisible_)
			{
				lowerManager()->cDental.sortObject(_camera_pos, result);
			}
		}
		else
		{
			if (lowerManager()->bVisible_)
			{
				lowerManager()->cDental.sortObject(_camera_pos, result);
			}

			if (upperManager()->bVisible_)
			{
				upperManager()->cDental.sortObject(_camera_pos, result);
			}
		}
	}
	else
	{
		if (upperManager() && upperManager()->bVisible_)
		{
			upperManager()->cDental.sortObject(_camera_pos, result);
		}
		if (upperManager())
		{
		}

		if (lowerManager() && lowerManager()->bVisible_)
		{
			// CureInfom removed - sortObject no longer takes CureInfom* parameter
			lowerManager()->cDental.sortObject(_camera_pos, result);
		}
	}
	return result;
}

vector<MeshModel*> FusionAlignData::sortMeshModelPointersSegment(Point3m _camera_pos)
{
	// Separation functionality removed - not used in FusionAnalyser
	vector<MeshModel*> result;
	return result;

	// if (p_upper_mesh_ != nullptr && p_lower_mesh_ != nullptr)
	// {
	// 	Point3m upper_central_point = p_upper_mesh_->original_mesh_->cm.bbox.Center();
	// 	Point3m lower_central_point = p_lower_mesh_->original_mesh_->cm.bbox.Center();
	// 	Point3m upper_edge = upper_central_point - _camera_pos;
	// 	Point3m lower_edge = lower_central_point - _camera_pos;
	// 	float distance_u = upper_edge * upper_edge;
	// 	float distance_l = lower_edge * lower_edge;

	// 	if (distance_u <= distance_l)
	// 	{
	// 		if (p_upper_mesh_ ->b_visible_)
	// 		{
	// 			p_upper_mesh_->sortObject(_camera_pos, result);
	// 		}
	// 		if (p_lower_mesh_->b_visible_)
	// 		{
	// 			p_lower_mesh_->sortObject(_camera_pos, result);
	// 		}
	// 	}
	// 	else
	// 	{
	// 		if (p_lower_mesh_->b_visible_)
	// 		{
	// 			p_lower_mesh_->sortObject(_camera_pos, result);
	// 		}
	// 		if (p_upper_mesh_->b_visible_)
	// 		{
	// 			p_upper_mesh_->sortObject(_camera_pos, result);
	// 		}
	// 	}
	// }
	// else
	// {
	// 	if (p_upper_mesh_ != nullptr && p_upper_mesh_->b_visible_)
	// 	{
	// 		p_upper_mesh_->sortObject(_camera_pos, result);
	// 	}

	// 	if (p_lower_mesh_ != nullptr && p_lower_mesh_->b_visible_)
	// 	{
	// 		p_lower_mesh_->sortObject(_camera_pos, result);
	// 	}
	// }
	// return result;
 }

vector<MeshModel*> FusionAlignData::sortComparableSourceMeshPointersSegment(Point3m _camera_pos)
{
	// Separation functionality removed - not used in FusionAnalyser
	vector<MeshModel*> result;
	return result;

	// if (p_upper_mesh_ != nullptr && p_lower_mesh_ != nullptr)
	// {
	// 	Point3m upper_central_point = p_upper_mesh_->original_mesh_->cm.bbox.Center();
	// 	Point3m lower_central_point = p_lower_mesh_->original_mesh_->cm.bbox.Center();
	// 	Point3m upper_edge = upper_central_point - _camera_pos;
	// 	Point3m lower_edge = lower_central_point - _camera_pos;
	// 	float distance_u = upper_edge * upper_edge;
	// 	float distance_l = lower_edge * lower_edge;

	// 	if (distance_u <= distance_l)
	// 	{
	// 		if (p_upper_mesh_->b_visible_ && p_upper_mesh_->compared_source_mesh_)
	// 		{
	// 			result.push_back(p_upper_mesh_->compared_source_mesh_);
	// 		}

	// 		if (p_lower_mesh_->b_visible_ && p_lower_mesh_->compared_source_mesh_)
	// 		{
	// 			result.push_back(p_lower_mesh_->compared_source_mesh_);
	// 		}
	// 	}
	// 	else
	// 	{
	// 		if (p_lower_mesh_->b_visible_ && p_lower_mesh_->compared_source_mesh_)
	// 		{
	// 			result.push_back(p_lower_mesh_->compared_source_mesh_);
	// 		}

	// 		if (p_upper_mesh_->b_visible_ && p_upper_mesh_->compared_source_mesh_)
	// 		{
	// 			result.push_back(p_upper_mesh_->compared_source_mesh_);
	// 		}
	// 	}
	// }
	// else
	// {
	// 	if (p_upper_mesh_ && p_upper_mesh_->b_visible_ && p_upper_mesh_->compared_source_mesh_)
	// 	{
	// 		result.push_back(p_upper_mesh_->compared_source_mesh_);
	// 	}

	// 	if (p_lower_mesh_ && p_lower_mesh_->b_visible_ && p_lower_mesh_->compared_source_mesh_)
	// 	{
	// 		result.push_back(p_lower_mesh_->compared_source_mesh_);
	// 	}
	// }
	// return result;
}

void FusionAlignData::reAutoAlignDetnal(DentalManager* _manager)
{

	// CureInfom removed - updateCurCureInfoAnimation no longer exists
}

void FusionAlignData::setCrowdingAnalysisModeSlot(bool _state)
{
	if ((_state&&(bolton_mode_ == E_BOLOTON_SIX)) || ((!_state)&&(bolton_mode_ == E_BOLOTON_SEVEN)))
	{
		return;
	}
	if (_state)
		bolton_mode_ = E_BOLOTON_SIX;
	else
		bolton_mode_ = E_BOLOTON_SEVEN;
	QSettings settings;
	settings.setValue(BoltonComputeKey, (int)bolton_mode_);

	DentalAnalysisData::setCrowdingAnalysisMode(_state);
	if (this->dental_analysis_ != nullptr)
	{
		dental_analysis_->redoCrowdingAnalysis();
	}
}
