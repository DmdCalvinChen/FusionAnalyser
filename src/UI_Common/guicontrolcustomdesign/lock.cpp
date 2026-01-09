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

#include "lock.h"

#include <GL/glew.h>
#include <wrap/qt/gl_label.h>

using namespace vcg;

namespace ui_common
{
Lock::Lock(const QString& _desrction, const QString& _helpDesrction, bool _bIsLockedStatus,
           TipShowDirection _tipShowDirection, const ControlTypeId& _typeControl)
    : CustomButtonDesignBase("", _desrction, _helpDesrction, _tipShowDirection, _typeControl)
{
    textureMapLocked = new TextureMap(":/default/res/default/images/lock/locked.png");
    textureMapUnLock = new TextureMap(":/default/res/default/images/lock/unlocked.png");
    this->baseAttributeInfomation.setBaseAttributePlayStatus(_bIsLockedStatus);
}

Lock::~Lock()
{
    if (textureMapLocked)
    {
        delete textureMapLocked;
        textureMapLocked = nullptr;
    }

    if (textureMapUnLock)
    {
        delete textureMapUnLock;
        textureMapUnLock = nullptr;
    }
}

bool Lock::getLockedStatus() const
{
    return baseAttributeInfomation.getBaseAttributePlayStatus();
}

void Lock::setLockedStatus(bool _bLocked)
{
    baseAttributeInfomation.setBaseAttributePlayStatus(_bLocked);
}

void Lock::setAttributeInfomation(const BaseAttributeInfoButton& _baseAttributeInfomation)
{
    this->baseAttributeInfomation = _baseAttributeInfomation;
}

void Lock::mousePressEvent(QPoint posCursor)
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

void Lock::draw(QWidget* _parent, QPainter* painter, QPoint posCursor,
                const Matrix44m& _matrixTransform, bool& _bIsPickedThisControl)
{
    if (!baseAttributeInfomation.getBaseAttributeIsVisible())
    {
        return;
    }

    this->parent = _parent;
    matrixTransform = _matrixTransform;

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

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    // 绘制锁 绘制两次防止出现阴影
    auto length = baseAttributeInfomation.getBaseAttributeRadius();
    length *= 0.7f;

    float widthBorder = length * 0.2f;
    auto backgroundColor = baseAttributeInfomation.getBaseAttributeBackgroundColor();

    if (bCursorStayOnThisButton)
    {
        backgroundColor = baseAttributeInfomation.getBaseAttributeMouseHoverStatusColor();
    }

    glColor4f(backgroundColor.X(), backgroundColor.Y(), backgroundColor.Z(), backgroundColor.W());

#define USE_CUSTOM_DESIGN_LOCK_STYLE false

#if !USE_CUSTOM_DESIGN_LOCK_STYLE
    {
        auto length = baseAttributeInfomation.getBaseAttributeRadius();
        float screenRatio = float(parent->width()) / float(parent->height());
        Point3m leftTop(-length, length, 0);
        Point3m leftBottom(-length, -length, 0);
        Point3m rightTop(length, length, 0);
        Point3m rightBottom(length, -length, 0);

        leftTop = matrixTransform * leftTop;
        leftBottom = matrixTransform * leftBottom;
        rightTop = matrixTransform * rightTop;
        rightBottom = matrixTransform * rightBottom;

        if (baseAttributeInfomation.getBaseAttributePlayStatus())
        {
            textureMapLocked->setUVCoordinateMap(leftTop, leftBottom, rightTop, rightBottom);
            textureMapLocked->draw();
            // textureMapLocked->drawBackground(screenRatio);
        }
        else
        {
            textureMapUnLock->setUVCoordinateMap(leftTop, leftBottom, rightTop, rightBottom);
            textureMapUnLock->draw();
            // textureMapUnLock->drawBackground(screenRatio);
        }
    }

#else
    std::vector<Point3m> vecLockPolygonCenter;
    vecLockPolygonCenter.push_back(Point3m(-length + widthBorder, length, 0));
    vecLockPolygonCenter.push_back(Point3m(-length, length - widthBorder, 0));
    vecLockPolygonCenter.push_back(Point3m(-length, -length + widthBorder, 0));
    vecLockPolygonCenter.push_back(Point3m(-length + widthBorder, -length, 0));
    vecLockPolygonCenter.push_back(Point3m(length - widthBorder, -length, 0));
    vecLockPolygonCenter.push_back(Point3m(length, -length + widthBorder, 0));
    vecLockPolygonCenter.push_back(Point3m(length, length - widthBorder, 0));
    vecLockPolygonCenter.push_back(Point3m(length - widthBorder, length, 0));

    float widthLineTop = length * 0.5f;
    std::vector<Point3m> vecLockLineTop;

    if (baseAttributeInfomation.getBaseAttributePlayStatus())
    {
        vecLockLineTop.push_back(Point3m(-length / 2.0f, length, 0));
        vecLockLineTop.push_back(Point3m(-length / 2.0f, length + length, 0));
        vecLockLineTop.push_back(Point3m(length / 2.0f, length + length, 0));
        vecLockLineTop.push_back(Point3m(length / 2.0f, length + length / 5.0f, 0));
    }
    else
    {
        vecLockLineTop.push_back(Point3m(-length / 2.0f, length, 0));
        vecLockLineTop.push_back(Point3m(-length / 2.0f, length + length * 1.2, 0));
        vecLockLineTop.push_back(Point3m(-length * 1.5f, length + length * 1.2, 0));
        vecLockLineTop.push_back(Point3m(-length * 1.5f, length + length / 3.0f, 0));
    }

    std::vector<Point3m> vecLockPointsCenter;
    float dx = length * 0.01f;
    float dy = dx;
    float beginX = 0.0f;
    float beginY = -length * 0.1f;

    for (int i = 0; i < 40; ++i)
    {
        for (int j = i; j >= 0; --j)
        {
            vecLockPointsCenter.push_back(Point3m(i * dx, i * dy + beginY, 0));
            vecLockPointsCenter.push_back(Point3m(-i * dx, i * dy + beginY, 0));
        }
    }

    glBegin(GL_POLYGON);
    for (auto v : vecLockPolygonCenter)
    {
        v = matrixTransform * v;
        glVertex2f(v.X(), v.Y());
    }
    glEnd();

    glBegin(GL_POLYGON);
    for (auto v : vecLockPolygonCenter)
    {
        v = matrixTransform * v;
        glVertex2f(v.X(), v.Y());
    }
    glEnd();

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glLineWidth(3.0f);
    glBegin(GL_LINE_STRIP);
    for (auto v : vecLockLineTop)
    {
        v = matrixTransform * v;
        glVertex2f(v.X(), v.Y());
    }
    glEnd();

    // 绘制中心点
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glBegin(GL_POINTS);
    for (auto v : vecLockPointsCenter)
    {
        v = matrixTransform * v;
        glVertex2f(v.X(), v.Y());
    }
    glEnd();

#endif

#undef USE_CUSTOM_DESIGN_LOCK_STYLE
}

void Lock::mouseMoveCustomEvent(QWidget* parent, QPainter* painter, QPoint posCursor)
{
    Point3m transformCursor;

    bCursorStayOnThisButton = cursorIsStayOnThisButton(posCursor, transformCursor);

    if (!bCursorStayOnThisButton)
    {
        return;
    }

    auto length = baseAttributeInfomation.getBaseAttributeRadius();
    QString infomationTip = getHelpDesrctionTip();

    if (baseAttributeInfomation.getBaseAttributePlayStatus())
    {
        infomationTip = QString(tr("locked"));
    }
    else
    {
        infomationTip = QString(tr("unlock"));
    }

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
            float centerStartX = transformCursor.X() + length * 2.0;
            float centerStartY = transformCursor.Y() + length * 0.3;
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

bool Lock::cursorIsStayOnThisButton(QPoint cursorPos, Point3m& transformCursor)
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

void Lock::setCurrentPlayStatus(bool _bIsPlaying /* = true */)
{
    this->baseAttributeInfomation.setBaseAttributePlayStatus(_bIsPlaying);
}

void Lock::setVisible(bool _bIsVisible)
{
    baseAttributeInfomation.setBaseAttributeIsVisible(_bIsVisible);
}

void Lock::setChecked(bool _bIsChecked)
{
    // todo something when the button is pressed.
    this->baseAttributeInfomation.setBaseAttributePlayStatus(_bIsChecked);
    emit clicked();
    emit stateChanged(this->baseAttributeInfomation.getBaseAttributePlayStatus());
}

}    // namespace ui_common
