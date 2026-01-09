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

#ifndef UTILTOOLS_H
#define UTILTOOLS_H

#define CTRLNODE_SLICE 16
#define CTRLNODE_STACK 16
#define CTRLNODE_RADIUS_L  0.1f
#define CTRLNODE_RADIUS_M  0.07f

#include <QObject>
#include "GL/glew.h"
#include "vcg/space/point3.h"
#include "vcg/space/index/kdtree/kdtree.h"
//#include "util/assistgeometry.h"
#include "util/assist_geometry.h"

#include "common_ext_global.h"
#include "common/config.h"
#include "pointcloud/bilateralfiltercsr.h"

#if PLATFORM_WINDOWS
#include <wrap/gl/addons.h>
#include <wrap/gl/pick.h>

#else

#endif

class MeshModel;
class MLSceneGLSharedDataContext;
class QGLWidget;
class QGLContext;
struct Axis;
class QMainWindow;

class CloudOctree;

class COMMON_EXT_EXPORT UtilityTools : public QObject
{
	Q_OBJECT

public:
	class Garbo
	{
	public:
		~Garbo()
		{
			if (UtilityTools::p_instance_)
				delete UtilityTools::p_instance_;
		}
	};
	static Garbo garbo;
private:
	UtilityTools(QObject* parent = nullptr);
	~UtilityTools();
	static UtilityTools* p_instance_;

public:
	static UtilityTools* getInstance()
	{
		if (p_instance_ == nullptr)
			p_instance_ = new UtilityTools;
		return p_instance_;
	}

	vcg::Point3f getProjPointOnPlane(vcg::Point3f p, vcg::Point3f d, vcg::Point3f pos, vcg::Point3f n);
	vcg::Point3f getProjPointOnPlane(vcg::Point3f p, vcg::Point3f posOnPlane, vcg::Point3f n);

	void getCurrentCameraState(MeshModel* p_mesh_, int width, int height, vcg::Point3f& viewDir, vcg::Point3f& cameraPos);
	void getCurrentCameraState2(MeshModel* p_mesh_, int width, int height, vcg::Point3f& viewDir, vcg::Point3f& rightHandDir, vcg::Point3f& cameraPos);

	vcg::Point3d projectPoint(vcg::Point3f p, MeshModel* p_mesh_ = nullptr);
	// 3d ->2d
	vcg::Point3d unProjectPoint(vcg::Point3f p, MeshModel* p_mesh_ = nullptr);
	vcg::Point3d projectPoint(vcg::Point3f p, CMeshO* p_mesh_);
	vcg::Point3d unProjectPoint(vcg::Point3f p, CMeshO* p_mesh_);
	// 二维屏幕鼠标位置点等转换到3d场景顶点
	vcg::Point3d transformPointFrom2DscreenTo3Dworld(int screen_x, int screen_y, MeshModel* p_mesh_ = nullptr);
	vcg::Point3d transformPointFrom2DscreenTo3Dworld(int screen_x, int screen_y, CMeshO* p_mesh_);

	// 搜索与网格上最近的顶点相匹配，并返回对应的顶点索引
	int getIndexPointClosestMesh(MeshModel& p_mesh, const Point3m& point);
	int getIndexPointClosestMesh(CMeshO* p_mesh, const Point3m& point);
	// 对一堆顶点进行匹配网格上的顶点索引并返回对应的索引容器
	std::vector<int> matchPointsFromMesh(MeshModel* _mesh, const std::vector<vcg::Point3f>& pos_points);

	void updateMeshVertexInfoBuffer(int id, MLSceneGLSharedDataContext* sharedContext, QGLContext* curContext = nullptr);
	void updateMeshVertexInfoBuffer(MeshModel* curMesh, MLSceneGLSharedDataContext* sharedContext, QGLContext* curContext = nullptr);
	void updateMeshVertexInfoBuffer(MeshModel* p_mesh_, MLSceneGLSharedDataContext* sharedContext, QGLWidget* widget);
	void updateMeshTopo(MeshModel* p_mesh_);
	void updateMeshTopo(CMeshO* cMesh);
	// 含有删除多余的顶点后进行法线、拓扑更新
	void updateMeshData(CMeshO* mesh);
	// 含有删除多余的顶点后进行法线、（不包含拓扑更新，提高效率）
	void removeMeshDegenerateVertexAndFaceData(CMeshO* mesh);
	void setMeshOpacity(MeshModel* pmesh, int opacity);

	Point3m getMixedPointFromScreenToWorldOnVerticalPlane(
		float mX, float mY, Point3m origin, Point3m mainV, int width_window, int height_window, CMeshO* p_mesh_);
	Point3m getMixedPointFromScreenToWorld1(
		float mX, float mY, Point3m nV, Point3m hP, Point3m mainV, int width_window, int height_window, CMeshO* p_mesh_);
	Point3m getMixedPointFromScreenToWorld2(float mX, float mY, Point3m nV1, Point3m nV2, Point3m hP, CMeshO* curMesh);
	bool  getPickedFaceIndex(int _mouseX, int _mouseY, vector<CFaceO*>& _vf, CMeshO* curMesh, int& _faceIndex, vcg::Matrix44f* _designative_matrix = nullptr);
	bool  getPickedPointOnMesh(int _mouseX, int _mouseY, vector<CFaceO*>& _vf, CMeshO* curMesh,Point3m& _point, vcg::Matrix44f *_designative_matrix = nullptr);
	bool pickHandle(int x, int y, int& indexPicked,
		const std::vector<Point3m>& loop, float radius,
		bool _cull_face = true, int width = 4, int height = 4, bool sorted = false);
	bool pickHandle(int x, int y, int& indexPicked,
		const std::vector<Point3m>& loop, const std::vector<Point3m>& _judge_vec_loop, const std::vector<bool>& _cullface_loop, float radius,
		bool _cull_face = false, int width = 4, int height = 4, bool sorted = false);
	bool pickHandle2(int x, int y, int& indexPicked,
		const std::vector<Point3m>& loop, float radius,
		int width = 4, int height = 4, bool sorted = false);
	bool pickOnCircle(int x, int y, Point3m pos, Point3m N, float fRadius, Point3m&);
	bool pickConeHandle(int x, int y, int& indexPicked,
		const std::vector<pair<Point3m, Point3m>>& _cps, float _width, Matrix44f _adjMatrix = Matrix44f::Identity(),
		int width = 4, int height = 4, bool sorted = false);
	bool pickCylinder(int x, int y, int& indexPicked,
		const std::vector<FEdge>& _cps, float _width, Matrix44f _adjMatrix = Matrix44f::Identity(),
		int width = 4, int height = 4, bool sorted = false);
	std::vector<int> collectNeighborVerts(std::vector<int> tril, CMeshO* curMesh, int iN);
	bool pointBlocked(Point3m _point, float _deepth_limit = 0);

	std::vector<int> oneRingNeighborhoodVF(int vertIndex, CMeshO* curMesh);
	std::vector<int> oneRingNeighborhoodVV(int vertIndex, CMeshO* curMesh);
	std::vector<int> oneRingNeighborhoodFF(int faceIndex, CMeshO* curMesh);

	std::vector<int> vertNeighborVerts(int index, CMeshO* curMesh, int iN);
	std::vector<int> vertNeighborFaces(int index, CMeshO* curMesh, int iN);

	float findTheAreaOfTriangle(int faceIndex, CMeshO* curMesh);
	Point3m findTheCentroidOfTriangle(int faceIndex, CMeshO* curMesh);
	bool vertOutOfPolygon(Point3m vert, std::vector<Point3m>& polygon);
	bool vertOutOfPolygon(Point2m vert, std::vector<Point2m>& polygon);

	void smoothVert(int ivert, CMeshO* curMesh, float fT);

	// Point3f与Point3m互转
	template<class PointA, class PointB>
	void PointAToPointB(const PointA& pointA, PointB& pointB)
	{
		pointB.X() = pointA.X();
		pointB.Y() = pointA.Y();
		pointB.Z() = pointB.Z();
	}

	void Point3fToPoint3m(const vcg::Point3f& point3f, Point3m& point3m);
	void Point3mToPoint3f(const Point3m& point3m, vcg::Point3f& point3f);

	// 构建坐标轴
	Axis consturctAxis(Point3m center, Point3m hDirect, Axis oldAxis);

	bool  computeCrossPoint(Point3m startP, Point3m endP, Point3m cutFaceNormalV, Point3m cutFacePosP, Point3m& CrossPoint);
	void  computeCrossSegmentByFace(CFaceO* face, Point3m cutFaceNormalV, Point3m cutFacePosP, std::vector<Point3m>& result);
	bool  getCutOutline(CMeshO* mesh, const Point3m& planeNormal, const Point3m& planePos, std::vector<Point3m>& result);
	bool  getCutOutline(CMeshO* mesh, const Point3m& planeNormal, const Point3m& planePos, const std::vector<int> &_face_list, std::vector<Point3m>& result);

	pair<bool, Point3m> intersectTriangle(Point3m& orig, Point3m& dir, CMeshO* mesh, int iFace);
	pair<bool, Point3m> intersectTriangle(Point3m& orig, Point3m& dir, Point3m& v0, Point3m& v1, Point3m& v2);
	pair<bool, Point3m> intersectMesh(Point3m& orig, Point3m& dir, CMeshO* mesh);
	pair<bool, Point3m> intersectMeshAndGetNearestIntersection(Point3m& orig, Point3m& dir, CMeshO* mesh);
	pair<bool, Point3m> intersectCandidateFaces(Point3m& orig, Point3m& dir, CMeshO* mesh, const std::vector<int>& _candidate_faces, int _ignored_index = -1);
	bool pointInTriangle(const Point3m& A, const Point3m& B, const Point3m& C, const Point3m& P);

	vcg::Matrix44f transformMeshByAxis(const Axis& src_axis, const Axis& dest_axis, MeshModel* _mesh);
	vcg::Matrix44f transformMeshByAxis(Axis& src_axis, Axis& dest_axis, MeshModel* _mesh);

	vector<FEdge> getIntersectionRingOfTwoMesh(CMeshO *_mesh_a, CMeshO *_mesh_b, int _istart);
	bool intersectFaceWithMesh(int _iface, CMeshO* _mesh_a, CMeshO* _mesh_b, vcg::KdTree<float> &_kt, FEdge &_intersect_edge);

	QString GetFileVersion(QString fileFullName);

	QString getFileMd5(QString fileName);
	QString getDataMd5(QByteArray& data);

    QMainWindow* getSubMainWindow(QObject* mainWindow);

	void removeFilePath(QString filePath);

	Point3m catmull_Rom_SplinePoint_new(float u, vector<Point3m> subconctrl_P);
	vector<Point3m> catmull_Rom_SplinePoints_new(vector<Point3m> conctrl_P, int Unum = 10);
	vector<Point3m> catmull_Rom_SplinePoints_Equidistant(vector<Point3m> conctrl_P, float _spacing);

	// 对人工智能给出的边界进行筛选排序
	std::vector<vcg::Point3f> sortBorderVertexs(const std::vector<vcg::Point3f>& pos_points);

	//Mesh 相关
	// Mesh hole filling functionality removed - not used in FusionAnalyser

	bool checkToothFeaturePtsMarkFinished();
	bool checkUpperToothFeaturePtsMarkFinished();
	bool checkLowerToothFeaturePtsMarkFinished();
	float getTangentBallRadius(Point3m _v, Point3m _Nv, Point3m _p, Point3m _Np);

	void occlusionColoringTwoMesh(CMeshO* _upper_mesh, CMeshO* _lower_mesh);

	CloudOctree* createMeshOctree(MeshModel* m);
	void unitCorrectionOfDentalModel(CMeshO* _mesh);

	std::string wstring2string(std::wstring wstr);
	std::wstring string2wstring(std::string str);

	/// 工作路径
	QString getWorkingDir();

	QString getCorrectUnicode(const QByteArray& content);
	std::vector<QString> getProjModelFiles(const QString& projFileName);
	QStringList getNetParam(char* param, const char*tag);
	bool    getAppIsLocal();

};
#endif // UTILTOOLS_H
