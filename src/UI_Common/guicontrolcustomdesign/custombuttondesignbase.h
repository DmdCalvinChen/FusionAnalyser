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

#ifndef CUSTOM_BUTTON_DESIGN_BASE_H
#define CUSTOM_BUTTON_DESIGN_BASE_H

#include <QApplication>
#include <QCursor>
#include <QObject>
#include <QPoint>
#include <QScreen>
#include <QString>

#include <vcg/math/matrix44.h>

#include "../uicommon_global.h"
#include "GL/glew.h"
#include "vcg/space/point3.h"
#include "wrap/qt/gl_label.h"

#include <../common/ml_mesh_type.h>
/*
 *some button custom base class designed. designed on 15/4/2019 by liuchuang.
 */

using namespace vcg;

namespace ui_common
{

#define PI 3.1415926535897932384626433832795

enum TipShowDirection
{
    UPPER_TIP_SHOW_DIRECTION,
    DOWN_TIP_SHOW_DIRECTION,
    LEFT_TIP_SHOW_DIRECTION,
    RIGHT_TIP_SHOW_DIRECTION
};

enum ControlTypeId
{
    DEFAULT_CONTROL,
    PREVIOUS_PLAY_BUTTON,
    PLAY_PASUE_PLAY_BUTTON,
    NEXT_PLAY_BUTTON,
    LABEL,
    UPPER_JAW_CHECK_BOX,
    UPPER_JAW_SLIDER,
    LOWER_JAW_CHECK_BOX,
    LOWER_JAW_SLIDER,
    WAITING_BUTTON,
    RHOMBUS,
    RECTANGLE,
    ARROW,
    LOCK,
    BILLBOARD,
    COLLECT_FOLDED
    // ... ADD other button enum here please...
};

class BaseAttributeInfoButton;

class UI_COMMONSHARED_EXPORT CustomButtonDesignBase
{
public:
    CustomButtonDesignBase()
    {
    }
    CustomButtonDesignBase(const QString& _idNameLabel, const QString& _descriptionTip,
                           const QString& _helpDescriptionTip = "help...",
                           TipShowDirection _tipShowDirection = DOWN_TIP_SHOW_DIRECTION,
                           const ControlTypeId& _controlTtype = DEFAULT_CONTROL)
    {
        this->idNameLabel = _idNameLabel;
        this->descriptionTip = _descriptionTip;
        this->helpDescriptionTip = _helpDescriptionTip;
        this->tipShowDirection = _tipShowDirection;
        this->controlTtype = _controlTtype;
    }

    virtual ~CustomButtonDesignBase()
    {
    }

public:
    virtual void draw(QWidget* parent, QPainter* painter, QPoint posCursor,
                      const vcg::Matrix44f& matrixTransform, bool& _bIsPickedThisControl
                      /* just as assiest parameter if we picked this control*/
                      ) = 0;

    virtual void setVisible(bool _bIsVisible) = 0;

    virtual void mouseMoveCustomEvent(QWidget* parent, QPainter* painter, QPoint posCursor) = 0;

    virtual void mousePressEvent(QPoint posCursor) = 0;

    virtual void mouseReleaseEvent(QPoint posCursor) = 0;

    virtual void setAttributeInfomation(const BaseAttributeInfoButton& baseAttributeInfoButton) = 0;

    QString getIdNameLabel() const
    {
        return idNameLabel;
    }

    QString getDesrctionTip() const
    {
        return descriptionTip;
    }

    void setIdNameLabel(const QString& _idNameLabel)
    {
        this->idNameLabel = _idNameLabel;
    }

    void setDesrctionTip(const QString& _descriptionTip)
    {
        this->descriptionTip = _descriptionTip;
    }

    void setHelpDesrctionTip(const QString& _helpDescriptionTip)
    {
        this->helpDescriptionTip = _helpDescriptionTip;
    }

    QString getHelpDesrctionTip() const
    {
        return helpDescriptionTip;
    }

    TipShowDirection getTipShowDirection() const
    {
        return tipShowDirection;
    }

    ControlTypeId getTypeControl() const
    {
        return controlTtype;
    }

private:
    ControlTypeId controlTtype;
    QString idNameLabel;
    QString descriptionTip;
    QString helpDescriptionTip;
    TipShowDirection tipShowDirection;
};

/*
** some attribute info you may need changed by this class. it designed by liuchuang on
** 17/4/2019.
*/

class UI_COMMONSHARED_EXPORT BaseAttributeInfoButton
{
public:
    BaseAttributeInfoButton()
    {
        centerPos = Point3m(0, 0, 0);
        radius = 0.04f;    // radius or length  of this button
        heightTip = 0.04f;

        float sizeFont = 16;

        auto dpi = qApp->primaryScreen()->logicalDotsPerInch() * 1.0f / 96;

        sizeFont /= dpi;

        auto tipLabelPen = vcg::glLabel::Mode(vcg::Color4b::Yellow);
        tipLabelPen.qFont.setFamily("Microsoft YaHei UI");
        tipLabelPen.qFont.setStyleStrategy(QFont::PreferAntialias);
        tipLabelPen.qFont.setPixelSize(sizeFont);

        tipLabelFontMode = tipLabelPen;
        colorForeground = vcg::Color4f::Blue;
        colorBackground = vcg::Color4f::Green;
        colorMouseHoverStatus = vcg::Color4f::DarkGray;
        colorTipBackground = vcg::Color4f::Green;
        colorHelpTipBackground = vcg::Color4f::Green;
        colorBorderMarquee = vcg::Color4f::Yellow;
        colorBackgroundDisabled = vcg::Color4f(220 / 255.0f, 220 / 255.0f, 220 / 255.0f, 1.0f);

        bIsPlayStatus = false;    // just only used when you press the play button or checkbox
        bIsVisible = true;
    }

    BaseAttributeInfoButton(const Point3m& _centerPos, float _radius,
                            const vcg::glLabel::Mode& _tipLabelFontMode, float _heightTip = 0.1f,
                            const vcg::Color4f& _colorForeground = vcg::Color4f::Blue,
                            const vcg::Color4f& _colorBackground = vcg::Color4f::Green,
                            const vcg::Color4f& _colorMouseHoverStatus = vcg::Color4f::DarkGray,
                            const vcg::Color4f& _colorTipBackground = vcg::Color4f::Green,
                            const vcg::Color4f& _colorHelpTipBackground = vcg::Color4f::Green,
                            const vcg::Color4f& _colorBorderMarquee = vcg::Color4f::Yellow,
                            bool _bIsPlayStatus = false, bool _bIsVisible = true,
                            bool _bEnabled = true)
    {
        this->centerPos = _centerPos;
        this->radius = _radius;
        this->heightTip = _heightTip;
        this->tipLabelFontMode = _tipLabelFontMode;
        this->colorForeground = _colorForeground;
        this->colorBackground = _colorBackground;
        this->colorMouseHoverStatus = _colorMouseHoverStatus;
        this->colorTipBackground = _colorTipBackground;
        this->colorHelpTipBackground = _colorHelpTipBackground;
        this->colorBorderMarquee = _colorBorderMarquee;
        this->bIsPlayStatus = _bIsPlayStatus;
        this->bIsVisible = _bIsVisible;
        this->bEnabled = _bEnabled;
        this->colorBackgroundDisabled = vcg::Color4f::Gray;
    }

    BaseAttributeInfoButton(const BaseAttributeInfoButton& _baseAttributionInfo)
    {
        this->centerPos = _baseAttributionInfo.centerPos;
        this->radius = _baseAttributionInfo.radius;
        this->heightTip = _baseAttributionInfo.heightTip;
        this->tipLabelFontMode = _baseAttributionInfo.tipLabelFontMode;
        this->colorForeground = _baseAttributionInfo.colorForeground;
        this->colorBackground = _baseAttributionInfo.colorBackground;
        this->colorMouseHoverStatus = _baseAttributionInfo.colorMouseHoverStatus;
        this->colorTipBackground = _baseAttributionInfo.colorTipBackground;
        this->colorHelpTipBackground = _baseAttributionInfo.colorHelpTipBackground;
        this->colorBorderMarquee = _baseAttributionInfo.colorBorderMarquee;
        this->bIsPlayStatus = _baseAttributionInfo.bIsPlayStatus;
        this->bIsVisible = _baseAttributionInfo.bIsVisible;
        this->bEnabled = _baseAttributionInfo.bEnabled;
        this->colorBackgroundDisabled = _baseAttributionInfo.colorBackgroundDisabled;
    }

    ~BaseAttributeInfoButton()
    {
    }

public:
    bool getBaseAttributeIsVisible() const
    {
        return bIsVisible;
    }

    bool getBaseAttributeIsEnabled() const
    {
        return bEnabled;
    }

    Point3m getBaseAttributeCenterPos() const
    {
        return centerPos;
    }

    vcg::Color4f getBaseAttributeForegroundColor() const
    {
        return colorForeground;
    }

    vcg::Color4f getBaseAttributeBackgroundColor() const
    {
        return colorBackground;
    }

    vcg::Color4f getBaseAttributeMouseHoverStatusColor() const
    {
        return colorMouseHoverStatus;
    }

    vcg::Color4f getBaseAttributeTipBackgroundColor() const
    {
        return colorTipBackground;
    }

    vcg::Color4f getBaseAttributeHelpTipBackgroundColor() const
    {
        return this->colorHelpTipBackground;
    }

    vcg::Color4f getBaseAttributeBackgroundDisabledStatus() const
    {
        return this->colorBackgroundDisabled;
    }

    bool getBaseAttributePlayStatus() const
    {
        return bIsPlayStatus;
    }

    vcg::glLabel::Mode getBaseAttributeTipLabelFontModel() const
    {
        return tipLabelFontMode;
    }

    vcg::Color4f getBaseAttributeColorBorderMarquee() const
    {
        return colorBorderMarquee;
    }

    float getBaseAttributionHeightTip() const
    {
        return this->heightTip;
    }

    float getBaseAttributeRadius() const
    {
        return radius;
    }

    float getBaseAttributeHeight() const
    {
        return height;
    }

    float getBaseAttributeWidthBorderScaleRadius() const
    {
        return widthBorderScaleRadius;
    }

    void setBaseAttributeIsVisible(bool _bIsVisible)
    {
        this->bIsVisible = _bIsVisible;
    }

    void setBaseAttributeIsEnabled(bool _bEnabeld)
    {
        this->bEnabled = _bEnabeld;
    }

    void setBaseAttributeBackgroundDisabled(const vcg::Color4f _colorDisbaled)
    {
        this->colorBackgroundDisabled = _colorDisbaled;
    }

    void setBaseAttributeCenterPos(const Point3m& _centerPos)
    {
        this->centerPos = _centerPos;
    }

    void setBaseAttributeRadius(float _radius)
    {
        this->radius = _radius;
    }

    void setBaseAttributeHeight(float _height)
    {
        this->height = _height;
    }

    void setBaseAttributeWidthBorderScaleRadius(float _widthBorderScaleRadius)
    {
        this->widthBorderScaleRadius = _widthBorderScaleRadius;
    }

    void setBaseAttributionHeightTip(float _heightTip)
    {
        this->heightTip = _heightTip;
    }

    void setBaseAttributeForegroundColor(const vcg::Color4f& _colorForeground)
    {
        this->colorForeground = _colorForeground;
    }

    void setBaseAttributeBackgroundColor(const vcg::Color4f& _colorBackground)
    {
        this->colorBackground = _colorBackground;
    }

    void setBaseAttributeMouseHoverStatusColor(const vcg::Color4f& _colorMouseHoverStatus)
    {
        this->colorMouseHoverStatus = _colorMouseHoverStatus;
    }

    void setBaseAttributeTipBackgroundColor(const vcg::Color4f& _colorTipBackground)
    {
        this->colorTipBackground = _colorTipBackground;
    }

    void setBaseAttributeHelpTipBackgroundColor(const vcg::Color4f& _colorHelpTipBackground)
    {
        this->colorHelpTipBackground = _colorHelpTipBackground;
    }

    void setBaseAttributePlayStatus(bool _bIsPlayStatus)
    {
        this->bIsPlayStatus = _bIsPlayStatus;
    }

    void setBaseAttributeTipLabelFontModel(const vcg::glLabel::Mode& _tipLabelFontMode)
    {
        this->tipLabelFontMode = _tipLabelFontMode;
    }

    void setBaseAttributeColorMarquee(const vcg::Color4f& _colorBorderMarquee)
    {
        this->colorBorderMarquee = _colorBorderMarquee;
    }

    float getBaseAttributeWidthTipRadio() const
    {
        return this->widthTipRadio;
    }

    float getBaseAttributeHeightTipRadio() const
    {
        return this->heightTipRadio;
    }

    void setBaseAttributeWidthTipRadio(float _radio)
    {
        this->widthTipRadio = _radio;
    }

    void setBaseAttributeHeightTipRadio(float _radio)
    {
        this->heightTipRadio = _radio;
    }

private:
    Point3m centerPos;
    float radius;    // radius or length  of this button  radius = width(if the button is rectangle,
                     // the width is radius)
    float height = 0.1f;    // if the button is rectangle, the width is radius and height too.
    float widthBorderScaleRadius =
            0.1f;    // if you need the border, you could set the value radio by radius
    float heightTip;
    float widthTipRadio = 1.0;
    float heightTipRadio = 2.5f;
    vcg::glLabel::Mode tipLabelFontMode;
    vcg::Color4f colorForeground;
    vcg::Color4f colorBackground;
    vcg::Color4f colorMouseHoverStatus;
    vcg::Color4f colorTipBackground;
    vcg::Color4f colorHelpTipBackground;
    vcg::Color4f colorBorderMarquee;
    vcg::Color4f colorBackgroundDisabled;
    bool bIsPlayStatus = false;    // just only used when you press the play button or checkbox
    bool bIsVisible = true;
    bool bEnabled = true;
};

}    // namespace ui_common

#endif
