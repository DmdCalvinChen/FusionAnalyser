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

#ifndef SLIDER_H
#define SLIDER_H

#include <GL/glew.h>
#include <wrap/qt/gl_label.h>

#include "../uicommon_global.h"
#include "custombuttondesignbase.h"

/*
** slider designed by  liuchuang on 19/3/2019.
*/

namespace ui_common
{
enum ValueTypeSlider
{
    INT_VALUE_TTYPE,    // [0,225]
    FLOAT_VALUE_TYPE    //[0,1.0]
};

class UI_COMMONSHARED_EXPORT SliderAttributionInfo : public BaseAttributeInfoButton
{
public:
    SliderAttributionInfo()
    {
        widthSlider = 0.1f;
    }
    SliderAttributionInfo(const BaseAttributeInfoButton& _baseAttributeInfo, const int& _minValue,
                          const int& _maxValue, const int& _valueDefault, int _widthSlider,
                          const vcg::Color4f& _colorAnchor)
        : BaseAttributeInfoButton(_baseAttributeInfo)
    {
        this->minValueRange = _minValue;
        this->maxValueRange = _maxValue;
        this->valueDefault = _valueDefault;
        this->widthSlider = _widthSlider;
        this->colorAnchor = _colorAnchor;
    }

    ~SliderAttributionInfo()
    {
    }

    vcg::Color4f getColorAnchor() const
    {
        return colorAnchor;
    }

    float getSliderWidth() const
    {
        return widthSlider;
    }

    int getDefaultValueOfSlider() const
    {
        return valueDefault;
    }

    int getMinValueOfSliderRange() const
    {
        return minValueRange;
    }

    int getMaxValueOfSliderRange() const
    {
        return maxValueRange;
    }

    void setDefultValueOfSlider(const int& _valueDefaultSlider)
    {
        this->valueDefault = _valueDefaultSlider;
    }

    void setMinValueOfSliderRange(const int& _minValueSlider)
    {
        this->minValueRange = _minValueSlider;
    }
    void setMaxValueOfSliderRange(const int& _maxValueSlider)
    {
        this->maxValueRange = _maxValueSlider;
    }

    void setSliderWidth(float _widthSlider)
    {
        this->widthSlider = _widthSlider;
    }
    void setColorAnchor(const vcg::Color4f _colorAnchor)
    {
        this->colorAnchor = _colorAnchor;
    }

private:
    vcg::Color4f colorAnchor;
    float widthSlider;
    float minValueRange;
    float maxValueRange;
    float valueDefault;
};

class UI_COMMONSHARED_EXPORT SignalSliderProxy : public QObject
{
    Q_OBJECT
public:
    SignalSliderProxy()
    {
    }
    ~SignalSliderProxy()
    {
    }

signals:
    void valueCurrentSliderIntValueRange(int value);
    void valueCurrentSliderFloatValueRange(int value);
    void clicked();
    void pressedStatus(bool bIsChecked);
};

class UI_COMMONSHARED_EXPORT Slider : public CustomButtonDesignBase, public SignalSliderProxy
{
public:
    Slider()
    {
    }
    Slider(const QString& _desrction, const QString& _helpDesrction,
           TipShowDirection _tipShowDirection = DOWN_TIP_SHOW_DIRECTION,
           const ControlTypeId& _typeControl = DEFAULT_CONTROL)
        : CustomButtonDesignBase("", _desrction, _helpDesrction, _tipShowDirection, _typeControl)
    {
    }
    virtual ~Slider()
    {
    }

public:
    void setVisible(bool _bIsVisible);
    void setValue(const int& _value);
    void setAttributeInfomation(const BaseAttributeInfoButton& _baseAttributeInfomation);
    void mouseMoveCustomEvent(QWidget* parent, QPainter* painter, QPoint posCursor);
    void mousePressEvent(QPoint posCursor);
    void mouseReleaseEvent(QPoint posCursor);
    void draw(QWidget* parent, QPainter* painter, QPoint posCursor,
              const Matrix44m& matrixTransform, bool& _bTouchedThisControl);
    bool cursorIsStayOnThisButton(QPoint cursorPos, Point3m& transformCursor);
    void setCurrentPlayStatus(bool _bIsPlaying = false);
    SliderAttributionInfo baseAttributeInfomation;

private:
    Matrix44m matrixTransform = Matrix44m::Identity();
    Matrix44m matrixCurrentAnchor = Matrix44m::Identity();
    QWidget* parent = nullptr;
    int numDiv = 20;
    bool bTouchedThisAnchor = false;
    bool bCursorStayOnThisButton;
    ValueTypeSlider valueType = FLOAT_VALUE_TYPE;
    bool bFirstUseDefaultValue = true;
};

}    // namespace ui_common
#endif
