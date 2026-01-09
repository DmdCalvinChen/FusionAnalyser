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

#include "planeview.h"

PlaneView::PlaneView()
{
	this->center = Point3m(0, 0, 0);
	this->axisXV = Point3m(1, 0, 0);
	this->axisYV = Point3m(0, 1, 0);
	this->axisZV = Point3m(0, 0, 1);
	this->xPos = center + axisXV * scale;
	this->yPos = center + axisYV * scale;
	this->zPos = center + axisZV * scale;
}

PlaneView::~PlaneView()
{
}

void PlaneView::initalPlane(Point3m locaPL, Point3m locaPM, Point3m locaPR, float scale)
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

	this->xPos = axisXV * scale;
	this->yPos = axisYV * scale;
	this->zPos = axisZV * scale;
	this->d = -locaPL.dot(this->normalVector);
}

Point3m PlaneView::getNormalVector(Point3m pl, Point3m pm, Point3m pr)
{
	Point3m RVector = pr - pm;
	Point3m LVector = pl - pm;
	return LVector ^ RVector;
}

Point3m PlaneView::getCenterPoint(Point3m pl, Point3m pm, Point3m pr)
{
	float t;
	Point3m MR = pr - pm;
	Point3m ML = pl - pm;
	Point3m result = pm + MR * 0.25f + ML *0.25f;

	return result;
}

void PlaneView::getPlaneEquation(Point3m& normal, float& d)
{
	normal = this->normalVector;
	d = this->d;
}
