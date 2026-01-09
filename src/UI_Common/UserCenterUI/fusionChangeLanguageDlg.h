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

#ifndef FUSIONCHANGELANGUAGEDLG_H
#define FUSIONCHANGELANGUAGEDLG_H

#include <QDialog>

#include "uicommon_global.h"

namespace Ui
{
class FusionChangeLanguageDlg;
}

class UI_COMMONSHARED_EXPORT FusionChangeLanguageDlg : public QDialog
{
    Q_OBJECT

public:
    explicit FusionChangeLanguageDlg(QWidget* parent = nullptr);
    ~FusionChangeLanguageDlg();

public slots:
    void onLanguageIndexChanged(int index);
    void onOkBtnClickedSlot();

private:
    Ui::FusionChangeLanguageDlg* ui;
};

#endif    // FUSIONCHANGELANGUAGEDLG_H
