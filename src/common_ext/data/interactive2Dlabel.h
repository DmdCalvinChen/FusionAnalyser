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

#ifndef INTERACTIVE2DLABEL_H
#define INTERACTIVE2DLABEL_H

#include "util/assist_geometry.h"
#include "qstring.h"
#include "qpainter.h"
#include "common_ext_global.h"

class COMMON_EXT_EXPORT Interactive2DLabel
{
public:
	enum AccompanyMotionId {
		DEFAULT_VIEW,
		TOP_VIEW,
		BOTTOM_VIEW,
		FRONT_VIEW,
		BACK_VIEW,
		LEFT_VIEW,
		RIGHT_VIEW,
		FRONT_SHOW_ONLY_UPPER,
		MIDDLE_SHOW_ONLY_UPPER,
		REAR_SHOW_ONLY_UPPER,
		BASALBONE_SHOW_ONLY_UPPER,
		FRONT_SHOW_ONLY_LOWER,
		MIDDLE_SHOW_ONLY_LOWER,
		REAR_SHOW_ONLY_LOWER,
		BASALBONE_SHOW_ONLY_LOWER,
		BASALBONE_WID_SHOW_ONLY_UPPER,
		BASALBONE_LEN_SHOW_ONLY_UPPER,
		BASALBONE_WID_SHOW_ONLY_LOWER,
		BASALBONE_LEN_SHOW_ONLY_LOWER,
		CROWDING_SWITCH_TO_5_5,
		CROWDING_SWITCH_TO_7_7
	};

	Interactive2DLabel();
	~Interactive2DLabel();
	Interactive2DLabel(AccompanyMotionId _id, QString _context, Point2i _pos, bool _b_interactive = true);
	Interactive2DLabel(AccompanyMotionId _id, QString _context, Point2i _pos, int _size, bool _b_interactive = true);
	void updateBoundingBox();

	void draw(QPainter *_painter, bool _dark_mode = true);
	bool mousePress(int _mouse_x, int _mouse_y);
	bool mouseMove(int _mouse_x, int _mouse_y);
	bool mouseRelease(int _mouse_x, int _mouse_y);
	inline void setCheckable(bool _enable) { checkable_ = _enable; }
	inline void setChecked(bool _state) { checked_ = _state; }
	inline void setEnable(bool _state) { enable_ = _state; }
	inline void setTextColor(QColor _color) { text_color_ = _color; }
	static void setFontSize(int _size);

public:
	AccompanyMotionId motion_id_ = DEFAULT_VIEW;
	QRect bounding_rect_;
	QString context_;
	Point2i show_pos_;
	bool interactive_ = true, clicked_ = false;
	bool checkable_ = false, checked_ = false;
	bool enable_ = true;
	int size_ = 10;
	bool covered_ = false;
	int text_width_, text_height_;
	QColor text_color_ = QColor(255, 255, 255);
	QFont font_;

public:
	static int font_size_;
};

#endif

