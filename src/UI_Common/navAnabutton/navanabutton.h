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

#ifndef NAVANABUTTON_H
#define NAVANABUTTON_H

#include <QLineEdit>
#include <QPushButton>

// Inline uicommon_global.h to avoid circular dependency
#include <QtCore/qglobal.h>
#ifdef UI_COMMON_LIBRARY
#    define UI_COMMONSHARED_EXPORT Q_DECL_EXPORT
#else
#    define UI_COMMONSHARED_EXPORT Q_DECL_IMPORT
#endif

class UI_COMMONSHARED_EXPORT navAnabutton : public QPushButton
{
    Q_OBJECT

public:
    navAnabutton(QWidget* parent = nullptr);
    ~navAnabutton() override;

    QString pixmapName;
    QColor frameColor;
    QColor normalColor;
    QColor pressColor;
    void setpixmapName(QString pixmapName);
    void setFrameColor(QColor frameColor);
    void setNormalColor(QColor normalColor);
    void setPressColor(QColor pressColor);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    bool isHover = false;
    bool isPressed = false;
};

#endif    // NAVANABUTTON_H
