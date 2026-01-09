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

#ifndef ANALYSISREPORTGUI_H
#define ANALYSISREPORTGUI_H

#include <QScrollArea>

QT_BEGIN_NAMESPACE
namespace Ui
{
class AnalysisReportGui;
}
QT_END_NAMESPACE

class AnalysisReport;

/*
 * 模型分析报告界面
 */
class AnalysisReportGui : public QScrollArea
{
    Q_OBJECT

public:
    AnalysisReportGui(QWidget* parent = nullptr);
    ~AnalysisReportGui();

private:
    Ui::AnalysisReportGui* ui;
    AnalysisReport* report_gui_ = nullptr;
};
#endif    // ANALYSISREPORTGUI_H
