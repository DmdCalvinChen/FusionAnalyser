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

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "../uicommon_global.h"
#include "custombuttondesignbase.h"
/*
**  RECTANGLE STYLE DESIGNED by liuchuang on 1/8/2019.
**             *
**        *         *
**             *
*/

namespace ui_common
{
class UI_COMMONSHARED_EXPORT Rectangle : public QObject, public CustomButtonDesignBase
{
    Q_OBJECT
public:
    Rectangle()
    {
    }
    Rectangle(const QString& _desrction, const QString& _helpDesrction,
              TipShowDirection _tipShowDirection = DOWN_TIP_SHOW_DIRECTION,
              const ControlTypeId& _typeControl = DEFAULT_CONTROL);
    virtual ~Rectangle();

public:
    void setAttributeInfomation(const BaseAttributeInfoButton& _baseAttributeInfomation);
    void setAttributeInfomationBtnWidthScalse(float _scale);
    void mouseMoveCustomEvent(QWidget* parent, QPainter* painter, QPoint posCursor);
    void mousePressEvent(QPoint posCursor);
    void mouseReleaseEvent(QPoint posCursor)
    {
    }
    void draw(QWidget* parent, QPainter* painter, QPoint posCursor,
              const vcg::Matrix44f& matrixTransform, bool& _bIsPickedThisControl);
    bool cursorIsStayOnThisButton(QPoint cursorPos, vcg::Point3f& transformCursor);
    void setCurrentPlayStatus(bool _bIsPlaying = false);
    void setBaseAttributeInfoButtonColor(const vcg::Point4f& _color);
    void setBaseAttributeInfoButtonBackgroundColor(const vcg::Point4f& _color);

signals:
    void clicked();
    void stateChanged(int bIsCheckedStatus);

public:
    void setChecked(bool _bIsChecked);
    void setVisible(bool _bIsVisible);

private:
    BaseAttributeInfoButton baseAttributeInfomation;
    vcg::Matrix44f matrixTransform;
    QWidget* parent = nullptr;
    int numDiv = 10;
    int count = 0;
    int loop = 1;
};
}    // namespace ui_common

#endif
