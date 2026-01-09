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

#ifndef OBBOX_H
#define OBBOX_H

#include "common_ext/util/mesh_generator.h"
#include <../vcglib/wrap/io_trimesh/export_stl.h>
#include "qdebug.h"
#include "common/ml_mesh_type.h"
#include "common_ext/util/assist_geometry.h"
#include "common_ext_global.h"
#include "qopengl.h"

class COMMON_EXT_EXPORT OBBox
{
public:
	OBBox() {};
	OBBox(Point3m _center, Point3m _axisX, Point3m _axisY, Point3m _axisZ, float _lenX, float _lenY, float _lenZ);
	inline void setMatrix(vcg::Matrix44f _matrix) { matrix_ = _matrix; }
	bool intersectWithBox(OBBox _box);
	vector<Point3m> getCurrentConners();
	vector<Point3m> getCurrentAxises();
	void getInterval(const vector<Point3m> &_conners, const Point3m &_axis, float &_min, float &_max);

	void draw(Point4m _color = Point4m(1, 1, 1, 0.4f));
	void exportBoxStl(QString _filename);

public:
	Point3m center_;
	Point3m axis_[3];
	float extents_[3];
	vcg::Matrix44f matrix_;
};

#endif // !OBBOX_H
