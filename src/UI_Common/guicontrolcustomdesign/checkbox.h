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

#ifndef CHECK_BOX_H
#define CHECK_BOX_H

#include "../uicommon_global.h"
#include "custombuttondesignbase.h"
#include "label.h"

/*
** check box designed by liuchuang on 22/4/2019.
*/

namespace ui_common
{
class UI_COMMONSHARED_EXPORT CheckBox : public QObject, public CustomButtonDesignBase
{
    Q_OBJECT
public:
    CheckBox()
    {
    }
    CheckBox(const QString& _desrction, const QString& _helpDesrction,
             TipShowDirection _tipShowDirection = DOWN_TIP_SHOW_DIRECTION,
             const ControlTypeId& _typeControl = DEFAULT_CONTROL);
    virtual ~CheckBox();

public:
    void setAttributeInfomation(const BaseAttributeInfoButton& _baseAttributeInfomation);
    void mouseMoveCustomEvent(QWidget* parent, QPainter* painter, QPoint posCursor);
    void mousePressEvent(QPoint posCursor);
    void mouseReleaseEvent(QPoint posCursor)
    {
    }
    void draw(QWidget* parent, QPainter* painter, QPoint posCursor,
              const Matrix44m& matrixTransform, bool& _bIsPickedThisControl);
    bool cursorIsStayOnThisButton(QPoint cursorPos, Point3m& transformCursor);
    void setCurrentPlayStatus(bool _bIsPlaying = false);

signals:
    void clicked();
    void stateChanged(int bIsCheckedStatus);

public:
    void setChecked(bool _bIsChecked);
    void setVisible(bool _bIsVisible);

public:
    BaseAttributeInfoButton baseAttributeInfomation;

private:
    Matrix44m matrixTransform;
    QWidget* parent;
    int numDiv = 10;
    int count = 0;
    int loop = 1;
};

}    // namespace ui_common
#endif
