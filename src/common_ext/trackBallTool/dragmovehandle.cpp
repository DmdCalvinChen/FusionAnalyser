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

#include "dragmovehandle.h"
#include "meshExt/AbsMesh.h"
#include "util/utility_tools.h"

DragMoveHandle::DragMoveHandle(AbsMesh *_obj, Point3m _origin, DentalManager* _manager, int *_index, int *_check_sign)
	: CustomAdjustingHandle(_obj, _origin, 0.01f)
{
	associated_manager_ = _manager;
	p_sign_index_ = _index;
	p_check_sign_ = _check_sign;
}

DragMoveHandle::~DragMoveHandle()
{
}

bool DragMoveHandle::drawHandle()
{
	return true;
}

bool DragMoveHandle::pickHandel(int _mouseX, int _mouseY)
{
	if (associated_manager_ == nullptr)
	{
		return false;
	}
	if (p_check_sign_ == nullptr)
	{
		return false;
	}

	return false;
}

bool DragMoveHandle::adjustAction(int _mouseX, int _mouseY)
{
	if (associated_manager_ == nullptr || p_sign_index_ == nullptr)
	{
		return false;
	}
	std::vector<CFaceO*> vf;
	int iTempFaceIndex = -1, iCurrentSelectTooth = -1;
	Point3m newAccessoryCenter, normalVec;

	return false;
}

void DragMoveHandle::updateCtrlSys()
{

}
