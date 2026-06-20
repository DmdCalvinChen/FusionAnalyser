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

#ifndef CLOUDKDTREE_H
#define CLOUDKDTREE_H
#define MIN_CONTAIN_NUM 25
#define MIN_SCREEN_RADIUS 1.0f
#define MAX_SCREEN_RADIUS 30.0f
#define MAX_SCREEN_RADIUS_SQUARE 900.0f
#define PERSPECTIVE_STATE false
#define NODE_ELEMENT_LEN 27
#define MIN_BALL_RADIUS 0.1f
#define MAX_BALL_RADIUS 0.14f
#define MAX_POINT_NUM 1500000
#define PRINT_DEBUG_INFO_O false

#include <QObject>
#include <qtimer.h>
#include <qpainter.h>
#include "cloudnode.h"
#include "cloudshader.h"
#include <qcoreapplication.h>
#include "camerastate.h"
#include "bilateralfiltercsr.h"
#include <thread>
#include <future>
#include "common/config.h"
#include "common_ext/common_ext_global.h"

enum ObjectName {DENTAL_UPPER, DENTAL_LOWER, OCCLUSION_CLOUD, IMPLANT, TOOTH, ANYTHING};

class CloudOctree;
class OctreeLeaf;
class OctreeSubTreeLeaf;

class PiecewiseCloud
{
public:
	PiecewiseCloud() {}
	PiecewiseCloud(const vector<CloudNode>& _nodes, CloudOctree* _tree, int _index, float* _m = nullptr);
	~PiecewiseCloud();
	PiecewiseCloud(const PiecewiseCloud& rhs);
	bool modifyMatrix(const std::vector<float>& _m);
	void deletePickedVerts();
	void fusionWith(vector<CloudNode>& _cur_nodes, float* _cur_matrix);
	void fusionProcessing(vector<CloudNode>& _cur_nodes, float* _cur_matrix);
	void fusionAction(CloudNode* _node, float* _matrix, BilateralFilterCSR* _csr, int _s);
	bool isOutlierWith(CloudNode* _node, float* _matrix, BilateralFilterCSR* _csr);
public:
	vector<CloudNode> nodes_;
	float* matrix_ = nullptr;
	CloudOctree* sub_tree_ = nullptr;
	bool picked_ = false;
	int index_;
};

class COMMON_EXT_EXPORT  CloudOctree : QObject
{
	Q_OBJECT
public:
	CloudOctree();
	CloudOctree(CloudOctree *_father);
	CloudOctree(vector<CloudNode> &_nodes, vcg::Box3f &_box, CloudOctree *_father = nullptr);
	CloudOctree(vector<CloudOctree*> _nodes, vector<PiecewiseCloud> _input_record, vcg::Box3f &_box, CloudOctree *_father = nullptr);
	~CloudOctree();

	void setNodeCloud(vector<CloudNode> &_nodes);
	void generateBranch(CloudOctree*& _btree, vector<CloudNode> &_nodes, vcg::Box3f &_box, CloudOctree *_father = nullptr);
	vector<OctreeLeaf> generateLeaf(OctreeLeaf &_node);
	vector<OctreeSubTreeLeaf> generateSubTreeLeaf(OctreeSubTreeLeaf &_node);
	void CheckAndEliminateDuplication(vector<CloudNode> &_nodes);
	void initalLayerBall(vector<CloudNode>& _nodes);
	void initalLayerBall2();
	bool findNearestLeafPoint(int &_ia, float &_aim, vector<CloudNode> &_nodes);
	int findNearestNodeWith(Point3m _pos);
	void allocateVAOVBOSpace();
	void draw(int _w, int _h, QPainter *_painter);
	void drawScreenParts(int _w, int _h, QPainter* _painter);
	void drawAligningFrame(int _w, int _h, QPainter* _painter);
	void drawTreeNode(CloudOctree *_tree, CameraState &_cam, vector<float> &_elements);
	void getBallDepth(HBall *_ball, CameraState &_cam);
	void getBallProjectRadius(HBall *_ball, CameraState &_cam);
	float getBallSectionDistance(HBall* _ball, vcg::Matrix44f &_matrix, Point3m &_pos, Point3m &_normal);
	bool judgeVisible(CloudOctree *_tree, CameraState &_cam);
	bool cullBackFace(HBall *_ball, CameraState &_cam);
	void drawLeafNodes(CloudOctree *_tree, CameraState &_cam, vector<float>& _elements);
	void drawCurrentNode(CloudOctree *_tree, CameraState &_cam, vector<float>& _elements);
	void drawSonNodes(CloudOctree *_tree, CameraState &_cam, vector<float>& _elements);
	void setLeafPointer(int i, CloudOctree* &_leaf);
	void setLeafNodeContainList(vector<CloudNode> _nodes);
	void getSubTreesTogether(vector<CloudOctree*> _subtrees, vector<PiecewiseCloud> _input_records);
	void copyFromTree(CloudOctree* _tree);
	Axis alignModelMatrixTo(CloudOctree *_tree, vcg::Box3f &_box, int _w, int _h);
	Axis constructLocalAxis(CloudOctree *_tree, vcg::Box3f &_box, const Matrix44f &_father_matrix);
	Axis constructAimViewAxis(CloudOctree *_tree, CameraState &_cam, Axis &_temp_axis);
	void setTransformMatrix(float* _m);
	void startMoving();
	void stopMoving();
	bool intersectWithLeafBall(HBall& _ball);
	vector<int> collectIntersectNodes(HBall& _ball, vector<CloudNode>& _nodes);
	vector<int> collectIntersectNodes(HBall &_ball);
	bool pickLeafNodesWhichInside(vector<Point2f>& _loop, vector<CloudNode> &_nodes);
	bool pickLeafNodesOnTree(vector<Point2f> &_loop, vcg::Box2f &_loop_box, vector<CloudNode>& _nodes);
	void unPickTree(vector<CloudNode>& _nodes);
	void setLeafsUseTrueColor(bool _yes);
	static void setColorMode(bool _mode);
	void setDisplayFittingMesh(bool _state);
	void updateWholeTreeData();
	void appendRenderElementsWith(CloudOctree* _tree);
	bool bindVAOVBO();
	bool updateWholeTree();
	int getRenderElementSize();
	CMeshO* getDisplayFittingMesh();
	void updateAxis();
	Matrix44f updateFixedScreenTransformMatrix(int _w, int _h, const Point3m &_view_direct, const Point3m& _right_direct);
	void setTransferHexavec(HexaVec _vec, float _aim_fixed_scale = -1);
	float updateCurScaleLength();
	void clearAllInputData();
	float sectionDistance(vcg::Matrix44f _matrix, Point3m _pos, Point3m _normal);

	inline float curScale() { return cur_scale_; }
	inline void setLocalHexaVec(HexaVec _vec) { this->local_vec_ = _vec; local_matrix_ = this->axis_.constructTransformMatrix(local_vec_);}
	inline Matrix44f localMatrix() { return local_matrix_; }
	inline Point3m ballCenter() { if (ball_ != nullptr) { return ball_->p_; } else { return Point3m(0, 0, 0); } }
	inline float* matrix() { return matrix_; }
	inline HBall* ball() { return ball_; }
	inline void setUpdatingWholeTree(bool _state) { b_updating_whole_tree_ = _state; }
	inline void setAxis(Axis _axis) { axis_ = _axis; }
	inline Axis getAxis() { return axis_; }
	inline ObjectName objectName() { return object_name_; }
	inline void setObjectName(ObjectName _name)
	{
		this->object_name_ = _name;
		for (int i = 0; i < 8; ++i)
		{
			if (this->leafs[i] != nullptr)
			{
				this->leafs[i]->setObjectName(_name);
			}
		}
	}
	inline void setFixedOnScreen(bool _state, float _aim_fixed_scale = -1)
	{
		if (!_state)
		{
			setTransferHexavec(local_vec_, _aim_fixed_scale);
		}
		this->b_fixed_on_screen_ = _state;
		fixed_on_screen_changed_ = true;
	}

public slots:
	void spreadAnimation(int _layer_num);
	void timer1Slot();
	void timer2Slot();
	void timer3Slot();
	void timerTransferSlot();

public:
	static float min_scree_radius;
	static bool true_color_mode_;

	vector<vector<int>> hole_list_;
	vector<CloudNode> node_cloud_;
	vector<CloudOctree*> subtree_list_;
	vector<PiecewiseCloud> input_record_;
	std::future<bool> handler;

	KdTree<float> *kdtree_ = nullptr;
	vector<Point3m> kdtree_points_;

	CMeshO* fitting_mesh_ = nullptr;

private:
	CloudOctree* father_ = nullptr;
	CloudOctree* leafs[8];
	vector<int> contain_;//contain node index list : only true leaf node have
	vector<HBall*> leaf_balls_;//draw leaf node
	HBall* ball_ = nullptr;//judge and draw middle node
	float *matrix_ = nullptr;//son point cloud transform matrix

	HexaVec local_vec_;
	vcg::Matrix44f local_matrix_ = vcg::Matrix44f::Identity();

	HexaVec cur_vec_, transfer_aim_vec_, animation_timer_vec_;
	float animation_aim_scale_ = -1;
	float cur_scale_ = 0.1f, cur_fixed_scale_ = 0.1f, fixed_scale_a_ = 0.1f, fixed_scale_b_ = 0.1f;
	bool b_transfer_scale_ = false;

	Axis axis_;
	ObjectName object_name_ = ObjectName::ANYTHING;

	int layer_num_ = 0;
	int timer_counter_sum_ = 50000;
	int timer_add_step_ = 0;
	int timer1_counter_ = 0, timer2_counter_ = 0, timer3_counter_ = 0, timer_transfer_counter_ = 0;
	QTimer* timer1_ = nullptr, * timer2_ = nullptr, * timer3_ = nullptr, * timer_transfer_ = nullptr;
	bool b_updating_whole_tree_ = false;
	bool b_display_fitting_mesh_ = false;
	bool b_fixed_on_screen_ = false, fixed_on_screen_changed_ = false, b_transfer_timer_slot_ = false;

public:
	int render_elements_size_ = 0;
	vector<pair<pair<int, int>, float*>> update_render_vertices_list_;
	bool b_need_update_record_trees_ = false;
	unsigned int VBO = 0;
	unsigned int VAO = 0;
	int old_length = 0;
	int new_length = 0;
	bool first_time_draw = true;
	int draw_time_ = 0;
};

class OctreeLeaf
{
public:
	OctreeLeaf(CloudOctree* _father, vector<CloudNode> &_nodes, vcg::Box3f _box, int _iNo)
	{
		self_ = new CloudOctree(_father);
		father_ = _father;
		father_->setLeafPointer(_iNo, self_);
		nodes_ = _nodes;
		box_ = _box;
		i_sub_no_ = _iNo;
	}

	void becomeTrueLeafNode()
	{
		self_->setLeafNodeContainList(nodes_);
	}

public:
	CloudOctree* father_ = nullptr, *self_ = nullptr;
	vector<CloudNode> nodes_;
	vcg::Box3f box_;
	int i_sub_no_;
};

class OctreeSubTreeLeaf
{
public:
	OctreeSubTreeLeaf(CloudOctree* _father, vector<CloudOctree*> &_nodes, vcg::Box3f _box, int _iNo)
	{
		self_ = new CloudOctree(_father);
		father_ = _father;
		father_->setLeafPointer(_iNo, self_);
		nodes_ = _nodes;
		box_ = _box;
		i_sub_no_ = _iNo;
	}

	void becomeTrueSubTreeLeafNode()
	{
		if (father_ != nullptr)
		{
			father_->setLeafPointer(i_sub_no_, nodes_[0]);
		}
	}

public:
	CloudOctree* father_ = nullptr, *self_ = nullptr;
	vector<CloudOctree*> nodes_;
	vcg::Box3f box_;
	int i_sub_no_;

};

#endif // !CLOUDKDTREE_H
