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

#pragma once

#include <QObject>
#include "../common_base/logsingleton.h"
#include "common_base/SignalManager.h"
#include "common_ext/data/fusionaligndata.h"
#include <thread>
#include <future>
#include <QVector>

#include "common_ext_global.h"

class MeshDocument;
class MeshModel;
class MLSceneGLSharedDataContext;
class DentalManager;
class COMMON_EXT_EXPORT ProjectParser : public QObject
{
	Q_OBJECT

public:
	ProjectParser(QObject *parent = nullptr);

    // TODO: Separate display and data logic when refactoring design side
    ProjectParser(MLSceneGLSharedDataContext *context, QObject *parent = nullptr);
	~ProjectParser();

public:
	// Parse alignment project data
	bool parseAlnData(MeshDocument* md, QString name = "");
	// Parse historical comparison alignment data
	bool parseHistoricalComparisonAlnData(MeshDocument* md);

	// Parse alignment solution file, sorted by modification time
	bool parseFusionSlnData(const QString &fileName, QString &patientName, QMap <sAlignProj::eAlignProjType, QVector<sAlignProj>>& alnPrjs);

    bool getFusionSlnFromAln(const QString &fileName, QString &patientName, QMap <sAlignProj::eAlignProjType, QVector<sAlignProj>>& alnPrjs);

    bool getRencentProjFromSolution(QString &fileName, bool submit = true);

    bool saveProjToFas(const QMap <sAlignProj::eAlignProjType, QVector<sAlignProj>>& alnPrjs);

	void setRealRootAndJawBoneBtnEnable();

	void setRealGingivaBtnEnable();

	void updateAutoAverageAnimation(MeshDocument* md);

protected:
	void addVirtualGingivalToMeshVertexPreferential(DentalManager *curManager, vector<Point3m> *pVertList, vector<FFace> *pFaceList);
	vector<CVertexO*> getOrAddVertexPointerFromMesh(CMeshO *_mesh, vector<Point3m> *pVertList);
	CVertexO* addOrFindThisPointsPointer(Point3f p, int start, CMeshO *_mesh);
	// Collect dental mesh for each animation step
	bool collectDentalEachStepMesh(DentalManager *curManager, int &completeNum, float fVelocity, MeshDocument *md);
	// Collect dental historical stage mesh
	bool collectDentalHistoricalStageEachStepMesh(DentalManager* curManager, int& completeNum, float fVelocity, MeshDocument* md);
    void initSignalSlots();

    bool getDomDoc(const QString &filePath, QDomDocument &doc);

private:
	QGLWidget *p_gla_ = nullptr;
    MLSceneGLSharedDataContext *p_shared_context_ = nullptr;
	MeshModel *meshMode_L = nullptr, *meshMode_U = nullptr;
	bool lowerDentalExisted = false, upperDentalExisted = false;
	// Gingival mesh vertex and face start positions
	int gumVertStartPos, gumFaceStartPos;

	int loadCompleteNum = 0;
	std::future<bool> handleUpper, handleLower;

    int i_mesh_loaded_num_ = 0, i_historical_mesh_loaded_num_ = 0;
	public slots:
	void onUpdateTreatmentStepSlot(int iStageIndex, bool bShowTreatmentMesh);
	void showStageTreatmentOrCompareModel(bool bShowTreatmentMesh, int iStageIndex);
};
