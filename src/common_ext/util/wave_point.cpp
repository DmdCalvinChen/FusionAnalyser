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

#include "wave_point.h"

WavePoint::WavePoint()
{
}

WavePoint::WavePoint(int _vi, int _vi_m, int _vi_p, CMeshO *_mesh)
{
    inital(_vi, _vi_m, _vi_p, _mesh);
}

WavePoint::~WavePoint()
{
}

void WavePoint::inital(int _vi, int _vi_m, int _vi_p, CMeshO *_mesh)
{
	this->Vi = _vi;
	this->Vi_mins = _vi_m;
	this->Vi_plus = _vi_p;
	this->pMesh = _mesh;
	this->Pi = pMesh->vert[Vi].P();
	this->Pi_mins = pMesh->vert[Vi_mins].P();
	this->Pi_plus = pMesh->vert[Vi_plus].P();

	getAngleAndAverageLength();
}

Point3m WavePoint::getBisectDividePoint()
{
	Point3m edgeIM, edgeIP;
	edgeIM = Pi_mins - Pi;
	edgeIP = Pi_plus - Pi;
	edgeIM = edgeIM.Normalize();
	edgeIP = edgeIP.Normalize();

	Point3m midpoint;
	midpoint = ((edgeIM + edgeIP) / 2.0f).Normalize();
	midpoint *= this->fAverageLength;

	return (midpoint + Pi);
}

Point3m WavePoint::getBisectDividePoint(Point3m edgeIM, Point3m edgeIP, Point3m vertex)
{
	edgeIM = edgeIM - vertex;
	edgeIP = edgeIP - vertex;
	edgeIM = edgeIM.Normalize();
	edgeIP = edgeIP.Normalize();
	edgeIM += vertex;
	edgeIP += vertex;

	Point3m midpoint;
	midpoint = (edgeIM + edgeIP) / 2.0f;
	midpoint = midpoint - vertex;
	midpoint = midpoint.Normalize();
	midpoint *= this->fAverageLength;

	return (midpoint + Pi);
}

pair<Point3m, Point3m> WavePoint::getTrisectionDividePoint()
{
	Point3m up, mid1, mid2, down;
	Point3m vert = this->pMesh->vert[Vi].P();

	up = this->pMesh->vert[Vi_mins].P();
	down = this->pMesh->vert[Vi_plus].P();
	for (int i = 0; i < 10; i++)
	{
		if (i % 2 == 0)
		{
			mid1 = getBisectDividePoint(up, down, vert);
			up = mid1;
		}
		else
		{
			mid1 = getBisectDividePoint(up, down, vert);
			down = mid1;
		}
	}

	up = this->pMesh->vert[Vi_mins].P();
	down = this->pMesh->vert[Vi_plus].P();
	for (int i = 0; i < 10; i++)
	{
		if (i % 2 == 0)
		{
			mid2 = getBisectDividePoint(up, down, vert);
			down = mid2;
		}
		else
		{
			mid2 = getBisectDividePoint(up, down, vert);
			up = mid2;
		}
	}

	return pair<Point3m, Point3m>(mid1, mid2);
}

void WavePoint::getAngleAndAverageLength()
{
	Point3m edgeIM, edgeIP;
	edgeIM = Pi_mins - Pi;
	edgeIP = Pi_plus - Pi;

	float length1, length2;
	length1 = sqrtf(edgeIM.X() * edgeIM.X() + edgeIM.Y() * edgeIM.Y() + edgeIM.Z() * edgeIM.Z());
	length2 = sqrtf(edgeIP.X() * edgeIP.X() + edgeIP.Y() * edgeIP.Y() + edgeIP.Z() * edgeIP.Z());
	this->fAverageLength = (length1 + length2) / 2.0f;

	edgeIM = edgeIM.Normalize();
	edgeIP = edgeIP.Normalize();
	this->fAngle = (acos(edgeIM * edgeIP) * 180.0f) / PI;

	if (this->bIsCaveAngle)
	{
		this->fAngle = 360 - this->fAngle;
	}
}
