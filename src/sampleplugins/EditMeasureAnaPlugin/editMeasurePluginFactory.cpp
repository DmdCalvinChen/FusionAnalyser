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

#include "editMeasurePluginFactory.h"

EditMeasurePluginFactory::EditMeasurePluginFactory(QObject* parent /*= nullptr*/) : QObject(parent)
{
	if (p_translator_ == nullptr)
		p_translator_ = new QTranslator;
	if(PFusionAppData->getAppLanguage() == E_CHINESE)
	p_translator_->load(":/EditMeasureAnaPlugin_zh_CN.qm");
	else if (PFusionAppData->getAppLanguage() == E_ENGLISH)
		p_translator_->load(":/EditMeasureAnaPlugin_en.qm");
	qApp->installTranslator(p_translator_);

	this->setProperty("type", E_EditPluginMeasureAnaType);
	p_teeth_width_ana_act_ = new QAction(tr("Tooth W"), this);
	p_teeth_width_ana_act_->setToolTip(tr("Tooth Width"));
	p_teeth_width_ana_act_->setObjectName(EDIT_TOOTH_WIDTH_ANA_ACT_NAME);
	p_crowding_degree_ana_act_ = new QAction(tr("Crowd"), this);
	p_crowding_degree_ana_act_->setToolTip(tr("Crowding"));
	p_bolton_ana_act_ = new QAction(tr("Bolton"), this);
	p_bolton_ana_act_->setToolTip(tr("Bolton Ratio"));
	p_spee_ana_act_ = new QAction(tr("Spee"), this);
	p_spee_ana_act_->setToolTip(tr("Curve of Spee"));
	p_molar_relation_act_ = new QAction(tr("Molar"), this);
	p_molar_relation_act_->setToolTip(tr("Molar REL"));
	p_midline_relation_act_ = new QAction(tr("Center"), this);
	p_midline_relation_act_->setToolTip(tr("Midline"));
	p_arch_width_act_ = new QAction(tr("Arch W"), this);
	p_arch_width_act_->setToolTip(tr("Arch Width"));
	p_arch_length_act_ = new QAction(tr("Arch L"), this);
	p_arch_length_act_->setToolTip(tr("Arch Length"));
	p_gnathotectum_ana_act_ = new QAction(tr("Pal H"), this);
	p_gnathotectum_ana_act_->setToolTip(tr("Height of palatal vault"));
	p_basalbonearch_ana_act_ = new QAction(tr("Basebn"), this);
	p_basalbonearch_ana_act_->setToolTip(tr("Basal bone measurement"));
	iconName.append("archlength");
	iconName.append("archwide");
	iconName.append("bollton_ana");
	iconName.append("Bolton");
	iconName.append("crowding");
	iconName.append("midline");
	iconName.append("molar");
	iconName.append("palateheight");
	iconName.append("spee");
	iconName.append("basalbonearch");
	for (int num = 0; num < iconName.size(); num++)
	{
		QPixmap enabled_icon(QString(":/res/images/icon/toolbarSVG/%1_normal.svg").arg(iconName[num]));
		QPixmap disabled_icon(QString(":/res/images/icon/toolbarSVG/%1_disabled.svg").arg(iconName[num]));
		QIcon icon(enabled_icon);
		icon.addPixmap(disabled_icon, QIcon::Disabled);
		iconChange[iconName[num]] = icon;
	}
	p_teeth_width_ana_act_->setIcon(iconChange["bollton_ana"]);
	p_crowding_degree_ana_act_->setIcon(iconChange["crowding"]);
	p_bolton_ana_act_->setIcon(iconChange["Bolton"]);
	p_spee_ana_act_->setIcon(iconChange["spee"]);
	p_molar_relation_act_->setIcon(iconChange["molar"]);
	p_midline_relation_act_->setIcon(iconChange["midline"]);
	p_arch_width_act_->setIcon(iconChange["archwide"]);
	p_arch_length_act_->setIcon(iconChange["archlength"]);
	p_gnathotectum_ana_act_->setIcon(iconChange["palateheight"]);
	p_basalbonearch_ana_act_->setIcon(iconChange["basalbonearch"]);

	action_list_ << p_teeth_width_ana_act_;
	action_list_ << p_crowding_degree_ana_act_;
	action_list_ << p_bolton_ana_act_;
	action_list_ << p_spee_ana_act_;
	action_list_ << p_molar_relation_act_;
	action_list_ << p_midline_relation_act_;
	action_list_ << p_arch_width_act_;
	action_list_ << p_arch_length_act_;
	action_list_ << p_gnathotectum_ana_act_;
	action_list_ << p_basalbonearch_ana_act_;

	foreach(QAction * editAction, action_list_)
	{
		editAction->setCheckable(true);
		editAction->setData(E_ProgramAnalyser);
	}

	connect(PSIGNALMANAGER, &SignalManager::updateAvaliableItemActionsSignal, this, &EditMeasurePluginFactory::updateAnalyzerAvaliableItemActionsSlot);
	connect(PSIGNALMANAGER, &SignalManager::triggerItemActionSignal, this, &EditMeasurePluginFactory::triggerItemActionSlot);
}

EditMeasurePluginFactory::~EditMeasurePluginFactory()
{
	if (p_translator_)
	{
		qApp->removeTranslator(p_translator_);
		SAFE_DELETE(p_translator_);
	}
}

QList<QAction*> EditMeasurePluginFactory::actions() const
{
	return action_list_;
}

MeshEditInterface* EditMeasurePluginFactory::getMeshEditInterface(QAction* pAct)
{
	if (pAct == p_teeth_width_ana_act_)
	{
		if (p_teeth_width_ana_plugin_ == nullptr)
			p_teeth_width_ana_plugin_ = new TeethWidthAnaPlugin(pAct);
		return p_teeth_width_ana_plugin_;
	}
	else if (pAct == p_crowding_degree_ana_act_)
	{
		if (p_crowding_degree_ana_plugin_ == nullptr)
			p_crowding_degree_ana_plugin_ = new CrowdingDegreeAnaPlugin(pAct);
		return p_crowding_degree_ana_plugin_;
	}
	else if (pAct == p_bolton_ana_act_)
	{
		if (p_bolton_ana_plugin_ == nullptr)
			p_bolton_ana_plugin_ = new BoltonAnaPlugin(pAct);
		return p_bolton_ana_plugin_;
	}
	else if (pAct == p_spee_ana_act_)
	{
		if (p_spee_ana_plugin_ == nullptr)
			p_spee_ana_plugin_ = new SpeeCurveDepthAnaPlugin(pAct);
		return p_spee_ana_plugin_;
	}
	else if (pAct == p_molar_relation_act_)
	{
		if (p_molar_relation_ana_plugin_ == nullptr)
			p_molar_relation_ana_plugin_ = new MolarRelationshipAnaPlugin(pAct);
		return p_molar_relation_ana_plugin_;
	}
	else if (pAct == p_midline_relation_act_)
	{
		if (p_midline_relation_ana_plugin_ == nullptr)
			p_midline_relation_ana_plugin_ = new MidlineRelationshipAnaPlugin(pAct);
		return p_midline_relation_ana_plugin_;
	}
	else if (pAct == p_arch_width_act_)
	{
		if (p_arch_width_ana_plugin_ == nullptr)
			p_arch_width_ana_plugin_ = new ArchWidthAnaPlugin(pAct);
		return p_arch_width_ana_plugin_;
	}
	else if (pAct == p_arch_length_act_)
	{
		if (p_arch_length_ana_plugin_ == nullptr)
			p_arch_length_ana_plugin_ = new ArchLengthAnaPlugin(pAct);
		return p_arch_length_ana_plugin_;
	}
	else if (pAct == p_gnathotectum_ana_act_)
	{
		if (p_gnathotectum_ana_plugin_ == nullptr)
			p_gnathotectum_ana_plugin_ = new GnathotectumHeightAnaPlugin(pAct);
		return p_gnathotectum_ana_plugin_;
	}
	else if (pAct == p_basalbonearch_ana_act_)
	{
		if (p_basalbonearch_ana_plugin_ == nullptr)
			p_basalbonearch_ana_plugin_ = new BasalBoneArchAnaPlugin(pAct);
		return p_basalbonearch_ana_plugin_;
	}
	return nullptr;
}

QString EditMeasurePluginFactory::getEditToolDescription(QAction*)
{
	return tr("tooth measure ana plugin");
}

void EditMeasurePluginFactory::triggerItemActionSlot(EditItemIndex _item_index)
{
	switch (_item_index)
	{
	case TEETH_WIDTH:
		if (p_teeth_width_ana_act_->isEnabled())
		{
			p_teeth_width_ana_act_->trigger();
		}
		break;
	}
}

void EditMeasurePluginFactory::updateAnalyzerAvaliableItemActionsSlot(vector<EditItemIndex> _avaliable_item_indexes)
{
	for (auto& action : action_list_)
	{
		if (action != nullptr)
		{
			action->setEnabled(false);
		}
	}

	for (auto& item_index : _avaliable_item_indexes)
	{
		switch (item_index)
		{
		case TEETH_WIDTH:
			p_teeth_width_ana_act_->setEnabled(true);
			break;

		case CUR_LENGTH_OF_DENTAL_ARCH:
			p_crowding_degree_ana_act_->setEnabled(true);
			break;

		case BOLTON_COMPLETELY:
			p_bolton_ana_act_->setEnabled(true);
			break;

		case SPEE_CURVE_DEPTH:
			p_spee_ana_act_->setEnabled(true);
			break;

		case MOLAR_RELATIONSHIP:
			p_molar_relation_act_->setEnabled(true);
			break;

		case	MIDLINE_RELATIONSHIP:
			p_midline_relation_act_->setEnabled(true);
			break;

		case ARCH_WIDTH:
			p_arch_width_act_->setEnabled(true);
			break;

		case ARCH_LENGTH:
			p_arch_length_act_->setEnabled(true);
			break;

		case GNATHOTECTUM_HEIGHT:
			p_gnathotectum_ana_act_->setEnabled(true);
			break;

		case BASAL_BONE_ARCH:
			p_basalbonearch_ana_act_->setEnabled(true);
			break;

		default:
			break;
		}
	}
	return;
}
