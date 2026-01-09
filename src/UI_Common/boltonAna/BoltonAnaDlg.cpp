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

#include "BoltonAnaDlg.h"

#include <assert.h>

#include <QChar>
#include <QTableWidgetItem>

#include "common_ext/bolton/BoltonAna.h"
#include "ui_BoltonAnaDlg.h"

using namespace common_ext;

#define TABLEHEIGHT 240
#define TABLEWIDTH 150
#define WIDTH 330
#define HEIGHT
using namespace ui_common;
BoltonAnaDlg::BoltonAnaDlg(QWidget* parent) : QWidget(parent)
{
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    ui = new Ui::BoltonAnaDlg();
    ui->setupUi(this);

    setupTableWidget();
}

BoltonAnaDlg::BoltonAnaDlg(BoltonAna* pBoltonAna, QWidget* parent /*= Q_NULLPTR*/) : QWidget(parent)
{
    assert(pBoltonAna);
    this->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |
                         Qt::WindowCloseButtonHint);
    this->setWindowIcon(parent->windowIcon());
    ui = new Ui::BoltonAnaDlg();
    ui->setupUi(this);
    setupTableWidget();
    updateUI(pBoltonAna);
}

void BoltonAnaDlg::updateBoltonRatio()
{
    ui->bolton33NumLabel->setText(p_bolton_ana_->getBolton_33());
    ui->bolton66NumLabel->setText(p_bolton_ana_->getBolton_66());

    ui->discrepancy33infoLabel->setText(p_bolton_ana_->getDiscrepancy_33_info());
    ui->discrepancy66InfoLabel->setText(p_bolton_ana_->getDiscrepancy_66_info());

    ui->discrepancy33NumLabel->setText(p_bolton_ana_->getDiscrepancy_33_num());
    ui->discrepancy66NumLabel->setText(p_bolton_ana_->getDiscrepancy_66_num());
}

void BoltonAnaDlg::updateUpperToothWidth()
{
    const std::map<QString, QString>& upperToothWidth = p_bolton_ana_->getUpper_tooth_width_data();
    int index = 0;
    for (auto toothWidthInfo : upperToothWidth)
    {
        if (toothWidthInfo.first != "")
        {
            bool bSusscess = false;
            assert(toothWidthInfo.first.size() == 2);
            QString sIndex = toothWidthInfo.first.at(1);
            index = sIndex.toInt(&bSusscess, 10) - 1;
            assert(index >= 0);

            if (toothWidthInfo.first.at(0) == QChar('1'))
            {
                ui->upperRightTableWidget->setItem(index, 0,
                                                   new QTableWidgetItem(toothWidthInfo.first));
                ui->upperRightTableWidget->setItem(index, 1,
                                                   new QTableWidgetItem(toothWidthInfo.second));
            }

            if (toothWidthInfo.first.at(0) == QChar('2'))
            {
                ui->upperLeftTableWidget->setItem(index, 1,
                                                  new QTableWidgetItem(toothWidthInfo.first));
                ui->upperLeftTableWidget->setItem(index, 0,
                                                  new QTableWidgetItem(toothWidthInfo.second));
            }
        }
    }
}

void BoltonAnaDlg::updateLowerToothWidth()
{
    const std::map<QString, QString>& upperToothWidth = p_bolton_ana_->getLower_tooth_width_data();
    int index = 0;
    for (auto toothWidthInfo : upperToothWidth)
    {
        if (toothWidthInfo.first != "")
        {
            bool bSusscess = false;
            assert(toothWidthInfo.first.size() == 2);
            QString sIndex = toothWidthInfo.first.at(1);
            index = sIndex.toInt(&bSusscess, 10) - 1;
            assert(index >= 0);

            if (toothWidthInfo.first.at(0) == QChar('4'))
            {
                ui->lowerRightTableWidget->setItem(index, 0,
                                                   new QTableWidgetItem(toothWidthInfo.first));
                ui->lowerRightTableWidget->setItem(index, 1,
                                                   new QTableWidgetItem(toothWidthInfo.second));
            }

            if (toothWidthInfo.first.at(0) == QChar('3'))
            {
                ui->lowerLeftTableWidget->setItem(index, 1,
                                                  new QTableWidgetItem(toothWidthInfo.first));
                ui->lowerLeftTableWidget->setItem(index, 0,
                                                  new QTableWidgetItem(toothWidthInfo.second));
            }
        }
    }
}

BoltonAnaDlg::~BoltonAnaDlg()
{
    delete ui;
}

void BoltonAnaDlg::updateUI(BoltonAna* pBoltonAna)
{
    if (pBoltonAna == nullptr || pBoltonAna == p_bolton_ana_)
    {
        return;
    }

    if (p_bolton_ana_ != pBoltonAna)
    {
        p_bolton_ana_ = pBoltonAna;
        updateBoltonRatio();
        updateToothWidth();
    }
}
#include <QScrollBar>

void BoltonAnaDlg::setupTableWidget()
{
    auto setTableView = [](QTableWidget* tableWidget)
    {
        tableWidget->verticalHeader()->setHidden(true);
        tableWidget->horizontalHeader()->setHidden(true);
        tableWidget->horizontalHeader()->sectionResizeMode(QHeaderView::Stretch);
        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidget->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
        tableWidget->setVisible(true);
        tableWidget->setColumnCount(2);
        tableWidget->setRowCount(8);
        tableWidget->setFixedHeight(TABLEHEIGHT);
        tableWidget->setFixedWidth(TABLEWIDTH);
        tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        tableWidget->setFocusPolicy(Qt::NoFocus);
        tableWidget->setShowGrid(false);
        tableWidget->setFrameShape(QFrame::NoFrame);
    };

    setTableView(ui->upperRightTableWidget);
    setTableView(ui->upperLeftTableWidget);
    setTableView(ui->lowerRightTableWidget);
    setTableView(ui->lowerLeftTableWidget);
}

void BoltonAnaDlg::paintEvent(QPaintEvent* event)
{
}

void ui_common::BoltonAnaDlg::updateToothWidth()
{
    QColor itemColor(240, 239, 253);
    auto setTableWidgetStyle = [=](QTableWidget* widget)
    {
        for (int idex = 0; idex < 8; ++idex)
        {
            widget->item(idex, 0)->setTextAlignment(Qt::AlignLeft);
            widget->item(idex, 1)->setTextAlignment(Qt::AlignRight);
            if (idex % 2 != 0)
            {
                widget->item(idex, 0)->setBackgroundColor(itemColor);
                widget->item(idex, 1)->setBackgroundColor(itemColor);
            }
        }
    };
    updateUpperToothWidth();
    updateLowerToothWidth();

    setTableWidgetStyle(ui->upperLeftTableWidget);
    setTableWidgetStyle(ui->upperRightTableWidget);
    setTableWidgetStyle(ui->lowerLeftTableWidget);
    setTableWidgetStyle(ui->lowerRightTableWidget);
}

void ui_common::BoltonAnaDlg::closeEvent(QCloseEvent* event)
{
    emit closedSignal();

    QWidget::closeEvent(event);
}
