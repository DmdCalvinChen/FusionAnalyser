/****************************************************************************
* MeshLab                                                           o o     *
* A versatile mesh processing toolbox                             o     o   *
*                                                                _   O  _   *
* Copyright(C) 2005-2008                                           \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/

#include "edit_sample_factory.h"
#include "overlay.h"

SampleEditFactory::SampleEditFactory()
{
	if (PFusionAppData->getAppLanguage() == E_CHINESE)
		translator_.load(":/overlayCh.qm");
	else if (PFusionAppData->getAppLanguage() == E_ENGLISH)
		translator_.load(":/overlayEn.qm");
	QCoreApplication::installTranslator(&translator_);
	this->setObjectName("Overbite/Overjet");

	// FusionAnalyser only supports Analyser mode
	editSample = new QAction(tr("OJ/OB"), this);

	editSample->setToolTip(tr("Overbite/Overjet"));
	QPixmap enabled_icon(QString(":/images/overlap_normal.svg"));
	QPixmap disabled_icon(QString(":/images/overlap_disabled.svg"));
    icon = new QIcon(enabled_icon);
	icon->addPixmap(disabled_icon, QIcon::Disabled);
	editSample->setIcon(*icon);
	connect(editSample, &QAction::triggered, this, &SampleEditFactory::setCheckedIcon);
	connect(PSIGNALMANAGER, &SignalManager::mutualExclusionWithOcclusalSignal, this, &SampleEditFactory::mutualExclusionWithOcclusal);

	actionList << editSample;

	foreach(QAction * editAction, actionList)
	{
		editAction->setCheckable(true);
		editAction->setData(0);
	}

#if PLATFORM_WINDOWS
	connect(PSIGNALMANAGER, &SignalManager::updateAvaliableItemActionsSignal, this, &SampleEditFactory::updateAnalyzerAvaliableItemActionsSlot);
#endif
}

//gets a list of actions available from this plugin
QList<QAction *> SampleEditFactory::actions() const
{
	return actionList;
}

//get the edit tool for the given action
MeshEditInterface* SampleEditFactory::getMeshEditInterface(QAction *action)
{
	if(action == editSample)
	{
		if (p_overlay_plugin_ == nullptr)
		{
			p_overlay_plugin_ = new Overlay(action);
			connect(p_overlay_plugin_, &Overlay::doneSignal, this, &SampleEditFactory::onDoneSlot);
		}

		return p_overlay_plugin_;
	} else assert(0); //should never be asked for an action that isnt here
}

QString SampleEditFactory::getEditToolDescription(QAction *)
{
	return Overlay::Info();
}

void SampleEditFactory::setCheckedIcon(bool checked)
{
	// FusionAnalyser only supports Analyser mode
	if (checked)
	{
		editSample->setIcon(QIcon(":/images/overlap_pressed.svg"));
	}
	else
	{
		editSample->setIcon(*icon);
	}
}

void SampleEditFactory::onDoneSlot()
{
	editSample->setChecked(false);
	setCheckedIcon(false);
}

void SampleEditFactory::mutualExclusionWithOcclusal()
{
	if (editSample->isChecked())
	{
		editSample->trigger();
	}
}

#if PLATFORM_WINDOWS
void SampleEditFactory::updateAnalyzerAvaliableItemActionsSlot(std::vector<EditItemIndex> _avaliable_item_indexes)
{
	if (editSample)
	{
		editSample->setEnabled(false);
	}

	for (auto& item_index : _avaliable_item_indexes)
	{
		switch (item_index)
		{
		case OVERLAY_ANALYSIS:
			editSample->setEnabled(true);
			break;

		default:
			break;
		}
	}
	return;
}
#endif

MESHLAB_PLUGIN_NAME_EXPORTER(SampleEditFactory)
