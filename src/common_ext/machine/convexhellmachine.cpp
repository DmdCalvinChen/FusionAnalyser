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

#include "convexhellmachine.h"

ConvexHullMachine::ConvexHullMachine()
{
}

ConvexHullMachine::ConvexHullMachine(const vector<Point3m> &_a, Axis _localAxis)
{
	this->A = _a;
	this->localAxis = _localAxis;
	this->initalMachine();
}

ConvexHullMachine::~ConvexHullMachine()
{
}

void ConvexHullMachine::inital(const vector<Point3m> &_a, Axis _localAxis)
{
	this->A = _a;
	this->localAxis = _localAxis;
	this->initalMachine();
	this->creatConvexHull();
	return;
}

void ConvexHullMachine::initalConvexVerts(const vector<Point3m>& _a)
{
	this->convexVerts = _a;
}

void ConvexHullMachine::initalMachine()
{
	Point3m directD, directA, directB, directC;
	directD = localAxis.axisYVector;
	directA = (localAxis.axisZVector * sin(120) - localAxis.axisYVector * cos(60)).Normalize();
	directB = ((localAxis.axisXVector * sin(120) - localAxis.axisZVector * cos(60)).Normalize() * sin(120) - localAxis.axisYVector * cos(60)).Normalize();
	directC = ((localAxis.axisXVector * sin(-120) - localAxis.axisZVector * cos(60)).Normalize() * sin(120) - localAxis.axisYVector * cos(60)).Normalize();

	int id, ia, ib, ic;
	Point3m d, a, b, c;
	d = getFurtherestVert(directD, this->A, id);
	a = getFurtherestVert(directA, this->A, ia);
	b = getFurtherestVert(directB, this->A, ib);
	c = getFurtherestVert(directC, this->A, ic);
	if (A.size() == 0)
		return;

	swap(A[0], A[id]);
	swap(A[1], A[ia]);
	swap(A[2], A[ib]);
	swap(A[3], A[ic]);

	this->n = A.size();
	this->F.clear();
	for (int i = 0; i < n * 8; ++i)
	{
		F.push_back(ConvexFace());
	}
	this->g.clear();
    for (int i = 0; i < n; ++i)
    {
        vector<int> list(n, -1);
        g.push_back(list);
    }
	return;
}

Point3m ConvexHullMachine::getFurtherestVert(Point3m direction, const vector<Point3m> &vertList, int &index)
{
	float maxProj;
	int maxVertIndex = 0;
	if (vertList.size() == 0) return Point3m(0, 0, 0);
	maxProj = (vertList[0] - localAxis.centerPoint) * direction;

	float tempProj;
	for (int i = 1; i < vertList.size(); ++i)
	{
		tempProj = (vertList[i] - localAxis.centerPoint) * direction;
		if (tempProj > maxProj)
		{
			maxProj = tempProj;
			maxVertIndex = i;
		}
	}

	index = maxVertIndex;
	return vertList[index];
}

void ConvexHullMachine::creatConvexHull()
{
	int i, j, tmp;
	ConvexFace add;
	bool flag = true;
	num = 0;
	if (n<4)
		return;

	for (i = 0; i<4; i++)
	{
		add.ia = (i + 1) % 4;
		add.ib = (i + 2) % 4;
		add.ic = (i + 3) % 4;
		add.bOk = 1;

		if(A.size() != 0)
		{
			if (dblcmp(A[i], add) > EPS)
			{
				swap(add.ib, add.ic);
			}

			g[add.ia][add.ib] = g[add.ib][add.ic] = g[add.ic][add.ia] = num;
			F[num++] = add;
		}
	}

	for (i = 4; i<n; i++)
	{
		for (j = 0; j<num; j++)
		{
			if (F[j].bOk && A.size() != 0 &&  dblcmp(A[i], F[j]) > EPS)
			{
				dfs(i, j);
				break;
			}
		}
	}

	tmp = num;
	for (i = num = 0; i < tmp; i++)
	{
		if (F[i].bOk)
		{
			F[num++] = F[i];
		}
	}

	// Collect convex hull vertices
	this->collectVertsList();
	this->convexFaces.clear();
	for (int i = 0; i < num; ++i)
	{
		convexFaces.push_back(FFace(A[F[i].ia], A[F[i].ib], A[F[i].ic]));
	}
	this->F.clear();
	for (int i = 0; i < g.size(); ++i)
	{
		this->g[i].clear();
	}

	return;
}

void ConvexHullMachine::creatConvexHullSingleStep()
{
	int i = this->iCurstep;
	if (i < n)
	{
		for (int j = 0; j<num; j++)
		{
			if (F[j].bOk && dblcmp(A[i], F[j]) > EPS)
			{
				dfs(i, j);
				break;
			}
		}

		int tmp = num;
		convexFaces.clear();
		for (int ii = 0; ii < tmp; ii++)
		{
			if (F[ii].bOk)
			{
				convexFaces.push_back(FFace(A[F[ii].ia], A[F[ii].ib], A[F[ii].ic]));
			}
		}

		this->iCurstep += 1;
	}
}

double ConvexHullMachine::dblcmp(Point3m &p, ConvexFace &f)
{
	Point3m m = A[f.ib] - A[f.ia];
	Point3m n = A[f.ic] - A[f.ia];
	Point3m t = p - A[f.ia];
	t = t.Normalize();
	Point3m c = m ^ n;
	c = c.Normalize();
	return c * t;
}

void ConvexHullMachine::dfs(int p, int now)
{
	F[now].bOk = 0;
	deal(p, F[now].ib, F[now].ia);
	deal(p, F[now].ic, F[now].ib);
	deal(p, F[now].ia, F[now].ic);
}

void ConvexHullMachine::deal(int p, int a, int b)
{
	int f = g[a][b];
	ConvexFace add;
	if (f >= 0 && F[f].bOk)
	{
		double tempJudgeValue = dblcmp(A[p], F[f]);
		if (tempJudgeValue > EPS)
		{
			dfs(p, f);
		}
		else
		{
			add.ia = b;
			add.ib = a;
			add.ic = p;
			add.bOk = 1;
			g[p][b] = g[a][p] = g[b][a] = num;
			F[num++] = add;
		}

	}
}

void ConvexHullMachine::collectVertsList()
{
	this->convexVerts.clear();
	set<int> s;
	for (int i = 0; i < num; ++i)
	{
		s.insert(F[i].ia);
		s.insert(F[i].ib);
		s.insert(F[i].ic);
	}
	for (auto index : s)
	{
		convexVerts.push_back(A[index]);
	}
	s.clear();
	return;
}
