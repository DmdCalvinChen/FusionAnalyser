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

#include "archlinemachine.h"

ArchLineMachine::ArchLineMachine()
{
}

ArchLineMachine::ArchLineMachine(CustomPlane _plane, vector<Point3m> _inputNodes, bool _upperDental)
{
	this->curPlane = _plane;
	this->inputNodes = _inputNodes;
	this->bUpperDental = _upperDental;
	this->plane2DNodes.clear();
	this->plane3DNodes.clear();
	this->fDentalArchLength = 0;
	for (auto node : inputNodes)
	{
		Point3m tempP = dimensionReduction(node);
		this->plane2DNodes.push_back(tempP);
		tempP = dimensionIncrese(tempP);
		this->plane3DNodes.push_back(tempP);
	}
}

ArchLineMachine::~ArchLineMachine()
{
}

Point3m ArchLineMachine::dimensionReduction(Point3m p)
{
	Point3m O;
	Point3m xV, yV, zV;
	O = this->curPlane.center;
	xV =this->curPlane.axisXV;
	yV =this->curPlane.axisYV;
	zV =this->curPlane.axisZV;
	Point3m tempVector = p - O;
	float projX, projY, projZ;
	projX = tempVector * xV;
	projY = tempVector * yV;

	return Point3m(projX, projY, 0);
}

Point3m ArchLineMachine::dimensionIncrese(Point3m p)
{
	Point3m O;
	Point3m xV, yV, zV;
	O = this->curPlane.center;
	xV = this->curPlane.axisXV;
	yV = this->curPlane.axisYV;
	zV = this->curPlane.axisZV;
	Point3m result;
	result = xV * p.X() + yV * p.Y() + zV * p.Z();
	result += O;

	return result;
}

void ArchLineMachine::GenerateDentalArch()
{
	ResortCtrlPoints();
	fDentalArchLength = 0;
	vector<Point3m> dentalPoints_proj;

	// Calculate iteration times: (size-1)/2 for 3 points per iteration
	int size = this->plane2DNodes.size();
	if (size < 3)
		return;
	else
	{
		int iTime = 0;
		iTime = (size - 1) / 2;

		if (bUpperDental)
		{
			for (int i = 0; i < iTime; i++)
			{
				GetDentalArchLine(i * 2, i * 2 + 1, i * 2 + 2, dentalPoints_proj, fDentalArchLength);
			}
		}
		else
		{
			for (int i = 0; i < iTime; i++)
			{
				GetDentalArchLine(i * 2, i * 2 + 1, i * 2 + 2, dentalPoints_proj, fDentalArchLength);
			}
		}
	}
	for (int i = 0; i < dentalPoints_proj.size(); i++)
	{
		archLineNodes.push_back(dimensionIncrese(dentalPoints_proj[i]));
	}

	float dentalLength = 0;
	for (int i = 0; i < (archLineNodes.size() - 1); i++)
	{
		float tempLength = 0;
		Point3m tempP = archLineNodes[i + 1] - archLineNodes[i];
		tempLength = sqrt(tempP.X() * tempP.X() + tempP.Y() * tempP.Y() + tempP.Z() * tempP.Z());
		dentalLength += tempLength;
	}
	fDentalArchLength = dentalLength;
}

void ArchLineMachine::GetDentalArchLine(int i1, int i2, int i3, vector<Point3m> &DA, float &Length)
{
	float x1, y1;
	float x2, y2;
	float x3, y3;

	x1 = plane2DNodes[i1].X(); y1 = plane2DNodes[i1].Y();
	x2 = plane2DNodes[i2].X(); y2 = plane2DNodes[i2].Y();
	x3 = plane2DNodes[i3].X(); y3 = plane2DNodes[i3].Y();

	// Dental Arch Line function Parameters: F(x) = a*X^4 + b*X^2 + c
	float a, b, c;
	a = ((y1 - y3) * (pow(x1, 2) - pow(x2, 2)) - (y1 - y2) * (pow(x1, 2) - pow(x3, 2)))
		/
		((pow(x1, 4) - pow(x3, 4)) * (pow(x1, 2) - pow(x2, 2)) - (pow(x1, 4) - pow(x2, 4)) * (pow(x1, 2) - pow(x3, 2)));
	b = ((y1 - y2) - a * (pow(x1, 4) - pow(x2, 4))) / (pow(x1, 2) - pow(x2, 2));
	c = y2 - a * pow(x2, 4) - b * pow(x2, 2);

	float XCursor, XEndPoint;
	if (x1 < x3)
	{
		XCursor = x1;
		XEndPoint = x3;
	}
	else
	{
		XCursor = x3;
		XEndPoint = x1;
	}
	while (XCursor <= XEndPoint)
	{
		float tempY = a * pow(XCursor, 4) + b * pow(XCursor, 2) + c;
		Point3f vert;
		vert.X() = XCursor;
		vert.Y() = tempY;
		vert.Z() = 0;
		DA.push_back(vert);

		XCursor += 0.05f;
	}

	float tempLength = 0;
	for (int i = 0; i < (DA.size() - 1); i++)
	{
		float temp;
		temp = (DA[i].X() - DA[i + 1].X()) * (DA[i].X() - DA[i + 1].X()) + (DA[i].Y() - DA[i + 1].Y()) * (DA[i].Y() - DA[i + 1].Y());
		tempLength += sqrtf(temp);
	}

	Length += tempLength;
}

void ArchLineMachine::ResortCtrlPoints()
{
	int i, j;
	Point3f temp;

	for (i = plane2DNodes.size() - 1; 0 < i; i--) {
		for (j = 0; j < i; j++) {
			if (plane2DNodes[j].X() > plane2DNodes[j + 1].X()) {
				temp = inputNodes[j];
				inputNodes[j] = inputNodes[j + 1];
				inputNodes[j + 1] = temp;

				temp = plane3DNodes[j];
				plane3DNodes[j] = plane3DNodes[j + 1];
				plane3DNodes[j + 1] = temp;

				temp = plane2DNodes[j];
				plane2DNodes[j] = plane2DNodes[j + 1];
				plane2DNodes[j + 1] = temp;
			}
		}
	}
}
