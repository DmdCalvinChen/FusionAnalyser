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

#include "interactive2Dframe.h"
#include "GL/glew.h"

static QColor Normal_Frame_Color_(95, 102, 113);
static QColor Covered_Frame_Color_(49, 61, 73);
static QColor Focus_Frame_Color_(218, 220, 224);
static QColor Covered_Frame_Background_Color_(255, 255, 255, 25);
static QColor Reference_Frame_Background_Color_(15, 22, 31);
static int Frame_Line_Width_ = 4;
static int Focus_Frame_Line_Width_ = 6;

Interactive2DFrame::Interactive2DFrame()
{

}

Interactive2DFrame::Interactive2DFrame(Interactive2DLabel::AccompanyMotionId _id, vector<Interactive2DLabel> _include_labels, bool _enable_interactive , bool _is_reference )
{
	this->motion_id_ = _id;
	this->include_labels_ = _include_labels;
	this->interactive_ = _enable_interactive;
	this->reference_ = _is_reference;
	updateFrameSize();
}

void Interactive2DFrame::updateFrameSize()
{
	vcg::Box2i box;
	for (auto& label : include_labels_)
	{
		box.Add(Point2i(label.show_pos_.X(), label.show_pos_.Y() - label.text_height_));
		box.Add(Point2i(label.show_pos_.X() + label.text_width_, label.show_pos_.Y()));
	}
	int left_margin = 10;
	int right_margin = 10;
	int top_margin = 0;
	int bottom_margin = 20;

	QPoint topleft(box.min.X() - left_margin, box.min.Y() - top_margin);
	QPoint bottomright(box.max.X() + right_margin, box.max.Y() + bottom_margin);
	this->show_pos_ = Point2i(topleft.x(), topleft.y());
	rect_ = QRect(topleft, bottomright);
}

void Interactive2DFrame::setWidthCompensation(float _value)
{
	QPoint topleft = rect_.topLeft();
	QPoint bottomright = rect_.bottomRight();
	bottomright.setX(bottomright.x() + _value);
	rect_ = QRect(topleft, bottomright);
}

void Interactive2DFrame::setIsCheckBox(bool _state)
{
	is_check_box_ = _state;
	for (int i = 0; i < include_labels_.size(); ++i)
	{
		if (include_labels_[i].interactive_)
		{
			include_labels_[i].setCheckable(_state);
			include_labels_[i].updateBoundingBox();
		}
	}
	updateFrameSize();
}

void Interactive2DFrame::updateCheckboxState(Interactive2DLabel::AccompanyMotionId _checked_item_id)
{
	if (is_check_box_)
	{
		for (int i = 0; i < this->include_labels_.size(); ++i)
		{
			if (include_labels_[i].checkable_)
			{
				if (include_labels_[i].motion_id_ == _checked_item_id)
				{
					include_labels_[i].setChecked(true);
				}
				else
				{
					include_labels_[i].setChecked(false);
				}
			}
		}
	}

	if (p_motion_)
	{
		*p_motion_ = _checked_item_id;
	}
}

void Interactive2DFrame::draw(QPainter* _painter, bool _dark_mode )
{
	drawFrame(_painter, _dark_mode);

	if (!include_labels_.empty())
	{
		for (int i = 0; i < include_labels_.size(); ++i)
		{
			include_labels_[i].draw(_painter);
		}
	}
}

void Interactive2DFrame::drawFrame(QPainter* _painter, bool _dark_mode)
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

	_painter->setRenderHint(QPainter::SmoothPixmapTransform);
	if (interactive_)
	{
		if (covered_)
		{
			_painter->setPen(QPen(Covered_Frame_Color_, Frame_Line_Width_));
			_painter->setBrush(QBrush(Covered_Frame_Background_Color_));
		}
		else
		{
			_painter->setPen(QPen(Normal_Frame_Color_, Frame_Line_Width_));
			_painter->setBrush(Qt::BrushStyle::NoBrush);
		}

		if (focus_)
		{
			_painter->setPen(QPen(Focus_Frame_Color_, Focus_Frame_Line_Width_));
		}
	}
	else
	{
		if (reference_)
		{
			_painter->setPen(Qt::PenStyle::NoPen);
			_painter->setBrush(QBrush(Reference_Frame_Background_Color_));
		}
		else
		{
			_painter->setPen(Qt::PenStyle::NoPen);
			_painter->setBrush(Qt::BrushStyle::NoBrush);
		}
	}
	_painter->drawRoundedRect(rect_, 10, 10);

	_painter->beginNativePainting();
	_painter->restore();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}

bool Interactive2DFrame::mousePress(int _mouse_x, int _mouse_y, Interactive2DLabel::AccompanyMotionId& _active_motion_id)
{
	if (!interactive_)
	{
		covered_ = false;
		return false;
	}

	if (!include_labels_.empty())
	{
		for (int i = 0; i < include_labels_.size(); ++i)
		{
			if (include_labels_[i].mousePress(_mouse_x, _mouse_y))
			{
				_active_motion_id = include_labels_[i].motion_id_;
				this->focus_ = true;
				return true;
			}
		}
	}

	if (mouseWithinTheScopeOfFrame(_mouse_x, _mouse_y))
	{
		_active_motion_id = this->motion_id_;
		this->focus_ = true;
		return true;
	}
	else
	{
		this->focus_ = false;
		return false;
	}
}

bool Interactive2DFrame::mouseMove(int _mouse_x, int _mouse_y)
{
	if (!interactive_)
	{
		covered_ = false;
		return false;
	}

	if (!include_labels_.empty())
	{
		for (int i = 0; i < include_labels_.size(); ++i)
		{
			include_labels_[i].mouseMove(_mouse_x, _mouse_y);
		}
	}

	if (mouseWithinTheScopeOfFrame(_mouse_x, _mouse_y))
	{
		covered_ = true;
	}
	else
	{
		covered_ = false;
	}
	return covered_;
}

bool Interactive2DFrame::mouseRelease(int _mouse_x, int _mouse_y)
{
	if (!include_labels_.empty())
	{
		for (int i = 0; i < include_labels_.size(); ++i)
		{
			include_labels_[i].mouseRelease(_mouse_x, _mouse_y);
		}
	}

	return false;
}

bool Interactive2DFrame::mouseWithinTheScopeOfFrame(int _mouse_x, int _mouse_y)
{
	if (show_pos_.X() < _mouse_x && _mouse_x < show_pos_.X() + rect_.width() &&
		show_pos_.Y() < _mouse_y && _mouse_y < show_pos_.Y() + rect_.height())
	{
		return true;
	}
	else
	{
		return false;
	}
}
