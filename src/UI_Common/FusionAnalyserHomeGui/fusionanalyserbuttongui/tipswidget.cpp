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

#include "tipswidget.h"

#include <QPainter>

#include "common_base/SignalManager.h"
#include "ui_tipswidget.h"

tipsWidget::tipsWidget(QWidget* pGLa, QWidget* parent) : QDialog(parent), ui(new Ui::tipsWidget)
{
    this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    this->setMinimumHeight(30);
    ui->setupUi(this);
    this->setMouseTracking(true);
    p_gla_ = pGLa;
    connect(ui->tipsButton, &QPushButton::clicked, this, &tipsWidget::tipsbuttonSlot);
    openGif = false;
}

tipsWidget::~tipsWidget()
{
    delete ui;
}

void tipsWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(15, 120, 80, 1));
}

void tipsWidget::resizeEvent(QResizeEvent* event)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(15, 120, 80, 1));

    QPoint pointOri = p_gla_->mapToGlobal(QPoint(0, 0));
    this->setGeometry(pointOri.x() + p_gla_->width() - this->width() / 1.2, pointOri.y(),
                      this->width(), this->height());
}

void tipsWidget::tipsbuttonSlot()
{
    if (open_gif_type == "overlaygif")
    {
        emit PSIGNALMANAGER->showOverlaySignal();
        return;
    }
    else if (open_gif_type == "teethwidthgif")
    {
        emit PSIGNALMANAGER->showTeethWidthSignal();
        return;
    }
    openGif = !openGif;
    emit PSIGNALMANAGER->showGifSignal(openGif);
}
