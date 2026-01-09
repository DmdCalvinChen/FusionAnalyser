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

#ifndef FUSIONANALYSERHOMEGUI_H
#define FUSIONANALYSERHOMEGUI_H

#include <QWidget>

#include "../uicommon_global.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class FusionAnalyserHomeGui;
}
QT_END_NAMESPACE
/*
 *
 * 分析软件首页打开等页面
 */

namespace ui_common
{
class UI_COMMONSHARED_EXPORT FusionAnalyserHomeGui : public QWidget
{
    Q_OBJECT

public:
    FusionAnalyserHomeGui(QWidget* parent = nullptr);
    ~FusionAnalyserHomeGui();
    void initToolButtonsGui();

    void paintEvent(QPaintEvent* event);
    void addRecentlyProject(const QString& project, const QString& name, const QString& date);

    // 检测路径下是否有工程存在
    std::vector<QString> getProjectsDir(const QString& dir, const QStringList& modelName);

public slots:
    void selectWhichRecentlyProject();
    bool importModelSlot();
    bool openRecentlyMeasureProject();
    // 更新最近的几条工程记录 通过注册表
    void updateRecentProjects();
    // 显示与影藏控制 ，显示的时候会更新最近的工程列表
    void setShowState(bool enable_show);

signals:
    void importModelSignal(QStringList& models_file);

    // 打开最近工程包括打开最近的测量工程
    void openRecentlyProject(const QString& project);

private:
    Ui::FusionAnalyserHomeGui* ui;
    // 最多的最近工程文件记录
    int max_number_projects_record_ = 5;
};
}    // namespace ui_common
#endif    // FUSIONANALYSERHOMEGUI_H
