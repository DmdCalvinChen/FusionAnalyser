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

#include "fusionFramelessBaseDlg.h"

#include <QMouseEvent>
#define CLOSEBTN_WIDTH 20
#define CLOSEBTN_HEIGHT 20
FusionFramelessBaseDlg::FusionFramelessBaseDlg(QWidget* parent /*= nullptr*/) : QWidget(parent)
{
    this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (parent != nullptr)
    {
        this->setWindowIcon(parent->windowIcon());
    }
}

FusionFramelessBaseDlg::~FusionFramelessBaseDlg()
{
}

bool FusionFramelessBaseDlg::isCaption(int x, int y)
{
    QRect rect = this->rect();
    QRect titlebarRect = QRect(rect.x(), rect.y(), rect.width(), 30);
    if (titlebarRect.contains(x, y))
    {
        return true;
    }
    return false;
}

void FusionFramelessBaseDlg::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        b_left_btn_pressed_ = true;
        start_pos_ = event->globalPos();

        b_is_catpion_ = isCaption(event->x(), event->y());
    }
}

void FusionFramelessBaseDlg::mouseMoveEvent(QMouseEvent* event)
{
    if (b_left_btn_pressed_)
    {
        if (b_is_catpion_)
        {
            int cursor = event->globalY();
            this->move(this->geometry().topLeft() + event->globalPos() - start_pos_);
            start_pos_ = event->globalPos();
        }
    }
}

void FusionFramelessBaseDlg::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        b_left_btn_pressed_ = false;
    }
}

void FusionFramelessBaseDlg::createCloseBtn(QWidget* parent, int width)
{
    p_close_btn_ = new QPushButton(parent);
    p_close_btn_->setAutoFillBackground(true);
    p_close_btn_->setObjectName("closeBtn");
    p_close_btn_->setAttribute(Qt::WA_TranslucentBackground);
    p_close_btn_->setFlat(true);
    p_close_btn_->setFixedSize(CLOSEBTN_WIDTH, CLOSEBTN_HEIGHT);
    int width3 = p_close_btn_->width();
    p_close_btn_->setGeometry(width - p_close_btn_->width() - 8, 8, p_close_btn_->width(),
                              p_close_btn_->height());
    connect(p_close_btn_, &QPushButton::clicked, [&] { this->close(); });
}

void FusionFramelessBaseDlg::createTilte(QLabel* pTitleLabel, int width)
{
    pTitleLabel->setGeometry(8, 8, pTitleLabel->width(), pTitleLabel->height());
}
