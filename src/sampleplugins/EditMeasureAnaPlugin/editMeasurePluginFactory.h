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

#ifndef EDITMEASUREPLUGINFACTORY_H
#define EDITMEASUREPLUGINFACTORY_H

#include <QObject>
#include <QTranslator>

#include "common/config.h"
#include "common/interfaces.h"
#include "common_ext/data/dentalanalysisdata.h"

#include "editMeasureAnaPlugin.h"

class EditMeasurePluginFactory : public QObject, public MeshEditInterfaceFactory
{
    Q_OBJECT
		MESHLAB_PLUGIN_IID_EXPORTER(MESH_EDIT_INTERFACE_FACTORY_IID)
		Q_INTERFACES(MeshEditInterfaceFactory)
public:

	explicit EditMeasurePluginFactory(QObject* parent = nullptr);
	virtual ~EditMeasurePluginFactory();

	virtual QList<QAction*> actions() const override;
	virtual MeshEditInterface* getMeshEditInterface(QAction*);
	virtual QString getEditToolDescription(QAction*);

	virtual int order() { return E_PluginOrderMeasureAnalyis; }

signals:

public slots:
	void updateAnalyzerAvaliableItemActionsSlot(vector<EditItemIndex> _avaliable_item_indexes);
	void triggerItemActionSlot(EditItemIndex _item_index);

private:
	QList <QAction*> action_list_;
	QAction* p_teeth_width_ana_act_ = nullptr;
	QAction* p_crowding_degree_ana_act_ = nullptr;
	QAction* p_bolton_ana_act_ = nullptr;
	QAction* p_spee_ana_act_ = nullptr;
	QAction* p_molar_relation_act_ = nullptr;
	QAction* p_midline_relation_act_ = nullptr;
	QAction* p_arch_width_act_ = nullptr;
	QAction* p_arch_length_act_ = nullptr;
	QAction* p_gnathotectum_ana_act_ = nullptr;
	QAction* p_basalbonearch_ana_act_ = nullptr;
	std::map<QString, QIcon> iconChange;
	QStringList iconName;
	TeethWidthAnaPlugin* p_teeth_width_ana_plugin_ = nullptr;
	CrowdingDegreeAnaPlugin* p_crowding_degree_ana_plugin_ = nullptr;
	BoltonAnaPlugin* p_bolton_ana_plugin_ = nullptr;
	SpeeCurveDepthAnaPlugin* p_spee_ana_plugin_ = nullptr;
	MolarRelationshipAnaPlugin* p_molar_relation_ana_plugin_ = nullptr;
	MidlineRelationshipAnaPlugin* p_midline_relation_ana_plugin_ = nullptr;
	ArchWidthAnaPlugin* p_arch_width_ana_plugin_ = nullptr;
	ArchLengthAnaPlugin* p_arch_length_ana_plugin_ = nullptr;
	GnathotectumHeightAnaPlugin* p_gnathotectum_ana_plugin_ = nullptr;
	BasalBoneArchAnaPlugin* p_basalbonearch_ana_plugin_ = nullptr;
	QTranslator* p_translator_ = nullptr;

	QAction* p_cur_act_ = nullptr, * p_prev_act_ = nullptr;

};

#endif // EDITMEASUREPLUGINFACTORY_H
