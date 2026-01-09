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

#ifndef FUSIONALIGNDATA_H
#define FUSIONALIGNDATA_H

#include <qobject.h>
#include <vector>
#include "common/config.h"
#include "common_ext_global.h"
#include "dentalmanager.h"
#include "separationmanager.h"
#include "data/fusion/segmentedstatusrecord.h"

#include "common_base/SignalManager.h"
#include "meshExt/dentalfeatures.h"
#include "data/fusionAnalyserData.h"
#include "data/fusionDataCommon.h"

class DentalAnalysisData;

struct sExportResultFileInfo
{
    enum eExportFileType
    {
        EEXPORTFILETYPE_NORMAL,
        EEXPORTFILETYPE_ACCESSORY,
        EEXPORTFILETYPE_QR,
        EEXPORTFILETYPE_ROBOTARM,
        EEXPORTFILETYPE_NCMACHINE
    };

    eExportFileType type_;
    QString file_path_;
    QString file_name_;
    int step_;
};

struct sAutoSegResult
{
	bool b_seg_ = false;
	QString s_seg_result_;
	void init()
	{
		b_seg_ = false;
		s_seg_result_.clear();
	}
};

enum eSkinType
{
	E_SkinDefault = 0,
	E_SkinWhite = 0,
	E_SkinBlack = 1
};

class COMMON_EXT_EXPORT FusionAppData : public QObject
{
	Q_OBJECT
public:
	FusionAppData(QObject* parent = nullptr);
	~FusionAppData();

	eSkinType getSkin() const { return skin_; }
	void setSkin(eSkinType val) { skin_ = val; }
	QString getSkinName() const { return skinName; }
	void setSkinName(QString val) { skinName = val; }

public:
	class Garbo
	{
	public:
		~Garbo()
		{
			if (FusionAppData::p_instance_)
				delete FusionAppData::p_instance_;
		}
	};
	static Garbo appGarbo_;

	bool getAppIsLocal() const { return b_local_; }
	void setAppIsLocal(bool val) { b_local_ = val; }
	eLanguageType getAppLanguage();
	eLanguageType getAppSettingLanguage();
	void setAppLanguage(eLanguageType type);
public:
	static FusionAppData* getInstance()
	{
		if (p_instance_ == nullptr)
		{
			p_instance_ = new FusionAppData;
		}
		return p_instance_;
	}

private:
	eSkinType skin_ = E_SkinDefault;
	QString   skinName = "default";

	bool b_local_ = false;

	static FusionAppData* p_instance_;
	eLanguageType app_language_type_ = E_LAN_NONE;
};

class COMMON_EXT_EXPORT FusionAlignData :	 public QObject
{
	Q_OBJECT
public:
	FusionAlignData(QObject * p = nullptr);
	~FusionAlignData();
	void deleteMemory();

	static FusionAlignData *p_instance_;
	static FusionAlignData* p_cur_proj_data_;
	DentalManager *upper_dental_ = nullptr, *lower_dental_ = nullptr;
	DentalManager* cur_dental_ = nullptr;
	AbsMesh* upper_compared_source_mesh_ = nullptr, *lower_compared_source_mesh_ = nullptr;
	DentalFeatures* upper_dental_features_ = nullptr, * lower_dental_features_ = nullptr;
	DentalAnalysisData* dental_analysis_ = nullptr;
	int dental_summary_ = 0;
	bool bUpperDentalSelected = true;
	QMap<MeshModel*, CloudOctree*> mesh_cloud_Octree_;

	SeparationManager *p_upper_mesh_ = nullptr, *p_lower_mesh_ = nullptr;
	SeparationManager *p_cur_mesh_ = nullptr;

    eProjStage getProjStage() const
	{
		return proj_stage_;
	}
    void setProjStage(eProjStage val)
	{
        last_stage_ = proj_stage_;
		proj_stage_ = val;
	}
    void rollbackStage()
    {
        proj_stage_ = last_stage_;
    }
	QString getShortProjectName()
	{
		QFileInfo file_info(align_proj_file_name_);
		return file_info.fileName().remove("." + file_info.suffix());
	}

	QString getAlignProjFileName() const { return align_proj_file_name_; }
    void setAlignProjFileName(QString val) { align_proj_file_name_ = val; }
    QString getFusionSlnFileName() const
	{
		return fas_file_name_;
	}
    void setFusionSlnFileName(QString val) { fas_file_name_ = val; }
    QMap <sAlignProj::eAlignProjType, QVector<sAlignProj>>& getAlignProjs()  { return align_projs_; }
    QVector<sAlignProj> &getTempAlignProjs()
	{
		return align_projs_[sAlignProj::EALIGNPROJTYPE_TEMPORARY];
	}
    QVector<sAlignProj>&getSubmitAlignProjs()
	{
		return align_projs_[sAlignProj::EALIGNPROJTYPE_SUBMIT];
	}
    QString getAlignProjOriFileName() const { return align_proj_origin_file_name_; }
    void setAlignProjOriFileName(QString val) { align_proj_origin_file_name_ = val; }
    bool  getAlignProjectByName(QString alignProjectName, sAlignProj *&proj);
    sAlignProj& getAlignProjectByName(QString alignProjName);
    QVector<sExportResultFileInfo>& getExportFiles() {
        return export_files_;
    }

	void pulledOutPickedToothAndAutoAlign();
	void reAutoAlignDetnal(DentalManager *_manager);
	ToothFeatureConfig &getToothFeatureConfig()  { return tooth_feature_config_; }

	eBoltonComputeType getBoltonMode() const { return bolton_mode_; }
	void setBoltonMode(eBoltonComputeType val) { bolton_mode_ = val; }

	void deleteUpperDentalData();
	void deleteLowerDentalData();

public:
	class Garbo
	{
	public:
		~Garbo()
		{
			if (FusionAlignData::p_instance_)
				delete FusionAlignData::p_instance_;
		}
	};
	static Garbo garbo;

public:
	static FusionAlignData* getInstance()
	{
		if (p_instance_ == nullptr)
		{
			p_instance_ = new FusionAlignData;
			p_cur_proj_data_ = p_instance_;
		}
		return p_instance_;
	}

	vector<MeshModel*> sortMeshModelPointers(Point3m _camera_pos);
	vector<MeshModel*> sortUpperMeshModelPointers(Point3m _camera_pos);
	vector<MeshModel*> sortLowerMeshModelPointers(Point3m _camera_pos);
	vector<MeshModel*>sortComparableMeshModelPointers(Point3m _camera_pos);
	vector<MeshModel*>sortComparableSourceMeshPointers(Point3m _camera_pos);
	vector<MeshModel*>sortComparableSourceMeshPointersSegment(Point3m _camera_pos);
	vector<MeshModel*> sortMeshModelPointersSegment(Point3m _camera_pos);
	void setUpperManagerSelected(bool _yes);
	void setUpperDentalObjectsVisible(bool _visible);
	void setLowerDentalObjectsVisible(bool _visible);
	void createUpperDentalManager();
	void createLowerDentalManager();
	DentalManager* upperManager();
	DentalManager* lowerManager();
	DentalManager* curManager();
	DentalManager* anotherManager();
	DentalAnalysisData* analysisData();
	void setDentalSummary(int _sum);
	void initialDentalAnalysisData(SegmentedStatusInfo& _info);
	void initialDentalAnalysisDataFeatureMode(SegmentedStatusInfo& _info, bool _mark_points_modified, int _screen_width, int _screen_height, bool _read_record = true);

    void clearData();
    void clearSolutionData();

	void setCrowdingAnalysisModeSlot(bool _state);

	inline int comparedStageIndex() { return compared_stage_index_; }
	inline void setComparedStageIndex(int _index) { compared_stage_index_ = _index; }

    QString getProjectFilePath()
	{
		return proj_file_path_;
	}
	void setProjectFilePath(const QString &projFilePath)
	{
		proj_file_path_ = projFilePath;
	}

	sFusionAnalyserData& getAnalyserData() { return fusion_analyser_data_; }
public slots:
    void onSetCurrentTreatmentStepSlot(int stage, bool isCompare);
	void unFocusFramesSlot(int _direction);

signals:
    void updateTreatmentStepSignal(int index, bool isUpDental);
	void haveChangedSelectedJaw();

public slots:

private:
	MeshDocument *p_mesh_doc_ = nullptr;

    bool b_have_occlusal_plane_ = false;
	CustomPlane view_plane_;

	QString proj_file_path_;
	QString align_proj_file_name_;
	QString align_proj_origin_file_name_ = "align_origin.aln";
	QString fas_file_name_;

    QMap <sAlignProj::eAlignProjType, QVector<sAlignProj>> align_projs_;

	eProjStage proj_stage_ = E_ProjStageNone;
    eProjStage last_stage_ = E_ProjStageNone;
	QVector<sExportResultFileInfo> export_files_;
	int compared_stage_index_ = 0;
	sFusionAnalyserData fusion_analyser_data_;
	ToothFeatureConfig tooth_feature_config_;
	eBoltonComputeType bolton_mode_ = E_BOLOTON_NONE;
};

#define PFusionAlignData FusionAlignData::p_cur_proj_data_
#define PFusionAppData FusionAppData::getInstance()

#endif
