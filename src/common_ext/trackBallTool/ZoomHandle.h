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

#ifndef COMMON_ZOOMHANDLE_H
#define COMMON_ZOOMHANDLE_H

#include "trackBallTool/customadjustinghandle.h"
#include "common_ext_global.h"
#include "util/assist_geometry.h"
class AbsMesh;
class COMMON_EXT_EXPORT ZoomHandle :
	public CustomAdjustingHandle
{
public:
	ZoomHandle(AbsMesh *_obj, AdjDirect _direct, float _radius, float _base_length, HexaVec *_assist_vec, float _fine_step);
	ZoomHandle(AbsMesh *_obj, AdjDirect _direct, float _radius, float _coeffient, float _base_length, HexaVec *_assist_vec, float _fine_step);
	~ZoomHandle();

	bool drawHandle();
	bool pickHandel(int _mouseX, int _mouseY);
	bool adjustAction(int _mouseX, int _mouseY);
	bool adjustFineTuneAction(int _direct);
	void updateCtrlSys();
	void carroutTransform(float _value) {}
	void afterMouseRelease() {}
	Point3m currentMoveDirect() { return Point3m(0, 0, 0); }

private:
	Point3m node_;
	float base_length_;
	float *zoom_value_ = nullptr;
	float *adj_move_value = nullptr;
	HexaVec *assist_vec_ = nullptr;
	AdjDirect direct_;
};

#endif // !COMMON_ZOOMHANDLE_H
