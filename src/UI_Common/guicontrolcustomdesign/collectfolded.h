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

#ifndef COLLECTFOLDED_H
#define COLLECTFOLDED_H

#include "../uicommon_global.h"
#include "custombuttondesignbase.h"

/*
**  COLLECTFOLDED STYLE DESIGNED by liuchuang on 1/11/2019.
**        ***********      **********
**     *****  >>  ***      ** <<      ****
**        ***********      **********
*/

namespace ui_common
{
class UI_COMMONSHARED_EXPORT CollectFoldedGui : public QObject, public CustomButtonDesignBase
{
    Q_OBJECT
public:
    CollectFoldedGui()
    {
    }
    CollectFoldedGui(const QString& _desrction, const QString& _helpDesrction,
                     ui_common::TipShowDirection _tipShowDirection = DOWN_TIP_SHOW_DIRECTION,
                     const ui_common::ControlTypeId& _typeControl = DEFAULT_CONTROL);
    virtual ~CollectFoldedGui();

public:
    void setAttributeInfomation(const BaseAttributeInfoButton& _baseAttributeInfomation);
    void setAttributeInfomationBtnWidthScalse(float _scale);
    void mouseMoveCustomEvent(QWidget* parent, QPainter* painter, QPoint posCursor);
    void mousePressEvent(QPoint posCursor);
    void mouseReleaseEvent(QPoint posCursor)
    {
    }
    void draw(QWidget* parent, QPainter* painter, QPoint posCursor,
              const Matrix44m& matrixTransform, bool& _bIsPickedThisControl);
    bool cursorIsStayOnThisButton(QPoint cursorPos, Point3m& transformCursor);
    void setBaseAttributeInfoButtonColor(const Point4f& _color);
    void setBaseAttributeInfoButtonBackgroundColor(const Point4f& _color);
    float getAllLengthControl() const;

signals:
    void clicked();
    void stateChanged(int bIsCheckedStatus);
    void setUnfoldOrFoldStatusSignal(bool bUnfold = true);

public slots:
    void setChecked(bool _bIsChecked);
    void setVisible(bool _bIsVisible);
    void setEnabled(bool _bEnabled);
    void setCurrentPlayStatus(bool _bIsPlaying = false);

private:
    bool bCursorStayOnThisButton;
    BaseAttributeInfoButton baseAttributeInfomation;
    Matrix44m matrixTransform;
    QWidget* parent = nullptr;
    QPainter* painter;
    int numDiv = 10;
    float x_transform_scale = 0;
    float width_arrow_scale = 0.4;
    float height_arrow_scale = 0.6;
    int count = 0;
    int loop = 1;
};
}    // namespace ui_common

#endif
