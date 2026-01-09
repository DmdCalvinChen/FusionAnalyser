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

#include "workDirSetingDlg.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QTextCodec>

#include "common/mlapplication.h"
#include "common_base/util/uitools.h"
#include "ui_workDirSetingDlg.h"

WorkDirSetingDlg::WorkDirSetingDlg(QWidget* parent) : QDialog(parent), ui(new Ui::WorkDirSetingDlg)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |
                         Qt::WindowCloseButtonHint);
    QSettings settings;
    if (!settings.value("FusionAnalyser WorkSpace").isValid())
    {
        ui->lineStoragePath->setText(QCoreApplication::applicationDirPath());
    }
    else
    {
        ui->lineStoragePath->setText(settings.value("FusionAnalyser WorkSpace").toString());
    }
    bool isValid = settings.value("FusionAnalyser WorkSpace").isValid();
    connect(ui->chooseBtn, &QPushButton::clicked, this, &WorkDirSetingDlg::onShowFileSlot);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &WorkDirSetingDlg::onCancelClickSlot);
    connect(ui->okBtn, &QPushButton::clicked,
            [&]
            {
                if (isDirExist(ui->lineStoragePath->text()))
                {
                    QSettings settings;
                    settings.setValue("FusionAnalyser WorkSpace", ui->lineStoragePath->text());
                    this->accept();
                }
                else
                {
                    UiUtilityTools::getInstance()->showInfoMessageBox(tr("error"),
                                                                   tr("The path is incorrect"));
                    return;
                }
            });
}

WorkDirSetingDlg::~WorkDirSetingDlg()
{
    delete ui;
}

void WorkDirSetingDlg::showEvent(QShowEvent* event)
{
    QSettings settings;
    if (!settings.value("FusionAnalyser WorkSpace").isValid())
    {
        ui->lineStoragePath->setText(QCoreApplication::applicationDirPath());
    }
    else
    {
        ui->lineStoragePath->setText(settings.value("FusionAnalyser WorkSpace").toString());
    }
    return QDialog::showEvent(event);
}

void WorkDirSetingDlg::onShowFileSlot()
{
    QString filePath = QFileDialog::getExistingDirectory(
            this, tr("Select local data storage path"), "");
    filePath.toLocal8Bit().constData();
    if (filePath.isEmpty())
    {
        return;
    }
    else
    {
        ui->lineStoragePath->setText(filePath);
    }
    return;
}

void WorkDirSetingDlg::onCancelClickSlot()
{
    QSettings settings;
    if (!settings.value("FusionAnalyser WorkSpace").isValid())
    {
        UiUtilityTools::getInstance()->showInfoMessageBox(tr("error"),
                                                       tr("please choose the work dir"));
        return;
    }
    else
        this->close();
}

bool WorkDirSetingDlg::isDirExist(QString fullPath)
{
    if (fullPath == "")
        return false;
    QDir dir(fullPath);
    if (dir.exists())
    {
        return true;
    }
    else
    {
        return false;
    }
}
