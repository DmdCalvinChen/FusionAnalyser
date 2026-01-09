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

#include "ProjectParser.h"
#include "common/meshmodel.h"
#include "common/meshlabdocumentxml.h"
#include "common/ml_shared_data_context.h"
#include "common_ext/data/dentalmanager.h"

// Save interval for auto-save functionality
#define SAVE_INTERVEL 10

#include <QMessageBox>

const QString FUSIONSLNNODENAME("fusionAlignSolution");
const QString FUSIONSLNDOCTYPE("fusionAlignSolution");
const QString FUSIONTREATMENTPROJSNODE("treatmentProjects");
const QString SUBMITPROJECTNODE("submitProjects");
const QString TEMPPROJECTNODE("tempProjects");
const QString FUSIONTREATMENTPROJNODE("treatmentProject");
const QString TIMEATTRIBUTE("saveTime");
const QString PATIENTNAMEATTRIBUTE("patientName");
const QString SCHEMENAMEATTRIBUTE("schemeName");
const QString DESIGNIDATTRIBUTE("designId");
ProjectParser::ProjectParser(QObject *parent)
	: QObject(parent)
{
    initSignalSlots();
}

ProjectParser::ProjectParser(MLSceneGLSharedDataContext *context,  QObject *parent /*= nullptr*/)
{
    p_shared_context_ = context;
    p_gla_ = qobject_cast<QGLWidget*>(parent);
    initSignalSlots();
}

ProjectParser::~ProjectParser()
{
}

// Load and parse XML document from file
bool ProjectParser::getDomDoc(const QString &filePath, QDomDocument &doc)
{
    QFile qf(filePath);
    QFileInfo qfInfo(filePath);
    QDir tmpDir = QDir::current();
    QDir::setCurrent(qfInfo.absoluteDir().absolutePath());
    if (!qf.open(QIODevice::ReadOnly))
        return false;
    QString errorMsg; int errorLine;
    if (!doc.setContent(&qf, &errorMsg, &errorLine))
        return false;
    qf.close();
    return true;
}

// Parse fusion solution data from XML file
bool ProjectParser::parseFusionSlnData(const QString &fileName, QString &patientName, QMap <sAlignProj::eAlignProjType, QVector<sAlignProj>>& alnPrjs)
{
	alnPrjs.clear();

	QDomDocument doc("fusionAlignSolution");    //It represents the XML document

    if (!getDomDoc(fileName, doc))
    {
        return false;
    }

	QDomElement root = doc.documentElement();
	QDomNode node = root.firstChild();       // projects

    auto parsePoject = [&](QDomNode &node, sAlignProj::eAlignProjType type, sAlignProj &proj)
    {
        assert(!node.attributes().namedItem("schemeName").isNull());
        QString schemeName = node.attributes().namedItem("schemeName").nodeValue();
        proj.scheme_name_ = schemeName;
        proj.type_ = type;
        assert(!node.attributes().namedItem(TIMEATTRIBUTE).isNull());
        proj.time_ = node.attributes().namedItem(TIMEATTRIBUTE).nodeValue();

        if (!node.attributes().namedItem("designId").isNull())
        {
            proj.design_id_ = node.attributes().namedItem("designId").nodeValue().toInt();
        }
        else
            proj.design_id_ = -1;

        if(node.isElement())
        {
            proj.file_name_ = node.toElement().text().trimmed();
        }
    };
	if (!node.isNull())
	{
		assert(!node.attributes().namedItem("patientName").isNull());
		patientName = node.attributes().namedItem("patientName").nodeValue();
		QDomNode projects = node.firstChild();
		while (!projects.isNull())
		{
            sAlignProj::eAlignProjType type;

            QString nodeName = projects.nodeName();
            if (projects.nodeName() == SUBMITPROJECTNODE)
                type = sAlignProj::EALIGNPROJTYPE_SUBMIT;
            else if (projects.nodeName() == TEMPPROJECTNODE)
                type = sAlignProj::EALIGNPROJTYPE_TEMPORARY;
            QDomNode projNode = projects.firstChild();
            while (!projNode.isNull())
            {
                sAlignProj proj;
                parsePoject(projNode, type, proj);
                alnPrjs[type].push_back(proj);  // Sort by time
                projNode = projNode.nextSibling();
            }
			projects = projects.nextSibling();
		}
	}
	return true;
}

// Get fusion solution from alignment project file
bool ProjectParser::getFusionSlnFromAln(const QString &fileName, QString &patientName, QMap <sAlignProj::eAlignProjType, QVector<sAlignProj>>& alnPrjs)
{
    QDomDocument doc("toothAdjustProject");    //It represents the XML document
    if (!getDomDoc(fileName, doc))
    {
        return false;
    }

    QDomElement root = doc.documentElement();
    QDomNode node = root.firstChild();       // projects
    while (!node.isNull())
    {
        if(node.nodeName() == FUSIONSLNNODENAME)
        {
            break;
        }
        node = node.nextSibling();
    }

    if(!node.isNull() && node.nodeName() == FUSIONSLNNODENAME)
    {
        if (node.isElement())
        {
            QString slnFileName = node.toElement().text().trimmed();
            if (parseFusionSlnData(slnFileName, patientName, alnPrjs))
                return true;
        }
    }
    return false;
}

// Get the most recent project from solution
bool ProjectParser::getRencentProjFromSolution(QString &fileName, bool submit /*= true*/)
{
    QString time = "";

    auto getRencentProj = [&](const QVector<sAlignProj>&projs)
    {
        for (auto &proj : projs)
        {
            if (time < proj.time_)
            {
                time = proj.time_;
                fileName = proj.file_name_;
            }
        }
    };
    if(submit)
    {
        if (PFusionAlignData->getSubmitAlignProjs().size() == 0)
            return false;
        getRencentProj(PFusionAlignData->getSubmitAlignProjs());
    }
    else
    {
        if (PFusionAlignData->getSubmitAlignProjs().size() == 0 && PFusionAlignData->getTempAlignProjs().size() == 0)
            return false;
        getRencentProj(PFusionAlignData->getSubmitAlignProjs());
        getRencentProj(PFusionAlignData->getTempAlignProjs());
    }

    return true;
}

// Save projects to fusion alignment solution file
bool ProjectParser::saveProjToFas(const QMap <sAlignProj::eAlignProjType, QVector<sAlignProj>>& alnPrjs)
{
    QDomDocument ddoc(FUSIONSLNDOCTYPE);
    QDomElement root = ddoc.createElement(FUSIONSLNNODENAME);
    QDomElement projsNode = ddoc.createElement(FUSIONTREATMENTPROJSNODE);
    QString patientName = PFusionAlignData->getAnalyserData().patient_name_;
    if (patientName.isEmpty()) {
        patientName = PFusionAlignData->getAnalyserData().patient_data_.patient_name_;
    }
    projsNode.setAttribute(PATIENTNAMEATTRIBUTE, patientName);
    QDomElement submitProjsNode = ddoc.elementById(SUBMITPROJECTNODE);
    QDomElement temporyProjsNode = ddoc.elementById(TEMPPROJECTNODE);

    for(auto& projs : alnPrjs)
    {
        for (auto &item : projs)
        {
            QDomElement projNode = ddoc.createElement(FUSIONTREATMENTPROJNODE);
            projNode.setAttribute(TIMEATTRIBUTE, item.time_);
            projNode.setAttribute(SCHEMENAMEATTRIBUTE, item.scheme_name_);
            projNode.setAttribute(DESIGNIDATTRIBUTE, item.design_id_);
            QDomText fileName = ddoc.createTextNode("\n" + item.file_name_);
            projNode.appendChild(fileName);
            if (item.type_ == sAlignProj::EALIGNPROJTYPE_SUBMIT)
            {
                if (submitProjsNode.isNull())
                {
                    submitProjsNode = ddoc.createElement(SUBMITPROJECTNODE);
                }
                submitProjsNode.appendChild(projNode);
            }
            else if (item.type_ == sAlignProj::EALIGNPROJTYPE_TEMPORARY)
            {
                if (temporyProjsNode.isNull())
                {
                    temporyProjsNode = ddoc.createElement(TEMPPROJECTNODE);
                }
                temporyProjsNode.appendChild(projNode);
            }
        }
    }
    projsNode.appendChild(temporyProjsNode);
    projsNode.appendChild(submitProjsNode);
    root.appendChild(projsNode);
    ddoc.appendChild(root);

    QFile file(PFusionAlignData->getFusionSlnFileName());
    file.open(QIODevice::WriteOnly);
    QTextStream qstream(&file);
    ddoc.save(qstream, 1);
    file.close();

    return true;
}

// Parse alignment project data from MeshDocument
bool ProjectParser::parseAlnData(MeshDocument *md, QString fileName/*=""*/)
{
	assert(md);
	// Collect Upper and Lower dentition MeshModel Info
	lowerDentalExisted = false;
	upperDentalExisted = false;
	CMeshO *upper_source_mesh = nullptr, *lower_source_mesh = nullptr;
	QString upper_label_seg_mark, lower_label_seg_mark, upper_full_path_filename_seg, lower_full_path_filename_seg;
	SegmentedStatusInfo upper_info, lower_info;
	int dental_sum = 0;
	for (int i = 0; i < md->meshList.size(); ++i)
	{
		if (md->meshList[i]->upperOrLowerToothModelMark == UpperToothModel)
		{
			meshMode_U = md->meshList[i];
			upperDentalExisted = true;
			++dental_sum;
			PFusionAlignData->createUpperDentalManager();
			upper_source_mesh = new CMeshO;
			vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(*upper_source_mesh, md->meshList[i]->cm);
			upper_label_seg_mark = md->meshList[i]->_labelSegmentMark;
			upper_full_path_filename_seg = md->meshList[i]->_fullPathFileNameSegment;
		}
		else if (md->meshList[i]->upperOrLowerToothModelMark == LowerToothModel)
		{
			meshMode_L = md->meshList[i];
			lowerDentalExisted = true;
			++dental_sum;
			PFusionAlignData->createLowerDentalManager();
			lower_source_mesh = new CMeshO;
			vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(*lower_source_mesh, md->meshList[i]->cm);
			lower_label_seg_mark = md->meshList[i]->_labelSegmentMark;
			lower_full_path_filename_seg = md->meshList[i]->_fullPathFileNameSegment;
		}
	}

	if (!md->meshList.empty())//清空meshList
	{
		int numMesh = md->meshList.size();
		for (int i = 0; i < numMesh; ++i)
		{
			md->delMesh(*md->meshList.begin());
		}
	}

	PFusionAlignData->setDentalSummary(dental_sum);
	if (upperDentalExisted)
	{
		if (!PFusionAlignData->upperManager()->cDental.bDentalIsReady)
		{
			Dental* p_ref_dental = nullptr;
			PFusionAlignData->upperManager()->GetDental(upper_source_mesh, true, upper_label_seg_mark, upper_full_path_filename_seg, upper_info, md, p_ref_dental);
			for (auto index : upper_info.fixed_tooth_list_)
			{
				PFusionAlignData->upperManager()->setToothFixed(index);
			}
		}
		else
		{
			if (!PFusionAlignData->upperManager()->cDental.convexHullGenerated)
			{
				PFusionAlignData->upperManager()->cDental.constructDentalConvexHull(upper_info, nullptr);
			}
		}
	}
	SAFE_DELETE(upper_source_mesh);
	if (lowerDentalExisted)
	{
		if (!PFusionAlignData->lowerManager()->cDental.bDentalIsReady)
		{
			Dental* p_ref_dental = nullptr;
			PFusionAlignData->lowerManager()->GetDental(lower_source_mesh, false, lower_label_seg_mark, lower_full_path_filename_seg, lower_info, md, p_ref_dental);
			for (auto index : lower_info.fixed_tooth_list_)
			{
				PFusionAlignData->lowerManager()->setToothFixed(index);
			}
		}
		else
		{
			if (!PFusionAlignData->lowerManager()->cDental.convexHullGenerated)
			{
				PFusionAlignData->lowerManager()->cDental.constructDentalConvexHull(lower_info, nullptr);
			}
		}
	}
	SAFE_DELETE(lower_source_mesh);

	// FusionAnalyser always runs in Analyser mode
	PFusionAlignData->initialDentalAnalysisData(upper_info);
	PSIGNALMANAGER->updateSceneSignal();

	setRealRootAndJawBoneBtnEnable();

	int frameSummary = 0;
	if (upperDentalExisted)
	{
		int upperBraceSummary = PFusionAlignData->upperManager()->cDental.getTreatmentSubStageStepSummary();
		frameSummary += upperBraceSummary;
	}
	if (lowerDentalExisted)
	{
		int lowerBraceSummary = PFusionAlignData->lowerManager()->cDental.getTreatmentSubStageStepSummary();
		frameSummary += lowerBraceSummary;
	}
	if (frameSummary == 0)
	{
		PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
		return true;
	}

	int FRONT_VIEW = 3;
	emit PSIGNALMANAGER->setViewDirByPhotosMouseSelectedSignal(FRONT_VIEW);

	float fVelocity = 100 / float(frameSummary);
	loadCompleteNum = 0;
	// Initialize and generate dental animation models
	if (upperDentalExisted)
	{
		this->handleUpper = std::async(std::launch::async, &ProjectParser::collectDentalEachStepMesh, this, PFusionAlignData->upperManager(), std::ref(loadCompleteNum), fVelocity, md);
	}
	if (lowerDentalExisted)
	{
		this->handleLower = std::async(std::launch::async, &ProjectParser::collectDentalEachStepMesh, this, PFusionAlignData->lowerManager(), std::ref(loadCompleteNum), fVelocity, md);
	}

	PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
	return true;
}

// Parse historical comparison alignment data
bool ProjectParser::parseHistoricalComparisonAlnData(MeshDocument* md)
{
	int frameSummary = 0;
	if (frameSummary == 0)
	{
		PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
		return true;
	}
	return true;
}

// Update auto average animation
void ProjectParser::updateAutoAverageAnimation(MeshDocument* md)
{
	int frameSummary = 0;
	if (PFusionAlignData->upperManager())
	{
		int upperBraceSummary = PFusionAlignData->upperManager()->cDental.getTreatmentSubStageStepSummary();
		frameSummary += upperBraceSummary;
	}
	if (PFusionAlignData->lowerManager())
	{
		int lowerBraceSummary = PFusionAlignData->lowerManager()->cDental.getTreatmentSubStageStepSummary();
		frameSummary += lowerBraceSummary;
	}
	if (frameSummary == 0)
	{
		PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
		return;
	}

	float fVelocity = 100 / float(frameSummary);
	loadCompleteNum = 0;
	if (PFusionAlignData->upperManager())
	{
		this->handleUpper = std::async(std::launch::async, &ProjectParser::collectDentalEachStepMesh, this, PFusionAlignData->upperManager(), std::ref(loadCompleteNum), fVelocity, md);
	}
	if (PFusionAlignData->lowerManager())
	{
		this->handleLower = std::async(std::launch::async, &ProjectParser::collectDentalEachStepMesh, this, PFusionAlignData->lowerManager(), std::ref(loadCompleteNum), fVelocity, md);
	}
	PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
}

// Initialize signal-slot connections
void ProjectParser::initSignalSlots()
{
    connect(PSIGNALMANAGER, &SignalManager::currentStepIdPlayAnimationSignal_PP,
        this, &ProjectParser::showStageTreatmentOrCompareModel);
    connect(PSIGNALMANAGER, &SignalManager::updateTreatmentStepSignal,
        this, &ProjectParser::onUpdateTreatmentStepSlot);
}

// Add virtual gingival mesh to vertex list
void ProjectParser::addVirtualGingivalToMeshVertexPreferential(DentalManager *curManager, vector<Point3m> *pVertList, vector<FFace> *pFaceList)
{
	curManager->cDental.chewinggumFaces.clear();
	vector<CVertexO*> pGumVertList;
	pGumVertList = this->getOrAddVertexPointerFromMesh(curManager->cDental.p_mesh_, pVertList);

	int newGumFaceIndex = this->gumFaceStartPos;
	for (int i = 0; i < pFaceList->size(); ++i)
	{
		vcg::tri::Allocator<CMeshO>::AddFace(*curManager->cDental.p_mesh_, pGumVertList[pFaceList->at(i).n1], pGumVertList[pFaceList->at(i).n2], pGumVertList[pFaceList->at(i).n3]);
		curManager->cDental.chewinggumFaces.push_back(newGumFaceIndex);
		newGumFaceIndex++;
	}
    curManager->updateModel(curManager->cDental.p_mesh_);
}

vector<CVertexO*> ProjectParser::getOrAddVertexPointerFromMesh(CMeshO *_mesh, vector<Point3m> *pVertList)
{
	vector<CVertexO*> result;

	Point3f currentP;
	CVertexO *vertPointer;
	for (int i = 0; i < pVertList->size(); i++)
	{
		currentP = pVertList->at(i);
		vertPointer = addOrFindThisPointsPointer(currentP, gumVertStartPos, _mesh);//always new Vertex
		vertPointer->Q() = 0;
		vertPointer->C() = CCOLOR_GINGIVA;

		result.push_back(vertPointer);
	}

	return result;
}

CVertexO* ProjectParser::addOrFindThisPointsPointer(Point3f p, int start, CMeshO *_mesh)
{
	bool isNewVert = true;

	if (isNewVert)
	{
		CMeshO::VertexPointer newVertPointer;
		newVertPointer = &*vcg::tri::Allocator<CMeshO>::AddVertex(*_mesh, p);
		vcg::tri::Allocator<CMeshO>::CompactVertexVector(*_mesh);
		return newVertPointer;
	}
}

// Collect dental mesh for each step in animation
bool ProjectParser::collectDentalEachStepMesh(DentalManager *curManager, int &completeNum, float fVelocity, MeshDocument *md)
{
	curManager->asyncMonitor = curManager->asyncMonitor ^ MeshModel::ASYNC_UPDATING_ANIMATION;

	if (!curManager->cDental.bCureModelGenerated)
	{
		return false;
	}

	emit PSIGNALMANAGER->onePartOfInitalAnimationOverSignal_PP();

	curManager->asyncMonitor = curManager->asyncMonitor ^ MeshModel::ASYNC_UPDATING_ANIMATION;
    ++i_mesh_loaded_num_;

	int iAimLoadAnimationNum = 0;
	for (int i = 0; i < md->meshList.size(); ++i)
	{
		if (md->meshList[i]->upperOrLowerToothModelMark == UpperSingleDental ||
			md->meshList[i]->upperOrLowerToothModelMark == LowerSingleDental)
		{
			++iAimLoadAnimationNum;
		}
	}
    if (i_mesh_loaded_num_ == iAimLoadAnimationNum)
    {
		curManager->cDental.setRenderObject(Dental::VIRTUAL_JAW_ARRANGE);
		if (PFusionAlignData->upperManager())
		{
			PFusionAlignData->upperManager()->cDental.makeToothCatchingEye(-1);
		}
		if (PFusionAlignData->lowerManager())
		{
			PFusionAlignData->lowerManager()->cDental.makeToothCatchingEye(-1);
		}
        emit PSIGNALMANAGER->parseAlnProjFinishedSignal();
		emit PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
		emit PSIGNALMANAGER->clickAccessoryVisibleBtnSignal();
		emit PSIGNALMANAGER->setUpperDentalSelectedSignal();
        i_mesh_loaded_num_ = 0;
    }

	return true;
}

// Collect dental historical stage mesh for each step
bool ProjectParser::collectDentalHistoricalStageEachStepMesh(DentalManager* curManager, int& completeNum, float fVelocity, MeshDocument* md)
{
	curManager->asyncMonitor = curManager->asyncMonitor ^ MeshModel::ASYNC_UPDATING_ANIMATION;
	curManager->asyncMonitor = curManager->asyncMonitor ^ MeshModel::ASYNC_UPDATING_ANIMATION;

	++i_historical_mesh_loaded_num_;

	int iAimLoadAnimationNum = 0;
	for (int i = 0; i < md->meshList.size(); ++i)
	{
		if (md->meshList[i]->upperOrLowerToothModelMark == UpperSingleDental ||
			md->meshList[i]->upperOrLowerToothModelMark == LowerSingleDental)
		{
			++iAimLoadAnimationNum;
		}
	}
	if (i_historical_mesh_loaded_num_ == iAimLoadAnimationNum)
	{
		emit PSIGNALMANAGER->updateMeshVertexInfoBufferCompare_PP();
		i_mesh_loaded_num_ = 0;
	}

	return true;
}

void ProjectParser::setRealRootAndJawBoneBtnEnable()
{
}

void ProjectParser::setRealGingivaBtnEnable()
{
}

// Slot: Show stage treatment or compare model
void ProjectParser::showStageTreatmentOrCompareModel(bool bShowTreatmentMesh, int iStageIndex)
{
	PSIGNALMANAGER->updateSceneSignal();
}

void ProjectParser::onUpdateTreatmentStepSlot(int iStageIndex, bool bShowTreatmentMesh)
{
	showStageTreatmentOrCompareModel(bShowTreatmentMesh, iStageIndex);
}

