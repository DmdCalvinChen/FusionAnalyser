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

#include "mainwindow.h"
#include "saveSnapshotDialog.h"
#include "savemaskexporter.h"
#include <exception>
#include "xmlgeneratorgui.h"
#include "ml_default_decorators.h"

#include <QToolBar>
#include <QToolTip>
#include <QStatusBar>
#include <QMenuBar>
#include <QProgressBar>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QClipboard>
#include <QScreen>
#include <QMessageBox>

#include "common/scriptinterface.h"
#include "common/meshlabdocumentxml.h"
#include "common/mlapplication.h"
#include "common/filterscript.h"
#include "common/config.h"
#include "common_ext/data/dentalmanager.h"
#include "common_ext/util/utility_tools.h"
#include <common_base/SignalManager.h>
#include <vcg/complex/algorithms/update/curvature.h>

#include <UI_Common/guicontrolcustomdesign/collectfolded.h>
#include "UI_Common/framelessWindow/fusionTitlebar.h"
#include "UI_Common/aboutDlg/QtAboutDialog.h"
#include "UI_Common/qFusionProgressBar/QFusionProgressDlg.h"
#include "UI_Common/fusionViewGui/fusionViewGui.h"
#include "UI_Common/UserCenterUI/fusionChangeLanguageDlg.h"
#include "common_base/util/uitools.h"
#include "common_ext/util/utility_tools.h"
#include "common_ext/data/fusionDataCommon.h"
#include "ui_common/UserCenterUI/workDirSetingDlg.h"
#include "common_base/util/fusionMessageBox.h"

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <windowsx.h>
#pragma comment(lib, "user32.lib")
#endif

using namespace std;
using namespace vcg;
using namespace ui_common;

void MainWindow::updateRecentFileActions()
{
	bool activeDoc = true;
	QSettings settings;
	QStringList files = settings.value("recentFileList").toStringList();

	int numRecentFiles = qMin(files.size(), (int)MAXRECENTFILES);

	for (int i = 0; i < numRecentFiles; ++i)
	{
		if(QFileInfo(files[i]).exists())
		{
			QString text = QFileInfo(files[i]).fileName();
			recentFileActs[i]->setText(text);
			recentFileActs[i]->setData(files[i]);
			recentFileActs[i]->setEnabled(activeDoc);
		}
	}
	for (int j = numRecentFiles; j < MAXRECENTFILES; ++j)
		recentFileActs[j]->setVisible(false);
}

void MainWindow::updateRecentProjActions()
{

	QSettings settings;
	QStringList projs = settings.value("recentProjList").toStringList();

	int numRecentProjs = qMin(projs.size(), (int)MAXRECENTFILES);

	recentProjMenu->clear();
	for (int i = 0; i < numRecentProjs; ++i)
	{
		if (QFileInfo(projs[i]).exists())
		{
			QString text = QFileInfo(projs[i]).fileName();
			QAction* act = new QAction(this);
			act->setText(text);
			act->setData(projs[i]);
			act->setEnabled(true);
			connect(act, &QAction::triggered, this, [&]() {if (whetherSaveProject() >= 0) openRecentProj(); });
			recentProjMenu->addAction(act);
			recentProjMenu->setFont(fontTitle);
		}
	}
}

void MainWindow::updateCustomSettings()
{
	mwsettings.updateGlobalParameterSet(currentGlobalParams);
	emit dispatchCustomSettings(currentGlobalParams);
}

void MainWindow::enableDocumentSensibleActionsContainer(const bool allowed)
{
	QAction* fileact = fileMenu->menuAction();
	if (fileact != NULL)
		fileact->setEnabled(allowed);
	if (mainToolBar != NULL)
		mainToolBar->setEnabled(allowed);

	QAction* editact = editMenu->menuAction();
	if (editact != NULL)
		editact->setEnabled(allowed);
}

void MainWindow::updateMenuItems(QMenu* menu, const bool enabled)
{
	foreach(QAction * act, menu->actions())
		act->setEnabled(enabled);
}

void MainWindow::switchOffDecorator(QAction* decorator)
{
	if (GLA() != NULL)
	{
		int res = GLA()->iCurPerMeshDecoratorList().removeAll(decorator);
		if (res == 0)
			GLA()->iPerDocDecoratorlist.removeAll(decorator);
		updateMenus();
		GLA()->update();
	}
}

void MainWindow::updateMenus()
{
	//bool activeDoc = !(mdiarea->subWindowList().empty()) && (mdiarea->currentSubWindow() != NULL);
	if (meshDoc() == NULL)
		return;
	bool activeDoc = true;
	bool notEmptyActiveDoc = activeDoc && (meshDoc() != NULL) && !(meshDoc()->meshList.empty());

	exportMeshAct->setEnabled(notEmptyActiveDoc);
	exportMeshAsAct->setEnabled(notEmptyActiveDoc);

	saveProjectAct->setEnabled(activeDoc);
	saveProjectAsAct->setEnabled(activeDoc);
	closeProjectAct->setEnabled(activeDoc);

	updateRecentFileActions();
	updateRecentProjActions();

	showTrackBallAct->setEnabled(activeDoc);
	if (activeDoc && GLA())
	{

		foreach(QAction * a, PM.editActionList)
		{
			a->setChecked(false);
		}

		if (GLA()->getCurrentEditAction())
		{
			GLA()->getCurrentEditAction()->setChecked(!GLA()->suspendedEditor);
		}

		showTrackBallAct->setChecked(GLA()->isTrackBallVisible());

		// Decorator Menu Checking and unChecking
		// First uncheck and disable all the decorators
		foreach(QAction * a, PM.decoratorActionList)
		{
			a->setChecked(false);
			a->setEnabled(true);
		}
		// Check the decorator per Document of the current glarea
		foreach(QAction * a, GLA()->iPerDocDecoratorlist)
		{
			a->setChecked(true);
		}

		// Then check the decorator enabled for the current mesh.
		if (GLA()->mm())
			foreach(QAction * a, GLA()->iCurPerMeshDecoratorList())
			a->setChecked(true);
	} // if active
	else
	{
		foreach(QAction * a, PM.editActionList)
		{
			a->setEnabled(false);
		}
		foreach(QAction * a, PM.decoratorActionList)
			a->setEnabled(false);

	}

	if (GLA() == NULL)
	{
		foreach(QAction * a, PM.decoratorActionList)
		{
			a->setChecked(false);
			a->setEnabled(false);
		}
	}
}

void MainWindow::linkViewers()
{
	//MultiViewer_Container* mvc = currentViewContainer();
	//mvc->updateTrackballInViewers();
}

void MainWindow::toggleOrtho()
{
	if (GLA()) GLA()->toggleOrtho();
}

void MainWindow::viewFrom(QAction* qa)
{
	if (GLA()) GLA()->createOrthoView(qa->text());
}

void MainWindow::trackballStep(QAction* qa)
{
	if (GLA()) GLA()->trackballStep(qa->text());
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
	event->accept();
}

void MainWindow::dropEvent(QDropEvent* event)
{

	const QMimeData* data = event->mimeData();
	if (data->hasUrls())
	{
		QList< QUrl > url_list = data->urls();
		bool layervis = false;
		for (int i = 0, size = url_list.size(); i < size; i++)
		{
			QString path = url_list.at(i).toLocalFile();
			if ((event->keyboardModifiers() == Qt::ControlModifier) || (QApplication::keyboardModifiers() == Qt::ControlModifier))
			{
				this->newProject();
			}

			if (path.endsWith("mlp", Qt::CaseInsensitive) || path.endsWith("fa", Qt::CaseInsensitive) || path.endsWith("aln", Qt::CaseInsensitive) || path.endsWith("out", Qt::CaseInsensitive) || path.endsWith("nvm", Qt::CaseInsensitive))
			{
				if (!openProject(path))
				{
				}
			}
			else
			{
				QStringList files;
				files.push_back(path);
				loadMeshMoelsSlot(files);
			}
		}
	}
}

void MainWindow::endEdit()
{
	emit setSwitchToMainMenu(true);
	emit setParallelSwitched(false);

	MultiViewer_Container* mvc = currentViewContainer();
	if ((meshDoc() == NULL) || (GLA() == NULL) || (mvc == NULL))
		return;

	for (int ii = 0; ii < meshDoc()->meshList.size(); ++ii)
	{
		MeshModel* mm = meshDoc()->meshList[ii];
		if (mm != NULL)
			addRenderingDataIfNewlyGeneratedMesh(mm->id());
	}
	meshDoc()->meshDocStateData().clear();

    updateProgressBar(0, "");
	GLA()->endEdit();
}

void MainWindow::showTooltip(QAction* q)
{
	QString tip = q->toolTip();
	QToolTip::showText(QCursor::pos(), tip);
}

void MainWindow::scriptCodeExecuted(const QScriptValue& val, const int time, const QString& output)
{
	if (val.isError())
	{
		meshDoc()->Log.Logf(GLLogStream::SYSTEM, "Interpreter Error: line %i: %s", val.property("lineNumber").toInt32(), qPrintable(val.toString()));
	}
	else
	{
		meshDoc()->Log.Logf(GLLogStream::SYSTEM, "Code executed in %d millisecs.\nOutput:\n%s", time, qPrintable(output));
		//bool res;
		GLA()->update();
	}
}

void MainWindow::suspendEditMode()
{
	// return if no window is open
	if (!GLA()) return;

	// return if no editing action is currently ongoing
	if (!GLA()->getCurrentEditAction()) return;

	GLA()->suspendEditToggle();
	updateMenus();
	GLA()->update();
}

void MainWindow::applyEditToothAdjust(QAction* action)
{
	if (!GLA()) { //prevents crash without mesh
		return;
	}
	if (GLA()->getCurrentEditAction()) //prevents multiple buttons pushed
	{
		if (action == GLA()->getCurrentEditAction()) // We have double pressed the same action and that means disable that actioon
		{
			if (GLA()->suspendedEditor)
			{
				suspendEditMode();
			}
			endEdit();
			updateMenus();

		}
	}

	//if this GLArea does not have an instance of this action's MeshEdit tool then give it one
	if (!GLA()->editorExistsForAction(action))
	{
		MeshEditInterfaceFactory* iEditFactory = qobject_cast<MeshEditInterfaceFactory*>(action->parent());
		MeshEditInterface* iEdit = iEditFactory->getMeshEditInterface(action);
		GLA()->addMeshEditor(action, iEdit);
	}
	meshDoc()->meshDocStateData().create(*meshDoc());
	emit GLA()->setProjectFirstEnterSinceSoftwareRunTrueSignal();
	GLA()->setCurrentEditAction(action);
	updateMenus();
	GLA()->update();
}

void MainWindow::applyEditMode()
{
	SPDLOG_FUNC
	if (!GLA()) { //prevents crash without mesh
		QAction* action = qobject_cast<QAction*>(sender());
		action->setChecked(false);
		return;
	}

	if (b_compare_view_split_)
	{
		p_cur_compare_act_->triggered(false);
	}

	QAction* action = qobject_cast<QAction*>(sender());
	MeshEditInterfaceFactory* iEditFactory = qobject_cast<MeshEditInterfaceFactory*>(action->parent());
	QString strPluginName("");
	if (iEditFactory) strPluginName = iEditFactory->getEditToolDescription(action);

	SPDLOG->info(QString("%1 start edit").arg(strPluginName));
/*	QString icon_normal = action->icon().name();
	int _normal = icon_normal.lastIndexOf("_noraml");
	QString icon_pressed;
	if (_normal == -1)
	{
		icon_pressed = icon_normal.insert(icon_normal.length() - 4, "_normal");
	}
	else
	{
		icon_pressed = icon_normal.replace(_normal, 7, "_pressed");
	}*/
	if (GLA()->getCurrentEditAction()) //prevents multiple buttons pushed
	{
		if (action == GLA()->getCurrentEditAction()) // We have double pressed the same action and that means disable that actioon
		{
		//	action->setIcon(QIcon(icon_normal));
			emit setSwitchToMainMenu(true);
			emit setParallelSwitched(false);

			if (GLA()->suspendedEditor)
			{
				suspendEditMode();
				return;
			}
			endEdit();
			updateMenus();
			return;
		}
		else
		{
		//	action->setIcon(QIcon(icon_pressed));
			emit setSwitchToMainMenu(false);
			emit setParallelSwitched(true);

			endEdit();
		}
	}
	//if this GLArea does not have an instance of this action's MeshEdit tool then give it one
	if (!GLA()->editorExistsForAction(action))
	{
		MeshEditInterface* iEdit = iEditFactory->getMeshEditInterface(action);
		GLA()->addMeshEditor(action, iEdit);
	}
	meshDoc()->meshDocStateData().create(*meshDoc());
	GLA()->setCurrentEditAction(action);

	updateMenus();
	GLA()->update();
}

void MainWindow::applyRenderMode()
{
	QAction* action = qobject_cast<QAction*>(sender());		// find the action which has sent the signal
	if ((GLA() != NULL) && (GLA()->getRenderer() != NULL))
	{
		GLA()->getRenderer()->Finalize(GLA()->getCurrentShaderAction(), meshDoc(), GLA());
		GLA()->setRenderer(NULL, NULL);
	}
	// Make the call to the plugin core
	MeshRenderInterface* iRenderTemp = qobject_cast<MeshRenderInterface*>(action->parent());
	bool initsupport = false;

	if (currentViewContainer() == NULL)
		return;

	MLSceneGLSharedDataContext* shared = currentViewContainer()->sharedDataContext();

	if ((shared != NULL) && (iRenderTemp != NULL))
	{
		MLSceneGLSharedDataContext::PerMeshRenderingDataMap rdmap;
		shared->getRenderInfoPerMeshView(GLA()->context(), rdmap);
		iRenderTemp->Init(action, *(meshDoc()), rdmap, GLA());
		initsupport = iRenderTemp->isSupported();
		if (initsupport)
			GLA()->setRenderer(iRenderTemp, action);
		else
		{
			if (!initsupport)
			{
				QString msg = "The selected shader is not supported by your graphic hardware!";
				GLA()->Logf(GLLogStream::SYSTEM, qPrintable(msg));
			}
			iRenderTemp->Finalize(action, meshDoc(), GLA());
		}
	}

	/*I clicked None in renderMenu */
	if ((action->parent() == this) || (!initsupport))
	{
		QString msg("No Shader.");
		GLA()->Logf(GLLogStream::SYSTEM, qPrintable(msg));
		GLA()->setRenderer(0, 0); //default opengl pipeline or vertex and fragment programs not supported
	}
	GLA()->update();
}

void MainWindow::applyDecorateMode()
{
	if (GLA()->mm() == 0) return;
	QAction* action = qobject_cast<QAction*>(sender());		// find the action which has sent the signal

	MeshDecorateInterface* iDecorateTemp = qobject_cast<MeshDecorateInterface*>(action->parent());

	GLA()->toggleDecorator(iDecorateTemp->decorationName(action));

	updateMenus();
	GLA()->update();
}

void MainWindow::saveProject()
{
	if (GLA()->getWhetherAlignProjectNeedToSaveAuto())
	{
		autoSaveProject(true);
	}
	if (GLA()->getWhetherSegmentProjectNeedToSaveAuto())
	{
		autoSaveProject(false);
	}

}

void MainWindow::saveProject(QString fileName, bool bAlignProject /*= true*/)
{
    if (!meshDoc() || fileName.isEmpty())
    {
        return;
    }
    bool ret;

    QString infoSaveProject;
    if (bAlignProject)
    {
        infoSaveProject = tr("align project");
		if(fileName.right(4) != ".aln")
		{
			fileName.append(".aln");
		}
        //PFusionAlignData->setAlignProjFileName(fileName);
		if (PFusionAlignData->getFusionSlnFileName() == "")
		{
			PFusionAlignData->setFusionSlnFileName("project.fas");
		}
		QFileInfo fasInfo(PFusionAlignData->getFusionSlnFileName());
		if (!fasInfo.isFile())
		{
            if(fileName != PFusionAlignData->getAlignProjOriFileName())
            {
                PFusionAlignData->setAlignProjFileName(fileName);
            }
			onSaveAlignProjectSlot();
		}
    }
    else
    {
        infoSaveProject = tr("analyser project");
    }
    //infoSaveProject.append(fileName.toLocal8Bit().toStdString());
    infoSaveProject.append(tr("Project is being saved"));
    QCallBack(60, infoSaveProject.toStdString().c_str());
    ret = MeshDocumentToXMLFile(*meshDoc(), fileName, false, !bAlignProject);
    if (!ret)
    {
        //QMessageBox::critical(this, tr("Fusionalign Saving Error"), QString("Unable to save project file %1\n").arg(fileName));
        UiUtilityTools::getInstance()->showInfoMessageBox(tr("Error"), tr("Unable to save project file %1").arg(fileName));
        logInstance->log->error("Fusionalign project Saving Error");
        logInstance->log->flush();
    }
    QCallBack(0, infoSaveProject.toStdString().c_str());

}

void MainWindow::saveProjectAs()
{
	if (meshDoc() == NULL)
		return;
	//if a mesh has been created by a create filter we must before to save it. Otherwise the project will refer to a mesh without file name path.
	foreach(MeshModel * mp, meshDoc()->meshList)
	{
		if ((mp != NULL) && (mp->fullName().isEmpty()))
		{
			bool saved = exportMesh(tr(""), mp, false);
			if (!saved)
			{
				QString msg = tr("Mesh layer ") + mp->label() + tr(" cannot be saved on a file.\nProject \"") + meshDoc()->getDocLabel() + tr("\" saving has been aborted.");
				//QMessageBox::warning(this, tr("Project as Saving Aborted"), msg);
                UiUtilityTools::getInstance()->showInfoMessageBox(tr("Info"), msg);
				return;
			}
		}
	}
	QFileDialog* saveDiag = new QFileDialog(this, tr("Save Project As File"), lastUsedDirectory.path().append(""), tr("Tooth Segment Project (*.mlp);;Tooth Align Project (*.aln)"));
	saveDiag->exec();
	QStringList files = saveDiag->selectedFiles();
	if (files.size() != 1)
		return;
	QString fileName = files[0];
	// this change of dir is needed for subsequent textures/materials loading
	QFileInfo fi(fileName);
	if (fi.isDir())
		return;
	if (fi.suffix().isEmpty())
	{
		QRegExp reg("\\.\\w+");
		saveDiag->selectedNameFilter().indexOf(reg);
		QString ext = reg.cap();
		fileName.append(ext);
		fi.setFile(fileName);
	}
	QDir::setCurrent(fi.absoluteDir().absolutePath());

	/*********WARNING!!!!!! CHANGE IT!!! ALSO IN THE OPENPROJECT FUNCTION********/
	meshDoc()->setDocLabel(fileName);
	/****************************************************************************/

	bool ret;
	qDebug("Saving aln file %s\n", qPrintable(fileName));
	if (fileName.isEmpty()) return;
	else
	{
		//save path away so we can use it again
		QString path = fileName;
		path.truncate(path.lastIndexOf("/"));
		lastUsedDirectory.setPath(path);
	}
	if (QString(fi.suffix()).toLower() == "aln")
	{
		ret = MeshDocumentToXMLFile(*meshDoc(), fileName, false, false);
	}
	else
	{
		ret = MeshDocumentToXMLFile(*meshDoc(), fileName, false);
	}

	if (!ret)
    {
        UiUtilityTools::getInstance()->showInfoMessageBox(tr("Error"), tr("Unable to save project file %1").arg(fileName));
    }

}

void MainWindow::closeProject()
{
	emit clearUI();

	// Save current project before closing
	saveProject();

	bool bNeedSaveAlignProjectAuto = false;
	GLA()->setAlignProjectToSaveAuto(bNeedSaveAlignProjectAuto);
	GLA()->setSegmentProjectToSaveAuto(bNeedSaveAlignProjectAuto);

	bool bEnabledCreateModel = false;
	createModelToolBar->setEnabled(bEnabledCreateModel);
	treatmentModelToolBar->setEnabled(bEnabledCreateModel);
	// Analysis group removed from FusionAnalyser

	if (!meshDoc()->meshList.empty())
	{
		int numMesh = meshDoc()->meshList.size();
		for (int i = 0; i < numMesh; ++i)
		{
			meshDoc()->delMesh(*meshDoc()->meshList.begin());
		}
	}

	endEdit();
}

void MainWindow::canUndoOrRedoStatusCommand(bool bCanUndo, bool bCanRedo)
{
	unDoAct->setEnabled(bCanUndo);
	reDoAct->setEnabled(bCanRedo);
}

void MainWindow::unDo()
{
	emit PSIGNALMANAGER->unDoSignal();
}

void MainWindow::reDo()
{
	emit PSIGNALMANAGER->reDoSignal();
}

void MainWindow::autoSaveProject(bool bAlignProject /* = true */)
{

}

void MainWindow::doToothAdjustPluginOnceSlot()
{
	autoSaveProject(false);
    if (this->projectParser != nullptr)
    {
        this->projectParser->parseAlnData(GLA()->md());
    }

}
bool MainWindow::openZip(QString fileName)
{
    return false;
}

bool MainWindow::openFas(QString fileName, bool submit)
{
    if (projectParser)
    {
        PFusionAlignData->clearSolutionData();
        QString patientName;
        PFusionAlignData->setFusionSlnFileName(fileName);
        projectParser->parseFusionSlnData(fileName, patientName, PFusionAlignData->getAlignProjs());

        QString projFileName;
        if (projectParser->getRencentProjFromSolution(projFileName, submit))
        {

            openProject(projFileName);

            sAlignProj *proj = nullptr;
            if (PFusionAlignData->getAlignProjectByName(projFileName, proj))
            {
                if (proj)
                {
                    p_fusion_title_bar_->setScheduleName(proj->scheme_name_);
                }
            }
        }
        return true;
    }
    return false;
}
bool MainWindow::openProject(QString fileName)
{
	if (fileName.isEmpty())
	{
		fileName =
			QFileDialog::getOpenFileName(this, tr("Open Project File"), lastUsedDirectory.path(),
				tr("All Project Files (*.mlp *.fa);"));
	}

	if (fileName.isEmpty())
	{
		QString errorMsgFormat = tr("Empty file name");
		logInstance->log->info(errorMsgFormat.toStdString());
		logInstance->log->flush();
		return false;
	}
	PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageNone;
	onChangeAnaStage(PFusionAlignData->getAnalyserData().cur_proj_stage_);
	emit PSIGNALMANAGER->photosPatientFileSourcePathSignal(PFusionAlignData->getProjectFilePath());
	if (meshDoc()->meshList.size() > 0 && !clearOldProject())
	{
		return false;
	}

	p_fusion_title_bar_->setPatientName("");
	setHomePageVisible(false);
	emit clearUI();

	QFileInfo fi(fileName);
	lastUsedDirectory = fi.absoluteDir();

	if ((fi.suffix().toLower() != "aln") && (fi.suffix().toLower() != "mlp") && (fi.suffix().toLower() != "fa"))
	{
        QString errorMsgFormat = tr("file %1 is an unknown project type");
        UiUtilityTools::getInstance()->showInfoMessageBox(tr("Error"), errorMsgFormat.arg(fileName));
		logInstance->log->info(errorMsgFormat.toStdString());
		logInstance->log->flush();
		return false;
	}

	bool activeEmpty = meshDoc()->meshList.empty();

	if (!activeEmpty)
	{
		if (!meshDoc()->meshList.empty())
		{
			int numMesh = meshDoc()->meshList.size();
			for (int i = 0; i < numMesh; ++i)
			{
				meshDoc()->delMesh(*meshDoc()->meshList.begin());
			}

			QString errorMsgFormat = tr("Delete old imported model information");
			logInstance->log->info(errorMsgFormat.toStdString());
			logInstance->log->flush();
		}
	}

	meshDoc()->setFileName(fileName);
	meshDoc()->setDocLabel(fileName);
	meshDoc()->setBusy(true);

	// this change of dir is needed for subsequent textures/materials loading
	QDir::setCurrent(fi.absoluteDir().absolutePath());
	qb->show();

	if (QString(fi.suffix()).toLower() == "mlp" || QString(fi.suffix()).toLower() == "fa")
	{
		if (!MeshDocumentFromXML(*meshDoc(), fileName))
		{
            UiUtilityTools::getInstance()->showInfoMessageBox(tr("Error"), tr("can not open fa file"));
			QString errorMsgFormat = tr("Could not open split project MLP file");
			logInstance->log->info(errorMsgFormat.toStdString());
			logInstance->log->flush();

			if (!meshDoc()->meshList.empty())
			{
				int numMesh = meshDoc()->meshList.size();
				for (int i = 0; i < numMesh; ++i)
				{
					meshDoc()->delMesh(*meshDoc()->meshList.begin());
				}
			}

			meshDoc()->setFileName("");
			meshDoc()->setDocLabel("");
			meshDoc()->setBusy(false);
			onChangeProjStageSlot(E_ProjStageNone);

			return false;
		}
		if (meshDoc()->meshList.empty())
		{
			UiUtilityTools::getInstance()->showInfoMessageBox(tr("Info"), tr("There is no model in the current measurement record, please import the model again."));
			PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageNone;
			onChangeAnaStage(PFusionAlignData->getAnalyserData().cur_proj_stage_);
			setHomePageVisible(true);
			meshDoc()->setBusy(false);
			return false;
		}

		int indexSuffix = fileName.lastIndexOf(".");

		for (int i = 0; i < meshDoc()->meshList.size(); i++)
		{
			QString fullPath = meshDoc()->meshList[i]->fullName();
			Matrix44m trm = this->meshDoc()->meshList[i]->cm.Tr; // save the matrix, because loadMeshClear it...
			if (!loadMeshWithStandardParams(fullPath, this->meshDoc()->meshList[i], trm))
			{
				meshDoc()->delMesh(meshDoc()->meshList[i]);
				return false;
			}

			if(meshDoc()->meshList[i])
			{
				if (meshDoc()->meshList[i]->upperOrLowerToothModelMark == UpperCTRootModel ||
					meshDoc()->meshList[i]->upperOrLowerToothModelMark == LowerCTRootModel ||
					meshDoc()->meshList[i]->upperOrLowerToothModelMark == UpperJawBoneMesh)
				{
					meshDoc()->meshList[i]->visible = false;
				}
			}
		}

		// Assign boundary
		parseBoundaryPtsFromIndex();
	}

	meshDoc()->setBusy(false);

	if (this->GLA() == 0)  return false;

	MultiViewer_Container* mvc = currentViewContainer();
	if (mvc != NULL)
	{
		mvc->resetAllTrackBall();
		mvc->updateAllDecoratorsForAllViewers();
	}

	setCurrentMeshBestTab();
	qb->reset();
	saveRecentProjectList(fileName);

	emit updateProgressBar(0, QStringLiteral(""));
	updateDentalAnalysisDataSlot(true);

	p_fusion_title_bar_->setPatientName(PFusionAlignData->getShortProjectName());
	onChangeAnaStage(PFusionAlignData->getAnalyserData().cur_proj_stage_);
	return true;
}

bool  MainWindow::readFeatureSignData(QString _fileName)
{
	// Separation functionality removed - not used in FusionAnalyser
	return false;
}

void MainWindow::initialDentalAnalysisDataFeatureMode(bool _read_mlp_record)
{
	// Separation functionality removed - not used in FusionAnalyser
}

void MainWindow::updateDentalAnalysisDataSlot(bool _read_mlp_record)
{
		emit PSIGNALMANAGER->updateAvaliableItemActionsSignal(vector<EditItemIndex>());
}

void MainWindow::setCrowdingAnalysisModeSlot(bool _six_to_six_mode)
{
	if (PFusionAlignData)
	{
		PFusionAlignData->setCrowdingAnalysisModeSlot(_six_to_six_mode);
	}
}

void MainWindow::setCurrentMeshBestTab()
{
	MultiViewer_Container* mvc = currentViewContainer();
	if (mvc != NULL)
	{
		MLSceneGLSharedDataContext* cont = mvc->sharedDataContext();
		if ((GLA() != NULL) && (meshDoc() != NULL) && (meshDoc()->mm() != NULL))
		{
			MLRenderingData dt;
			cont->getRenderInfoPerMeshView(meshDoc()->mm()->id(), GLA()->context(), dt);
		}
	}
}

void MainWindow::createEmptyProject(const QString& projName)
{
	if (gpumeminfo == NULL)
		return;
	MultiViewer_Container* mvcont = new MultiViewer_Container(*gpumeminfo, mwsettings.highprecision, mwsettings.perbatchprimitives, mwsettings.minpolygonpersmoothrendering, this);
	connect(&mvcont->meshDoc, SIGNAL(meshAdded(int)), this, SLOT(meshAdded(int)));
	connect(&mvcont->meshDoc, SIGNAL(meshRemoved(int)), this, SLOT(meshRemoved(int)));
	connect(&mvcont->meshDoc, SIGNAL(documentUpdated()), this, SLOT(documentUpdateRequested()));
	connect(mvcont, SIGNAL(closingMultiViewerContainer()), this, SLOT(closeCurrentDocument()));

	SAFE_DELETE(_currviewcontainer);
	_currviewcontainer = mvcont;
	//setCentralWidget(_currviewcontainer);
	_currviewcontainer->setAcceptDrops(true);
	connect(mvcont, SIGNAL(updateMainWindowMenus()), this, SLOT(updateMenus()));
	connect(&mvcont->meshDoc.Log, SIGNAL(logUpdated()), this, SLOT(updateLog()));

	GLArea* gla = new GLArea(this, mvcont, &currentGlobalParams);
    {
        connect(this, &MainWindow::clearUI,gla, &GLArea::clearUI);

        // Analysis group connections removed from FusionAnalyser
        connect(this, &MainWindow::setSwitchToMainMenu,
            gla, &GLArea::weatherSwitchToMainMenu);
        connect(this, &MainWindow::setParallelSwitched,
            gla, &GLArea::weatherParallelSwitched);

        connect(gla, SIGNAL(autoSaveProjectSignal(bool)),
            this, SLOT(autoSaveProject(bool)));

        connect(gla, SIGNAL(importMeshWithLayerManagement(QString)),
            this, SLOT(importMeshWithLayerManagement(QString)));

        //connect(frontViewActCB, SIGNAL(triggered()),
        //    gla, SLOT(showFrontView()));
        //connect(backViewActCB, SIGNAL(triggered()),
        //    gla, SLOT(showBackView()));
        //connect(topViewActCB, SIGNAL(triggered()),
        //    gla, SLOT(showTopView()));
        //connect(bottomViewActCB, SIGNAL(triggered()),
        //    gla, SLOT(showBottomView()));
        //connect(leftViewActCB, SIGNAL(triggered()),
        //    gla, SLOT(showLeftView()));
        //connect(rightViewActCB, SIGNAL(triggered()),
        //    gla, SLOT(showRightView()));

        //connect(occlusionToothAct, SIGNAL(triggered()),
        //    gla, SLOT(occlusionShow()));
        //connect(gla, SIGNAL(closeOcclusionToothAct()),
        //    occlusionToothAct, SIGNAL(triggered()));
        //connect(treatmentCompareAct, SIGNAL(triggered()),
        //    gla, SLOT(dentalCompareModelShow()));
        //connect(gla, SIGNAL(closeTreatmentCompareAct()),
        //    treatmentCompareAct, SIGNAL(triggered()));
    }
	mvcont->main_gla_ = gla;
	mvcont->addView(gla, Qt::Horizontal);

	if (projName.isEmpty())
	{
		mvcont->meshDoc.setDocLabel(QString(""));
	}
	else
		mvcont->meshDoc.setDocLabel(projName);

	mvcont->setWindowTitle(mvcont->meshDoc.getDocLabel());

	mvcont->showMaximized();

	if (gla->bEnableShadowMap)
	{
		gla->setDecorator(QString("Enable shadow mapping"), gla->bEnableShadowMap);
	}
}

void MainWindow::newProject(const QString& projName)
{
	if (importMeshWithLayerManagement())
	{
		bool bNeedSaveAlignProjectAuto = false;
		GLA()->setAlignProjectToSaveAuto(bNeedSaveAlignProjectAuto);
		GLA()->setSegmentProjectToSaveAuto(!bNeedSaveAlignProjectAuto);

		PFusionAlignData->setProjectFilePath(lastUsedDirectory.absolutePath());
		emit PSIGNALMANAGER->photosPatientFileSourcePathSignal(PFusionAlignData->getProjectFilePath());
	}
	else
	{
		bool bNeedSaveAlignProjectAuto = false;
		GLA()->setAlignProjectToSaveAuto(bNeedSaveAlignProjectAuto);
		GLA()->setSegmentProjectToSaveAuto(bNeedSaveAlignProjectAuto);
	}
	autoSaveProject(false);
	readAutoFeatureResult("");
}
void MainWindow::onNewAlignProjectSlot()
{

}

void MainWindow::onSaveAlignProjectSlot()
{
    QString alignProjFileName = PFusionAlignData->getAlignProjFileName();
	if (alignProjFileName == PFusionAlignData->getAlignProjOriFileName())
	{
		onSaveAlignProjectAsSlot();
		return;
	}
    if (!alignProjFileName.isEmpty())
    {
		QFileInfo fileInfo(alignProjFileName);
		QString alignProj = fileInfo.baseName();

        sAlignProj *proj = nullptr;
        if (PFusionAlignData->getAlignProjectByName(alignProj, proj))
        {
            if (proj->type_ == sAlignProj::EALIGNPROJTYPE_SUBMIT)
                UiUtilityTools::getInstance()->showInfoMessageBox(tr("Error"), tr("can not save the submit project"));
            else
            {
                proj->time_ = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                projectParser->saveProjToFas(PFusionAlignData->getAlignProjs());
                saveProject(alignProjFileName);
            }
        }
        else
        {
            sAlignProj proj;
            proj.file_name_ = fileInfo.fileName();
            proj.scheme_name_ = alignProj;
            proj.time_ = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            proj.type_ = sAlignProj::EALIGNPROJTYPE_TEMPORARY;
            PFusionAlignData->getTempAlignProjs().push_back(proj);
            projectParser->saveProjToFas(PFusionAlignData->getAlignProjs());
            saveProject(alignProjFileName);
        }
        // FusionHistorySchemeDlg removed - file deleted
    }
}

void MainWindow::onSaveSubmitAlignProjectSlot()
{
	QString projFileName = PFusionAlignData->getAlignProjFileName();
	QFileInfo fileInfo(projFileName);
	QString projName = fileInfo.baseName();

	sAlignProj *proj;
	if (PFusionAlignData->getAlignProjectByName(projName, proj))
	{
		if (proj->type_ == sAlignProj::EALIGNPROJTYPE_TEMPORARY)
		{
			sAlignProj tempProj = *proj;
			PFusionAlignData->getTempAlignProjs().removeOne(*proj);
			PFusionAlignData->getTempAlignProjs();
			int index = PFusionAlignData->getSubmitAlignProjs().size();
			tempProj.scheme_name_ = tr("treatment scheme ") + QString::number(index + 1);
			tempProj.type_ = sAlignProj::EALIGNPROJTYPE_SUBMIT;
			PFusionAlignData->getSubmitAlignProjs().push_back(tempProj);
		}

		projectParser->saveProjToFas(PFusionAlignData->getAlignProjs());
	}

}
// ProjectHandleDlg and FusionHistorySchemeDlg removed - files deleted
void MainWindow::onSaveAlignProjectAsSlot()
{
    QString alignProjFileName = PFusionAlignData->getAlignProjFileName();
    if (!alignProjFileName.isEmpty())
    {
        // ProjectHandleDlg removed - functionality disabled
        /*
        if (p_project_name_handle_dlg_ == nullptr)
            p_project_name_handle_dlg_ = new  ProjectHandleDlg(this);
        p_project_name_handle_dlg_->setWindowTitle(tr("save as"));
        int rec = p_project_name_handle_dlg_->exec();
		if (rec == QDialog::Accepted)
		{
			if (!p_project_name_handle_dlg_->getProjectName().isEmpty())
			{
				QString alignProj = p_project_name_handle_dlg_->getProjectName();
				sAlignProj proj;
				proj.file_name_ = alignProj + ".aln";
				proj.scheme_name_ = alignProj;
				proj.time_ = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
				proj.type_ = sAlignProj::EALIGNPROJTYPE_TEMPORARY;
				PFusionAlignData->getTempAlignProjs().push_back(proj);
				PFusionAlignData->setAlignProjFileName(proj.file_name_);
				projectParser->saveProjToFas(PFusionAlignData->getAlignProjs());
				saveProject(p_project_name_handle_dlg_->getProjectName());
			}
		}
        if (p_history_schemes_dlg_)p_history_schemes_dlg_->updateTableView();
        */
    }
}

// FusionHistorySchemeDlg removed - file deleted
void MainWindow::onOpenHistorySchemesSlot()
{
    // FusionHistorySchemeDlg removed - functionality disabled
    /*
    if(p_history_schemes_dlg_ == nullptr)
    {
        p_history_schemes_dlg_ = new FusionHistorySchemeDlg(this);
        connect(p_history_schemes_dlg_, &FusionHistorySchemeDlg::reNameProjSignal, this, &MainWindow::onRenameHistroyAlignProjSlot);
        connect(p_history_schemes_dlg_, &FusionHistorySchemeDlg::deleteProjSignal, this, &MainWindow::onDeleteHistoryAlignProjSlot);
        connect(p_history_schemes_dlg_, &FusionHistorySchemeDlg::openProjSignal, this, &MainWindow::onOpenHistoryAlignProjSlot);
		p_history_schemes_dlg_->setSkin();
    }
	p_history_schemes_dlg_->updateTableView();
	p_history_schemes_dlg_->show();
	p_history_schemes_dlg_->setSkin();
    */
}
// FusionHistorySchemeDlg removed - file deleted
void MainWindow::onOpenHistoryAlignProjSlot(QString projName)
{
    sAlignProj *proj = nullptr;
    if (PFusionAlignData->getAlignProjectByName(projName, proj))
    {
        openProject(proj->file_name_);
        p_fusion_title_bar_->setScheduleName(projName);
    }
}

// ProjectHandleDlg and FusionHistorySchemeDlg removed - files deleted
void MainWindow::onRenameHistroyAlignProjSlot(QString projName)
{
    sAlignProj *proj = nullptr;
    if (PFusionAlignData->getAlignProjectByName(projName, proj))
    {
        if (proj->type_ == sAlignProj::EALIGNPROJTYPE_SUBMIT)
        {
            UiUtilityTools::getInstance()->showInfoMessageBox(tr("Error"), tr("cannot rename submit project"));
            return;
        }
        else if (proj->type_ == sAlignProj::EALIGNPROJTYPE_TEMPORARY)
        {
            // ProjectHandleDlg removed - functionality disabled
            /*
            if(p_project_name_handle_dlg_ == nullptr)
            {
                p_project_name_handle_dlg_ = new ProjectHandleDlg(this);
            }
            p_project_name_handle_dlg_->setWindowTitle(tr("rename"));
            int rec = p_project_name_handle_dlg_->exec();
            if(rec == QDialog::Accepted)
            {
                proj->scheme_name_ = p_project_name_handle_dlg_->getProjectName();
                proj->time_ = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                QFile file(proj->file_name_);
                proj->file_name_ = proj->scheme_name_ + ".aln";
                file.rename(proj->file_name_);

                if (p_history_schemes_dlg_)
                    p_history_schemes_dlg_->updateTableView();

                projectParser->saveProjToFas(PFusionAlignData->getAlignProjs());
            }
            */
        }

    }
}

void MainWindow::onDeleteHistoryAlignProjSlot(QString projName)
{
    sAlignProj *proj = nullptr;
    if (PFusionAlignData->getAlignProjectByName(projName, proj))
    {
        if (proj->type_ == sAlignProj::EALIGNPROJTYPE_SUBMIT)
        {
            UiUtilityTools::getInstance()->showInfoMessageBox(tr("Error"), tr("cannot delete submit project"));
            return;
        }
        else if (proj->type_ == sAlignProj::EALIGNPROJTYPE_TEMPORARY)
        {
            auto &tempProjs = PFusionAlignData->getTempAlignProjs();
            if (UiUtilityTools::getInstance()->showQuestionMessageBox(tr("Info"), tr("whether delete the %1 project").arg(projName)) > 0)
            {
                QVector<sAlignProj>::iterator it = tempProjs.begin();
                for (; it != tempProjs.end(); ++it)
                {
                    if (it->scheme_name_ == proj->scheme_name_)
                        break;
                }

                QFile::remove(proj->file_name_);
                tempProjs.erase(it);

                projectParser->saveProjToFas(PFusionAlignData->getAlignProjs());
            }
        }

    }
}

void MainWindow::documentUpdateRequested()
{
	if (meshDoc() == NULL)
		return;
	for (int ii = 0; ii < meshDoc()->meshList.size(); ++ii)
	{
		MeshModel* mm = meshDoc()->meshList[ii];
		if (mm != NULL)
		{
			addRenderingDataIfNewlyGeneratedMesh(mm->id());
			if (currentViewContainer() != NULL)
			{
				currentViewContainer()->resetAllTrackBall();
				currentViewContainer()->updateAllViewers();
			}
		}
	}
}
bool MainWindow::loadMesh(const QString& fileName, MeshIOInterface* pCurrentIOPlugin, MeshModel* mm, int& mask, RichParameterSet* prePar, const Matrix44m& mtr, bool isareload)
{
	if ((GLA() == NULL) || (mm == NULL))
		return false;

	QFileInfo fi(fileName);
	QString extension = fi.suffix();
    QString errorMsg;
	if (!fi.exists())
	{
        errorMsg = tr("can not open file : %1 is not existed").arg(fileName);
        UiUtilityTools::getInstance()->showInfoMessageBox(tr("Info"),errorMsg);
		setHomePageVisible(true);
		PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageNone;
		onChangeAnaStage(E_ProjAnaStageNone);
		logInstance->log->info(errorMsg.toStdString());
		logInstance->log->flush();
		meshDoc()->setBusy(false);
		return false;
	}
	if (!fi.isReadable())
	{

        errorMsg = tr("can not open file : %1 can not be read").arg(fileName);
        UiUtilityTools::getInstance()->showInfoMessageBox(tr("Info"), errorMsg);

		logInstance->log->info(errorMsg.toStdString());
		logInstance->log->flush();
		meshDoc()->setBusy(false);
		return false;
	}

	// this change of dir is needed for subsequent textures/materials loading
	QDir::setCurrent(fi.absoluteDir().absolutePath());

	// retrieving corresponding IO plugin
	if (pCurrentIOPlugin == 0)
	{
        errorMsg = tr("can not open file : %1 format not supported ").arg(fileName);
        UiUtilityTools::getInstance()->showInfoMessageBox(tr("Info"), errorMsg);

		logInstance->log->info(errorMsg.toStdString());
		logInstance->log->flush();
		meshDoc()->setBusy(false);
		return false;
	}
	meshDoc()->setBusy(true);
	pCurrentIOPlugin->setLog(&meshDoc()->Log);

	if (!pCurrentIOPlugin->open(extension, fileName, *mm, mask, *prePar, QCallBack, this /*gla*/))
	{

        errorMsg = tr("the opening file %1 is error.").arg(fileName) + pCurrentIOPlugin->errorMsg();
        UiUtilityTools::getInstance()->showInfoMessageBox(tr("Info"), errorMsg);

		pCurrentIOPlugin->clearErrorString();
		meshDoc()->setBusy(false);

		logInstance->log->info(errorMsg.toStdString());
		logInstance->log->flush();
		return false;
	}

	QString err = pCurrentIOPlugin->errorMsg();
	if (!err.isEmpty())
	{
        errorMsg = tr("the opening file %1 is error.").arg(fileName) + pCurrentIOPlugin->errorMsg();
        UiUtilityTools::getInstance()->showInfoMessageBox(tr("Info"), errorMsg);

		pCurrentIOPlugin->clearErrorString();
		logInstance->log->info(errorMsg.toStdString());
		logInstance->log->flush();
	}

	saveRecentFileList(fileName);

	if (!(mm->cm.textures.empty()))
		updateTexture(mm->id());

	// In case of polygonal meshes the normal should be updated accordingly
	if (mask & vcg::tri::io::Mask::IOM_BITPOLYGONAL)
	{
	} // standard case
	else
	{
		vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(mm->cm);
		if (!(mask & vcg::tri::io::Mask::IOM_VERTNORMAL))
			vcg::tri::UpdateNormal<CMeshO>::PerVertexAngleWeighted(mm->cm);
	}

	vcg::tri::UpdateBounding<CMeshO>::Box(mm->cm);					// updates bounding box
	if (mm->cm.fn == 0 && mm->cm.en == 0)
	{
		if (mask & vcg::tri::io::Mask::IOM_VERTNORMAL)
			mm->updateDataMask(MeshModel::MM_VERTNORMAL);
	}

	if (mm->cm.fn == 0 && mm->cm.en > 0)
	{
		if (mask & vcg::tri::io::Mask::IOM_VERTNORMAL)
			mm->updateDataMask(MeshModel::MM_VERTNORMAL);
	}

	updateMenus();

	mm->cm.Tr = mtr;

	UtilityTools::getInstance()->removeMeshDegenerateVertexAndFaceData(&mm->cm);

	computeRenderingDataOnLoading(mm, isareload);

	meshDoc()->setBusy(false);

	return true;
}

void MainWindow::computeRenderingDataOnLoading(MeshModel* mm, bool isareload)
{
	MultiViewer_Container* mv = currentViewContainer();
	if (mv != NULL)
	{
		MLSceneGLSharedDataContext* shared = mv->sharedDataContext();
		if ((shared != NULL) && (mm != NULL))
		{
			MLRenderingData defdt;
			MLPoliciesStandAloneFunctions::suggestedDefaultPerViewRenderingData(mm, defdt, mwsettings.minpolygonpersmoothrendering);
			for (int glarid = 0; glarid < mv->viewerCounter(); ++glarid)
			{
				GLArea* ar = mv->getViewer(glarid);
				if (ar != NULL)
				{

					if (isareload)
					{
						MLRenderingData currentdt;
						shared->getRenderInfoPerMeshView(mm->id(), ar->context(), currentdt);
						MLRenderingData newdt;
						MLPoliciesStandAloneFunctions::computeRequestedRenderingDataCompatibleWithMeshSameGLOpts(mm, currentdt, newdt);
						MLPoliciesStandAloneFunctions::setPerViewGLOptionsAccordindToWireModality(mm, newdt);
						MLPoliciesStandAloneFunctions::setBestWireModality(mm, newdt);
						shared->setRenderingDataPerMeshView(mm->id(), ar->context(), newdt);
						shared->meshAttributesUpdated(mm->id(), true, MLRenderingData::RendAtts(true));
					}
					else
						shared->setRenderingDataPerMeshView(mm->id(), ar->context(), defdt);
				}
			}
			shared->manageBuffers(mm->id());
		}
	}
}

bool MainWindow::clearOldProject()
{
	clearDecoratorState();
	endEdit();
	if (!meshDoc()->meshList.empty())
	{

		int numMesh = meshDoc()->meshList.size();
		for (int i = 0; i < numMesh; ++i)
		{
			meshDoc()->delMesh(*meshDoc()->meshList.begin());
		}
	}

    PFusionAlignData->clearData();
    if (GLA())
    {
		GLA()->getViewGui()->onlyShowUpperDentalSlot(false);
        GLA()->getViewGui()->onlyShowLowerDentalSlot(false);
    }
    if(p_align_toolbar_)
    {
        for (auto btn : v_decorate_temp_btns_)
        {
            btn->setChecked(false);
            for (auto action : btn->actions())
            {
                action->setChecked(false);
                action->triggered(false);
            }

        }
        v_decorate_temp_btns_.clear();

        p_align_toolbar_->setEnabled(false);
    }

	p_fusion_title_bar_->setPatientName("");

	return true;

}

bool MainWindow::loadMeshModel(const QString& fileName)
{
	SPDLOG_LINE
	QFileInfo fileInfo(fileName);

	if (!(fileInfo.exists() && fileInfo.isFile()))
	{
		UiUtilityTools::getInstance()->showInfoMessageBox(tr("info"), tr("File %1 is error").arg(fileName));
		return false;
	}
	SPDLOG_LINE
	QDir::setCurrent(fileInfo.absoluteDir().absolutePath());

	QString extension = fileInfo.suffix();
	MeshIOInterface* pCurrentIOPlugin = PM.allKnowInputFormats[extension.toLower()];
	SPDLOG_LINE

	if (pCurrentIOPlugin == NULL)
	{
		QString errorMsg = tr("can not open file : %1 format not supported ").arg(fileName);
		UiUtilityTools::getInstance()->showInfoMessageBox(tr("Info"), errorMsg);
		return false;
	}

	MeshModel* mm = meshDoc()->addNewMesh(fileName, fileInfo.fileName(), true);
	SPDLOG_LINE

	QString fullPath = mm->fullName();
	Matrix44m trm = mm->cm.Tr; // save the matrix, because loadMeshClear it...
	if (!loadMeshWithStandardParams(fullPath, mm, trm))
	{
		meshDoc()->delMesh(mm);
		return false;
	}

	SPDLOG_LINE

	return true;
}

void MainWindow::nextStepSlot() {
	PSIGNALMANAGER->updatePluginUISignal();
	if (PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageMissLower)
	{
		PFusionAlignData->getAnalyserData().Ppre_proj_stage_ = PFusionAlignData->getAnalyserData().pre_proj_stage_;
		PFusionAlignData->getAnalyserData().pre_proj_stage_ = PFusionAlignData->getAnalyserData().cur_proj_stage_;
		PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageFixLower;
		onChangeAnaStage(PFusionAlignData->getAnalyserData().cur_proj_stage_);
	}
	else {
		PFusionAlignData->getAnalyserData().pre_proj_stage_ = E_ProjAnaStageMissUpper;
		PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageFixUpper;
		onChangeAnaStage(PFusionAlignData->getAnalyserData().cur_proj_stage_);
	}
	emit PSIGNALMANAGER->setCurrentMarkingStatusSignal(true);
}

void MainWindow::PreviousStepSlot() {
	emit skipupSignal();
	GLA()->resetTrackBall();
	emit PSIGNALMANAGER->setUpperDentalSelectedSignal();
	p_tooth_seg_act_->triggered();
	emit PSIGNALMANAGER->setCurrentMarkingStatusSignal(false);
	GLA()->Skip_the_upper_jaw->show();
	GLA()->Previous_step->hide();
	PSIGNALMANAGER->updatePluginUISignal();
}

void MainWindow::forwardStepSlot()
{
	if (PFusionAlignData->getAnalyserData().pre_proj_stage_ == E_ProjAnaStageMissUpper)
	{
//		qDebug() << "forwardStep" << 1;
		emit PSIGNALMANAGER->setUpperDentalSelectedSignal();
//		p_tooth_seg_act_->triggered();
		emit PSIGNALMANAGER->setCurrentMarkingStatusSignal(false);
		onChangeAnaStage(PFusionAlignData->getAnalyserData().pre_proj_stage_);
		PFusionAlignData->getAnalyserData().cur_proj_stage_ = PFusionAlignData->getAnalyserData().pre_proj_stage_;
		PFusionAlignData->getAnalyserData().pre_proj_stage_ = E_ProjAnaStageNone;
//		GLA()->nowStage = GLA()->preStage;
//		GLA()->preStage = E_ProjAnaStageImport;
	}
	else if (PFusionAlignData->getAnalyserData().pre_proj_stage_ == E_ProjAnaStageFixUpper)
	{
//		qDebug() << "forwardStep" << 2;
		emit PSIGNALMANAGER->setUpperDentalSelectedSignal();
		emit PSIGNALMANAGER->setCurrentMarkingStatusSignal(true);
		onChangeAnaStage(PFusionAlignData->getAnalyserData().pre_proj_stage_);
		PFusionAlignData->getAnalyserData().cur_proj_stage_ = PFusionAlignData->getAnalyserData().pre_proj_stage_;
		PFusionAlignData->getAnalyserData().pre_proj_stage_ = E_ProjAnaStageMissUpper;
	}
	else if (PFusionAlignData->getAnalyserData().pre_proj_stage_ == E_ProjAnaStageMissLower)
	{
		qDebug() << "forwardStep" << 3;
		emit PSIGNALMANAGER->setLowerDentalSelectedSignal();
		emit PSIGNALMANAGER->setCurrentMarkingStatusSignal(false);
		onChangeAnaStage(PFusionAlignData->getAnalyserData().pre_proj_stage_);
		PFusionAlignData->getAnalyserData().cur_proj_stage_ = PFusionAlignData->getAnalyserData().pre_proj_stage_;
		PFusionAlignData->getAnalyserData().pre_proj_stage_ = PFusionAlignData->getAnalyserData().Ppre_proj_stage_;
	}
}

void MainWindow::SkipUpperJaw()
{
	emit skipupSignal();
	GLA()->resetTrackBall();
	emit PSIGNALMANAGER->setLowerDentalSelectedSignal();
//	GLA()->next_step_button->show();
//	GLA()->forward_step_button_->hide();
	p_tooth_seg_act_->triggered();
	emit PSIGNALMANAGER->setCurrentMarkingStatusSignal(false);
//	GLA()->Skip_the_upper_jaw->hide();
//	GLA()->Previous_step->show();
	PFusionAlignData->getAnalyserData().Ppre_proj_stage_ = PFusionAlignData->getAnalyserData().pre_proj_stage_;
	PFusionAlignData->getAnalyserData().pre_proj_stage_ = PFusionAlignData->getAnalyserData().cur_proj_stage_;
    PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageMissLower;
	onChangeAnaStage(PFusionAlignData->getAnalyserData().cur_proj_stage_);
	PSIGNALMANAGER->updatePluginUISignal();
}

void MainWindow::FixedPointAnalysisSlot() {
	if (PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageSignalComplete || PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageDualComplete)
	{
		if (UiUtilityTools::getInstance()->showQuestionMessageBox(tr("info"), tr("All points on this model have been marked, would you like to edit it ?"), 0, QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No), QMessageBox::StandardButton(QMessageBox::Yes)) <=0)
		{
			return;
		}

	}
    emit PSIGNALMANAGER->setUpperDentalSelectedSignal();
	p_tooth_seg_act_->triggered();
	emit PSIGNALMANAGER->setCurrentMarkingStatusSignal(false);
	if (meshDoc()->meshList.size() == 1)
	{
		PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageMissLower;
	}
	else
	{
		PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageMissUpper;
	}
	onChangeAnaStage(PFusionAlignData->getAnalyserData().cur_proj_stage_);
	PSIGNALMANAGER->updatePluginUISignal();
//	GLA()->next_step_button->show();
/*	p_align_toolbar_->setEnabled(false);

	if (meshDoc()->meshList.size() == 1)
	{
		GLA()->Fixed_point_analysis->hide();
		GLA()->complete_btn->show();
		GLA()->Import_jaw_alignment->hide();
		return;
	}
	GLA()->Import_jaw_alignment->hide();
	GLA()->Skip_the_upper_jaw->show();
	GLA()->Fixed_point_analysis->hide();
	GLA()->complete_btn->show();  */
}
void MainWindow::loadJawMoelsSlot(QString& models_file)
{

	for (auto& it : meshDoc()->meshList)
	{
		if (it->fullName() == models_file)
		{
			UiUtilityTools::getInstance()->showInfoMessageBox(tr("info"), tr("This model has been imported already. Please select another model to import"));
			p_align_toolbar_->setEnabled(true);
//			PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageSignalNotComplete;
			onChangeAnaStage(PFusionAlignData->getAnalyserData().cur_proj_stage_);
			return;
		}
	}
	bool load_success = true;
	load_success &= loadMeshModel(models_file);

	if (!load_success || models_file.isEmpty())
	{
		p_align_toolbar_->setEnabled(true);
//		PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageSignalNotComplete;
		onChangeAnaStage(PFusionAlignData->getAnalyserData().cur_proj_stage_);
		return;
	}
	if (_currviewcontainer != NULL)
	{
		_currviewcontainer->resetAllTrackBall();
		_currviewcontainer->updateAllDecoratorsForAllViewers();
	}
	qb->reset();
	GLA()->setNeedSortModels(false);
	emit PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
	qDebug() << meshDoc()->meshList.size();
	meshDoc()->meshList[1]->upperOrLowerToothModelMark = ((meshDoc()->meshList[0]->upperOrLowerToothModelMark == UpperToothModel) ? LowerToothModel : UpperToothModel);
	qDebug() << meshDoc()->meshList[0]->upperOrLowerToothModelMark;
	qDebug() << meshDoc()->meshList[1]->upperOrLowerToothModelMark;
	for (int i_mesh = 0; i_mesh < meshDoc()->meshList.size(); ++i_mesh)
	{
		meshDoc()->meshList[i_mesh]->visible = true;
	}

	// Import opposite jaw and allocate marker structure space
	createRecordData(false);

	bool bNeedSaveAlignProjectAuto = false;
	GLA()->setAlignProjectToSaveAuto(bNeedSaveAlignProjectAuto);
	GLA()->setSegmentProjectToSaveAuto(!bNeedSaveAlignProjectAuto);

	classifyModels();
	PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageDualNotComplete;
	onChangeAnaStage(PFusionAlignData->getAnalyserData().cur_proj_stage_);
	GLA()->setFixAnaBtnText();
	p_align_toolbar_->setEnabled(true);
}

void MainWindow::importJawSlot()
{
	GLA()->getViewGui()->onlyShowUpperDentalSlot(false);
	GLA()->getViewGui()->onlyShowLowerDentalSlot(false);
	QStringList files_name = QFileDialog::getOpenFileNames(this, tr("import model"), "", "*.stl");
	if (files_name.empty())
	{
		PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageSignalNotComplete;
		return;
	}

	if (files_name.size() > 1)
	{
		UiUtilityTools::getInstance()->showInfoMessageBox(tr("Info"), tr("Please select no more than 2 models (maxilla + mandible)!"));
		return;
	}

	QString file_name = files_name.at(0);
	//	PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageDualNotComplete;
	//emit GLA()->importJawSignal(file_name);
	GLA()->endEdit();
	//	GLA()->endAllDecorate();
	clearDecoratorState();
	onChangeAnaStage(E_ProjAnaStageNone);
	p_align_toolbar_->setEnabled(false);
	loadJawMoelsSlot(file_name);
}
void MainWindow::startAnalysisSlot()
{
	p_tooth_seg_act_->triggered();
	bool mark_over = true;

	if (meshDoc()->meshList.size() == 2 && mark_over)
	{
		PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageDualComplete;
	}
	else if (meshDoc()->meshList.size() == 2 && !mark_over)
	{
		PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageDualNotComplete;
	}
	else if (meshDoc()->meshList.size() == 1 && mark_over)
	{
		PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageSignalComplete;
	}
	else if (meshDoc()->meshList.size() == 1 && !mark_over)
	{
		PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageSignalNotComplete;
	}
	GLA()->setFixAnaBtnText();
	onChangeAnaStage(PFusionAlignData->getAnalyserData().cur_proj_stage_);
/*	if (meshDoc()->meshList.size() == 1)
	{
		GLA()->Import_jaw_alignment->show();
	}
	GLA()->Skip_the_upper_jaw->hide();
	GLA()->Previous_step->hide();
	GLA()->Fixed_point_analysis->show();
	GLA()->complete_btn->hide();  */
	GLA()->Previous_step->hide();
	GLA()->forward_step_button_->hide();
	GLA()->next_step_button->hide();
	updateDentalAnalysisDataSlot(false);
	emit PSIGNALMANAGER->triggerItemActionSignal(EditItemIndex::TEETH_WIDTH);
}

void MainWindow::loadMeshMoelsSlot(QStringList& models_file)
{
	PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageNone;
	onChangeAnaStage(PFusionAlignData->getAnalyserData().cur_proj_stage_);
	for (auto it : models_file)
	{
		SPDLOG->info(it);
	}
	if (models_file.size() == 0)
		return;
	SPDLOG->info("load Mesh model start");
	clearOldProject();
	bool load_success = true;
	SPDLOG->info("load Mesh model start 1");
	for (auto it : models_file)
	{
		SPDLOG->info(it);
		load_success &= loadMeshModel(it);
	}
	SPDLOG->info("load Mesh model start 2");

	if (!load_success || models_file.isEmpty())
	{
		return;
	}
	setHomePageVisible(false);
	if (_currviewcontainer != NULL)
	{
		_currviewcontainer->resetAllTrackBall();
		_currviewcontainer->updateAllDecoratorsForAllViewers();
	}
	qb->reset();
	GLA()->setNeedSortModels(false);
	emit PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
	// Set to show only first model and ask if it's upper jaw model
	for (int i_mesh = 0; i_mesh < meshDoc()->meshList.size(); ++i_mesh)
	{
		meshDoc()->meshList[i_mesh]->visible = i_mesh == 0 ? true : false;
	}
	if (meshDoc()->meshList.size() == 1)
	{
		PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageSignalNotComplete;
	}
	else if (meshDoc()->meshList.size() > 1)
	{
		PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageDualNotComplete;
	}
	// Show dialog asking if current mesh model is upper jaw
	tellMeThisModelIsUpper();
}

void MainWindow::isMaxillaSlot(bool ok)
{
	ToothModelType first_model_type = ok ? UpperToothModel : LowerToothModel;
	ToothModelType other_model_type = ok ? LowerToothModel : UpperToothModel;
	// Set whether first model is upper jaw model
	// And restore visibility properties for all models
	for (int i_mesh = 0; i_mesh < meshDoc()->meshList.size(); ++i_mesh)
	{
		ToothModelType type = i_mesh == 0 ? first_model_type : other_model_type;
		meshDoc()->meshList[i_mesh]->upperOrLowerToothModelMark = type;
		meshDoc()->meshList[i_mesh]->visible = true;
	}
	createMarkingProject();

	createRecordData();
	// Classify models by sorting
	classifyModels();

	// Save project
	autoSaveProject(false);
	p_fusion_title_bar_->getMainMenuBtn()->setEnabled(true);
	p_fusion_title_bar_->getMaximizeBtn()->setEnabled(true);
	p_fusion_title_bar_->getCloseBtn()->setEnabled(true);
	GLA()->getViewGui()->setEnabled(true);
	onChangeAnaStage(PFusionAlignData->getAnalyserData().cur_proj_stage_);

}

void MainWindow::tellMeThisModelIsUpper()
{
	p_fusion_title_bar_->getMainMenuBtn()->setEnabled(false);
	p_fusion_title_bar_->getMaximizeBtn()->setEnabled(false);
	p_fusion_title_bar_->getCloseBtn()->setEnabled(false);
	GLA()->getViewGui()->setEnabled(false);
	FusionMessageBox* msg_box = nullptr;
	bool ok =  UiUtilityTools::getInstance()->showQuestionNonMessageBox(tr(""), tr("Selected model:"), tr("Maxilla"),tr("Mandible"), msg_box);
	if (msg_box != nullptr)
	{
		connect(msg_box, &FusionMessageBox::accepted, [&]() {this->isMaxillaSlot(true); });
		connect(msg_box, &FusionMessageBox::rejected, [&]() {this->isMaxillaSlot(false); });
	}

}

void MainWindow::createMarkingProject()
{
	QString path_name;
	for (auto& mesh : meshDoc()->meshList)
	{
		if (mesh->upperOrLowerToothModelMark == UpperToothModel)
		{
			path_name = mesh->fullName();
//			pathName = mesh->pathName();
		}
	}

	if (path_name.isEmpty() && !meshDoc()->meshList.isEmpty())
	{
		path_name = meshDoc()->meshList.first()->fullName();
	}

	if (path_name.isEmpty())
	{
		return;
	}
//	qDebug() << path_name;
//	qDebug() << pathName;
	QString full_folder_name = path_name.left(path_name.lastIndexOf("/"));
	QString last_folder_name = full_folder_name.mid(full_folder_name.lastIndexOf("/") + 1);
	QFileInfo file_info(path_name);
	QString patient_name = file_info.fileName().remove("." + file_info.suffix());
    QDir dir(full_folder_name);
	QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs);
	patient_name = createProjectName(fileInfoList, patient_name, 0);
	QString nameProject = full_folder_name + "/" + patient_name;

	PFusionAlignData->getAnalyserData().patient_name_ = nameProject;

	p_fusion_title_bar_->setPatientName(PFusionAlignData->getShortProjectName());
}

QString MainWindow::createProjectName(QFileInfoList &fileInfoList, QString patient_name, int number)
{
	QString proName;
	if (number)
	{
		proName = patient_name + QString("(%1)").arg(number);
	}
	else
	{
		proName = patient_name;
	}
//	qDebug() << proName << 1;
	foreach(auto fileInfo, fileInfoList) {

		if (QString(".") + fileInfo.suffix() != FusionAnalyserPrjSuffix)
		{
			continue;
		}
//		qDebug() << fileInfo.fileName().remove("." + fileInfo.suffix());
//		qDebug() << proName;
		if (fileInfo.fileName().remove("." + fileInfo.suffix()) == proName)
		{
			proName = createProjectName(fileInfoList, patient_name, ++number);
//			qDebug() << proName << 2;
			return proName;
		}
	}
	return proName;
}

void MainWindow::createRecordData(bool need_empty_first)
{

}

void MainWindow::classifyModels()
{

}

void MainWindow::toggleUpperLowerMesh()
{

	bool need_waring = false;

	if (need_waring)
	{
		auto result = UiUtilityTools::getInstance()->showQuestion(this, tr(""),
			tr("The current model already has been marked, this operation clears all point and measurement information, would you like to exchange ?"));

		if (!(result == QMessageBox::Ok || result == QMessageBox::Yes))
		{
			return;
		}
	}

	if (meshDoc()->meshList.size() < 1)
	{
		return;
	}

	// Delete measurement records
	PFusionAlignData->deleteMemory();
	clearDecoratorState();
	endEdit();
	if (meshDoc()->meshList.size() == 1)
	{
		PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageSignalNotComplete;
	}
	else
	{
		PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageDualNotComplete;
	}
	for (auto& mesh : meshDoc()->meshList)
	{
		if (mesh->upperOrLowerToothModelMark == UpperToothModel)
		{
			mesh->upperOrLowerToothModelMark = LowerToothModel;
		}
		else if (mesh->upperOrLowerToothModelMark == LowerToothModel)
		{
			mesh->upperOrLowerToothModelMark = UpperToothModel;
		}
	}

	createRecordData();
	// Reclassify and initialize
	classifyModels();

	onChangeAnaStage(PFusionAlignData->getAnalyserData().cur_proj_stage_);

}

void MainWindow::removeCurrentMesh(int id_mesh)
{

	MeshModel* delete_mesh = nullptr;
	for (int i = 0; i < meshDoc()->meshList.size(); ++i)
	{
		if (id_mesh == meshDoc()->meshList[i]->id())
		{
			delete_mesh = meshDoc()->meshList[i];
			break;
		}
	}

	if (delete_mesh)
	{
		bool need_waring = false;
		if (delete_mesh->upperOrLowerToothModelMark == UpperToothModel)
		{

		}
		else
		{
		}

		if (need_waring)
		{
			auto result = UiUtilityTools::getInstance()->showQuestion(this, tr(""),
				tr("The current model has been marked,would you like to remove ?"));

			if (!(result == QMessageBox::Ok || result == QMessageBox::Yes))
			{
				return;
			}
		}

		// Delete measurement records
		if (delete_mesh->upperOrLowerToothModelMark == UpperToothModel)
		{
			PFusionAlignData->deleteUpperDentalData();
		}
		else
		{
			PFusionAlignData->deleteLowerDentalData();
		}

		// Remove current mesh
		meshDoc()->delMesh(delete_mesh);

		// 重新分类初始化
		classifyModels();

		// Control UI based on remaining count after deleting model
		if (meshDoc()->meshList.empty())
		{
			p_fusion_title_bar_->setPatientName("");
			onChangeProjStageSlot(E_ProjStageNone);
			clearOldProject();
			emit clearUI();
			// Home page interface
			setHomePageVisible(true);
			updateRecentProjActions();
		}
		else
		{
			// Import opposite jaw interface
			bool mark_over = true;
			if (meshDoc()->meshList.size() == 1 && mark_over)
			{
				PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageSignalComplete;
			}
			else if (meshDoc()->meshList.size() == 1 && !mark_over)
			{
				PFusionAlignData->getAnalyserData().cur_proj_stage_ = E_ProjAnaStageSignalNotComplete;
			}
			onChangeAnaStage(PFusionAlignData->getAnalyserData().cur_proj_stage_);

		}
	}
}

bool MainWindow::importMeshWithLayerManagement(QString fileName)
{
	bool res = importMesh(fileName, false);
	if (res)
	{
		setCurrentMeshBestTab();
	}
	emit updateProgressBar(0, QStringLiteral(""));
	return res;
}

// Opening files in a transparent form (IO plugins contribution is hidden to user)
bool MainWindow::importMesh(QString fileName, bool isareload)
{
	if (!GLA())
	{
		this->newProject();
		if (!GLA())
			return false;
	}

	QStringList fileNameList;

	// here we will import upper and lower tooth model, and we
	// mark the mesh when add mesh to this current document.11/2/2018
	std::vector<ToothModelType> importToothModelType;

	if (fileName.isEmpty())
	{
		// TODO: Need to modify - creating new project should not be placed here
		if (QDialog::Rejected == fileImportDlg->exec())
			return false;
 		else
 			clearOldProject();

		if (fileImportDlg->getOrderIdInfomation().isEmpty())
		{
			return false;
		}

		GLA()->patientInfo.setOrderId(fileImportDlg->getOrderIdInfomation());

		if (!fileImportDlg->upperToothFilePathNameList.isEmpty())
		{
			fileNameList.push_back(fileImportDlg->upperToothFilePathNameList.first());
			importToothModelType.push_back(UpperToothModel);
		}

		if (!fileImportDlg->lowerToothFilePathNameList.isEmpty())
		{
			fileNameList.push_back(fileImportDlg->lowerToothFilePathNameList.first());
			importToothModelType.push_back(LowerToothModel);
		}

		PFusionAlignData->getAnalyserData().patient_name_ = fileImportDlg->getOrderIdInfomation();
		fileImportDlg->cancelImportToothFile();
	}
	else
	{
		fileNameList.push_back(fileName);
		importToothModelType.push_back(UpperToothModel);
	}

	if (fileNameList.isEmpty())
	{
		return false;
	}
	else
	{
		//save path away so we can use it again
		QString path = fileNameList.first();
		path.truncate(path.lastIndexOf("/"));
		lastUsedDirectory.setPath(path);
	}

	QTime allFileTime;
	allFileTime.start();
	int numModel = 0;
	foreach(fileName, fileNameList)
	{
		QFileInfo fi(fileName);
		QString extension = fi.suffix();
		MeshIOInterface* pCurrentIOPlugin = PM.allKnowInputFormats[extension.toLower()];

		if (pCurrentIOPlugin == NULL)
		{

            QString errorMsg = tr("can not open file : %1 format not supported ").arg(fileName);
            UiUtilityTools::getInstance()->showInfoMessageBox(tr("Info"), errorMsg);
			return false;
		}

		//
		RichParameterSet prePar;
		pCurrentIOPlugin->initPreOpenParameter(extension, fileName, prePar);
		if (!prePar.isEmpty())
		{
			GenericParamDialog preOpenDialog(this, &prePar, tr("Pre-Open Options"));
			preOpenDialog.setFocus();
			preOpenDialog.exec();
		}
		prePar.join(currentGlobalParams);
		int mask = 0;
		//MeshModel *mm= new MeshModel(gla->meshDoc);
		QFileInfo info(fileName);
		if (importToothModelType[numModel] == UpperToothModel)
		{
			std::string strPluginName("导入上颌模型文件:");
			QByteArray data = fileName.toLocal8Bit();
			strPluginName.append(data);
			logInstance->log->info(strPluginName);
			logInstance->log->flush();
		}
		else
		{
			std::string strPluginName("导入下颌模型文件:");
			QByteArray data = fileName.toLocal8Bit();
			strPluginName.append(data);
			logInstance->log->info(strPluginName);
			logInstance->log->flush();
			// maybe just import lower tooth only
		}
		MeshModel* mm = meshDoc()->addNewMesh(fileName, info.fileName(), true, true, importToothModelType[numModel++]);
		qb->show();
		QTime t;
		t.start();
		Matrix44m mtr;
		mtr.SetIdentity();
		bool open = loadMesh(fileName, pCurrentIOPlugin, mm, mask, &prePar, mtr, false);
		if (open)
		{
			RichParameterSet par;
			pCurrentIOPlugin->initOpenParameter(extension, *mm, par);
			if (!par.isEmpty())
			{
				GenericParamDialog postOpenDialog(this, &par, tr("Post-Open Processing"));
				postOpenDialog.setFocus();
				postOpenDialog.exec();
				pCurrentIOPlugin->applyOpenParameter(extension, *mm, par);
			}

			vcg::tri::Allocator<CMeshO>::CompactFaceVector(mm->cm);
			vcg::tri::Allocator<CMeshO>::CompactVertexVector(mm->cm);
			mm->cm.face.EnableFFAdjacency();
			mm->cm.vert.EnableVFAdjacency();
			mm->cm.face.EnableVFAdjacency();
			//mm->cm.vert.EnableQuality();
			mm->cm.vert.EnableCurvature();
			//mm->cm.vert.EnableCurvatureDir();

			//tri::UpdateTopology<MeshType>::FaceFace(m)
			vcg::tri::UpdateTopology<CMeshO>::VertexFace(mm->cm);
			vcg::tri::UpdateTopology<CMeshO>::FaceFace(mm->cm);
			vcg::tri::UpdateCurvature<CMeshO>::MeanAndGaussian(mm->cm);
			//vcg::tri::UpdateCurvature<CMeshO>::PrincipalDirections(mm->cm);
			// update Normals
			vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(mm->cm);
			vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(mm->cm);
			vcg::tri::RequirePerVertexCurvature(mm->cm);

		}
		else
		{
			meshDoc()->delMesh(mm);
			GLA()->Logf(0, "Warning: Mesh %s has not been opened", qPrintable(fileName));
			return false;
		}
	}// end foreach file of the input list
	GLA()->Logf(0, "All files opened in %i msec", allFileTime.elapsed());
	if (fileNameList.size() >= 2)
	{
		emit PSIGNALMANAGER->setUpperDentalSelectedSignal();
	}
	if (_currviewcontainer != NULL)
	{
		_currviewcontainer->resetAllTrackBall();
		_currviewcontainer->updateAllDecoratorsForAllViewers();
	}
	qb->reset();

	createRecordData();
	classifyModels();

	autoSaveProject(false);

	return true;
}

void MainWindow::openRecentMesh()
{
	if (!GLA()) return;
	if (meshDoc()->isBusy()) return;
	QAction* action = qobject_cast<QAction*>(sender());
	if (action && !action->data().toString().isEmpty())
	{
		bool activeDoc = true;
		bool activeEmpty = activeDoc && meshDoc()->meshList.empty();

		if (!activeEmpty)
		{
			if (!meshDoc()->meshList.empty())
			{
				int numMesh = meshDoc()->meshList.size();
				for (int i = 0; i < numMesh; ++i)
				{
					meshDoc()->delMesh(*meshDoc()->meshList.begin());
				}
			}
		}
	}

	if (action && importMeshWithLayerManagement(action->data().toString()))
	{

		meshDoc()->setFileName(action->data().toString());
		meshDoc()->setDocLabel(action->data().toString());

		bool bNeedSaveAlignProjectAuto = false;
		GLA()->setAlignProjectToSaveAuto(bNeedSaveAlignProjectAuto);
		GLA()->setSegmentProjectToSaveAuto(!bNeedSaveAlignProjectAuto);
	}
	else
	{
		if (!meshDoc()->meshList.empty())
		{
			int numMesh = meshDoc()->meshList.size();
			for (int i = 0; i < numMesh; ++i)
			{
				meshDoc()->delMesh(*meshDoc()->meshList.begin());
			}
		}

		meshDoc()->setFileName("");
		meshDoc()->setDocLabel("");

		bool bNeedSaveAlignProjectAuto = false;
		GLA()->setAlignProjectToSaveAuto(bNeedSaveAlignProjectAuto);
		GLA()->setSegmentProjectToSaveAuto(bNeedSaveAlignProjectAuto);
	}
}

void MainWindow::openRecentProj()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if (action)
		openProject(action->data().toString());
}

bool MainWindow::loadMeshWithStandardParams(QString& fullPath, MeshModel* mm,
	const Matrix44m& mtr, bool isreload)
{
	if ((meshDoc() == NULL) || (mm == NULL))
		return false;
	bool ret = false;
	mm->Clear();
	QFileInfo fi(fullPath);
	if (fi.size() == 17)
	{
		UiUtilityTools::getInstance()->showInfoMessageBox(tr("info"), tr("File %1 is error").arg(fullPath));
		return false;
	}
	QString extension = fi.suffix();
	MeshIOInterface* pCurrentIOPlugin = PM.allKnowInputFormats[extension.toLower()];
	if (pCurrentIOPlugin != NULL)
	{
		RichParameterSet prePar;
		pCurrentIOPlugin->initPreOpenParameter(extension, fullPath, prePar);
		prePar = prePar.join(currentGlobalParams);
		int mask = 0;
		QTime t; t.start();
		//bool open = loadMesh(fullPath, pCurrentIOPlugin, mm, mask, &prePar, mtr, isreload);
		bool open = loadMesh(fullPath, pCurrentIOPlugin, mm, mask, &prePar, mtr, false);
		if (open)
		{
			UtilityTools::getInstance()->unitCorrectionOfDentalModel(&mm->cm);
			tri::Clean<CMeshO>::RemoveDuplicateVertex(mm->cm);
			mm->UpdateBoxAndNormals();

			vcg::Color4b tooth_color(255, 255, 255, 255);
			for (int i = 0; i < mm->cm.vert.size(); ++i)
			{
				mm->cm.vert[i].C() = tooth_color;
			}

			CloudOctree* pOctree = UtilityTools::getInstance()->createMeshOctree(mm);
			if (GLA())
				PFusionAlignData->mesh_cloud_Octree_[mm] = pOctree;
			GLA()->Logf(0, "Opened mesh %s in %i msec", qPrintable(fullPath), t.elapsed());
			RichParameterSet par;
			pCurrentIOPlugin->initOpenParameter(extension, *mm, par);
			pCurrentIOPlugin->applyOpenParameter(extension, *mm, par);
			ret = true;

			std::string strPluginName("导入模型:");
			QByteArray data = fullPath.toLocal8Bit();
			strPluginName.append(data);
			logInstance->log->info(strPluginName);
			logInstance->log->flush();

			emit PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
		}
	}
	return ret;
}

bool MainWindow::exportMesh(QString fileName, MeshModel* mod, const bool saveAllPossibleAttributes)
{
	QStringList& suffixList = PM.outFilters;

	QFileInfo fi(fileName);
	//PM.LoadFormats( suffixList, allKnownFormats,PluginManager::EXPORT);
	//QString defaultExt = "*." + mod->suffixName().toLower();
	QString defaultExt = "*." + fi.suffix().toLower();
	if (defaultExt == "*.")
		defaultExt = "*.ply";
	if (mod == NULL)
		return false;
	mod->meshModified() = false;
	QString laylabel = tr("export mesh save as");
	QString ss = fi.absoluteFilePath();
	QFileDialog* saveDialog = new QFileDialog(this, laylabel, fi.absolutePath());
	saveDialog->setNameFilters(suffixList);
	saveDialog->setAcceptMode(QFileDialog::AcceptSave);
	saveDialog->setFileMode(QFileDialog::AnyFile);
	saveDialog->selectFile(fileName);
	QStringList matchingExtensions = suffixList.filter(defaultExt);
	if (!matchingExtensions.isEmpty())
		saveDialog->selectNameFilter(matchingExtensions.last());
	connect(saveDialog, SIGNAL(filterSelected(const QString&)), this, SLOT(changeFileExtension(const QString&)));

	if (fileName.isEmpty()) {
		saveDialog->selectFile(meshDoc()->mm()->fullName());
		int dialogRet = saveDialog->exec();
		if (dialogRet == QDialog::Rejected)
			return false;
		fileName = saveDialog->selectedFiles().first();
		QFileInfo fni(fileName);
		if (fni.suffix().isEmpty())
		{
			QString ext = saveDialog->selectedNameFilter();
			ext.chop(1); ext = ext.right(4);
			fileName = fileName + ext;
			qDebug("File without extension adding it by hand '%s'", qPrintable(fileName));
		}
	}

	bool ret = false;

	QStringList fs = fileName.split(".");

	if (!fileName.isEmpty() && fs.size() < 2)
	{

        QString errorMsg = tr("%1 format not supported ").arg(fileName);
        UiUtilityTools::getInstance()->showInfoMessageBox(tr("Info"), errorMsg);

		return ret;
	}

	if (!fileName.isEmpty())
	{
		//save path away so we can use it again
		QString path = fileName;
		path.truncate(path.lastIndexOf("/"));
		lastUsedDirectory.setPath(path);

		QString extension = fileName;
		extension.remove(0, fileName.lastIndexOf('.') + 1);

		QStringListIterator itFilter(suffixList);

		MeshIOInterface* pCurrentIOPlugin = PM.allKnowOutputFormats[extension.toLower()];
		if (pCurrentIOPlugin == 0)
		{
            QString errorMsg = tr("%1 format not supported ").arg(fileName);
            UiUtilityTools::getInstance()->showInfoMessageBox(tr("Info"), errorMsg);
			return false;
		}
		//MeshIOInterface* pCurrentIOPlugin = meshIOPlugins[idx-1];
		pCurrentIOPlugin->setLog(&meshDoc()->Log);

		int capability = 0, defaultBits = 0;
		pCurrentIOPlugin->GetExportMaskCapability(extension, capability, defaultBits);

		// optional saving parameters (like ascii/binary encoding)
		RichParameterSet savePar;

		pCurrentIOPlugin->initSaveParameter(extension, *(mod), savePar);

		SaveMaskExporterDialog maskDialog(new QWidget(), mod, capability, defaultBits, &savePar, this->GLA());
		if (!saveAllPossibleAttributes)
		{
			QDesktopWidget* desk = QApplication::desktop();
			maskDialog.move((desk->width() - maskDialog.width()) / 2, (desk->height() - maskDialog.height()) / 2);
			maskDialog.exec();
		}
		else
		{
			maskDialog.SlotSelectionAllButton();
			maskDialog.updateMask();
		}
		int mask = maskDialog.GetNewMask();
		if (!saveAllPossibleAttributes)
		{
			maskDialog.close();
			if (maskDialog.result() == QDialog::Rejected)
				return false;
		}
		if (mask == -1)
			return false;

		qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
		qb->show();
		QTime tt; tt.start();
		ret = pCurrentIOPlugin->save(extension, fileName, *mod, mask, savePar, QCallBack, this);
		qb->reset();
		GLA()->Logf(GLLogStream::SYSTEM, "Saved Mesh %s in %i msec", qPrintable(fileName), tt.elapsed());

		qApp->restoreOverrideCursor();
		mod->setFileName(fileName);
		QSettings settings;
		int savedMeshCounter = settings.value("savedMeshCounter", 0).toInt();
		settings.setValue("savedMeshCounter", savedMeshCounter + 1);

		if (ret)
			QDir::setCurrent(fi.absoluteDir().absolutePath()); //set current dir
	}
	return ret;
}

void MainWindow::changeFileExtension(const QString& st)
{
	QFileDialog* fd = qobject_cast<QFileDialog*>(sender());
	if (fd == NULL)
		return;
	QRegExp extlist("\\*.\\w+");
	int start = st.indexOf(extlist);
	(void)start;
	QString ext = extlist.cap().remove("*");
	QStringList stlst = fd->selectedFiles();
	if (!stlst.isEmpty())
	{
		QFileInfo fi(stlst[0]);
		fd->selectFile(fi.baseName() + ext);
	}
}

bool MainWindow::save(const bool saveAllPossibleAttributes)
{
	return exportMesh(meshDoc()->mm()->fullName(), meshDoc()->mm(), saveAllPossibleAttributes);
}

bool MainWindow::saveAs(QString fileName, const bool saveAllPossibleAttributes)
{
	return exportMesh(fileName, meshDoc()->mm(), saveAllPossibleAttributes);
}

void MainWindow::about()
{
	QtAboutDialog::show(this);
}

void MainWindow::showToolbarFile() {
	mainToolBar->setVisible(!mainToolBar->isVisible());
}

void MainWindow::showInfoPane() { if (GLA() != 0)	GLA()->infoAreaVisible = !GLA()->infoAreaVisible; }
void MainWindow::showTrackBall() { if (GLA() != 0) 	GLA()->showTrackBall(!GLA()->isTrackBallVisible()); }

void MainWindow::fullScreen()
{
}

void MainWindow::keyPressEvent(QKeyEvent* e)
{
	GLA()->keyPressEventSpecial(e);
	if (e->key() == Qt::Key_Q && e->modifiers() == Qt::AltModifier)
	{
		QString exePath = QCoreApplication::applicationDirPath();
		QFile file(exePath + "/../FusionAnalyser/res/default/images/fusionAnalyser_dark.css");
		//QFile file(exePath + "/fusionAnalyser_dark.css");

		if (file.open(QIODevice::ReadOnly))
		{
			auto qssText = file.readAll();
			qApp->setStyleSheet(qssText);
			file.close();
		}

	}
	if (e->key() == Qt::Key_K && e->modifiers() == Qt::AltModifier)
	{
		setSplit();
	}
	return;
}

void MainWindow::keyReleaseEvent(QKeyEvent* e)
{
	GLA()->keyReleaseEventSpecial(e);
}

bool MainWindow::QCallBack(int pos, const char* strPluginName)
{
	int static lastPos = -1;
	if (pos == lastPos) return true;
	if (pos == 100)
	{
		lastPos = -1;
	}
	else
	{
		lastPos = pos;
	}

	static QTime currTime = QTime::currentTime();
	if (currTime.elapsed() < 10 && pos != 0)
		return true;

	currTime.start();
	MainWindow::globalStatusBar()->showMessage(QString(strPluginName), 3000);
	qb->show();
	qb->setEnabled(true);
	qb->setValue(pos);
	MainWindow::globalStatusBar()->update();
	qApp->processEvents();

	if (pos <= 0 || pos >= 100)
	{
		qb->hide();
	}
	return true;
}

void MainWindow::updateTexture(int meshid)
{
	MultiViewer_Container* mvc = currentViewContainer();
	if ((mvc == NULL) || (meshDoc() == NULL))
		return;

	MLSceneGLSharedDataContext* shared = mvc->sharedDataContext();
	if (shared == NULL)
		return;

	MeshModel* mymesh = meshDoc()->getMesh(meshid);
	if (mymesh == NULL)
		return;

	shared->deAllocateTexturesPerMesh(mymesh->id());

	int textmemMB = int(mwsettings.maxTextureMemory / ((float)1024 * 1024));

	size_t totalTextureNum = 0;
	foreach(MeshModel * mp, meshDoc()->meshList)
		totalTextureNum += mp->cm.textures.size();

	int singleMaxTextureSizeMpx = int(textmemMB / ((totalTextureNum != 0) ? totalTextureNum : 1));
	bool sometextfailed = false;
	QString unexistingtext = "In mesh file <i>" + mymesh->fullName() + "</i> : Failure loading textures:<br>";
	for (size_t i = 0; i < mymesh->cm.textures.size(); ++i)
	{
		QImage img;
		QFileInfo fi(mymesh->cm.textures[i].c_str());
		QString filename = fi.absoluteFilePath();
		bool res = img.load(filename);
		sometextfailed = sometextfailed || !res;
		if (!res)
		{
			res = img.load(filename);
			if (!res)
			{
				QString errmsg = QString("Failure of loading texture %1").arg(fi.fileName());
				meshDoc()->Log.Log(GLLogStream::WARNING, qPrintable(errmsg));
				unexistingtext += "<font color=red>" + filename + "</font><br>";
			}
		}

		if (!res)
			res = img.load(":/res/default/images/dummy.png");
		GLuint textid = shared->allocateTexturePerMesh(meshid, img, singleMaxTextureSizeMpx);

        if (sometextfailed);
            //QMessageBox::warning(this, "Texture file has not been correctly loaded", unexistingtext);

		for (int tt = 0; tt < mvc->viewerCounter(); ++tt)
		{
			GLArea* ar = mvc->getViewer(tt);
			if (ar != NULL)
				ar->setupTextureEnv(textid);
		}
	}
    if (sometextfailed);
		//QMessageBox::warning(this, "Texture file has not been correctly loaded", unexistingtext);
}

void MainWindow::updateProgressBar(const int pos, const QString& text)
{
	this->QCallBack(pos, text.toStdString().c_str());
}

void MainWindow::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	p_fusion_title_bar_->update();
	//sendUsAMail();
}

void MainWindow::meshAdded(int mid)
{
	MultiViewer_Container* mvc = currentViewContainer();
	if (mvc != NULL)
	{
		MLSceneGLSharedDataContext* shared = mvc->sharedDataContext();
		if (shared != NULL)
		{
			shared->meshInserted(mid);
			QList<QGLContext*> contlist;
			for (int glarid = 0; glarid < mvc->viewerCounter(); ++glarid)
			{
				GLArea* ar = mvc->getViewer(glarid);
				if (ar != NULL)
					contlist.push_back(ar->context());
			}
			MLRenderingData defdt;
			if (meshDoc() != NULL)
			{
				MeshModel* mm = meshDoc()->getMesh(mid);
				if (mm != NULL)
				{
					for (int glarid = 0; glarid < mvc->viewerCounter(); ++glarid)
					{
						GLArea* ar = mvc->getViewer(glarid);
						if (ar != NULL)
							shared->setRenderingDataPerMeshView(mid, ar->context(), defdt);
					}
					shared->manageBuffers(mid);
				}
			}
		}

	}
}

void MainWindow::meshRemoved(int mid)
{
	MultiViewer_Container* mvc = currentViewContainer();
	if (mvc != NULL)
	{
		MLSceneGLSharedDataContext* shared = mvc->sharedDataContext();
		if (shared != NULL)
			shared->meshRemoved(mid);
	}

}

void MainWindow::getRenderingData(int mid, MLRenderingData& dt) const
{
	if (mid == -1)
	{
	}
	else
	{
		MultiViewer_Container* cont = currentViewContainer();
		if (cont != NULL)
		{
			MLSceneGLSharedDataContext* share = cont->sharedDataContext();
			if ((share != NULL) && (GLA() != NULL))
				share->getRenderInfoPerMeshView(mid, GLA()->context(), dt);
		}
	}
}

void MainWindow::setRenderingData(int mid, const MLRenderingData& dt)
{
	if (mid == -1)
	{
		/*if (GLA() != NULL)
			GLA()->setPerDocGlobalRenderingData(dt);*/
	}
	else
	{
		MultiViewer_Container* cont = currentViewContainer();
		if (cont != NULL)
		{
			MLSceneGLSharedDataContext* share = cont->sharedDataContext();
			if ((share != NULL) && (GLA() != NULL))
			{
				share->setRenderingDataPerMeshView(mid, GLA()->context(), dt);
				share->manageBuffers(mid);
				if (globrendtoolbar != NULL)
				{
					MLSceneGLSharedDataContext::PerMeshRenderingDataMap mp;
					share->getRenderInfoPerMeshView(GLA()->context(), mp);
					globrendtoolbar->statusConsistencyCheck(mp);
				}
			}
		}
	}
}

void MainWindow::updateRenderingDataAccordingToActionsCommonCode(int meshid, const QList<MLRenderingAction*>& acts)
{
	if (meshDoc() == NULL)
		return;

	MLRenderingData olddt;
	getRenderingData(meshid, olddt);
	MLRenderingData dt(olddt);
	foreach(MLRenderingAction * act, acts)
	{
		if (act != NULL)
			act->updateRenderingData(dt);
	}
	MeshModel* mm = meshDoc()->getMesh(meshid);
	if (mm != NULL)
	{
		MLPoliciesStandAloneFunctions::setBestWireModality(mm, dt);
		MLPoliciesStandAloneFunctions::computeRequestedRenderingDataCompatibleWithMeshSameGLOpts(mm, dt, dt);
	}
	setRenderingData(meshid, dt);

	/*if (meshid == -1)
	{
		foreach(MeshModel* mm, meshDoc()->meshList)
		{
			if (mm != NULL)
			{
				MLDefaultMeshDecorators dec(this);
				dec.updateMeshDecorationData(*mm, olddt, dt);
			}
		}
	}
	else
	{*/
	if (mm != NULL)
	{
		MLDefaultMeshDecorators dec(this);
		dec.updateMeshDecorationData(*mm, olddt, dt);
	}
	/*}*/

}

void MainWindow::updateRenderingDataAccordingToActions(int meshid, const QList<MLRenderingAction*>& acts)
{
	updateRenderingDataAccordingToActionsCommonCode(meshid, acts);
	if (GLA() != NULL)
		GLA()->update();
}

void MainWindow::updateRenderingDataAccordingToActionsToAllVisibleLayers(const QList<MLRenderingAction*>& acts)
{
	if (meshDoc() == NULL)
		return;
	for (int ii = 0; ii < meshDoc()->meshList.size(); ++ii)
	{
		MeshModel* mm = meshDoc()->meshList[ii];
		if ((mm != NULL) && (mm->isVisible()))
		{
			updateRenderingDataAccordingToActionsCommonCode(mm->id(), acts);
		}
	}

	if (GLA() != NULL)
		GLA()->update();
}

void MainWindow::updateRenderingDataAccordingToActions(int /*meshid*/, MLRenderingAction* act, QList<MLRenderingAction*>& acts)
{
	if ((meshDoc() == NULL) || (act == NULL))
		return;

	QList<MLRenderingAction*> tmpacts;
	for (int ii = 0; ii < acts.size(); ++ii)
	{
		if (acts[ii] != NULL)
		{
			MLRenderingAction* sisteract = NULL;
			acts[ii]->createSisterAction(sisteract, NULL);
			sisteract->setChecked(acts[ii] == act);
			tmpacts.push_back(sisteract);
		}
	}

	for (int hh = 0; hh < meshDoc()->meshList.size(); ++hh)
	{
		if (meshDoc()->meshList[hh] != NULL)
			updateRenderingDataAccordingToActionsCommonCode(meshDoc()->meshList[hh]->id(), tmpacts);
	}

	for (int ii = 0; ii < tmpacts.size(); ++ii)
		delete tmpacts[ii];
	tmpacts.clear();

	if (GLA() != NULL)
		GLA()->update();
}

void MainWindow::updateRenderingDataAccordingToActionCommonCode(int meshid, MLRenderingAction* act)
{
	if ((meshDoc() == NULL) || (act == NULL))
		return;

	if (meshid != -1)
	{
		MLRenderingData olddt;
		getRenderingData(meshid, olddt);
		MLRenderingData dt(olddt);
		act->updateRenderingData(dt);
		MeshModel* mm = meshDoc()->getMesh(meshid);
		if (mm != NULL)
		{
			MLPoliciesStandAloneFunctions::setBestWireModality(mm, dt);
			MLPoliciesStandAloneFunctions::computeRequestedRenderingDataCompatibleWithMeshSameGLOpts(mm, dt, dt);
		}
		setRenderingData(meshid, dt);
		if (mm != NULL)
		{
			MLDefaultMeshDecorators dec(this);
			dec.updateMeshDecorationData(*mm, olddt, dt);
		}
	}
}

void MainWindow::updateRenderingDataAccordingToAction(int meshid, MLRenderingAction* act)
{
	updateRenderingDataAccordingToActionCommonCode(meshid, act);
	if (GLA() != NULL)
		GLA()->update();
}

void MainWindow::updateRenderingDataAccordingToActionToAllVisibleLayers(MLRenderingAction* act)
{
	if (meshDoc() == NULL)
		return;

	for (int ii = 0; ii < meshDoc()->meshList.size(); ++ii)
	{
		MeshModel* mm = meshDoc()->meshList[ii];
		if ((mm != NULL) && (mm->isVisible()))
		{
			updateRenderingDataAccordingToActionCommonCode(mm->id(), act);
		}
	}

	if (GLA() != NULL)
		GLA()->update();
}

void  MainWindow::updateRenderingDataAccordingToActions(QList<MLRenderingGlobalAction*> actlist)
{
	if (meshDoc() == NULL)
		return;

	for (int ii = 0; ii < meshDoc()->meshList.size(); ++ii)
	{
		MeshModel* mm = meshDoc()->meshList[ii];
		if (mm != NULL)
		{
			foreach(MLRenderingGlobalAction * act, actlist)
			{
				foreach(MLRenderingAction * ract, act->mainActions())
					updateRenderingDataAccordingToActionCommonCode(mm->id(), ract);

				foreach(MLRenderingAction * ract, act->relatedActions())
					updateRenderingDataAccordingToActionCommonCode(mm->id(), ract);
			}
		}
	}

	if (GLA() != NULL)
		GLA()->update();
}

void MainWindow::updateRenderingDataAccordingToAction(int /*meshid*/, MLRenderingAction* act, bool check)
{
	MLRenderingAction* sisteract = NULL;
	act->createSisterAction(sisteract, NULL);
	sisteract->setChecked(check);
	foreach(MeshModel * mm, meshDoc()->meshList)
	{
		if (mm != NULL)
			updateRenderingDataAccordingToActionCommonCode(mm->id(), sisteract);
	}
	delete sisteract;
	if (GLA() != NULL)
		GLA()->update();
}

bool MainWindow::addRenderingDataIfNewlyGeneratedMesh(int meshid)
{
	MultiViewer_Container* mvc = currentViewContainer();
	if (mvc == NULL)
		return false;
	MLSceneGLSharedDataContext* shared = mvc->sharedDataContext();
	if (shared != NULL)
	{
		MeshModel* mm = meshDoc()->getMesh(meshid);
		if ((meshDoc()->meshDocStateData().find(meshid) ==
			meshDoc()->meshDocStateData().end()) && (mm != NULL))
		{
			MLRenderingData dttoberendered;
			MLPoliciesStandAloneFunctions::suggestedDefaultPerViewRenderingData(
				mm, dttoberendered, mwsettings.minpolygonpersmoothrendering);
			foreach(GLArea * gla, mvc->viewerList)
			{
				if (gla != NULL)
					shared->setRenderingDataPerMeshView(meshid, gla->context(), dttoberendered);
			}
			shared->manageBuffers(meshid);
			return true;
		}
	}
	return false;
}

unsigned int MainWindow::viewsRequiringRenderingActions(int meshid, MLRenderingAction* act)
{
	unsigned int res = 0;
	MultiViewer_Container* cont = currentViewContainer();
	if (cont != NULL)
	{
		MLSceneGLSharedDataContext* share = cont->sharedDataContext();
		if (share != NULL)
		{
			foreach(GLArea * area, cont->viewerList)
			{
				MLRenderingData dt;
				share->getRenderInfoPerMeshView(meshid, area->context(), dt);
				if (act->isRenderingDataEnabled(dt))
					++res;
			}
		}
	}
	return res;
}

void MainWindow::updateLog()
{
	GLLogStream* senderlog = qobject_cast<GLLogStream*>(sender());
}

void MainWindow::switchCurrentContainer(QMdiSubWindow* subwin)
{
	return;
	if (subwin == NULL)
	{
		if (globrendtoolbar != NULL)
			globrendtoolbar->reset();
		return;
	}
	if (_currviewcontainer != NULL)
	{
		updateMenus();
	}
}

void MainWindow::closeCurrentDocument()
{
	_currviewcontainer = NULL;
	updateMenus();
}

void MainWindow::onParseAlnProjFinishedSlot()
{
    if(p_align_toolbar_)
        p_align_toolbar_->setEnabled(true);
	updateTreamtmentStepGUISlot();
}

void MainWindow::updateTreamtmentStepGUISlot()
{
	// Treatment preview removed from FusionAnalyser
}

void MainWindow::onCategoryToolBarToggledSlot(bool toogled)
{
	// Category toolbar functionality removed - FusionAnalyser does not use category toolbars
	return;
}

void MainWindow::updateGUI()
{
	if (GLA() && GLA()->getCurrentMeshEditor())
	{
		GLA()->getCurrentMeshEditor()->updateUI();
	}

	if (_currviewcontainer && !_currviewcontainer->isVisible())
	{
		if (GLA() && GLA()->getCurrentMeshEditor())
		{
			GLA()->getCurrentMeshEditor()->updateUI(false);
		}
	}

    if(p_fusion_title_bar_)
        p_fusion_title_bar_->update();
    this->update();
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#ifdef Q_OS_WIN

    MSG* msg = static_cast<MSG*>(message);
    //qDebug() << QString::number(msg->message, 16);
    switch (msg->message)
    {
    case WM_NCCALCSIZE:
    {
        return true;
    }

    case WM_NCHITTEST:
    {
        int xPos = GET_X_LPARAM(msg->lParam);
        int yPos = GET_Y_LPARAM(msg->lParam);
        *result = onTestBorder(mapFromGlobal(QPoint(xPos, yPos)));
        if (p_fusion_title_bar_->isCaption(xPos - this->frameGeometry().x(), yPos - this->frameGeometry().y()))
        {
            *result = HTCAPTION;
            return true;
        }
        return true;
    }

    case WM_GETMINMAXINFO:
    {
        if(this->isMaximized())
        {
			RECT frame = { 0, 0, 0, 0 };
			AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);

			//record frame area data

			frame.right = frame.bottom = 0;
			frames_.setLeft(abs(frame.left) + 0.5);
			frames_.setTop(abs(frame.bottom) + 0.5);
			frames_.setRight(abs(frame.right) + 0.5);
			frames_.setBottom(abs(frame.bottom) + 0.5);

			QMainWindow::setContentsMargins(frames_.left() + margins_.left(), \
				frames_.top() + margins_.top(), \
				frames_.right() + margins_.right(), \
				frames_.bottom() + margins_.bottom());
			b_maximized_ = true;
        }
        else
        {
			if (b_maximized_)
			{
				RECT frame = { 0, 0, 0, 0 };
				AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);

				QMainWindow::setContentsMargins(frames_.left() + margins_.left(), \
					frames_.top() + margins_.top(), \
					frames_.right() + margins_.right(), \
					frames_.bottom() + margins_.bottom());

				//QMainWindow::setContentsMargins(margins_);
				frames_ = QMargins();
				b_maximized_ = false;
			}
        }
        GLA()->update();
		b_need_update_ui_ = true;
        *result = ::DefWindowProc(msg->hwnd, msg->message, msg->wParam, msg->lParam);
        //return true;
    }
    break;
    case WM_MOVE:
    case WM_SIZE:
    {
        if (GLA())
            GLA()->update();
        updateGUI();
    }
    break;
	case WM_NCACTIVATE:
		p_fusion_title_bar_->update();
		break;
    default:
        break;
    }

#endif
	return QMainWindow::nativeEvent(eventType, message, result);
}

#ifdef Q_OS_WIN

LRESULT MainWindow::onTestBorder(const QPoint &pt)
{
    if (::IsZoomed((HWND)this->winId()))
    {
        return HTCLIENT;
    }
    int borderSize = 6;
    int cx = this->size().width();
    int cy = this->size().height();
    QRect rectTopLeft(0, 0, borderSize, borderSize);
    if (rectTopLeft.contains(pt))
    {
        return HTTOPLEFT;
    }
    QRect rectLeft(0, borderSize, borderSize, cy - borderSize * 2);
    if (rectLeft.contains(pt))
    {
        return HTLEFT;
    }
    QRect rectTopRight(cx - borderSize, 0, borderSize, borderSize);
    if (rectTopRight.contains(pt))
    {
        return HTTOPRIGHT;
    }
    QRect rectRight(cx - borderSize, borderSize, borderSize, cy - borderSize * 2);
    if (rectRight.contains(pt))
    {
        return HTRIGHT;
    }
    QRect rectTop(borderSize, 0, cx - borderSize * 2, borderSize);
    if (rectTop.contains(pt))
    {
        return HTTOP;
    }
    QRect rectBottomLeft(0, cy - borderSize, borderSize, borderSize);
    if (rectBottomLeft.contains(pt))
    {
        return HTBOTTOMLEFT;
    }
    QRect rectBottomRight(cx - borderSize, cy - borderSize, borderSize, borderSize);
    if (rectBottomRight.contains(pt))
    {
        return HTBOTTOMRIGHT;
    }
    QRect rectBottom(borderSize, cy - borderSize, cx - borderSize * 2, borderSize);
    if (rectBottom.contains(pt))
    {
        return HTBOTTOM;
    }
    return HTCLIENT;
}
#endif

void MainWindow::onChangeProjStageSlot(eProjStage stage)
{

}

void MainWindow::onChangeAnaStage(eProjAnalyserStage stage)
{
	int size = meshDoc()->meshList.size();
	if (stage == E_ProjAnaStageNone)
	{
		GLA()->Import_jaw_alignment->hide();
		GLA()->Fixed_point_analysis->hide();
		GLA()->Skip_the_upper_jaw->hide();
		GLA()->Previous_step->hide();
		GLA()->Previous_step->hide();
		GLA()->complete_btn->hide();
		GLA()->forward_step_button_->hide();
		GLA()->next_step_button->hide();
		GLA()->tips_button_gui_->hide();
		GLA()->getViewGui()->hide();
	}
	else if (stage == E_ProjAnaStageImport || stage == E_ProjAnaStageSignalComplete || stage == E_ProjAnaStageSignalNotComplete )
	{
		GLA()->Import_jaw_alignment->show();
		GLA()->Fixed_point_analysis->show();
		GLA()->Skip_the_upper_jaw->hide();
		GLA()->Previous_step->hide();
		GLA()->complete_btn->hide();
		GLA()->forward_step_button_->hide();
		GLA()->next_step_button->hide();
		GLA()->tips_button_gui_->hide();
		GLA()->getViewGui()->show();
	}
	else if (stage == E_ProjAnaStageDualNotComplete || stage == E_ProjAnaStageDualComplete)
	{
		GLA()->Import_jaw_alignment->hide();
		GLA()->Fixed_point_analysis->show();
		GLA()->Skip_the_upper_jaw->hide();
		GLA()->Previous_step->hide();
		GLA()->complete_btn->hide();
		GLA()->forward_step_button_->hide();
		GLA()->next_step_button->hide();
		GLA()->tips_button_gui_->hide();
		GLA()->getViewGui()->show();
	}
	else if (stage == E_ProjAnaStageAnalyser)
	{
		p_align_toolbar_->setEnabled(false);
		if (size == 1)
		{
			GLA()->Import_jaw_alignment->hide();
			GLA()->Fixed_point_analysis->hide();
			GLA()->Skip_the_upper_jaw->hide();
			GLA()->Previous_step->hide();
			GLA()->complete_btn->show();
		}
		else
		{
			GLA()->Import_jaw_alignment->hide();
			GLA()->Fixed_point_analysis->hide();
			GLA()->Skip_the_upper_jaw->show();
			GLA()->Previous_step->hide();
			GLA()->complete_btn->show();
		}
	}
	else if (stage == E_ProjAnaStageMissUpper)
	{
		GLA()->getViewGui()->hide();
		p_align_toolbar_->setEnabled(false);
		GLA()->Import_jaw_alignment->hide();
		GLA()->Fixed_point_analysis->hide();
		GLA()->next_step_button->show();
		GLA()->forward_step_button_->hide();
		GLA()->complete_btn->show();
		GLA()->Skip_the_upper_jaw->show();
		GLA()->tips_button_gui_->show();
		int heightBtn = GLA()->next_step_button->height();
		if (GLA()->next_step_button->height() > (GLA()->height()) * 0.075)
		{
			GLA()->next_step_button->setGeometry((GLA()->width()) * 0.85,
				(GLA()->height()) * 0.9 - heightBtn * 2,
				GLA()->next_step_button->width() * 1.5,
				GLA()->next_step_button->height() * 1.5);
		}
		else
		{
			GLA()->next_step_button->setGeometry((GLA()->width()) * 0.85,
				(GLA()->height()) * 0.75,
				GLA()->next_step_button->width() * 1.5,
				GLA()->next_step_button->height() * 1.5);
		}
	}
	else if (stage == E_ProjAnaStageFixUpper)
	{
		GLA()->getViewGui()->hide();
		p_align_toolbar_->setEnabled(false);
		GLA()->Import_jaw_alignment->hide();
		GLA()->Fixed_point_analysis->hide();
		GLA()->next_step_button->hide();
		GLA()->forward_step_button_->show();
		GLA()->complete_btn->show();
		GLA()->Skip_the_upper_jaw->show();
		GLA()->tips_button_gui_->show();
		int heightBtn = GLA()->next_step_button->height();
		if (GLA()->forward_step_button_->width() > (GLA()->width()) * 0.04)
		{
			GLA()->forward_step_button_->setGeometry((GLA()->width()) * 0.85 - GLA()->forward_step_button_->width(),
				(GLA()->height()) * 0.9 - heightBtn,
				GLA()->forward_step_button_->width() * 1.5,
				GLA()->forward_step_button_->height() * 1.5);
		}
		else if(GLA()->next_step_button->height() > (GLA()->height()) * 0.075)
		{
			GLA()->forward_step_button_->setGeometry((GLA()->width()) * 0.81,
				(GLA()->height()) * 0.9 - heightBtn,
				GLA()->forward_step_button_->width() * 1.5,
				GLA()->forward_step_button_->height() * 1.5);
		}
		else
		{
			GLA()->forward_step_button_->setGeometry((GLA()->width()) * 0.81,
				(GLA()->height()) * 0.825,
				GLA()->forward_step_button_->width() * 1.5,
				GLA()->forward_step_button_->height() * 1.5);
		}

	}
	else if (stage == E_ProjAnaStageMissLower)
	{
		GLA()->getViewGui()->hide();
		p_align_toolbar_->setEnabled(false);
		int heightBtn = GLA()->next_step_button->height();
		if (GLA()->next_step_button->height() > (GLA()->height()) * 0.075)
		{
			GLA()->next_step_button->setGeometry((GLA()->width()) * 0.85,
				(GLA()->height()) * 0.9 - heightBtn,
				GLA()->next_step_button->width() * 1.5,
				GLA()->next_step_button->height() * 1.5);
		}
		else
		{
			GLA()->next_step_button->setGeometry((GLA()->width()) * 0.85,
				(GLA()->height()) * 0.825,
				GLA()->next_step_button->width() * 1.5,
				GLA()->next_step_button->height() * 1.5);
		}
		if (GLA()->forward_step_button_->width() > (GLA()->width()) * 0.04)
		{
			GLA()->forward_step_button_->setGeometry((GLA()->width()) * 0.85 - GLA()->forward_step_button_->width(),
				(GLA()->height()) * 0.9 - heightBtn,
				GLA()->forward_step_button_->width() * 1.5,
				GLA()->forward_step_button_->height() * 1.5);
		}
		else if(GLA()->next_step_button->height() > (GLA()->height()) * 0.075)
		{
			GLA()->forward_step_button_->setGeometry((GLA()->width()) * 0.81,
				(GLA()->height()) * 0.9 - heightBtn,
				GLA()->forward_step_button_->width() * 1.5,
				GLA()->forward_step_button_->height() * 1.5);
		}
		else
		{
			GLA()->forward_step_button_->setGeometry((GLA()->width()) * 0.81,
				(GLA()->height()) * 0.825,
				GLA()->forward_step_button_->width() * 1.5,
				GLA()->forward_step_button_->height() * 1.5);
		}
		if (size == 1)
		{
			GLA()->Import_jaw_alignment->hide();
			GLA()->Fixed_point_analysis->hide();
			GLA()->next_step_button->show();
			GLA()->forward_step_button_->hide();
			GLA()->complete_btn->show();
			GLA()->Skip_the_upper_jaw->hide();

		}
		else
		{
			GLA()->Import_jaw_alignment->hide();
			GLA()->Fixed_point_analysis->hide();
			GLA()->next_step_button->show();
			GLA()->forward_step_button_->show();
			GLA()->complete_btn->show();
			GLA()->Skip_the_upper_jaw->hide();
		}
		GLA()->tips_button_gui_->show();
	}
	else if (stage == E_ProjAnaStageFixLower)
	{
	    GLA()->getViewGui()->hide();
	    p_align_toolbar_->setEnabled(false);
		GLA()->Import_jaw_alignment->hide();
		GLA()->Fixed_point_analysis->hide();
		GLA()->next_step_button->hide();
		GLA()->forward_step_button_->show();
		GLA()->complete_btn->show();
		GLA()->Skip_the_upper_jaw->hide();
		GLA()->tips_button_gui_->show();
		if (GLA()->forward_step_button_->width() > (GLA()->width()) * 0.04)
		{
			GLA()->forward_step_button_->setGeometry((GLA()->width()) * 0.85 - GLA()->forward_step_button_->width(),
				(GLA()->height()) * 0.9,
				GLA()->forward_step_button_->width() * 1.5,
				GLA()->forward_step_button_->height() * 1.5);
		}
		else
		{
			GLA()->forward_step_button_->setGeometry((GLA()->width()) * 0.81,
				(GLA()->height()) * 0.9,
				GLA()->forward_step_button_->width() * 1.5,
				GLA()->forward_step_button_->height() * 1.5);
		}
	}
	GLA()->setFixAnaBtnText();
}

void MainWindow::onCreateProjectSlot(bool success, bool isSegment)
{
	if (!isSegment)
	{
		if(p_align_toolbar_)
			p_align_toolbar_->setToolBarEnabled(!success);
	}
}

void MainWindow::updateTreatmentInfoSlot()
{
	emit PSIGNALMANAGER->setPatientInfomationSignal("", "", "", "");
}

void MainWindow::onShowBusyProgressSlot(bool isBusy /*= true*/)
{
    if (!p_progress_dlg_) return;
    if (isBusy)
        p_progress_dlg_->showBusyProgressBar();
    else
        p_progress_dlg_->hide();
}

void MainWindow::updateProgressGuiSlot(const QString& text, int value, int max)
{
	if (!p_progress_dlg_)
	{
		return;
	}

	if (value == 0 || value >= max)
	{
		p_progress_dlg_->hide();
	}
	else
	{
		p_progress_dlg_->showProgressBar(text, 1000 * (value * 1.0f / max));
	}
	update();
}

void MainWindow::onUploadProgress(qint64 sended, qint64 total)
{
	if (!p_progress_dlg_)return;
    if (total != 0)
    {
        //p_upload_progress_dlg_->show();
        //p_upload_progress_dlg_->setLabelText(tr("upload File"));
        double percent = (double)sended / (double)total;
        //p_upload_progress_dlg_->setValue(100 * percent);
        p_progress_dlg_->showProgressBar(tr("upload File"), 1000 * percent);
    }
}

void MainWindow::onDownloadingProgress(qint64 sended, qint64 total)
{
	if (sended == 17 && total == 17)return; //"The File Is Error" size  17 文件错误
	if (!p_progress_dlg_)return;
    if (total != 0)
    {
        double percent = (double)sended / (double)total;
        p_progress_dlg_->showProgressBar(tr("downloading File"), 1000 * percent);
    }
}

void MainWindow::onNetworkFinished(QNetworkReply*)
{
	if (!p_progress_dlg_)return;
    p_progress_dlg_->hide();
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == p_seg_redo_btn_ || watched == p_align_redo_btn_)
	{
		if (event->type() == QEvent::MouseButtonPress)
		{
			unDoAct->setIcon(QIcon(":/svg/res/default/images/redo_selected.svg"));
		}
		else
		{
			unDoAct->setIcon(QIcon(":/svg/res/default/images/redo.svg"));
		}
		return true;
	}
	else if (watched == p_seg_undo_btn_ || watched == p_align_undo_btn_)
	{
		if (event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverLeave)
		{
			//unDoAct->setIcon(QIcon(":/svg/res/default/images/undo_selected.svg"));
			unDoAct->setIcon(QIcon(":/svg/res/default/images/undo.svg"));
		}
		return true;
	}
	//}

	return QWidget::eventFilter(watched, event);
}

void MainWindow::onGetStageListSlot()
{
	std::vector<sFusionBSHStageData> stages = PFusionAlignData->getAnalyserData().stages_;

	if (p_compare_act_group_ == nullptr)
		p_compare_act_group_ = new QActionGroup(this);

	for (auto &stage : stages)
	{
		bool isExist = false;
		for (auto& pAct:p_align_toolbar_->getStageMenu()->actions())
		{
			if (pAct->text() == stage.stage_name_)
			{
				isExist = true;
				break;
			}
		}
		if (isExist) break;
		QAction *pAct = new QAction(stage.stage_name_, p_compare_act_group_);
		pAct->setCheckable(true);
		if(stage.stage_id_ != PFusionAlignData->getAnalyserData().cur_stage_id_)
			p_align_toolbar_->getStageMenu()->addAction(pAct);

		pAct->setData(stage.stage_id_);
		connect(pAct, &QAction::triggered, this, &MainWindow::openRecentStageProjSlot);
	}
}

void MainWindow::openRecentStageProjSlot()
{
	QAction *pAct = qobject_cast<QAction*>(sender());
	p_cur_compare_act_ = pAct;

	QString stageId = pAct->data().toString();

	if(PFusionAlignData->getAnalyserData().compare_stage_id_!=stageId)
	{
		PFusionAlignData->getAnalyserData().compare_stage_id_ = stageId;
		PFusionAlignData->getAnalyserData().compare_stage_name_ = pAct->text();
		PFusionAlignData->getAnalyserData().b_open_compare_ = true;
		// HTTP functionality removed - FusionAnalyser does not depend on network
	}
	else
	{
		if(b_compare_view_split_)
			pAct->setChecked(false);
		setSplit();
		PFusionAlignData->getAnalyserData().b_open_compare_ = false;
	}

	// For testing
	qDebug() << "openTest";
	//onOpenCompareModelSlot("");
}

bool MainWindow::checkProjectExist()
{
	QString projFile = PFusionAlignData->getProjectFilePath();

	QDir dir(projFile);
	dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
	QStringList filter;
	filter << "*.mlp" << "*.aln";
	QStringList projList = dir.entryList(filter);
	if (projList.isEmpty())
		return false;

	return true;
}

void MainWindow::setSplit()
{
	MultiViewer_Container* mvc = currentViewContainer();
	if (mvc && !b_compare_view_split_)
	{
		//emit PSIGNALMANAGER->updateTreatmentStepSignal(0, false);
		b_compare_view_split_ = true;

		GLArea* glwClone = new GLArea(this, mvc, &currentGlobalParams);
		mvc->addView(glwClone, Qt::Horizontal);
		updateMenus();

		glwClone->b_first_open_clone_ = true;
		glwClone->resetTrackBall();

		if (PFusionAlignData->upper_dental_features_ && PFusionAlignData->upper_dental_features_->base_plane_ready_)
		{
			glwClone->setViewTabPlane(GLA()->viewTabPlane);
			glwClone->cameraRotateEndAxis = GLA()->cameraRotateEndAxis;
			glwClone->cameraTransEndPoint = GLA()->cameraTransEndPoint;
			glwClone->bHavePreferViewDirect_ = GLA()->bHavePreferViewDirect_;
			glwClone->left_prefer_view_direct_ = GLA()->left_prefer_view_direct_;
			glwClone->right_prefer_view_direct_ = GLA()->right_prefer_view_direct_;
		}
		//glwClone->b_is_clone_ = true;
		glwClone->setGlareaIsClone(true);
		glwClone->udpateEachMeshVertexInfoBuffer();
		compare_view_id_ = glwClone->getId();
		//glwClone->getViewGui()->getUpDentalBtn()->setVisible(false);
		//glwClone->getViewGui()->getDownDentalBtn()->setVisible(false);
		glwClone->getViewGui()->setVisible(false);
		glwClone->update();

		MultiViewer_Container* mvc = currentViewContainer();
		for (int tt = 0; tt < mvc->viewerCounter(); ++tt)
		{
			GLArea* ar = mvc->getViewer(tt);
			ar->udpateEachMeshVertexInfoBuffer();
		}
	}
	else if (mvc && b_compare_view_split_)
	{
		mvc->removeView(compare_view_id_);
		compare_view_id_ = -1;
		b_compare_view_split_ = false;
	}

	//setUpperToothOperator();
}

void MainWindow::onSetWorkPathSlot(bool triggered)
{
	if (p_work_path_dlg_ == nullptr)
		p_work_path_dlg_ = new WorkDirSetingDlg(this);
	p_work_path_dlg_->show();
}

void MainWindow::getPatientFiles()
{
	// HTTP functionality removed - FusionAnalyser does not depend on network
	// Removed sensitive token and stage ID from commented code
}

void MainWindow::clearDecoratorState()
{

	auto reAddAction = [&](QAction* pAct, QToolBar* pToolBar)
	{
		if (pAct && pToolBar)
		{
			if (pAct->isChecked())
			{
				pAct->setChecked(false);
				pAct->triggered(false);
			}
			pToolBar->removeAction(pAct);
			pToolBar->addAction(pAct);
		}
	};
	if (p_align_toolbar_->getDecorateToolBar())
	{
		foreach(MeshDecorateInterface * iDecorate, PM.meshDecoratePlugins())
		{
			foreach(QAction * pAct, iDecorate->actions())
			{
				if (pAct == p_show_fdi_act_ || pAct == p_show_occlusion_act_)
					continue;
				if (pAct->data().toUInt() & E_ProgramAnalyser)
				{
					reAddAction(pAct, p_align_toolbar_->getDecorateToolBar());
				}
			}
		}
	}

	p_align_toolbar_->getDecorateToolBar()->setEnabled(true);

	if (p_align_toolbar_->getEditDecorateToolBar())
	{
		reAddAction(p_show_fdi_act_, p_align_toolbar_->getEditDecorateToolBar());
		reAddAction(p_show_occlusion_act_, p_align_toolbar_->getEditDecorateToolBar());
		//p_align_toolbar_->getAnalysisToolBar()->insertAction(p_show_fdi_act_, p_tooth_width_act_);
	}
}

void MainWindow::readAutoFeatureResult(QString fileName)
{
	if (!readFeatureSignData(fileName))
	{
		return;
	}
	updateDentalAnalysisDataSlot(false);
}

void MainWindow::closeEvent(QCloseEvent* event)
{

	// Auto-save project when closing program
	if (!meshDoc()->meshList.empty())
	{
		std::map<QMessageBox::StandardButton, QString>btns;
		btns[QMessageBox::Yes] = tr("Yes");
		btns[QMessageBox::No] = tr("No");
		btns[QMessageBox::Cancel] = tr("Cancel");
		//int index = UiUtilityTools::getInstance()->showQuestionMessageBox(tr("save project"), tr("whether save current project"));
		int index = UiUtilityTools::getInstance()->showSaveMessageBox(tr("save project"), tr("whether save %1 project").arg(PFusionAlignData->getShortProjectName()), btns, QMessageBox::Cancel);
		if (index < 0)
		{
			event->ignore();
			return;
		}
		else if (index)
		{
			autoSaveProject(false);
		}
	}
	return QMainWindow::closeEvent(event);
}

int MainWindow::whetherSaveProject()
{
	if (PFusionAlignData->getAnalyserData().cur_proj_stage_ != E_ProjAnaStageNone && !meshDoc()->meshList.empty())
	{
		std::map<QMessageBox::StandardButton, QString>btns;
		btns[QMessageBox::Yes] = tr("Yes");
		btns[QMessageBox::No] = tr("No");
		btns[QMessageBox::Cancel] = tr("Cancel");
		//int index = UiUtilityTools::getInstance()->showQuestionMessageBox(tr("save project"), tr("whether save current project"));
		int index = UiUtilityTools::getInstance()->showSaveMessageBox(tr("save project"), tr("whether save %1 project").arg(PFusionAlignData->getShortProjectName()), btns, QMessageBox::Cancel);
		if (index < 0)
		{
			return index;
		}
		else if (index)
		{
			autoSaveProject(false);
			return index;
		}
	}
	else
	{
		return 0;
	}
}

bool MainWindow::parseBoundaryPtsFromIndex()
{

	std::vector<int> upper_fdi = { 18, 17, 16, 15, 14, 13, 12, 11, 21, 22, 23, 24, 25, 26, 27, 28 };
	std::vector<int> lower_fdi = { 48, 47, 46, 45, 44, 43, 42, 41, 31, 32, 33, 34, 35, 36, 37, 38 };

	auto parsePts = [&](SegmentedStatusInfo* info, SeparationManager* pMesh, std::vector<int>& fdi_map)
	{
		if (info == nullptr || pMesh == nullptr) return false;

		for (int i_tooth = 1; i_tooth < info->borderVertexIndexEachTooth.size(); ++i_tooth)
		{
			if (!info->borderVertexIndexEachTooth[i_tooth].empty())
			{
				QString fdi = QString::number(fdi_map[i_tooth - 1]);

				std::vector<Point3m> boundry;
				for (auto index : info->borderVertexIndexEachTooth[i_tooth])
				{
					if(pMesh->original_mesh_->cm.vert.size() > index)
						boundry.push_back(pMesh->original_mesh_->cm.vert[index].cP());
				}

				info->feature_points_mark[fdi].boundryVertexsCrown = boundry;
			}
		}

		return true;
	};
	return false;
}

void MainWindow::onShowChangeLanguageDlgSlot(bool triggerd)
{
	if (p_change_language_dlg_ == nullptr)
	{
		p_change_language_dlg_ = new FusionChangeLanguageDlg( this);
	}
	p_change_language_dlg_->show();
}

void MainWindow::onChangeLanguageSlot(int index)
{
	PFusionAppData->setAppLanguage((eLanguageType)index);
	QSettings settings;

	int value = -1;
	if (settings.value(LanguageTypeKey).isValid())
		value = settings.value(LanguageTypeKey).toInt();
	qDebug() << index;
	qDebug() << value;
	if ((value == 1 && PFusionAppData->getAppLanguage() == E_ENGLISH) || (value == 0 && PFusionAppData->getAppLanguage() == E_CHINESE))
		return;

	UiUtilityTools::getInstance()->showInfoMessageBox(tr("info"), tr("Language Settings have changed,it will take effect after you restart FusionAnalyser"));
	qApp->installTranslator(&translator_);
}

void MainWindow::setHomePageVisible(bool visible)
{
	// Show main scene layout when home page is not visible
	if (_currviewcontainer)
	{
		_currviewcontainer->setVisible(!visible);
		GLA()->getViewGui()->setVisible(!visible);
	}

	if (p_home_gui_)
	{
		p_home_gui_->setShowState(visible);
	}
	saveProjectAct->setEnabled(!visible);
	if (visible)
	{
		clearOldProject();
	}
}

void MainWindow::open_stl_slot()
{
	SPDLOG->info("open stl");
	if (p_home_gui_)
	{
		p_home_gui_->importModelSlot();
	}
}

void MainWindow::quickOpenFile(QString file)
{
	QFileInfo file_info = QFileInfo(file);
	if (file_info.exists() && file_info.isFile())
	{
		QString suffix = "." + file_info.suffix().toLower();
		if (suffix == ".stl")
		{
			QStringList files;
			files << file;
			loadMeshMoelsSlot(files);
		}
		else if (suffix == ".mlp" || suffix == FusionAnalyserPrjSuffix)
		{
			openProject(file);
		}

	}
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
	if (GLA())
		GLA()->update();
	QMainWindow::resizeEvent(event);
	int width = this->width();
	int height = this->height();
	updateGUI();
}

void MainWindow::paintEvent(QPaintEvent* event)
{
	QMainWindow::paintEvent(event);
	qDebug() << this->width();
	qDebug() << this->height();
	if (b_need_update_ui_)
	{
		updateGUI();
		b_need_update_ui_ = false;
	}

	return;
}

void MainWindow::onNeedUpdatePluginSlot()
{
	b_need_update_ui_ = true;
}
