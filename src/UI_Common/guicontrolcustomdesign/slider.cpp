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

#include "slider.h"

namespace ui_common
{
void Slider::setVisible(bool _bIsVisible)
{
    bFirstUseDefaultValue = true;
    this->baseAttributeInfomation.setBaseAttributeIsVisible(_bIsVisible);
}

void Slider::setValue(const int& _value)
{
    baseAttributeInfomation.setDefultValueOfSlider(_value);
}

void Slider::setAttributeInfomation(const BaseAttributeInfoButton& _baseAttributeInfomation)
{
    this->baseAttributeInfomation = *(SliderAttributionInfo*)&_baseAttributeInfomation;

    bool bIsFloatValueType =
            baseAttributeInfomation.getMaxValueOfSliderRange() -
                                    baseAttributeInfomation.getMinValueOfSliderRange() >
                            1.0f
                    ? false
                    : true;

    if (bIsFloatValueType)
    {
        valueType = FLOAT_VALUE_TYPE;
    }
    else
    {
        valueType = INT_VALUE_TTYPE;
    }
}

void Slider::mousePressEvent(QPoint posCursor)
{
    if (!baseAttributeInfomation.getBaseAttributeIsVisible())
    {
        return;
    }

    Point3m transformCursorPos(0, 0, 0);
    bool bCursorStayOnThisButton = cursorIsStayOnThisButton(posCursor, transformCursorPos);

    bTouchedThisAnchor = bCursorStayOnThisButton;

    if (!bCursorStayOnThisButton)
    {
        return;    // the press dont stay on this button
    }

    bFirstUseDefaultValue = false;

    // todo something when the button is pressed.
    this->baseAttributeInfomation.setBaseAttributePlayStatus(
            !this->baseAttributeInfomation.getBaseAttributePlayStatus());
    emit clicked();
    emit pressedStatus(this->baseAttributeInfomation.getBaseAttributePlayStatus());
}

void Slider::mouseReleaseEvent(QPoint posCursor)
{
    if (!baseAttributeInfomation.getBaseAttributeIsVisible())
    {
        return;
    }

    bTouchedThisAnchor = false;
}

void Slider::draw(QWidget* _parent, QPainter* painter, QPoint posCursor,
                  const Matrix44m& _matrixTransform, bool& _bTouchedThisControl)
{
    if (!baseAttributeInfomation.getBaseAttributeIsVisible() || _parent == nullptr)
    {
        return;
    }

    this->parent = _parent;
    matrixTransform = _matrixTransform;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    // 绘制左右半圆 需要绘制两次来删除混合带来的黑影
    auto backgroundColor = baseAttributeInfomation.getBaseAttributeBackgroundColor();
    glColor4f(backgroundColor.X(), backgroundColor.Y(), backgroundColor.Z(), backgroundColor.W());
    auto length = baseAttributeInfomation.getBaseAttributeRadius();
    auto widthSlider =
            ((ui_common::SliderAttributionInfo*)&baseAttributeInfomation)->getSliderWidth();

    Matrix44f rotateMatrix = Matrix44f::Identity();
    rotateMatrix.SetRotateDeg(90.0f, Point3m(0, 0, 1.0));
    Matrix44f translateMatrix = Matrix44f::Identity();
    Matrix44f changMatrix = matrixTransform * translateMatrix * rotateMatrix;

    // left half circle
    glBegin(GL_POLYGON);
    for (int i = 0; i <= numDiv; i++)
    {
        auto v = changMatrix *
                 Point3m(length * cos(PI / numDiv * i), length * sin(PI / numDiv * i), 0.0f);
        glVertex2f(v.X(), v.Y());
    }
    glEnd();
    glBegin(GL_POLYGON);
    for (int i = 0; i <= numDiv; i++)
    {
        auto v = changMatrix *
                 Point3m(length * cos(PI / numDiv * i), length * sin(PI / numDiv * i), 0.0f);
        glVertex2f(v.X(), v.Y());
    }
    glEnd();

    rotateMatrix.SetIdentity();
    rotateMatrix.SetRotateDeg(-90.0f, Point3m(0, 0, 1.0));
    translateMatrix.SetIdentity();
    translateMatrix.SetTranslate(widthSlider, 0, 0);
    changMatrix = matrixTransform * translateMatrix * rotateMatrix;

    // right half circle
    glBegin(GL_POLYGON);
    for (int i = 0; i <= numDiv; i++)
    {
        auto v = changMatrix *
                 Point3m(length * cos(PI / numDiv * i), length * sin(PI / numDiv * i), 0.0f);
        glVertex2f(v.X(), v.Y());
    }
    glEnd();
    glBegin(GL_POLYGON);
    for (int i = 0; i <= numDiv; i++)
    {
        auto v = changMatrix *
                 Point3m(length * cos(PI / numDiv * i), length * sin(PI / numDiv * i), 0.0f);
        glVertex2f(v.X(), v.Y());
    }
    glEnd();

    // draw rectangle
    glBegin(GL_QUADS);

    auto v = matrixTransform * Point3m(0.0f, length, 0.0f);
    glVertex2f(v.X(), v.Y());
    v = matrixTransform * Point3m(0.0f, -length, 0.0f);
    glVertex2f(v.X(), v.Y());
    v = matrixTransform * Point3m(widthSlider, -length, 0.0f);
    glVertex2f(v.X(), v.Y());
    v = matrixTransform * Point3m(widthSlider, length, 0.0f);
    glVertex2f(v.X(), v.Y());

    v = matrixTransform * Point3m(0.0f, length, 0.0f);
    glVertex2f(v.X(), v.Y());
    v = matrixTransform * Point3m(0.0f, -length, 0.0f);
    glVertex2f(v.X(), v.Y());
    v = matrixTransform * Point3m(widthSlider, -length, 0.0f);
    glVertex2f(v.X(), v.Y());
    v = matrixTransform * Point3m(widthSlider, length, 0.0f);
    glVertex2f(v.X(), v.Y());

    glEnd();

    // draw circle center of slider anchor
    float addRadius = length + length * 0.6f;
    Matrix44m matAnchor;
    if (bFirstUseDefaultValue)
    {
        auto minValue = baseAttributeInfomation.getMinValueOfSliderRange();
        auto maxValue = baseAttributeInfomation.getMaxValueOfSliderRange();
        auto defaultValue = baseAttributeInfomation.getDefaultValueOfSlider();
        float percentSlider = (defaultValue - minValue) / (maxValue - minValue);
        Matrix44f translateMatrix = Matrix44f::Identity();
        translateMatrix.SetTranslate(widthSlider * percentSlider, 0.0, 0.0);
        matAnchor = matrixTransform * translateMatrix;
    }
    else
    {
        if (matrixCurrentAnchor != Matrix44m::Identity())
        {
            matAnchor = matrixCurrentAnchor;
        }
    }

    glBegin(GL_POLYGON);
    for (int i = 0; i <= numDiv; i++)
    {
        auto v = matAnchor * Point3m(addRadius * cos(2 * PI / numDiv * i),
                                     addRadius * sin(2 * PI / numDiv * i), 0.0f);
        glVertex2f(v.X(), v.Y());
    }
    glEnd();

    glBegin(GL_POLYGON);
    for (int i = 0; i <= numDiv; i++)
    {
        auto v = matAnchor * Point3m(addRadius * cos(2 * PI / numDiv * i),
                                     addRadius * sin(2 * PI / numDiv * i), 0.0f);
        glVertex2f(v.X(), v.Y());
    }
    glEnd();

    auto anchorColor = baseAttributeInfomation.getColorAnchor();
    glColor4f(anchorColor.X(), anchorColor.Y(), anchorColor.Z(), anchorColor.W());
    glBegin(GL_POLYGON);
    for (int i = 0; i <= numDiv; i++)
    {
        auto v = matAnchor * Point3m(length * cos(2 * PI / numDiv * i),
                                     length * sin(2 * PI / numDiv * i), 0.0f);
        glVertex2f(v.X(), v.Y());
    }
    glEnd();

    glBegin(GL_POLYGON);
    for (int i = 0; i <= numDiv; i++)
    {
        auto v = matAnchor * Point3m(length * cos(2 * PI / numDiv * i),
                                     length * sin(2 * PI / numDiv * i), 0.0f);
        glVertex2f(v.X(), v.Y());
    }
    glEnd();

    // 显示提示信息
    mouseMoveCustomEvent(parent, painter, posCursor);

    _bTouchedThisControl = bCursorStayOnThisButton;

    glPopAttrib();
    glDisable(GL_BLEND);
    glDisable(GL_POLYGON_SMOOTH);
}

void Slider::mouseMoveCustomEvent(QWidget* parent, QPainter* painter, QPoint posCursor)
{
    if (!baseAttributeInfomation.getBaseAttributeIsVisible() || parent == nullptr)
    {
        return;
    }

    Point3m transformCursor;

    bCursorStayOnThisButton = cursorIsStayOnThisButton(posCursor, transformCursor);

    if (!bCursorStayOnThisButton)
    {
        return;
    }

    if (bTouchedThisAnchor)
    {
        matrixCurrentAnchor[0][3] = transformCursor.X();
        matrixCurrentAnchor[1][3] = matrixTransform[1][3];
    }

    auto length = baseAttributeInfomation.getBaseAttributeRadius();

    QString infomationTip = getDesrctionTip();

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
            float centerStartX = transformCursor.X();
            float centerStartY = transformCursor.Y() - length * 2.5;
            float heightRectangleTip =
                    tipLabelFontMode.qFont.pixelSize() * 1.0f / parent->height() * heightTipRadio;
            float widthRectangleTip = infomationTip.length() * heightRectangleTip * widthTipRadio;

            float leftBlankX = length * 0.12f;
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

bool Slider::cursorIsStayOnThisButton(QPoint cursorPos, Point3m& transformCursor)
{
    if (parent == nullptr)
        return false;
    auto height = baseAttributeInfomation.getBaseAttributeRadius();
    auto width = baseAttributeInfomation.getSliderWidth();

    Point3m leftTop(0.0f, height, 0.0f);
    Point3m leftBottom(0.0, -height, 0.0f);
    Point3m rightBottom(width, -height, 0.0f);
    Point3m rightTop(width, height, 0.0f);

    auto v = matrixTransform * Point3m(0.0f, 0.0f, 0.0f);

    transformCursor = Point3m(cursorPos.x() * 2.0f / parent->width() - 1,
                              -2.0f * cursorPos.y() / parent->height() + 1.0f, 0.0f);

    float screenRatio = float(parent->width()) / float(parent->height());

    transformCursor.X() *= screenRatio;

    Point3m leftTopPointTransform = matrixTransform * leftTop;
    Point3m rightBottomPointTransform = matrixTransform * rightBottom;

    if (transformCursor.X() >= (leftTopPointTransform).X() &&
        transformCursor.X() <= (rightBottomPointTransform).X() &&
        transformCursor.Y() >= (rightBottomPointTransform).Y() &&
        transformCursor.Y() <= (leftTopPointTransform).Y())
    {
        float valuePercent = (transformCursor.X() - leftTopPointTransform.X()) /
                             (rightBottomPointTransform.X() - leftTopPointTransform.X());

        if (bTouchedThisAnchor)
        {
            if (valueType == FLOAT_VALUE_TYPE)
            {
                float minValue = baseAttributeInfomation.getMinValueOfSliderRange();
                float maxValue = baseAttributeInfomation.getMaxValueOfSliderRange();
                emit valueCurrentSliderFloatValueRange(minValue +
                                                       valuePercent * (maxValue - minValue));
            }
            else
            {
                int minValue = baseAttributeInfomation.getMinValueOfSliderRange();
                int maxValue = baseAttributeInfomation.getMaxValueOfSliderRange();
                emit valueCurrentSliderIntValueRange(minValue +
                                                     valuePercent * (maxValue - minValue));
            }
        }

        return true;
    }

    return false;
}

void Slider::setCurrentPlayStatus(bool _bIsPlaying /* = true */)
{
    this->baseAttributeInfomation.setBaseAttributePlayStatus(_bIsPlaying);
}

}    // namespace ui_common
