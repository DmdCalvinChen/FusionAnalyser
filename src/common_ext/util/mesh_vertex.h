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

#ifndef COMMON_MESH_VERTEX_H_
#define COMMON_MESH_VERTEX_H_

#define MaxAdjPNum 200
#define MaxAdjFNum 100

#include "common/ml_mesh_type.h"

#include "common_ext_global.h"

using namespace std;

struct  UdColor
{
	float red, green, blue;

	UdColor() {}
	UdColor(float r, float g, float b)
	{
		red = r;
		green = g;
		blue = b;
	}
	void initColor(float r, float g, float b)
	{
		red = r;
		green = g;
		blue = b;
	}
};

/**MeshVertex:网格变形参照点类
 *
 * 记录了对应的网格模型点位置信息
 * 记录了网格变形方法需要的成员变量
 * 实现了构建和移动变形参照点的方法
 */
class COMMON_EXT_EXPORT MeshVertex
{
public:
	MeshVertex();
	~MeshVertex();

public:
	Point3m vert;
	UdColor Color;
	void changeCoordinate(Point3m v);
	void initPoint(Point3m v, int index);
	void movePoint3f(float x, float y, float z);

	bool bIsBalanced;
	bool bIsBorder;
	int iFactIndex;
	int iLayer; //层号
	int iAdjPNum;//相邻点的数目
	std::vector<int> adj_vertex;//相邻顶点号数组
	std::vector<int> adj_triangle;//相邻三角面号数组
	std::vector<double> adj_EdjeLength;
	int iAdjFNum;//相邻面的数目
};

#endif // COMMON_MESH_VERTEX_H_

