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

#include "fileimportdlg.h"
#include "ui_fileimportdlg.h"
#include <QFileDialog>
#include <QSettings>

#include "common_ext/data/fusionaligndata.h"

FileImportDialog::FileImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileImportDialog)
{
    ui->setupUi(this);
	this->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
	connect(ui->upperToothOpenFileBtn, SIGNAL(clicked()),
		this, SLOT(importUpperToothModelFile()));
	connect(ui->lowerToothOpenFileBtn, SIGNAL(clicked()),
		this, SLOT(importLowerToothModelFile()));
	connect(ui->cancelBtn, SIGNAL(clicked()),
		this, SLOT(cancelImportToothFile()));
	connect(ui->okBtn, SIGNAL(clicked()),
		this, SLOT(endToothModelFileChooseSend()));

}

FileImportDialog::~FileImportDialog()
{
    delete ui;
}

void FileImportDialog::importUpperToothModelFile()
{
	QSettings setting("UpperToothLastFilePath");
	QString lastPath = setting.value("UpperToothLastFilePath").toString();

	upperToothFilePathNameList.clear();
	upperToothFilePathNameList = QFileDialog::getOpenFileNames(this, tr("import the up dental"), lastPath, "*.stl");
	if (upperToothFilePathNameList.size() > 0)
	{
		ui->upDentalPathLabel->setText(upperToothFilePathNameList.first());
		setting.setValue("UpperToothLastFilePath", upperToothFilePathNameList);  //记录路径到QSetting中保存
	}
	else
	{
		ui->upDentalPathLabel->setText(tr("No files were selected"));
	}
}

void FileImportDialog::importLowerToothModelFile()
{
	lowerToothFilePathNameList.clear();
	lowerToothFilePathNameList = QFileDialog::getOpenFileNames(this, tr("import the low dental"), "", "*.stl");
	if (lowerToothFilePathNameList.size() > 0)
	{
		ui->downDentalPathLabel->setText(lowerToothFilePathNameList.first());
	}
	else
	{
		ui->downDentalPathLabel->setText(tr("No files were selected"));
	}

}

void FileImportDialog::cancelImportToothFile()
{
	ui->orderIdLabelLineEdit->setText("");
	ui->upDentalPathLabel->setText("");
	ui->downDentalPathLabel->setText("");
	upperToothFilePathNameList.clear();
	lowerToothFilePathNameList.clear();
	this->hide();
}

void FileImportDialog::endToothModelFileChooseSend()
{
	if (ui->orderIdLabelLineEdit->text().isEmpty())
	{
		UiUtilityTools::getInstance()->showInfoMessageBox(tr("info"), tr("please input the patient Id"));
		ui->orderIdLabelLineEdit->setFocus();
		QTimer::singleShot(0, ui->orderIdLabelLineEdit, &QLineEdit::selectAll);
		return;
	}
	if (ui->orderIdLabelLineEdit->text().size() > 50)
	{
		UiUtilityTools::getInstance()->showInfoMessageBox(tr("info"), tr("the order id is to long"));
		ui->orderIdLabelLineEdit->setFocus();
		QTimer::singleShot(0, ui->orderIdLabelLineEdit, &QLineEdit::selectAll);
		return;
	}
	else
	{
		QString pattern("[\\\\/:*?\"<>]");
		QRegExp rx(pattern);
		int match = ui->orderIdLabelLineEdit->text().indexOf(rx);
		if (match >= 0)
		{
			UiUtilityTools::getInstance()->showInfoMessageBox(tr("info"), tr("the order id is illegal"));
			ui->orderIdLabelLineEdit->setFocus();
			QTimer::singleShot(0, ui->orderIdLabelLineEdit, &QLineEdit::selectAll);
		}
	}
	if (upperToothFilePathNameList.size() <= 0)
	{
		ui->upDentalPathLabel->setText(tr("no files were selected!"));
	}
	if (lowerToothFilePathNameList.size() <= 0)
	{
		ui->downDentalPathLabel->setText(tr("no files were selected!"));
	}
	if (upperToothFilePathNameList.size() <= 0 && lowerToothFilePathNameList.size() <= 0)
	{
		return;
	}
	patient_name_ = ui->orderIdLabelLineEdit->text();

	ui->orderIdLabelLineEdit->setText("");
	ui->upDentalPathLabel->setText(tr("Please select the up dental."));
	ui->downDentalPathLabel->setText(tr("Please select the down dental."));
	this->accept();
}

void FileImportDialog::orderIdEdit(const QString &)
{
	ui->orderIdLabelLineEdit->setText("");
}

QString FileImportDialog::getOrderIdInfomation()
{
	return patient_name_;
}
