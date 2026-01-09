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

#include "fusionViewGui.h"

#include <assert.h>

#include <QFile>
#include <QPainter>
#include <QSignalMapper>

#include <common_base/SignalManager.h>

#include "common_base/util/uitools.h"
#include "common_ext/data/fusionaligndata.h"
#include "uicommon.h"
#include "ui_fusionViewGui.h"

using namespace ui_common;
#define FUSIONVIEWGUIHIGHT 30
fusionViewGui::fusionViewGui(QWidget* pGLa, QWidget* parent)
    : QDialog(parent), ui(new Ui::fusionViewGui)
{
    this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    this->setMinimumHeight(FUSIONVIEWGUIHIGHT);
    ui->setupUi(this);
    this->setMouseTracking(true);

    p_gla_ = pGLa;
    this->ui->horizontalLayout->setContentsMargins(0, 0, 0, 0);
    connect(PSIGNALMANAGER, &SignalManager::setSelectDentalConsoleVisibleSignal, this,
            &fusionViewGui::setSelectDentalConsoleVisible);
    connect(PSIGNALMANAGER, &SignalManager::setViewDirectConsoleVisibleSignal, this,
            &fusionViewGui::setViewDirectConsoleVisible);
    connect(PSIGNALMANAGER, &SignalManager::setDentalVisibleConsoleVisibleSignal, this,
            &fusionViewGui::setDentalVisibleConsoleVisible);
    connect(PSIGNALMANAGER, &SignalManager::setLMRViewDirectConsoleVisibleSignal, this,
            &fusionViewGui::setLMRViewDirectConsoleVisible);
    connect(PSIGNALMANAGER, &SignalManager::setUpperDentalSelectedSignal, this,
            &fusionViewGui::setUpperDentalSelectedSlot);
    connect(PSIGNALMANAGER, &SignalManager::setLowerDentalSelectedSignal, this,
            &fusionViewGui::setLowerDentalSelectedSlot);
    connect(PSIGNALMANAGER, &SignalManager::onlyShowUpperDentalSignal, this,
            &fusionViewGui::onlyShowUpperDentalSlot);
    connect(PSIGNALMANAGER, &SignalManager::onlyShowLowerDentalSignal, this,
            &fusionViewGui::onlyShowLowerDentalSlot);
    initBtn();
}

void ui_common::fusionViewGui::initBtn()
{
    p_signal_mapper_ = new QSignalMapper(this);
    ui->upDentalBtn->setCheckable(true);
    ui->downDentalBtn->setCheckable(true);
    ui->selectDownDentalBtn->setCheckable(true);
    ui->selectUpDentalBtn->setCheckable(true);
    QSignalMapper* p_select_signal_mapper = new QSignalMapper(this);

    connect(ui->upDentalBtn, &QPushButton::clicked, p_signal_mapper_,
            static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    connect(ui->upViewBtn, &QPushButton::clicked, p_signal_mapper_,
            static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    connect(ui->rightViewBtn, &QPushButton::clicked, p_signal_mapper_,
            static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    connect(ui->frontViewBtn, &QPushButton::clicked, p_signal_mapper_,
            static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    connect(ui->leftViewBtn, &QPushButton::clicked, p_signal_mapper_,
            static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    connect(ui->bottomViewBtn, &QPushButton::clicked, p_signal_mapper_,
            static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    connect(ui->downDentalBtn, &QPushButton::clicked, p_signal_mapper_,
            static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    connect(ui->selectDownDentalBtn, &QPushButton::released, p_select_signal_mapper,
            static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    connect(ui->selectUpDentalBtn, &QPushButton::released, p_select_signal_mapper,
            static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));

    p_signal_mapper_->setMapping(ui->upDentalBtn, E_UP_DENTAL_VIEW);
    p_signal_mapper_->setMapping(ui->upViewBtn, E_BOTTOM_VIEW);
    p_signal_mapper_->setMapping(ui->rightViewBtn, E_RIGHT_VIEW);
    p_signal_mapper_->setMapping(ui->frontViewBtn, E_FRONT_VIEW);
    p_signal_mapper_->setMapping(ui->leftViewBtn, E_LEFT_VIEW);
    p_signal_mapper_->setMapping(ui->bottomViewBtn, E_TOP_VIEW);
    p_signal_mapper_->setMapping(ui->downDentalBtn, E_DOWN_DENTAL_VIEW);

    p_select_signal_mapper->setMapping(ui->selectUpDentalBtn, E_SELECT_UP_DENTAL);
    p_select_signal_mapper->setMapping(ui->selectDownDentalBtn, E_SELECT_DOWN_DENTAL);

    connect(p_signal_mapper_, SIGNAL(mapped(int)), this, SIGNAL(setViewDirSignal(int)));
    connect(p_select_signal_mapper, SIGNAL(mapped(int)), this,
            SLOT(onSelectDentalBtnClickedSlot(int)));
    setViewDirectConsoleVisible(false);

    connect(ui->upDentalBtn, &QPushButton::clicked, this,
            []() { emit PSIGNALMANAGER->setUpperOrLowerToothDataShowSignal(true); });
    connect(ui->downDentalBtn, &QPushButton::clicked, this,
            []() { emit PSIGNALMANAGER->setUpperOrLowerToothDataShowSignal(false); });

    if (!PFusionAlignData->getAnalyserData().getIsBmu())
    {
        ui->selectUpDentalBtn->setVisible(false);
        ui->selectDownDentalBtn->setVisible(false);
    }
}

fusionViewGui::~fusionViewGui()
{
    delete ui;
}

QPushButton* fusionViewGui::getUpDentalBtn() const
{
    return ui->upDentalBtn;
}
QPushButton* fusionViewGui::getDownDentalBtn() const
{
    return ui->downDentalBtn;
}

void ui_common::fusionViewGui::setDentalBtnState(eViewDirection viewDirection)
{
    bool state = true;
    if (viewDirection == E_UP_DENTAL_VIEW)
    {
        b_up_dental_check_state_ = !b_up_dental_check_state_;
        ui->upDentalBtn->setChecked(b_up_dental_check_state_);
        b_down_dental_check_state_ = false;
        ui->downDentalBtn->setChecked(b_down_dental_check_state_);
    }
    else if (viewDirection == E_DOWN_DENTAL_VIEW)
    {
        b_down_dental_check_state_ = !b_down_dental_check_state_;
        ui->downDentalBtn->setChecked(b_down_dental_check_state_);
        b_up_dental_check_state_ = false;
        ui->upDentalBtn->setChecked(b_up_dental_check_state_);
    }
    else
    {
        b_up_dental_check_state_ = false;
        ui->upDentalBtn->setChecked(b_up_dental_check_state_);
        b_down_dental_check_state_ = false;
        ui->downDentalBtn->setChecked(b_down_dental_check_state_);
    }
}

void ui_common::fusionViewGui::setDentalBtnState(bool isUp, bool isCheck)
{
    if (isUp)
    {
        b_up_dental_check_state_ = isCheck;
        ui->upDentalBtn->setChecked(b_up_dental_check_state_);
    }
    else
    {
        b_down_dental_check_state_ = isCheck;
        ui->downDentalBtn->setChecked(b_down_dental_check_state_);
    }
}

void ui_common::fusionViewGui::onSelectDentalBtnClickedSlot(int dentalType)
{
    ui->selectUpDentalBtn->setChecked(dentalType == E_SELECT_UP_DENTAL);
    ui->selectDownDentalBtn->setChecked(dentalType == E_SELECT_DOWN_DENTAL);
    if (dentalType == E_SELECT_UP_DENTAL && ui->downDentalBtn->isChecked())
    {
        emit setViewDirSignal(E_DOWN_DENTAL_VIEW);
    }
    else if (dentalType == E_SELECT_DOWN_DENTAL && ui->upDentalBtn->isChecked())
    {
        emit setViewDirSignal(E_UP_DENTAL_VIEW);
    }
    emit setSelectDentalSignal(dentalType);
}

void ui_common::fusionViewGui::setUpperDentalSelectedSlot()
{
    onSelectDentalBtnClickedSlot(E_SELECT_UP_DENTAL);
}

void ui_common::fusionViewGui::setLowerDentalSelectedSlot()
{
    onSelectDentalBtnClickedSlot(E_SELECT_DOWN_DENTAL);
}

void ui_common::fusionViewGui::onlyShowUpperDentalSlot(bool _sure)
{
    if ((_sure && !ui->upDentalBtn->isChecked()) || (!_sure && ui->upDentalBtn->isChecked()))
    {
        emit setViewDirSignal(E_UP_DENTAL_VIEW);
    }
}

void ui_common::fusionViewGui::onlyShowLowerDentalSlot(bool _sure)
{
    if ((_sure && !ui->downDentalBtn->isChecked()) || (!_sure && ui->downDentalBtn->isChecked()))
    {
        emit setViewDirSignal(E_DOWN_DENTAL_VIEW);
    }
}

void ui_common::fusionViewGui::setViewDirectConsoleVisible(bool _visible)
{
    ui->upViewBtn->setVisible(_visible);
    ui->rightViewBtn->setVisible(_visible);
    ui->frontViewBtn->setVisible(_visible);
    ui->leftViewBtn->setVisible(_visible);
    ui->bottomViewBtn->setVisible(_visible);
}

void ui_common::fusionViewGui::setSelectDentalConsoleVisible(bool _visible)
{
    ui->selectDownDentalBtn->setVisible(_visible);
    ui->selectUpDentalBtn->setVisible(_visible);
    setDentalVisibleConsoleVisible(_visible);
}

void ui_common::fusionViewGui::setDentalVisibleConsoleVisible(bool _visible)
{
    ui->upDentalBtn->setVisible(_visible);
    ui->downDentalBtn->setVisible(_visible);
}

void ui_common::fusionViewGui::setLMRViewDirectConsoleVisible(bool _visible)
{
    setViewDirectConsoleVisible(_visible);
    setDentalVisibleConsoleVisible(_visible);
}

void ui_common::fusionViewGui::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(15, 120, 80, 1));
}

void ui_common::fusionViewGui::resizeEvent(QResizeEvent* event)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(15, 120, 80, 1));
}
