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

#include "importmodelgui.h"

#include <QFileDialog>
#include <QPushButton>

#include "ui_importmodelgui.h"

ImportModelGui::ImportModelGui(QWidget* parent) : QDialog(parent), ui(new Ui::ImportModelGui)
{
    ui->setupUi(this);

    connect(ui->upCTLoadBtn, &QPushButton::clicked, this, &ImportModelGui::loadCtModel);
    connect(ui->downCTLoadBtn, &QPushButton::clicked, this, &ImportModelGui::loadCtModel);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &ImportModelGui::hide);
    connect(ui->importbtn, &QPushButton::clicked, this, &ImportModelGui::hide);
}

ImportModelGui::~ImportModelGui()
{
    delete ui;
}

void ImportModelGui::loadCtModel()
{
    QString path_file =
            QFileDialog::getOpenFileName(this, tr("open CT model"), ".", tr("models files(*.stl)"));

    QPushButton* btn = qobject_cast<QPushButton*>(sender());

    bool isUpCtLoad = true;

    if (btn)
    {
        isUpCtLoad = btn == ui->upCTLoadBtn;
    }
    else
    {
        return;
    }

    if (!path_file.isEmpty())
    {
        btn->setText(tr("selected"));
        isUpCtLoad ? up_ct_file_path_ = path_file : down_ct_file_path_ = path_file;
    }
}

void ImportModelGui::hide()
{
    ui->upCTLoadBtn->setText(tr("select file"));
    ui->downCTLoadBtn->setText(tr("select file"));
    this->setVisible(false);
}
