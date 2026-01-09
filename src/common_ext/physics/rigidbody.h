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

#ifndef COMMON_RIGIDBODY_H
#define COMMON_RIGIDBODY_H

#include "common/ml_mesh_type.h"
#include "util/assist_geometry.h"
#include "machine/gjkmachine.h"
using namespace std;

#include "common_ext_global.h"
class COMMON_EXT_EXPORT  Rigidbody
{
public:
	Rigidbody();
	Rigidbody(const vector<Point3m> &_sourceConvex, Point3m _sourceConvexCenter);
	~Rigidbody();

public:
	bool updateCurrentConvex();
	void setFixed(bool _state);
	void transferCurrentConvexWith(Point3m transferVector);

    void initData(const std::vector<Point3m> &_sourceConvex, Point3m &_sourceConvexCenter);
	inline bool fixed() { return fixed_; }

	Point3m stressTransferVector; // Stress deformation vector

	Rigidbody* activeRigid = NULL; // Active rigid body applying force
	vector<Rigidbody*> passiveRigids; // All passive rigid bodies receiving force

	Point3m sourceConvexCenter;
	vector<Point3m> sourceConvex; // Original rigid body convex hull
	Point3m currentConvexCenter;
	vector<Point3m> currentConvex; // Current rigid body convex hull
	float fFrozenDistance = EFFECTIVE_CREVICE_WIDTH;

private:
	bool fixed_ = false;
};

#endif // !COMMON_RIGIDBODY_H
