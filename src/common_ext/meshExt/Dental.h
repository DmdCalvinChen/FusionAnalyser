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

#ifndef COMMON_DENTAL_H_
#define COMMON_DENTAL_H_

#define LAYERNUM 10
#define BoundLength 10000
#define Kw 0.7
#define kp 10.0
#define STANDARD_TOOTH_SUM 17

#define VIRTUAL_GINGIVA_BASEPLANE_DEPTH 13.0f
#define USING_MASSPOINT_STRUCT

#include "common/ml_mesh_type.h"

#include "meshExt/AbsMesh.h"
#include "util/mesh_vertex.h"
#include "util/mass_point.h"
#include "util/custom_vector_3d.h"
#include "util/custom_plane.h"
#include "meshExt/Tooth.h"
#include "machine/booleanmachine.h"
// CureInfom removed - treatment functionality not used in FusionAnalyser
#include "data/fusion/segmentedstatusrecord.h"
#include "machine//archlinemachine.h"
#include "machine/convexhellmachine.h"
#include "machine/gjkmachine.h"
// SingleToothTreatmentInfomation removed - treatment functionality not used in FusionAnalyser
#include "util/utility_tools.h"
#include "common_base/util/uitools.h"

#include "common_ext_global.h"

/**Dental:牙颌类
 *
 * 继承自AbsMesh类
 * 保存牙颌类对象需要具备的特殊成员变量
 * 例如单颗牙齿数据数组和与网格变形相关成员变量
 * 提供牙颌分析功能，构建可用的牙颌类对象
 * 提供了网格变形基础方法
 */

class MeshModel;
class MeshDocument;
//class VirtualGingiva;
class COMMON_EXT_EXPORT Dental : public AbsMesh
{
	// 牙齿、牙龈颜色备用
	vcg::Point4f toothClr;        // 牙齿颜色
	vcg::Point4f gingivaClr;      // 牙龈颜色
public:
	enum DentalRenderObject { VIRTUAL_JAW_ARRANGE, REAL_ROOT_JAWBONE, BASAL_LABEL, EXPORTABLE_RESULT};

	Dental();
	~Dental();
	DentalRenderObject render_obj_ = VIRTUAL_JAW_ARRANGE;
	bool bCompareModelVisible_ = false;
	AbsMesh *basalLabel_ = nullptr, *export_result_ = nullptr;
	AbsMesh* compare_obj_ = nullptr;//dental gum's compared object
	CMeshO frozenMesh;
	bool bDentalIsReady = false;
	bool bBasalPlaneIsReady = false;
	bool bArchIsReady = false;
	bool bExportResultReady = false;
	Tooth teeth[STANDARD_TOOTH_SUM];          //牙颌上Tooth类对象，表示牙颌上的各个牙齿

	AbsMesh* original_mesh_ = nullptr;
	AbsMesh* compared_source_mesh_ = nullptr;

	CMeshO tempmesh[STANDARD_TOOTH_SUM];
	bool bToothExist[STANDARD_TOOTH_SUM];
	bool bUpperDental = true;
	MeshVertex   *verList;       //弹簧质点数组。牙颌模型的每个点初始化对应的“质点”，用于牙龈变形。
	int iCurrToothIndex;          //当前正在操作的牙齿的序号。
	int LayerNumber;              //牙颌要分多少层
	int toothIndex[STANDARD_TOOTH_SUM]; //牙颌分割标记
	int iToothNum;                //当前牙颌模型上真实的牙齿个数
	int *Boundary;                //牙颌边界点索引数组
	vector<int> chewinggumFaces;
	vector<int> frozenChewinggumFaces;
	vector<int> gumVertIndexes;
	//vector<int> Boundary;
	int BoundaryLength;
	CustomPlane basePlane;
	float fDentalArchLength = 0;
	vector<Point3m> dentalArch;
	Point3m labelPos, labelAxisX, labelAxisY, labelAxisZ;
	float fWidth = 0, fDepth = 0;
	vector<FFace> baseRing;
	vector<int> toothSortIndexList;

	vector<Point3m> toothCreviceLimit;
	int gingivaVertIndexStart;
	bool convexHullGenerated = false;
	// CureInfom-related functions removed - treatment functionality not used in FusionAnalyser
	void setExportableMeshReady(bool _ready);
	// void sortObject(Point3m _camera_pos, CureInfom* _cur_compared_stage, vector<MeshModel*>& _model_list);
	// void sortComparableObject(Point3m _camera_pos, CureInfom *_cur_compared_stage, vector<MeshModel*> &_model_list);
	// vector<int> getIndexListOfToothOnGum(CureInfom *_info);
	// vector<int> getIndexListOfNormalTooth(CureInfom *_info);
	int getToothIndexAccording(int _i_fdi);
	void afterChangeModelMatrix(){}
	void collectSortIndexList();
	void intialSortToothCreviceLimitList();
	void loadDental(CMeshO *sourceMesh, bool bIsItUpperDental, QString _label_seg_mark, QString _full_path_filename_seg, SegmentedStatusInfo savedInfo, MeshDocument *_md, Dental *_ref_dental);
	void sortArray(int array[], int length);
	void getAdjoingFaceAndPoints();
	void oneRingNeighborhoodVF(int index);
	vector<int> getVertsOneNeighbourhoodVertIndexes(int index);
	void getATooth(Tooth &Teeth, int sign, CMeshO *signedMesh, CMeshO &tempmesh, Axis toothSys, QString strFDI, vector<Point3m> _width_def_verts_list, MeshDocument *_md);
	void layerGumByTooth(int iToothIndex);
	void deleteChewinggumFaces();
	void deleteChewinggumFacesOnFrozenMesh();
	void initalBaseLabelPosition(Point3m pos, Point3m axisX, Point3m axisY, Point3m axisZ);
	void initalDentalWidthAndDepth(float width, float depth);
	void initalBaseSelectedRing();
	void initalBaseSelectedRing(CustomPlane plane);
	// CureInfom-related functions removed - treatment functionality not used in FusionAnalyser
	// collectPassiveCureInfomations - removed due to sonCureInfo dependency
	void makeTeethCatchingEye(vector<int> teethIndexList);
	void makeToothCatchingEye(int teethIndex);
	void makeDentalNotCatchingEye();

	void getTeethNum();
	void analysisExportableOriginalMesh(MeshDocument* _md, QString _fileName);
	void generateExportableOriginalMesh(CMeshO* _mesh);
	int getIntersectedToothIndexByBoundBoxWay(AbsMesh *_obj);
	void loadSourceMesh(QString &_file_path, MeshDocument* _md, vcg::Matrix44f _matrix = vcg::Matrix44f::Identity());

	void freshDentationWithRepairedMesh(CMeshO *repairedMesh);
	void updateTreatmentDragRecords(vector<FEdge> moveRecords, vector<FEdge> rotateRecords);

	void getEachToothSideVerts();

	float UpdateDifferentialVectorSummary();

	void constructDentalArchLine(SegmentedStatusInfo *savedInfo);
	void constructDentalConvexHull(const SegmentedStatusInfo &savedInfo, Dental *_ref_dental);
	void constructEachToothConvexHull(Dental *_ref_dental);
	bool constructOneToothConvexHull(int i);
	void computeEachToothCreviceEdges(const vector<int> &_tooth_list);
	// void saveToothCreviceEdgesRecord(CureInfom *_info);
	void autoGenerateOcclusalPlane();
	// void autoPickOcclusalPlaneSignNode(Point3m &_left_node, Point3m &_middle_node, Point3m &_right_node, CureInfom *_transform_info = nullptr);
	bool getTwoTeethCreviceEdge(int iToothA, int iToothB, FEdge &showEdge);
	int computeColestVertIndexOnToothConvexHull(Point3m v, int iTooth);
	Point3m computeColestVertOnToothConvexHull(Point3m v, int iTooth);
	bool  computeIntersectedToothCPwithAnohterDental(vector<FEdge>& result, Dental* aimDental);
	bool  singleToothCollideWithAnohterDental(int _i_tooth, Dental* aimDental, vector<FEdge>& result);
	bool  singleToothMarkedBallCollideWithAnohterDental(int _i_tooth, Dental* aimDental, vector<FEdge>&result);
	FEdge computeAdjacentToothCreviceEdge(int iToothA, int iToothB);
	vector<Point3m> constructBall(Point3m _center);

	bool constructRigidbodySys(int startToothIndex);
	void constructLinkageRigidbody(int sortListIndex, Rigidbody* active, int step);

	bool insertKeyStageWithSubStage(int iIndex, int &tempNewKeyStageIndex, vector<FEdge> &oldMoveRecords, vector<FEdge> &oldRotateRecords, MeshDocument *_md);
	bool recoverKeyStageWith(int iKeyStage, vector<FEdge> moveRecords, vector<FEdge> rotateRecords);
	vector<FEdge> splitLinkRecordsWith(int iIndex, vector<FEdge> records);
	void clearAllLabelMesh();
	void clearLabelMeshAndContent();
	void initializeCompareObject(CMeshO* _pmesh, MeshModel* _pmodel);
	void restoreFixtureCSYS();

	// CMeshO* updateGingivalMesh(CureInfom *_info = nullptr);
	bool getCtrlPTsOnEachToothNewestWay(int iTooth);
	bool getCtrlPTsOnEachToothNewestWay(const vector<int> &_tooth_list);
	bool getFillingbodyCtrlPTsOnEachTooth(const vector<int> &_tooth_list);
	bool getCutLineCtrlPTsOnEachToothNewestWay(CMeshO *curMesh);
	bool getCutLineCtrlPTsOnEachToothTangentWay(CMeshO *curMesh);
	// void updateComparedModelMatrix(CureInfom *_info);
	// void updateHistoricalComparedModelMatrix(CureInfom* _info);
	void setRenderObject(DentalRenderObject _object);
	void setCompareModelVisible(bool _visible);
	int pickTooth(int _mouseX, int _mouseY, vector<CFaceO*>& _vf, int &_iface);
public:

	void generateProfileSurfaceVertsIndex(vector<int> &transVertList, vector<vector<int>> &frontVertToothMatrix, vector<vector<int>> &backVertToothMatrix);
	void generateTopSurfaceVertsIndex(vector<int> &transVertList, vector<vector<int>> &topVertToothMatrix);

	/*********牙龈网格变形**************/
public:
	int **LNM = NULL;                    //Layer Node Matrix:层级矩阵
	int *tNN = NULL;                     //记录每一层的顶点个数
	void gumDeformation(int iToothLabel, Dental &dental, CMeshO &signedMesh);//依据第iToothlabe个Tooth对象的边界点进行牙龈变形。
	void layeredModel(int* Boundary, int boundSize);
	void getBaseEdjeLength();
	void getBoundaryOfTooth(int iToothIndex, int *&boundary, int &cursorI, CMeshO *signedMesh);
	void getBoundaryOfTooth(Tooth &tooth);
	void getLayeredMatrix();
	void disturbLayer(int layer);
	void destabilizationFeedback(int layer);
	CustomVector3D getMoveVector(int p);
	vector<vector<int>> backVertsIndexesHH;
	///////////////////////////////////////
	void collectGumMassPoints();

	vector<MassPoint> gumMassPointes;
	vector<Point3m> archBezierCtrlNodes2D;//一键排牙目标牙弓曲线控制点集
	Point3m left_node_, middle_node_, right_node_;
public:
	BooleanMachine booleanMachine;

	void transform(vcg::Matrix44f transfMatrix);//依据变形矩阵进行变形
	void traslate(Point3m moveVector);//依据moveVector进行平移
	void rotate(Point3m rotateVector);//依据rotateVector进行旋转
									  //void SaveObject();//保存数据结果

public:
	/*********牙颌治疗信息记录**************/
	// SingleToothTreatmentInfomation removed - treatment functionality not used in FusionAnalyser
	vcg::Matrix44f computeTransformMatrix(int iSign, HexaVec transHexaVec);
	// CureInfom-related functions removed - treatment functionality not used in FusionAnalyser
	// void collectThreeKindsOfAttachmentExistSteps(int iTooth, vector<CureInfom*> &treatments, QString curFDI,
	int getTreatmentSubStageStepSummary();
	void getEachToothDefaultMarkerVert(const vector<int> &_tooth_list);
	bool generateSwelledFrozenMesh(float _height, float _reduction);
	bool deleteSwelledTeeth();
	bool gotFeatureMarks();
	void setToothWidthDefVerts(int _itooth, Point3m _a, Point3m _b, Point3m _c);
	void updateTeethWidthDefVertsList();

	vector<Point2i> synLinkCPRecords;//moveLinkRecord和rotateLinkRecord同步关系记录
	vector<FEdge> moveLinkRecords, rotateLinkRecords;
	vector<vector<Point3m>> teeth_width_def_verts_list_;
	bool bCureModelGenerated = false;
	bool bNeedAutoArrangeAction = false;
	bool bfirstTime = true;
	// CureInfom stage lists removed - treatment functionality not used in FusionAnalyser
	int currentSelectedStage = -1;
	bool bVirtualGingivaGenerated = false;

	vector<SliceCutRecord> sliceCutRecords;
	// SingleToothTreatmentInfomation removed - treatment functionality not used in FusionAnalyser

	QString strOrderID;
	bool bLabelContentReady = false;
	bool bLabelMeshReady = false;
	vector<QString> labelContentList;

	// CureInfom-related functions removed - treatment functionality not used in FusionAnalyser

	void updateFixtureCircle();
	float fFixtureLimitRadius = 45.0f;
	float fHeightOfGumBase = 13.0f;
	Axis fixtureCSYS;
	std::vector<Point3m> circle_Fixture;

public:
	void autoSmoothToEliminateOverlap(int _toothA, int _toothB);
	vector<vector<int>> searchGradationRingWithCenter(Point3m _search_center, int _iTooth, int _ring_num);
	void smoothToothMeshToEliminateOverlap(int _iTooth, vector<vector<int>> &_gradation, float _max_adj_value);
};

#endif // COMMON_DENTAL_H_

