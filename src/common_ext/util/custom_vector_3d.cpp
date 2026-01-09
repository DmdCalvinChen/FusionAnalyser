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

#include "custom_vector_3d.h"

CustomVector3D::CustomVector3D()
{
}

CustomVector3D::CustomVector3D(float _x, float _y, float _z)
{
	this->x = _x;
	this->y = _y;
	this->z = _z;
}

CustomVector3D::~CustomVector3D()
{
}

void CustomVector3D::initVector(float _x, float _y, float _z)
{
	this->x = _x;
	this->y = _y;
	this->z = _z;
}

void CustomVector3D::initVector(MeshVertex start, MeshVertex end)
{
	this->x = end.vert.X() - start.vert.X();
	this->y = end.vert.Y() - start.vert.Y();
	this->z = end.vert.Z() - start.vert.Z();
}

void CustomVector3D::initVector(Point3m start, Point3m end)
{
	this->x = end.X() - start.X();
	this->y = end.Y() - start.Y();
	this->z = end.Z() - start.Z();
}

float CustomVector3D::operator*(const CustomVector3D v)const
{
	return (this->x * v.x + this->y * v.y + this->z * v.z);
}

CustomVector3D CustomVector3D::operator+(const CustomVector3D v)const
{
	return CustomVector3D(this->x + v.x, this->y + v.y, this->z + v.z);
}

void CustomVector3D::negate()
{
	this->x *= -1;
	this->y *= -1;
	this->z *= -1;
}

void CustomVector3D::unitization()
{
	float length;
	length = sqrtf(x * x + y * y + z * z);

	this->x /= length;
	this->y /= length;
	this->z /= length;
}

float CustomVector3D::getLength()
{
	return sqrt(x * x + y * y + z * z);
}

