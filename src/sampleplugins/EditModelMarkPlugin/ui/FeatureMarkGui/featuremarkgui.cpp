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

#include "featuremarkgui.h"
#include "./ui_featuremarkgui.h"
#include <QDebug>
#include<QScreen>
#include <common_base/SignalManager.h>
#include <UI_Common/toothfdiguidegui/toothfdiguidegui.h>
FeatureMarkGui::FeatureMarkGui(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FeatureMarkGui)
{
    ui->setupUi(this);

	this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect mm = screen->availableGeometry();
	int screen_width = mm.width();
	int screen_height = mm.height();
	qreal dpiVal = screen->logicalDotsPerInch();
	qDebug() << dpiVal;
	tooth_fdi_preview_ui_ = new ui_common::ToothFdiGuideGui(QSize(350, 260));
	ui->verticalLayout->addWidget(tooth_fdi_preview_ui_);
	ui->doneBtn->hide();
	connect(PSIGNALMANAGER, &SignalManager::setCurrentMarkingStatusSignal, this, &FeatureMarkGui::setTextSlot);
	ui->titleLabel->setText(tr("Please confirm the number of teeth"));
	ui->textLabel->setText(tr("Confirm that the number of teeth is consistent with the actual situation of the model. Select the tooth bitmap and right-click to mark the missing tooth position."));
	this->setAutoFillBackground(true);
	this->setAttribute(Qt::WA_StyledBackground, true);
	this->setContentsMargins(0, 0, 0, 0);
	connect(ui->doneBtn, &QPushButton::clicked, this, [&]() {emit doneSignal(); });
}

FeatureMarkGui::~FeatureMarkGui()
{
    delete ui;
}

void FeatureMarkGui::setTextSlot(bool is_marking_status) {

	QString pre_label = tooth_fdi_preview_ui_->currentMeshIsUpper() ? tr("Upper") : tr("Lower");

	if (is_marking_status)
	{
		ui->toothFdiGuideLab->setText(pre_label + " " + tr("mark teeth"));
		ui->titleLabel->setText(tr("Mark feature points according to the diagram."));
		ui->textLabel->setText(tr("Please mark three feature points for each tooth in turn along the direction shown in the figure."));
	}
	else {
		ui->toothFdiGuideLab->setText(pre_label + " " + tr("missing tooth"));
		ui->titleLabel->setText(tr("Confirm the number of teeth"));
		ui->textLabel->setText(tr("Confirm that the number of teeth is consistent with the actual situation of the model. Select the tooth bitmap and right-click to mark the missing tooth position."));
	}
}
