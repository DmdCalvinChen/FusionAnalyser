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

#ifndef EDITMODELMARKFACTORY_H
#define EDITMODELMARKFACTORY_H

#include <QObject>
#include <QTranslator>

#include "common/config.h"
#include "common/interfaces.h"

class EditModelMarkPlugin;
class EditModelMarkFactory : public QObject, public MeshEditInterfaceFactory
{
	Q_OBJECT
		MESHLAB_PLUGIN_IID_EXPORTER(MESH_EDIT_INTERFACE_FACTORY_IID)
		Q_INTERFACES(MeshEditInterfaceFactory)
public:

	explicit EditModelMarkFactory(QObject* parent = nullptr);
	virtual ~EditModelMarkFactory();

	virtual QList<QAction*> actions() const override;
	virtual MeshEditInterface* getMeshEditInterface(QAction*);
	virtual QString getEditToolDescription(QAction*);

	virtual int order() { return E_PluginOrderModelMark; }

signals:

public slots:

private:
	QList <QAction*> action_list_;
	QAction* p_mark_act_ = nullptr;

	QTranslator* p_translator_ = nullptr;
	EditModelMarkPlugin* p_mark_plugin_ = nullptr;

};

#endif // EDITMODELMARKFACTORY_H
