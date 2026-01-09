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

#ifndef QFUSION_PROCESS_BAR_H_
#define QFUSION_PROCESS_BAR_H_

#include <QWidget>

namespace ui_common
{
class QFusionProgressBar : public QWidget
{
    Q_OBJECT
public:
    explicit QFusionProgressBar(QWidget* parent = 0);
    ~QFusionProgressBar();

    double nullPosition() const
    {
        return start_pos_;
    }
    void setNullPosition(double position);
    enum BarStyle
    {
        StyleDonut,
        StylePie,
        StyleLine,
        StyleAnimation
    };
    void setBarStyle(BarStyle style);
    BarStyle barStyle() const
    {
        return bar_style_;
    }
    void setOutlinePenWidth(double penWidth);
    double outlinePenWidth() const
    {
        return outline_pen_width_;
    }
    void setDataPenWidth(double penWidth);
    double dataPenWidth() const
    {
        return data_pen_width_;
    }
    void setDataColors(const QGradientStops& stopPoints);
    void setFormat(const QString& format);
    void resetFormat();
    QString format() const
    {
        return s_format_;
    }
    void setDecimals(int count);
    int decimals() const
    {
        return i_decimals;
    }
    double value() const
    {
        return d_value_;
    }
    double minimum() const
    {
        return d_min_;
    }
    double maximum() const
    {
        return d_max_;
    }

public slots:
    void setRange(double min, double max);
    void setMinimum(double min);
    void setMaximum(double max);
    void setValue(double val);
    void setValue(int val);
    void onThreadUpdateSlot();

protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void timerEvent(QTimerEvent* event);

protected:
    void drawBackground(QPainter& p, const QRectF& baseRect);
    void drawBase(QPainter& p, const QRectF& baseRect, const QRectF& innerRect);
    void drawValue(QPainter& p, const QRectF& baseRect, double value, double arcLength,
                   const QRectF& innerRect, double innerRadius);
    void calculateInnerRect(const QRectF& baseRect, double outerRadius, QRectF& innerRect,
                            double& innerRadius);
    void drawInnerBackground(QPainter& p, const QRectF& innerRect);
    void drawText(QPainter& p, const QRectF& innerRect, double innerRadius, double value);
    QString valueToText(double value) const;
    void valueFormatChanged();
    QSize minimumSizeHint() const
    {
        return QSize(32, 32);
    }
    bool hasHeightForWidth() const
    {
        return true;
    }
    int heightForWidth(int w) const
    {
        return w;
    }
    void rebuildDataBrushIfNeeded();

protected:
    double d_min_, d_max_;
    double d_value_;

    double start_pos_;
    BarStyle bar_style_;
    double outline_pen_width_, data_pen_width_;
    float inner_outter_rate_ = 0.8f;

    QGradientStops gradient_data_;
    bool b_rebuild_brush_;

    QString s_format_;
    int i_decimals;

    static const int UF_VALUE = 1;
    static const int UF_PERCENT = 2;
    static const int UF_MAX = 4;
    int m_updateFlags;

public:    // animation
    void startAnimation(unsigned int delay);
    void stopAnimation(unsigned int delay);

protected:
    int timer_id_ = -1;
    unsigned int angle_ = 0;
    unsigned int animate_speed_ = 20;
    unsigned int delay_ = 0;
    QPixmap* animate_img_ = nullptr;
};
}    // namespace ui_common

#endif    // QROUNDPROGRESSBAR_H
