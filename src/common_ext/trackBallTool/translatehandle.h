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

#ifndef  COMMON_TRANSLATEHANDLE_H
#define  COMMON_TRANSLATEHANDLE_H

#include "trackBallTool/customadjustinghandle.h"
#include "common_ext_global.h"
#include "common_ext/util/assist_geometry.h"

class AbsMesh;
class COMMON_EXT_EXPORT  TranslateHandle :
	public CustomAdjustingHandle
{
public:
	TranslateHandle(AbsMesh *_obj, AdjDirect _direct, float _radius, float _fine_step,  bool _b_limit = false, float* _limit_value = nullptr);
	TranslateHandle(AbsMesh *_obj, AdjDirect _direct, float _radius, float _coeffient, float _fine_step, bool _b_limit = false, float* _limit_value = nullptr);
	TranslateHandle(AbsMesh *_obj, Point3m *_origin, Point3m *_axis, float _radius, float _fine_step, bool _b_limit = false, float* _limit_value = nullptr);

	~TranslateHandle();

	bool drawHandle();
	bool pickHandel(int _mouseX, int _mouseY);
	bool adjustAction(int _mouseX, int _mouseY);
	bool adjustFineTuneAction(int _direct);
	void updateCtrlSys();
	void carroutTransform(float _value);
	void carryoutLocalAxisTransformH(Point3m origin, Axis curAxis, float _value);
	void afterMouseRelease() {}
	void setLocalAxisAdjRange(float _range);
	Point3m currentMoveDirect();

private:
	std::pair<Point3m, Point3m> node_;
	bool b_limit_ = false;
	float *limit_value_ = nullptr;
	float adj_enable_radius_ = 0;
	Point3m real_time_translate_direct_;
};

#endif // ! COMMON_TRANSLATEHANDLE_H
