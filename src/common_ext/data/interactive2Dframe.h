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

#ifndef INTERACTIVE2DFRAME_H
#define INTERACTIVE2DFRAME_H

#include "util/assist_geometry.h"
#include "qpainter.h"
#include "data/interactive2Dlabel.h"
#include "common_ext_global.h"

class COMMON_EXT_EXPORT Interactive2DFrame
{
public:
	Interactive2DFrame();
	Interactive2DFrame(Interactive2DLabel::AccompanyMotionId _id, vector<Interactive2DLabel> _include_labels, bool _enable_interactive = true, bool _is_reference = false);
	void updateFrameSize();
	void updateCheckboxState(Interactive2DLabel::AccompanyMotionId _checked_item_id);
	void setIsCheckBox(bool _state);

	void draw(QPainter* _painter, bool _dark_mode = true);
	void drawFrame(QPainter* _painter, bool _dark_mode);
	bool mousePress(int _mouse_x, int _mouse_y, Interactive2DLabel::AccompanyMotionId &_active_motion_id);
	bool mouseMove(int _mouse_x, int _mouse_y);
	bool mouseRelease(int _mouse_x, int _mouse_y);
	bool mouseWithinTheScopeOfFrame(int _mouse_x, int _mouse_y);

	void unFocus() { focus_ = false; }
	void setSpecialSign(bool _state) { special_sign_ = _state; }
	void setMotionPointer(Interactive2DLabel::AccompanyMotionId* _p) { p_motion_ = _p; }
	void setWidthCompensation(float _value);

public:
	Interactive2DLabel::AccompanyMotionId motion_id_ = Interactive2DLabel::DEFAULT_VIEW;
	Interactive2DLabel::AccompanyMotionId *p_motion_ = nullptr;
	vector<Interactive2DLabel> include_labels_;
	bool interactive_ = true;
	bool reference_ = false;
	bool covered_ = false;
	bool focus_ = false;
	Point2i show_pos_;
	int width_, height_;
	QRect rect_;
	bool special_sign_ = true;
	bool is_check_box_ = false;
};

#endif
