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

#include "editModelMarkFactory.h"
#include "editModelMarkPlugin.h"

EditModelMarkFactory::EditModelMarkFactory(QObject* parent /*= nullptr*/)
{

	if (p_translator_ == nullptr)
		p_translator_ = new QTranslator;
	if (PFusionAppData->getAppLanguage() == E_CHINESE)
		p_translator_->load(":/EditModelMarkPlugin_zh_CN.qm");
	else if (PFusionAppData->getAppLanguage() == E_ENGLISH)
		p_translator_->load(":/EditModelMarkPlugin_en.qm");
	qApp->installTranslator(p_translator_);
	p_mark_act_ = new QAction(tr("ModelMark"), this);
	p_mark_act_->setObjectName(EDIT_MODEL_MARKING_ACTION_NAME);
	p_mark_act_->setToolTip(tr("ModelMark"));
	p_mark_act_->setProperty("Plugin", "ModelMark");
	p_mark_act_->setData(E_ProgramAnalyser);

	action_list_ << p_mark_act_;
}

EditModelMarkFactory::~EditModelMarkFactory()
{

}

QList<QAction*> EditModelMarkFactory::actions() const
{
	return action_list_;
}

MeshEditInterface* EditModelMarkFactory::getMeshEditInterface(QAction* pAct)
{
	if (pAct == p_mark_act_)
	{
		if (p_mark_plugin_ == nullptr)
			p_mark_plugin_ = new EditModelMarkPlugin;

		return p_mark_plugin_;
	}
}

QString EditModelMarkFactory::getEditToolDescription(QAction*)
{
	return tr("tooth model mark plugin");
}
