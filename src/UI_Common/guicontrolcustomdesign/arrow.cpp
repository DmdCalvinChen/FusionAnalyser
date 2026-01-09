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

#include "arrow.h"

#include <GL/glew.h>
#include <QDebug>

using namespace vcg;
namespace ui_common
{
Arrow::Arrow(const QString& _desrction, const QString& _helpDesrction, bool _bIsUpperDirection,
             TipShowDirection _tipShowDirection, const ControlTypeId& _typeControl)
    : CustomButtonDesignBase("", _desrction, _helpDesrction, _tipShowDirection, _typeControl)
{
    this->bIsUpperDirection = _bIsUpperDirection;
}

Arrow::~Arrow()
{
}

void Arrow::setAttributeInfomation(const BaseAttributeInfoButton& _baseAttributeInfomation)
{
    this->baseAttributeInfomation = _baseAttributeInfomation;
    this->baseAttributeInfomation.setBaseAttributeRadius(
            _baseAttributeInfomation.getBaseAttributeRadius() * 0.5f);
}

void Arrow::mousePressEvent(QPoint posCursor)
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

void Arrow::draw(QWidget* _parent, QPainter* painter, QPoint posCursor,
                 const Matrix44m& _matrixTransform, bool& _bIsPickedThisControl)
{
    if (!baseAttributeInfomation.getBaseAttributeIsVisible())
    {
        return;
    }

    this->parent = _parent;
    matrixTransform = _matrixTransform;

    Matrix44m rotateMat;
    rotateMat.SetIdentity();

    if (this->bIsUpperDirection)
    {
        rotateMat = matrixTransform;
    }
    else
    {
        rotateMat = matrixTransform * rotateMat.SetRotateRad(PI, Point3m(0, 0, 1.0f));
    }

    matrixTransform = rotateMat;

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    // 绘制三角形代替箭头 绘制两次防止出现阴影
    auto length = baseAttributeInfomation.getBaseAttributeRadius();

    float widthBorderRhombus = length * 0.15f;
    auto backgroundColor = baseAttributeInfomation.getBaseAttributeBackgroundColor();

    if (bCursorStayOnThisButton)
    {
        backgroundColor = baseAttributeInfomation.getBaseAttributeMouseHoverStatusColor();
    }

    glColor4f(backgroundColor.X(), backgroundColor.Y(), backgroundColor.Z(), backgroundColor.W());

    glBegin(GL_TRIANGLES);
    auto v = rotateMat * Point3m(-length, 0.0, 0.0f);
    glVertex2f(v.X(), v.Y());
    v = rotateMat * Point3m(0, length, 0.0f);
    glVertex2f(v.X(), v.Y());
    v = rotateMat * Point3m(length, 0, 0.0f);
    glVertex2f(v.X(), v.Y());
    glEnd();

    glBegin(GL_TRIANGLES);
    v = rotateMat * Point3m(-length, 0.0, 0.0f);
    glVertex2f(v.X(), v.Y());
    v = rotateMat * Point3m(0, length, 0.0f);
    glVertex2f(v.X(), v.Y());
    v = rotateMat * Point3m(length, 0, 0.0f);
    glVertex2f(v.X(), v.Y());
    glEnd();
    glPopAttrib();

    // 显示提示信息
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    mouseMoveCustomEvent(parent, painter, posCursor);
    glDisable(GL_BLEND);
    glDisable(GL_POLYGON_SMOOTH);
    glPopAttrib();
}

void Arrow::mouseMoveCustomEvent(QWidget* parent, QPainter* painter, QPoint posCursor)
{
    Point3m transformCursor;

    bCursorStayOnThisButton = cursorIsStayOnThisButton(posCursor, transformCursor);

    if (!bCursorStayOnThisButton)
    {
        return;
    }

    auto length = baseAttributeInfomation.getBaseAttributeRadius();
    QString infomationTip = getHelpDesrctionTip();
    TipShowDirection tipShowDirection = getTipShowDirection();

    if (infomationTip.isEmpty())
    {
        return;
    }

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
            float centerStartX = transformCursor.X() + length * 1.0;
            float centerStartY = transformCursor.Y() + length * 0.5;
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

bool Arrow::cursorIsStayOnThisButton(QPoint cursorPos, Point3m& transformCursor)
{
    if (parent == nullptr)
        return false;
    auto v = matrixTransform * Point3m(0.0f, 0.0f, 0.0f);

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

void Arrow::setCurrentPlayStatus(bool _bIsPlaying /* = true */)
{
    this->baseAttributeInfomation.setBaseAttributePlayStatus(_bIsPlaying);
}

void Arrow::setVisible(bool _bIsVisible)
{
    baseAttributeInfomation.setBaseAttributeIsVisible(_bIsVisible);
}

void Arrow::setChecked(bool _bIsChecked)
{
    // todo something when the button is pressed.
    this->baseAttributeInfomation.setBaseAttributePlayStatus(_bIsChecked);
    emit clicked();
    emit stateChanged(this->baseAttributeInfomation.getBaseAttributePlayStatus());
}

}    // namespace ui_common
