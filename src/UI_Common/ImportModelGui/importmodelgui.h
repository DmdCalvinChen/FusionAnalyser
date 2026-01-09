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

#ifndef IMPORTMODELGUI_H
#define IMPORTMODELGUI_H

#include <QDialog>

#include "uicommon_global.h"

namespace Ui
{
class ImportModelGui;
}

/*
 *   导入上下ct模型文件选择对话框
 */

class UI_COMMONSHARED_EXPORT ImportModelGui : public QDialog
{
    Q_OBJECT

public:
    explicit ImportModelGui(QWidget* parent = nullptr);
    ~ImportModelGui();

    QString getUpCtModelFile() const
    {
        return up_ct_file_path_;
    }
    QString getDownCtModelFile() const
    {
        return down_ct_file_path_;
    }

private:
    void loadCtModel();
    void hide();

private:
    Ui::ImportModelGui* ui;
    QString up_ct_file_path_, down_ct_file_path_;
};

#endif    // IMPORTMODELGUI_H
