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

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>

#include "toothadjustdock.h"
#include "ui_toothadjustdock.h"

ToothAdjustDock::ToothAdjustDock(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ToothAdjustDock)
{
    ui->setupUi(this);
	title_Label = new QLabel("Tooth Adjust", this);
	ui->gridLayout->addWidget(title_Label, 0, 0);

	x_Translate_Label = new QLabel("X: ", this);
	y_Translate_Label = new QLabel("Y: ", this);
	z_Translate_Label = new QLabel("Z: ", this);
	a_Rotate_Label = new QLabel("A: ", this);
	b_Rotate_Label = new QLabel("B: ", this);
	g_Rotate_Label = new QLabel("G: ", this);

	x_Translate_Edit = new QTextEdit("0", this);
	y_Translate_Edit = new QTextEdit("0", this);
	z_Translate_Edit = new QTextEdit("0", this);
	a_Rotate_Edit = new QTextEdit("0", this);
	b_Rotate_Edit = new QTextEdit("0", this);
	g_Rotate_Edit = new QTextEdit("0", this);

	ui->gridLayout->setAlignment(Qt::AlignCenter);
	ui->gridLayout->addWidget(x_Translate_Label, 2, 0); ui->gridLayout->addWidget(x_Translate_Edit, 2, 1);
	ui->gridLayout->addWidget(y_Translate_Label, 3, 0); ui->gridLayout->addWidget(y_Translate_Edit, 3, 1);
	ui->gridLayout->addWidget(z_Translate_Label, 4, 0); ui->gridLayout->addWidget(z_Translate_Edit, 4, 1);
	ui->gridLayout->addWidget(a_Rotate_Label, 5, 0);    ui->gridLayout->addWidget(a_Rotate_Edit, 5, 1);
	ui->gridLayout->addWidget(b_Rotate_Label, 6, 0);    ui->gridLayout->addWidget(b_Rotate_Edit, 6, 1);
	ui->gridLayout->addWidget(g_Rotate_Label, 7, 0);    ui->gridLayout->addWidget(g_Rotate_Edit, 7, 1);

	translate_Mode_Btn = new QPushButton("T", this);
	rotate_Mode_Btn = new QPushButton("R", this);
	ui->gridLayout->addWidget(translate_Mode_Btn, 8, 0);
	ui->gridLayout->addWidget(rotate_Mode_Btn, 8, 1);

	this->x_Value = 0;
	this->y_Value = 0;
	this->z_Value = 0;
	this->a_Value = 0;
	this->b_Value = 0;
	this->g_Value = 0;

}

ToothAdjustDock::~ToothAdjustDock()
{
    delete ui;
}

void ToothAdjustDock::updateToothHexValue()
{
	//this->x = this->x_Translate_Edit->
}
