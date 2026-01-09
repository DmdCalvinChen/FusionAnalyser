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

#ifndef SIGNAL_MANAGER_H_
#define SIGNAL_MANAGER_H_

#include <QObject>
#include <qaction.h>
#include "common_base_global.h"

class SingleToothTreatmentInfomation;
struct sImageInfo;
enum EditItemIndex : int ;

class COMMON_BASESHARED_EXPORT SignalManager : public QObject
{
	Q_OBJECT

private:
	SignalManager(QObject* parent = nullptr);
	~SignalManager();
	static SignalManager* p_instance_;

public:
	class Garbo
	{
	public:
		~Garbo()
		{
			if (SignalManager::p_instance_)
				delete SignalManager::p_instance_;
		}
	};
	static Garbo garbo;

public:

	static SignalManager* getInstance()
	{
		if (p_instance_ == nullptr)
		{
			p_instance_ = new SignalManager;
		}
		return p_instance_;
	}

signals:
	// Work toolbar operation signal emission area
	void startSignal();
	void stopScanSignal();
	void clearSignal();
	void exportSignal();
	// Edit toolbar operation signal emission area
	void helpSignal();
	void centerFocusSignal();
	void rotateTransformSignal();
	void colorSetSignal();
	void editSignal();
	void tabUpperOrLowerSignal();
	// Top toolbar operation signal emission area
	void patientSignal();
	void scanSignal();
	void analyzeSignal();
	void uploadSignal();

	// Upper jaw marking completion notification signal
	void markingDoneSignal(bool succeed);

	// Single tooth marking completion notification signal
	void markingAnyOneDentalDoneSignal(bool succeed);

	// Save feature points before saving project
	void saveFeaturePointsSignal();
	void saveProjectSignal();
	void saveWhatProjectSignal(bool saveAlignProject = true);
	// Treatment preview dialog signals here
	void setUnfoldOrFoldStatusToTreatmentGuiSignal(bool bUnfold = true);
	void setUnfoldOrFoldStatusFromTreatmentGuiSignal(bool bUnfold = false);
	void setEnabledStatusFromTreatementGuiSignal(bool bEnabled = true);
	void setOverLapEnabled(bool bEnabled);

	// Treatment overview data transmission signal
	void setPatientInfomationSignal(
		const QString& patient_name, // Patient: Zhang San
		const QString& patient_id, // Patient ID: variable2 123456
		const QString& patient_doctor, // Patient doctor: variable2 Li Si
		const QString& patient_order_for_goods // Order number: variable2 909696
	);
	// SingleToothTreatmentInfomation removed - treatment functionality not used in FusionAnalyser
	// Treatment overview patient image resource file path information signal
	void photosPatientFileSourcePathSignal(const QString& _pathFileImage);
	// Switch view direction based on mouse click on patient image
	void setViewDirByPhotosMouseSelectedSignal(int viewDir);
	void switchDentalRenderObjectSignal();

	void setUpperOrLowerToothDataShowSignal(bool upper); // Signal for switching upper/lower jaw display in movement table
	void setCurrentTreatmentStepSignal(int index, bool isUpDental); // Signal for modifying current step data
	void updateTreatmentStepSignal(int index, bool isUpDental);     // Signal for updating related displays (movement table, treatment overview, model, playback UI, etc.), similar to broadcast message
	void updateDentalAnalysisDataReportSignal();
	void exportReportInfoPdf(); // Export PDF report information
	// Initialize check state of analysis report checkboxes based on item description matching corresponding checkbox property values
	void launchEachItemConfirmState(std::vector<std::pair<QString, bool>> _state_list);
	void setMeshDocCurrentMesh(int _index);

	void parseAlnProjFinishedSignal();                              // Signal emitted after project loading completes
	void setProgressBarSignal(int percent, QString content);        // Content update signal
	void setProgressBarSignal_async(int percent, QString content);  // Async content update signal
	void setToothAdjustFirstRunSignal();                            // Signal for initial toothadjust parsing, to be removed after project parsing reorganization

	void setMoveDistanceTabelUpDownSignal(bool up);      // Signal for switching upper/lower jaw
	void occlusionShowSignal();

	// Get vertex position information on any mesh from double-click in scene
	void transmitPickedPosSignal(float x, float y, float z);
	void setAccessoryVisibleActionSignal();
	void clickAccessoryVisibleBtnSignal();
	void getAccessoryVisibleBtnCheckStateSignal(bool&);

	void setNeedUpdateOverlayOutlineSignal(bool state); // Whether to refresh cut outline after updating animation model

	void setGlobalTrackBallEnableSignal(bool enble);

	// Project Parser signals
	void currentStepIdPlayAnimationSignal_PP(bool bUpOrDownControl, int _currentStepId);
	void onePartOfInitalAnimationOverSignal_PP();
	void updateMeshVertexInfoBuffer_PP();
	void updateMeshVertexInfoBufferCompare_PP();
	void updateToothAdjustTableNow_PP();
	void updateOverlaySectionOutlineInstantly_PP();

	void endEditSignal();                  // End signal for edit, connected to completion button of each page
	// Signal to call tooth adjustment plugin once after segmentation ends to generate gums
	void doToothAdjustPluginOnceSignal();
	// Delete a specific meshmodel
	void deleteMeshModelSignal(int);

	// Undo/redo signals
	void unDoSignal();
	void reDoSignal();
	void canUndoOrRedoStatusSignal(bool bCanUndo, bool bCanRedo);

	// Animation playback signal
	void setPlayAnimationGuiShowStatusIAndInitSignal(bool _bShowStatus, int _numStepUpper, int _numStepLower);

	// Update treatment overview information
	void updateTreamtmentStepGUISignal();

	// Set whether plugin can exit globally
	void setWhetherThePluginCanExit(bool);

	// Set tooth gap adjustment function switch
	void updateDentalRigidbodyActiveState(int);

	// Tooth crevice control
	void wantToLockCreviceWidth(int iCrevice, bool);
	void wantToUnlockCreviceWidth(int iCrevice, bool);
	void wantToAddCreviceWidth(int iCrevice, bool);
	void wantToSubtractingCreviceWidth(int iCrevice, bool);

	void setSelectDentalConsoleVisibleSignal(bool);
	void setViewDirectConsoleVisibleSignal(bool);
	void setDentalVisibleConsoleVisibleSignal(bool);
	void setLMRViewDirectConsoleVisibleSignal(bool);
	void setUpperDentalSelectedSignal();
	void setLowerDentalSelectedSignal();
	void onlyShowUpperDentalSignal(bool);
	void onlyShowLowerDentalSignal(bool);
	void enamelReductionBtnPushedSignal(bool);
	void occlusalColoringInProgress(bool);

	void setRealRootBtnEnableSignal(bool);
	void setJawBoneBtnEnableSignal(bool);
	bool setRealGingivaBtnEnableSignal(bool);
	void addActionToGlareaPerDocDecoratorlistSignal(QAction*);
	void removeActionFromGlareaPerDocDecoratorlistSignal(QAction*);

	void setTreatmentTabSelectItem(int, int);

    void updateSceneSignal();             // Notify glarea to update
	void updateSceneSizeSignal(int _width, int _height);
	void updateTrackBallEnableSignal(bool);
	void updatePluginUISignal();

	void updateProgressGuiSignal(const QString& text, int value, int max);
	void updateProgressGuiSignal_async(const QString& text, int value, int max);
	void showBusyProgressBarSignal(bool isBusy);
	void setToothAxisAdjCheckBoxEnableSignal(bool);

	void setSegmentModuleEnabledSignal(bool enabled);
	void updateDentalAnalysisDataSignal(bool _read_mlp_record);

	// Auto tooth arrangement related signals
	void autoSegToothSuccessfulSignal(bool up);
	// Tooth segmentation closure operation start signal
	void segmentStartSignal();
	// Tooth segmentation closure completion signal (async segmentation end notification)
	void segmentDoneSignal();
	void setOcclusalSplitSignal(bool);

	// FusionAnalyser signal related
	void createNewProjectSignal();
	void openCompareModelSignal(QString fileName);
	void getStageListSignal();

	void setEnableToothOcclusionSignal(bool);

	// Signal for whether to enter browsable analysis items
	void updateBrowserItemSignal();

	// Currently selected upper/lower jaw signal
	void setSelectDentalSignal(int dentalType);

	void setSendMsgBtnEnabledSignal(bool enable);
	void sendImagesInfoSignal(const std::map<QString, sImageInfo>&imageInfo);
	void checkTaskStatusSignal();

	void mutualExclusionWithOcclusalSignal();
	void mutualExclusionWithOverlaySignal();

	void triggerItemActionSignal(EditItemIndex _item_index);
	void updateAvaliableItemActionsSignal(std::vector<EditItemIndex> _avaliable_item_indexes);

	void changeLanguageSignal(int type = 0);       // Language change signal

	// Signal for setting whether current state is missing tooth marking or marking state
	void setCurrentMarkingStatusSignal(bool is_marking_status);
	// Show GIF image signal
	void showGifSignal(bool openGif);
	void showOverlaySignal();
	void showTeethWidthSignal();
	void openHelpSignal();

	void setSixToSixModeCheckedStateSignal(bool _state, bool _actrual_changed = false);
	void getQtDeviceToLogicalValueSignal(float _a, float& _b);
	void getQtLogicalToDeviceValueSignal(float _a, float& _b);
public slots:
};

extern  COMMON_BASESHARED_EXPORT SignalManager* p_signal_mgr;
#define PSIGNALMANAGER   p_signal_mgr
#endif
