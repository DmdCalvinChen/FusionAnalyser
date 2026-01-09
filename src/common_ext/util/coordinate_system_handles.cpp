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

#include "coordinate_system_handles.h"

CoordinateSystemHandles::CoordinateSystemHandles()
{
}

CoordinateSystemHandles::CoordinateSystemHandles(Point3f _originP, Point3f _xHandle, Point3f _yHandle, Point3f _zHandle)
{
	this->originP = _originP;
	this->xHandle = _xHandle;
	this->yHandle = _yHandle;
	this->zHandle = _zHandle;
	this->initalCoordinateSys();
}

CoordinateSystemHandles::~CoordinateSystemHandles()
{
}

void CoordinateSystemHandles::update(Point3f _originP, Point3f _xHandle, Point3f _yHandle, Point3f _zHandle)
{
	this->originP = _originP;
	this->xHandle = _xHandle;
	this->yHandle = _yHandle;
	this->zHandle = _zHandle;
	this->initalCoordinateSys();
}

void CoordinateSystemHandles::initalCoordinateSys()
{
	Point3m axisX, axisY, axisZ;
	axisX = this->xHandle - this->originP;
	axisY = this->yHandle - this->originP;
	axisZ = this->zHandle - this->originP;
	this->coorSystem = Axis(this->originP, axisX, axisY, axisZ);
}

void CoordinateSystemHandles::transformTo(CoordinateSystemHandles aim, Point3f &move, Point3f &rotate, Point3f &zoom)
{
	this->transformTo(aim, move, rotate);

	Point3m transOrigin, transXhandle, transYhandle, transZhandle;
	vcg::Matrix44f transform, transformInverse;
	vcg::Matrix44f transMat, rotMatLocal, transformValue;

	//////////Translate//////////
	Point3f trans;
	transMat.SetIdentity();
	rotMatLocal.SetIdentity();
	transform.SetIdentity();
	transformInverse.SetIdentity();
	transform.SetTranslate(this->originP);
	transformInverse.SetTranslate(-this->originP);
	trans = this->coorSystem.axisXVector * move.X();
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	trans = this->coorSystem.axisYVector * move.Y();
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	trans = this->coorSystem.axisZVector * move.Z();
	transformValue.SetTranslate(trans);
	transMat *= transformValue;
	transform *= transMat;
	transform *= transformInverse;

	transOrigin = transform * this->originP;
	transXhandle = transform * this->xHandle;
	transYhandle = transform * this->yHandle;
	transZhandle = transform * this->zHandle;
	////////////Rotate////////////
	transMat.SetIdentity();
	rotMatLocal.SetIdentity();
	transform.SetIdentity();
	transformInverse.SetIdentity();
	transform.SetTranslate(transOrigin);
	transformInverse.SetTranslate(-transOrigin);
	transformValue.SetRotateDeg(rotate.X(), this->coorSystem.axisXVector);
	rotMatLocal *= transformValue;
	transformValue.SetRotateDeg(rotate.Y(), this->coorSystem.axisYVector);
	rotMatLocal *= transformValue;
	transformValue.SetRotateDeg(rotate.Z(), this->coorSystem.axisZVector);
	rotMatLocal *= transformValue;
	transform *= rotMatLocal;
	transform *= transformInverse;

	transOrigin = transform * this->originP;
	transXhandle = transform * this->xHandle;
	transYhandle = transform * this->yHandle;
	transZhandle = transform * this->zHandle;
	////////////Compute Zoom Values////////////
	float zoomX, zoomY, zoomZ;
	float curLength, aimLength;
	Point3m segmentThis, segmentAim;

	segmentThis = transXhandle - transOrigin;
	segmentAim = aim.xHandle - aim.originP;
	curLength = sqrtf(segmentThis * segmentThis);
	aimLength = sqrtf(segmentAim * segmentAim);
	if (segmentThis * segmentAim < 0)
	{
		aimLength *= -1;
	}
	zoomX = aimLength / curLength;

	segmentThis = transYhandle - transOrigin;
	segmentAim = aim.yHandle - aim.originP;
	curLength = sqrtf(segmentThis * segmentThis);
	aimLength = sqrtf(segmentAim * segmentAim);
	if (segmentThis * segmentAim < 0)
	{
		aimLength *= -1;
	}
	zoomY = aimLength / curLength;

	segmentThis = transZhandle - transOrigin;
	segmentAim = aim.zHandle - aim.originP;
	curLength = sqrtf(segmentThis * segmentThis);
	aimLength = sqrtf(segmentAim * segmentAim);
	if (segmentThis * segmentAim < 0)
	{
		aimLength *= -1;
	}
	zoomZ = aimLength / curLength;

	zoom = Point3f(zoomX, zoomY, zoomZ);
	//zoom = Point3f(1, 1, 1);
}

void CoordinateSystemHandles::transformTo(const CoordinateSystemHandles &aim, Point3f &move, Point3f &rotate)
{
	vcg::Matrix44f transformValue, rotMatLocal;
	rotMatLocal.SetIdentity();

	Point3m sCenter, sX, sY, sZ;
	Point3m eCenter, eX, eY, eZ;
	float l1, l2, l3, m1, m2, m3, n1, n2, n3;
	float A, B, C;//x, y, zҪתĽǶ
	float a, b, c;//x, y, zƽ
	float F = PI / 180.0f, D = PI;

	sX = this->coorSystem.axisXVector;
	sY = this->coorSystem.axisYVector;
	sZ = this->coorSystem.axisZVector;
	eX = aim.coorSystem.axisXVector;
	eY = aim.coorSystem.axisYVector;
	eZ = aim.coorSystem.axisZVector;
	Point3m rotateEX = sX;

	l1 = eX * sX;
	l2 = eY * sX;
	l3 = eZ * sX;
	m1 = eX * sY;
	m2 = eY * sY;
	m3 = eZ * sY;
	n1 = eX * sZ;
	n2 = eY * sZ;
	n3 = eZ * sZ;

	if (eZ * sZ < 0)
	{
		l3 *= -1;
	}

	sCenter = this->coorSystem.centerPoint;
	eCenter = aim.coorSystem.centerPoint;

	Point3m translateV = eCenter - sCenter;
	a = translateV * sX;
	b = translateV * sY;
	c = translateV * sZ;

	float m, n;//endZAxis WϵͶӰֱsYsZϵķ
	Point3m projEZVert = getProjPointOnPlane(eZ, -sX, Point3m(0, 0, 0), sX);
	m = projEZVert * sY;
	n = projEZVert * sZ;
	A = atan(m / n);
	A = A / F;
	transformValue.SetRotateDeg(-A, sX);
	rotMatLocal *= transformValue;

	B = acos(l3);
	B = B - D / 2.0f;
	B /= F;
	transformValue.SetRotateDeg(-B, sY);
	rotMatLocal *= transformValue;

	rotateEX = rotMatLocal * rotateEX;
	rotateEX = rotateEX.Normalize();
	float temp = rotateEX * eX;
	if (eZ * sZ < 0)
	{
		temp *= -1;
	}
	if (abs(temp - 1) < 1e-6)
	{
		temp = 1;
	}
	C = acos(temp);
	C /= F;
	if ((rotateEX ^ eX) * eZ > 0)
	{
		C *= -1;
	}

	move = Point3m(a, b, c);
	rotate = Point3m(-A, -B, -C);
}

Point3m CoordinateSystemHandles::getProjPointOnPlane(Point3m p, Point3m D, Point3f pos, Point3m N)
{
	Point3m result;

	float up, down;
	up = (pos - p) * N;
	down = D * N;
	float t = up / down;

	result = p + D * t;
	return result;
}
