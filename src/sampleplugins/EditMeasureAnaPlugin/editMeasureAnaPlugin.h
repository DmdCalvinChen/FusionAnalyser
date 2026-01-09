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

#ifndef EDITMEASUREANAPLUGIN_H
#define EDITMEASUREANAPLUGIN_H

#include "EditMeasureAnaPlugin_global.h"

#include<QObject>
#include <common/config.h>
#include"common/interfaces.h"
#include "data/fusionaligndata.h"
#include "data/dentalanalysisdata.h"
#include "FusionAnalyser/glarea.h"
#include <UI_Common/gifViewLabel/gifviewgui.h>
#include <UI_Common//FusionAnalyserHomeGui/fusionanalyserbuttongui/tipswidget.h>

enum EditItemIndex : int ;

class EditMeasureAnaPlugin :public QObject, public MeshEditInterface
{
	Q_OBJECT
		Q_INTERFACES(MeshEditInterface)

public:
	EditMeasureAnaPlugin(QObject* parent = 0);
	EditMeasureAnaPlugin(QAction* pAct,QObject* parent = 0);
	virtual ~EditMeasureAnaPlugin();
public:
	virtual const QString Info() { return QString(); };
	void suggestedRenderingData(MeshModel& m, MLRenderingData& dt);
	virtual bool StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	virtual void EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	void Decorate(MeshModel& m, GLArea* parent, QPainter* p);
	void keyPressEvent(QKeyEvent* event, MeshModel& m, GLArea* parent);
	void mousePressEvent(QMouseEvent* event, MeshModel& m, GLArea* parent);
	void mouseMoveEvent(QMouseEvent* event, MeshModel& m, GLArea* parent);
	void mouseReleaseEvent(QMouseEvent* event, MeshModel& m, GLArea* parent);
	void mouseDoubleClickEvent(QMouseEvent* event, MeshModel& m, GLArea* parent);
	void updateUI(bool visible = true);

public:
	int mouseX_ = 0, mouseY_ = 0;
	Qt::MouseButton mouse_btn_;
	GLArea* parent_ = nullptr;
	bool dental_analysis_press_ = false;
	bool dental_analysis_move_ = false;
	bool dental_analysis_release_ = false;
    EditItemIndex edit_item_index_;
	QAction* p_act_ = nullptr;
};

class MissingToothAnaPlugin : public EditMeasureAnaPlugin
{
	Q_OBJECT
		Q_INTERFACES(MeshEditInterface)

public:
	MissingToothAnaPlugin(QObject* parent = 0);
	virtual ~MissingToothAnaPlugin();
public:
	const QString Info();
};

class TeethWidthAnaPlugin : public EditMeasureAnaPlugin
{
	Q_OBJECT
		Q_INTERFACES(MeshEditInterface)

public:
	TeethWidthAnaPlugin(QObject* parent = 0);
	TeethWidthAnaPlugin(QAction* pAct , QObject* parent = 0);

	bool StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	void EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	virtual ~TeethWidthAnaPlugin();
	void updateUI(bool visible = true);
	GLArea* GLA = nullptr;

public:
	const QString Info();
	Gifviewgui* gif_TeethWidth_ui = nullptr;  // Tooth width measurement GIF
	tipsWidget* TeethWidthtipBtn = nullptr;

public slots:
	void showTeethWidthGifSlot();
};

class CrowdingDegreeAnaPlugin : public EditMeasureAnaPlugin
{
	Q_OBJECT
		Q_INTERFACES(MeshEditInterface)

public:
	CrowdingDegreeAnaPlugin(QObject* parent = nullptr);
	CrowdingDegreeAnaPlugin(QAction* pAct, QObject* parent = nullptr);

	bool StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	void EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	virtual ~CrowdingDegreeAnaPlugin();

public:
	const QString Info();
};

class BoltonAnaPlugin : public EditMeasureAnaPlugin
{
	Q_OBJECT
		Q_INTERFACES(MeshEditInterface)

public:
	BoltonAnaPlugin(QObject* parent = nullptr);
	BoltonAnaPlugin(QAction* pAct, QObject* parent = 0);

	bool StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	void EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	virtual ~BoltonAnaPlugin();

public:
	const QString Info();
};

class SpeeCurveDepthAnaPlugin : public EditMeasureAnaPlugin
{
	Q_OBJECT
		Q_INTERFACES(MeshEditInterface)

public:
	SpeeCurveDepthAnaPlugin(QObject* parent = nullptr);
	SpeeCurveDepthAnaPlugin(QAction* pAct, QObject* parent = 0);

	bool StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	void EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	virtual ~SpeeCurveDepthAnaPlugin();

public:
	const QString Info();
};

class MolarRelationshipAnaPlugin : public EditMeasureAnaPlugin
{
	Q_OBJECT
		Q_INTERFACES(MeshEditInterface)

public:
	MolarRelationshipAnaPlugin(QObject* parent = nullptr);
	MolarRelationshipAnaPlugin(QAction* pAct , QObject* parent = 0);

	bool StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	void EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	virtual ~MolarRelationshipAnaPlugin();

public:
	const QString Info();
};

class MidlineRelationshipAnaPlugin : public EditMeasureAnaPlugin
{
	Q_OBJECT
		Q_INTERFACES(MeshEditInterface)

public:
	MidlineRelationshipAnaPlugin(QObject* parent = nullptr);
	MidlineRelationshipAnaPlugin(QAction* pAct, QObject* parent = 0);

	bool StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	void EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	virtual ~MidlineRelationshipAnaPlugin();

public:
	const QString Info();
};

class ArchWidthAnaPlugin : public EditMeasureAnaPlugin
{
	Q_OBJECT
		Q_INTERFACES(MeshEditInterface)

public:
	ArchWidthAnaPlugin(QObject* parent = nullptr);
	ArchWidthAnaPlugin(QAction* pAct, QObject* parent = 0);

	bool StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	void EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	virtual ~ArchWidthAnaPlugin();

public:
	const QString Info();
};

class ArchLengthAnaPlugin : public EditMeasureAnaPlugin
{
	Q_OBJECT
		Q_INTERFACES(MeshEditInterface)

public:
	ArchLengthAnaPlugin(QObject* parent = nullptr);
	ArchLengthAnaPlugin(QAction* pAct , QObject* parent = 0);

	bool StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	void EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	virtual ~ArchLengthAnaPlugin();

public:
	const QString Info();
};

class GnathotectumHeightAnaPlugin : public EditMeasureAnaPlugin
{
	Q_OBJECT
		Q_INTERFACES(MeshEditInterface)

public:
	GnathotectumHeightAnaPlugin(QObject* parent = nullptr);
	GnathotectumHeightAnaPlugin(QAction* pAct, QObject* parent = 0);

	bool StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	void EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	virtual ~GnathotectumHeightAnaPlugin();
public:
	const QString Info();
};

class BasalBoneArchAnaPlugin : public EditMeasureAnaPlugin
{
	Q_OBJECT
		Q_INTERFACES(MeshEditInterface)

public:
	BasalBoneArchAnaPlugin(QObject* parent = nullptr);
	BasalBoneArchAnaPlugin(QAction* pAct, QObject* parent = 0);

	bool StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	void EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	virtual ~BasalBoneArchAnaPlugin();

public:
	const QString Info();
};

#endif // EDITMEASUREANAPLUGIN_H
