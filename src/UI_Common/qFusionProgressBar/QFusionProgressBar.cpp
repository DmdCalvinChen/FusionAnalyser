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

#include "QFusionProgressBar.h"

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>

using namespace ui_common;
QFusionProgressBar::QFusionProgressBar(QWidget* parent)
    : QWidget(parent), d_min_(0), d_max_(100), d_value_(25), start_pos_(90), bar_style_(StyleDonut),
      outline_pen_width_(1), data_pen_width_(1), b_rebuild_brush_(false), s_format_("%p%"),
      i_decimals(1), m_updateFlags(UF_PERCENT)
{
    if (animate_img_ == nullptr)
        animate_img_ = new QPixmap(":/default/res/default/images/progressBar/loading.png");
    QPalette palette;
    palette.setBrush(QPalette::Window, Qt::NoBrush);
    palette.setBrush(QPalette::AlternateBase, Qt::NoBrush);
    palette.setBrush(QPalette::Highlight, QBrush(QColor(0, 140, 255)));
    this->setPalette(palette);
}

QFusionProgressBar::~QFusionProgressBar()
{
    if (animate_img_)
    {
        delete animate_img_;
        animate_img_ = nullptr;
    }
}

void QFusionProgressBar::setRange(double min, double max)
{
    d_min_ = min;
    d_max_ = max;

    if (d_max_ < d_min_)
        qSwap(d_max_, d_min_);

    if (d_value_ < d_min_)
        d_value_ = d_min_;
    else if (d_value_ > d_max_)
        d_value_ = d_max_;

    if (!gradient_data_.isEmpty())
        b_rebuild_brush_ = true;

    update();
}

void QFusionProgressBar::setMinimum(double min)
{
    setRange(min, d_max_);
}

void QFusionProgressBar::setMaximum(double max)
{
    setRange(d_min_, max);
}

void QFusionProgressBar::setValue(double val)
{
    if (d_value_ != val)
    {
        if (val < d_min_)
            d_value_ = d_min_;
        else if (val > d_max_)
            d_value_ = d_max_;
        else
            d_value_ = val;

        update();
    }
}

void QFusionProgressBar::setValue(int val)
{
    setValue(double(val));
}

void QFusionProgressBar::setNullPosition(double position)
{
    if (position != start_pos_)
    {
        start_pos_ = position;

        if (!gradient_data_.isEmpty())
            b_rebuild_brush_ = true;

        update();
    }
}

void QFusionProgressBar::setBarStyle(QFusionProgressBar::BarStyle style)
{
    if (style != bar_style_)
    {
        bar_style_ = style;

        update();
    }
}

void QFusionProgressBar::setOutlinePenWidth(double penWidth)
{
    if (penWidth != outline_pen_width_)
    {
        outline_pen_width_ = penWidth;

        update();
    }
}

void QFusionProgressBar::setDataPenWidth(double penWidth)
{
    if (penWidth != data_pen_width_)
    {
        data_pen_width_ = penWidth;

        update();
    }
}

void QFusionProgressBar::setDataColors(const QGradientStops& stopPoints)
{
    if (stopPoints != gradient_data_)
    {
        gradient_data_ = stopPoints;
        b_rebuild_brush_ = true;

        update();
    }
}

void QFusionProgressBar::setFormat(const QString& format)
{
    if (format != s_format_)
    {
        s_format_ = format;

        valueFormatChanged();
    }
}

void QFusionProgressBar::resetFormat()
{
    s_format_ = QString::null;

    valueFormatChanged();
}

void QFusionProgressBar::setDecimals(int count)
{
    if (count >= 0 && count != i_decimals)
    {
        i_decimals = count;

        valueFormatChanged();
    }
}

void QFusionProgressBar::paintEvent(QPaintEvent* /*event*/)
{
    double outerRadius = qMin(width(), height());
    QRectF baseRect(1, 1, outerRadius - 2, outerRadius - 2);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // data brush
    rebuildDataBrushIfNeeded();

    drawBackground(p, rect());
    double innerRadius(0);
    QRectF innerRect;
    calculateInnerRect(baseRect, outerRadius, innerRect, innerRadius);
    double arcStep = 360.0 / (d_max_ - d_min_) * d_value_;

    drawBase(p, baseRect, innerRect);

    drawValue(p, baseRect, d_value_, arcStep, innerRect, innerRadius);

    drawInnerBackground(p, innerRect);
    drawText(p, innerRect, innerRadius, d_value_);
    p.end();
}

void QFusionProgressBar::timerEvent(QTimerEvent* event)
{
    angle_ = (angle_ + animate_speed_) % 360;
    update();
}

void QFusionProgressBar::drawBackground(QPainter& p, const QRectF& baseRect)
{
    p.fillRect(baseRect, palette().background());
}

void QFusionProgressBar::drawBase(QPainter& p, const QRectF& baseRect, const QRectF& innerRect)
{
    p.save();
    auto drawDonut = [&]()
    {
        QPainterPath dataPath;
        dataPath.setFillRule(Qt::OddEvenFill);
        dataPath.moveTo(baseRect.center());
        dataPath.addEllipse(baseRect);
        dataPath.addEllipse(innerRect);
        p.setPen(QPen(palette().base().color(), outline_pen_width_));
        p.setBrush(palette().base());
        p.drawPath(dataPath);
    };
    auto setPainter = [&]()
    {
        p.setPen(QPen(palette().base().color(), outline_pen_width_));
        p.setBrush(palette().base());
    };
    switch (bar_style_)
    {
        case StyleDonut:
        {
            drawDonut();
            break;
        }

        case StylePie:
        {
            setPainter();
            p.drawEllipse(baseRect);
            break;
        }

        case StyleLine:
        {
            setPainter();
            p.drawEllipse(baseRect.adjusted(outline_pen_width_ / 2, outline_pen_width_ / 2,
                                            -outline_pen_width_ / 2, -outline_pen_width_ / 2));
            break;
        }

        case StyleAnimation:
        {
            break;
        }
        default:;
    }
    p.restore();
}

void QFusionProgressBar::drawValue(QPainter& p, const QRectF& baseRect, double value,
                                   double arcLength, const QRectF& innerRect, double innerRadius)
{
    if (value == d_min_)
        return;

    if (bar_style_ == StyleLine)
    {
        p.setPen(QPen(palette().highlight().color(), data_pen_width_));
        p.setBrush(Qt::NoBrush);
        p.drawArc(baseRect.adjusted(outline_pen_width_ / 2, outline_pen_width_ / 2,
                                    -outline_pen_width_ / 2, -outline_pen_width_ / 2),
                  start_pos_ * 16, -arcLength * 16);
        return;
    }

    if (bar_style_ == StylePie || bar_style_ == StyleDonut)
    {
        QPainterPath dataPath;
        dataPath.setFillRule(Qt::WindingFill);

        dataPath.moveTo(baseRect.center());
        dataPath.arcTo(baseRect, start_pos_, -arcLength);

        if (bar_style_ == StylePie)
        {
            dataPath.lineTo(baseRect.center());
            p.setPen(QPen(palette().shadow().color(), data_pen_width_));
        }

        auto drawDonut = [&]()
        {
            QPointF curPoint = dataPath.currentPosition();
            curPoint = baseRect.center() + ((curPoint - baseRect.center()) * inner_outter_rate_);
            dataPath.lineTo(curPoint);
            dataPath.moveTo(baseRect.center());
            dataPath.arcTo(innerRect, start_pos_ - arcLength, arcLength);
            curPoint = dataPath.currentPosition();
            curPoint =
                    baseRect.center() + ((curPoint - baseRect.center()) * (2 - inner_outter_rate_));
            dataPath.lineTo(curPoint);
            p.setPen(Qt::NoPen);
        };
        if (bar_style_ == StyleDonut)
        {
            drawDonut();
        }
        p.setBrush(palette().highlight());
        p.drawPath(dataPath);
    }

    if (bar_style_ == StyleAnimation)
    {
        p.save();
        p.translate(width() / 2, height() / 2);
        int wid = width();
        p.rotate(angle_);
        float scaleX = (float)this->width() / animate_img_->width();
        float scaleY = (float)this->height() / animate_img_->height();

        p.scale(scaleX, scaleY);
        p.drawPixmap(-animate_img_->width() / 2, -animate_img_->height() / 2, *animate_img_);
        p.restore();
    }
}

void QFusionProgressBar::calculateInnerRect(const QRectF& /*baseRect*/, double outerRadius,
                                            QRectF& innerRect, double& innerRadius)
{
    if (bar_style_ == StyleLine)
    {
        innerRadius = outerRadius - outline_pen_width_;
    }
    else
    {
        innerRadius = outerRadius * inner_outter_rate_;
    }

    double delta = (outerRadius - innerRadius) / 2;
    innerRect = QRectF(delta, delta, innerRadius, innerRadius);
}

void QFusionProgressBar::drawInnerBackground(QPainter& p, const QRectF& innerRect)
{
    if (bar_style_ == StyleDonut)
    {
        p.setBrush(palette().alternateBase());
        p.drawEllipse(innerRect);
    }
}

void QFusionProgressBar::drawText(QPainter& p, const QRectF& innerRect, double innerRadius,
                                  double value)
{
    if (s_format_.isEmpty())
        return;

    QFont f(font());
    f.setPointSize(8);
    p.setFont(f);

    QRectF textRect(innerRect);
    p.setPen(palette().text().color());
    p.drawText(textRect, Qt::AlignCenter, QObject::tr("Under analysis"));
}

QString QFusionProgressBar::valueToText(double value) const
{
    QString textToDraw(s_format_);

    if (m_updateFlags & UF_VALUE)
        textToDraw.replace("%v", QString::number(value, 'f', i_decimals));

    if (m_updateFlags & UF_PERCENT)
    {
        double procent = (value - d_min_) / (d_max_ - d_min_) * 100.0;
        textToDraw.replace("%p", QString::number(procent, 'f', i_decimals));
    }

    if (m_updateFlags & UF_MAX)
        textToDraw.replace("%m", QString::number(d_max_ - d_min_ + 1, 'f', i_decimals));

    return textToDraw;
}

void QFusionProgressBar::valueFormatChanged()
{
    m_updateFlags = 0;

    if (s_format_.contains("%v"))
        m_updateFlags |= UF_VALUE;

    if (s_format_.contains("%p"))
        m_updateFlags |= UF_PERCENT;

    if (s_format_.contains("%m"))
        m_updateFlags |= UF_MAX;

    update();
}

void QFusionProgressBar::rebuildDataBrushIfNeeded()
{
    if (b_rebuild_brush_)
    {
        b_rebuild_brush_ = false;

        QConicalGradient dataBrush;
        dataBrush.setCenter(0.5, 0.5);
        dataBrush.setCoordinateMode(QGradient::StretchToDeviceMode);

        // invert colors
        for (int i = 0; i < gradient_data_.count(); i++)
        {
            dataBrush.setColorAt(1.0 - gradient_data_.at(i).first, gradient_data_.at(i).second);
        }

        QPalette p(palette());
        p.setBrush(QPalette::Highlight, dataBrush);
        setPalette(p);
    }
}

void QFusionProgressBar::startAnimation(unsigned int delay)
{
    angle_ = 0;
    if (timer_id_ == -1)
    {
        timer_id_ = startTimer(delay);
    }
}

void QFusionProgressBar::stopAnimation(unsigned int delay)
{
    if (timer_id_ != -1)
    {
        killTimer(timer_id_);
    }
    timer_id_ = -1;
    update();
}

void ui_common::QFusionProgressBar::onThreadUpdateSlot()
{
    angle_ = (angle_ + animate_speed_) % 360;
    update();
}
