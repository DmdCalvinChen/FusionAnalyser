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

#include "overlapoverbitegui.h"
#include "ui_overlapoverbitegui.h"
#include <QButtonGroup>
#include <wrap/qt/device_to_logical.h>

OverlapOverbiteGui::OverlapOverbiteGui(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OverlapOverbiteGui)
{
    ui->setupUi(this);
	this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
	//this->setWidget(ui->frame);
	//this->setTitleBarWidget(new QWidget);

	//setFixedWidth(getWidthWindow());

	//ui->line->setStyleSheet(getSpilterLineStyle());
	////ui->line_2->setStyleSheet(getSpilterLineStyle());

	//QPalette colorMainTitle = getColorMainTitle();
	//ui->mainTitleName->setPalette(colorMainTitle);

	overlayOutlinePreview = new OverlayOutlinePreview(parent);

	int numRowCurrent = 1;
	int numRowUse = 10;

	//ui->gridLayout->addWidget(overlayOutlinePreview, 0, 0, numRowUse, 1);
    QVBoxLayout *vBoxlayout = new QVBoxLayout;
    vBoxlayout->addWidget(overlayOutlinePreview);
    vBoxlayout->setContentsMargins(0, 0, 0, 0);
    ui->overlayOverviewWidget->setLayout(vBoxlayout);

	QButtonGroup* qButtonGroup = new QButtonGroup;
	qButtonGroup->setExclusive(true);
	qButtonGroup->addButton(ui->oneBtn);
	ui->oneBtn->setChecked(true);
	ui->oneBtn->setCheckable(true);
	ui->oneBtn->setAutoExclusive(true);
	ui->twoBtn->setCheckable(true);
	ui->twoBtn->setAutoExclusive(true);
	qButtonGroup->addButton(ui->twoBtn);
	ui->oneBtn->setStyleSheet("QPushButton:pressed {border-image:url(:/res/default/images/11_selected.svg);}");
	connect(ui->closeBtn, &QPushButton::clicked,
		this, &OverlapOverbiteGui::wellDoneSlot);
	connect(ui->pathSlider, &QSlider::valueChanged,
		this, &OverlapOverbiteGui::updateStepOverlaySliderLabelSlot);
	connect(ui->pathSlider, &QSlider::sliderReleased,
		this, &OverlapOverbiteGui::updateCutOutlineSignal);
	connect(ui->oneBtn, &QPushButton::clicked,
		this, &OverlapOverbiteGui::switchToRightSide);
	connect(ui->twoBtn, &QPushButton::clicked,
		this, &OverlapOverbiteGui::switchToLeftSide);
}

OverlapOverbiteGui::~OverlapOverbiteGui()
{
    delete ui;
}

void OverlapOverbiteGui::wellDoneSlot()
{
	emit doneSignal();
	initControlDefaultValueSlot();
}

void OverlapOverbiteGui::initControlDefaultValueSlot()
{
	ui->pathSlider->setValue(500);
	float distance = ui->pathSlider->value() * 1.0f / ui->pathSlider->maximum();
}

void OverlapOverbiteGui::updateStepOverlaySliderLabelSlot(int step)
{
	float distance = ui->pathSlider->value() * 1.0f / ui->pathSlider->maximum();
	emit setStepOverlaySendSignal(distance);
}

void OverlapOverbiteGui::resizeEvent(QResizeEvent *_event)
{
	if (overlayOutlinePreview == nullptr)
	{
		return;
	}

}

void OverlapOverbiteGui::showEvent(QShowEvent *event)
{
	//int width = QTLogicalToDevice(ui->overlayOverviewWidget, ui->overlayOverviewWidget->width());
	int width = QTLogicalToDevice(this, this->width());
    int height = QTLogicalToDevice(ui->overlayOverviewWidget, ui->overlayOverviewWidget->height());
    //overlayOutlinePreview->setFixedSize(ui->overlayOverviewWidget->width(), ui->overlayOverviewWidget->height());
    //overlayOutlinePreview->resizeGL(ui->overlayOverviewWidget->width(), ui->overlayOverviewWidget->height());
    //overlayOutlinePreview->setFixedSize(width,height);

    overlayOutlinePreview->resizeGL(width, height);
}

void OverlapOverbiteGui::switchToRightSide()
{
	if (overlayOutlinePreview)
	{
		ui->oneBtn->setChecked(true);
		overlayOutlinePreview->setCheekSide(OverlayOutlinePreview::RIGHT_SIDE);
	}
}

void OverlapOverbiteGui::switchToLeftSide()
{
	if (overlayOutlinePreview)
	{
		ui->twoBtn->setChecked(true);
		overlayOutlinePreview->setCheekSide(OverlayOutlinePreview::LEFT_SIDE);
	}
}

