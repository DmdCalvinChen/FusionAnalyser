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

#ifndef PDFREADERGUI_H
#define PDFREADERGUI_H

#include <QCheckBox>
#include <QWidget>

#include <common/config.h>

#include "framelessWindow/fusionFramelessBaseDlg.h"
#include "uicommon_global.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class PdfReaderGui;
}
QT_END_NAMESPACE

class PdfReader;
class AnalysisReport;

enum EditItemIndex : int;

/*
 * pdf预览窗口
 */
class UI_COMMONSHARED_EXPORT PdfReaderGui : public FusionFramelessBaseDlg
{
    Q_OBJECT

public:
    PdfReaderGui(QWidget* parent = nullptr);
    ~PdfReaderGui();

    void closeEvent(QCloseEvent* event);

    void createSelectBoxs();

    void initCheckBoxsAllSelect();

    void initCheckBoxsStateFromProject();

    // 设置某些项是否确认
    void setItemBrowsedState(QString item, bool is_confirm);

public slots:
    void updateAnalyzerAvaliableItemActionsSlot(std::vector<EditItemIndex> _avaliable_item_indexes);

    // 更新*即是否浏览的记录
    void updateBrowserItemSlot();

signals:
    void closeGuiSignal();

    // 选项卡映射
    void selectBoxChangedSignal(QString text, bool check_state);

private:
    PdfReader* pdf_reader_ = nullptr;
    AnalysisReport* pdf_data_ = nullptr;

    // 存储确认*以便显示与隐藏控制通过工程配置等
    std::map<QString, QLabel*> select_item_boxs_suffix_;
    std::vector<QCheckBox*> select_item_boxs_;
    QCheckBox* all_select_box_ = nullptr;
    virtual void showEvent(QShowEvent* event);

private:
    Ui::PdfReaderGui* ui;
};
#endif    // PDFREADERGUI_H
