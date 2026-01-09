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

#ifndef LABEL_H
#define LABEL_H

#include "../uicommon_global.h"
#include "custombuttondesignbase.h"

/*
**  LABEL STYLE DESIGNED by liuchuang on 19/3/2019.
*/

namespace ui_common
{
class UI_COMMONSHARED_EXPORT Label : public QObject, public CustomButtonDesignBase
{
    Q_OBJECT
public:
    Label()
    {
    }
    Label(const QString& _desrction, const QString& _helpDesrction,
          TipShowDirection _tipShowDirection = DOWN_TIP_SHOW_DIRECTION,
          const ControlTypeId& _typeControl = DEFAULT_CONTROL);
    ~Label();

public:
    void setVisible(bool _bIsVisible);
    void setAttributeInfomation(const BaseAttributeInfoButton& _baseAttributeInfomation);
    void mouseMoveCustomEvent(QWidget* parent, QPainter* painter, QPoint posCursor);
    void mousePressEvent(QPoint posCursor);
    void mouseReleaseEvent(QPoint posCursor)
    {
    }
    void draw(QWidget* parent, QPainter* painter, QPoint posCursor,
              const Matrix44m& matrixTransform, bool& _bIsPickedThisControl);
    bool cursorIsStayOnThisButton(QPoint cursorPos, Point3m& transformCursor);

signals:
    void clicked();

private:
    BaseAttributeInfoButton baseAttributeInfomation;
    Matrix44m matrixTransform;
    QWidget* parent = nullptr;
};
}    // namespace ui_common

#endif
