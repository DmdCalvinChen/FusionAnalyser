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

#include "label.h"

#include <GL/glew.h>
#include <wrap/qt/gl_label.h>

namespace ui_common
{
Label::Label(const QString& _desrction, const QString& _helpDesrction,
             TipShowDirection _tipShowDirection, const ControlTypeId& _typeControl)
    : CustomButtonDesignBase("", _desrction, _helpDesrction, _tipShowDirection, _typeControl)
{
}

Label::~Label()
{
}

void Label::setVisible(bool _bIsVisible)
{
    this->baseAttributeInfomation.setBaseAttributeIsVisible(_bIsVisible);
}

void Label::setAttributeInfomation(const BaseAttributeInfoButton& _baseAttributeInfomation)
{
    this->baseAttributeInfomation = _baseAttributeInfomation;
}

void Label::mousePressEvent(QPoint posCursor)
{
}

void Label::draw(QWidget* _parent, QPainter* painter, QPoint posCursor,
                 const Matrix44m& _matrixTransform, bool& _bIsPickedThisControl)
{
    if (!baseAttributeInfomation.getBaseAttributeIsVisible() || _parent == nullptr)
    {
        return;
    }

    this->parent = _parent;
    matrixTransform = _matrixTransform;

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // 显示标签
    QString infomationTip = getDesrctionTip();
    auto tipLabelFontMode = baseAttributeInfomation.getBaseAttributeTipLabelFontModel();
    auto length = baseAttributeInfomation.getBaseAttributeRadius();
    auto v = matrixTransform * Point3m(length + length, length * 0.3, 0.0f);
    vcg::glLabel::render(painter, v, infomationTip, tipLabelFontMode);

    glPopAttrib();
}

void Label::mouseMoveCustomEvent(QWidget* parent, QPainter* painter, QPoint posCursor)
{
}

bool Label::cursorIsStayOnThisButton(QPoint cursorPos, Point3m& transformCursor)
{
    return false;
}

}    // namespace ui_common
