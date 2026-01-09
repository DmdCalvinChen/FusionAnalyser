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

#include "toothfdiguidegui.h"

#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QPushButton>

#include <common/config.h>
#include <common_base/SignalManager.h>
#include <common_ext/data/fusionaligndata.h>

#include "ui_toothfdiguidegui.h"

using namespace ui_common;

ToothFdiGuideGui::ToothFdiGuideGui(const QSize& _size, QWidget* parent)
    : QWidget(parent), ui(new Ui::ToothFdiGuideGui)
{
    ui->setupUi(this);

    this->size = _size;
    setFixedSize(_size);

    label_upper_fdi = {"18",
                       "17",
                       "16",
                       "15",
                       "14",
                       "13",
                       "12",
                       "11",
                       "21",
                       "22",
                       "23",
                       "24",
                       "25",
                       "26",
                       "27",
                       "28",
                       upper_direction_name};

    label_lower_fdi = {"48",
                       "47",
                       "46",
                       "45",
                       "44",
                       "43",
                       "42",
                       "41",
                       "31",
                       "32",
                       "33",
                       "34",
                       "35",
                       "36",
                       "37",
                       "38",
                       lower_direction_name};

    setSourceButtons();

    createAnomalyButtonGui();

    createPopMenuGui();

    setDefaultToothFdiShow();

    connect(PSIGNALMANAGER, &SignalManager::setCurrentMarkingStatusSignal, this,
            &ToothFdiGuideGui::setRightMouseMenuStatusSlot);

    if (!PFusionAlignData->getAnalyserData().getIsBmu())
    {
        disabled_fdi_.insert("18");
        disabled_fdi_.insert("28");
        disabled_fdi_.insert("38");
        disabled_fdi_.insert("48");
    }
}

ToothFdiGuideGui::~ToothFdiGuideGui()
{
    delete ui;
}

void ToothFdiGuideGui::setSourceButtons()
{
    auto loadImagesSrcFun = [&](std::vector<QString>& fdi_vec, QString file_prefix)
    {
        for (auto it : fdi_vec)
        {
            buttons_src_[it] =
                    std::make_pair(file_prefix + it + ".png", file_prefix + it + "2.png");
        }
    };

    QString file_prefix_upper = ":/res/dark/images/upper_tooth_fdi/";
    QString file_prefix_lower = ":/res/dark/images/lower_tooth_fdi/";

    loadImagesSrcFun(label_upper_fdi, file_prefix_upper);
    loadImagesSrcFun(label_lower_fdi, file_prefix_lower);
}

void ToothFdiGuideGui::createAnomalyButtonGui()
{
    QGridLayout* main_layout = new QGridLayout(this);

    for (auto it : buttons_src_)
    {
        auto btn = new AnomalyButton(it.first, size, it.second.first, it.second.second, this);
        btn->setToolTip(it.first);

        if (it.first != upper_direction_name && it.first != lower_direction_name)
        {
            connect(btn, &AnomalyButton::pressedSignal, this,
                    &ToothFdiGuideGui::buttonClickedEvent);
        }

        buttons_.push_back(btn);
        main_layout->addWidget(btn, 1, 1);
    }

    main_layout->setMargin(0);
    this->setLayout(main_layout);
}

void ToothFdiGuideGui::createPopMenuGui()
{
    QAction* loss_marking = new QAction(tr("Tooth Missing Mark"));
    QAction* undo_loss_marking = new QAction(tr("Cancel Missing Mark"));
    QAction* re_marking = new QAction(tr("Clear"));
    QAction* re_marking_all = new QAction(tr("Clear All"));

    connect(loss_marking, &QAction::triggered, this, &ToothFdiGuideGui::lossToothMarkingSlot);
    connect(undo_loss_marking, &QAction::triggered, this,
            &ToothFdiGuideGui::undoLossToothMarkingSlot);
    connect(re_marking, &QAction::triggered, this, &ToothFdiGuideGui::reMarkingCurrentToothSlot);
    connect(re_marking_all, &QAction::triggered, this, &ToothFdiGuideGui::reMarkingAllToothSlot);

    pop_menu_ = new QMenu(this);
    pop_menu_->setObjectName("popMenu");
    pop_menu_->addAction(loss_marking);
    pop_menu_->addAction(undo_loss_marking);
    pop_menu_->addAction(re_marking);
    pop_menu_->addAction(re_marking_all);
}

void ToothFdiGuideGui::buttonClickedEvent()
{
    AnomalyButton* button = qobject_cast<AnomalyButton*>(sender());

    if (button == nullptr)
    {
        return;
    }

    label_working_button_ = button->objectName();

    if (button->status_ != AnomalyButton::Lost)
    {
        setButtonStatusSlot(label_working_button_, AnomalyButton::Working);
        emit setWhichToothWorkingSignal(label_working_button_);
    }

    if (!button->is_left_mouse_pressed_)
    {
        bool need_pop = true;
        if (!disabled_fdi_.empty())
        {
            auto it = disabled_fdi_.find(label_working_button_);
            if (it != disabled_fdi_.end())
            {
                need_pop = false;
            }
        }

        if (need_pop)
        {
            if (pop_menu_->isEnabled())
            {
                pop_menu_->exec(QCursor::pos());
            }
        }
    }
}

void ToothFdiGuideGui::setDefaultToothFdiShow()
{
    std::vector<std::pair<QString, bool>> default_existed_fdi;

    for (auto label : label_upper_fdi)
    {
        default_existed_fdi.push_back(std::make_pair(label, true));
    }

    setToothExistedMapSlot(default_existed_fdi);
}

void ToothFdiGuideGui::setToothExistedMapSlot(
        const std::vector<std::pair<QString, bool>>& _tooth_existed_map)
{
    if (_tooth_existed_map.empty())
    {
        return;
    }

    for (auto it : _tooth_existed_map)
    {
        setButtonStatusSlot(it.first, it.second ? AnomalyButton::General : AnomalyButton::Lost);
    }

    is_upper = _tooth_existed_map.front().first.toInt() <= 28;

    QString direction_show_image = is_upper ? upper_direction_name : lower_direction_name;
    current_direction_name = direction_show_image;
    for (auto& button : buttons_)
    {
        bool visible = false;

        if (is_marking_status_)
        {
            for (auto it : _tooth_existed_map)
            {
                if (button->objectName() == direction_show_image)
                {
                    visible = true;
                    break;
                }
            }
        }

        for (auto it : _tooth_existed_map)
        {
            if (button->objectName() == it.first)
            {
                visible = true;
                break;
            }
        }

        button->setVisible(visible);
    }
    update();
}

void ToothFdiGuideGui::setButtonStatusSlot(const QString& label,
                                           const AnomalyButton::Status& status_button)
{
    for (auto& button : buttons_)
    {
        if (status_button == AnomalyButton::Working && button->status_ == AnomalyButton::Working)
        {
            button->setStatus(AnomalyButton::General);    // 保证仅有一个按钮处于工作状态
        }

        if (button->objectName() == label)
        {
            button->setStatus(status_button);
        }
    }
}

void ToothFdiGuideGui::setWhichToothWorkingSlot(const QString& label_tooth_fdi)
{
    if (!disabled_fdi_.empty())
    {
        auto it = disabled_fdi_.find(label_tooth_fdi);
        if (it != disabled_fdi_.end())
        {
            return;
        }
    }
    label_working_button_ = label_tooth_fdi;
    setButtonStatusSlot(label_working_button_, AnomalyButton::Working);
}

void ToothFdiGuideGui::setRightMouseMenuEnabledStatusSlot(bool enbaled)
{
    pop_menu_->setEnabled(enbaled);
}

void ToothFdiGuideGui::setRightMouseMenuStatusSlot(bool is_marking_status)
{
    is_marking_status_ = is_marking_status;
    pop_menu_->clear();

    if (is_marking_status)
    {
        QAction* re_marking = new QAction(tr("Clear"));
        QAction* re_marking_all = new QAction(tr("Clear All"));

        connect(re_marking, &QAction::triggered, this,
                &ToothFdiGuideGui::reMarkingCurrentToothSlot);
        connect(re_marking_all, &QAction::triggered, this,
                &ToothFdiGuideGui::reMarkingAllToothSlot);

        pop_menu_->addAction(re_marking);
        pop_menu_->addAction(re_marking_all);

        for (auto& button : buttons_)
        {
            if (button->objectName() == current_direction_name)
            {
                button->setVisible(true);
            }
        }
    }
    else
    {
        QAction* loss_marking = new QAction(tr("Tooth Missing Mark"));
        QAction* undo_loss_marking = new QAction(tr("Cancel Missing Mark"));

        connect(loss_marking, &QAction::triggered, this, &ToothFdiGuideGui::lossToothMarkingSlot);
        connect(undo_loss_marking, &QAction::triggered, this,
                &ToothFdiGuideGui::undoLossToothMarkingSlot);

        pop_menu_->addAction(loss_marking);
        pop_menu_->addAction(undo_loss_marking);

        for (auto& button : buttons_)
        {
            if (button->objectName() == upper_direction_name ||
                button->objectName() == lower_direction_name)
            {
                button->setVisible(false);
            }
        }
    }
}

void ToothFdiGuideGui::lossToothMarkingSlot()
{
    setButtonStatusSlot(label_working_button_, AnomalyButton::Lost);
    emit setWhichToothLostSignal(label_working_button_);
}

void ToothFdiGuideGui::undoLossToothMarkingSlot()
{
    setButtonStatusSlot(label_working_button_, AnomalyButton::General);
    emit setWhichToothExistSignal(label_working_button_);
}

void ToothFdiGuideGui::reMarkingCurrentToothSlot()
{
    emit reMarkingCurrentToothSignal(label_working_button_);
}

void ToothFdiGuideGui::reMarkingAllToothSlot()
{
    emit reMarkingAllToothSignal();
}
