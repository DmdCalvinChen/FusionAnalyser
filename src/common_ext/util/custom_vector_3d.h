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

#ifndef COMMON_CUSTOM_VECTOR_3D_H_
#define COMMON_CUSTOM_VECTOR_3D_H_

#include "common/ml_mesh_type.h"

#include <math.h>

#include "util/mesh_vertex.h"
#include "common_ext_global.h"

/**CustomVector3D:自定义三维向量类
 *
 * 提供三维向量基础成员数据
 * 提供多种构造方法
 * 提供三维向量基础数学计算方法
 */
class COMMON_EXT_EXPORT CustomVector3D
{
public:
	CustomVector3D();
	CustomVector3D(float _x, float _y, float _z);
	~CustomVector3D();

public:
	float x, y, z;
	void initVector(float _x, float _y, float _z);
	void initVector(MeshVertex start, MeshVertex end);
	void initVector(Point3m start, Point3m end);
	void unitization();
	float operator*(const CustomVector3D v)const;
	CustomVector3D operator+(const CustomVector3D v)const;
	void negate();
	float getLength();
};

#endif // !COMMON_CUSTOM_VECTOR_3D_H_

