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

#include "delaunaymachine.h"

DelaunayMachine::DelaunayMachine()
{
}

DelaunayMachine::~DelaunayMachine()
{
}

void DelaunayMachine::InitDelaunayMachine(vector<Point3f> _vertexs)
{
	this->ClearMachine();
	this->vertexs = _vertexs;
}

void DelaunayMachine::InitDelaunayMachine(vector<Point3f> _vertexs, FFace defaultSuperTriangle)
{
	this->ClearMachine();
	this->vertexs = _vertexs;
	this->STriangle = defaultSuperTriangle;
}

void DelaunayMachine::InitDelaunayMachine(vector<TraceablePoint> _vertexs, FFace defaultSuperTriangle)
{
	this->ClearMachine();
	this->vertexs_trace = _vertexs;
	this->STriangle = defaultSuperTriangle;
}

vector<FFace> DelaunayMachine::DelaunayMachineArbitraryTurnOn2(vector<Point3m> _vertexs, vector<FEdge> _edgelist, FFace superFace)
{
	//构建了一个更大的superFace
	this->ClearMachine();
	this->vertexs = _vertexs;
	this->STriangle = superFace;
	vector<FFace> splitFaces;

	Point3f axisX, axisY;//两个映射坐标轴
	Point3f E1 = superFace.v2 - superFace.v1;
	Point3f E2 = superFace.v3 - superFace.v1;

	Point3f tempNormalV = E1 ^ E2;
	axisX = E1;
	axisX = axisX.Normalize();
	axisY = axisX ^ tempNormalV;
	axisY = axisY.Normalize();

	_vertexs.push_back(superFace.v1);
	_vertexs.push_back(superFace.v3);
	_vertexs.push_back(superFace.v2);
	vector<Point3f> projVertexes;
	projVertexes.clear();
	for (int i = 0; i < _vertexs.size(); i++)
	{
		Point3f E3;
		E3 = _vertexs[i] - superFace.v1;
		float fu, fv;
		//this->computeCoordinateIn2D(E3, fu, fv, axisX, axisY, E1, E2);
		this->coordinateIn2D(_vertexs[i], fu, fv, superFace.v1, axisX, axisY);

		if (abs(fu) < 0.00001f)
		{
			fu = 0;
		}
		if (abs(fv) < 0.00001f)
		{
			fv = 0;
		}

		projVertexes.push_back(Point3f(fv, fu, 0));
	}
	for (int i = 0; i < _edgelist.size(); i++)
	{
		for (int j = 0; j < _vertexs.size(); j++)
		{
			if (_vertexs[j] == _edgelist[i].vertA)
			{
				_edgelist[i].vertA = projVertexes[j];
				break;
			}
		}

		for (int j = 0; j < _vertexs.size(); j++)
		{
			if (_vertexs[j] == _edgelist[i].vertB)
			{
				_edgelist[i].vertB = projVertexes[j];
				break;
			}
		}
	}

	Point3f stV0, stV1, stV2;
	bool pushIt;
	vector<FFace> projSplitFaces;
	projSplitFaces.clear();
	this->InitDelaunayMachine(projVertexes);
	this->DelaunayMachineTurnOnModel3();
	//this->triangles = this->DelaunayMachineTurnOnModel3Newest(projVertexes);
	if (_edgelist.size() > 0)
	{
		this->adjustByComplusiveBorder2(_edgelist, projVertexes, STriangle);
	}
	for (auto face : this->triangles)
	{
		pushIt = true;
		if (face.v1 == stV0 || face.v1 == stV1 || face.v1 == stV2)
		{
			pushIt = false;
		}
		else if (face.v2 == stV0 || face.v2 == stV1 || face.v2 == stV2)
		{
			pushIt = false;
		}
		else if (face.v3 == stV0 || face.v3 == stV1 || face.v3 == stV2)
		{
			pushIt = false;
		}

		if (pushIt)
		{
			projSplitFaces.push_back(face);
		}
	}

	float tempX, tempY;
	stV0 = Point3f(0, 0, 0);
	coordinateIn2D(superFace.v2, tempX, tempY, superFace.v1, axisX, axisY);
	stV1 = Point3f(tempX, tempY, 0);
	coordinateIn2D(superFace.v3, tempX, tempY, superFace.v1, axisX, axisY);
	stV2 = Point3f(tempX, tempY, 0);

	for (int i = 0; i < projSplitFaces.size(); i++)
	{
		int outSign[3];
		Point3f factVert[3];
		bool get1 = false, get2 = false, get3 = false;
		for (int j = 0; j < projVertexes.size(); j++)
		{
			if (projSplitFaces[i].v1 == stV0)
			{
				factVert[0] = superFace.v1;
				get1 = true;
				break;
			}
			if (projSplitFaces[i].v1 == stV1)
			{
				factVert[0] = superFace.v2;
				get1 = true;
				break;
			}
			if (projSplitFaces[i].v1 == stV2)
			{
				factVert[0] = superFace.v3;
				get1 = true;
				break;
			}
			if (projSplitFaces[i].v1 == projVertexes[j])
			{
				factVert[0] = _vertexs[j];
				get1 = true;
				break;
			}
		}
		for (int j = 0; j < projVertexes.size(); j++)
		{
			if (projSplitFaces[i].v2 == stV0)
			{
				factVert[1] = superFace.v1;
				get2 = true;
				break;
			}
			if (projSplitFaces[i].v2 == stV1)
			{
				factVert[1] = superFace.v2;
				get2 = true;
				break;
			}
			if (projSplitFaces[i].v2 == stV2)
			{
				factVert[1] = superFace.v3;
				get2 = true;
				break;
			}
			if (projSplitFaces[i].v2 == projVertexes[j])
			{
				factVert[1] = _vertexs[j];
				get2 = true;
				break;
			}
		}
		for (int j = 0; j < projVertexes.size(); j++)
		{
			if (projSplitFaces[i].v3 == stV0)
			{
				factVert[2] = superFace.v1;
				get3 = true;
				break;
			}
			if (projSplitFaces[i].v3 == stV1)
			{
				factVert[2] = superFace.v2;
				get3 = true;
				break;
			}
			if (projSplitFaces[i].v3 == stV2)
			{
				factVert[2] = superFace.v3;
				get3 = true;
				break;
			}
			if (projSplitFaces[i].v3 == projVertexes[j])
			{
				factVert[2] = _vertexs[j];
				outSign[2] = ONDENTAL;
				get3 = true;
				break;
			}
		}
		splitFaces.push_back(FFace(factVert[0], factVert[2], factVert[1]));
	}

	return splitFaces;
}

vector<vector<FFace>> DelaunayMachine::DelaunayMachineArbitraryTurnOn2Traceable(vector<Point3m> _vertexs, vector<FEdge> _edgelist, FFace superFace)
{
	this->ClearMachine();
	this->vertexs = _vertexs;
	this->STriangle = superFace;

	Point3f center = superFace.v1;
	Point3f E1 = superFace.v2 - superFace.v1;
	Point3f E2 = superFace.v3 - superFace.v1;

	_vertexs.push_back(superFace.v1);
	_vertexs.push_back(superFace.v3);
	_vertexs.push_back(superFace.v2);
	vector<Point3f> projVertexes;
	projVertexes.clear();
	for (int i = 0; i < _vertexs.size(); i++)
	{
		Point3f E3;
		E3 = _vertexs[i] - superFace.v1;
		float fu, fv;
		fu = ((E2 * E2) * (E3 * E1) - (E2 * E1) * (E3 * E2)) / ((E1 * E1) * (E2 * E2) - (E1 * E2) * (E2 * E1));
		fv = ((E1 * E1) * (E3 * E2) - (E1 * E2) * (E3 * E1)) / ((E1 * E1) * (E2 * E2) - (E1 * E2) * (E2 * E1));
		if (abs(fu) < 0.00001f)
		{
			fu = 0;
		}
		if (abs(fv) < 0.00001f)
		{
			fv = 0;
		}

		projVertexes.push_back(Point3f(fv, fu, 0));
	}

	for (int i = 0; i < _edgelist.size(); i++)
	{
		for (int j = 0; j < _vertexs.size(); j++)
		{
			if (_vertexs[j] == _edgelist[i].vertA)
			{
				_edgelist[i].vertA = projVertexes[j];
				break;
			}
		}

		for (int j = 0; j < _vertexs.size(); j++)
		{
			if (_vertexs[j] == _edgelist[i].vertB)
			{
				_edgelist[i].vertB = projVertexes[j];
				break;
			}
		}
	}

	Point3f stV0, stV1, stV2;
	stV0 = Point3f(0.5f, 1.5f, 0);
	stV1 = Point3f(-3, -1, 0);
	stV2 = Point3f(4, -1, 0);
	FFace tempSuperTriangle(stV0, stV1, stV2);

	bool pushIt;
	vector<FFace> projSplitFaces;
	vector<vector<FFace>> splitSteps;
	projSplitFaces.clear();
	this->InitDelaunayMachine(projVertexes, FFace(stV0, stV1, stV2));
	this->DelaunayMachineTurnOnModel3Traceable(splitSteps);
	if (_edgelist.size() > 0)
	{
		this->adjustByComplusiveBorder2(_edgelist, projVertexes, tempSuperTriangle);
		splitSteps.push_back(triangles);
	}

	vector<vector<FFace>> result;
	for (auto facelist : splitSteps)
	{
		vector<FFace> translateFacelist;
		for (auto face : facelist)
		{
			Point3m v1, v2, v3;
			v1 = center + E2 * face.v1.X() + E1 * face.v1.Y();
			v2 = center + E2 * face.v2.X() + E1 * face.v2.Y();
			v3 = center + E2 * face.v3.X() + E1 * face.v3.Y();

			translateFacelist.push_back(FFace(v1, v2, v3));
		}
		result.push_back(translateFacelist);
	}

	vector<Point3f>().swap(sortedVertexs);
	for (auto v : this->vertexs)
	{
		Point3m v1;
		v1 = center + E2 * v.X() + E1 * v.Y();
		sortedVertexs.push_back(v1);
	}

	return result;
}

bool DelaunayMachine::DelaunayMachineArbitraryTurnOnNewest(const vector<Point3m>& _vertexs, vector<FEdge>& _edgelist,
    const FFace &superFace, const Point3m &judgeNormal,    vector<FFace> &result)
{
	this->ClearMachine();
	this->vertexs = _vertexs;
	this->STriangle = superFace;
	this->sourceSTriangle = superFace;

	vector<FEdge> boundary = _edgelist;

	Point3f axisX, axisY;//两个映射坐标轴
	Point3f E1 = superFace.v2 - superFace.v1;
	Point3f E2 = superFace.v3 - superFace.v1;

	vector<Point3f> projVertexes;
	projVertexes.clear();
	for (int i = 0; i < _vertexs.size(); i++)
	{
		Point3f E3;
		E3 = _vertexs[i] - superFace.v1;
		float fu, fv;
		fu = ((E2 * E2) * (E3 * E1) - (E2 * E1) * (E3 * E2)) / ((E1 * E1) * (E2 * E2) - (E1 * E2) * (E2 * E1));
		fv = ((E1 * E1) * (E3 * E2) - (E1 * E2) * (E3 * E1)) / ((E1 * E1) * (E2 * E2) - (E1 * E2) * (E2 * E1));
		if (abs(fu) < 0.00001f)
		{
			fu = 0;
		}
		if (abs(fv) < 0.00001f)
		{
			fv = 0;
		}

		projVertexes.push_back(Point3f(fv, fu, 0));
	}

	for (int i = 0; i < _edgelist.size(); i++)
	{
		for (int j = 0; j < _vertexs.size(); j++)
		{
			if (_vertexs[j] == _edgelist[i].vertA)
			{
				_edgelist[i].vertA = projVertexes[j];
				break;
			}
		}

		for (int j = 0; j < _vertexs.size(); j++)
		{
			if (_vertexs[j] == _edgelist[i].vertB)
			{
				_edgelist[i].vertB = projVertexes[j];
				break;
			}
		}
	}

	Point3f stV0, stV1, stV2;
	stV0 = Point3f(0, 0, 0);
	stV1 = Point3f(0, 1, 0);
	stV2 = Point3f(1, 0, 0);
	FFace tempSuperTriangle(stV0, stV1, stV2);

	bool pushIt;
	vector<FFace> projSplitFaces;
	projSplitFaces.clear();
	this->InitDelaunayMachine(projVertexes, FFace(stV0, stV1, stV2));
	this->DelaunayMachineTurnOnModel3();
	if (_edgelist.size() > 0)
	{
		this->adjustByComplusiveBorder2(_edgelist, projVertexes, tempSuperTriangle);
	}
	for (auto face : this->triangles)
	{
		pushIt = true;
		if (face.v1 == stV0 || face.v1 == stV1 || face.v1 == stV2)
		{
			pushIt = false;
		}
		else if (face.v2 == stV0 || face.v2 == stV1 || face.v2 == stV2)
		{
			pushIt = false;
		}
		else if (face.v3 == stV0 || face.v3 == stV1 || face.v3 == stV2)
		{
			pushIt = false;
		}

		if (pushIt)
		{
			projSplitFaces.push_back(face);
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//this->embedVerts = translateVerts(embedVerts, projVertexes, _vertexs);                                        //
	//this->embedFacesSteps = translateSteps(embedFacesCollect, tempSuperTriangle, projVertexes, _vertexs);         //
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	vector<FFace> splitFaces;
	for (int i = 0; i < projSplitFaces.size(); i++)
	{
		int outSign[3];
		Point3f factVert[3];
		bool get1 = false, get2 = false, get3 = false;
		for (int j = 0; j < projVertexes.size(); j++)
		{
			if (projSplitFaces[i].v1 == projVertexes[j])
			{
				factVert[0] = _vertexs[j];
				get1 = true;
				break;
			}
		}
		for (int j = 0; j < projVertexes.size(); j++)
		{
			if (projSplitFaces[i].v2 == projVertexes[j])
			{
				factVert[1] = _vertexs[j];
				get2 = true;
				break;
			}
		}
		for (int j = 0; j < projVertexes.size(); j++)
		{
			if (projSplitFaces[i].v3 == projVertexes[j])
			{
				factVert[2] = _vertexs[j];
				outSign[2] = ONDENTAL;
				get3 = true;
				break;
			}
		}
		splitFaces.push_back(FFace(factVert[0], factVert[2], factVert[1]));
	}
    this->getRidOfOutsideBoundaryTriangles(splitFaces, boundary, judgeNormal, result);

    return true;
}

vector<FFace> DelaunayMachine::DelaunayMachineArbitraryTurnOn2(vector<TraceablePoint> _vertexs, FFace superFace)
{
	this->ClearMachine();
	this->onEdgePList.clear();
	this->vertexs_trace = _vertexs;
	this->STriangle = superFace;

	Point3f axisX, axisY;//两个映射坐标轴
	Point3f E1 = superFace.v2 - superFace.v1;
	Point3f E2 = superFace.v3 - superFace.v1;

	_vertexs.push_back(TraceablePoint(superFace.v1, -1));
	_vertexs.push_back(TraceablePoint(superFace.v3, -1));
	_vertexs.push_back(TraceablePoint(superFace.v2, -1));
	vector<Point3f> projVertexes;
	projVertexes.clear();
	for (int i = 0; i < _vertexs.size(); i++)
	{
		Point3f E3;
		E3 = _vertexs[i].point - superFace.v1;
		float fu, fv;
		fu = ((E2 * E2) * (E3 * E1) - (E2 * E1) * (E3 * E2)) / ((E1 * E1) * (E2 * E2) - (E1 * E2) * (E2 * E1));
		fv = ((E1 * E1) * (E3 * E2) - (E1 * E2) * (E3 * E1)) / ((E1 * E1) * (E2 * E2) - (E1 * E2) * (E2 * E1));
		if (abs(fu) < 0.00001f)
		{
			fu = 0;
		}
		if (abs(fv) < 0.00001f)
		{
			fv = 0;
		}

		projVertexes.push_back(Point3f(fv, fu, 0));
	}
	projVertexes.push_back(Point3f(0, 0, 0));
	projVertexes.push_back(Point3f(0, 1, 0));
	projVertexes.push_back(Point3f(1, 0, 0));

	Point3f stV0, stV1, stV2;
	stV0 = Point3f(0.5f, 1.5f, 0);
	stV1 = Point3f(-3, -1, 0);
	stV2 = Point3f(4, -1, 0);

	vector<FFace> projSplitFaces;
	vector<vector<FFace>> splitCollect;
	projSplitFaces.clear();
	this->InitDelaunayMachine(projVertexes, FFace(stV0, stV1, stV2));
	this->DelaunayMachineTurnOnModel3();
	bool pushIt;
	for (auto face : this->triangles)
	{
		pushIt = true;
		if (face.v1 == stV0 || face.v1 == stV1 || face.v1 == stV2)
		{
			pushIt = false;
		}
		else if (face.v2 == stV0 || face.v2 == stV1 || face.v2 == stV2)
		{
			pushIt = false;
		}
		else if (face.v3 == stV0 || face.v3 == stV1 || face.v3 == stV2)
		{
			pushIt = false;
		}

		if (pushIt)
		{
			projSplitFaces.push_back(face);
		}
	}

	stV0 = Point3f(0, 0, 0);
	stV1 = Point3f(0, 1, 0);
	stV2 = Point3f(1, 0, 0);
	vector<FFace> splitFaces;
	for (int i = 0; i < projSplitFaces.size(); i++)
	{
		int outSign[3];
		Point3f factVert[3];
		bool get1 = false, get2 = false, get3 = false;
		for (int j = 0; j < projVertexes.size(); j++)
		{
			if (projSplitFaces[i].v1 == stV0)
			{
				factVert[0] = superFace.v1;
				get1 = true;
				break;
			}
			if (projSplitFaces[i].v1 == stV1)
			{
				factVert[0] = superFace.v2;
				get1 = true;
				break;
			}
			if (projSplitFaces[i].v1 == stV2)
			{
				factVert[0] = superFace.v3;
				get1 = true;
				break;
			}
			if (projSplitFaces[i].v1 == projVertexes[j])
			{
				factVert[0] = _vertexs[j].point;
				get1 = true;
				break;
			}
		}
		for (int j = 0; j < projVertexes.size(); j++)
		{
			if (projSplitFaces[i].v2 == stV0)
			{
				factVert[1] = superFace.v1;
				get2 = true;
				break;
			}
			if (projSplitFaces[i].v2 == stV1)
			{
				factVert[1] = superFace.v2;
				get2 = true;
				break;
			}
			if (projSplitFaces[i].v2 == stV2)
			{
				factVert[1] = superFace.v3;
				get2 = true;
				break;
			}
			if (projSplitFaces[i].v2 == projVertexes[j])
			{
				factVert[1] = _vertexs[j].point;
				get2 = true;
				break;
			}
		}
		for (int j = 0; j < projVertexes.size(); j++)
		{
			if (projSplitFaces[i].v3 == stV0)
			{
				factVert[2] = superFace.v1;
				get3 = true;
				break;
			}
			if (projSplitFaces[i].v3 == stV1)
			{
				factVert[2] = superFace.v2;
				get3 = true;
				break;
			}
			if (projSplitFaces[i].v3 == stV2)
			{
				factVert[2] = superFace.v3;
				get3 = true;
				break;
			}
			if (projSplitFaces[i].v3 == projVertexes[j])
			{
				factVert[2] = _vertexs[j].point;
				outSign[2] = ONDENTAL;
				get3 = true;
				break;
			}
		}
		splitFaces.push_back(FFace(factVert[0], factVert[1], factVert[2]));
	}

	return splitFaces;
}

void DelaunayMachine::sortVertex(vector<Point3f> &vertexs)
{
	//各个点的x值按从小到大的顺序进行排列。
	int i, j;
	Point3f temp;
	int length = vertexs.size();
	for (i = length - 1; 0 < i; i--) {
		for (j = 0; j < i; j++) {
			if (vertexs[j].X() > vertexs[j + 1].X()) {
				temp = vertexs[j];
				vertexs[j] = vertexs[j + 1];
				vertexs[j + 1] = temp;
			}
		}
	}
}

void DelaunayMachine::sortVertex(vector<TraceablePoint> &vertexs)
{
	//各个点的x值按从小到大的顺序进行排列。
	int i, j;
	TraceablePoint temp;
	int length = vertexs.size();
	for (i = length - 1; 0 < i; i--) {
		for (j = 0; j < i; j++) {
			if (vertexs[j].point.X() > vertexs[j + 1].point.X()) {
				temp = vertexs[j];
				vertexs[j] = vertexs[j + 1];
				vertexs[j + 1] = temp;
			}
		}
	}
}

void DelaunayMachine::getSuperTriangle(vector<Point3f> v)
{
	float maxX, minX, maxY, minY;
	GetMaximumIn(vertexs, maxX, minX, maxY, minY);

	Point3f v1((maxX + minX) / 2.0f, 2 * maxY - minY, 0);
	Point3f v2(maxX + (maxX - minX) / 2.0f + 8.0f, minY - 1.0f, 0);
	Point3f v3(maxX + minX - v2.X(), v2.Y(), 0);

	STriangle.InitFFace(v1, v2, v3);
}

FFace DelaunayMachine::getAndReturnSuperTriangle(vector<Point3f> v)
{
	float maxX, minX, maxY, minY;
	GetMaximumIn(v, maxX, minX, maxY, minY);

	Point3f v1((maxX + minX) / 2.0f, 2 * maxY - minY, 0);
	Point3f v2(maxX + (maxX - minX) / 2.0f + 8.0f, minY - 1.0f, 0);
	Point3f v3(maxX + minX - v2.X(), v2.Y(), 0);

	FFace result;
	result.InitFFace(v1, v2, v3);
	return result;
}

void DelaunayMachine::GetMaximumIn(vector<Point3f> vertexs, float &maxX, float &minX, float &maxY, float &minY)
{
	minX = vertexs[0].X();
	//maxX = vertexs[vertexs.size() - 1].X();
	maxX = vertexs[0].X();
	for (int i = 1; i < vertexs.size(); i++)
	{
		if (minX > vertexs[i].X())
			minX = vertexs[i].X();
		if (maxX < vertexs[i].X())
			maxX = vertexs[i].X();
	}

	minY = vertexs[0].Y();
	maxY = vertexs[0].Y();
	for (int i = 1; i < vertexs.size(); i++)
	{
		if (minY > vertexs[i].Y())
			minY = vertexs[i].Y();
		if (maxY < vertexs[i].Y())
			maxY = vertexs[i].Y();
	}
}

void DelaunayMachine::getCircleCenterAndRadius(Point3f v1, Point3f v2, Point3f v3, Point3f &ccenter, float &fRadius)
{
	float xa, ya, xb, yb, xc, yc;
	xa = v1.X();
	ya = v1.Y();
	xb = v2.X();
	yb = v2.Y();
	xc = v3.X();
	yc = v3.Y();

	float deta, detaX, detaY;
	deta = 2 * (xa - xb) * (yc - yb) - 2 * (ya - yb) * (xc - xb);
	detaX = (yc - yb) * (xa*xa + ya*ya - xb*xb - yb*yb) - (ya - yb) * (xc*xc + yc*yc - xb*xb - yb*yb);
	detaY = (xa - xb) * (xc*xc + yc*yc - xb*xb - yb*yb) - (xc - xb) * (xa*xa + ya*ya - xb*xb - yb*yb);

	float x, y;
	x = (float)detaX / deta;
	y = (float)detaY / deta;

	fRadius = sqrt( (x - xa)*(x - xa) + (y - ya)*(y - ya) );
	ccenter.X() = x;
	ccenter.Y() = y;
	ccenter.Z() = 0;
}

void DelaunayMachine::getCircleCenterAndRadius(FFace face, Point3f &ccenter, float &fRadius)
{
	float xa, ya, xb, yb, xc, yc;
	xa = face.v1.X();
	ya = face.v1.Y();
	xb = face.v2.X();
	yb = face.v2.Y();
	xc = face.v3.X();
	yc = face.v3.Y();

	float deta, detaX, detaY;
	deta = 2 * (xa - xb) * (yc - yb) - 2 * (ya - yb) * (xc - xb);
	detaX = (yc - yb) * (xa*xa + ya*ya - xb*xb - yb*yb) - (ya - yb) * (xc*xc + yc*yc - xb*xb - yb*yb);
	detaY = (xa - xb) * (xc*xc + yc*yc - xb*xb - yb*yb) - (xc - xb) * (xa*xa + ya*ya - xb*xb - yb*yb);

	float x, y;
	x = (float)detaX / deta;
	y = (float)detaY / deta;

	fRadius = sqrt((x - xa)*(x - xa) + (y - ya)*(y - ya));
	ccenter.X() = x;
	ccenter.Y() = y;
	ccenter.Z() = 0;
}

SITUATION DelaunayMachine::WhichPosSituationIs(Point3f p, Point3f center, float riduas)
{
	if (p.X() > center.X() + riduas)
		return LEFT;

	float length = sqrt((p.X() - center.X())*(p.X() - center.X()) + (p.Y() - center.Y())*(p.Y() - center.Y()));
	if(length < riduas + this->allowedDeviation)
	{
		return INSIDE;
	}
	else
	{
		return OUTSIDE;
	}
}

SITUATION DelaunayMachine::WhichPosSituationIs(Point3f p, Point3f center, float riduas, FFace triangle, bool &c1, bool &c2, bool &c3)
{
	if (p.X() > center.X() + riduas)
		return LEFT;

	float length = sqrt((p.X() - center.X())*(p.X() - center.X()) + (p.Y() - center.Y())*(p.Y() - center.Y()));
	if (length < riduas + this->allowedDeviation)
	{
		for (auto vert : this->onEdgePList)
		{
			if (vert.p == p)
			{
				if (belongToSameBorder(FEdge(triangle.v1, triangle.v2),vert.edge))
				{
					c1 = true;
				}
				if (belongToSameBorder(FEdge(triangle.v2, triangle.v3), vert.edge))
				{
					c2 = true;
				}
				if (belongToSameBorder(FEdge(triangle.v3, triangle.v1), vert.edge))
				{
					c3 = true;
				}

				break;
			}
		}
		return INSIDE;
	}
	else
	{
		return OUTSIDE;
	}
}

SITUATION DelaunayMachine::WhichPosSituationIsWithTraceableEdge(TraceablePoint p, Point3f center, float riduas, FFace triangle)
{
	if (p.point.X() > center.X() + riduas)
		return LEFT;

	bool bColline1 = false;
	if (tracePointOnEdge(p, FEdge(triangle.v1, triangle.v2), bColline1))
	{
		return INSIDE;
	}
	if (p.edgeIndex >= 0 && bColline1)
	{
		return OUTSIDE;
	}

	bool bColline2 = false;
	if (tracePointOnEdge(p, FEdge(triangle.v2, triangle.v3), bColline2))
	{
		return INSIDE;
	}
	if (p.edgeIndex >= 0 && bColline2)
	{
		return OUTSIDE;
	}

	bool bColline3 = false;
	if (tracePointOnEdge(p, FEdge(triangle.v3, triangle.v1), bColline3))
	{
		return INSIDE;
	}
	if (p.edgeIndex >= 0 && bColline3)
	{
		return OUTSIDE;
	}

	float length = sqrt((p.point.X() - center.X())*(p.point.X() - center.X()) + (p.point.Y() - center.Y())*(p.point.Y() - center.Y()));
	if (length < riduas + this->allowedDeviation)
	{
		return INSIDE;
	}
	else
	{
		return OUTSIDE;
	}
}

void DelaunayMachine::AddNewTempTriangles(Point3f p)
{
	for (int i = 0; i < edgeList.size(); i++)
	{
		FFace newTempFace;
		newTempFace.v1 = p;
		newTempFace.v2 = edgeList[i].vertA;
		newTempFace.v3 = edgeList[i].vertB;

		tempTriangles.push_back(newTempFace);
	}
}

void DelaunayMachine::AddNewTempTriangles2(Point3f p)
{
	vector<FFace> newAddFaces;
	for (int i = 0; i < edgeList.size(); i++)
	{
		float x1, y1, x2, y2;
		x1 = edgeList[i].vertA.X(); y1 = edgeList[i].vertA.Y();
		x2 = edgeList[i].vertB.X(); y2 = edgeList[i].vertB.Y();
		float a, b, t;
		bool Collineation = false;
		float minDistance = this->allowedDeviation;
		if (abs(x1 - x2) <= minDistance)
		{
			t = x1 - p.X();
			if (abs(t) <= minDistance)
				Collineation = true;
		}
		else if (abs(y1 - y2) <= minDistance)
		{
			t = y1 - p.Y();
			if (abs(t) <= minDistance)
				Collineation = true;
		}
		else
		{
			a = (y2 - y1) / (x2 - x1);
			b = y1 - a * x1;
			t = a * p.X() + b - p.Y();
			if (abs(t) <= minDistance)
				Collineation = true;
		}

		if (!Collineation)
		{
			FFace newTempFace;
			newTempFace.v1 = p;
			newTempFace.v2 = edgeList[i].vertA;
			newTempFace.v3 = edgeList[i].vertB;

			//newAddFaces.push_back(newTempFace);
			tempTriangles.push_back(newTempFace);
		}
	}
	////对新添加面进行局部优化检测
}

void DelaunayMachine::localOptimization(vector<FFace> &faces)
{
	int iTime = 0;
	for (; iTime < 10; ++iTime)
	{
		bool badTriangleExist = subLocalOptimization(faces);
		if (!badTriangleExist)
		{
			break;
		}
	}
}

bool DelaunayMachine::subLocalOptimization(vector<FFace> &faces)
{
	vector<pair<int, int>> badTriCP = checkBadTriangleCoupleFrom(faces);
	if (badTriCP.empty())
	{
		return false;
	}
	else
	{
		optimizeAction(badTriCP, faces);
		return true;
	}
}

vector<pair<int, int>> DelaunayMachine::checkBadTriangleCoupleFrom(vector<FFace> &faces)
{
	//1.从faces中提取公共边信息
	vector<pair<pair<int, int>, FEdge>> infoList;
	infoList = collectCommonEdgeInfo(faces);
	//2.对commonEdgeInfo中出现badTriangle的过滤
	vector<pair<int, int>> badCPList = extractBadTriangleCouple(infoList, faces);
	vector<pair<pair<int, int>, FEdge>>().swap(infoList);
	//3.去除badTriangle中索引相连的CP
	vector<pair<int, int>> result;
	if (badCPList.empty())
	{
		return result;
	}

	vector<pair<int, int>>::iterator ite;
	while (!badCPList.empty())
	{
		ite = badCPList.begin();
		pair<int, int> curStandardCP = (*ite);
		result.push_back(curStandardCP);
		badCPList.erase(ite);

		while (ite != badCPList.end())
		{
			bool linked = false;
			if ((*ite).first == curStandardCP.first)
			{
				linked = true;
			}
			else if ((*ite).first == curStandardCP.second)
			{
				linked = true;
			}
			else if ((*ite).second == curStandardCP.first)
			{
				linked = true;
			}
			else if ((*ite).second == curStandardCP.second)
			{
				linked = true;
			}

			if (linked)
			{
				badCPList.erase(ite);
			}
			else
			{
				++ite;
			}
		}
	}
	vector<pair<int, int>>().swap(badCPList);
	return result;
}

vector<pair<pair<int, int>, FEdge>> DelaunayMachine::collectCommonEdgeInfo(vector<FFace> &faces)
{
	vector<pair<int, FEdge>> edgeList;
	for (int i = 0; i < faces.size(); ++i)
	{
		edgeList.push_back(pair<int,  FEdge>(i, FEdge(faces[i].v1, faces[i].v2)));
		edgeList.push_back(pair<int, FEdge>(i, FEdge(faces[i].v2, faces[i].v3)));
		edgeList.push_back(pair<int, FEdge>(i, FEdge(faces[i].v3, faces[i].v1)));
	}
	vector<pair<pair<int, int>, FEdge>> result;
	vector<pair<int, FEdge>>::iterator ite;
	while (!edgeList.empty())
	{
		ite = edgeList.begin();
		pair<int, FEdge> curEdge = (*ite);
		edgeList.erase(ite);

		while (ite != edgeList.end())
		{
			if ((*ite).second.IsReplicateWith(curEdge.second))
			{
				result.push_back(pair<pair<int, int>, FEdge>(pair<int, int>(curEdge.first, (*ite).first), curEdge.second));
				edgeList.erase(ite);
			}
			else
			{
				++ite;
			}
		}
	}
	vector<pair<int, FEdge>>().swap(edgeList);
	return result;
}

vector<pair<int, int>> DelaunayMachine::extractBadTriangleCouple(vector<pair<pair<int, int>, FEdge>> &infoList, vector<FFace> &faces)
{
	vector<pair<int, int>> result;
	for (int i = 0; i < infoList.size(); ++i)
	{
		FFace faceA = faces[infoList[i].first.first];
		FFace faceB = faces[infoList[i].first.second];
		FEdge commonEdge = infoList[i].second;
		if (isBadTriangleCouple(faceA, faceB, commonEdge))
		{
			result.push_back(infoList[i].first);
		}
	}
	return result;
}

bool DelaunayMachine::isBadTriangleCouple(FFace &faceA, FFace &faceB, FEdge &commonEdge)
{
	Point3m centerA, centerB, curCenter, curVert;
	float fRadiusA, fRadiusB, curRadius;
	getCircleCenterAndRadius(faceA, centerA, fRadiusA);
	getCircleCenterAndRadius(faceB, centerB, fRadiusB);
	if (fRadiusA > fRadiusB)
	{
		curCenter = centerA;
		curRadius = fRadiusA;
		if (faceB.v1 != commonEdge.vertA && faceB.v1 != commonEdge.vertB)
		{
			curVert = faceB.v1;
		}
		else if (faceB.v2 != commonEdge.vertA && faceB.v2 != commonEdge.vertB)
		{
			curVert = faceB.v2;
		}
		else if (faceB.v3 != commonEdge.vertA && faceB.v3 != commonEdge.vertB)
		{
			curVert = faceB.v3;
		}
	}
	else
	{
		curCenter = centerB;
		curRadius = fRadiusB;
		if (faceA.v1 != commonEdge.vertA && faceA.v1 != commonEdge.vertB)
		{
			curVert = faceA.v1;
		}
		else if (faceA.v2 != commonEdge.vertA && faceA.v2 != commonEdge.vertB)
		{
			curVert = faceA.v2;
		}
		else if (faceA.v3 != commonEdge.vertA && faceA.v3 != commonEdge.vertB)
		{
			curVert = faceA.v3;
		}
	}

	Point3m edge = curVert - curCenter;
	float distance = sqrtf(edge * edge);
	if (distance < curRadius)
	{
		return true;
	}
	return false;
}

void DelaunayMachine::optimizeAction(vector<pair<int, int>> badTriCP, vector<FFace> &faces)
{
	for (auto cp : badTriCP)
	{
		Point3m diffA, diffB, same1, same2;
		FFace faceA, faceB;
		faceA = faces[cp.first];
		faceB = faces[cp.second];

		if (faceA.v1 != faceB.v1 && faceA.v1 != faceB.v2 && faceA.v1 != faceB.v3)
		{
			diffA = faceA.v1;
			same1 = faceA.v2;
			same2 = faceA.v3;
		}
		else if (faceA.v2 != faceB.v2 && faceA.v2 != faceB.v2 && faceA.v2 != faceB.v3)
		{
			diffA = faceA.v2;
			same1 = faceA.v1;
			same2 = faceA.v2;
		}
		else if (faceA.v3 != faceB.v3 && faceA.v3 != faceB.v2 && faceA.v3 != faceB.v3)
		{
			diffA = faceA.v3;
			same1 = faceA.v1;
			same2 = faceA.v2;
		}

		if (faceB.v1 != same1 && faceB.v1 != same2)
		{
			diffB = faceB.v1;
		}
		else if (faceB.v2 != same1 && faceB.v2 != same2)
		{
			diffB = faceB.v2;
		}
		else if (faceB.v3 != same1 && faceB.v3 != same2)
		{
			diffB = faceB.v3;
		}

		faces[cp.first] = FFace(diffA, same1, same2);
		faces[cp.second] = FFace(diffB, same2, same1);
	}
}

void DelaunayMachine::AddNewTempTrianglesWithTraceableInfomation(TraceablePoint p)
{
	for (int i = 0; i < edgeList.size(); i++)
	{
		bool Collineation = this->tracePointOnEdge2(p, edgeList[i]);

		if (!Collineation)
		{
			FFace newTempFace;
			newTempFace.v1 = p.point;
			newTempFace.v2 = edgeList[i].vertA;
			newTempFace.v3 = edgeList[i].vertB;

			tempTriangles.push_back(newTempFace);
		}
	}
}

void DelaunayMachine::CombineTwoTriangleVector()
{
	for (int i = 0; i < tempTriangles.size(); i++)
	{
		triangles.push_back(tempTriangles[i]);
	}
}

void DelaunayMachine::RipeConnectionWithSuperTriangle(FFace superTri)
{
	vector<FFace> tempV;
	for (int i = 0; i < triangles.size(); i++)
	{
		if (!IsConnectWithST(triangles[i], STriangle))
		{
			tempV.push_back(triangles[i]);
		}
	}
	triangles.clear();
	for (int i = 0; i < tempV.size(); i++)
	{
		triangles.push_back(tempV[i]);
	}
}

bool DelaunayMachine::IsConnectWithST(FFace f,FFace superTri)
{
	bool sameAsSTsV1, sameAsSTsV2, sameAsSTsV3;
	sameAsSTsV1 = false;
	sameAsSTsV2 = false;
	sameAsSTsV3 = false;
	if ((f.v1.X() == superTri.v1.X() && f.v1.Y() == superTri.v1.Y())
		||
		(f.v2.X() == superTri.v1.X() && f.v2.Y() == superTri.v1.Y())
		||
		(f.v3.X() == superTri.v1.X() && f.v3.Y() == superTri.v1.Y()))
		sameAsSTsV1 = true;

	if ((f.v1.X() == superTri.v2.X() && f.v1.Y() == superTri.v2.Y())
		||
		(f.v2.X() == superTri.v2.X() && f.v2.Y() == superTri.v2.Y())
		||
		(f.v3.X() == superTri.v2.X() && f.v3.Y() == superTri.v2.Y()))
		sameAsSTsV1 = true;

	if ((f.v1.X() == superTri.v3.X() && f.v1.Y() == superTri.v3.Y())
		||
		(f.v2.X() == superTri.v3.X() && f.v2.Y() == superTri.v3.Y())
		||
		(f.v3.X() == superTri.v3.X() && f.v3.Y() == superTri.v3.Y()))
		sameAsSTsV1 = true;

	if (sameAsSTsV1 || sameAsSTsV2 || sameAsSTsV3)
		return true;
	else
		return false;
}

void DelaunayMachine::DelaunayMachineTurnOn()
{
	sortVertex(vertexs);
	getSuperTriangle(vertexs);
	bCanDrawSTriangle = true;

	tempTriangles.push_back(STriangle);

	int i = 0;
	do {
		edgeList.clear();

		vector<FFace> delTriangles;
		delTriangles.clear();
		for (int ii = 0; ii < tempTriangles.size(); ii++)
		{
			Point3f center;
			float fRadius;
			getCircleCenterAndRadius(tempTriangles[ii], center, fRadius);

			SITUATION Situation = WhichPosSituationIs(vertexs[i], center, fRadius);

			if (Situation == LEFT)
			{
				triangles.push_back(tempTriangles[ii]);
				delTriangles.push_back(tempTriangles[ii]);
			}
			else if (Situation == OUTSIDE)
			{
				continue;
			}
			else if (Situation == INSIDE)
			{
				edgeList.push_back(FEdge(tempTriangles[ii].v1, tempTriangles[ii].v2));
				edgeList.push_back(FEdge(tempTriangles[ii].v2, tempTriangles[ii].v3));
				edgeList.push_back(FEdge(tempTriangles[ii].v3, tempTriangles[ii].v1));
				delTriangles.push_back(tempTriangles[ii]);
			}
		}

		/*for (int ii = 0; ii < delTriangles.size(); ii++)
		{
			FFace cDelFace = delTriangles[ii];

			vector<FFace> tempV;
			for (int jj = 0; jj < tempTriangles.size(); jj++)
			{
				if (!tempTriangles[jj].IsSameAs(cDelFace))
					tempV.push_back(tempTriangles[jj]);
			}
			tempTriangles.clear();
			for (int i = 0; i < tempV.size(); i++)
				tempTriangles.push_back(tempV[i]);
		}*/
		vector<FFace> tempV;
		for (int ii = 0; ii < tempTriangles.size(); ii++)
		{
			bool needToDeleteIt = false;
			for (int jj = 0; jj < delTriangles.size(); jj++)
			{
				if (tempTriangles[ii].IsSameAs(delTriangles[jj]))
				{
					needToDeleteIt = true;
					break;
				}
			}

			if (!needToDeleteIt)
			{
				tempV.push_back(tempTriangles[ii]);
			}
		}
		tempTriangles.clear();
		for (int i = 0; i < tempV.size(); i++)
			tempTriangles.push_back(tempV[i]);

		SeachAndDeleteReplicateEdge(edgeList);
		edgeNum = edgeList.size();

		AddNewTempTriangles(vertexs[i]);

		i++;
	} while (i < vertexs.size());

	CombineTwoTriangleVector();
	RipeConnectionWithSuperTriangle(STriangle);
}

void DelaunayMachine::DelaunayMachineTurnOnModel2()
{
	sortVertex(vertexs);
	tempTriangles.push_back(STriangle);

	int i = 0;
	do {
		edgeList.clear();

		vector<FFace> delTriangles;
		delTriangles.clear();
		for (int ii = 0; ii < tempTriangles.size(); ii++)
		{
			Point3f center;
			float fRadius;
			getCircleCenterAndRadius(tempTriangles[ii], center, fRadius);

			SITUATION Situation = WhichPosSituationIs(vertexs[i], center, fRadius);

			if (Situation == LEFT)
			{
				triangles.push_back(tempTriangles[ii]);
				delTriangles.push_back(tempTriangles[ii]);
			}
			else if (Situation == OUTSIDE)
			{
				continue;
			}
			else if (Situation == INSIDE)
			{
				edgeList.push_back(FEdge(tempTriangles[ii].v1, tempTriangles[ii].v2));
				edgeList.push_back(FEdge(tempTriangles[ii].v2, tempTriangles[ii].v3));
				edgeList.push_back(FEdge(tempTriangles[ii].v3, tempTriangles[ii].v1));
				delTriangles.push_back(tempTriangles[ii]);
			}
		}

		vector<FFace> tempV;
		for (int ii = 0; ii < tempTriangles.size(); ii++)
		{
			bool needToDeleteIt = false;
			for (int jj = 0; jj < delTriangles.size(); jj++)
			{
				if (tempTriangles[ii].IsSameAs(delTriangles[jj]))
				{
					needToDeleteIt = true;
					break;
				}
			}

			if (!needToDeleteIt)
			{
				tempV.push_back(tempTriangles[ii]);
			}
		}
		tempTriangles.clear();
		for (int i = 0; i < tempV.size(); i++)
			tempTriangles.push_back(tempV[i]);

		SeachAndDeleteReplicateEdge(edgeList);
		edgeNum = edgeList.size();

		AddNewTempTriangles2(vertexs[i]);

		i++;
	} while (i < vertexs.size());

	CombineTwoTriangleVector();
}

void DelaunayMachine::DelaunayMachineTurnOnModel3()
{
	// Determinate the super triangle
	float minX = vertexs[0].X();
	float minY = vertexs[0].Y();
	float maxX = minX;
	float maxY = minY;
	for (std::size_t i = 0; i < vertexs.size(); ++i)
	{
		if (vertexs[i].X() < minX) minX = vertexs[i].X();
		if (vertexs[i].Y() < minY) minY = vertexs[i].Y();
		if (vertexs[i].X() > maxX) maxX = vertexs[i].X();
		if (vertexs[i].Y() > maxY) maxY = vertexs[i].Y();
	}
	const float dx = maxX - minX;
	const float dy = maxY - minY;
	const float deltaMax = std::max(dx, dy);
	const float midx = (minX + maxX) * 0.5f;
	const float midy = (minY + maxY) * 0.5f;
	const Point3f p1(midx - 20 * deltaMax, midy - deltaMax, 0);
	const Point3f p2(midx, midy + 20 * deltaMax, 0);
	const Point3f p3(midx + 20 * deltaMax, midy - deltaMax, 0);
	STriangle = FFace(p1, p2, p3);

	sortVertex(vertexs);
	tempTriangles.push_back(STriangle);

	int i = 0;
	do {
		vector<FEdge>().swap(edgeList);

		vector<FFace>::iterator tempTriIter = tempTriangles.begin();
		while (tempTriIter != tempTriangles.end())
		{
			Point3f center;
			float fRadius;
			getCircleCenterAndRadius(*tempTriIter, center, fRadius);
			SITUATION Situation = WhichPosSituationIs(vertexs[i], center, fRadius);
			if (Situation == LEFT)
			{
				triangles.push_back(*tempTriIter);
				tempTriangles.erase(tempTriIter);
			}
			else if (Situation == OUTSIDE)
			{
				++tempTriIter;
			}
			else if (Situation == INSIDE)
			{
				addEdgeToConstructSplitPolygon(FEdge((*tempTriIter).v1, (*tempTriIter).v2), edgeList);
				addEdgeToConstructSplitPolygon(FEdge((*tempTriIter).v2, (*tempTriIter).v3), edgeList);
				addEdgeToConstructSplitPolygon(FEdge((*tempTriIter).v3, (*tempTriIter).v1), edgeList);
				tempTriangles.erase(tempTriIter);
			}
		}

		edgeNum = edgeList.size();
		AddNewTempTriangles2(vertexs[i]);
		i++;
	} while (i < vertexs.size());

	CombineTwoTriangleVector();

	triangles.erase(std::remove_if(begin(triangles), end(triangles), [p1, p2, p3](FFace &t) {
		return t.containsVertex(p1) || t.containsVertex(p2) || t.containsVertex(p3);
	}), end(triangles));
}

void DelaunayMachine::DelaunayMachineTurnOnModel3Traceable(vector<vector<FFace>> &splitCollect)
{
	sortVertex(vertexs);
	tempTriangles.push_back(STriangle);

	int i = 0;
	do {
		edgeList.clear();

		vector<FFace> delTriangles;
		delTriangles.clear();
		for (int ii = 0; ii < tempTriangles.size(); ii++)
		{
			Point3f center;
			float fRadius;
			getCircleCenterAndRadius(tempTriangles[ii], center, fRadius);

			SITUATION Situation = WhichPosSituationIs(vertexs[i], center, fRadius);

			if (Situation == LEFT)
			{
				triangles.push_back(tempTriangles[ii]);
				delTriangles.push_back(tempTriangles[ii]);
			}
			else if (Situation == OUTSIDE)
			{
				continue;
			}
			else if (Situation == INSIDE)
			{
				edgeList.push_back(FEdge(tempTriangles[ii].v1, tempTriangles[ii].v2));
				edgeList.push_back(FEdge(tempTriangles[ii].v2, tempTriangles[ii].v3));
				edgeList.push_back(FEdge(tempTriangles[ii].v3, tempTriangles[ii].v1));

				delTriangles.push_back(tempTriangles[ii]);
			}
		}

		vector<FFace> tempV;
		for (int ii = 0; ii < tempTriangles.size(); ii++)
		{
			bool needToDeleteIt = false;
			for (int jj = 0; jj < delTriangles.size(); jj++)
			{
				if (tempTriangles[ii].IsSameAs(delTriangles[jj]))
				{
					needToDeleteIt = true;
					break;
				}
			}

			if (!needToDeleteIt)
			{
				tempV.push_back(tempTriangles[ii]);
			}
		}
		tempTriangles.clear();
		for (int i = 0; i < tempV.size(); i++)
			tempTriangles.push_back(tempV[i]);

		SeachAndDeleteReplicateEdge(edgeList);
		edgeNum = edgeList.size();

		AddNewTempTriangles2(vertexs[i]);

		vector<FFace> tempStep;
		for (int i = 0; i < tempTriangles.size(); i++)
		{
			tempStep.push_back(tempTriangles[i]);
		}
		for (int i = 0; i < triangles.size(); i++)
		{
			tempStep.push_back(triangles[i]);
		}
		splitCollect.push_back(tempStep);
		i++;
	} while (i < vertexs.size());

	CombineTwoTriangleVector();
	splitCollect.push_back(triangles);
}

vector<FFace> DelaunayMachine::DelaunayMachineTurnOnModel3Newest(const vector<Point3m> &vertices)
{
	// Store the vertices locally
	vector<Point3m> _vertices = vertices;

	// Determinate the super triangle
	float minX = vertices[0].X();
	float minY = vertices[0].Y();
	float maxX = minX;
	float maxY = minY;

	for (std::size_t i = 0; i < vertices.size(); ++i)
	{
		if (vertices[i].X() < minX) minX = vertices[i].X();
		if (vertices[i].Y() < minY) minY = vertices[i].Y();
		if (vertices[i].X() > maxX) maxX = vertices[i].X();
		if (vertices[i].Y() > maxY) maxY = vertices[i].Y();
	}

	const float dx = maxX - minX;
	const float dy = maxY - minY;
	const float deltaMax = std::max(dx, dy);
	const float midx = (minX + maxX) * 0.5f;
	const float midy = (minY + maxY) * 0.5f;

	const Point3f p1(midx - 20 * deltaMax, midy - deltaMax, 0);
	const Point3f p2(midx, midy + 20 * deltaMax, 0);
	const Point3f p3(midx + 20 * deltaMax, midy - deltaMax, 0);

	// Create a list of triangles, and add the supertriangle in it
	vector<FFace> _triangles;
	_triangles.push_back(FFace(p1, p2, p3));

	for (auto p = begin(vertices); p != end(vertices); p++)
	{
		std::vector<FEdge> polygon;

		for (auto & t : _triangles)
		{
			if (t.circumCircleContains(*p))
			{
				t.bBad = true;
				polygon.push_back(FEdge(t.v1, t.v2));
				polygon.push_back(FEdge(t.v2, t.v3));
				polygon.push_back(FEdge(t.v3, t.v1));
			}
		}

		_triangles.erase(std::remove_if(begin(_triangles), end(_triangles), [](FFace &t) {
			return t.bBad;
		}), end(_triangles));

		for (auto e1 = begin(polygon); e1 != end(polygon); ++e1)
		{
			for (auto e2 = e1 + 1; e2 != end(polygon); ++e2)
			{
				if ((*e1).almost_equal(*e2))
				{
					e1->bBad = true;
					e2->bBad = true;
				}
			}
		}

		polygon.erase(std::remove_if(begin(polygon), end(polygon), [](FEdge &e) {
			return e.bBad;
		}), end(polygon));

		for (const auto e : polygon)
		{
			_triangles.push_back(FFace(e.vertA, e.vertB, *p));
		}
	}

	_triangles.erase(std::remove_if(begin(_triangles), end(_triangles), [p1, p2, p3](FFace &t) {
		return t.containsVertex(p1) || t.containsVertex(p2) || t.containsVertex(p3);
	}), end(_triangles));

	return _triangles;
}

void DelaunayMachine::DelaunayMachineTurnOnModeTraceable(vector<vector<FFace>> &splitCollect)
{
	sortVertex(this->vertexs_trace);
	tempTriangles.push_back(STriangle);

	int i = 0;
	bool collineEdge1, collineEdge2, collineEdge3;
	do {
		edgeList.clear();

		vector<FFace> delTriangles;
		delTriangles.clear();
		for (int ii = 0; ii < tempTriangles.size(); ii++)
		{
			Point3f center;
			float fRadius;
			getCircleCenterAndRadius(tempTriangles[ii], center, fRadius);

			SITUATION Situation = WhichPosSituationIsWithTraceableEdge(vertexs_trace[i], center, fRadius, tempTriangles[ii]);

			if (Situation == LEFT)
			{
				triangles.push_back(tempTriangles[ii]);
				delTriangles.push_back(tempTriangles[ii]);
			}
			else if (Situation == OUTSIDE)
			{
				continue;
			}
			else if (Situation == INSIDE)
			{
				edgeList.push_back(FEdge(tempTriangles[ii].v1, tempTriangles[ii].v2));
				edgeList.push_back(FEdge(tempTriangles[ii].v2, tempTriangles[ii].v3));
				edgeList.push_back(FEdge(tempTriangles[ii].v3, tempTriangles[ii].v1));

				delTriangles.push_back(tempTriangles[ii]);
			}
		}

		vector<FFace> tempV;
		for (int ii = 0; ii < tempTriangles.size(); ii++)
		{
			bool needToDeleteIt = false;
			for (int jj = 0; jj < delTriangles.size(); jj++)
			{
				if (tempTriangles[ii].IsSameAs(delTriangles[jj]))
				{
					needToDeleteIt = true;
					break;
				}
			}

			if (!needToDeleteIt)
			{
				tempV.push_back(tempTriangles[ii]);
			}
		}
		tempTriangles.clear();
		for (int i = 0; i < tempV.size(); i++)
			tempTriangles.push_back(tempV[i]);

		SeachAndDeleteReplicateEdge(edgeList);
		edgeNum = edgeList.size();

		AddNewTempTrianglesWithTraceableInfomation(vertexs_trace[i]);

		vector<FFace> tempStep;
		for (int i = 0; i < tempTriangles.size(); i++)
		{
			tempStep.push_back(tempTriangles[i]);
		}
		for (int i = 0; i < triangles.size(); i++)
		{
			tempStep.push_back(triangles[i]);
		}
		splitCollect.push_back(tempStep);
		i++;
	} while (i < vertexs_trace.size());

	CombineTwoTriangleVector();
	splitCollect.push_back(triangles);
}

void DelaunayMachine::addEdgeToConstructSplitPolygon(FEdge newEdge, vector<FEdge> &edgeList)
{
	bool existed = false;
	vector<FEdge>::iterator ite = edgeList.begin();
	while (ite != edgeList.end())
	{
		if (newEdge.IsReplicateWith(*ite))
		{
			edgeList.erase(ite);
			existed = true;
			break;
		}
		else
		{
			++ite;
		}
	}

	if (!existed)
	{
		edgeList.push_back(newEdge);
	}
}

void DelaunayMachine::SeachAndDeleteReplicateEdge(vector<FEdge> &edgeList)
{
	vector<FEdge> delList;
	for (int i = 0; i < edgeList.size(); i++)
	{
		FEdge edgeC = edgeList[i];
		for (int j = i + 1; j < edgeList.size(); j++)
		{
			if (edgeC.IsReplicateWith(edgeList[j]))
			{
				bool bRe = false;
				for (int k = 0; k < delList.size(); k++)
					if (edgeC.IsReplicateWith(delList[k]))
					{
						bRe = true;
						break;
					}
				if (!bRe)
					delList.push_back(edgeC);

				break;
			}
		}
	}
	ReEdgeList = delList;
	vector<FEdge> tempV;
	for (int i = 0; i < edgeList.size(); i++)
	{
		FEdge edgeC = edgeList[i];
		bool PushThis = true;
		for (int j = 0; j < delList.size(); j++)
		{
			if (edgeC.IsReplicateWith(delList[j]))
			{
				PushThis = false;
				break;
			}
		}
		if (PushThis)
			tempV.push_back(edgeC);
	}
	edgeList.clear();
	for (int i = 0; i < tempV.size(); i++)
		edgeList.push_back(tempV[i]);
}

void DelaunayMachine::ClearMachine()
{
	vector<Point3f>().swap(vertexs);
	vector<TraceablePoint>().swap(vertexs_trace);
	vector<FFace>().swap(triangles);
	vector<FFace>().swap(tempTriangles);
	vector<FEdge>().swap(edgeList);
	vector<FEdge>().swap(ReEdgeList);
}

bool DelaunayMachine::tracePointOnEdge(TraceablePoint p, FEdge edge, bool &bColline)
{
	if (p.edgeIndex < 0)
	{
		return false;
	}

	bColline = false;
	bool findVertAInList = false, findVertBInList = false;
	for (auto vert : this->vertexs_trace)
	{
		if (vert.point == edge.vertA)
		{
			findVertAInList = true;
			if (vert.edgeIndex == p.edgeIndex)
			{
				if (edge.vertB == traceEdgeTab[p.edgeIndex].vertA || edge.vertB == traceEdgeTab[p.edgeIndex].vertB)
				{
					bColline = true;
					Point3m v1 = p.point - edge.vertA;
					Point3m v2 = edge.vertB - p.point;
					if (v1 * v2 > 0)
					{
						return true;
					}
					else
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}

	for (auto vert : this->vertexs_trace)
	{
		if (vert.point == edge.vertB)
		{
			findVertBInList = true;
			if (vert.edgeIndex == p.edgeIndex)
			{
				if (edge.vertA == traceEdgeTab[p.edgeIndex].vertA || edge.vertA == traceEdgeTab[p.edgeIndex].vertB)
				{
					bColline = true;
					Point3m v1 = p.point - edge.vertA;
					Point3m v2 = edge.vertB - p.point;
					if (v1 * v2 > 0)
					{
						return true;
					}
					else
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}

	if (!findVertAInList && !findVertBInList)
	{
		if (traceEdgeTab[p.edgeIndex] == edge)
		{
			bColline = true;
			return true;
		}
		else
		{
			return false;
		}
	}
}

bool DelaunayMachine::tracePointOnEdge2(TraceablePoint p, FEdge edge)
{
	if (p.edgeIndex < 0)
	{
		return false;
	}

	bool findVertAInList = false, findVertBInList = false;
	for (auto vert : this->vertexs_trace)
	{
		if (vert.point == edge.vertA)
		{
			findVertAInList = true;
			if (vert.edgeIndex == p.edgeIndex)
			{
				if (edge.vertB == traceEdgeTab[p.edgeIndex].vertA || edge.vertB == traceEdgeTab[p.edgeIndex].vertB)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}

	for (auto vert : this->vertexs_trace)
	{
		if (vert.point == edge.vertB)
		{
			findVertBInList = true;
			if (vert.edgeIndex == p.edgeIndex)
			{
				if (edge.vertA == traceEdgeTab[p.edgeIndex].vertA || edge.vertA == traceEdgeTab[p.edgeIndex].vertB)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}

	if (!findVertAInList && !findVertBInList)
	{
		if (traceEdgeTab[p.edgeIndex] == edge)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

vector<int> DelaunayMachine::pickOutAllCoverFacesWith(const FEdge &border, const vector<Point3f> &nodelist, const vector<FEdge> &edgelist, const vector<FFace> &facelist)
{
	vector<FEdge> mixEdges;
	int i = 0;
	vector<int> mixEdgeIndexes;
	for (auto edge : edgelist)
	{
		if (edge != border)
		{
			edge.vertA = nodelist[edge.indexA];
			edge.vertB = nodelist[edge.indexB];
			if (edge.intersectWith(border))
			{
				mixEdges.push_back(edge);
				mixEdgeIndexes.push_back(i);
			}
		}
		++i;
	}

	qDebug() << "MixEdge: ";
	for (auto index : mixEdgeIndexes)
	{
		qDebug() << index<<", ";
	}
	qDebug() << endl;

	vector<int> result;
	for (auto edge : mixEdges)
	{
		bool exited = false;
		for (auto index : result)
		{
			if (index == edge.faceA)
			{
				exited = true;
				break;
			}
		}
		if (!exited && edge.faceA != -1)
		{
			result.push_back(edge.faceA);
		}

		exited = false;
		for (auto index : result)
		{
			if (index == edge.faceB)
			{
				exited = true;
				break;
			}
		}
		if (!exited && edge.faceB != -1)
		{
			result.push_back(edge.faceB);
		}
	}

	return result;
}

void DelaunayMachine::adjustByComplusiveBorder2(vector<FEdge> boundary, vector<Point3m> seedlist, FFace superTriangle)
{
	vector<Point3m> nodelist;
	vector<FEdge> edgelist;
	vector<FFace> facelist;

	//添加superTriangle三个顶点和seedlist到nodelist
	//搜索当前的剖分结果triangls的三个顶点在nodelist中的索引，添加索引构成的面facelist
	//搜集facelist中的边，提取无重的edgelist
	analysisAndFillThreeVector(nodelist, edgelist, facelist, seedlist, superTriangle);

	//分配强制嵌入边在nodelist中的索引
	for (int i = 0; i < boundary.size(); i++)
	{
		int vertIndex1, vertIndex2;
		for (int j = 0; j < nodelist.size(); j++)
		{
			if (boundary[i].vertA == nodelist[j])
			{
				vertIndex1 = j;
				break;
			}
		}
		for (int j = 0; j < nodelist.size(); j++)
		{
			if (boundary[i].vertB == nodelist[j])
			{
				vertIndex2 = j;
				break;
			}
		}

		boundary[i].indexA = vertIndex1;
		boundary[i].indexB = vertIndex2;
	}

	int iTimer = 1;
	for (auto border : boundary)
	{
		bool ForcibleEmbed = true;
		for (auto edge : edgelist)
		{
			if (edge.isSameAs(border))
			{
				ForcibleEmbed = false;
				break;
			}
		}

		if (ForcibleEmbed)
		{
			combedSingleBoundary(border, nodelist, edgelist, facelist);
		}
	}

	this->triangles.clear();
	for (auto face : facelist)
	{
		FFace tempFace(nodelist[face.n1], nodelist[face.n2], nodelist[face.n3]);
		tempFace.adjustNormalVector(Point3m(0, 0, 1));
		triangles.push_back(tempFace);
	}
	nodelist.clear();
	edgelist.clear();
	facelist.clear();
}

void DelaunayMachine::combedSingleBoundary(FEdge border, vector<Point3m> &nodelist, vector<FEdge> &edgelist, vector<FFace> &facelist)
{
	vector<int> coverArea;//当前约束边border穿过的三角形区域集合
	coverArea = pickOutAllCoverFacesWith(border, nodelist, edgelist, facelist);
	if (!checkCoverArea(coverArea, border, facelist))
	{
		int stop = 1;
		return;
	}

	QString borderInfo("Combed Edge--->");
	borderInfo += QString::number(border.indexA);
	borderInfo += QString(", ");
	borderInfo += QString::number(border.indexB);
	qDebug() << "||~~~||" << borderInfo << "||~~~||" << endl;

	CombedEdgeMachine combedMachine;
	combedMachine.initalMachine(border, coverArea, nodelist, edgelist, facelist);
	FEdge subBorder;
	bool existConcave = combedMachine.getConcaveEdge(subBorder);
	if (existConcave)
	{
		combedSingleBoundary(subBorder, nodelist, edgelist, facelist);
		combedSingleBoundary(border, nodelist, edgelist, facelist);
	}
	else
	{
		combedMachine.combedEdgeByCheckForbiddenEdgeCP();
		edgelist = combedMachine.edgelist;
		for (auto i : coverArea)
		{
			facelist[i] = combedMachine.facelist[i];
		}
		coverArea.clear();
	}
	return;
}

bool DelaunayMachine::checkCoverArea(const vector<int> &coverArea, const FEdge &border, const vector<FFace> &facelist)
{
	set<int> vertsIndexes;
	for (int i = 0; i < coverArea.size(); ++i)
	{
		vertsIndexes.insert(facelist[coverArea[i]].n1);
		vertsIndexes.insert(facelist[coverArea[i]].n2);
		vertsIndexes.insert(facelist[coverArea[i]].n3);
	}
	bool findA = false, findB = false;
	for (auto index : vertsIndexes)
	{
		if (index == border.indexA)
		{
			findA = true;
			break;
		}
	}
	for (auto index : vertsIndexes)
	{
		if (index == border.indexB)
		{
			findB = true;
			break;
		}
	}
	set<int>().swap(vertsIndexes);
	return findA & findB;
}

void DelaunayMachine::analysisAndFillThreeVector(vector<Point3m> &nodelist, vector<FEdge> &edgelist, vector<FFace> &facelist)
{
	for (auto vert : this->vertexs_trace)
	{
		nodelist.push_back(vert.point);
	}
	nodelist.push_back(STriangle.v1);
	nodelist.push_back(STriangle.v2);
	nodelist.push_back(STriangle.v3);

	for (auto face : this->triangles)
	{
		int vertIndex1, vertIndex2, vertIndex3;
		for (int i = 0; i < nodelist.size(); i++)
		{
			if (face.v1 == nodelist[i])
			{
				vertIndex1 = i;
				break;
			}
		}
		for (int i = 0; i < nodelist.size(); i++)
		{
			if (face.v2 == nodelist[i])
			{
				vertIndex2 = i;
				break;
			}
		}
		for (int i = 0; i < nodelist.size(); i++)
		{
			if (face.v3 == nodelist[i])
			{
				vertIndex3 = i;
				break;
			}
		}

		FFace tempFace(vertIndex1, vertIndex2, vertIndex3);
		facelist.push_back(tempFace);
	}

	for (int i = 0; i < facelist.size(); i++)
	{
		bool isNewEdge = true;
		for (int j = 0; j < edgelist.size(); j++)
		{
			if (edgelist[j] == facelist[i].edge1)
			{
				isNewEdge = false;
				edgelist[j].faceB = i;
				facelist[i].edge1Index = j;
				break;
			}
		}
		if (isNewEdge)
		{
			FEdge tempEdge(facelist[i].edge1.X(), facelist[i].edge1.Y());
			tempEdge.faceA = i;
			edgelist.push_back(tempEdge);
			facelist[i].edge1Index = edgelist.size() - 1;
		}

		isNewEdge = true;
		for (int j = 0; j < edgelist.size(); j++)
		{
			if (edgelist[j] == facelist[i].edge2)
			{
				isNewEdge = false;
				edgelist[j].faceB = i;
				facelist[i].edge2Index = j;
				break;
			}
		}
		if (isNewEdge)
		{
			FEdge tempEdge(facelist[i].edge2.X(), facelist[i].edge2.Y());
			tempEdge.faceA = i;
			edgelist.push_back(tempEdge);
			facelist[i].edge2Index = edgelist.size() - 1;
		}

		isNewEdge = true;
		for (int j = 0; j < edgelist.size(); j++)
		{
			if (edgelist[j] == facelist[i].edge3)
			{
				isNewEdge = false;
				edgelist[j].faceB = i;
				facelist[i].edge3Index = j;
				break;
			}
		}
		if (isNewEdge)
		{
			FEdge tempEdge(facelist[i].edge3.X(), facelist[i].edge3.Y());
			tempEdge.faceA = i;
			edgelist.push_back(tempEdge);
			facelist[i].edge3Index = edgelist.size() - 1;
		}
	}
}

void DelaunayMachine::analysisAndFillThreeVector(vector<Point3m> &nodelist, vector<FEdge> &edgelist, vector<FFace> &facelist, vector<Point3m> seedlist, FFace superTriangle)
{
	nodelist = seedlist;
	nodelist.push_back(superTriangle.v1);
	nodelist.push_back(superTriangle.v2);
	nodelist.push_back(superTriangle.v3);

	for (auto face : this->triangles)
	{
		int vertIndex1, vertIndex2, vertIndex3;
		for (int i = 0; i < nodelist.size(); i++)
		{
			if (face.v1 == nodelist[i])
			{
				vertIndex1 = i;
				break;
			}
		}
		for (int i = 0; i < nodelist.size(); i++)
		{
			if (face.v2 == nodelist[i])
			{
				vertIndex2 = i;
				break;
			}
		}
		for (int i = 0; i < nodelist.size(); i++)
		{
			if (face.v3 == nodelist[i])
			{
				vertIndex3 = i;
				break;
			}
		}

		FFace tempFace(vertIndex1, vertIndex2, vertIndex3);
		facelist.push_back(tempFace);
	}

	for (int i = 0; i < facelist.size(); i++)
	{
		bool isNewEdge = true;
		for (int j = 0; j < edgelist.size(); j++)
		{
			if (edgelist[j] == facelist[i].edge1)
			{
				isNewEdge = false;
				edgelist[j].faceB = i;
				facelist[i].edge1Index = j;
				break;
			}
		}
		if (isNewEdge)
		{
			FEdge tempEdge(facelist[i].edge1.X(), facelist[i].edge1.Y());
			tempEdge.faceA = i;
			edgelist.push_back(tempEdge);
			facelist[i].edge1Index = edgelist.size() - 1;
		}

		isNewEdge = true;
		for (int j = 0; j < edgelist.size(); j++)
		{
			if (edgelist[j] == facelist[i].edge2)
			{
				isNewEdge = false;
				edgelist[j].faceB = i;
				facelist[i].edge2Index = j;
				break;
			}
		}
		if (isNewEdge)
		{
			FEdge tempEdge(facelist[i].edge2.X(), facelist[i].edge2.Y());
			tempEdge.faceA = i;
			edgelist.push_back(tempEdge);
			facelist[i].edge2Index = edgelist.size() - 1;
		}

		isNewEdge = true;
		for (int j = 0; j < edgelist.size(); j++)
		{
			if (edgelist[j] == facelist[i].edge3)
			{
				isNewEdge = false;
				edgelist[j].faceB = i;
				facelist[i].edge3Index = j;
				break;
			}
		}
		if (isNewEdge)
		{
			FEdge tempEdge(facelist[i].edge3.X(), facelist[i].edge3.Y());
			tempEdge.faceA = i;
			edgelist.push_back(tempEdge);
			facelist[i].edge3Index = edgelist.size() - 1;
		}
	}
}

bool DelaunayMachine::belongToSameBorder(FEdge aimEdge, FEdge objEdge)
{
	/*bool find = false;
	FEdge aimBorder;
	for (auto vert : onEdgePList)
	{
		if (vert.p == aimEdge.vertA || vert.p == aimEdge.vertB)
		{
			find = true;
			aimBorder = vert.edge;
			break;
		}
	}
	if (find)
	{
		if (objEdge == aimBorder)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}*/

	Point3m v1, v2, v3;
	v1 = aimEdge.vertB - aimEdge.vertA;
	v2 = objEdge.vertB - objEdge.vertA;
	v3 = objEdge.vertA - aimEdge.vertA;
	v1 = v1.Normalize();
	v2 = v2.Normalize();
	v3 = v3.Normalize();
	if ((v1.X() * v2.Y() - v1.Y() * v2.Y()) == 0 && v1 * v3 == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

 bool DelaunayMachine::getRidOfOutsideBoundaryTriangles(const vector<FFace> &splitFaces, const vector<FEdge> &_edgelist, const Point3m &judgeNormal, vector<FFace> &result)
{
     result.clear();
	bool needGetRid;
	for (int i = 0; i < splitFaces.size(); i++)
	{
		needGetRid = ifThisFaceOutofBoundary(splitFaces[i], _edgelist, -judgeNormal);

		if (!needGetRid)
		{
			result.push_back(splitFaces[i]);
		}
	}

    return true;
}

bool DelaunayMachine::ifThisFaceOutofBoundary(const FFace &face, const vector<FEdge> &orderedBoundary, const Point3m &judgeNormal)
{
	int borderIndex;
	bool exitEdgeOnBoundary = false;

	FEdge EdgeOne(face.v1, face.v2);
	for (int i = 0; i < orderedBoundary.size(); ++i)
	{
		if (EdgeOne == orderedBoundary[i])
		{
			exitEdgeOnBoundary = true;
			borderIndex = i;
			break;
		}
	}
	if (exitEdgeOnBoundary)
	{
		Point3m center = (face.v1 + face.v2 + face.v3) / 3.0f;
		Point3m edge1, edge2;
		edge1 = center - orderedBoundary[borderIndex].vertA;
		edge2 = orderedBoundary[borderIndex].vertB - orderedBoundary[borderIndex].vertA;
		Point3m normalV = (edge1 ^ edge2);
		float judgeValue = normalV * judgeNormal;
		if (judgeValue > 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	////////////////////////////////////////////////////////////////////////////
	FEdge EdgeTwo(face.v2, face.v3);
	for (int i = 0; i < orderedBoundary.size(); ++i)
	{
		if (EdgeTwo == orderedBoundary[i])
		{
			exitEdgeOnBoundary = true;
			borderIndex = i;
			break;
		}
	}
	if (exitEdgeOnBoundary)
	{
		Point3m center = (face.v1 + face.v2 + face.v3) / 3.0f;
		Point3m edge1, edge2;
		edge1 = center - orderedBoundary[borderIndex].vertA;
		edge2 = orderedBoundary[borderIndex].vertB - orderedBoundary[borderIndex].vertA;
		Point3m normalV = (edge1 ^ edge2);
		float judgeValue = normalV * judgeNormal;
		if (judgeValue > 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	////////////////////////////////////////////////////////////////////////////
	FEdge EdgeThree(face.v3, face.v1);
	for (int i = 0; i < orderedBoundary.size(); ++i)
	{
		if (EdgeThree == orderedBoundary[i])
		{
			exitEdgeOnBoundary = true;
			borderIndex = i;
			break;
		}
	}
	if (exitEdgeOnBoundary)
	{
		Point3m center = (face.v1 + face.v2 + face.v3) / 3.0f;
		Point3m edge1, edge2;
		edge1 = center - orderedBoundary[borderIndex].vertA;
		edge2 = orderedBoundary[borderIndex].vertB - orderedBoundary[borderIndex].vertA;
		Point3m normalV = (edge1 ^ edge2);
		float judgeValue = normalV * judgeNormal;
		if (judgeValue > 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	bool result = ifThisFaceOutofBoundaryByItersectionMethod(face, orderedBoundary, judgeNormal);
	return result;
}

bool DelaunayMachine::ifThisFaceOutofBoundaryByItersectionMethod(const FFace &face, const vector<FEdge> &orderedBoundary, const Point3m &judgeNormal)
{
	Point3m startVert, endVert;
	startVert = (face.v1 + face.v2 + face.v3) / 3.0f;
	endVert = this->STriangle.v2;

	Point3m uselessPoint;
	int intersectTime = 0;
	WordEdge judgeEdge(startVert, endVert);
	for (auto edge : orderedBoundary)
	{
		WordEdge aimEdge(edge);
		if (judgeEdge.intersectWith(aimEdge, judgeNormal, uselessPoint))
		{
			intersectTime += 1;
		}
	}

	if (intersectTime % 2 == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

vector<vector<FFace>> DelaunayMachine::translateSteps(vector<vector<FFace>> faceMatrix, FFace superTriangle, vector<Point3f> projVertexes, vector<Point3m> _vertexs)
{
	Point3m stV0, stV1, stV2;
	stV0 = superTriangle.v1;
	stV1 = superTriangle.v2;
	stV2 = superTriangle.v3;

	vector<vector<FFace>> result;
	for(auto facelist : faceMatrix)
	{
		vector<FFace> splitFaces;
		for (int i = 0; i < facelist.size(); ++i)
		{
			int outSign[3];
			Point3f factVert[3];
			bool get1 = false, get2 = false, get3 = false;
			for (int j = 0; j < projVertexes.size(); j++)
			{
				if (facelist[i].v1 == stV0)
				{
					factVert[0] = this->sourceSTriangle.v1;
					get1 = true;
					break;
				}
				if (facelist[i].v1 == stV1)
				{
					factVert[0] = this->sourceSTriangle.v2;
					get1 = true;
					break;
				}
				if (facelist[i].v1 == stV2)
				{
					factVert[0] = this->sourceSTriangle.v3;
					get1 = true;
					break;
				}
				if (facelist[i].v1 == projVertexes[j])
				{
					factVert[0] = _vertexs[j];
					get1 = true;
					break;
				}
			}
			for (int j = 0; j < projVertexes.size(); j++)
			{
				if (facelist[i].v2 == stV0)
				{
					factVert[1] = this->sourceSTriangle.v1;
					get2 = true;
					break;
				}
				if (facelist[i].v2 == stV1)
				{
					factVert[1] = this->sourceSTriangle.v2;
					get2 = true;
					break;
				}
				if (facelist[i].v2 == stV2)
				{
					factVert[1] = this->sourceSTriangle.v3;
					get2 = true;
					break;
				}
				if (facelist[i].v2 == projVertexes[j])
				{
					factVert[1] = _vertexs[j];
					get2 = true;
					break;
				}
			}
			for (int j = 0; j < projVertexes.size(); j++)
			{
				if (facelist[i].v3 == stV0)
				{
					factVert[2] = this->sourceSTriangle.v1;
					get3 = true;
					break;
				}
				if (facelist[i].v3 == stV1)
				{
					factVert[2] = this->sourceSTriangle.v2;
					get3 = true;
					break;
				}
				if (facelist[i].v3 == stV2)
				{
					factVert[2] = this->sourceSTriangle.v3;
					get3 = true;
					break;
				}
				if (facelist[i].v3 == projVertexes[j])
				{
					factVert[2] = _vertexs[j];
					outSign[2] = ONDENTAL;
					get3 = true;
					break;
				}
			}
			splitFaces.push_back(FFace(factVert[0], factVert[2], factVert[1]));
		}

		result.push_back(splitFaces);
	}

	return result;
}

vector<Point3m> DelaunayMachine::translateVerts(vector<Point3m> vertlist, vector<Point3f> projVertexes, vector<Point3m> _vertexs)
{
	vector<Point3m> result;
	for (int i = 0; i < vertlist.size(); ++i)
	{
		for (int j = 0; j < projVertexes.size(); j++)
		{
			if (vertlist[i] == projVertexes[j])
			{
				result.push_back(_vertexs[j]);
				break;
			}
		}
	}

	return result;
}

void DelaunayMachine::computeCoordinateIn2D(Point3f E3, float &fu, float &fv, Point3f axisX, Point3f axisY, Point3f U, Point3f V)
{
	float fUx, fUy, fVx, fVy, fX, fY;
	fUx = U * axisX;
	fUy = U * axisY;
	fVx = V * axisX;
	fVy = V * axisY;
	fX = E3 * axisX;
	fY = E3 * axisY;

	fv = (fY * fUx - fX * fUy) / (fVy * fUx - fVx * fUy);
	fu = (fX + fY - fv * (fVx + fVy)) / (fUx + fUy);
	return;
}

void DelaunayMachine::coordinateIn2D(Point3f e, float &fu, float &fv, Point3f origin, Point3f axisX, Point3f axisY)
{
	Point3m edge = e - origin;
	fv = edge * axisX;
	fu = edge * axisY;
}
