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

#ifndef QFUSIONPROCESSDLG_H
#define QFUSIONPROCESSDLG_H

#include <QColor>
#include <QDialog>
#include <QProgressBar>

#include "uicommon_global.h"

namespace Ui
{
class QFusionProgressDlg;
}

class QLabel;

namespace ui_common
{

class QFusionProgressBar;
class UI_COMMONSHARED_EXPORT QFusionProgressDlg : public QDialog
{
    Q_OBJECT

public:
    explicit QFusionProgressDlg(QWidget* parent = nullptr);
    ~QFusionProgressDlg();

    void setText(QString text);
    void setValue(int value);

public slots:
    void showProgressBar(QString text, int value);
    void showBusyProgressBar();

protected:
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* event);
    void showEvent(QShowEvent* event);

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent*) override;

private:
    Ui::QFusionProgressDlg* ui;
    QProgressBar* p_progress_bar_ = nullptr;
    QColor background_color = QColor(15, 120, 80, 1);
    bool b_setStyle = false;
    QWidget* p_parent_ = nullptr;
    bool b_busy_ = false;

    QFusionProgressBar* p_busy_progress_bar_ = nullptr;
};

// class UI_COMMONSHARED_EXPORT QFusionProgressDlg : public QDialog
//{
//     Q_OBJECT

// public:
//     explicit QFusionProgressDlg(QWidget *parent = nullptr);
//     ~QFusionProgressDlg();

// public slots:
//     //void onTimeOut();
//     void onSetProgressBarSlot(int percent, QString status);

// protected:
//     void paintEvent(QPaintEvent *event);
// private:
}    // namespace ui_common

#endif    // QFUSIONPROCESSDLG_H
