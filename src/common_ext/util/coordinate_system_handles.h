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

#ifndef COMMON_COORDINATE_SYSTEM_HANDLES_
#define COMMON_COORDINATE_SYSTEM_HANDLES_

#define PI 3.141592653

#include "common/ml_mesh_type.h"
#include "util/assist_geometry.h"

#include "common_ext_global.h"

using namespace std;
using namespace vcg;

class COMMON_EXT_EXPORT CoordinateSystemHandles
{
public:
	CoordinateSystemHandles();
	CoordinateSystemHandles(Point3f _originP, Point3f _xHandle, Point3f _yHandle, Point3f _zHandle);
	~CoordinateSystemHandles();

public:
	void transformTo(CoordinateSystemHandles aim, Point3f &move, Point3f &rotate, Point3f &zoom);
	void transformTo(const CoordinateSystemHandles &aim, Point3f &move, Point3f &rotate);
	void initalCoordinateSys();
	Point3m getProjPointOnPlane(Point3m p, Point3m D, Point3f pos, Point3m N);
	void update(Point3f _originP, Point3f _xHandle, Point3f _yHandle, Point3f _zHandle);

	Point3f originP, xHandle, yHandle, zHandle;
	Axis coorSystem;
};

#endif // !COMMON_COORDINATE_SYSTEM_HANDLES_
