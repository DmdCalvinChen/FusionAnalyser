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

#ifndef COMMON_PLANEVIEW_H_
#define COMMON_PLANEVIEW_H_
#include "ml_mesh_type.h"

class PlaneView
{
public:
	PlaneView();
	~PlaneView();
public:
	Point3m center;
	Point3m locaPL, locaPM, locaPR;
	Point3m normalVector;
	/** plane equation : Ax + By +Cz + D = 0
	N(A,B,C) +  D(D) = 0  */
	float d;
	Point3m axisZV, axisXV, axisYV;
	Point3m xPos, yPos, zPos;
	float scale = 3.0f;

	void initalPlane(Point3m locaPL, Point3m locaPM, Point3m locaPR, float scale);
	Point3m getNormalVector(Point3m pl, Point3m pm, Point3m pr);
	Point3m getCenterPoint(Point3m pl, Point3m pm, Point3m pr);
	void getPlaneEquation(Point3m& normal, float& d);
private:

};
#endif
