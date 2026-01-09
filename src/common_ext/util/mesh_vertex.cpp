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

#include "mesh_vertex.h"

MeshVertex::MeshVertex()
{
}

MeshVertex::~MeshVertex()
{
	vector<int>().swap(this->adj_vertex);
	vector<int>().swap(this->adj_triangle);
	vector<double>().swap(this->adj_EdjeLength);
}

void MeshVertex::changeCoordinate(Point3m v)
{
	this->vert = v;
}

void MeshVertex::initPoint(Point3m v, int index)
{
	this->vert = v;
	this->iLayer = -1;
	this->iAdjPNum = 0;
	this->iAdjFNum = 0;
	this->bIsBalanced = true;
	this->bIsBorder = false;
	this->iFactIndex = index;
	this->Color = UdColor(255.0f, 255.0f, 255.0f);
	this->adj_vertex.clear();
	this->adj_triangle.clear();
	this->adj_EdjeLength.clear();
}

void MeshVertex::movePoint3f(float x, float y, float z)
{
	this->vert += vcg::Point3f(x, y, z);
}
