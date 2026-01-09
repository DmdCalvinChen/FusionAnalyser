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

#ifndef COMMON_TOOTH_H_
#define COMMON_TOOTH_H_

#include "meshExt/AbsMesh.h"
#include "util/mesh_vertex.h"
#include "util/custom_bounding_box.h"
#include "util/assist_geometry.h"
#include "util/definite_intersection.h"
#include "util/custom_plane.h"
#include "physics/rigidbody.h"
#include <fstream>
#include <sstream>

#include "common_ext_global.h"

// ToothAdjState and ToothAdjParametersState removed - tooth segmentation functionality not used in FusionAnalyser
// AssociatedGumData removed - gingiva functionality not used in FusionAnalyser

/**Tooth:牙齿类
 *
 * 继承自AbsMesh类
 * 保存了牙齿标号，左右是否缺牙标记等成员变量
 */
class  COMMON_EXT_EXPORT Tooth :
	public AbsMesh
{
public:
	enum TrigerType { XPLUS = 0, XMINS, YPLUS, YMINS, ZPLUS, ZMINS, APLUS, AMINS, BPLUS, BMINS, GPLUS, GMINS };
	enum InputType { INPUT_X = 0, INPUT_Y, INPUT_Z, INPUT_A, INPUT_B, INPUT_G};
	Tooth();
	Tooth(const Tooth &stooth);
	~Tooth();

	AbsMesh *compare_obj_ = nullptr;//tooth's compared object
	AbsMesh* compare_obj2_ = nullptr;//tooth's compared object2
	int iMixedNumberLeft, iMixedNumberRight;
	float fToothWidth = 0;
	float fSliceCutValue = 0;
	vector<int> toothIndex;
	vector<int> toothFaceIndex;
	vector<int> boundaryIndex;
	void InitalTooth(CMeshO *_mesh, int _lable, bool belongToUpperDental, Axis toothSys, QString fdiString, bool _b_filterToothMesh = true);
	void InitalFeatureMarks(vector<Point3m> _marks);
	void InitalToothBoundbox();
	void InitalToothBoundbox(CMeshO *curmesh);
	void getArrangeMarkedPoint();
	void getArrangeMarkedPoint_AnteriorRegion();
	void getArrangeMarkedPoint_PosteriorRegion();
	void getArrangeMarkOnDesireXAxis();
	void getLocationPoints();
	int getDirectionalPeakIndex(Point3m _x, Point3m _y, Point3m _z, bool proj_length = true);
	Point3m getDirectionalPeak(Point3m _x, Point3m _y, Point3m _z, bool proj_length = true);
	void getSpecialOrigins();
	Point3m getLateralOrigin(Point3m direct, Point3m origin, const vector<Point3m>& verts);
	void getVirtualRootOrigin();
	void sortObject(Point3m _camera_pos, bool _b_compared, vector<MeshModel*>& _model_list);
	void sortComparableObject(Point3m _camera_pos, vector<MeshModel*> &_model_list);
	void afterChangeModelMatrix();
	// Tooth adjustment control system
	void updateToothCtrlSys();
	void updateSpecialRotateCircle();
	void updateKeySignVerts(HexaVec _transVec);
	void updateKeySignVerts();
	void updateBoundary(const vcg::Matrix44f &_matrix);
	void updateConvexHull(const vcg::Matrix44f &_matrix);

	void drawSpecialCtrlSys();
	bool pickSpecialCtrlSys(int x, int y);
	void drawAdjLocalAxisCtrlSys();
	void toothAdjLocalAxisAction(int _mouseX, int _mouseY);
	void carryoutLocalAxisTransform(Point3m origin, Axis curAxis);
	void carroutTransform(Point3m origin, Axis curAxis);
	void carryoutTransform(HexaVec _vec);
	void setFineTuneState(bool _state);
	void computeArtificialData(float cur_interval_length, FEdge cur_crevice_edge, Tooth *tooth_a, Tooth *tooth_b);
	void setFixed(bool _state);
	inline bool fixed() { return bFixed_; }

	Point3m lateralOrigin_Left, lateralOrigin_Right, crownOrigin, rootOrigin, source_LateralOrigin_Left, source_LateralOrigin_Right, source_CrownOrigin, source_RootOrigin;
	std::vector<Point3m> circle_LeftSide, circle_RightSide, circle_Root, circle_Crown;

	CustomBoundingBox localBoundbox;
	Point3m desireVertA, desireVertB;
	Point3m width_definition_point_a_, width_definition_point_b_, width_definition_point_c_;
	FEdge desiredXAxis;
	int locationVertA=-1, locationVertB=-1;
	int arrageMarkedVert = -1, localArrageMarkedVert = -1;
	Point3m arrageMarkOnDesireXAxis, arrageMarkOnDesireCurve;
	Axis arrageCoorSys;
	vector<Point3m> front_CtrlPTs, back_CtrlPTs;
	vector<Point3m> front_CutLineCtrlPTs, back_CutLineCtrlPTs;
	Point3m front_CutMark_, back_CutMark_;
	vector<DefiniteIntersection> intersections_;
	vector<Point3m> frozenBoundaryVerts;
	vector<Point3m> realtimeBoundary;
	Point3m Plf, Plb, Prf, Prb;//left-front, left-back, right-front, right-back
	Point3m Plm, Prm;//left-middle, right-middle
	Point3m labialSideOrigin;

	CMeshO *swelledOutMesh_ = nullptr;
	CMeshO *realTimeMesh = nullptr;
	Axis localCrownAxis, realTimeCrownAxis, localRootAxis, realTimeRootAxis;

	vector<Point3m> frozenConvexVerts;
	vector<Point3m> convexVerts;
	vector<FFace> convexFaces;
	Point3m frozenConvexHullCenter, convexHullCenter;

	vector<vector<int>> massnode_LIM;

	void initialWidthDefinitionPoint();
	void setWidthDefinitionPoint(Point3m _a, Point3m _b, Point3m _c);
	void setAtrificial(ArtificialToothRecord *_record);
	void setPulledOut(bool _state);
	bool bArtificial_ = false;
	bool bPulled_out_ = false;
	bool bFixed_ = false;

public:
	int iLabel;
	QString strFDI;//牙齿的解剖学编号
	bool bHasLeftNeighboor, bHasRightNeighboor;
	bool bHavePickedCtrlPTs = false;
	float LipsSurfaceMinAngle = 0, LipsSurfaceMaxAngle = 180.0f;
	bool bPullOut = false;
	float fOverlapDegreeXMinusDirection = 0, fOverlapDegreeXPlusDirection = 0;

	vector<HexaVec> TransHexaVec, TransHexaVec_Root;
	HexaVec TempBeginHexaVec, TempBeginHexaVec_Root, AimHexaVec, AimHexaVec_Root;
	QString displayedX, displayedY, displayedZ, displayedA, displayedB, displayedG;
	QString displayedX_Root, displayedY_Root, displayedZ_Root, displayedA_Root, displayedB_Root, displayedG_Root;
	vector<int> CollideTab;
	Point3m sideVert_L, sideVert_R, curSideVert_L, curSideVert_R, featureVert, curFeatureVert;//牙齿左右侧面标记点
	Point3m differentialVector;
	float fDifferentialVectorLength;
	Point3m tempSpaceVector_Last, tempSpaceVector_Next;
	vector<int> defaultMarkerPointIndexes;

	Rigidbody *rigid = NULL;
	Point2i root_vertindex_interval_;

	vector<Point3m> feature_marks_;
	vector<Point3m> align_marks_;

	vector<Point3m> whitening_boundary_;
	vector<FFace> whitening_accessory_;

	void setRootInterval(Point2i _interval);
	void initializeCompareObject(CMeshO* _pmesh, MeshModel* _pmodel);
	void initializeCompareObject2(CMeshO *_pmesh, MeshModel* _pmodel);
	void updateRigidbody();
	bool isInFrontOfTheTooth(Tooth *otherTooth, Point3m cameraPosition, Point3m viewDirect, bool bCameraFront);
	vector<Point3m> getOverlapAreaVertsWithOtherTooth(Tooth *otherTooth, CMeshO *curMesh);
	vector<Point3m> getOverlapAreaVertsWithOtherToothSourceMesh(Tooth *otherTooth);
	void  depthRealTimeIntersectionTestWith(CMeshO *aimMesh, Axis &viewAxis, vector<pair<int, Point3m>> &result);
	void collectTooth2DProjFaces(CMeshO *curMesh, Tooth *tooth, bool bVisible, Axis virtualEye, Point3m limitOrigin, vector<int> &faceList, vector<int> &vertList, vector<FFace> &proj2DFaces, vector<Point2m> &proj2DVerts,
		Point2i &maxProjVert, Point2i &minProjVert);
	void collectTooth2DProjFaces(CMeshO *curMesh, bool bVisible, Axis virtualEye, Point3m limitOrigin, vector<int> &faceList, vector<int> &vertList, vector<FFace> &proj2DFaces, vector<Point2m> &proj2DVerts,
		Point2i &maxProjVert, Point2i &minProjVert);
	void coloredMapWithDepth(float **map, vector<FFace> *faceList, int areaWidth, int areaHeight);
	void coloredMapWithSingleTriangle(float **map, FFace *face, int areaWidth, int areaHeight);
	bool getABC(int i, int j, Point3m p0, Point3m p1, Point3m p2, double k0, double k1, double k2, double &a, double &b, double &c);

	void updateCurrentSignedVerts(Point3m v);
	void getCurrentSideVerts(CMeshO *curMesh, Axis curAxis);
	void computeToothDifferentialVector(Tooth *toothLast, Tooth *toothNext, CustomPlane curPlane);
	Point3m computeInterdentalSpaceVector(Tooth *tooth, bool bIsLastOne, CustomPlane curPlane);
	void computeEachOtherDiffVector(Tooth *toothOther, bool bIsLastOne, CustomPlane curPlane);
	void computeDefaultMarkerPointIndexes();
	Point3m selectNearestDefaultMarkerPoint(Point3m v);
	void swelledOutToothOn(float _height, float _reduction);
	void deleteSwelledOutTooth();
	CMeshO *getRealtimeMesh();
	vector<std::pair<int, Point2m>> getSwelledAreaVerts(CMeshO *curMesh, Axis &swellAxis, float &maxRadius, float &_reduction);
	void swellOutVerts(vector<pair<int, Point2m>>& pickedVertIndexes, CMeshO* curMesh, Axis& swellAxis, float& maxRadius);
	void swellOutVerts2(vector<pair<int, Point2m>> &pickedVertIndexes, CMeshO *curMesh, Axis &swellAxis, float &_value);
	void generateSwellOutAccessory(Axis &_axis, float& _value);
	float swellOutByGaussianFunc(Point2m pos2D, Point2m center2D, Point2m alpha2D, float A);
	void eliminateOuterCircleVerts(vector<std::pair<int, Point2m>>& _result, CMeshO* _curMesh, Axis& _axis, float &_reduction);
	void generateToothSourceMesh();

	// InitalAimHexaVec removed - unused method
	void updateDisplayedParameters();
	QString getDisplayedX(int Sign, float value);
	QString getDisplayedY(int Sign, float value);
	QString getDisplayedZ(int Sign, float value);
	QString getDisplayedA(int Sign, float value);
	QString getDisplayedB(int Sign, float value);
	QString getDisplayedG(int Sign, float value);
	QString getDisplayedRootX(int Sign, float value);
	QString getDisplayedRootY(int Sign, float value);
	QString getDisplayedRootZ(int Sign, float value);
	QString getDisplayedRootA(int Sign, float value);
	QString getDisplayedRootB(int Sign, float value);
	QString getDisplayedRootG(int Sign, float value);
	void adjToothAimParameter(TrigerType type, float stepValue);
	void adjToothAimParameter(InputType type, float stepValue);
    void adjToothAimParameter(float value, float &parameter);
	float keepNDecimalPlace(int n, float value);
	void getItsCollideTable(Tooth &neighbor, int stageNum);
	void doHexagonTransform(HexaVec V);
	vcg::Matrix44f computeTransformMatrix(HexaVec transHexaVec);
	void getCtrlPTs();
	void getRidOfCrevicePoints3(bool bNotChangeMin, bool bNotChangeMax, float fFrontIndex, float fBackIndex);
	void computeOneOfLipsSurfaceAngles(Tooth &neighborTooth);
	void pickCtrlPTsUniformly(int num);
	void pickCtrlPTsTangentWay(Tooth &neighborTooth, int pickNumLimit, Point3m occlusalPlaneNormalV);
	void pickCtrlPTsLowestCtrlPTWay(int pickNumLimit, vector<Point3m> &gumFrontCtrlPTs, vector<Point3m> &gumBackCtrlPTs,  Point3m occlusalPlaneNormalV);
	FEdge pickOneSideCtrlPTsTangentWay(vector<Point3m> &vertSetA, vector<Point3m> &vertSetB, Axis projAxis);
	void updateRealTimeTooth(vcg::Matrix44f transformMatrix);
	void updateRealTimeSpecialOrigins(vcg::Matrix44f transformMatrix);
	float computeAngle(Point3m e, Point3m axisX, Point3m axisY);
	void adjLinkFrontArea(Tooth &neighborTooth);//削平邻牙控制点集
	void adjLinkBackArea(Tooth &neighborTooth);//削平邻牙控制点集
	void adjToothsLinkedFrontArea(Tooth &changeTooth, Point3m planePos, Point3m planeNormal, vector<int> checkList);
	void adjToothsLinkedBackArea(Tooth &changeTooth, Point3m planePos, Point3m planeNormal, vector<int> checkList);

	void computeAxisFromSourceAxis();
	HexaVec computeTransformVectors_Root(Point3m &moveRecord, Point3m &rotateRecord);

	void freshToothWithRepairedMesh(CMeshO *pToothMesh, const std::vector<int> &indexs, const std::vector<int> &indexsFace, CMeshO *pDentalMesh);
	void freshToothBoundaryOnRepairedMesh(CMeshO *dentalMesh);
	void collectFrozenBoundaryVerts(CMeshO *dentalMesh);

	bool stressTransfer();
	void adjArrageCoorSys();
	void setVisible(bool _visible);
};

#endif // COMMON_TOOTH_H_

