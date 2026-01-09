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

#ifndef ANOMALYBUTTON_H
#define ANOMALYBUTTON_H

#include <QMouseEvent>
#include <QPushButton>

/*
 *
 *	管理三种状态的按钮，对应牙齿导航图中牙位的不同三种状态，
 *   可读取设置状态以及更新控件。2020/7/31
 *
 */

class AnomalyButton : public QPushButton
{
    Q_OBJECT
public:
    // 多状态按钮
    // @param 名字标签
    // @param 普通状态时的按钮图像资源路径
    // @param 工作状态时的按钮图像资源路径
    // @param 父窗口 default
    AnomalyButton(const QString& label, const QSize& _size_control,
                  const QString& _general_image_file_path, const QString& _working_image_file_path,
                  QWidget* parent = nullptr);

    ~AnomalyButton()
    {
    }

    // 按钮处于的状态
    enum Status
    {
        General,    // 普通状态
        Lost,       // 丢失状态
        Working     // 工作状态
    };
    // 设置按钮的状态
    // @param 状态
    void setStatus(Status _status);

    // 设置按钮的大小
    // @param 尺寸
    void setSize(const QSize& size);

private:
    void setLabel(const QString& label);
    void setImage(const QString& image_file_path);
    void mousePressEvent(QMouseEvent* e);
    void updateGuiWhenStatusChanging();

public:
    QSize size_control_;    // 控件大小
    QString general_image_file_path_, lost_image_file_path_, working_image_file_path_;
    bool is_left_mouse_pressed_ = false;

    Status status_ = General;

signals:
    void pressedSignal();
};

#endif
