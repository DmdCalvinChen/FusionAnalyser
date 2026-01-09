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

#include "rigidbody.h"

Rigidbody::Rigidbody()
{
}

Rigidbody::Rigidbody(const vector<Point3m> &_sourceConvex, Point3m _sourceConvexCenter)
{
    initData(_sourceConvex, _sourceConvexCenter);
}

Rigidbody::~Rigidbody()
{
	this->activeRigid = NULL;
	this->passiveRigids.clear();
	this->sourceConvex.clear();
	this->currentConvex.clear();
}

void Rigidbody::setFixed(bool _state)
{
	this->fixed_ = _state;
}

bool Rigidbody::updateCurrentConvex()
{
	if (this->activeRigid == NULL)
	{
		this->stressTransferVector = Point3m(0, 0, 0);
		return false;
	}

	GJKMachine gjkTestMachine;
	vector<Point3m> convexA, convexB;
	Point3m convexACenter, convexBCenter;
	convexA = this->activeRigid->currentConvex;
	convexACenter = this->activeRigid->currentConvexCenter;
	convexB = this->sourceConvex;
	convexBCenter = this->sourceConvexCenter;

	if (!convexA.empty() && !convexB.empty())
	{
		Point3m transferVector;
		bool stressTransferHappend = gjkTestMachine.test(convexACenter, convexA, convexBCenter, convexB, transferVector, this->fFrozenDistance);

		bool resistence = false;
		if (stressTransferHappend)
		{
			if (this->fixed_)
			{
				this->stressTransferVector = Point3m(0,0,0);
				return true;
			}

			this->transferCurrentConvexWith(transferVector);
			if (!this->passiveRigids.empty())
			{
				for (int i = 0; i < this->passiveRigids.size(); ++i)
				{
					if (this->passiveRigids[i] != NULL)
					{
						resistence = this->passiveRigids[i]->updateCurrentConvex();

						if (resistence)
						{
							this->stressTransferVector = Point3m(0, 0, 0);
						}
						else
						{
							this->stressTransferVector = transferVector;
						}
					}
				}
			}
			else
			{
				this->stressTransferVector = transferVector;
			}
		}
		else
		{
			this->stressTransferVector = Point3m(0, 0, 0);
		}
		return resistence;
	}
	this->stressTransferVector = Point3m(0, 0, 0);
	return false;
}

void Rigidbody::transferCurrentConvexWith(Point3m transferVector)
{
	this->currentConvex.clear();
	for (int i = 0; i < this->sourceConvex.size(); ++i)
	{
		this->currentConvex.push_back(sourceConvex[i] + transferVector);
	}
	this->currentConvexCenter = sourceConvexCenter + transferVector;
}

void Rigidbody::initData(const std::vector<Point3m>& _sourceConvex, Point3m &_sourceConvexCenter)
{
    this->stressTransferVector = Point3m(0, 0, 0); // Stress deformation vector
    this->activeRigid = NULL; // Active rigid body applying force
    this->passiveRigids.clear();

    this->sourceConvexCenter = _sourceConvexCenter;
    this->sourceConvex = _sourceConvex;
    this->currentConvexCenter = _sourceConvexCenter;
    this->currentConvex = _sourceConvex;
	this->fixed_ = false;

    this->fFrozenDistance = EFFECTIVE_CREVICE_WIDTH;
}
