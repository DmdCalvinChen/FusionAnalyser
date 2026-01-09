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

#ifndef COMMON_TRIANGLEINTERSECTMACHINE_H
#define COMMON_TRIANGLEINTERSECTMACHINE_H

#include "common/ml_mesh_type.h"
#include "util/assist_geometry.h"
#include "common_ext_global.h"

enum TopologicalStructure
{
	INTERSECT,
	NONINTERSECT
};

struct point
{
	float x, y;
};

class COMMON_EXT_EXPORT TriangleIntersectMachine
{
public:
	TriangleIntersectMachine();
	~TriangleIntersectMachine();
public:
	void copy_point(point& p, Point3m f);
	inline float get_vector4_det(Point3m v1, Point3m v2, Point3m v3, Point3m v4);
	inline double direction(point p1, point p2, point p);
	inline int on_segment(point p1, point p2, point p);
	inline bool line_triangle_intersert_inSamePlane(FFace *tri, Point3m f1, Point3m f2);
	inline bool triangle_intersert_inSamePlane(FFace *tri1, FFace *tri2);
	inline int segments_intersert(point p1, point p2, point p3, point p4);
	inline bool is_point_within_triangle(FFace *tri, Point3m p);
	TopologicalStructure judge_triangle_topologicalStructure(FFace *tri1, FFace *tri2);

	FFace faceA, faceB;
};

#endif
