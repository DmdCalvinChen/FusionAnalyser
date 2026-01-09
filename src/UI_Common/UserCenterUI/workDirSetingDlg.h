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

#ifndef WORKDIRSETINGDLG_H
#define WORKDIRSETINGDLG_H

#include <QDialog>

#include "uicommon_global.h"

namespace Ui
{
class WorkDirSetingDlg;
}

class UI_COMMONSHARED_EXPORT WorkDirSetingDlg : public QDialog
{
    Q_OBJECT

public:
    explicit WorkDirSetingDlg(QWidget* parent = nullptr);
    ~WorkDirSetingDlg();

protected:
    void showEvent(QShowEvent* event);

private:
    Ui::WorkDirSetingDlg* ui;
    bool isDirExist(QString fullPath);

public slots:
    void onShowFileSlot();
    void onCancelClickSlot();
};

#endif    // WORKDIRSETINGDLG_H
