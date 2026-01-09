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

#ifndef SEGMENTEDSTATUSRECORD_H
#define SEGMENTEDSTATUSRECORD_H
#include <vector>

#include "common/config.h"
// CureInfom removed - treatment functionality not used in FusionAnalyser
#include "data/CutFace.h"
#include "util/VectorAssist.h"

struct COMMON_EXT_EXPORT SliceCutRecord
{
public:
	SliceCutRecord(){}
	SliceCutRecord(int _toothA_Index, int _toothB_Index, float _sliceCutAmount, int _initialStep)
	{
		this->iToothA_Index = _toothA_Index;
		this->iToothB_Index = _toothB_Index;
		this->fSliceCutAmount = _sliceCutAmount;
		this->iInitialStep = _initialStep;
	}
	void initial(int _toothA_Index, int _toothB_Index, float _sliceCutAmount, int _initialStep)
	{
		this->iToothA_Index = _toothA_Index;
		this->iToothB_Index = _toothB_Index;
		this->fSliceCutAmount = _sliceCutAmount;
		this->iInitialStep = _initialStep;
	}
	SliceCutRecord& operator=(const SliceCutRecord &e)
	{
		this->iToothA_Index = e.iToothA_Index;
		this->iToothB_Index = e.iToothB_Index;
		this->fSliceCutAmount = e.fSliceCutAmount;
		this->iInitialStep = e.iInitialStep;

		return *this;
	}
public:
	int iToothA_Index, iToothB_Index;
	float fSliceCutAmount;
	int iInitialStep;
};

struct ToothFDIInfo
{
	QString labelFDI;
	ToothFDIInfo() {}
	ToothFDIInfo(const QString& _labelFDI)
	{
		labelFDI = _labelFDI;
	}
};

enum SegmentDoingStepStatus
{
	SEGMENT_BEGIN,
	SEGMENT_DOING_FDI_MARKING,
	SEGMENT_DOING_SEGMENTING,
	SEGMENT_DOING_ROOT_FILLING
};

struct SegmentedStatusInfo
{
public:
	SegmentDoingStepStatus segmentDoingStatusRecord = SEGMENT_BEGIN;

	bool bSegmentThisMeshDone = false;
	int numToothSegmented = 0;

	std::vector<int> bIsLossTooth;

	std::vector<std::pair<Point3f, Point3f>> posVertexPickedLocated;

	std::vector<ToothFDIInfo> toothFDIInfo;

	std::vector<Axis> axisTooth;
	std::vector<Point2i> root_interval_list_;
	std::vector<vector<Point3m>> width_def_verts_list_;

	std::vector<std::vector<int>> contourVertexIndexTooth;

	std::vector<std::vector<int>> borderVertexIndexEachTooth;

	std::map<QString, CrownInfoSegmentedIntelligent> feature_points_mark;

	CustomPlane* occlusalPlane = nullptr;

	Axis* fixtureCSys = nullptr;

	Axis* dentalInitialCSys = nullptr;

	vector<Point3m> archCtrlNodes;

	bool bitePlaneGenerated = false;
	CustomPlane* bitePlane = NULL;
	float fPlaneWidth, fPlaneDepth;

	vector<int> pullOutList;

	vector<int> fixed_tooth_list_;

	// CureInfom removed - treatment functionality not used in FusionAnalyser

	class AccessoryTool;
	vector<vector<AccessoryTool*>> accInfoListRecord;
	std::vector<NeighboringTeethStateRecord> recover_records_;

	vector<FEdge> tooth_width_edges_upper_, tooth_width_edges_lower_;
	vector<Point3m> cur_length_arch_ctrlnodes_upper_, cur_length_arch_ctrlnodes_lower_;
	vector<Point3m> crowding_arch_5_5_upper_, crowding_arch_7_7_upper_, crowding_arch_5_5_lower_, crowding_arch_7_7_lower_;
	vector<Point3m> spee_ctrlnodes_;
	vector <FEdge> molar_ctrledges_upper_, molar_ctrledges_lower_;
	vector <FEdge> midline_ctrledges_upper_, midline_ctrledges_lower_;
	vector <FEdge> arch_width_ctrledges_upper_, arch_width_ctrledges_lower_;
	vector <FEdge> arch_len_ctrledges_upper_, arch_len_ctrledges_lower_;
	vector <FEdge> basal_bone_arch_ctrledges_upper_, basal_bone_arch_ctrledges_lower_;
	vector <FEdge> gna_height_ctrledges_upper_;
	CutFace* left_overlay_cut_face_ = nullptr, * right_overlay_cut_face_ = nullptr;
	FEdge left_overlay_measure_edge_, right_overlay_measure_edge_;
	float overbite_value_, overjet_value_;
	QString overbite_rank_, overjet_rank_;
	QString patient_name_, doctor_name_;
	QString remark_;
	bool confirm_missing_fdi_ = false;
	bool confirm_tooth_width_ = false;
	bool confirm_crowding_ = false;
	bool confirm_bolton_ = false;
	bool confirm_anterior_parameter_ = false;
	bool confirm_molar_ = false;
	bool confirm_spee_ = false;
	bool confirm_arch_width_ = false;
	bool confirm_arch_length_ = false;
	bool confirm_gnathotectum_height_ = false;
	bool confirm_overlay_ = false;
	bool confirm_basal_bone_arch_ = false;
	vector<QString> browsed_item_prompt_;

	bool have_analyzer_measure_record_ = false;
	bool mark_point_modifyed_ = false;

	vector<FEdge> moveRecords, rotateRecords;
	vector<Point2i> synCPs;

	vector<vector<Point3m>> eachToothConvexVerts;

	vector<vector<Point3m>> eachToothFeatureMarks;

	vector<SliceCutRecord> sliceCutRecords;

	QString orderID;

	QString cbct_model_file_name;
	MeshModel* ct_model_mesh_ = nullptr;

	QString root_model_file_name;

	MeshModel* jaw_bone_mesh_ = nullptr;

	QString jaw_bone_file_name_;

	bool markComplete()
	{
		if (feature_points_mark.size() == 0)
		{
			return false;
		}
		for (auto& data : feature_points_mark)
		{
			int fdi = data.first.toInt();
			if (fdi % 10 == 8)
			{
				continue;
			}
			if (data.second.landmarks.size() < 3)
			{
				return false;
			}
		}
		return true;
	}

	bool judgeHadFeaturePoints()const
	{

		for (auto& it : feature_points_mark)
		{
			if (!it.second.landmarks.empty())
			{
				return true;
			}
		}

		return false;
	}

	SegmentedStatusInfo()
	{
		segmentDoingStatusRecord = SEGMENT_BEGIN;
		bSegmentThisMeshDone = false;
		occlusalPlane = nullptr;
		fixtureCSys = nullptr;
		dentalInitialCSys = nullptr;
		vector<Point3m>().swap(archCtrlNodes);
		bitePlaneGenerated = false;
		bitePlane = NULL;
		// CureInfom removed - treatment functionality not used in FusionAnalyser
		accInfoListRecord.clear();
		pullOutList.clear();
		moveRecords.clear();
		rotateRecords.clear();
		synCPs.clear();
		eachToothConvexVerts.clear();
		sliceCutRecords.clear();
		feature_points_mark.clear();
		orderID = QString("");
		cbct_model_file_name = QString("");
		root_model_file_name = QString("");
		jaw_bone_file_name_ = QString("");
		ct_model_mesh_ = nullptr;
		jaw_bone_mesh_ = nullptr;
		have_analyzer_measure_record_ = false;
	}

	void clearData()
	{
		bSegmentThisMeshDone = false;
		numToothSegmented = 0;
		std::vector<std::pair<Point3f, Point3f>>().swap(posVertexPickedLocated);
		std::vector<int>().swap(bIsLossTooth);
		clearVector(axisTooth);
		std::vector<ToothFDIInfo>().swap(toothFDIInfo);
		pullOutList.clear();
		eachToothConvexVerts.clear();
		feature_points_mark.clear();

		orderID = QString("");

		for (auto& contour : contourVertexIndexTooth)
		{
			std::vector<int>().swap(contour);
		}
		contourVertexIndexTooth.clear();
		for (auto& tooth : borderVertexIndexEachTooth)
		{
			std::vector<int>().swap(tooth);
		}
		borderVertexIndexEachTooth.clear();

		if (bitePlane != NULL)
		{
			bitePlaneGenerated = false;
			delete bitePlane;
			bitePlane = NULL;
		}
		if (occlusalPlane)
		{
			delete occlusalPlane;
			occlusalPlane = nullptr;
		}
		if (fixtureCSys)
		{
			delete fixtureCSys;
			fixtureCSys = nullptr;
		}
		if (dentalInitialCSys)
		{
			delete dentalInitialCSys;
			dentalInitialCSys = nullptr;
		}
		if (!archCtrlNodes.empty())
		{
			vector<Point3m>().swap(archCtrlNodes);
		}
	}
};

#endif

