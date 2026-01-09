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

#ifndef REDOUNDOGUI_H
#define REDOUNDOGUI_H

#include <QDialog>

#include "uicommon_global.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class redoUndoGui;
}
QT_END_NAMESPACE

class UI_COMMONSHARED_EXPORT redoUndoGui : public QDialog
{
    Q_OBJECT

public:
    explicit redoUndoGui(QWidget* parent = nullptr);
    ~redoUndoGui();

protected:
    //        void initBtn();
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* event);
    void keyPressEvent(QKeyEvent* event);

private:
    Ui::redoUndoGui* ui;
    QWidget* p_gla_ = nullptr;
    QWidget* p_parent_ = nullptr;
public slots:
    void canUndoOrRedoStatusSlot(bool bCanUndo, bool bCanRedo);
};
#endif    // REDOUNDOGUI_H
