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

#ifndef DRAGMOVEHANDLE_H
#define DRAGMOVEHANDLE_H
#include "common_ext_global.h"
#include "TrackBallTool/customadjustinghandle.h"
// Forward declaration to avoid circular dependency
class DentalManager;

class AbsMesh;
class COMMON_EXT_EXPORT DragMoveHandle :
	public CustomAdjustingHandle
{
public:
	DragMoveHandle(AbsMesh *_obj, Point3m _origin, DentalManager* _manager, int *_index, int *_check_sign);
	~DragMoveHandle();
	bool drawHandle();
	bool pickHandel(int _mouseX, int _mouseY);
	bool adjustAction(int _mouseX, int _mouseY);
	bool adjustFineTuneAction(int _direct) { return false; }
	void updateCtrlSys();
	void carroutTransform(float _value){};
	void afterMouseRelease() {}
	Point3m currentMoveDirect() { return Point3m(0, 0, 0); }

private:
	DentalManager *associated_manager_ = nullptr;
	int *p_sign_index_ = nullptr;
	int *p_check_sign_ = nullptr;
};

#endif //DRAGMOVEHANDLE_H
