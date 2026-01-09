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

#include "navanabutton.h"

#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QPainter>
#include <QPen>
#include <QRect>

navAnabutton::navAnabutton(QWidget* parent) : QPushButton(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setFlat(true);
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    this->setStyleSheet(
            ".QPusButton{\
		iconSize:32px 32px;\
	    border-style:flat;\
        background:transparent;\
        }");
}

navAnabutton::~navAnabutton()
{
}

void navAnabutton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    QColor blue;
    if (isHover)
        blue = pressColor;
    else
        blue = normalColor;

    if (isPressed)
        blue = pressColor;

    // draw background of the button
    painter.fillRect(this->rect(), QColor(155, 0, 0, 125));
}

void navAnabutton::enterEvent(QEvent* event)
{
    isHover = true;
    QPushButton::enterEvent(event);
}

void navAnabutton::leaveEvent(QEvent* event)
{
    isHover = false;
    QPushButton::leaveEvent(event);
}

void navAnabutton::mousePressEvent(QMouseEvent* event)
{
    isPressed = true;
    QPushButton::mousePressEvent(event);
}

void navAnabutton::mouseReleaseEvent(QMouseEvent* event)
{
    isPressed = false;
    QPushButton::mouseReleaseEvent(event);
}

void navAnabutton::setFrameColor(QColor frameColor)
{
    this->frameColor = frameColor;
}

void navAnabutton::setPressColor(QColor pressColor)
{
    this->pressColor = pressColor;
}

void navAnabutton::setpixmapName(QString pixmapName)
{
    this->pixmapName = pixmapName;
}

void navAnabutton::setNormalColor(QColor normalColor)
{
    this->normalColor = normalColor;
}
