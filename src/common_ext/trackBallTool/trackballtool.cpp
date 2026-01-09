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

#include "TrackBallTool/trackballtool.h"
#include <QMouseEvent>
#include "GL/glew.h"
#include <vcg/space/point3.h>
#include "wrap/gl/addons.h"
#include "common/meshmodel.h"
#include "common_base/SignalManager.h"
#include <common_ext/util/utility_tools.h>

TrackBallTool::TrackBallTool()
{

}

TrackBallTool::~TrackBallTool()
{

	std::vector<Point3m>().swap(circlePointYOZ);
	std::vector<Point3m>().swap(circlePointZOX);
	std::vector<Point3m>().swap(circlePointXOY);
}

void TrackBallTool::initAxis(const Axis& _axis)
{
	this->axis = _axis;
	transInfo.SetZero();
	rotateInfo.SetZero();
	updateCtrlHandles();
	updateLocalCircle();
}

void TrackBallTool::initAxis(const Axis& _axis, Axis* _changing_axis)
{
	this->axis = _axis;

	if (_changing_axis)
	{
		changing_axis = _changing_axis;
	}

	transInfo.SetZero();
	rotateInfo.SetZero();

	updateCtrlHandles();
	updateLocalCircle();
}

void TrackBallTool::setNeedChangeMesh(bool need)
{
	b_need_modify_mesh = need;
}

void TrackBallTool::initMesh(MeshModel* _mesh)
{
	if (_mesh)
	{
		this->mesh = _mesh;
	}

	transInfo.SetZero();
	rotateInfo.SetZero();

	vcg::tri::UpdateBounding<CMeshO>::Box(this->mesh->cm);

	// Set local coordinate system origin to bounding box center
	this->axis.centerPoint = this->mesh->cm.bbox.Center();

	updateCtrlHandles();
	updateLocalCircle();
}

void TrackBallTool::initMesh(MeshModel* _mesh, Axis& _axis, Axis* _changing_axis /* = nullptr */)
{
	if (_mesh)
	{
		this->mesh = _mesh;
	}

	this->axis = _axis;

	if (_changing_axis)
	{
		changing_axis = _changing_axis;
	}

	transInfo.SetZero();
	rotateInfo.SetZero();

	updateCtrlHandles();
	updateLocalCircle();
}

void TrackBallTool::updateCtrlHandles()
{
	translateHandles.clear();
	rotateHandles.clear();

	Point3m center, axisX, axisY, axisZ;
	center = axis.centerPoint;
	axisX = axis.axisXVector;
	axisY = axis.axisYVector;
	axisZ = axis.axisZVector;
	center = center +
		axisX * transInfo.X() +
		axisY * transInfo.Y() +
		axisZ * transInfo.Z();
	axis.centerPoint = center;
	translateHandles.push_back(center + axisX * fMangnifValue);
	translateHandles.push_back(center - axisX * fMangnifValue);
	translateHandles.push_back(center + axisY * fMangnifValue);
	translateHandles.push_back(center - axisY * fMangnifValue);
	translateHandles.push_back(center + axisZ * fMangnifValue);
	translateHandles.push_back(center - axisZ * fMangnifValue);

	float fRadius = fMangnifValue;
	translateHandlesTemp.clear();
	float beginPos = 1.4f, endPos = 1.6f;
	translateHandlesTemp.push_back(make_pair(
		center + axisX * fRadius * beginPos,
		center + axisX * fRadius * endPos));
	translateHandlesTemp.push_back(make_pair(
		center - axisX * fRadius * beginPos,
		center - axisX * fRadius * endPos));
	translateHandlesTemp.push_back(make_pair(
		center + axisY * fRadius * beginPos,
		center + axisY * fRadius * endPos));
	translateHandlesTemp.push_back(make_pair(
		center - axisY * fRadius * beginPos,
		center - axisY * fRadius * endPos));
	translateHandlesTemp.push_back(make_pair(
		center + axisZ * fRadius * beginPos,
		center + axisZ * fRadius * endPos));
	translateHandlesTemp.push_back(make_pair(
		center - axisZ * fRadius * beginPos,
		center - axisZ * fRadius * endPos));

	float xRotate, yRotate, zRotate;
	xRotate = rotateInfo.X();
	yRotate = rotateInfo.Y();
	zRotate = rotateInfo.Z();

	Point3m tempCtrl, tempCtrl2;
	// Calculate rotation control points around Z axis
	tempCtrl = translateHandles[XP];
	rotateOnePoint(center, axisZ, zRotate, tempCtrl);
	rotateHandles.push_back(tempCtrl);
	tempCtrl2.X() = 2 * center.X() - tempCtrl.X();
	tempCtrl2.Y() = 2 * center.Y() - tempCtrl.Y();
	tempCtrl2.Z() = 2 * center.Z() - tempCtrl.Z();
	rotateHandles.push_back(tempCtrl2);
	// Calculate rotation control points around X axis
	tempCtrl = translateHandles[YP];
	rotateOnePoint(center, axisX, xRotate, tempCtrl);
	rotateHandles.push_back(tempCtrl);
	tempCtrl2.X() = 2 * center.X() - tempCtrl.X();
	tempCtrl2.Y() = 2 * center.Y() - tempCtrl.Y();
	tempCtrl2.Z() = 2 * center.Z() - tempCtrl.Z();
	rotateHandles.push_back(tempCtrl2);
	// Calculate rotation control points around Y axis
	tempCtrl = translateHandles[ZP];
	rotateOnePoint(center, axisY, yRotate, tempCtrl);
	rotateHandles.push_back(tempCtrl);
	tempCtrl2.X() = 2 * center.X() - tempCtrl.X();
	tempCtrl2.Y() = 2 * center.Y() - tempCtrl.Y();
	tempCtrl2.Z() = 2 * center.Z() - tempCtrl.Z();
	rotateHandles.push_back(tempCtrl2);

	bCanDrawHandles = true;
}

void TrackBallTool::updateLocalCircle()
{
	std::vector<Point3m>().swap(
		circlePointXOY);
	std::vector<Point3m>().swap(
		circlePointYOZ);
	std::vector<Point3m>().swap(
		circlePointZOX);
	float circleRadius = 6.0;
	float costhita = 0.0f;
	float sinthita = 0.0f;
	vcg::Point3f p;
	for (int i = 0; i < CIRCLE_DIVIDE_NUMBER; ++i)
	{
		costhita = cos(STEP_RADS * i);
		sinthita = sin(STEP_RADS * i);
		p = axis.centerPoint + (
			axis.axisYVector * costhita +
			axis.axisZVector * sinthita) * circleRadius;
		circlePointYOZ.push_back(p);

		p = axis.centerPoint + (
			axis.axisZVector * costhita +
			axis.axisXVector * sinthita) * circleRadius;
		circlePointZOX.push_back(p);

		p = axis.centerPoint + (
			axis.axisXVector * costhita +
			axis.axisYVector * sinthita) * circleRadius;
		circlePointXOY.push_back(p);
	}
}

void TrackBallTool::Decorate(QGLWidget* gla, QPainter* p)
{
	updateOnce = false;

	if (this->parent != gla)
	{
		this->parent = gla;
	}

	if (!b_is_enabled || b_need_modify_mesh && this->mesh == nullptr)
	{
		return;
	}

	glEnable(GL_MULTISAMPLE);

	if (b_is_enabled && bAction)
	{
		adjustAction(ActionMode);
	}

	if (b_is_enabled && bCanHandles)
	{

		if (iOperateMode != NOTING_MODE && bCanDrawHandles)
		{
			generateActionVectorEvent(gla);
		}
		bCanHandles = false;
	}

	if (bCanDrawHandles && b_is_enabled)
	{
		drawIsAdjustingLocalToothAxis();
	}

}

void TrackBallTool::setEnabled(bool b_enable_show_status)
{
	b_is_enabled = b_enable_show_status;
}

void TrackBallTool::setScaleModeStatus(bool b_is_scale_mode /* = false */)
{
	if (b_is_scale_mode)
	{
		iOperateMode = SCALE_MODE;
	}
	else
	{
		iOperateMode = ROTATE_MODE;
	}
}

void TrackBallTool::mousePressEvent(QMouseEvent* event, QGLWidget* gla)
{

	if (!b_is_enabled)
	{
		return;
	}

	currentScreenPoint = event->pos();

	if (b_is_enabled)
	{
		mouseX = event->pos().x();
		mouseY = gla->height() - event->pos().y();

		mouseStart.X() = mouseX;
		mouseStart.Y() = mouseY;
		mouseStart.Z() = 0;
		bCanHandles = true;
	}

	gla->update();
}

void TrackBallTool::mouseMoveEvent(QMouseEvent* event, QGLWidget* gla)
{
	if (!b_is_enabled)
	{
		return;
	}

	currentScreenPoint = event->pos();

	if (b_is_enabled)
	{
		mouseX = event->pos().x();
		mouseY = gla->height() - event->pos().y();

		if (bActionReady)
		{
			mouseEnd.X() = mouseX;
			mouseEnd.Y() = mouseY;
			mouseEnd.Z() = 0;

			if (mouseEnd == mouseStart)
				bAction = false;
			else
				bAction = true;
		}
	}

	need_update_mesh_position = bAction;

	gla->update();
}

void TrackBallTool::mouseReleaseEvent(QMouseEvent* event, QGLWidget* gla)
{
	if (!b_is_enabled)
	{
		return;
	}

	currentScreenPoint = event->pos();

	if (bAction)
	{
		bEndAdjustAxis = true;
	}

	need_update_mesh_position = false;

	PSIGNALMANAGER->setGlobalTrackBallEnableSignal(true);
	pickEnabled = false;
	bActionReady = false;
	PickOnehandle = false;
	gla->update();
}

void TrackBallTool::rotateOnePoint(Point3m origionCopy,
	Point3m axis, float angle, Point3m& P)
{
	vcg::Matrix44f transform, transformInverse;
	transform.SetIdentity();
	transformInverse.SetIdentity();
	transform.SetTranslate(origionCopy);
	transformInverse.SetTranslate(-origionCopy);

	vcg::Matrix44f transformValue, rotMatLocal;
	rotMatLocal.SetIdentity();
	vcg::Point3f trans;

	transformValue.SetRotateDeg(angle, axis);
	rotMatLocal *= transformValue;

	transform *= rotMatLocal;
	transform *= transformInverse;

	P = transform * P;
}

vcg::Matrix44f TrackBallTool::getScaleMatrix(Point3m scale)
{
	return zoom(axis, scale);
}

vcg::Matrix44f TrackBallTool::zoom(Axis curSys, Point3m zoomVector)
{
	vcg::Matrix44f transMat;
	vcg::Matrix44f transform, transformInverse;
	transform.SetIdentity();
	transform.SetTranslate(curSys.centerPoint);
	transformInverse.SetTranslate(-curSys.centerPoint);

	vcg::Matrix44f transformValue, rotMatLocal, zoomMatLocal;
	transMat.SetIdentity();
	rotMatLocal.SetIdentity();
	zoomMatLocal.SetIdentity();

	transformValue = this->setZoomDeg(zoomVector.X(), curSys.axisXVector);
	zoomMatLocal *= transformValue;
	transformValue = this->setZoomDeg(zoomVector.Y(), curSys.axisYVector);
	zoomMatLocal *= transformValue;
	transformValue = this->setZoomDeg(zoomVector.Z(), curSys.axisZVector);
	zoomMatLocal *= transformValue;

	transform *= rotMatLocal * zoomMatLocal * transMat;
	transform *= transformInverse;

	return transform;

}

vcg::Matrix44f TrackBallTool::setZoomDeg(float value, Point3m axis)
{
	float rotateValue;
	Point3m A, B, rotateAxis;
	A = axis.Normalize();
	B = Point3m(1, 0, 0);
	rotateAxis = (B ^ A).Normalize();
	float dotMulit = A * B;
	if (abs(dotMulit) > 1)
	{
		if (dotMulit > 0)
		{
			dotMulit = 1.0f;
		}
		else
		{
			dotMulit = -1.0f;
		}
	}
	rotateValue = acosf(dotMulit);
	rotateValue = (rotateValue * 180.0f) / PI;

	vcg::Matrix44f rotMatLocal, rotMatReturn, zoomLocal;
	rotMatLocal.SetRotateDeg(rotateValue, rotateAxis);
	rotMatReturn.SetRotateDeg(-rotateValue, rotateAxis);
	zoomLocal.SetScale(value, 1, 1);
	return rotMatLocal * zoomLocal * rotMatReturn;
}

vcg::Matrix44f TrackBallTool::getTranslateMatrix(Point3m translate)
{
	vcg::Matrix44f transform;
	transform.SetIdentity();
	transform.SetTranslate(translate);
	return transform;
}

vcg::Matrix44f TrackBallTool::getRotateMatrix(Point3m origionCopy,
	Point3m axis, float angle)
{
	vcg::Matrix44f transform, transformInverse;
	transform.SetIdentity();
	transformInverse.SetIdentity();
	transform.SetTranslate(origionCopy);
	transformInverse.SetTranslate(-origionCopy);

	vcg::Matrix44f transformValue, rotMatLocal;
	rotMatLocal.SetIdentity();
	vcg::Point3f trans;

	transformValue.SetRotateDeg(angle, axis);
	rotMatLocal *= transformValue;

	transform *= rotMatLocal;
	transform *= transformInverse;

	return transform;
}

void TrackBallTool::generateActionVectorEvent(QGLWidget* gla)
{

	iSelectedHandle = -1;
	PickOnehandle = false;
	PickOnehandle = pickHandle(mouseX, mouseY, iSelectedHandle);
	if (PickOnehandle)
		PSIGNALMANAGER->setGlobalTrackBallEnableSignal(false);

	if (iSelectedHandle != -1)
		bActionReady = true;

	if (bActionReady)
	{
		if (iOperateMode == TRANSLATE_MODE)
		{
			switch (iSelectedHandle)
			{
			case XP:
				ActionMode = X_PLUS_TRANSLATE;
				break;
			case XM:
				ActionMode = X_MINS_TRANSLATE;
				break;
			case YP:
				ActionMode = Y_PLUS_TRANSLATE;
				break;
			case YM:
				ActionMode = Y_MINS_TRANSLATE;
				break;
			case ZP:
				ActionMode = Z_PLUS_TRANSLATE;
				break;
			case ZM:
				ActionMode = Z_MINS_TRANSLATE;
				break;

			default:
				break;
			}
		}
		else if (iOperateMode == ROTATE_MODE)
		{
			switch (iSelectedHandle)
			{
			case XP:
				ActionMode = X_PLUS_ROTATE;
				break;
			case XM:
				ActionMode = X_MINS_ROTATE;
				break;
			case YP:
				ActionMode = Y_PLUS_ROTATE;
				break;
			case YM:
				ActionMode = Y_MINS_ROTATE;
				break;
			case ZP:
				ActionMode = Z_PLUS_ROTATE;
				break;
			case ZM:
				ActionMode = Z_MINS_ROTATE;
				break;

			default:
				break;
			}
		}
		else if (iOperateMode == SCALE_MODE)
		{
			switch (iSelectedHandle)
			{
			case XP:
				ActionMode = X_PLUS_SCALE;
				break;
			case XM:
				ActionMode = X_MINS_SCALE;
				break;
			case YP:
				ActionMode = Y_PLUS_SCALE;
				break;
			case YM:
				ActionMode = Y_MINS_SCALE;
				break;
			case ZP:
				ActionMode = Z_PLUS_SCALE;
				break;
			case ZM:
				ActionMode = Z_MINS_SCALE;
				break;

			default:
				break;
			}
		}
	}
}

void TrackBallTool::computeMainActionVector(int iHandles, Point3m& mainVector)
{
	Point3m start, end;
	start = axis.centerPoint;

	if (iOperateMode == TRANSLATE_MODE)
	{
		switch (iSelectedHandle)
		{
		case XP:
			end = start + axis.axisXVector;
			break;
		case XM:
			end = start - axis.axisXVector;
			break;
		case YP:
			end = start + axis.axisYVector;
			break;
		case YM:
			end = start - axis.axisYVector;
			break;
		case ZP:
			end = start + axis.axisZVector;
			break;
		case ZM:
			end = start - axis.axisZVector;
			break;
		default:
			break;
		}
	}

	vcg::Point3d projStart, projEnd;
	projStart = unProjectPoint(start);
	projEnd = unProjectPoint(end);

	mainVector.X() = projEnd.X() - projStart.X();
	mainVector.Y() = projEnd.Y() - projStart.Y();
	mainVector.Z() = 0;
	bActionReady = true;
}

vcg::Point3d TrackBallTool::unProjectPoint(Point3m p)
{
	// Get gl state values
	double mm[16], mp[16];
	GLint vp[4];
	glPushMatrix();
	glMultMatrix(mesh->cm.Tr);

	glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	glGetDoublev(GL_PROJECTION_MATRIX, mp);
	glGetIntegerv(GL_VIEWPORT, vp);
	vcg::Point3d result(0, 0, 0);
	gluProject(p[0], p[1], p[2], mm, mp, vp, &result[0], &result[1], &result[2]);
	glPopMatrix();

	return result;
}

vcg::Point3d TrackBallTool::projectPoint(Point3m p)
{
	// Get gl state values
	double mm[16], mp[16];
	GLint vp[4];
	glPushMatrix();
	glMultMatrix(mesh->cm.Tr);

	glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	glGetDoublev(GL_PROJECTION_MATRIX, mp);
	glGetIntegerv(GL_VIEWPORT, vp);
	vcg::Point3d result(0, 0, 0);
	gluUnProject(p[0], p[1], p[2], mm, mp, vp, &result[0], &result[1], &result[2]);
	glPopMatrix();

	return result;
}

bool TrackBallTool::pickHandle(int x, int y, int& indexPicked)
{
	int width = 4;
	int height = 4;
	bool sorted = true;

	if (iOperateMode == TRANSLATE_MODE || iOperateMode == ROTATE_MODE)
	{
		std::vector<Point3f> control_points;
		for (int i = 0; i < 6; ++i)
		{
			control_points.push_back((translateHandlesTemp[i].first + translateHandlesTemp[i].second) * 0.5f);
		}
		for (int i = 0; i < 6; ++i)
		{
			control_points.push_back(rotateHandles[i]);
		}

		bool bPickOneControlTooth = UtilityTools::getInstance()->pickHandle(x, y, indexPicked, control_points, radiusControlAxis);

		if (!bPickOneControlTooth || indexPicked < 0)
		{
			return false;
		}
		else
		{
			if (indexPicked < 6)
			{
				iOperateMode = TRANSLATE_MODE;
			}
			else
			{
				iOperateMode = ROTATE_MODE;
				indexPicked -= int(Z_MINS_TRANSLATE) + 1;
			}

			return true;
		}
	}

	if (iOperateMode == SCALE_MODE)
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
		for (int i = 0; i < 6; ++i)
		{
			if (i % 2 != 0)
			{
				continue;
			}
			glLoadName(i);

			vcg::Add_Ons::glCuboid<vcg::Add_Ons::DMSolid>(translateHandlesTemp[i].first,
				translateHandlesTemp[i].second, cuboid_length, cuboid_width, cuboid_height);

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

}

void TrackBallTool::drawIsAdjustingLocalToothAxis()
{
	if (translateHandlesTemp.empty())
	{
		return;
	}

	glEnable(GL_MULTISAMPLE);
	// Enable anti-aliasing
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);  // Antialias the lines
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	if (iOperateMode == SCALE_MODE)
	{
		glEnable(GL_LIGHTING);

		for (int i = 0; i < 6; ++i)
		{

			if (i % 2 != 0)
			{
				continue;
			}

			if (i == 0 || i == 1)
			{
				glColor3f(0, 0, 1);
			}
			else if (i == 2 || i == 3)
			{
				glColor3f(1, 0, 0);
			}
			else
			{
				glColor3f(0, 1, 0);
			}

			vcg::Add_Ons::glCuboid<vcg::Add_Ons::DMSolid>(translateHandlesTemp[i].first,
				translateHandlesTemp[i].second, cuboid_length, cuboid_width, cuboid_height);

		}

		vcg::Point3f tv1, tv2;
		glEnable(GL_LINE_SMOOTH);
		glPushMatrix();
		glLineWidth(2.0f);
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		glColor3f(0, 0, 1);
		tv1 = translateHandlesTemp[XP].first;
		tv2 = axis.centerPoint;
		glVertex3f(tv1.X(), tv1.Y(), tv1.Z());
		glVertex3f(tv2.X(), tv2.Y(), tv2.Z());
		tv1 = translateHandlesTemp[YP].first;
		tv2 = axis.centerPoint;
		glColor3f(1, 0, 0);
		glVertex3f(tv1.X(), tv1.Y(), tv1.Z());
		glVertex3f(tv2.X(), tv2.Y(), tv2.Z());
		tv1 = translateHandlesTemp[ZP].first;
		tv2 = axis.centerPoint;
		glColor3f(0, 1, 0);
		glVertex3f(tv1.X(), tv1.Y(), tv1.Z());
		glVertex3f(tv2.X(), tv2.Y(), tv2.Z());
		glEnd();

		glLineWidth(1.0f);
		glPopMatrix();
	}
	else
	{
		glEnable(GL_LIGHTING);

		for (int i = 0; i < 6; ++i)
		{
			if (i == 0 || i == 1)
			{
				glColor3f(0, 0, 1);
			}
			else if (i == 2 || i == 3)
			{
				glColor3f(1, 0, 0);
			}
			else
			{
				glColor3f(0, 1, 0);
			}

			vcg::Add_Ons::glCone<vcg::Add_Ons::DMSolid>(translateHandlesTemp[i].first,
				translateHandlesTemp[i].second, widthCone);

		}

		vcg::Point3f tv1, tv2;
		glEnable(GL_LINE_SMOOTH);
		glPushMatrix();
		glLineWidth(2.0f);
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		glColor3f(0, 0, 1);
		tv1 = translateHandlesTemp[XP].first;
		tv2 = translateHandlesTemp[XM].first;
		glVertex3f(tv1.X(), tv1.Y(), tv1.Z());
		glVertex3f(tv2.X(), tv2.Y(), tv2.Z());
		tv1 = translateHandlesTemp[YP].first;
		tv2 = translateHandlesTemp[YM].first;
		glColor3f(1, 0, 0);
		glVertex3f(tv1.X(), tv1.Y(), tv1.Z());
		glVertex3f(tv2.X(), tv2.Y(), tv2.Z());
		tv1 = translateHandlesTemp[ZP].first;
		tv2 = translateHandlesTemp[ZM].first;
		glColor3f(0, 1, 0);
		glVertex3f(tv1.X(), tv1.Y(), tv1.Z());
		glVertex3f(tv2.X(), tv2.Y(), tv2.Z());
		glEnd();

		glLineWidth(1.0f);
		glPopMatrix();

		glEnable(GL_LIGHTING);
	}

	if (iOperateMode != SCALE_MODE)
	{
		float r = 0.85, g = 0.85, b = 0.85;
		glEnable(GL_LIGHTING);
		glColor3f(0, 0, b);
		vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(rotateHandles[XP], radiusControlAxis, slices, stacks);
		vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(rotateHandles[XM], radiusControlAxis, slices, stacks);

		glColor3f(r, 0, 0);
		vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(rotateHandles[YP], radiusControlAxis, slices, stacks);
		vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(rotateHandles[YM], radiusControlAxis, slices, stacks);

		glColor3f(0, g, 0);
		vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(rotateHandles[ZP], radiusControlAxis, slices, stacks);
		vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(rotateHandles[ZM], radiusControlAxis, slices, stacks);

		glPushMatrix();
		glLineWidth(2.0f);
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		glColor3f(0, 0, b);

		glVertex3f(rotateHandles[XP].X(), rotateHandles[XP].Y(), rotateHandles[XP].Z());
		glVertex3f(rotateHandles[XM].X(), rotateHandles[XM].Y(), rotateHandles[XM].Z());
		glEnd();

		glBegin(GL_LINES);
		glColor3f(r, 0, 0);
		glVertex3f(rotateHandles[YP].X(), rotateHandles[YP].Y(), rotateHandles[YP].Z());
		glVertex3f(rotateHandles[YM].X(), rotateHandles[YM].Y(), rotateHandles[YM].Z());
		glEnd();

		glBegin(GL_LINES);
		glColor3f(0, g, 0);
		glVertex3f(rotateHandles[ZP].X(), rotateHandles[ZP].Y(), rotateHandles[ZP].Z());
		glVertex3f(rotateHandles[ZM].X(), rotateHandles[ZM].Y(), rotateHandles[ZM].Z());
		glEnd();

		glLineWidth(1.0f);

		glEnable(GL_LINE_SMOOTH);
		glColor3f(r, 0.0f, 0.0f);
		glBegin(GL_LINE_STRIP);
		for (auto v : circlePointYOZ)
		{
			glVertex3f(v.X(), v.Y(), v.Z());
		}
		glEnd();

		glColor3f(0, g, 0.0f);
		glBegin(GL_LINE_STRIP);
		for (auto v : circlePointZOX)
		{
			glVertex3f(v.X(), v.Y(), v.Z());
		}
		glEnd();

		glColor3f(0.0f, 0.0f, b);
		glBegin(GL_LINE_STRIP);
		for (auto v : circlePointXOY)
		{
			glVertex3f(v.X(), v.Y(), v.Z());
		}
		glEnd();
		glPopMatrix();
		glEnable(GL_LIGHTING);

	}
	glPopAttrib();

}

void TrackBallTool::adjustAction(int actionMode)
{

	if (iOperateMode == TRANSLATE_MODE)
	{
		Point3m handleP, projLineV, normalV, normalV2, mixP;
		switch (actionMode)
		{
		case X_PLUS_TRANSLATE:
			handleP = translateHandlesTemp[actionMode].first;
			projLineV = axis.axisXVector;
			normalV = axis.axisYVector;
			normalV2 = axis.axisZVector;
			break;

		case X_MINS_TRANSLATE:
			handleP = translateHandlesTemp[actionMode].first;
			projLineV = -axis.axisXVector;
			normalV = axis.axisYVector;
			normalV2 = axis.axisZVector;
			break;

		case Y_PLUS_TRANSLATE:
			handleP = translateHandlesTemp[actionMode].first;
			projLineV = axis.axisYVector;
			normalV = axis.axisXVector;
			normalV2 = axis.axisZVector;
			break;

		case Y_MINS_TRANSLATE:
			handleP = translateHandlesTemp[actionMode].first;
			projLineV = -axis.axisYVector;
			normalV = axis.axisXVector;
			normalV2 = axis.axisZVector;
			break;

		case Z_PLUS_TRANSLATE:
			handleP = translateHandlesTemp[actionMode].first;
			projLineV = axis.axisZVector;
			normalV = axis.axisYVector;
			normalV2 = axis.axisXVector;
			break;

		case Z_MINS_TRANSLATE:
			handleP = translateHandlesTemp[actionMode].first;
			projLineV = -axis.axisZVector;
			normalV = axis.axisYVector;
			normalV2 = axis.axisXVector;
			break;

		default:
			break;
		}

		mixP = getMixedPointFromScreenToWorld2(mouseX, mouseY, normalV, normalV2, handleP);
		Point3m dragVector = mixP - handleP;
		float range = dragVector * projLineV;

		switch (actionMode)
		{
		case X_PLUS_TRANSLATE:
		{
			Point3m translate = axis.axisXVector * range;
			axis.centerPoint += translate;

			if (b_need_modify_mesh && this->mesh)
			{
				auto translate_mat = getTranslateMatrix(translate);
				transformAndUpdateMesh(translate_mat, this->mesh);
			}
		}

		break;

		case X_MINS_TRANSLATE:
		{
			Point3m translate = -axis.axisXVector * range;
			axis.centerPoint += translate;

			if (b_need_modify_mesh && this->mesh)
			{
				auto translate_mat = getTranslateMatrix(translate);
				transformAndUpdateMesh(translate_mat, this->mesh);
			}
		}
		break;

		case Y_PLUS_TRANSLATE:
		{
			Point3m translate = axis.axisYVector * range;
			axis.centerPoint += translate;

			if (b_need_modify_mesh && this->mesh)
			{
				auto translate_mat = getTranslateMatrix(translate);
				transformAndUpdateMesh(translate_mat, this->mesh);
			}
		}
		break;

		case Y_MINS_TRANSLATE:
		{
			Point3m translate = -axis.axisYVector * range;
			axis.centerPoint += translate;

			if (b_need_modify_mesh && this->mesh)
			{
				auto translate_mat = getTranslateMatrix(translate);
				transformAndUpdateMesh(translate_mat, this->mesh);
			}
		}
		break;

		case Z_PLUS_TRANSLATE:
		{
			Point3m translate = axis.axisZVector * range;
			axis.centerPoint += translate;

			if (b_need_modify_mesh && this->mesh)
			{
				auto translate_mat = getTranslateMatrix(translate);
				transformAndUpdateMesh(translate_mat, this->mesh);
			}
		}
		break;

		case Z_MINS_TRANSLATE:
		{
			Point3m translate = -axis.axisZVector * range;
			axis.centerPoint += translate;

			if (b_need_modify_mesh && this->mesh)
			{
				auto translate_mat = getTranslateMatrix(translate);
				transformAndUpdateMesh(translate_mat, this->mesh);
			}
		}
		break;

		default:
			break;
		}
		updateCtrlHandles();
	}
	else if (iOperateMode == ROTATE_MODE)
	{
		Point3m origin, normalV, mixP, handle, mainV;
		switch (actionMode)
		{
		case X_PLUS_ROTATE:
		case X_MINS_ROTATE:
			handle = rotateHandles[actionMode];
			origin = axis.centerPoint;
			normalV = axis.axisZVector;
			mainV = axis.axisXVector;
			mixP = getMixedPointFromScreenToWorld1(mouseX, mouseY, normalV, origin, mainV);
			break;

		case Y_PLUS_ROTATE:
		case Y_MINS_ROTATE:
			handle = rotateHandles[actionMode];
			origin = axis.centerPoint;
			normalV = axis.axisXVector;
			mainV = axis.axisYVector;
			mixP = getMixedPointFromScreenToWorld1(mouseX, mouseY, normalV, origin, mainV);
			break;

		case Z_PLUS_ROTATE:
		case Z_MINS_ROTATE:
			handle = rotateHandles[actionMode];
			origin = axis.centerPoint;
			normalV = axis.axisYVector;
			mainV = axis.axisZVector;
			mixP = getMixedPointFromScreenToWorld1(mouseX, mouseY, normalV, origin, mainV);
			break;

		default:
			break;
		}

		angle = 0;
		Point3m v1, v2;
		v1 = handle - origin;
		v2 = mixP - origin;
		v1 = v1.Normalize();
		v2 = v2.Normalize();
		Point3m tempNV = v1 ^ v2;
		angle = acos(v1 * v2);
		angle = float((angle * 180.0f) / PI);
		if (tempNV * normalV < 0)
			angle *= -1;

		if (isnan(angle))
		{
			angle = 0.0f;
		}
		rotateToothCoordinate(actionMode, angle);
		updateCtrlHandles(/*iCurrentSelectTooth*/);
	}
	else if (iOperateMode == SCALE_MODE)
	{
		Point3m handleP, projLineV, normalV, normalV2, mixP;
		switch (actionMode)
		{
		case X_PLUS_SCALE:
			handleP = translateHandlesTemp[actionMode].first;
			projLineV = axis.axisXVector;
			normalV = axis.axisYVector;
			normalV2 = axis.axisZVector;
			break;

		case X_MINS_SCALE:
			handleP = translateHandlesTemp[actionMode].first;
			projLineV = -axis.axisXVector;
			normalV = axis.axisYVector;
			normalV2 = axis.axisZVector;
			break;

		case Y_PLUS_SCALE:
			handleP = translateHandlesTemp[actionMode].first;
			projLineV = axis.axisYVector;
			normalV = axis.axisXVector;
			normalV2 = axis.axisZVector;
			break;

		case Y_MINS_SCALE:
			handleP = translateHandlesTemp[actionMode].first;
			projLineV = -axis.axisYVector;
			normalV = axis.axisXVector;
			normalV2 = axis.axisZVector;
			break;

		case Z_PLUS_SCALE:
			handleP = translateHandlesTemp[actionMode].first;
			projLineV = axis.axisZVector;
			normalV = axis.axisYVector;
			normalV2 = axis.axisXVector;
			break;

		case Z_MINS_SCALE:
			handleP = translateHandlesTemp[actionMode].first;
			projLineV = -axis.axisZVector;
			normalV = axis.axisYVector;
			normalV2 = axis.axisXVector;
			break;

		default:
			break;
		}
		mixP = getMixedPointFromScreenToWorld2(mouseX, mouseY, normalV, normalV2, handleP);
		Point3m dragVector = mixP - handleP;
		float range = dragVector * projLineV;

		switch (actionMode)
		{
		case X_PLUS_SCALE:
		{
			Point3m translate = axis.axisXVector * range;
			//axis.centerPoint += translate;

			if (b_need_modify_mesh && this->mesh)
			{
				auto translate_mat = getScaleMatrix(Point3m(1.0 + scale_factor * range, 1, 1));
				transformAndUpdateMesh(translate_mat, this->mesh);
			}

		}

		break;

		case X_MINS_SCALE:
		{
			Point3m translate = -axis.axisXVector * range;
			//axis.centerPoint += translate;

			if (b_need_modify_mesh && this->mesh)
			{
				auto translate_mat = getScaleMatrix(Point3m(1.0 - scale_factor * range, 1, 1));
				//transformAndUpdateMesh(translate_mat, this->mesh);
			}

		}
		break;

		case Y_PLUS_SCALE:
		{
			Point3m translate = axis.axisYVector * range;
			//axis.centerPoint += translate;

			if (b_need_modify_mesh && this->mesh)
			{
				auto translate_mat = getScaleMatrix(Point3m(1.0, 1.0 + scale_factor * range, 1));
				transformAndUpdateMesh(translate_mat, this->mesh);
			}

		}
		break;

		case Y_MINS_SCALE:
		{
			Point3m translate = -axis.axisYVector * range;
			//axis.centerPoint += translate;

			if (b_need_modify_mesh && this->mesh)
			{
				auto translate_mat = getScaleMatrix(Point3m(1.0, 1.0 - scale_factor * range, 1));
				//transformAndUpdateMesh(translate_mat, this->mesh);
			}

		}
		break;

		case Z_PLUS_SCALE:
		{
			Point3m translate = axis.axisZVector * range;
			//axis.centerPoint += translate;

			if (b_need_modify_mesh && this->mesh)
			{
				auto translate_mat = getScaleMatrix(Point3m(1.0, 1.0, 1.0 + scale_factor * range));
				transformAndUpdateMesh(translate_mat, this->mesh);
			}

		}
		break;

		case Z_MINS_SCALE:
		{
			Point3m translate = -axis.axisZVector * range;
			//axis.centerPoint += translate;

			if (b_need_modify_mesh && this->mesh)
			{
				auto translate_mat = getScaleMatrix(Point3m(1.0, 1.0, 1.0 - scale_factor * range));
				//transformAndUpdateMesh(translate_mat, this->mesh);
			}

		}
		break;

		default:
			break;
		}
		updateCtrlHandles();
	}
	else
		return;

	mouseStart = mouseEnd;
	updateLocalCircle();

	if (bEndAdjustAxis)
	{
		bEndAdjustAxis = false;
		bAction = false;
		updateOnce = true;
	}

	if (changing_axis)
	{
		*changing_axis = axis;
	}

	if (!b_need_modify_mesh && changing_axis)
	{
		*changing_axis = axis;
	}
	else
	{
		//this->mesh->axis_local_ = axis; todo
	}

	updateMesh(mesh);

}

void TrackBallTool::transformAndUpdateMesh(const vcg::Matrix44f& transform, MeshModel* current_mesh)
{
	for (auto& v : current_mesh->cm.vert)
	{
		v.P() = transform * v.P();
	}
}

void TrackBallTool::updateMesh(MeshModel* current_mesh)
{
	if (current_mesh == nullptr || !b_need_modify_mesh)
	{
		return;
	}

	// update Normals
	vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(current_mesh->cm);
	vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(current_mesh->cm);
}

void TrackBallTool::transformAndUpdatePoint(const vcg::Matrix44f& transform, Point3m& p)
{
	p = transform * p;
}

void TrackBallTool::rotateToothCoordinate(int iActionMode, float fAngle)
{
	Point3m center, axisX, axisY, axisZ;
	center = axis.centerPoint;
	axisX = axis.axisXVector;
	axisY = axis.axisYVector;
	axisZ = axis.axisZVector;
	Point3m xPoint, yPoint, zPoint;
	xPoint = center + axisX;
	yPoint = center + axisY;
	zPoint = center + axisZ;

	switch (iActionMode)
	{
	case X_PLUS_ROTATE:
	case X_MINS_ROTATE:

	{
		vcg::Matrix44f transform_mat = getRotateMatrix(center, axisZ, fAngle);
		transformAndUpdatePoint(transform_mat, xPoint);
		transformAndUpdatePoint(transform_mat, yPoint);

		if (b_need_modify_mesh && this->mesh)
		{
			transformAndUpdateMesh(transform_mat, this->mesh);
		}

		axis.axisXVector = xPoint - center;
		axis.axisYVector = yPoint - center;
	}
	break;

	case Y_PLUS_ROTATE:
	case Y_MINS_ROTATE:
	{
		vcg::Matrix44f transform_mat = getRotateMatrix(center, axisX, fAngle);
		transformAndUpdatePoint(transform_mat, yPoint);
		transformAndUpdatePoint(transform_mat, zPoint);

		if (b_need_modify_mesh && this->mesh)
		{
			transformAndUpdateMesh(transform_mat, this->mesh);
		}

		axis.axisZVector = zPoint - center;
		axis.axisYVector = yPoint - center;
	}
	break;

	case Z_PLUS_ROTATE:
	case Z_MINS_ROTATE:
	{
		vcg::Matrix44f transform_mat = getRotateMatrix(center, axisY, fAngle);
		transformAndUpdatePoint(transform_mat, xPoint);
		transformAndUpdatePoint(transform_mat, zPoint);

		if (b_need_modify_mesh && this->mesh)
		{
			transformAndUpdateMesh(transform_mat, this->mesh);
		}

		axis.axisXVector = xPoint - center;
		axis.axisZVector = zPoint - center;
	}
	break;

	default:
		break;
	}
}

Point3m TrackBallTool::getMixedPointFromScreenToWorld1(
	float mX, float mY, Point3m nV, Point3m hP, Point3m mainV)
{
	Point3m resultP;
	vcg::Point3d tempProjPoint;
	Point3m nearP, farP;

	tempProjPoint = projectPoint(Point3m(mX, mY, 0));
	nearP.X() = tempProjPoint.X();
	nearP.Y() = tempProjPoint.Y();
	nearP.Z() = tempProjPoint.Z();
	tempProjPoint = projectPoint(Point3m(mX, mY, 1));
	farP.X() = tempProjPoint.X();
	farP.Y() = tempProjPoint.Y();
	farP.Z() = tempProjPoint.Z();

	float t;
	Point3m fnP = farP - nearP;
	float base = fnP.X() * nV.X() + fnP.Y() * nV.Y() + fnP.Z() * nV.Z();
	if (base == 0)
	{
		resultP = getMixedPointFromScreenToWorldOnVerticalPlane(mX, mY, hP, mainV);
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

Point3m TrackBallTool::getMixedPointFromScreenToWorld2(
	float mX, float mY, Point3m nV1, Point3m nV2, Point3m hP)
{
	Point3m resultP;
	vcg::Point3d tempProjPoint;
	Point3m nearP, farP;

	tempProjPoint = projectPoint(Point3m(mX, mY, 0));
	nearP.X() = tempProjPoint.X();
	nearP.Y() = tempProjPoint.Y();
	nearP.Z() = tempProjPoint.Z();
	tempProjPoint = projectPoint(Point3m(mX, mY, 1));
	farP.X() = tempProjPoint.X();
	farP.Y() = tempProjPoint.Y();
	farP.Z() = tempProjPoint.Z();

	float t;
	Point3m fnP = farP - nearP;
	Point3m nV;
	Point3m fNPNormal = fnP;
	fNPNormal = fNPNormal.Normalize();

	float base1 = fNPNormal.dot(nV1);
	float base2 = fNPNormal.dot(nV2);
	float base;

	float threshold_value = 0.01f;

	if (fabs(base1) > threshold_value)
	{
		nV = nV1;
		base = fnP.dot(nV);
	}
	else if (fabs(base2) > threshold_value)
	{
		nV = nV2;
		base = fnP.dot(nV);
	}
	else
	{
		return hP;
	}

	t = float(((hP - nearP).dot(nV)) / base);

	if (t >= 0 && t <= 1)
	{
		resultP = nearP + fnP * t;
		return resultP;
	}
	else
		return hP;
}

Point3m TrackBallTool::getMixedPointFromScreenToWorldOnVerticalPlane(
	float mX, float mY, Point3m origin, Point3m mainV)
{
	Point3m resultP;
	vcg::Point3d tempProjPoint;
	Point3m nearP, farP;

	tempProjPoint = projectPoint(Point3m(parent->width() / 2.0f, parent->height() / 2.0f, 0));
	nearP.X() = tempProjPoint.X();
	nearP.Y() = tempProjPoint.Y();
	nearP.Z() = tempProjPoint.Z();
	tempProjPoint = projectPoint(Point3m(parent->width() / 2.0f, parent->height() / 2.0f, 1));
	farP.X() = tempProjPoint.X();
	farP.Y() = tempProjPoint.Y();
	farP.Z() = tempProjPoint.Z();

	Point3m normalVector = nearP - farP;
	normalVector = normalVector.Normalize();

	tempProjPoint = projectPoint(Point3m(mX, mY, 0));
	nearP.X() = tempProjPoint.X();
	nearP.Y() = tempProjPoint.Y();
	nearP.Z() = tempProjPoint.Z();
	tempProjPoint = projectPoint(Point3m(mX, mY, 1));
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
		float fRadius = fCtrlRadius;
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
