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

#ifndef COMMON_ABSMESH_H_
#define COMMON_ABSMESH_H_

#define CIRCLE_DIVIDE_NUMBER 360
#define PI_2X_ANGLE 360
#define PI 3.1415926
#define PI_2X  6.283185
#define  STEP_RADS  0.0174532 /**((2*PI)/(CIRCLE_DIVIDE_NUMBER))*/
#define TOOTH_BASE_CTRL_RADIUS 6.0f
#define ACCESSORY_BASE_CTRL_RADIUS 4.0f
#define ZOOM_HANDLE_EXTEND_LEN 1.0f

#define MAX_OVERLAP_DEPTH 0.5f
#define CONE_WIDTH 1.0f
#define ZOOM_CONE_WIDTH 0.2f

#include "common/ml_mesh_type.h"
#include "common_ext/util/assist_geometry.h"
#include "common_ext/machine/booleanmachine.h"
#include "common_ext/util/mesh_bounding_box.h"

#include "common_ext_global.h"

const QString DENTAL_LABEL("dental_label");
const QString TOOTHRD_LABEL("toothrd_label");
const QString REALROOT_LABEL("realroot_label");
const QString REALGINGIVA_LABEL("realgingiva_label");
const QString ACCESSORY_LABEL("accessory_label");
const QString BASALLABEL_LABEL("basallabel_label");
const QString EXPORTABLEMESH_LABEL("exportablemesh_label");
const QString COMPAREOBJ_LABEL("compareobj_label");
const QString EXPORTABLE_ORIGINAL_LABEL("exportable_original_label");

/**AbsMesh:抽象网格类
*
* 保存网格模型基本数据和可能进行的方法需使用的成员变量
* 进行包围盒二叉树构建的方法

*/

class MeshModel;
class MeshDocument;

enum AbsAdjDirect { X_PLUS_PART = 0, X_MINUS_PART, Y_PLUS_PART, Y_MINUS_PART, Z_PLUS_PART, Z_MINUS_PART};
enum AbsAdjMode { ABS_ADJ_NONE, ABS_ADJ_ROTATE, ABS_ADJ_TRANSLATE, ABS_ADJ_ZOOM, TOOTH_LEFT_LATERALSIDE, TOOTH_RIGHT_LATERALSIDE, TOOTH_ROOT, DRAG_FREEDOM_PART };

class COMMON_EXT_EXPORT AbsMesh
{
public:
	AbsMesh();
	AbsMesh(CMeshO *_pmesh, MeshModel* _pmodel);
    AbsMesh(MeshModel *pmodel);
	virtual ~AbsMesh();

    AbsMesh * getParent() const { return p_parent_; }
    void setParent(AbsMesh * val) { p_parent_ = val; }
public:
	CMeshO *p_mesh_ = nullptr;
	MeshModel *p_model_ = nullptr;
	Axis  realTimeAxis;//当前局部坐标系
	Axis localAxis;//源网格局部坐标系
	bool bSelected;//是否被选中，选中才能进行旋转，平移，缩放
	int i_render_ordernum_ = 0;
	HexaVec trans_vec_;//保存当前从local到realtime的变形向量

	vcg::Matrix44f transformMatrix_;//定位变形矩阵
	vcg::Matrix44f *p_father_trans_matrix_ = nullptr;//父渲染矩阵
	vector<AbsMesh*> children_;
	Point3m           moveVector;
	Point3m           rotateVector;

    Point3m           pos_;
    Point3m           scale_;
    Point3m           dir_;

	int *leafIndexList = NULL;
	MeshBoundingBox*     BBTreeRoot = nullptr;//依据网格模型构建的包围盒二叉树。用在牙齿与牙齿，牙颌与附件之间的碰撞检测。
	MeshBoundingBox*     BBLeafList = NULL;//包围盒二叉树叶节点列表。
	MeshBoundingBox*     BBNodeList = NULL;//包围盒二叉树非叶节点列表。

	BooleanMachine machine;

	virtual void sortObject(Point3m _camera_pos, vector<MeshModel*> &_model_list) {}
	virtual void afterChangeModelMatrix() {}
	virtual void setVisible(bool _visible);
	virtual CMeshO *getRealtimeMesh();
	void inital(CMeshO *_mesh);
	void getRootOfBBTree(CMeshO *cmesh = nullptr);
	void bulidLeafBBNode(int FIndex, MeshBoundingBox &BBox, AbsMesh* souce);
	void bulidLeafBBNode(int FIndex, MeshBoundingBox &BBox, CMeshO* souce);
	int** getCollideObjectMatrix(MeshBoundingBox &aimBox, int &rowNum, int &columnNum);
	int** getCollideObjectMatrix(MeshBoundingBox *aimBox, int &rowNum, int &columnNum);
	int** getCollideObjectMatrix(MeshBoundingBox *aimBox, int &rowNum, int &columnNum, vector<int> &waitingFace);
	void getCollideObjectMatrix(MeshBoundingBox *aimBox, vector<Point2m> &result);
	void getCollideObjectMatrixRough(MeshBoundingBox *aimBox, vector<int> &result);
	vector<FEdge> collectEachVertexClosestVert(CMeshO *curMesh, CMeshO *aimMesh);
	void rotateArbitraryLine(double pOut[16], Point3m v1, Point3m v2, float theta);
	void meshDeformer(CMeshO *curMesh, Axis curSys, Axis aimSys);
	void rotateArbitraryAxis(double pOut[16], Point3m axis, float theta);
	void rotateOnePoint(Point3m origionCopy, Point3m axis, float angle, Point3m &P);
	void clearAllLeavesBuffer();
	vector<FEdge> getIntersectCircleLoopTo(AbsMesh model, vector<FFace> &AINB_split);
	vector<FEdge> getIntersectCircleLoopTo(AbsMesh &model, vector<FFace> &AINB_split, CMeshO *usefulmesh);
	vector<vector<FFace>> getSplitResultOnTriangle(int faceIndex);
	vector<FFace> isItTriangleOutofModel(int faceIndex, bool& bOutSide, AbsMesh aimMesh);
	Point3m getProjPointOnPlane(Point3m p, Point3m D, Point3f pos, Point3m N);
	void updateModel(CMeshO *curMesh);
	void transformer(CMeshO *curMesh, Axis curSys, Axis aimSys);
	void zoom(CMeshO *curMesh, Axis curSys, Point3m zoomVector);
	vcg::Matrix44f setZoomDeg(float value, Point3m axis);
	template<typename T> bool exitInVector(T t, const vector<T> &list)
	{
		for (auto temp : list)
		{
			if (t == temp)
			{
				return true;
			}
		}
		return false;
	}

	void addChild(AbsMesh *_node);
	void removeChild(AbsMesh *_node);
	void unbundingFromParent();
	void updateModelRenderingMatrix();
	Axis updateRealTimeAxisSys(vcg::Matrix44f transformMatrix);
	vcg::Matrix44f getTransformMatrix(HexaVec _transHexaVec, Axis _axis);
	vcg::Matrix44f getZoomMatrixAccordZoomVec(const float &_x, const float &_y, const float &_z);
	void setModelTransparency(int _value, vector<int> _invalid_list = {});
	Axis getActualRealtimeAxisNow();
	CMeshO *getLocationMesh();

	bool pickedUp(int _mouseX, int _mouseY, vector<CFaceO*>& _vf, int &_iface);
	void setLocalAxis(Axis _local_axis);
	void setRealTimeAxis(Axis _real_time_axis);
	void setTransformMatrix(Matrix44f _matrix);
	HexaVec computeTransformVectors(Point3m &moveRecord, Point3m &rotateRecord);
	void setAdjustDirect(AbsAdjDirect _direct);
	void setAdjustMode(AbsAdjMode _mode);
	bool pickCtrlSys(int x, int y);
	void drawCtrlSys();
	void paintStraightLine(const Point3f &_v1, const Point3f &_v2, const Point3m &_color, const float &_width);
	void paintCircleLines(const vector<Point3m> &_circle, const Point3m &_color, const float &_width);
	void updateCtrlSys();
	void updateHandles();
	void updateRotateCircle();

	bool bFineTuneing = false;
	Point3m rotateCircleHandle_;
	vector<pair<Point3m, Point3m>> moveCtrlNodesPair_;
	vector<Point3m> zoomCtrlNodes_;
	std::vector<Point3m> circleYOZ, circleZOX, circleXOY;
	Point3m move_vec_, rotate_vec_;/*, zoom_vec_;*/
	float fCtrlRadius_, fBaseCtrlRadius_ = TOOTH_BASE_CTRL_RADIUS;
	Point3m baseLengthVec_;
protected:
    AbsMesh *p_parent_ = nullptr;
	AbsAdjMode cur_adj_mode_ = ABS_ADJ_NONE;
	AbsAdjDirect cur_adj_direct_;
};

#endif // COMMON_ABSMESH_H_

