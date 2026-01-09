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

#include "interactive2Dlabel.h"

#include "GL/glew.h"

QColor covered_back_ground_color(255, 255, 255, 50);
QColor clicked_back_ground_color(255, 255, 255, 100);

int Interactive2DLabel::font_size_ = 15;

void Interactive2DLabel::setFontSize(int _size)
{
	font_size_ = _size;
}

Interactive2DLabel::Interactive2DLabel(AccompanyMotionId _id, QString _context, Point2i _pos, bool _b_interactive )
{
	motion_id_ = _id;
	context_ = _context;
	show_pos_ = _pos;
	size_ = 15;
	interactive_ = _b_interactive;
	font_ = QFont("Microsoft YaHei", Interactive2DLabel::font_size_, QFont::Normal, false);
	font_.setLetterSpacing(QFont::AbsoluteSpacing, 1);
	updateBoundingBox();
}

Interactive2DLabel::Interactive2DLabel(AccompanyMotionId _id, QString _context, Point2i _pos, int _size, bool _b_interactive ) : Interactive2DLabel(_id, _context, _pos, _b_interactive)
{
	size_ = _size;
	Interactive2DLabel::font_size_ = _size;
}

Interactive2DLabel::Interactive2DLabel()
{

}

Interactive2DLabel::~Interactive2DLabel()
{

}

void Interactive2DLabel::updateBoundingBox()
{
	int label_append_margin = 0;
	if (checkable_)
	{
		QString checked_sign = QString::fromStdWString(L"●");
		QFontMetrics fm(font_);
		QRect rect = fm.boundingRect(checked_sign);
		label_append_margin += (rect.width() + 10);
	}

	QFontMetrics fm(font_);
	bounding_rect_ = fm.boundingRect(context_);
	text_width_ = bounding_rect_.width() + label_append_margin;
	text_height_ = bounding_rect_.height();

	int left_margin = -1;
	int right_margin = 1;
	int top_margin = 8;
	int bottom_margin = 8;
	QPoint topleft(show_pos_.X() + left_margin, show_pos_.Y() - text_height_ + top_margin);
	QPoint bottomright(show_pos_.X() + text_width_ + right_margin, show_pos_.Y() + bottom_margin);
	bounding_rect_ = QRect(topleft, bottomright);
}

void Interactive2DLabel::draw(QPainter* _painter, bool _dark_mode)
{
	glPushAttrib(GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	_painter->save();
	_painter->endNativePainting();

	QPen textPen(QColor(255, 255, 255, 200));
	textPen.setWidthF(0.2f);
	_painter->setPen(textPen);
	_painter->setFont(font_);
	if (interactive_ && covered_ && enable_)
	{
		_painter->setPen(Qt::PenStyle::NoPen);
		if (clicked_)
		{
			_painter->setBrush(QBrush(clicked_back_ground_color));
		}
		else
		{
			_painter->setBrush(QBrush(covered_back_ground_color));
		}
		_painter->drawRect(bounding_rect_);
	}

	Point2i cur_show_pos = show_pos_;
	if (checkable_)
	{
		if (checked_ && enable_)
		{
			_painter->setPen(QColor(255, 255, 255));
		}
		else
		{
			_painter->setPen(QColor(100, 100, 100));
		}
		QString checked_sign = QString::fromStdWString(L"●");
		_painter->drawText(show_pos_.X(), show_pos_.Y(), checked_sign);

		QFontMetrics fm(font_);
		QRect rect = fm.boundingRect(checked_sign);
		cur_show_pos.X() += (rect.width() + 10);
	}

	if (!enable_)
	{
		_painter->setPen(QColor(100, 100, 100));
	}
	else
	{
		_painter->setPen(text_color_);
	}
	_painter->drawText(cur_show_pos.X(), cur_show_pos.Y(), context_);

	_painter->beginNativePainting();
	_painter->restore();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}

bool Interactive2DLabel::mousePress(int _mouse_x, int _mouse_y)
{
	if (!enable_)
	{
		return false;
	}
	if (!interactive_)
	{
		covered_ = false;
		return false;
	}
	if (show_pos_.X() < _mouse_x && _mouse_x < show_pos_.X() + text_width_ &&
		show_pos_.Y() > _mouse_y && _mouse_y > show_pos_.Y() - text_height_)
	{
		clicked_ = true;
		return true;
	}
	else
	{
		clicked_ = false;
		return false;
	}
}

bool Interactive2DLabel::mouseMove(int _mouse_x, int _mouse_y)
{
	if (!enable_)
	{
		return false;
	}
	if (!interactive_)
	{
		covered_ = false;
		return false;
	}
	if (show_pos_.X() < _mouse_x && _mouse_x < show_pos_.X() + text_width_ &&
		show_pos_.Y() > _mouse_y && _mouse_y > show_pos_.Y() - text_height_)
	{
		covered_ = true;
	}
	else
	{
		covered_ = false;
	}
	return covered_;
}

bool Interactive2DLabel::mouseRelease(int _mouse_x, int _mouse_y)
{
	clicked_ = false;
	if (!interactive_)
	{
		covered_ = false;
		return false;
	}
	return true;
}
