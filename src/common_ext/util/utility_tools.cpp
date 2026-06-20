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

#include "utility_tools.h"
#include "pointCloud/cloudkdtree.h"

#include <QHostInfo>
#include "common/meshmodel.h"
#include <QMainWindow>
#include <QFileDialog>
#include <QNetworkInterface>
#include <QUrl>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>

#if PLATFORM_WINDOWS
#include <windows.h>
#include <DXGI.h>
#else
#endif

#include "data/fusionaligndata.h"
#include "common_base/SignalManager.h"
using namespace vcg;

UtilityTools::Garbo  UtilityTools::garbo;
UtilityTools* UtilityTools::p_instance_ = nullptr;

UtilityTools::UtilityTools(QObject* parent)
	: QObject(parent)
{
}

UtilityTools::~UtilityTools()
{
}

vcg::Point3f UtilityTools::getProjPointOnPlane(vcg::Point3f p, vcg::Point3f posOnPlane, vcg::Point3f n)
{
	Point3f result;
	n = n.Normalize();
	float up(0.0), down(0.0);
	up = (posOnPlane - p) * n;
	down = -n * n;
	float t = up / down;

	result = p - n * t;
	return result;
}
vcg::Point3f UtilityTools::getProjPointOnPlane(vcg::Point3f p, vcg::Point3f d, vcg::Point3f pos, vcg::Point3f n)
{
	Point3f result;
	float up(0.0), down(0.0);
	up = (pos - p) * n;
	down = d * n;
	float t = up / down;

	result = p + d * t;
	return result;
}

void UtilityTools::getCurrentCameraState(MeshModel* p_mesh_, int width, int height, vcg::Point3f& viewDir, vcg::Point3f& cameraPos)
{
	Point3d nearPos = projectPoint(Point3f(width / 2.0f, height / 2.0f, 0), p_mesh_);
	Point3d farPos = projectPoint(Point3f(width / 2.0f, height / 2.0f, 1), p_mesh_);
	Point3d dir = farPos - nearPos;

	viewDir = Point3f(dir.X(), dir.Y(), dir.Z()).normalized();
	cameraPos = Point3f(nearPos.X(), nearPos.Y(), nearPos.Z());
}

void UtilityTools::getCurrentCameraState2(MeshModel* p_mesh_, int width, int height, vcg::Point3f& viewDir, vcg::Point3f& rightHandDir, vcg::Point3f& cameraPos)
{
	Point3d nearPos = projectPoint(Point3f(width / 2.0f, height / 2.0f, 0), p_mesh_);
	Point3d farPos = projectPoint(Point3f(width / 2.0f, height / 2.0f, 1), p_mesh_);
	Point3d rightPos = projectPoint(Point3f(width, height / 2.0f, 0), p_mesh_);
	Point3d dir = farPos - nearPos;
	Point3d rightDir = rightPos - nearPos;

	viewDir = Point3f(dir.X(), dir.Y(), dir.Z()).normalized();
	rightHandDir = Point3f(rightDir.X(), rightDir.Y(), rightDir.Z()).Normalize();
	cameraPos = Point3f(nearPos.X(), nearPos.Y(), nearPos.Z());
}

void UtilityTools::updateMeshVertexInfoBuffer(MeshModel* p_mesh_, MLSceneGLSharedDataContext* sharedContext, QGLWidget* widget)
{
	updateMeshVertexInfoBuffer(p_mesh_, sharedContext, widget->context());
	widget->update();
}
void UtilityTools::updateMeshVertexInfoBuffer(MeshModel* curMesh, MLSceneGLSharedDataContext* sharedContext, QGLContext* curContext)
{
	if (curMesh == nullptr) return;
	updateMeshVertexInfoBuffer(curMesh->id(), sharedContext, curContext);
}

void UtilityTools::updateMeshVertexInfoBuffer(int id, MLSceneGLSharedDataContext* sharedContext, QGLContext* curContex)
{
	if (sharedContext != nullptr)
	{
		MLRenderingData::RendAtts atts;
		MLRenderingData dt;
		if (curContex)
		{
			sharedContext->getRenderInfoPerMeshView(id, curContex, dt);
		}

		atts[MLRenderingData::ATT_NAMES::ATT_VERTPOSITION] = true;
		atts[MLRenderingData::ATT_NAMES::ATT_VERTNORMAL] = true;
		atts[MLRenderingData::ATT_NAMES::ATT_FACENORMAL] = true;
		atts[MLRenderingData::ATT_NAMES::ATT_VERTCOLOR] = true;

		dt.set(MLRenderingData::PR_SOLID, atts);
		if (curContex)
		{
			sharedContext->setRenderingDataPerMeshView(id, curContex, dt);
		}
		if (!curContex)
		{
			sharedContext->meshAttributesUpdated(id, false, atts);
			sharedContext->manageBuffers(id);
		}
	}
}

void UtilityTools::updateMeshTopo(MeshModel* p_mesh_)
{
	if (p_mesh_ == nullptr)
		return;
	updateMeshTopo(&p_mesh_->cm);
}

void UtilityTools::updateMeshTopo(CMeshO* curMesh)
{
	if (curMesh == nullptr)
	{
		return;
	}

	vcg::tri::Allocator<CMeshO>::CompactEveryVector(*curMesh);
	//update topology
	//assert(tri::HasFFAdjacency(*curMesh) == false);
	if (!tri::HasFFAdjacency(*curMesh))
		curMesh->face.EnableFFAdjacency();
	assert(tri::HasFFAdjacency(*curMesh) == true);

	//assert(tri::HasVFAdjacency(*curMesh) == false);
	if (!tri::HasVFAdjacency(*curMesh))
	{
		curMesh->vert.EnableVFAdjacency();
		curMesh->face.EnableVFAdjacency();
	}
	assert(tri::HasVFAdjacency(*curMesh) == true);
	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*curMesh);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(*curMesh);

	// update Normals
	vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(*curMesh);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*curMesh);
}

void UtilityTools::updateMeshData(CMeshO* mesh)
{
	if (mesh == nullptr)
	{
		return;
	}
	mesh->face.EnableFFAdjacency();
	mesh->vert.EnableVFAdjacency();
	mesh->face.EnableVFAdjacency();

	int delVertNum = vcg::tri::Clean<CMeshO>::RemoveDegenerateVertex(*mesh);
	int delFaceNum = vcg::tri::Clean<CMeshO>::RemoveDegenerateFace(*mesh);
	tri::Clean<CMeshO>::RemoveDuplicateVertex(*mesh);// delete Redundant vertex data
	tri::Clean<CMeshO>::RemoveUnreferencedVertex(*mesh);

	vcg::tri::Allocator<CMeshO>::CompactFaceVector(*mesh);
	vcg::tri::Allocator<CMeshO>::CompactVertexVector(*mesh);
	mesh->vert.EnableVFAdjacency();
	mesh->face.EnableVFAdjacency();

	// update topology
	assert(tri::HasPerVertexVFAdjacency(*mesh) && tri::HasPerFaceVFAdjacency(*mesh));
	vcg::tri::UpdateTopology<CMeshO>::VertexFace(*mesh);
	vcg::tri::UpdateTopology<CMeshO>::FaceFace(*mesh);

	// update Normals
	tri::UpdateNormal<CMeshO>::PerVertexNormalized(*mesh);
	tri::UpdateNormal<CMeshO>::PerFaceNormalized(*mesh);

}

void UtilityTools::removeMeshDegenerateVertexAndFaceData(CMeshO* mesh)
{
	if (mesh == nullptr)
	{
		return;
	}

	int delVertNum = vcg::tri::Clean<CMeshO>::RemoveDegenerateVertex(*mesh);
	int delFaceNum = vcg::tri::Clean<CMeshO>::RemoveDegenerateFace(*mesh);
	tri::Clean<CMeshO>::RemoveDuplicateVertex(*mesh);// delete Redundant vertex data
	tri::Clean<CMeshO>::RemoveUnreferencedVertex(*mesh);

	vcg::tri::Allocator<CMeshO>::CompactFaceVector(*mesh);
	vcg::tri::Allocator<CMeshO>::CompactVertexVector(*mesh);
}

void UtilityTools::setMeshOpacity(MeshModel* pmesh, int opacity)
{
	if (pmesh)
	{
		opacity = opacity < 0 ? 0 : opacity;
		opacity = opacity > 255 ? 255 : opacity;
		for (auto& v : pmesh->cm.vert)
		{
			v.C().W() = opacity;
		}
	}
}

void UtilityTools::Point3fToPoint3m(const vcg::Point3f& point3f, Point3m& point3m)
{
	PointAToPointB<vcg::Point3f, Point3m>(point3f, point3m);
}

void UtilityTools::Point3mToPoint3f(const Point3m& point3m, vcg::Point3f& point3f)
{
	PointAToPointB<Point3m, vcg::Point3f>(point3m, point3f);
}

Axis UtilityTools::consturctAxis(Point3m center, Point3m hDirect, Axis oldAxis)
{
	Point3m axisX, axisY, axisZ;
	Point3m oldAxisX = oldAxis.axisXVector;
	Point3m oldAxisY = oldAxis.axisYVector;
	Point3m oldAxisZ = oldAxis.axisZVector;

	Point3m axisRotate = (oldAxisY ^ hDirect).Normalize();
	float cosValue = oldAxisY * hDirect;
	if (cosValue - 0.9999f > 0.00001f)
	{
		cosValue = 0.9998f;
	}
	if (cosValue + 0.9999f < 0.00001f)
	{
		cosValue = -0.9998f;
	}

	float angle = (acosf(cosValue) * 180.0f) / PI;

	vcg::Matrix44f rotateMatrix;
	rotateMatrix.SetRotateDeg(angle, axisRotate);

	axisX = rotateMatrix * oldAxisX;
	axisY = rotateMatrix * oldAxisY;
	axisZ = rotateMatrix * oldAxisZ;

	return Axis(center, axisX, axisY, axisZ);
}

bool UtilityTools::computeCrossPoint(Point3m startP, Point3m endP, Point3m cutFaceNormalV, Point3m cutFacePosP, Point3m& CrossPoint)
{
	float fPointMulit;
	cutFaceNormalV.Normalize();
	Point3m sideVector1, sideVector2;
	sideVector1 = startP - cutFacePosP;
	sideVector2 = endP - cutFacePosP;
	if ((sideVector1 * cutFaceNormalV) * (sideVector2 * cutFaceNormalV) > 0) //两点在平面的同一侧
		return false;

	Point3m cs, se;
	cs = startP - cutFacePosP;
	se = endP - startP;
	float base = se * cutFaceNormalV;
	if (base == 0)
		return false;

	float t = -(cs * cutFaceNormalV) / base;
	if (0 <= t && t <= 1)
	{
		CrossPoint = startP + se * t;
		return true;
	}
	return false;
}

void UtilityTools::computeCrossSegmentByFace(CFaceO* face, Point3m cutFaceNormalV, Point3m cutFacePosP, std::vector<Point3m>& result)
{

	auto notExistIn = [=](const Point3m& point, const std::vector<Point3m>& points) -> bool {
		if (std::find(points.begin(), points.end(), point) != points.end())
		{
			return false;
		}
		return true;
	};
	Point3m p1, p2, p3;
	bool bCrossed1, bCrossed2, bCrossed3;
	Point3m tempStartP, tempEndP;
	tempStartP = face->V(0)->P();
	tempEndP = face->V(1)->P();
	bCrossed1 = computeCrossPoint(tempStartP, tempEndP, cutFaceNormalV, cutFacePosP, p1);
	tempStartP = face->V(1)->P();
	tempEndP = face->V(2)->P();
	bCrossed2 = computeCrossPoint(tempStartP, tempEndP, cutFaceNormalV, cutFacePosP, p2);
	tempStartP = face->V(2)->P();
	tempEndP = face->V(0)->P();
	bCrossed3 = computeCrossPoint(tempStartP, tempEndP, cutFaceNormalV, cutFacePosP, p3);

	if (bCrossed1 && notExistIn(p1, result))
	{
		result.push_back(p1);
	}
	if (bCrossed2 && notExistIn(p2, result))
	{
		result.push_back(p2);
	}
	if (bCrossed3 && notExistIn(p3, result))
	{
		result.push_back(p3);
	}
	return;
}

bool UtilityTools::getCutOutline(CMeshO* mesh, const Point3m& planeNormal, const Point3m& planePos, std::vector<Point3m>& result)
{
	if (mesh == nullptr)
	{
		return false;
	}

	for (int i = 0; i < mesh->face.size(); i++)
	{
		computeCrossSegmentByFace(&mesh->face[i], planeNormal, planePos, result);
	}
	if (result.size() > 1)
	{
		return true;
	}

	return false;
}

bool UtilityTools::getCutOutline(CMeshO* mesh, const Point3m& planeNormal, const Point3m& planePos, const std::vector<int>& _face_list, std::vector<Point3m>& result)
{
	if (mesh == nullptr)
	{
		return false;
	}

	for (auto &i : _face_list)
	{
		computeCrossSegmentByFace(&mesh->face[i], planeNormal, planePos, result);
	}
	if (result.size() > 1)
	{
		return true;
	}

	return false;
}

vcg::Point3d UtilityTools::transformPointFrom2DscreenTo3Dworld(int screen_x, int screen_y, MeshModel* p_mesh_ /* = nullptr */)
{
	CMeshO* pCmesh = nullptr;
	if (p_mesh_)
	{
		pCmesh = &p_mesh_->cm;
	}

	return transformPointFrom2DscreenTo3Dworld(screen_x, screen_y, pCmesh);
}

vcg::Point3d UtilityTools::transformPointFrom2DscreenTo3Dworld(int screen_x, int screen_y, CMeshO* p_mesh_)
{
	// Get gl state values
	double mm[16], mp[16];
	GLint vp[4];
	glPushMatrix();
	if (p_mesh_)
	{
		glMultMatrix(p_mesh_->Tr);
	}

	glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	glGetDoublev(GL_PROJECTION_MATRIX, mp);
	glGetIntegerv(GL_VIEWPORT, vp);

	float z = 1;
	glReadPixels(screen_x, screen_y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
	Point3d result(0, 0, 0);
	gluUnProject(screen_x, screen_y, z, mm, mp, vp, &result[0], &result[1], &result[2]);
	glPopMatrix();

	return result;
}

int UtilityTools::getIndexPointClosestMesh(CMeshO* _p_mesh, const Point3m& point)
{
	if (_p_mesh == nullptr)
	{
		return -1;
	}

	static CMeshO* _mesh = _p_mesh; // 对已搜索的网格进行记录优化搜素
	static int num_vertex = _p_mesh->vert.size();

	static KdTree<float>* kt = new KdTree<float>(VertexConstDataWrapper<CMeshO>(*_p_mesh));

	if (_mesh != _p_mesh || num_vertex != _p_mesh->vert.size())
	{
		_mesh = _p_mesh;// 重新指向新的网格
		num_vertex = _p_mesh->vert.size();// 重新记录新的网格mesh

		if (kt)
		{
			delete kt;
		}
		kt = new KdTree<float>(VertexConstDataWrapper<CMeshO>(*_p_mesh));
	}

	typename KdTree<float>::PriorityQueue pq;
	int neighbor = 2;

	if (kt == nullptr)
	{
		return -1;
	}

	kt->doQueryK(point, neighbor, pq);

	float minDis = 0.1f;
	int nearstVId = -1;
	for (int i = 0; i < neighbor; i++)
	{
		int neighbourId = pq.getIndex(i);
		float pointSquareDist = vcg::SquaredNorm(_p_mesh->vert[neighbourId].cP() - point);
		if (pointSquareDist < minDis)
		{
			nearstVId = neighbourId;
			minDis = pointSquareDist;
		}
	}
	if (nearstVId < 0 || nearstVId >= _p_mesh->vert.size())
	{
		return-1;
	}
	else
	{
		return nearstVId;
	}
}

int UtilityTools::getIndexPointClosestMesh(MeshModel& p_mesh, const Point3m& point)
{
	if (&p_mesh == nullptr)
	{
		return -1;
	}

	static MeshModel* _mesh = &p_mesh; // 对已搜索的网格进行记录优化搜素

	static int num_vertex = _mesh->cm.vert.size();

	static KdTree<float>* kt = new KdTree<float>(VertexConstDataWrapper<CMeshO>(_mesh->cm));

	if (_mesh != &p_mesh || num_vertex != p_mesh.cm.vert.size())
	{
		_mesh = &p_mesh;// 重新指向新的网格
		num_vertex = _mesh->cm.vert.size();// 重新记录新的网格mesh

		if (kt)
		{
			delete kt;
		}

		kt = new KdTree<float>(VertexConstDataWrapper<CMeshO>(_mesh->cm));
	}

	typename KdTree<float>::PriorityQueue pq;
	int neighbor = 2;

	if (kt == nullptr)
	{
		return -1;
	}

	kt->doQueryK(point, neighbor, pq);

	float minDis = 1000000.0f;
	int nearstVId = -1;
	for (int i = 0; i < neighbor; i++)
	{
		int neighbourId = pq.getIndex(i);
		float pointSquareDist = vcg::SquaredNorm(_mesh->cm.vert[neighbourId].cP() - point);

		if (pointSquareDist < minDis)
		{
			nearstVId = neighbourId;
			minDis = pointSquareDist;
		}
	}

	if (nearstVId < 0 || nearstVId >= _mesh->cm.vert.size())
	{
		return-1;
	}
	else
	{
		return nearstVId;
	}
}

std::vector<int> UtilityTools::matchPointsFromMesh(MeshModel* _mesh, const std::vector<vcg::Point3f>& pos_points)
{
	if (pos_points.empty() || _mesh == nullptr)
	{
		return std::vector<int>();
	}

	std::vector<int> index_match_points;

	for (auto v : pos_points)
	{
		int nearstVId = UtilityTools::getInstance()->getIndexPointClosestMesh(*_mesh, v);

		if (nearstVId >= 0)
		{
			index_match_points.push_back(nearstVId);
		}
	}

	return index_match_points;
}

vcg::Point3d UtilityTools::projectPoint(vcg::Point3f p, MeshModel* p_mesh_ /*= nullptr*/)
{
	CMeshO* pCmesh = nullptr;
	if (p_mesh_)
	{
		pCmesh = &p_mesh_->cm;
	}

	return projectPoint(p, pCmesh);
}

vcg::Point3d UtilityTools::unProjectPoint(vcg::Point3f p, MeshModel* p_mesh_ /*= nullptr*/)
{
	CMeshO* pCmesh = nullptr;
	if (p_mesh_)
	{
		pCmesh = &p_mesh_->cm;
	}

	return unProjectPoint(p, pCmesh);;
}

vcg::Point3d UtilityTools::projectPoint(vcg::Point3f p, CMeshO* p_mesh_)
{
	double mm[16], mp[16];
	GLint vp[4];
	glPushMatrix();
	if (p_mesh_)
	{
		glMultMatrix(p_mesh_->Tr);
	}
	glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	glGetDoublev(GL_PROJECTION_MATRIX, mp);
	glGetIntegerv(GL_VIEWPORT, vp);
	Point3d result(0, 0, 0);
	gluUnProject(p[0], p[1], p[2], mm, mp, vp, &result[0], &result[1], &result[2]);
	glPopMatrix();

	return result;
}

vcg::Point3d UtilityTools::unProjectPoint(vcg::Point3f p, CMeshO* p_mesh_)
{
	double mm[16], mp[16];
	GLint vp[4];
	glPushMatrix();
	if (p_mesh_)
	{
		glMultMatrix(p_mesh_->Tr);
	}

	glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	glGetDoublev(GL_PROJECTION_MATRIX, mp);
	glGetIntegerv(GL_VIEWPORT, vp);
	Point3d result(0, 0, 0);
	gluProject(p[0], p[1], p[2], mm, mp, vp, &result[0], &result[1], &result[2]);
	glPopMatrix();

	return result;
}

Point3m UtilityTools::getMixedPointFromScreenToWorldOnVerticalPlane(
	float mX, float mY, Point3m origin, Point3m mainV, int width_window, int height_window, CMeshO* p_mesh_)
{
	Point3m resultP;
	vcg::Point3d tempProjPoint;
	Point3m nearP, farP;

	tempProjPoint = projectPoint(Point3m(width_window / 2.0f, height_window / 2.0f, 0), p_mesh_);
	nearP.X() = tempProjPoint.X();
	nearP.Y() = tempProjPoint.Y();
	nearP.Z() = tempProjPoint.Z();
	tempProjPoint = projectPoint(Point3m(width_window / 2.0f, height_window / 2.0f, 1), p_mesh_);
	farP.X() = tempProjPoint.X();
	farP.Y() = tempProjPoint.Y();
	farP.Z() = tempProjPoint.Z();

	Point3m normalVector = nearP - farP;
	normalVector = normalVector.Normalize();

	tempProjPoint = projectPoint(Point3m(mX, mY, 0), p_mesh_);
	nearP.X() = tempProjPoint.X();
	nearP.Y() = tempProjPoint.Y();
	nearP.Z() = tempProjPoint.Z();
	tempProjPoint = projectPoint(Point3m(mX, mY, 1), p_mesh_);
	farP.X() = tempProjPoint.X();
	farP.Y() = tempProjPoint.Y();
	farP.Z() = tempProjPoint.Z();

	float t;
	Point3m fnP = farP - nearP;
	float base = fnP.X() * normalVector.X() + fnP.Y() * normalVector.Y() + fnP.Z() * normalVector.Z();

	Point3m projP;
	t = float(((origin.X() - nearP.X()) * normalVector.X() + (origin.Y() - nearP.Y()) * normalVector.Y() + (origin.Z() - nearP.Z()) * normalVector.Z()) / base);
	if (t >= 0 && t <= 1)
	{
		projP = nearP + fnP * t;
		float alph = (projP - origin) * mainV;
		float fRadius = 1.5;
		if (alph >= 1)
		{
			resultP = origin + mainV * fRadius;
			return resultP;
		}
		else if (alph <= -1)
		{
			resultP = origin - mainV * fRadius;
			return resultP;
		}
		else
		{
			float beta = sqrtf(1 - alph * alph);
			resultP = origin + mainV * alph + normalVector * beta;
			return resultP;
		}
	}

	return origin;
}

Point3m UtilityTools::getMixedPointFromScreenToWorld1(
	float mX, float mY, Point3m nV, Point3m hP, Point3m mainV, int width_window, int height_window, CMeshO* p_mesh_)
{
	Point3m resultP;
	vcg::Point3d tempProjPoint;
	Point3m nearP, farP;

	tempProjPoint = projectPoint(Point3m(mX, mY, 0), p_mesh_);
	nearP.X() = tempProjPoint.X();
	nearP.Y() = tempProjPoint.Y();
	nearP.Z() = tempProjPoint.Z();
	tempProjPoint = projectPoint(Point3m(mX, mY, 1), p_mesh_);
	farP.X() = tempProjPoint.X();
	farP.Y() = tempProjPoint.Y();
	farP.Z() = tempProjPoint.Z();

	float t;
	Point3m fnP = farP - nearP;
	float base = fnP.X() * nV.X() + fnP.Y() * nV.Y() + fnP.Z() * nV.Z();
	if (base == 0)
	{
		resultP = getMixedPointFromScreenToWorldOnVerticalPlane(mX, mY, hP, mainV, width_window, height_window, p_mesh_);
		return hP;
	}

	t = float(((hP.X() - nearP.X()) * nV.X() + (hP.Y() - nearP.Y()) * nV.Y() + (hP.Z() - nearP.Z()) * nV.Z()) / base);
	if (t >= 0 && t <= 1)
	{
		resultP = nearP + fnP * t;
		return resultP;
	}
	else
		return hP;
}

Point3m UtilityTools::getMixedPointFromScreenToWorld2(float mX, float mY, Point3m nV1, Point3m nV2, Point3m hP, CMeshO* curMesh)
{
	Point3m resultP;
	vcg::Point3d tempProjPoint;
	Point3m nearP, farP;

	tempProjPoint = projectPoint(Point3m(mX, mY, 0), curMesh);
	nearP.X() = tempProjPoint.X();
	nearP.Y() = tempProjPoint.Y();
	nearP.Z() = tempProjPoint.Z();
	tempProjPoint = projectPoint(Point3m(mX, mY, 1), curMesh);
	farP.X() = tempProjPoint.X();
	farP.Y() = tempProjPoint.Y();
	farP.Z() = tempProjPoint.Z();

	float t;
	Point3m fnP = farP - nearP;
	Point3m nV;
	Point3m fNPNormal = fnP;
	fNPNormal = fNPNormal.Normalize();
	float base1 = fNPNormal.X() * nV1.X() + fNPNormal.Y() * nV1.Y() + fNPNormal.Z() * nV1.Z();
	float base2 = fNPNormal.X() * nV2.X() + fNPNormal.Y() * nV2.Y() + fNPNormal.Z() * nV2.Z();
	float base;
	if (base1 != 0)
	{
		nV = nV1;
		base = fnP.X() * nV1.X() + fnP.Y() * nV1.Y() + fnP.Z() * nV1.Z();
	}
	else if (base2 != 0)
	{
		nV = nV2;
		base = fnP.X() * nV2.X() + fnP.Y() * nV2.Y() + fnP.Z() * nV2.Z();
	}
	else
		return hP;

	t = float(((hP.X() - nearP.X()) * nV.X() + (hP.Y() - nearP.Y()) * nV.Y() + (hP.Z() - nearP.Z()) * nV.Z()) / base);
	if (t >= 0 && t <= 1)
	{
		resultP = nearP + fnP * t;
		return resultP;
	}
	else
		return hP;
}

bool UtilityTools::getPickedPointOnMesh(int _mouseX, int _mouseY, vector<CFaceO*>& _vf, CMeshO* curMesh, Point3m& _point, vcg::Matrix44f* _designative_matrix)
{
	vcg::GLPickTri<CMeshO> pickFace;
	glPushMatrix();
	if (_designative_matrix != nullptr)
	{
		glMultMatrix(*_designative_matrix);
	}
	else
	{
		glMultMatrix(curMesh->Tr);
	}
	_vf.clear();
	pickFace.PickFace(_mouseX, _mouseY, *curMesh, _vf);
	glPopMatrix();

	if (_vf.size() > 0)
	{
		int faceIndex;

		Point3d pStart = projectPoint(Point3m(_mouseX, _mouseY, 0), curMesh);
		Point3d pEnd = projectPoint(Point3m(_mouseX, _mouseY, 1), curMesh);
		Point3d tempV = (pEnd - pStart).Normalize();

		vector<int> tempFaceIndexes;
		for (auto vf : _vf)
		{
			Point3m faceNormal = vf->N();
			if (faceNormal.X() * tempV.X() + faceNormal.Y() * tempV.Y() + faceNormal.Z() * tempV.Z() < 0)
			{
				int face_index = vf - &curMesh->face[0];
				if (0 <= face_index && face_index < curMesh->face.size())
				{
					tempFaceIndexes.push_back(vf - &curMesh->face[0]);
				}
			}
		}
		float miniDistance = FLT_MAX;
		for (auto index : tempFaceIndexes)
		{
			Point3m edge = (curMesh->face[index].V(0)->P() + curMesh->face[index].V(1)->P() + curMesh->face[index].V(2)->P()) / 3.0f;
			edge = Point3m(edge.X() - pStart.X(), edge.Y() - pStart.Y(), edge.Z() - pStart.Z());
			float curDistance = edge * edge;
			if (curDistance < miniDistance)
			{
				miniDistance = curDistance;
				faceIndex = index;
			}
		}
		tempFaceIndexes.clear();

		if (0 <= faceIndex && faceIndex < curMesh->face.size())
		{
			Point3m intersection, plane_center, plane_normal, view_pos, view_direction;
			view_pos = Point3m(pStart.X(), pStart.Y(), pStart.Z());
			view_direction = Point3m(tempV.X(), tempV.Y(), tempV.Z());
			plane_center = (curMesh->face[faceIndex].V(0)->P() + curMesh->face[faceIndex].V(1)->P() + curMesh->face[faceIndex].V(2)->P()) / 3.0f;
			plane_normal = plane_center + curMesh->face[faceIndex].N();
			if (_designative_matrix != nullptr)
			{
				plane_center = (*_designative_matrix) * plane_center;
				plane_normal = (*_designative_matrix) * plane_normal;
			}
			else
			{
				plane_center = curMesh->Tr * plane_center;
				plane_normal = curMesh->Tr * plane_normal;
			}
			plane_normal = (plane_normal - plane_center).Normalize();
			intersection = this->getProjPointOnPlane(view_pos, view_direction, plane_center, -plane_normal);
			_point = intersection;

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool UtilityTools::getPickedFaceIndex(int _mouseX, int _mouseY, vector<CFaceO*>& _vf, CMeshO* curMesh, int& _faceIndex, vcg::Matrix44f* _designative_matrix)
{
	vcg::GLPickTri<CMeshO> pickFace;
	glPushMatrix();
	if (_designative_matrix != nullptr)
	{
		glMultMatrix(*_designative_matrix);
	}
	else
	{
		glMultMatrix(curMesh->Tr);
	}
	_vf.clear();
	pickFace.PickVisibleFace(_mouseX, _mouseY, *curMesh, _vf);
	glPopMatrix();

	if (_vf.size() > 0)
	{
		int faceIndex;

		Point3d pStart = projectPoint(Point3m(_mouseX, _mouseY, 0), curMesh);
		Point3d pEnd = projectPoint(Point3m(_mouseX, _mouseY, 1), curMesh);
		Point3d tempV = pEnd - pStart;

		vector<int> tempFaceIndexes;
		for (auto vf : _vf)
		{
			Point3m faceNormal = vf->N();
			if (faceNormal.X() * tempV.X() + faceNormal.Y() * tempV.Y() + faceNormal.Z() * tempV.Z() < 0)
			{
				int face_index = vf - &curMesh->face[0];
				if (0 <= face_index && face_index < curMesh->face.size())
				{
					tempFaceIndexes.push_back(vf - &curMesh->face[0]);
				}
			}
		}
		float miniDistance = FLT_MAX;
		for (auto index : tempFaceIndexes)
		{
			Point3m edge = (curMesh->face[index].V(0)->P() + curMesh->face[index].V(1)->P() + curMesh->face[index].V(2)->P()) / 3.0f;
			edge = Point3m(edge.X() - pStart.X(), edge.Y() - pStart.Y(), edge.Z() - pStart.Z());
			float curDistance = edge * edge;
			if (curDistance < miniDistance)
			{
				miniDistance = curDistance;
				faceIndex = index;
			}
		}
		tempFaceIndexes.clear();
		_faceIndex = faceIndex;

		return true;
	}
	else
	{
		_faceIndex = -1;
		return false;
	}
}

bool UtilityTools::pointBlocked(Point3m _point, float _deepth_limit /*= 0*/)
{
	double mm[16], mp[16];
	GLint vp[4];
	glPushMatrix();
	glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	glGetDoublev(GL_PROJECTION_MATRIX, mp);
	glGetIntegerv(GL_VIEWPORT, vp);
	Point3d proj_pos(0, 0, 0);
	gluProject(_point.X(), _point.Y(), _point.Z(), mm, mp, vp, &proj_pos[0], &proj_pos[1], &proj_pos[2]);
	float z = 1;
	glReadPixels(proj_pos[0], proj_pos[1], 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
	Point3d block_point(0, 0, 0);
	gluProject(proj_pos[0], proj_pos[1], z, mm, mp, vp, &block_point[0], &block_point[1], &block_point[2]);
	glPopMatrix();
	Point3m vec(block_point[0] - _point.X(), block_point[1] - _point.Y(), block_point[2] - _point.Z());

	return (z < proj_pos[2] && vec.Norm() > _deepth_limit);
}

bool UtilityTools::pickHandle(int x, int y, int& indexPicked,
	const std::vector<Point3m>& loop, float radius,
	bool _cull_face, int width, int height, bool sorted)
{
	CMeshO* pCmesh = nullptr;
	Point3d nearPos = projectPoint(Point3f(x, y, 0), pCmesh);
	Point3d farPos = projectPoint(Point3f(x, y, 1), pCmesh);
	Point3d dir = farPos - nearPos;

	Point3m o = Point3m(nearPos.X(), nearPos.Y(), nearPos.Z());
	Point3m d = Point3m(dir.X(), dir.Y(), dir.Z()).Normalize();
	int i = 0;
	set<pair<float, int>> tempRecords;
	for (auto v : loop)
	{
		Point3m n = (o - v).Normalize();
		Point3m intersection = getProjPointOnPlane(o, d, v, -n);
		Point3m tempV = intersection - v;
		float curDis = sqrtf(tempV * tempV);
		if (curDis <= radius)
		{
			bool actual_picked = true;
			if (!_cull_face)
			{
				double mm[16], mp[16];
				GLint vp[4];
				glPushMatrix();
				glGetDoublev(GL_MODELVIEW_MATRIX, mm);
				glGetDoublev(GL_PROJECTION_MATRIX, mp);
				glGetIntegerv(GL_VIEWPORT, vp);

				float z = 1;
				glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
				Point3d result(0, 0, 0);
				gluUnProject(x, y, z, mm, mp, vp, &result[0], &result[1], &result[2]);
				glPopMatrix();

				if (0 <= z && z <= 1)
				{
					tempV = Point3m(result.X() - v.X(), result.Y() - v.Y(), result.Z() - v.Z());
					if (tempV.Norm() > radius)
					{
						if (tempV * -n < 0)
						{
							actual_picked = false;
						}
					}
				}
			}

			if (actual_picked)
			{
				float distance = abs((v - o) * d);
				tempRecords.insert(pair<float, int>(distance, i));
			}
		}
		++i;
	}
	if (tempRecords.empty())
	{
		indexPicked = -1;
		return false;
	}
	else
	{
		indexPicked = (*tempRecords.begin()).second;
		return true;
	}
}

bool UtilityTools::pickHandle(int x, int y, int& indexPicked,
	const std::vector<Point3m>& loop, const std::vector<Point3m>& _judge_vec_loop, const std::vector<bool>& _cullface_loop, float radius,
	bool _cull_face, int width, int height, bool sorted)
{
	CMeshO* pCmesh = nullptr;
	Point3d nearPos = projectPoint(Point3f(x, y, 0), pCmesh);
	Point3d farPos = projectPoint(Point3f(x, y, 1), pCmesh);
	Point3d dir = farPos - nearPos;

	Point3m o = Point3m(nearPos.X(), nearPos.Y(), nearPos.Z());
	Point3m d = Point3m(dir.X(), dir.Y(), dir.Z()).Normalize();
	int i = 0;
	set<pair<float, int>> tempRecords;
	for (auto v : loop)
	{
		Point3m n = (o - v).Normalize();
		Point3m intersection = getProjPointOnPlane(o, d, v, n);
		Point3m tempV = intersection - v;
		float curDis = sqrtf(tempV * tempV);
		if (curDis <= radius)
		{
			// Get gl state values
			double mm[16], mp[16];
			GLint vp[4];
			glPushMatrix();
			glGetDoublev(GL_MODELVIEW_MATRIX, mm);
			glGetDoublev(GL_PROJECTION_MATRIX, mp);
			glGetIntegerv(GL_VIEWPORT, vp);

			float z = 1;
			glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
			Point3d result(0, 0, 0);
			gluUnProject(x, y, z, mm, mp, vp, &result[0], &result[1], &result[2]);
			glPopMatrix();

			bool actual_picked = true;
			tempV = Point3m(result.X() - v.X(), result.Y() - v.Y(), result.Z() - v.Z());
			if (_judge_vec_loop.size() == loop.size() && _judge_vec_loop[i] != Point3m(0, 0, 0))
			{
				if (tempV * _judge_vec_loop[i] > 0)
				{
					actual_picked = false;
				}
			}

			if (_cullface_loop.size() == loop.size() && !_cullface_loop[i] && 0 <= z && z <= 1 && actual_picked)
			{
				if (tempV.Norm() > radius)
				{
					if (tempV * -n < 0)
					{
						actual_picked = false;
					}
				}
			}

			if (actual_picked)
			{
				float distance = abs((v - o) * d);
				tempRecords.insert(pair<float, int>(distance, i));
			}
		}
		++i;
	}
	if (tempRecords.empty())
	{
		indexPicked = -1;
		return false;
	}
	else
	{
		indexPicked = (*tempRecords.begin()).second;
		return true;
	}
}

bool UtilityTools::pickHandle2(int x, int y, int& indexPicked,
	const std::vector<Point3m>& loop, float radius,
	int width, int height, bool sorted)
{
	if (width == 0 || height == 0) return false;
	long hits;
	int sz = 3 * 5;
	GLuint* selectBuf = new GLuint[sz];
	glSelectBuffer(sz, selectBuf);
	glRenderMode(GL_SELECT);
	glInitNames();

	/* Because LoadName() won't work with no names on the stack */
	glPushName(-1);
	double mp[16];

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glMatrixMode(GL_PROJECTION);
	glGetDoublev(GL_PROJECTION_MATRIX, mp);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix(x, y, width, height, viewport);
	glMultMatrixd(mp);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	for (int i = 0; i < loop.size(); i++)
	{
		glLoadName(i);
		vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(loop[i], radius, CTRLNODE_SLICE, CTRLNODE_STACK);
	}
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	hits = glRenderMode(GL_RENDER);
	std::vector< std::pair<double, unsigned int> > H;
	for (long ii = 0; ii < hits; ii++) {
		H.push_back(std::pair<double, unsigned int>(
			selectBuf[ii * 4 + 1] / 4294967295.0, selectBuf[ii * 4 + 3]));
	}
	if (sorted)
		std::sort(H.begin(), H.end());
	if (H.size() > 0)
	{
		indexPicked = H[0].second;
	}
	delete[] selectBuf;
	if (H.size() > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool UtilityTools::pickOnCircle(int x, int y, Point3m pos, Point3m N, float fRadius, Point3m& rotateCircleHandle)
{
	CMeshO* pCmesh = nullptr;
	Point3d start, end;
	start = projectPoint(Point3m(x, y, 0), pCmesh);
	end = projectPoint(Point3m(x, y, 1), pCmesh);

	Point3m temp_vert;
	if (pos.Z() != 0)
	{
		float temp_z = (pos.X() * N.X() + pos.Y() * N.Y() + pos.Z() * N.Z()) / pos.Z();
		temp_vert = Point3m(0, 0, temp_z);
	}
	else if (pos.X() != 0)
	{
		float temp_x = (pos.X() * N.X() + pos.Y() * N.Y() + pos.Z() * N.Z()) / pos.X();
		temp_vert = Point3m(temp_x, 0, 0);
	}
	else if (pos.Y() != 0)
	{
		float temp_y = (pos.X() * N.X() + pos.Y() * N.Y() + pos.Z() * N.Z()) / pos.Y();
		temp_vert = Point3m(0, temp_y, 0);
	}
	Point3m radius_vec = (temp_vert - pos).Normalize();
	Point3m radius_node = pos + radius_vec * fRadius;
	Point3d proj_center = unProjectPoint(pos, pCmesh);
	Point3d proj_radius = unProjectPoint(radius_node, pCmesh);
	double proj_radius_length = sqrtf(
		(proj_radius.X() - proj_center.X()) * (proj_radius.X() - proj_center.X()) +
		(proj_radius.Y() - proj_center.Y()) * (proj_radius.Y() - proj_center.Y()) +
		(proj_radius.Z() - proj_center.Z()) * (proj_radius.Z() - proj_center.Z()));
	float pixcel_distance = fRadius / proj_radius_length;
	pixcel_distance *= 3.0f;
	/*if (pixcel_distance)
	{
		pixcel_distance = 1.0f;
	}*/

	Point3m p, D;
	D.X() = (float)(end.X() - start.X());
	D.Y() = (float)(end.Y() - start.Y());
	D.Z() = (float)(end.Z() - start.Z());
	D = D.Normalize();

	float distanceSquare;
	Point3m intersection, directVector;
	{
		p.X() = (float)start.X();
		p.Y() = (float)start.Y();
		p.Z() = (float)start.Z();

		intersection = getProjPointOnPlane(p, D, pos, N);
		directVector = (intersection - pos).Normalize();
		distanceSquare = sqrtf((intersection - pos) * (intersection - pos));

		qDebug() << "mouse pick distance : " << distanceSquare << endl;
		if (abs(distanceSquare - (fRadius)) < pixcel_distance)
		{
			rotateCircleHandle = pos + directVector * fRadius;
			return true;
		}
	}
	return false;
}

bool UtilityTools::pickConeHandle(int x, int y, int& indexPicked,
	const std::vector<pair<Point3m, Point3m>>& _cps, float _width, Matrix44f _adjMatrix,
	int width, int height, bool sorted)
{
	/*glPushMatrix();
	glMultMatrix(_adjMatrix);*/

	if (width == 0 || height == 0) return false;
	long hits;
	int sz = 3 * 5;
	GLuint* selectBuf = new GLuint[sz];
	glSelectBuffer(sz, selectBuf);
	glRenderMode(GL_SELECT);
	glInitNames();

	/* Because LoadName() won't work with no names on the stack */
	glPushName(-1);
	double mp[16];

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glMatrixMode(GL_PROJECTION);
	glGetDoublev(GL_PROJECTION_MATRIX, mp);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix(x, y, width, height, viewport);
	glMultMatrixd(mp);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	for (int i = 0; i < _cps.size(); i++)
	{
		glLoadName(i);
		vcg::Add_Ons::glCone<vcg::Add_Ons::DMSolid>(_cps[i].first, _cps[i].second, _width);
	}
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	hits = glRenderMode(GL_RENDER);
	std::vector< std::pair<double, unsigned int> > H;
	for (long ii = 0; ii < hits; ii++) {
		H.push_back(std::pair<double, unsigned int>(
			selectBuf[ii * 4 + 1] / 4294967295.0, selectBuf[ii * 4 + 3]));
	}
	//glPopMatrix();
	if (sorted)
		std::sort(H.begin(), H.end());
	if (H.size() > 0)
	{
		indexPicked = H[0].second;
	}
	delete[] selectBuf;
	if (H.size() > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool UtilityTools::pickCylinder(int x, int y, int& indexPicked,
	const std::vector<FEdge>& _cps, float _width, Matrix44f _adjMatrix /*= Matrix44f::Identity()*/,
	int width /*= 4*/, int height /*= 4*/, bool sorted /*= false*/)
{
	if (width == 0 || height == 0) return false;
	long hits;
	int sz = 3 * 5;
	GLuint* selectBuf = new GLuint[sz];
	glSelectBuffer(sz, selectBuf);
	glRenderMode(GL_SELECT);
	glInitNames();

	/* Because LoadName() won't work with no names on the stack */
	glPushName(-1);
	double mp[16];

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glMatrixMode(GL_PROJECTION);
	glGetDoublev(GL_PROJECTION_MATRIX, mp);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix(x, y, width, height, viewport);
	glMultMatrixd(mp);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	for (int i = 0; i < _cps.size(); i++)
	{
		glLoadName(i);
		vcg::Add_Ons::glCylinder<vcg::Add_Ons::DMSolid>(_cps[i].vertA, _cps[i].vertB, _width);
	}
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	hits = glRenderMode(GL_RENDER);
	std::vector< std::pair<double, unsigned int> > H;
	for (long ii = 0; ii < hits; ii++) {
		H.push_back(std::pair<double, unsigned int>(
			selectBuf[ii * 4 + 1] / 4294967295.0, selectBuf[ii * 4 + 3]));
	}
	//glPopMatrix();
	if (sorted)
		std::sort(H.begin(), H.end());
	if (H.size() > 0)
	{
		indexPicked = H[0].second;
	}
	delete[] selectBuf;
	if (H.size() > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

vector<int> UtilityTools::oneRingNeighborhoodVF(int vertIndex, CMeshO* curMesh)
{
	vector<int> result;
	vcg::face::VFIterator<CFaceO> vfi(&curMesh->vert[vertIndex]);
	for (; !vfi.End(); ++vfi)
	{
		result.push_back(vfi.F() - &curMesh->face[0]);
	}
	return result;
}

vector<int> UtilityTools::oneRingNeighborhoodVV(int vertIndex, CMeshO* curMesh)
{
	vector<int> faces = oneRingNeighborhoodVF(vertIndex, curMesh);
	set<int> tempSet;
	for (auto faceIndex : faces)
	{
		int iV1 = curMesh->face[faceIndex].V(0) - &curMesh->vert[0];
		if (iV1 != vertIndex)
		{
			if (0 <= iV1 && iV1 < curMesh->vert.size())
			{
				tempSet.insert(iV1);
			}
		}
		int iV2 = curMesh->face[faceIndex].V(1) - &curMesh->vert[0];
		if (iV2 != vertIndex)
		{
			if (0 <= iV2 && iV2 < curMesh->vert.size())
			{
				tempSet.insert(iV2);
			}
		}
		int iV3 = curMesh->face[faceIndex].V(2) - &curMesh->vert[0];
		if (iV3 != vertIndex)
		{
			if (0 <= iV3 && iV3 < curMesh->vert.size())
			{
				tempSet.insert(iV3);
			}
		}
	}
	vector<int> result;
	result.insert(result.end(), tempSet.begin(), tempSet.end());
	set<int>().swap(tempSet);
	return result;
}

std::vector<int> UtilityTools::oneRingNeighborhoodFF(int faceIndex, CMeshO* curMesh)
{
	std::vector<int> result;
	result.push_back(curMesh->face[faceIndex].FFp(0) - &curMesh->face[0]);
	result.push_back(curMesh->face[faceIndex].FFp(1) - &curMesh->face[0]);
	result.push_back(curMesh->face[faceIndex].FFp(2) - &curMesh->face[0]);
	return result;
}

vector<int> UtilityTools::vertNeighborVerts(int index, CMeshO* curMesh, int iN)
{
	set<int> resultSet;
	set<int> newVerts;
	newVerts.insert(index);
	for (int i = 0; i < iN; ++i)
	{
		set<int> tempNewSet;
		for (auto index : newVerts)
		{
			vector<int> tempOneRing = oneRingNeighborhoodVV(index, curMesh);
			tempNewSet.insert(tempOneRing.begin(), tempOneRing.end());
		}
		resultSet.insert(tempNewSet.begin(), tempNewSet.end());
		newVerts = tempNewSet;
		set<int>().swap(tempNewSet);
	}
	vector<int> result;
	result.insert(result.end(), resultSet.begin(), resultSet.end());
	return result;
}

std::vector<int> UtilityTools::vertNeighborFaces(int index, CMeshO* curMesh, int iN)
{
	set<int> resultSet;
	set<int> newFaces;
	vector<int> newFaceList = oneRingNeighborhoodVF(index, curMesh);
	newFaces.insert(newFaceList.begin(), newFaceList.end());
	for (int i = 0; i < iN; ++i)
	{
		set<int> tempNewSet;
		for (auto index : newFaces)
		{
			vector<int> tempOneRing = oneRingNeighborhoodFF(index, curMesh);
			tempNewSet.insert(tempOneRing.begin(), tempOneRing.end());
		}
		resultSet.insert(tempNewSet.begin(), tempNewSet.end());
		newFaces = tempNewSet;
		set<int>().swap(tempNewSet);
	}
	vector<int> result;
	result.insert(result.end(), resultSet.begin(), resultSet.end());
	return result;
}

float UtilityTools::findTheAreaOfTriangle(int faceIndex, CMeshO* curMesh)
{
	Point3m a, b, c;
	a = curMesh->face[faceIndex].V(1)->P() - curMesh->face[faceIndex].V(0)->P();
	b = curMesh->face[faceIndex].V(2)->P() - curMesh->face[faceIndex].V(0)->P();
	float a_length = sqrtf(a * a);
	float b_length = sqrtf(b * b);
	float adotb = a * b;
	float area = sqrtf(a_length*a_length * b_length * b_length - adotb * adotb);
	return area;
}

Point3m UtilityTools::findTheCentroidOfTriangle(int faceIndex, CMeshO* curMesh)
{
	Point3m a, b, c;
	a = curMesh->face[faceIndex].V(0)->P();
	b = curMesh->face[faceIndex].V(1)->P();
	c = curMesh->face[faceIndex].V(2)->P();
	return (a + b + c) / 3.0f;
}

bool UtilityTools::vertOutOfPolygon(Point3m pt, std::vector<Point3m>& plist)
{
	int nCross = 0;    // 定义变量，统计目标点向右画射线与多边形相交次数

	for (int i = 0; i < plist.size(); i++) {   //遍历多边形每一个节点

		Point3m p1, p2;

		p1 = plist[i];
		p2 = plist[(i + 1) % plist.size()];  // p1是这个节点，p2是下一个节点，两点连线是多边形的一条边
	  // 以下算法是用是先以y轴坐标来判断的

		if (p1.Y() == p2.Y())
			continue;   //如果这条边是水平的，跳过

		if (pt.Y() < min(p1.Y(), p2.Y())) //如果目标点低于这个线段，跳过
			continue;

		if (pt.Y() >= max(p1.Y(), p2.Y())) //如果目标点高于这个线段，跳过
			continue;
		//那么下面的情况就是：如果过p1画水平线，过p2画水平线，目标点在这两条线中间
		double x = (double)(pt.Y() - p1.Y()) * (double)(p2.X() - p1.X()) / (double)(p2.Y() - p1.Y()) + p1.X();
		// 这段的几何意义是 过目标点，画一条水平线，x是这条线与多边形当前边的交点x坐标
		if (x > pt.X())
		{
			nCross++; //如果交点在右边，统计加一。这等于从目标点向右发一条射线（ray），与多边形各边的相交（crossing）次数
		}
	}

	if (nCross % 2 == 1)
	{
		return false; //如果是奇数，说明在多边形里
	}
	else
	{
		return true; //否则在多边形外 或 边上
	}
}

bool UtilityTools::vertOutOfPolygon(Point2m pt, std::vector<Point2m>& plist)
{
	int nCross = 0;    // 定义变量，统计目标点向右画射线与多边形相交次数

	for (int i = 0; i < plist.size(); i++) {   //遍历多边形每一个节点

		Point2m p1, p2;

		p1 = plist[i];
		p2 = plist[(i + 1) % plist.size()];  // p1是这个节点，p2是下一个节点，两点连线是多边形的一条边
	  // 以下算法是用是先以y轴坐标来判断的

		if (p1.Y() == p2.Y())
			continue;   //如果这条边是水平的，跳过

		if (pt.Y() < min(p1.Y(), p2.Y())) //如果目标点低于这个线段，跳过
			continue;

		if (pt.Y() >= max(p1.Y(), p2.Y())) //如果目标点高于这个线段，跳过
			continue;
		//那么下面的情况就是：如果过p1画水平线，过p2画水平线，目标点在这两条线中间
		double x = (double)(pt.Y() - p1.Y()) * (double)(p2.X() - p1.X()) / (double)(p2.Y() - p1.Y()) + p1.X();
		// 这段的几何意义是 过目标点，画一条水平线，x是这条线与多边形当前边的交点x坐标
		if (x > pt.X())
		{
			nCross++; //如果交点在右边，统计加一。这等于从目标点向右发一条射线（ray），与多边形各边的相交（crossing）次数
		}
	}

	if (nCross % 2 == 1)
	{
		return false; //如果是奇数，说明在多边形里
	}
	else
	{
		return true; //否则在多边形外 或 边上
	}
}

vector<int> UtilityTools::collectNeighborVerts(vector<int> tril, CMeshO* curMesh, int iN)
{
	set<int> ignoreSet;
	set<int> curSet;
	curSet.insert(tril.begin(), tril.end());
	ignoreSet.insert(tril.begin(), tril.end());

	for (auto index : tril)
	{
		vector<int> tempList = vertNeighborVerts(index, curMesh, iN);
		curSet.insert(tempList.begin(), tempList.end());
		vector<int>().swap(tempList);
	}
	vector<int> result;
	for (auto index : curSet)
	{
		if (ignoreSet.find(index) != ignoreSet.end())
		{
			result.push_back(index);
		}
	}
	set<int>().swap(curSet);
	set<int>().swap(ignoreSet);
	return result;
}

void UtilityTools::smoothVert(int ivert, CMeshO* curMesh, float fT)
{
	vector<int> oneRingVerts = oneRingNeighborhoodVV(ivert, curMesh);
	Point3m adjVector(0, 0, 0);
	for (auto iVert : oneRingVerts)
	{
		adjVector += curMesh->vert[iVert].P() - curMesh->vert[ivert].P();
	}
	adjVector *= fT;
	curMesh->vert[ivert].P() += adjVector;
}

pair<bool, Point3m>  UtilityTools::intersectMesh(Point3m& orig, Point3m& dir, CMeshO* mesh)
{
	float min_distance = FLT_MAX;
	pair<bool, Point3m> result(false, Point3m(0, 0, 0));
	for (int i = 0; i < mesh->face.size(); ++i)
	{
		pair<bool, Point3m> tempIntersectResult = intersectTriangle(orig, dir, mesh, i);
		if (tempIntersectResult.first)
		{
			Point3m vec = tempIntersectResult.second - orig;
			float distance = vec.Norm();
			if (distance < min_distance)
			{
				min_distance = distance;
				result = tempIntersectResult;
			}
			//return tempIntersectResult;
		}
	}
	return result;
}

pair<bool, Point3m> UtilityTools::intersectMeshAndGetNearestIntersection(Point3m& orig, Point3m& dir, CMeshO* mesh)
{
	vector<pair<bool, Point3m>> candidates;
	for (int i = 0; i < mesh->face.size(); ++i)
	{
		pair<bool, Point3m> tempIntersectResult = intersectTriangle(orig, dir, mesh, i);
		if (tempIntersectResult.first)
		{
			candidates.push_back(tempIntersectResult);
		}
	}

	if (!candidates.empty())
	{
		if (candidates.size() == 1)
		{
			return candidates[0];
		}
		else
		{
			int index = -1;
			float min_distance = FLT_MAX;
			for (int i = 0; i < candidates.size(); ++i)
			{
				float distance = (candidates[i].second - orig) * dir;
				if (distance < min_distance)
				{
					min_distance = distance;
					index = i;
				}
			}

			if (index >= 0)
			{
				return candidates[index];
			}
		}
	}
	return pair<bool, Point3m>(false, Point3m(0, 0, 0));
}

pair<bool, Point3m> UtilityTools::intersectCandidateFaces(Point3m& orig, Point3m& dir, CMeshO* mesh, const std::vector<int> &_candidate_faces, int _ignored_index /*= -1*/)
{
	for (auto &i : _candidate_faces)
	{
		if (i != _ignored_index)
		{
			pair<bool, Point3m> tempIntersectResult = intersectTriangle(orig, dir, mesh, i);
			if (tempIntersectResult.first)
			{
				return tempIntersectResult;
			}
		}
	}
	return pair<bool, Point3m>(false, Point3m(0, 0, 0));
}

pair<bool, Point3m> UtilityTools::intersectTriangle(Point3m& orig, Point3m& dir, CMeshO* mesh, int iFace)
{
	if (mesh == nullptr)
	{
		return pair<bool, Point3m>(false, Point3m(0, 0, 0));
	}
	if (iFace < 0 || iFace >= mesh->face.size())
	{
		return pair<bool, Point3m>(false, Point3m(0, 0, 0));
	}
	return intersectTriangle(orig, dir, mesh->face[iFace].V(0)->P(), mesh->face[iFace].V(1)->P(), mesh->face[iFace].V(2)->P());
}

pair<bool, Point3m> UtilityTools::intersectTriangle(Point3m& orig, Point3m& dir, Point3m& v0, Point3m& v1, Point3m& v2)
{
	// E1
	Point3m E1 = v1 - v0;
	// E2
	Point3m E2 = v2 - v0;
	// P
	Point3m P = dir ^ E2;
	// determinant
	float det = E1 * P;

	// keep det > 0, modify T accordingly
	Point3m T;
	if (det > 0)
	{
		T = orig - v0;
	}
	else
	{
		T = v0 - orig;
		det = -det;
	}

	// If determinant is near zero, ray lies in plane of triangle
	if (det < 0.0001f)
	{
		return pair<bool, Point3m>(false, Point3m(0, 0, 0));
	}

	// Calculate u and make sure u <= 1
	float t, u, v;
	u = T * P;
	if (u < 0.0f || u > det)
	{
		return pair<bool, Point3m>(false, Point3m(0, 0, 0));
	}
	// Q
	Point3m Q = T ^ E1;
	// Calculate v and make sure u + v <= 1
	v = dir * Q;
	if (v < 0.0f || u + v > det)
	{
		return pair<bool, Point3m>(false, Point3m(0, 0, 0));
	}
	// Calculate t, scale parameters, ray intersects triangle
	t = E2 * Q;
	float fInvDet = 1.0f / det;
	t *= fInvDet;
	u *= fInvDet;
	v *= fInvDet;

	if (t > 0)
	{
		return pair<bool, Point3m>(true, orig + dir * t);
	}
	else
	{
		return pair<bool, Point3m>(false, Point3m(0, 0, 0));
	}
}

bool UtilityTools::pointInTriangle(const Point3m& A, const Point3m& B, const Point3m& C, const Point3m& P)
{
	if (A == P)
	{
		return true;
	}
	if (B == P)
	{
		return true;
	}
	if (C == P)
	{
		return true;
	}

	Point3m v0 = C - A;
	Point3m v1 = B - A;
	Point3m v2 = P - A;

	float dot00 = v0 * v0;
	float dot01 = v0 * v1;
	float dot02 = v0 * v2;
	float dot11 = v1 * v1;
	float dot12 = v1 * v2;

	float inverDeno = 1 / (dot00 * dot11 - dot01 * dot01);

	float u = (dot11 * dot02 - dot01 * dot12) * inverDeno;
	if (u < 0 || u > 1) // if u out of range, return directly
	{
		return false;
	}

	float v = (dot00 * dot12 - dot01 * dot02) * inverDeno;
	if (v < 0 || v > 1) // if v out of range, return directly
	{
		return false;
	}

	return u + v <= 1;
}

vcg::Matrix44f UtilityTools::transformMeshByAxis(const Axis& src_axis, const Axis& dest_axis, MeshModel* _mesh)
{
	Axis curSys = src_axis, aimSys = dest_axis;
	return transformMeshByAxis(curSys, aimSys, _mesh);
}

vcg::Matrix44f UtilityTools::transformMeshByAxis(Axis& curSys, Axis& aimSys, MeshModel* _mesh)
{
	Point3m moveVector(0, 0, 0), rotateVector(0, 0, 0);
	curSys.computeTransformVectors(aimSys, moveVector, rotateVector);

	vcg::Matrix44f transMat;
	vcg::Matrix44f transform, transformInverse;
	transform.SetIdentity();

	transform.SetTranslate(curSys.centerPoint);
	transformInverse.SetTranslate(-curSys.centerPoint);

	vcg::Matrix44f transformValue, rotMatGlobal, rotMatLocal;
	transMat.SetIdentity();
	rotMatGlobal.SetIdentity();
	rotMatLocal.SetIdentity();
	Point3f trans;

	trans = curSys.axisXVector * moveVector.X();
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	trans = curSys.axisYVector * moveVector.Y();
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	trans = curSys.axisZVector * moveVector.Z();
	transformValue.SetTranslate(trans);
	transMat *= transformValue;

	transformValue.SetRotateDeg(rotateVector.X(), curSys.axisXVector);
	rotMatLocal *= transformValue;
	transformValue.SetRotateDeg(rotateVector.Y(), curSys.axisYVector);
	rotMatLocal *= transformValue;
	transformValue.SetRotateDeg(rotateVector.Z(), curSys.axisZVector);
	rotMatLocal *= transformValue;

	transform *= transMat * rotMatLocal;
	transform *= transformInverse;

	for (int i = 0; i < _mesh->cm.vert.size(); ++i)
	{
		_mesh->cm.vert[i].P() = transform * _mesh->cm.vert[i].P();
	}

	return transform;
}

QString UtilityTools::GetFileVersion(QString fileFullName)
{

	QString appVersion = QCoreApplication::applicationVersion();
	////获得文件基础信息 【Q tea  原创】
	////--------------------------------------------------------
	//bSuccess = VerQueryValue(lpData,
	//	(TEXT("\\VarFileInfo\\Translation")),
	//	&lpBuffer,
	//QString ver_value;
	//QString code;

	//code = "\\StringFileInfo\\" + strTranslation + "\\FileVersion";
	//bSuccess = VerQueryValue(lpData,
	//	(code.toStdWString().c_str()),
	//	&lpBuffer,
	//}
	//else
	return appVersion;
}

QString UtilityTools::getFileMd5(QString fileName)
{

	QFile file(fileName);//打开用户选择的文件
	if (file.open(QIODevice::ReadOnly))//以只读的方式打开文件并判断文件是否被成功打开
	{
		QByteArray array = file.readAll();//读取文件的所有内容
		//计算md5值，传入需要被计算的数据，传入计算的类型
		QByteArray md5 = QCryptographicHash::hash(array, QCryptographicHash::Md5);

		return md5.toHex();
	}

	return "";
}

QString UtilityTools::getDataMd5(QByteArray& data)
{
	QByteArray md5 = QCryptographicHash::hash(data, QCryptographicHash::Md5);
	return md5.toHex();
}

QMainWindow* UtilityTools::getSubMainWindow(QObject* pMainWindow)
{
    assert(pMainWindow);
    QMainWindow *pSubMainWindow = pMainWindow->findChild<QMainWindow*>("sub_main_window");
    assert(pSubMainWindow);

    return pSubMainWindow;
}

void UtilityTools::removeFilePath(QString filePath)
{
	if (filePath == QCoreApplication::applicationDirPath() || filePath == "")
		return;
	QDir::setCurrent(QCoreApplication::applicationDirPath());
	QDir dir(filePath);
	dir.removeRecursively();
}

Point3m  UtilityTools::catmull_Rom_SplinePoint_new(float u, vector<Point3m> subconctrl_P)
{
	float f0, f1, f2, f3;
	Point3m vert;
	f0 = (-0.5) * pow(u, 3) + pow(u, 2) + (-0.5 * u);
	f1 = 1.5 * pow(u, 3) - 2.5 * pow(u, 2) + 1;
	f2 = (-1.5) * pow(u, 3) + 2.0 * pow(u, 2) + 0.5 * u;
	f3 = 0.5 * pow(u, 3) - 0.5 * pow(u, 2);

	vert = subconctrl_P[0] * f0 + subconctrl_P[1] * f1 + subconctrl_P[2] * f2 + subconctrl_P[3] * f3;
	return vert;
}

vector<Point3m>  UtilityTools::catmull_Rom_SplinePoints_new(vector<Point3m> conctrl_P, int Unum)
{
	vector<Point3m> C_Line_vertices;
	vector<Point3m> New_conctrl_P;
	auto pointsnum = conctrl_P.size();
	auto P0 = conctrl_P[0] * 2 - conctrl_P[1];
	New_conctrl_P.push_back(P0);
	auto Pn = conctrl_P[pointsnum - 1] * 2 - conctrl_P[pointsnum - 2];
	for (int i = 0; i < pointsnum; i++)
	{
		New_conctrl_P.push_back(conctrl_P.at(i));
	}
	New_conctrl_P.push_back(Pn);
	pointsnum += 2;
	for (int i = 0; i < pointsnum; i++)
	{
		if (i < pointsnum - 3)
		{
			vector<Point3m> subcontrl;
			subcontrl.push_back(New_conctrl_P.at(i));
			subcontrl.push_back(New_conctrl_P.at(i + 1));
			subcontrl.push_back(New_conctrl_P.at(i + 2));
			subcontrl.push_back(New_conctrl_P.at(i + 3));
			float deltaU = 1.0 / float(Unum);
			for (int j = 0; j < Unum; j++)
			{
				auto Uu = j * deltaU;
				auto tempp = catmull_Rom_SplinePoint_new(Uu, subcontrl);
				C_Line_vertices.push_back(tempp);
			}
		}
	}
	C_Line_vertices.push_back(*--conctrl_P.end());
	return C_Line_vertices;
}

vector<Point3m> UtilityTools::catmull_Rom_SplinePoints_Equidistant(vector<Point3m> conctrl_P, float _spacing)
{
	vector<Point3m> fitting_nodes = catmull_Rom_SplinePoints_new(conctrl_P);

	vector<Point3m> result;
	Point3m interval_start, interval_end, interval_vec, p_one;
	int end_index = 1;
	result.push_back(fitting_nodes.front());
	p_one = fitting_nodes.front();
	do {
		interval_end = fitting_nodes[end_index];
		interval_vec = interval_end - p_one;

		float vec_length = sqrtf(interval_vec * interval_vec);
		if (vec_length >= _spacing)
		{
			float value = vec_length / _spacing;
			int itime = floor(value);
			Point3m vec = (interval_end - p_one).Normalize();
			for (int i = 1; i <= itime; ++i)
			{
				result.push_back(p_one + vec * _spacing * i);
			}

			if (end_index + 1 < fitting_nodes.size() && itime < value)
			{
				Point3m p1, pstart, pend, pvec, pedge;
				p1 = result.back();
				pstart = fitting_nodes[end_index];
				pend = fitting_nodes[end_index + 1];
				pvec = (pend - pstart).Normalize();
				pedge = pstart - p1;

				float b = 2.0f * (pedge * pvec);
				float c = pedge * pedge - _spacing * _spacing;
				if ((-b + sqrtf(b * b - 4 * c)) / 2.0f > 0)
				{
					float t = (-b + sqrtf(b * b - 4 * c)) / 2.0f;
					p_one = pstart + pvec * t;
					result.push_back(p_one);
				}
				else if ((-b - sqrtf(b * b - 4 * c)) / 2.0f > 0)
				{
					float t = (-b - sqrtf(b * b - 4 * c)) / 2.0f;
					p_one = pstart + pvec * t;
					result.push_back(p_one);
				}
			}
			else
			{
				p_one = result.back();
			}
		}
		++end_index;
	} while (end_index < fitting_nodes.size());
	result.push_back(fitting_nodes.back());
	return result;
}

// Mesh hole filling functionality removed - not used in FusionAnalyser
// 	// ... implementation removed ...

#include "data/fusionaligndata.h"
bool UtilityTools::checkToothFeaturePtsMarkFinished()
{
	bool successed = true;
	const auto& feaConfig = PFusionAlignData->getToothFeatureConfig().getMarkConfig();
	auto judge_func = [&](SegmentedStatusInfo* segment_info, int &size)->bool
	{
		bool succeed = false;
		if (segment_info == nullptr)
		{
			succeed = false;
			return succeed;
		}
		size = segment_info->feature_points_mark.size();

		//if(segment_info->feature_points_mark.size() == segment_info->toothFDIInfo.size())
		//{
			for (auto& mark : segment_info->feature_points_mark)
			{
				auto it = feaConfig.find(mark.first.toInt());
				if (it != feaConfig.end())
				{
					if (mark.second.landmarks.size() >= it->second.marks_.size())
					{
						succeed = true;
						continue;
					}
				}
				succeed = false;
				break;
			}
			if (segment_info->toothFDIInfo.size() != 0)
			{
				succeed = succeed && (segment_info->feature_points_mark.size() == segment_info->toothFDIInfo.size());
			}
		//}

		return succeed;
	};
	int upMarkedNum(0), lowMarkedNum(0);
	// Separation functionality removed - not used in FusionAnalyser
	return false;
}

bool UtilityTools::checkUpperToothFeaturePtsMarkFinished()
{
	bool successed = true;
	const auto& feaConfig = PFusionAlignData->getToothFeatureConfig().getMarkConfig();
	auto judge_func = [&](SegmentedStatusInfo* segment_info, int& size)->bool
	{
		bool succeed = false;
		if (segment_info == nullptr)
		{
			succeed = false;
			return succeed;
		}
		size = segment_info->feature_points_mark.size();

		//if(segment_info->feature_points_mark.size() == segment_info->toothFDIInfo.size())
		//{
		for (auto& mark : segment_info->feature_points_mark)
		{
			auto it = feaConfig.find(mark.first.toInt());
			if (it != feaConfig.end())
			{
				if (mark.second.landmarks.size() >= it->second.marks_.size())
				{
					succeed = true;
					continue;
				}
			}
			succeed = false;
			break;
		}
		if (segment_info->toothFDIInfo.size() != 0)
		{
			succeed = succeed && (segment_info->feature_points_mark.size() == segment_info->toothFDIInfo.size());
		}
		//}

		return succeed;
	};
	int upMarkedNum(0), lowMarkedNum(0);
	// Separation functionality removed - not used in FusionAnalyser
	return false;
}

bool UtilityTools::checkLowerToothFeaturePtsMarkFinished()
{
	bool successed = true;
	const auto& feaConfig = PFusionAlignData->getToothFeatureConfig().getMarkConfig();
	auto judge_func = [&](SegmentedStatusInfo* segment_info, int& size)->bool
	{
		bool succeed = false;
		if (segment_info == nullptr)
		{
			succeed = false;
			return succeed;
		}
		size = segment_info->feature_points_mark.size();

		//if(segment_info->feature_points_mark.size() == segment_info->toothFDIInfo.size())
		//{
		for (auto& mark : segment_info->feature_points_mark)
		{
			auto it = feaConfig.find(mark.first.toInt());
			if (it != feaConfig.end())
			{
				if (mark.second.landmarks.size() >= it->second.marks_.size())
				{
					succeed = true;
					continue;
				}
			}
			succeed = false;
			break;
		}
		if (segment_info->toothFDIInfo.size() != 0)
		{
			succeed = succeed && (segment_info->feature_points_mark.size() == segment_info->toothFDIInfo.size());
		}
		//}

		return succeed;
	};
	int upMarkedNum(0), lowMarkedNum(0);
	// Separation functionality removed - not used in FusionAnalyser
	return false;
}

QString UtilityTools::getWorkingDir()
{
	QSettings settings;
	if (!settings.value("FusionAnalyser WorkSpace").isValid())
	{
		QString file_path = QFileDialog::getExistingDirectory(nullptr, tr("select the work space"), "./");
		settings.setValue("FusionAnalyser WorkSpace", file_path);
	}

	return settings.value("FusionAnalyser WorkSpace").toString();
}

QString UtilityTools::getCorrectUnicode(const QByteArray& content)
 {
	QTextCodec::ConverterState state;
	QTextCodec* codec = QTextCodec::codecForName("UTF-8");
	QString text = codec->toUnicode(content.constData(), content.size(), &state);
	qDebug() << text;
	if (state.invalidChars > 0)
	{
		text = QTextCodec::codecForName("GBK")->toUnicode(content);
	}
	else
	{
		text = content;
	}
	QUrl url = QUrl::fromEncoded(text.toUtf8());
	text= url.toString();
	qDebug() << text;
	return text;
}

std::vector<QString> UtilityTools::getProjModelFiles(const QString& filename)
{
	std::vector<QString> model_files;
	QFile qf(filename);
	QFileInfo qfInfo(filename);
	QDir tmpDir = QDir::current();
	QDir::setCurrent(qfInfo.absoluteDir().absolutePath());
	if (!qf.open(QIODevice::ReadOnly))
		return std::vector<QString>();

	QString project_path = qfInfo.absoluteFilePath();

	QString errorMsg;
	QDomDocument doc("MeshLabDocument");    //It represents the XML document
	if (!doc.setContent(&qf, &errorMsg))
		return std::vector<QString>();

	QDomElement root = doc.documentElement();
	QDomNode node = root.firstChild();

	while (!node.isNull())
	{
		if (root.nodeName() == "MeshLabProject")
		{
			break;
		}
		node = node.nextSiblingElement();
	}
	//Devices
	while (!node.isNull())
	{
		if (QString::compare(node.nodeName(), "MeshGroup") == 0)
		{
			QDomNode mesh; QString filen, label;
			QString upperOrLowerJaw;
			mesh = node.firstChild();
			while (!mesh.isNull()) {
				//return true;
				filen = mesh.attributes().namedItem("filename").nodeValue();
				label = mesh.attributes().namedItem("label").nodeValue();
				upperOrLowerJaw = mesh.attributes().namedItem("UpperOrLowerJaw").nodeValue();

				if (!filen.isEmpty())
				{
					model_files.push_back(filen);
				}

				mesh = mesh.nextSibling();
			}
			break;
		}
	}
	return model_files;
}

QStringList UtilityTools::getNetParam(char* param, const char*tag)
{
	int tagLen = strlen(tag);
	if (tagLen >= 0)
		param += tagLen;
	QString parmstr(param);
	int index = parmstr.indexOf("/");
	parmstr = parmstr.left(index);
	QString param1 = UtilityTools::getInstance()->getCorrectUnicode(QByteArray(param, index));
	param1 = param1.left(param1.indexOf("\r"));
	QString params = QByteArray::fromBase64(param1.toLocal8Bit());

	//qDebug() << params << "\n";

	QStringList paraList = params.split("|");
	qDebug() << (paraList.size());

	return paraList;
}

bool UtilityTools::getAppIsLocal()
{
	QString fullPath = QCoreApplication::applicationDirPath() + "./configs/config.ini";
	QFileInfo fileInfo(fullPath);
	if (!fileInfo.exists())
		return false;

	QSettings hostsettings(QCoreApplication::applicationDirPath() + "./configs/config.ini", QSettings::IniFormat);

	bool value;
	if (!hostsettings.value(AppLocalConfig).isValid())
	{
		value = false;
		hostsettings.setValue(AppLocalConfig, value);
	}
	else
	{
		value = hostsettings.value(AppLocalConfig).toBool();
	}

	return value;
}

//eLanguageType UtilityTools::getAppLanguage()
//{
//
//}

//void UtilityTools::setAppLanguage(eLanguageType type)
//{
//	QSettings settings(
//		"HKEY_CURRENT_USER\\Software\\FusionalignLanguage",
//		QSettings::NativeFormat);
//
//	QString value = settings.value("Language").toInt();
//}

std::vector<vcg::Point3f> UtilityTools::sortBorderVertexs(const std::vector<vcg::Point3f>& pos_points)
{
	std::vector<vcg::Point3f> border_sorted;

	if (pos_points.empty())
	{
		return border_sorted;
	}

	KdTree<float>* kt = new KdTree<float>(vcg::ConstDataWrapper<vcg::Point3f>(pos_points.data(), pos_points.size()));

	if (kt == nullptr)
	{
		return border_sorted;
	}

	float radius = 0.5f;

	bool* visited = new bool[pos_points.size()];
	memset(visited, false, pos_points.size() * sizeof(bool));

	int seed_index_vertex = pos_points.size() / 2;
	// 搜索起始点保证是边界均匀的，即周边领域较少的（5）作为参考起始点
	for (int index = 0; index < pos_points.size(); ++index)
	{
		std::vector<unsigned int> index_points_in_sphere;
		std::vector<float> distance_points_in_sphere;
		kt->doQueryDist(pos_points[index], radius, index_points_in_sphere, distance_points_in_sphere);
		if (index_points_in_sphere.size() <= 5)
		{
			seed_index_vertex = index;
			break;
		}
	}

	border_sorted.push_back(pos_points[seed_index_vertex]);
	bool end_over = false;
	int num_visited = 1;

	Point3m temp_best_direct(0, 0, 0);
	vector<int> candidate_list;

	while (!end_over)
	{
		Point3f p = pos_points[seed_index_vertex];
		visited[seed_index_vertex] = true;

		std::vector<unsigned int> index_points_in_sphere;
		std::vector<float> distance_points_in_sphere;
		kt->doQueryDist(p, radius, index_points_in_sphere, distance_points_in_sphere);

		candidate_list.clear();
		if (border_sorted.size() >= 2)
		{
			temp_best_direct = (*(--border_sorted.end()) - *(----border_sorted.end())).Normalize();
		}
		else
		{
			temp_best_direct = Point3m(0, 0, 0);
		}

		float max_distance = -1;
		for (int it = 0; it < index_points_in_sphere.size(); ++it)
		{
			int index = index_points_in_sphere[it];

			if (!visited[index])
			{
				if (max_distance < distance_points_in_sphere[it])
				{
					max_distance = distance_points_in_sphere[it];
					seed_index_vertex = index;

					candidate_list.clear();
					candidate_list.push_back(index);
				}
				else if (max_distance == distance_points_in_sphere[it])
				{
					candidate_list.push_back(index);
				}

				visited[index] = true;
				++num_visited;
			}
		}

		if (max_distance < 0)
		{
			if (num_visited < pos_points.size())
			{
				radius = radius * 2;

				if (radius > 4.0f) // 远远超出正常的球体搜索半径，说明有个别点应该忽略防止乱序
				{
					end_over = true;
					border_sorted.pop_back();// 抛出最后一颗偏离起始点的数据，解决部分打圈现象
				}
			}
			else
			{
				end_over = true;
			}
		}
		else
		{
			if (candidate_list.size() > 1 && temp_best_direct != Point3m(0, 0, 0))
			{
				int best_index = seed_index_vertex;
				Point3m cvector;
				float max_cos_value = 0;
				for (auto cindex : candidate_list)
				{
					cvector = (pos_points[cindex] - p).Normalize();
					float cos_value = cvector * temp_best_direct;

					if (cos_value > max_cos_value)
					{
						max_cos_value = cos_value;
						best_index = cindex;
					}
				}
				seed_index_vertex = best_index;
			}

			border_sorted.push_back(pos_points[seed_index_vertex]);
		}
	}

	int counter_time = 0;
	int ipre, inext;
	Point3m vec_pre, vec_next;
	bool adj_happend = false;
	do
	{
		adj_happend = false;
		for (int i = 0; i < border_sorted.size(); ++i)
		{
			ipre = i - 1;
			ipre = ipre < 0 ? (border_sorted.size() - 1) : ipre;

			inext = i + 1;
			inext = inext >= border_sorted.size() ? 0 : inext;

			vec_pre = (border_sorted[i] - border_sorted[ipre]).Normalize();
			vec_next = (border_sorted[inext] - border_sorted[i]).Normalize();

			if (vec_pre * vec_next < 0)
			{
				border_sorted[i] = (border_sorted[ipre] + border_sorted[inext]) / 2.0f;
				adj_happend = true;
			}
		}

		++counter_time;
	} while (adj_happend && counter_time < 10);

	delete kt;
	kt = nullptr;

	delete visited;
	visited = nullptr;

	return border_sorted;
}

float UtilityTools::getTangentBallRadius(Point3m _v, Point3m _Nv, Point3m _p, Point3m _Np)
{
	Point3m vp = _v - _p;

	if (vp * _Nv >= 0)
	{
		return -1;
	}

	float a = vp * _Np;
	float b = 1 - _Nv * _Np;

	if (b < 1e-9)
	{
		return -1;
	}
	float radius = a / b;

	return a / b;
}

vector<FEdge> UtilityTools::getIntersectionRingOfTwoMesh(CMeshO* _mesh_a, CMeshO* _mesh_b, int _istart)
{
	if (!_mesh_a || !_mesh_b)
	{
		return vector<FEdge>();
	}

	int mesh_b_face_size = _mesh_b->face.size();
	vector<Point3m> mesh_b_face_center_list;
	for (int i = 0; i < mesh_b_face_size; ++i)
	{
		Point3m temp_center = (_mesh_b->face[i].V(0)->P() + _mesh_b->face[i].V(1)->P() + _mesh_b->face[i].V(2)->P()) / 3.0f;
		mesh_b_face_center_list.push_back(temp_center);
	}
	KdTree<float> kt(vcg::ConstDataWrapper<Point3m>(mesh_b_face_center_list.data(), mesh_b_face_center_list.size()));

	FEdge intersect_edge;
	bool start_index_intersected = intersectFaceWithMesh(_istart, _mesh_a, _mesh_b, kt, intersect_edge);
	if (!start_index_intersected)
	{
		return vector<FEdge>();
	}

	//...

	vector<FEdge> ring;
	return ring;
}

bool UtilityTools::intersectFaceWithMesh(int _iface, CMeshO* _mesh_a, CMeshO* _mesh_b, KdTree<float>& _kt, FEdge& _intersect_edge)
{
	int search_size = 10;
	Point3m face_center = (_mesh_a->face[_iface].V(0)->P() + _mesh_a->face[_iface].V(1)->P() + _mesh_a->face[_iface].V(2)->P()) / 3.0f;
	typename KdTree<float>::PriorityQueue pq;
	_kt.doQueryK(face_center, search_size + 1, pq);
	vector<int> klist;
	for (int j = 0; j < search_size + 1; ++j)
	{
		int index = pq.getIndex(j);
		klist.push_back(index);
	}

	for (auto& index : klist)
	{

	}

	return false;
}

void UtilityTools::occlusionColoringTwoMesh(CMeshO* _upper_mesh, CMeshO* _lower_mesh)
{
	if (_upper_mesh == nullptr || _lower_mesh == nullptr)
	{
		return;
	}

	static CMeshO* last_upper_mesh = nullptr;
	static CMeshO* last_lower_mesh = nullptr;
	static vector<float> upper_verts_deepth;
	static vector<float> lower_verts_deepth;

	bool recompute = false;
	float sum = _upper_mesh->vert.size() + _lower_mesh->vert.size();
	if (last_upper_mesh != _upper_mesh || last_lower_mesh != _lower_mesh)
	{
		sum += _upper_mesh->vert.size();
		sum += _lower_mesh->vert.size();
		recompute = true;
	}

	int icount = 0;

	if (recompute)
	{
		last_upper_mesh = _upper_mesh;
		vector<float>().swap(lower_verts_deepth);

		vector<CloudNode> node_cloud_upper;
		for (int i = 0; i < _upper_mesh->vert.size(); ++i)
		{
			node_cloud_upper.push_back(CloudNode(i, _upper_mesh->vert[i].P(), _upper_mesh->vert[i].N()));
		}
		BilateralFilterCSR csr_upper(&node_cloud_upper);
		for (int i = 0; i < _lower_mesh->vert.size(); ++i)
		{
			float func_value = csr_upper.getDistanceValue(_lower_mesh->vert[i].P());
			lower_verts_deepth.push_back(func_value);

			icount += 1;
			float t = 100.0f * (icount / sum);
			emit PSIGNALMANAGER->setProgressBarSignal(t, QString(tr("Analyzing")));
		}
	}

	if (lower_verts_deepth.size() == _lower_mesh->vert.size())
	{
		float func_value;
		for (int i = 0; i < _lower_mesh->vert.size(); ++i)
		{
			func_value = lower_verts_deepth[i];
			if (1.0f < func_value && func_value <= 1.2f)
			{
				_lower_mesh->vert[i].C() = vcg::Color4b(21, 21, 234, 255);
			}
			else if (0.8f < func_value && func_value <= 1.0f)
			{
				_lower_mesh->vert[i].C() = vcg::Color4b(20, 139, 226, 255);
			}
			else if (0.6f < func_value && func_value <= 0.8f)
			{
				_lower_mesh->vert[i].C() = vcg::Color4b(21, 234, 191, 255);
			}
			else if (0.4f < func_value && func_value <= 0.6f)
			{
				_lower_mesh->vert[i].C() = vcg::Color4b(191, 234, 21, 255);
			}
			else if (0.2f < func_value && func_value <= 0.4f)
			{
				_lower_mesh->vert[i].C() = vcg::Color4b(234, 207, 21, 255);
			}
			else if (0 < func_value && func_value <= 0.2f)
			{
				_lower_mesh->vert[i].C() = vcg::Color4b(234, 138, 21, 255);
			}
			else if (func_value <= 0)
			{
				_lower_mesh->vert[i].C() = vcg::Color4b(255, 0, 0, 255);
			}

			/*if (func_value <= 0)
			{
				func_value = abs(func_value);
				float t = func_value / 1.0f;
				if (t > 1.0f)
				{
					t = 1.0f;
				}
				_lower_mesh->vert[i].C() = vcg::Color4b(t * 255.0f, (1 - t) * 255.0f, 0, 255);
			}*/

			icount += 1;
			float t = 100.0f * (icount / sum);
			emit PSIGNALMANAGER->setProgressBarSignal(t, QString(tr("Analyzing")));
		}
	}

	if (recompute)
	{
		last_lower_mesh = _lower_mesh;
		vector<float>().swap(upper_verts_deepth);

		vector<CloudNode> node_cloud_lower;
		for (int i = 0; i < _lower_mesh->vert.size(); ++i)
		{
			node_cloud_lower.push_back(CloudNode(i, _lower_mesh->vert[i].P(), _lower_mesh->vert[i].N()));
		}
		BilateralFilterCSR csr_lower(&node_cloud_lower);
		for (int i = 0; i < _upper_mesh->vert.size(); ++i)
		{
			float func_value = csr_lower.getDistanceValue(_upper_mesh->vert[i].P());
			upper_verts_deepth.push_back(func_value);

			icount += 1;
			float t = 100.0f * (icount / sum);
			emit PSIGNALMANAGER->setProgressBarSignal(t, QString(tr("Analyzing")));
		}
	}

	if (upper_verts_deepth.size() == _upper_mesh->vert.size())
	{
		float func_value;
		for (int i = 0; i < _upper_mesh->vert.size(); ++i)
		{
			func_value = upper_verts_deepth[i];
			if (1.0f < func_value && func_value <= 1.2f)
			{
				_upper_mesh->vert[i].C() = vcg::Color4b(21, 21, 234, 255);
			}
			else if (0.8f < func_value && func_value <= 1.0f)
			{
				_upper_mesh->vert[i].C() = vcg::Color4b(20, 139, 226, 255);
			}
			else if (0.6f < func_value && func_value <= 0.8f)
			{
				_upper_mesh->vert[i].C() = vcg::Color4b(21, 234, 191, 255);
			}
			else if (0.4f < func_value && func_value <= 0.6f)
			{
				_upper_mesh->vert[i].C() = vcg::Color4b(191, 234, 21, 255);
			}
			else if (0.2f < func_value && func_value <= 0.4f)
			{
				_upper_mesh->vert[i].C() = vcg::Color4b(234, 207, 21, 255);
			}
			else if (0 < func_value && func_value <= 0.2f)
			{
				_upper_mesh->vert[i].C() = vcg::Color4b(234, 138, 21, 255);
			}
			else if (func_value <= 0)
			{
				_upper_mesh->vert[i].C() = vcg::Color4b(255, 0, 0, 255);
			}

			/*if (func_value <= 0)
			{
				func_value = abs(func_value);
				float t = func_value / 1.0f;
				if (t > 1.0f)
				{
					t = 1.0f;
				}
				_upper_mesh->vert[i].C() = vcg::Color4b(t * 255.0f, (1 - t) * 255.0f, 0, 255);
			}*/

			icount += 1;
			float t = 100.0f * (icount / sum);
			emit PSIGNALMANAGER->setProgressBarSignal(t, QString(tr("Analyzing")));
		}
	}
	emit PSIGNALMANAGER->setProgressBarSignal(100, QString(tr("Analyzing")));
	emit PSIGNALMANAGER->setProgressBarSignal(0, QString(tr("Analyzing")));
	emit PSIGNALMANAGER->setProgressBarSignal(0, QString(tr("Analyzing")));

	emit PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
}

CloudOctree* UtilityTools::createMeshOctree(MeshModel* m)
{
	if (m == nullptr) return nullptr;
	vector<CloudNode> nodes;
	int index = 0;
	for (auto& face : m->cm.face)
	{
		nodes.push_back(CloudNode(index, (face.V(0)->P() + face.V(1)->P() + face.V(2)->P()) / 3.0f, face.N()));
		++index;
	}

	vcg::tri::UpdateBounding<CMeshO>::Box(m->cm);
	vcg::Box3f box = m->cm.bbox;

	CloudOctree* octree_ = new CloudOctree(nodes, box, nullptr);
	octree_->initalLayerBall(nodes);
	return octree_;
}

void UtilityTools::unitCorrectionOfDentalModel(CMeshO* _mesh)
{
	if (_mesh == nullptr)
	{
		return;
	}
	vcg::tri::UpdateBounding<CMeshO>::Box(*_mesh);
	float dim = (_mesh->bbox.max - _mesh->bbox.min).Norm();
	if (dim >= 10)
	{
		return;
	}

	float adj_value = 1;
	if (1 < dim && dim < 10)
	{
		adj_value = 100;
	}
	else if (0 < dim && dim < 1)
	{
		adj_value = 1000;
	}
	for (int i = 0; i < _mesh->vert.size(); ++i)
	{
		_mesh->vert[i].P() *= adj_value;
	}
	return;
}

std::string UtilityTools::wstring2string(wstring wstr)
{
	char* strLocale = setlocale(LC_ALL, ".936");
	const wchar_t* wchSrc = wstr.c_str();
#if PLATFORM_WINDOWS
_locale_t locale = _get_current_locale();

#else

#endif
	size_t nDestSize = wcstombs(NULL, wchSrc, 0);
	char* chDest = new char[nDestSize + 1];
	memset(chDest, 0, nDestSize);
	wcstombs(chDest, wchSrc, nDestSize + 1);
	std::string strResult = chDest;
	delete[]chDest;
	//setlocale(LC_ALL, strLocale.c_str());
	return strResult;
}

std::wstring UtilityTools::string2wstring(string str)
{
	char* strLocale = setlocale(LC_ALL, ".936");
	const char* chSrc = str.c_str();

#if PLATFORM_WINDOWS
_locale_t locale = _get_current_locale();

#else

#endif

	size_t nDestSize = mbstowcs(NULL, chSrc, 0);
	wchar_t* chDest = new wchar_t[nDestSize + 1];
	memset(chDest, 0, nDestSize);
	mbstowcs(chDest, chSrc, nDestSize + 1);
	std::wstring strResult = chDest;
	delete[]chDest;
	//setlocale(LC_ALL, strLocale.c_str());
	return strResult;
}
