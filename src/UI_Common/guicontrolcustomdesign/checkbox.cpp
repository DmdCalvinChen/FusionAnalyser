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

#include "checkbox.h"

#include <GL/glew.h>
#include <wrap/qt/gl_label.h>

namespace ui_common

{
CheckBox::CheckBox(const QString& _desrction, const QString& _helpDesrction,
                   TipShowDirection _tipShowDirection, const ControlTypeId& _typeControl)
    : CustomButtonDesignBase("", _desrction, _helpDesrction, _tipShowDirection, _typeControl)
{
}

CheckBox::~CheckBox()
{
}

void CheckBox::setAttributeInfomation(const BaseAttributeInfoButton& _baseAttributeInfomation)
{
    this->baseAttributeInfomation = _baseAttributeInfomation;
}

void CheckBox::mousePressEvent(QPoint posCursor)
{
    if (!baseAttributeInfomation.getBaseAttributeIsVisible())
    {
        return;
    }

    Point3m transformCursorPos(0, 0, 0);
    bool bCursorStayOnThisButton = cursorIsStayOnThisButton(posCursor, transformCursorPos);

    if (!bCursorStayOnThisButton)
    {
        return;    // the press dont stay on this button
    }

    // todo something when the button is pressed.
    this->baseAttributeInfomation.setBaseAttributePlayStatus(
            !this->baseAttributeInfomation.getBaseAttributePlayStatus());
    emit clicked();
    emit stateChanged(this->baseAttributeInfomation.getBaseAttributePlayStatus());
}

void CheckBox::draw(QWidget* _parent, QPainter* painter, QPoint posCursor,
                    const Matrix44m& _matrixTransform, bool& _bIsPickedThisControl)
{
    if (!baseAttributeInfomation.getBaseAttributeIsVisible())
    {
        return;
    }

    this->parent = _parent;
    matrixTransform = _matrixTransform;

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // 显示标签
    Label label(getDesrctionTip(), getHelpDesrctionTip(), DOWN_TIP_SHOW_DIRECTION, LABEL);
    label.setAttributeInfomation(baseAttributeInfomation);
    bool tempBoolen;
    label.draw(_parent, painter, posCursor, _matrixTransform, tempBoolen);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    // 绘制单选框 需要绘制两次来删除混合带来的黑影
    auto backgroundColor = baseAttributeInfomation.getBaseAttributeBackgroundColor();
    glColor4f(backgroundColor.X(), backgroundColor.Y(), backgroundColor.Z(), backgroundColor.W());
    auto length = baseAttributeInfomation.getBaseAttributeRadius();
    glBegin(GL_QUADS);
    auto v = matrixTransform * Point3m(-length, length, 0.0f);
    glVertex2f(v.X(), v.Y());
    v = matrixTransform * Point3m(-length, -length, 0.0f);
    glVertex2f(v.X(), v.Y());
    v = matrixTransform * Point3m(length, -length, 0.0f);
    glVertex2f(v.X(), v.Y());
    v = matrixTransform * Point3m(length, length, 0.0f);
    glVertex2f(v.X(), v.Y());
    glEnd();

    glBegin(GL_QUADS);
    v = matrixTransform * Point3m(-length, length, 0.0f);
    glVertex2f(v.X(), v.Y());
    v = matrixTransform * Point3m(-length, -length, 0.0f);
    glVertex2f(v.X(), v.Y());
    v = matrixTransform * Point3m(length, -length, 0.0f);
    glVertex2f(v.X(), v.Y());
    v = matrixTransform * Point3m(length, length, 0.0f);
    glVertex2f(v.X(), v.Y());
    glEnd();

    // 绘制对号√
    if (this->baseAttributeInfomation.getBaseAttributePlayStatus())
    {
        auto foregroundColor = baseAttributeInfomation.getBaseAttributeForegroundColor();
        glColor4f(foregroundColor.X(), foregroundColor.Y(), foregroundColor.Z(),
                  foregroundColor.W());
        glLineWidth(2.0f);
        glBegin(GL_LINE_STRIP);
        auto v = matrixTransform * Point3m(-length + length * 0.2f, 0.0f, 0.0f);
        glVertex2f(v.X(), v.Y());
        v = matrixTransform * Point3m(-length * 0.3f, -length * 0.5f, 0.0f);
        glVertex2f(v.X(), v.Y());
        v = matrixTransform * Point3m(length - length * 0.2f, length * 0.6f, 0.0f);
        glVertex2f(v.X(), v.Y());
        glEnd();
        glLineWidth(1.0f);
    }

    // 显示提示信息
    mouseMoveCustomEvent(parent, painter, posCursor);

    glPopAttrib();
    glDisable(GL_BLEND);
    glDisable(GL_POLYGON_SMOOTH);
}

void CheckBox::mouseMoveCustomEvent(QWidget* parent, QPainter* painter, QPoint posCursor)
{
    Point3m transformCursor;

    bool bCursorStayOnThisButton = cursorIsStayOnThisButton(posCursor, transformCursor);

    if (!bCursorStayOnThisButton)
    {
        return;
    }

    // 绘制边界旋转

    if (count > loop)
    {
        count = 0;
    }

    int iStartVertex = (count + 0) % 2;
    auto length = baseAttributeInfomation.getBaseAttributeRadius();
    float lengthTip = length + length * 0.15f;
    auto colorBorderMarquee = baseAttributeInfomation.getBaseAttributeColorBorderMarquee();
    glColor4f(colorBorderMarquee.X(), colorBorderMarquee.Y(), colorBorderMarquee.Z(),
              colorBorderMarquee.W());

    glBegin(GL_LINES);
    for (iStartVertex; iStartVertex <= numDiv; ++iStartVertex)
    {
        auto v = matrixTransform *
                 Point3m(2.0 * lengthTip / numDiv * iStartVertex - lengthTip, lengthTip, 0.0f);
        glVertex2f(v.X(), v.Y());
    }
    glEnd();

    iStartVertex = (count + 1) % 2;
    glBegin(GL_LINES);
    for (iStartVertex; iStartVertex <= numDiv; ++iStartVertex)
    {
        auto v = matrixTransform *
                 Point3m(lengthTip, -2.0 * lengthTip / numDiv * iStartVertex + lengthTip, 0.0f);
        glVertex2f(v.X(), v.Y());
    }
    glEnd();

    iStartVertex = (count + 2) % 2;
    glBegin(GL_LINES);
    for (iStartVertex; iStartVertex <= numDiv; ++iStartVertex)
    {
        auto v = matrixTransform *
                 Point3m(-2.0 * lengthTip / numDiv * iStartVertex + lengthTip, -lengthTip, 0.0f);
        glVertex2f(v.X(), v.Y());
    }
    glEnd();

    iStartVertex = (count + 3) % 2;
    glBegin(GL_LINES);
    for (iStartVertex; iStartVertex <= numDiv; ++iStartVertex)
    {
        auto v = matrixTransform *
                 Point3m(-lengthTip, 2.0 * lengthTip / numDiv * iStartVertex - lengthTip, 0.0f);
        glVertex2f(v.X(), v.Y());
    }
    glEnd();

    ++count;

    QString infomationTip = getDesrctionTip();

    if (infomationTip.isEmpty())
    {
        return;
    }

    TipShowDirection tipShowDirection = getTipShowDirection();

    // 绘制矩形提示框
    auto heightTip = baseAttributeInfomation.getBaseAttributionHeightTip();
    auto tipBackgroundColor = baseAttributeInfomation.getBaseAttributeTipBackgroundColor();
    auto tipLabelFontMode = baseAttributeInfomation.getBaseAttributeTipLabelFontModel();
    auto heightTipRadio = baseAttributeInfomation.getBaseAttributeHeightTipRadio();
    auto widthTipRadio = baseAttributeInfomation.getBaseAttributeWidthTipRadio();
    switch (tipShowDirection)
    {
        case UPPER_TIP_SHOW_DIRECTION:
        {
        }
        break;
        case DOWN_TIP_SHOW_DIRECTION:
        {
            float centerStartX = transformCursor.X();
            float centerStartY = transformCursor.Y() - length * 0.5f;
            float heightRectangleTip =
                    tipLabelFontMode.qFont.pixelSize() * 1.0f / parent->height() * heightTipRadio;
            float widthRectangleTip = infomationTip.length() * heightRectangleTip * widthTipRadio;

            float leftBlankX = length * 0.1f;
            float leftBlankY = heightRectangleTip / 3.0f;

            glColor4f(tipBackgroundColor.X(), tipBackgroundColor.Y(), tipBackgroundColor.Z(),
                      tipBackgroundColor.W());

            // 绘制四分之一圆 绘制四角之四分之一圆 绘制矩形框

            {
                float radiusCornerCircle = heightRectangleTip * 0.25f;
                int numDiv = 20;
                std::vector<Point3m> vecCorner;
                vecCorner.push_back(Point3m(0.0, 0.0, 0.0));
                float addAngle = PI / 2.0f / numDiv;
                for (int i = 0; i <= numDiv; ++i)
                {
                    vecCorner.push_back(Point3m(radiusCornerCircle * cos(addAngle * i),
                                                radiusCornerCircle * sin(addAngle * i), 0.0f));
                }

                // 绘制左上角的弧度角
                Matrix44f matLeftUpCorner;
                matLeftUpCorner.SetIdentity();
                matLeftUpCorner.SetRotateDeg(90.0f, Point3m(0.0f, 0.0f, 1.0f));
                matLeftUpCorner[0][3] = centerStartX;
                matLeftUpCorner[1][3] = centerStartY;

                glBegin(GL_TRIANGLE_FAN);
                for (auto v : vecCorner)
                {
                    auto vTransform = matLeftUpCorner * v;
                    glVertex2f(vTransform.X(), vTransform.Y());
                }
                glEnd();

                // 绘制左下角的弧度角
                Matrix44f matLeftDownCorner;
                matLeftDownCorner.SetIdentity();
                matLeftDownCorner.SetRotateDeg(180.0f, Point3m(0.0f, 0.0f, 1.0f));
                matLeftDownCorner[0][3] = centerStartX;
                matLeftDownCorner[1][3] = centerStartY - heightRectangleTip;
                glBegin(GL_TRIANGLE_FAN);
                for (auto v : vecCorner)
                {
                    auto vTransform = matLeftDownCorner * v;
                    glVertex2f(vTransform.X(), vTransform.Y());
                }
                glEnd();

                // 绘制右上角的弧度角
                Matrix44f matRightUpCorner;
                matRightUpCorner.SetIdentity();
                matRightUpCorner[0][3] = centerStartX + widthRectangleTip;
                matRightUpCorner[1][3] = centerStartY;
                glBegin(GL_TRIANGLE_FAN);
                for (auto v : vecCorner)
                {
                    auto vTransform = matRightUpCorner * v;
                    glVertex2f(vTransform.X(), vTransform.Y());
                }
                glEnd();

                // 绘制右下角的弧度角
                Matrix44f matRightDownCorner;
                matRightDownCorner.SetIdentity();
                matRightDownCorner.SetRotateDeg(-90.0f, Point3m(0.0f, 0.0f, 1.0f));
                matRightDownCorner[0][3] = centerStartX + widthRectangleTip;
                matRightDownCorner[1][3] = centerStartY - heightRectangleTip;
                glBegin(GL_TRIANGLE_FAN);
                for (auto v : vecCorner)
                {
                    auto vTransform = matRightDownCorner * v;
                    glVertex2f(vTransform.X(), vTransform.Y());
                }
                glEnd();

                // left triangle
                glBegin(GL_TRIANGLES);
                {
                    auto v = Point3m(centerStartX - radiusCornerCircle,
                                     centerStartY - heightRectangleTip / 4, 0.0f);
                    glVertex2f(v.X(), v.Y());
                    v = Point3m(centerStartX - radiusCornerCircle,
                                centerStartY - heightRectangleTip / 4 * 3, 0.0f);
                    glVertex2f(v.X(), v.Y());
                    v = Point3m(centerStartX - 2 * radiusCornerCircle,
                                centerStartY - heightRectangleTip / 2, 0.0f);
                    glVertex2f(v.X(), v.Y());
                }

                {
                    auto v = Point3m(centerStartX - radiusCornerCircle,
                                     centerStartY - heightRectangleTip / 4, 0.0f);
                    glVertex2f(v.X(), v.Y());
                    v = Point3m(centerStartX - radiusCornerCircle,
                                centerStartY - heightRectangleTip / 4 * 3, 0.0f);
                    glVertex2f(v.X(), v.Y());
                    v = Point3m(centerStartX - 2 * radiusCornerCircle,
                                centerStartY - heightRectangleTip / 2, 0.0f);
                    glVertex2f(v.X(), v.Y());
                }
                glEnd();

                glBegin(GL_QUADS);
                auto v = Point3m(centerStartX - radiusCornerCircle, centerStartY, 0.0f);
                glVertex2f(v.X(), v.Y());
                v = Point3m(centerStartX - radiusCornerCircle, centerStartY - heightRectangleTip,
                            0.0f);
                glVertex2f(v.X(), v.Y());
                v = Point3m(centerStartX + widthRectangleTip + radiusCornerCircle,
                            centerStartY - heightRectangleTip, 0.0f);
                glVertex2f(v.X(), v.Y());
                v = Point3m(centerStartX + widthRectangleTip + radiusCornerCircle, centerStartY,
                            0.0f);
                glVertex2f(v.X(), v.Y());

                v = Point3m(centerStartX, centerStartY + radiusCornerCircle, 0.0f);
                glVertex2f(v.X(), v.Y());
                v = Point3m(centerStartX, centerStartY - heightRectangleTip - radiusCornerCircle,
                            0.0f);
                glVertex2f(v.X(), v.Y());
                v = Point3m(centerStartX + widthRectangleTip,
                            centerStartY - heightRectangleTip - radiusCornerCircle, 0.0f);
                glVertex2f(v.X(), v.Y());
                v = Point3m(centerStartX + widthRectangleTip, centerStartY + radiusCornerCircle,
                            0.0f);
                glVertex2f(v.X(), v.Y());

                glEnd();

                glDisable(GL_BLEND);
                v = Point3m(centerStartX + leftBlankX, centerStartY - leftBlankY, 0.0f);
                vcg::glLabel::render(painter, v, infomationTip, tipLabelFontMode);
                glEnable(GL_BLEND);
            }
        }
        break;
        case LEFT_TIP_SHOW_DIRECTION:
        {
        }
        break;
        case RIGHT_TIP_SHOW_DIRECTION:
        {
        }
        break;
        default:
            break;
    }
}

bool CheckBox::cursorIsStayOnThisButton(QPoint cursorPos, Point3m& transformCursor)
{
    auto v = matrixTransform * Point3m(0.0f, 0.0f, 0.0f);
    if (parent == nullptr)
    {
        return false;
    }
    transformCursor = Point3m(cursorPos.x() * 2.0f / parent->width() - 1,
                              -2.0f * cursorPos.y() / parent->height() + 1.0f, 0.0f);

    float screenRatio = float(parent->width()) / float(parent->height());

    transformCursor.X() *= screenRatio;

    auto length = baseAttributeInfomation.getBaseAttributeRadius();

    if ((v - transformCursor).SquaredNorm() < length * length)
    {
        return true;
    }

    return false;
}

void CheckBox::setCurrentPlayStatus(bool _bIsPlaying /* = true */)
{
    this->baseAttributeInfomation.setBaseAttributePlayStatus(_bIsPlaying);
}

void CheckBox::setVisible(bool _bIsVisible)
{
    baseAttributeInfomation.setBaseAttributeIsVisible(_bIsVisible);
}

void CheckBox::setChecked(bool _bIsChecked)
{
    // todo something when the button is pressed.
    this->baseAttributeInfomation.setBaseAttributePlayStatus(_bIsChecked);
    emit clicked();
    emit stateChanged(this->baseAttributeInfomation.getBaseAttributePlayStatus());
}

}    // namespace ui_common
