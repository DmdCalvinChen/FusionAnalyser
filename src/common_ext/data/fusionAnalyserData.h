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

#ifndef FUSIONANALYSERDATA_H
#define FUSIONANALYSERDATA_H

#include <QString>
#include <QUuid>
#include <vector>
#include <QObject>
#include "common/config.h"

#include "common_ext_global.h"

struct sFusionBSHProjFileData
{
	QString id_ = "";
	QString file_id_ = "";
	QString md5_ = "";
	QString file_name_ = "";
	QString time_ = "";
	bool b_downloaded_ = false;
};

struct sFusionBSHStageFileData
{
    enum eFileType
    {
        E_PIC = 0,
		E_DOWN_MODEL,
		E_DOWN_MODEL_SEGMENT_RESULT,
        E_UP_MODEL,
		E_UP_MODEL_SEGMENT_RESULT,
        E_PROJECT
    };
	QString file_id_ = "";
	QString file_name_ = "";
	QString file_expand_ = "";
	QString md5_ = "";
	int type_ = -1;
    eFileType file_type_= E_PIC;
	QString update_time_ = "";
    bool b_downloaded_ = false;
};
struct sFusionBSHStageData
{

    QString stage_id_ = "";
    QString stage_name_ = "";
    QString time_ = "";

    std::vector<sFusionBSHProjFileData> projs_;
    std::vector<sFusionBSHStageFileData> files_;

    sFusionBSHProjFileData latest_proj_;

};

struct SFusionBSHPatientData
{
    QString id_;
    QString patient_id_;
    QString card_no_ = "";
    QString card_id_ = "";
    QString patient_name_ = "";
    QString birthday_ = "";
    int height_ = 0;
    int weight_ = 0;
    QString chief_ = "";
    int sex_ = 0;
    QString custom_chief_ = "";
    QString all_chief_ = "";
    QString angle_ = "";
    QString plan_ = "";
    QString remarks_ = "";
    QString tel_ = "";
    QString tegistration_time_ = "";
    QString update_time_ = "";
    QString doctors_ = "";
};
struct COMMON_EXT_EXPORT sFusionAnalyserData
{
public:
    sFusionAnalyserData() {};

    std::vector<sFusionBSHStageData> stages_;
    QString patient_name_ = "";
    QString cur_stage_id_ = "";
    QString cur_stage_name_ = "";
    QString patient_id = "";
    QString compare_stage_id_ = "";
    QString compare_stage_name_ = "";

    SFusionBSHPatientData patient_data_;

    sFusionBSHStageData *cur_stage_=nullptr;
	sFusionBSHStageData* compare_stage_ = nullptr;

    bool b_open_compare_ = false;

	QString s_up_auto_seg_file_ = "";
	QString s_down_auto_seg_file_ = "";

    bool getIsBmu();

    eProjAnalyserStage cur_proj_stage_;
    eProjAnalyserStage pre_proj_stage_;
    eProjAnalyserStage Ppre_proj_stage_;

private:
    bool b_is_bmu_ = false;
};

class COMMON_EXT_EXPORT ToothFeatureConfig : public QObject
{
    Q_OBJECT
public:
    ToothFeatureConfig(QObject* parent = nullptr) ;
    ~ToothFeatureConfig();

    struct sToothFeatureConfigData
    {
        int fdi_;
        struct sFeatureNameConfig
        {
            QString mark_name_;
            QString name_;
            int type_;
            int index_ = -1;
        };
        std::vector<sFeatureNameConfig> marks_;
    };

    void readConfig(QString ConfigFilePath, bool bAll=false);

    std::map<int, sToothFeatureConfigData>getMarkConfig() const { return mark_configs_; }

    QString getMarkName(int fdi, QString& name, bool bAll = false);
    QString getName(int fdi, QString& makeName, bool bAll = false);
    int     getIndex(int fdi, QString name, bool bAll =false);

private:
	std::map<int, sToothFeatureConfigData> mark_configs_;
    std::map<int, sToothFeatureConfigData> all_mark_configs_;
    int type_;
};

#endif
