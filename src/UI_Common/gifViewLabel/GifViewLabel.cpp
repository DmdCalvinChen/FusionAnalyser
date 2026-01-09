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

#include "GifViewLabel.h"

#include <QButtonGroup>
#include <QDebug>
#include <QHBoxLayout>
#include <QMovie>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QSequentialAnimationGroup>
#include <QTimer>

#include <common_base/SignalManager.h>
const int btn_expand_width = 24;    // 按钮拉伸宽度
const int btn_shrik_width = 6;      // 按钮收缩宽度

GifViewLabel::GifViewLabel(QWidget* parent)
    : QLabel(parent), m_offset(0), m_curIndex(0), m_preIndex(0), m_blefttoright(true)
{
    this->setStyleSheet("background-color:#1A2835;");
    setMinimumSize(120, 240);
    m_btnGroup = new QButtonGroup(this);
    connect(m_btnGroup, SIGNAL(buttonClicked(int)), SLOT(onbuttonClicked(int)));

    m_btnParalGroup = new QParallelAnimationGroup(this);
    m_imageAnimation = new QPropertyAnimation(m_btnParalGroup, "");
    m_imageAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_imageAnimation->setDuration(400);    // 滑动时长
    connect(m_imageAnimation, SIGNAL(valueChanged(const QVariant&)), this,
            SLOT(onImagevalueChanged(const QVariant&)));

    QSequentialAnimationGroup* sequentialGroup = new QSequentialAnimationGroup(m_btnParalGroup);
    m_btnExpAnimation = new QPropertyAnimation(sequentialGroup, "");
    m_btnExpAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_btnExpAnimation, SIGNAL(valueChanged(const QVariant&)), this,
            SLOT(onBtnExpvalueChanged(const QVariant&)));
    m_btnExpAnimation->setDuration(200);

    m_btnShrikAnimation = new QPropertyAnimation(sequentialGroup, "");
    m_btnShrikAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_btnShrikAnimation, SIGNAL(valueChanged(const QVariant&)), this,
            SLOT(onBtnShrikvalueChanged(const QVariant&)));
    m_btnShrikAnimation->setDuration(200);

    // 按钮切换串行运行
    sequentialGroup->addAnimation(m_btnExpAnimation);
    sequentialGroup->addAnimation(m_btnShrikAnimation);

    // 图片切换与按钮切换并行运行
    m_btnParalGroup->addAnimation(m_imageAnimation);
    m_btnParalGroup->addAnimation(sequentialGroup);
    initControl();
}

GifViewLabel::~GifViewLabel()
{
}

void GifViewLabel::setGifSlot(bool is_marking_status)
{
    if (is_marking_status)
    {
        this->clear();
        mov.clear();

        addImage(m_imagepathpairlst2);
    }
    else
    {
        this->clear();
        addImage(m_imagepathpairlst);
    }
}

void GifViewLabel::initControl()
{
    m_horizontalLayoutWidget = new QWidget(this);
    m_horizontalLayoutWidget->setObjectName(QStringLiteral("horizontalLayoutWidget"));
    m_switchBtnLayout = new QHBoxLayout(m_horizontalLayoutWidget);
    m_switchBtnLayout->setSpacing(12);
    m_switchBtnLayout->setContentsMargins(0, 0, 0, 0);
}

void GifViewLabel::onImagevalueChanged(const QVariant& variant)
{
    m_offset = variant.toInt();
    update();
}

void GifViewLabel::onBtnExpvalueChanged(const QVariant& variant)
{
    m_btnGroup->button(m_curIndex)->setFixedWidth(variant.toInt());
}

void GifViewLabel::onBtnShrikvalueChanged(const QVariant& variant)
{
    for (int index = 0; index < m_imagepathpairlst.size(); index++)
    {
        if (m_curIndex != index && m_btnGroup->button(index)->width() > btn_shrik_width)
        {
            m_btnGroup->button(index)->setFixedWidth(variant.toInt());
        }
    }
}

void GifViewLabel::onbuttonClicked(int index)
{
    if (m_curIndex == index)
    {
        return;
    }
    if (index < 0)
    {
        index = m_imagepathpairlst.size() - 1;
    }
    if (index >= m_imagepathpairlst.size())
    {
        index = 0;
    }

    m_preIndex = m_curIndex;
    m_curIndex = index;
    qDebug("m_preIndex=%d,m_curIndex=%d,idx=%d", m_preIndex, m_curIndex, index);
    // 图片切换

    if (m_preIndex < m_curIndex)
    {
        // 向左滑
        m_imageAnimation->setStartValue(0);
        m_imageAnimation->setEndValue(0 - width());
        m_blefttoright = false;
        qDebug() << m_imagepathpairlst.at(m_curIndex).first;
        mov[m_preIndex]->stop();
        this->setMovie(mov[index]);
        mov[index]->start();
    }
    else
    {
        // 向右滑
        m_imageAnimation->setStartValue(0);
        m_imageAnimation->setEndValue(width());
        m_blefttoright = true;
        qDebug() << m_imagepathpairlst.at(m_curIndex).first;
        mov[m_preIndex]->stop();
        this->setMovie(mov[index]);
        mov[index]->start();
    }

    // 按钮切换
    m_btnShrikAnimation->setStartValue(btn_expand_width);
    m_btnShrikAnimation->setEndValue(btn_shrik_width);

    m_btnExpAnimation->setStartValue(btn_shrik_width);
    m_btnExpAnimation->setEndValue(btn_expand_width);

    m_btnParalGroup->start();
}

void GifViewLabel::paintEvent(QPaintEvent* event)
{
    QLabel::paintEvent(event);
}

void GifViewLabel::resizeEvent(QResizeEvent* event)
{
    m_horizontalLayoutWidget->setGeometry(
            QRect((width() - 45) / 2, height() - 5, width() - 24, 6));    // 按钮位置
    QLabel::resizeEvent(event);
}

void GifViewLabel::addImage(const QList<std::pair<QString, QString>>& imagepathpairlst)
{
    QLabel::repaint();
    m_imagepathpairlst = imagepathpairlst;
    for (int index = 0; index < imagepathpairlst.size(); index++)
    {
        QPushButton* btn = new QPushButton(m_horizontalLayoutWidget);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedSize(btn_shrik_width, btn_shrik_width);
        btn->setStyleSheet("QPushButton{background-color:white;border-width:0 3px;}");
        if (imagepathpairlst.size() == 1)
        {
            btn->hide();
        }
        m_btnGroup->addButton(btn, index);
        m_switchBtnLayout->addWidget(btn);
        QMovie* movie = new QMovie(m_imagepathpairlst.at(index).first);
        connect(movie, &QMovie::frameChanged, this, &GifViewLabel::onStopMovieSlot);
        mov[index] = movie;
        if (m_imagepathpairlst.at(index).first.indexOf("operation") != -1)
        {
            mov[index]->setScaledSize(QSize(320, 174));
        }
        else if (m_imagepathpairlst.at(index).first.indexOf("misstooth") != -1 ||
                 m_imagepathpairlst.at(index).first.indexOf("overlay") != -1)
        {
            mov[index]->setScaledSize(QSize(320, 225));
        }
        else
        {
            mov[index]->setScaledSize(QSize(320, 200));
        }
    }
    m_switchBtnLayout->addStretch();
    m_btnGroup->button(m_curIndex)->setFixedWidth(btn_expand_width);

    this->setMovie(mov[0]);
    mov[0]->start();
}

void GifViewLabel::onImageShowTimeOut()
{
    onbuttonClicked(m_curIndex + 1);
}

void GifViewLabel::onStopMovieSlot(int frameNumber)
{
    if (frameNumber == mov[m_curIndex]->frameCount() - 1)
    {
        mov[m_curIndex]->stop();
        onImageShowTimeOut();
    }
}
