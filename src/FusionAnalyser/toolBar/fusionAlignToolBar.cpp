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

#include "fusionAlignToolBar.h"

#include <QToolButton>
#include <QFile>
#include <QFrame>

#include "common_ext/data/fusionaligndata.h"
// ZIP functionality removed - JlCompress disabled

#include "common_base/util/uitools.h"

// HTTP functionality removed - FusionAnalyser does not depend on network
#include "common_ext/util/utility_tools.h"

FusionAlignToolBar::FusionAlignToolBar(QWidget *parent)
    :QToolBar(parent)
{
    createActAndBtn();
    initLayout();
	this->setObjectName("FusionAlignToolBar");
}

void FusionAlignToolBar::createActAndBtn()
{
	auto createSpecialBtn = [&](QAction *&pAct, QToolButton *&btn, QString icon, QString text, QString objName)
	{
		pAct = new QAction(QIcon(icon), text, this);
		btn = new QToolButton(this);
		btn->setObjectName(objName);
		btn->setDefaultAction(pAct);
		btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	};

	p_stage_menu_ = new QMenu(this);
	p_stage_menu_->setStyleSheet("QMenu::item:selected {background-color:rgb(55,185,218);}\
                                  QMenu::item::disabled {background-color:rgb(128,128,128);}");

	p_segment_btn_ = new QToolButton(this);
	p_segment_btn_->setObjectName("toothSegmentBtn");
	p_segment_btn_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    p_decorate_bar_ = new QToolBar(this);
    p_decorate_bar_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    p_decorate_bar_->setObjectName("alignDecoratgeBar");
	p_decorate_bar_->setContentsMargins(0, 0, 0, 0);

	p_edit_decorate_bar_ = new QToolBar(this);
	p_edit_decorate_bar_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	p_edit_decorate_bar_->setObjectName("alignEditDecoratgeBar");
	p_edit_decorate_bar_->setContentsMargins(0, 0, 0, 0);

    p_proj_bar_ = new QToolBar(this);
    p_proj_bar_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    p_proj_bar_->setObjectName("alignProjBar");
	p_proj_bar_->setContentsMargins(0, 0, 0, 0);

	p_analysis_bar = new QToolBar(this);
	p_analysis_bar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	p_analysis_bar->setObjectName("analysisProjBar");
	p_analysis_bar->setContentsMargins(0, 0, 0, 0);
    this->setContentsMargins(0, 0, 0, 0);
}

void FusionAlignToolBar::initLayout()
{
    this->addWidget(p_decorate_bar_);
	this->addSeparator();
	this->addWidget(p_edit_decorate_bar_);
	this->addSeparator();
	this->addWidget(p_analysis_bar);
    p_decorate_bar_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	if (PFusionAlignData->getAnalyserData().getIsBmu())
	{
		QWidget* spacer = new QWidget(this);
		spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		this->addWidget(spacer);
		this->addWidget(p_segment_btn_);
		QFrame* frame = new QFrame(this);
		frame->setFixedWidth(20);
		this->addWidget(frame);
	}
}

void FusionAlignToolBar::setToolBarEnabled(bool enable)
{
    p_segment_btn_->setEnabled(enable);
	p_proj_bar_->setEnabled(enable);
	p_decorate_bar_->setEnabled(enable);
	p_analysis_bar->setEnabled(enable);
}
