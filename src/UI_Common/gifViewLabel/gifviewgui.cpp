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

#include "gifviewgui.h"

#include <QDir>
#include <QSettings>
#include <QString>

#include <common_base/SignalManager.h>

#include "ui_gifviewgui.h"
Gifviewgui::Gifviewgui(bool showBtn, QWidget* parent) : QDialog(parent), ui(new Ui::Gifviewgui)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    connect(ui->hideButton, &QPushButton::clicked, this, &Gifviewgui::hideWidget);
    this->setAttribute(Qt::WA_StyledBackground, true);

    this->setStyleSheet(
            "QWidget{background-color:rgb(26,40,53);} QPushButton{background-color:#36404C; color:white; border-radius: 8px; font: 10pt; border:1px solid #b6c4d1; }\
                                  .QPushButton:pressed{background-color:#344A55; border:1px solid #a3d3d3;}\
									.QPushButton:hover{background-color:#43606B;border:1px solid #a3d3d3;}");
    ui->hideButton->setText(tr("Not tips"));
    showButton(showBtn);
}

Gifviewgui::~Gifviewgui()
{
    delete ui;
}

void Gifviewgui::showButton(bool showBtn)
{
    if (!showBtn)
    {
        ui->hideButton->hide();
    }
}
void Gifviewgui::hideWidget()
{
    QSettings settings(QCoreApplication::applicationDirPath() + "./configs/config.ini",
                       QSettings::IniFormat);
    if (gifType == "misstoothgif")
    {
        settings.setValue("misstoothgif", 1);
    }
    if (gifType == "fixtoothgif")
    {
        settings.setValue("fixtoothgif", 1);
    }
    if (gifType == "overlaygif")
    {
        settings.setValue("overlaygif", 1);
    }
    if (gifType == "teethwidthgif")
    {
        settings.setValue("teethwidthgif", 1);
    }
    ui->hideButton->hide();
    this->hide();
}

void Gifviewgui::showGif(QList<std::pair<QString, QString>> gifPath)
{
    this->gifPath = gifPath;
    ui->Giflabel->addImage(gifPath);
}
