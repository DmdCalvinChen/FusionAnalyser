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

#include "QFusionProgressDlg.h"

#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QTimer>
#include <QVBoxLayout>

#include "QFusionProgressBar.h"
#include "common_base/SignalManager.h"
#include "common_base/util/uitools.h"
#include "ui_QFusionProgressDlg.h"

using namespace ui_common;

#include <QApplication>
#include <QBoxLayout>
#include <QDesktopWidget>
#include <QPainter>

QFusionProgressDlg::QFusionProgressDlg(QWidget* parent)
    : QDialog(parent), ui(new Ui::QFusionProgressDlg)
{
    p_parent_ = parent;
    ui->setupUi(this);
    this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    p_progress_bar_ = new QProgressBar(this);
    p_progress_bar_->setFixedSize(QSize(750, 50));
    p_progress_bar_->setMinimum(0);
    p_progress_bar_->setMaximum(1000);
    p_progress_bar_->setObjectName("fusionProgressBar");
    p_progress_bar_->setOrientation(Qt::Horizontal);
    p_busy_progress_bar_ = new QFusionProgressBar(this);
    p_busy_progress_bar_->setBarStyle(QFusionProgressBar::StyleAnimation);

    int screen_width(0), screen_height(0);
    const QRect screenGeometry = QApplication::desktop()->screenGeometry(this);
    QRect desktop = parent->frameGeometry();
    screen_width = desktop.width();
    screen_height = desktop.height();
    this->setFixedSize(screen_width, screen_height);
}

QFusionProgressDlg::~QFusionProgressDlg()
{
    delete ui;
}

void QFusionProgressDlg::showProgressBar(QString text, int value)
{
    if (p_progress_bar_ == nullptr)
    {
        return;
    }

    b_busy_ = false;
    p_progress_bar_->setMinimum(0);
    p_progress_bar_->setMaximum(1000);
    if (value <= p_progress_bar_->minimum() || value >= p_progress_bar_->maximum())
    {
        this->hide();
        return;
    }

    double dProgress = (p_progress_bar_->value() - p_progress_bar_->minimum()) * 100.0 /
                       (p_progress_bar_->maximum() - p_progress_bar_->minimum());

    int width = this->width();
    int prograssbar = p_progress_bar_->width();
    p_progress_bar_->setValue(value);
    p_progress_bar_->setFormat(text);
    this->show();
}

void QFusionProgressDlg::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), background_color);
}

void QFusionProgressDlg::resizeEvent(QResizeEvent* event)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), background_color);

    if (p_progress_bar_)
        p_progress_bar_->setGeometry(this->width() / 2 - p_progress_bar_->width() / 2,
                                     this->height() / 2 - p_progress_bar_->height() / 2,
                                     p_progress_bar_->width(), p_progress_bar_->height());
}

void QFusionProgressDlg::mousePressEvent(QMouseEvent* event)
{
    background_color = QColor(55, 185, 218, 135);
    this->update();
}

void QFusionProgressDlg::mouseReleaseEvent(QMouseEvent* event)
{
    background_color = QColor(15, 120, 80, 1);
    this->update();
}

void ui_common::QFusionProgressDlg::showBusyProgressBar()
{
    b_busy_ = true;
    p_busy_progress_bar_->startAnimation(100);
    this->show();
}

void QFusionProgressDlg::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
            break;
        default:
            QDialog::keyPressEvent(event);
    }
}

void QFusionProgressDlg::showEvent(QShowEvent* event)
{
    QRect desktop = p_parent_->geometry();
    int screen_width = desktop.width();
    int screen_height = desktop.height();
    this->setFixedSize(screen_width, screen_height);
    p_busy_progress_bar_->hide();
    p_progress_bar_->hide();

    if (!b_busy_)
    {
        p_progress_bar_->setGeometry(this->width() / 2 - p_progress_bar_->width() / 2,
                                     this->height() / 2 - p_progress_bar_->height() / 2,
                                     p_progress_bar_->width(), p_progress_bar_->height());
        p_progress_bar_->show();
    }
    else
    {
        p_busy_progress_bar_->setGeometry(this->width() / 2 - 50, this->height() / 2 - 50, 100,
                                          100);
        p_busy_progress_bar_->show();
    }

    return QDialog::showEvent(event);
}
//
