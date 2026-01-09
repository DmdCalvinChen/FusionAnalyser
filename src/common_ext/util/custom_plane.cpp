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

#include "custom_plane.h"

CustomPlane::CustomPlane()
{
}

CustomPlane::~CustomPlane()
{
}

void CustomPlane::initalPlane(Point3m locaPL, Point3m locaPM, Point3m locaPR, float width, float height)
{
	this->locaPL = locaPL;
	this->locaPM = locaPM;
	this->locaPR = locaPR;

	this->normalVector = getNormalVector(locaPL, locaPM, locaPR);
	this->center = getCenterPoint(locaPL, locaPM, locaPR);

	this->axisYV = locaPM - center;
	this->axisZV = normalVector;
	this->axisXV = axisYV^axisZV;
	axisXV = axisXV.Normalize();
	axisYV = axisYV.Normalize();
	axisZV = axisZV.Normalize();
	this->normalVector = normalVector.Normalize();

	this->fWidth = width;
	this->fHeight = height;
	float halfWidth, halfHeight;
	halfWidth = width / 2.0f;
	halfHeight = height / 2.0f;
	this->ctrlV1 = center + axisXV * halfWidth * 2.0f + axisYV * halfHeight * 2.0f;
	this->ctrlV2 = center - axisXV * halfWidth * 2.0f + axisYV * halfHeight * 2.0f;
	this->ctrlV3 = center - axisXV * halfWidth * 2.0f - axisYV * halfHeight * 2.0f;
	this->ctrlV4 = center + axisXV * halfWidth * 2.0f - axisYV * halfHeight * 2.0f;
	this->ctrlV1Original = this->ctrlV1;
	this->ctrlV2Original = this->ctrlV2;
	this->ctrlV3Original = this->ctrlV3;
	this->ctrlV4Original = this->ctrlV4;
	this->centerOriginal = this->center;
	this->d = -locaPL.dot(this->normalVector);
}

Point3m CustomPlane::getNormalVector(Point3m pl, Point3m pm, Point3m pr)
{
	Point3m RVector = pr - pm;
	Point3m LVector = pl - pm;
	return LVector ^ RVector;
}

Point3m CustomPlane::getCenterPoint(Point3m pl, Point3m pm, Point3m pr)
{
	Point3m result = (pl + pr) / 2.0f;
	return result;
}

void CustomPlane::TranslatePlaneAlongNormal(float T)
{
	this->ctrlV1 += this->normalVector * T;
	this->ctrlV2 += this->normalVector * T;
	this->ctrlV3 += this->normalVector * T;
	this->ctrlV4 += this->normalVector * T;
	this->center += this->normalVector * T;
	this->d = -this->ctrlV1.dot(this->normalVector);

	this->ctrlV1Original = this->ctrlV1;
	this->ctrlV2Original = this->ctrlV2;
	this->ctrlV3Original = this->ctrlV3;
	this->ctrlV4Original = this->ctrlV4;
	this->centerOriginal = this->center;
}

void CustomPlane::transformPlaneByMatrix(const vcg::Matrix44f& transMat)
{
	this->ctrlV1 = transMat * this->ctrlV1Original;
	this->ctrlV2 = transMat * this->ctrlV2Original;
	this->ctrlV3 = transMat * this->ctrlV3Original;
	this->ctrlV4 = transMat * this->ctrlV4Original;
	this->center = transMat * this->centerOriginal;
	this->normalVector = getNormalVector(this->ctrlV1, this->ctrlV2, this->ctrlV3);
	this->normalVector.Normalize();
	this->d = -this->ctrlV1.dot(this->normalVector);
}

void CustomPlane::getPlaneEquation(Point3m& normal, float& d)
{
	normal = this->normalVector;
	d = this->d;
}

void CustomPlane::updateAnis()
{
	this->center = (this->ctrlV1 + this->ctrlV3) / 2.0f;

	this->axisXV = ((ctrlV2 + ctrlV3) / 2.0f) - this->center;
	this->axisXV = axisXV.Normalize();
	this->axisYV = ((ctrlV1 + ctrlV2) / 2.0f) - this->center;
	this->axisYV = axisYV.Normalize();
	this->axisZV = this->axisXV ^ this->axisYV;
	this->axisZV = axisZV.Normalize();
}

Point3m CustomPlane::getProjectionOnPlane(Point3m vert)
{
	Point3m result, edge;
	edge = vert - this->center;
	result.X() = edge * this->axisXV;
	result.Y() = edge * this->axisYV;
	result.Z() = 0;
	return result;
}
