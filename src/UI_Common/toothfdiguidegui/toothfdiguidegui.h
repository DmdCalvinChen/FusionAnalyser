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

#ifndef TOOTHFDIGUIDEGUI_H
#define TOOTHFDIGUIDEGUI_H

#include <QWidget>
#include <set>

#include "anomalybutton.h"
#include "uicommon_global.h"

class QMenu;

namespace Ui
{
class ToothFdiGuideGui;
}

/*
 *
 *	对应牙齿导航图中牙位的不同三种状态管理。2020/7/31
 *
 */
namespace ui_common
{
class UI_COMMONSHARED_EXPORT ToothFdiGuideGui : public QWidget
{
    Q_OBJECT

public:
    explicit ToothFdiGuideGui(const QSize& _size, QWidget* parent = nullptr);
    ~ToothFdiGuideGui();

public:
    bool currentMeshIsUpper() const
    {
        return is_upper;
    }

private:
    Ui::ToothFdiGuideGui* ui;
    QSize size;
    QString upper_direction_name = "upper_direction";
    QString lower_direction_name = "lower_direction";
    std::vector<QString> label_upper_fdi;
    std::vector<QString> label_lower_fdi;
    // 抽象按钮相关的资源文件名以及路径 <fdi,<general-image,working-image>>
    std::map<QString, std::pair<QString, QString>> buttons_src_;
    std::list<AnomalyButton*> buttons_;    // 实体按钮
    QString label_working_button_;
    QMenu* pop_menu_ = nullptr;
    // 禁用的fdi操作
    std::set<QString> disabled_fdi_;
    // 当前是标记状态还是缺失确认状态
    bool is_marking_status_ = false;

    // 当前是上颌还是下颌的方向图
    QString current_direction_name;

    bool is_upper = true;

private:
    void setSourceButtons();    // 上下颌所有牙位对应的按钮资源路径及标签初始化
    void createAnomalyButtonGui();
    void createPopMenuGui();
    void buttonClickedEvent();
    void setDefaultToothFdiShow();

    void setButtonStatusSlot(const QString& label, const AnomalyButton::Status& status_button);
    void lossToothMarkingSlot();
    void reMarkingCurrentToothSlot();
    void reMarkingAllToothSlot();
    void undoLossToothMarkingSlot();

public slots:
    // 控制显示上下颌相关的按钮以及按钮代表的牙齿是否存在,映射表，只能是上颌或下颌
    // @param <牙位, 是否存在>
    void setToothExistedMapSlot(const std::vector<std::pair<QString, bool>>& _tooth_existed_map);
    // 当前正在操作的牙位，槽
    // @param 牙位
    void setWhichToothWorkingSlot(const QString& label_tooth_fdi);
    // 右键菜单弹起功能是否启用
    // @param 启用状态
    void setRightMouseMenuEnabledStatusSlot(bool enbaled);

    // 右键菜单项的不同选项根据程序设置状态
    void setRightMouseMenuStatusSlot(bool is_marking_status);

signals:
    // 当前正在操作的牙位，信号
    // @param 牙位
    void setWhichToothWorkingSignal(const QString& label_tooth_fdi);
    void setWhichToothLostSignal(const QString& label_tooth_fdi);
    void setWhichToothExistSignal(const QString& label_tooth_fdi);    // 撤销某颗牙位丢失状态
    void reMarkingCurrentToothSignal(const QString& label_tooth_fdi);
    void reMarkingAllToothSignal();
};
}    // namespace ui_common
#endif    // TOOTHFDIGUIDEGUI_H
