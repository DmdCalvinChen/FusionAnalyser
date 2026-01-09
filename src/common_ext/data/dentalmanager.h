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

#ifndef DENTALMANAGER_COMMONEXT_H
#define DENTALMANAGER_COMMONEXT_H

#include "common/ml_mesh_type.h"
#include "data/fusion/segmentedstatusrecord.h"
#include "meshExt/Dental.h"
// DentalAnimate removed - files deleted
#include "util/oriented_bounding_box.h"
#include "common_ext_global.h"
#include <thread>
#include <future>
#include "TrackBallTool/customtrackball.h"
#include "TrackBallTool/customadjustinghandle.h"
#include "TrackBallTool/translatehandle.h"
#include "TrackBallTool/rotatehandle.h"
#include "TrackBallTool/ZoomHandle.h"
#include "TrackBallTool/dragmovehandle.h"
#include "autoAlign/toothmodelmanager.h"

enum ToothTdjustMode { ADJ_TOOTH_MATRIX = 0, ADJ_TOOTH_AXIS_SYS, ADJ_TOOTH_LEFT_LATERALSIDE, ADJ_TOOTH_RIGHT_LATERALSIDE, ADJ_TOOTH_ROOT };

class  COMMON_EXT_EXPORT DentalManager
{
public:
	DentalManager();
	~DentalManager();

	void GetDental(CMeshO *_cm, bool _bUpperDental, QString _label_seg_mark, QString _full_path_filename_seg, const SegmentedStatusInfo &_segmentedStatusInfo, MeshDocument *_md, Dental *_ref_dental = nullptr);
	vector<OrientedBoundingBox> intersectWithSelfTooth();
	vector<OrientedBoundingBox> intersectWithSelfToothDepthDiffMode();
	void updateModel(CMeshO *curMesh);
	// CureInfom-related functions removed - treatment functionality not used in FusionAnalyser
	void updateComparedDentalState(int _istep);
	void updateHistoricalComparedDentalState(int _istep);
	void setVisible(bool _visible);
	bool isVisible();
	bool pickTooth(int _mouseX, int _mouseY, vector<CFaceO*>& _vf);
	bool pickTooth(int _mouseX, int _mouseY, vector<CFaceO*>& _vf, int &pickedToothIndex, Point3m &pickedPos, Point3m &pickedFaceNormal, bool bDefaultPosEnable = false);
	bool pickOnTooth(int _index);
	bool haveSelectedTooth();
	void updateAfterAdjustSingleTooth(int _itooth);
	void updateAfterAdjustSomeTeeth(const vector<int> &_itoothlist);
	// CureInfom-related functions removed - treatment functionality not used in FusionAnalyser
	void deleteTrackBall();
	// Functions removed - implementation deleted from dentalmanager.cpp
	void assembleBasalLabelTrackball(AbsMesh* _obj);
	void assembleDentalTrackball(AbsMesh* _obj, float _radius);
	void artificialPickedTooth();
	void pulledOutPickedTooth();
	// CureInfom-related functions removed - treatment functionality not used in FusionAnalyser
	NeighboringTeethStateRecord collectNeighborTeethState();
	vector<pair<int, HexaVec>> getNeighborTeethVec(int _tooth_index, int _range = 2);
	void recoverPullOutTooth(int _index);
	void recoverPullOutToothWithRecord(NeighboringTeethStateRecord &_record);
	void saveTeethAsToothModel();
	void addToothModel(int _iFDI, MeshDocument *_md);
	bool haveSelectedInitialState();
	void switchJawBoneVisible();
	void switchRealGingivaVisible();
	void setRealGingivaVisible(bool _visible);
	void switchOverlapCompare(bool _visible);
	void setAnimationLoadComplete(bool _state);
	void setToothFixed(int _index);
	void unfixSeletedFixedTooth();
	bool pickFixedToothAction(int _mouseX, int _mouseY);

	bool bCompareModelVisible_ = false;
	bool bGotDental = false;
	bool bVisible_ = true;
	bool bAnimationLoadComplete_ = false;
	Dental cDental;
	int asyncMonitor = 0x00000000;
	// CureInfom-related members removed - treatment functionality not used in FusionAnalyser
	vector<std::pair<CMeshO*, int>> keyFrameDentalMeshes, keyFrameRealrootsMeshes, keyHistoricalFrameDentalMeshes;
	Tooth* curPickedTooth_ = nullptr;
	int iCurrentSelectTooth_ = -1;
	std::future<bool> gum_update_handle;
	CustomTrackball *cus_track_ball_ = nullptr;
	vector<NeighboringTeethStateRecord> neigh_state_;
	vector<Point3m> locaPoints;
	vector<int> fixed_tooth_list_;
	int picked_fixed_tooth_index_ = -1;
};

#endif
