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

#include "redoUndoGui.h"

#include <QKeyEvent>
#include <QPainter>

#include <common_base/SignalManager.h>

#include "ui_redoUndoGui.h"
#define FUSIONVIEWGUIHIGHT 30

redoUndoGui::redoUndoGui(QWidget* parent) : QDialog(parent), ui(new Ui::redoUndoGui)
{
    this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    this->setMinimumHeight(FUSIONVIEWGUIHIGHT);
    ui->setupUi(this);
    ui->redoBtn->setToolTip(tr("Redo"));
    ui->undoBtn->setToolTip(tr("Undo"));
    this->setMouseTracking(true);
    connect(ui->undoBtn, &QPushButton::clicked, PSIGNALMANAGER, &SignalManager::unDoSignal);
    connect(ui->redoBtn, &QPushButton::clicked, PSIGNALMANAGER, &SignalManager::reDoSignal);
    connect(PSIGNALMANAGER, &SignalManager::canUndoOrRedoStatusSignal, this,
            &redoUndoGui::canUndoOrRedoStatusSlot);
}

redoUndoGui::~redoUndoGui()
{
    delete ui;
}

void redoUndoGui::canUndoOrRedoStatusSlot(bool bCanUndo, bool bCanRedo)
{
    if (bCanUndo)
    {
        ui->undoBtn->setEnabled(true);
    }
    else
    {
        ui->undoBtn->setEnabled(false);
    }
    if (bCanRedo)
    {
        ui->redoBtn->setEnabled(true);
    }
    else
    {
        ui->redoBtn->setEnabled(false);
    }
}

void redoUndoGui::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(15, 120, 80, 1));
}

void redoUndoGui::resizeEvent(QResizeEvent* event)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(15, 120, 80, 1));
}

void redoUndoGui::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Z && event->modifiers() == Qt::ControlModifier)
    {
        if (ui->undoBtn->isEnabled())
        {
            emit PSIGNALMANAGER->unDoSignal();
        }
    }

    if (event->key() == Qt::Key_Y && event->modifiers() == Qt::ControlModifier)
    {
        if (ui->redoBtn->isEnabled())
        {
            emit PSIGNALMANAGER->reDoSignal();
        }
    }
}
