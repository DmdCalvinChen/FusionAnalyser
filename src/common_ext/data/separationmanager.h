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

#ifndef SEPARATIONMANAGER_COMMONEXT_H
#define SEPARATIONMANAGER_COMMONEXT_H

#include <QObject>
#include "common/ml_mesh_type.h"
#include "data/fusion/segmentedstatusrecord.h"
#include "meshExt/Dental.h"
// DentalAnimate removed - files deleted
#include "util/oriented_bounding_box.h"
#include "common_ext_global.h"
#include "common/meshmodel.h"
#include <thread>
#include <future>

enum RenderObjMode {ORIGINAL_MESH, SEPARATION_TEETH, NONE_MODEL};

class COMMON_EXT_EXPORT ToothDataPackage
{
public:
	ToothDataPackage();
	ToothDataPackage(CMeshO *_mesh, int _sign, Axis _axis, QString _fdi, Point2i _root_interval, vector<Point3m> _boundary_verts, vector<Point3m> _marks);
	~ToothDataPackage();
public:
	CMeshO *mesh_;
	int sign_;
	Axis local_axis_;
	QString fdi_;
	int boundary_num_ = 0;
	Point2i root_interval_;
	vector<Point3m> boundary_verts_;
	vector<Point3m> feature_marks_;
};

class  COMMON_EXT_EXPORT SeparationManager
{
public:
	SeparationManager();
	SeparationManager(MeshModel* _source, bool _bUpper);
	~SeparationManager();

	void init(MeshModel* _source, bool _bUpper);
	void distributionTeethResources(vector<ToothDataPackage> _teeth_mesh, CustomPlane *_p_plane, MeshDocument *_md);
	void setRenderMode(RenderObjMode _mode);
	void setVisible(bool _visible);
    bool isVisible() const { return b_visible_; }
	void switchVisible();
	void analysisToothConvexhull(Tooth *_tooth, int _i_end_limit);
	bool pickToothByMouse(int _mouseX, int _mouseY, vector<CFaceO*>& _vf);
	bool pickOnTooth(int _index);
	void setTeethTransparency(int _index, int _value_transparency = 128);

	void updateTeethTransparency(bool _update_all_mesh, int _value_transparency = 128);
	bool updateCurrentPickedToothConvexhull()
	{
		if (this->cur_picked_tooth_ == nullptr)
		{
			return false;
		}
		analysisToothConvexhull(cur_picked_tooth_, cur_picked_tooth_->boundaryIndex.size() * 40);
		temp_dental_->computeEachToothCreviceEdges(this->tooth_index_list);
		return true;
	}
	void updateTeethConvexhull(std::set<int> &_index_list);
	void sortObject(Point3m _camera_pos, vector<MeshModel*> &_result);
	void setCTMeshModel(MeshModel* _ct_model);
	void setJawMeshModel(MeshModel* model);
	void intersectWithSelfToothDepthDiffMode(std::vector<OrientedBoundingBox>& result);
	void optimizeToothSideMeshAndDeform();
	void deleteDental();
	void loadSourceMesh(QString& _file_path, MeshDocument* _md, vcg::Matrix44f _matrix = vcg::Matrix44f::Identity());

public:
	int index_tooth_picked_ = -1;
	int alpha_transparency_mesh_ = 128;
	bool b_visible_ = true;
	bool b_upper_part_;
	MeshModel* original_mesh_ = nullptr;
	MeshModel* ct_mesh_ = nullptr;
	MeshModel* jaw_ct_mesh_ = nullptr;
	MeshModel* compared_source_mesh_ = nullptr;
	Dental *temp_dental_ = nullptr;
	vector<int> tooth_index_list;
	Tooth* cur_picked_tooth_ = nullptr;
	RenderObjMode render_mode_ = ORIGINAL_MESH;
};

#endif

