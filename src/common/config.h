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

#ifndef _CONFIG_H
#define _CONFIG_H
#include "vcg/space/color4.h"
#include "qstring.h"
//#include <QObject>
//MACROS
#define SAFE_DELETE_ARRAY(p) do{if(p) {delete[] (p);    (p) = nullptr;} } while(0)
#define SAFE_DELETE(p)       do{if(p) {delete   (p);    (p) = nullptr;} } while(0)
#define SAFE_FREE(p)         do{if(p) {free(p);         (p) = nullptr;} } while(0)

#if defined(WIN32) || defined(_WIN32) || defined(_WIN32_) || defined(WIN64) || defined(_WIN64) || defined(_WIN64_)
#define PLATFORM_WINDOWS 1 // Windows platform
#elif defined(ANDROID) || defined(_ANDROID_)
#define PLATFORM_ANDROID 1 // Android platform
#elif defined(__linux__)
#define PLATFORM_LINUX	 1 // Linux platform
#elif defined(__APPLE__) || defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR) || defined(TARGET_OS_MAC)
#define PLATFORM_IOS	 1 // iOS/Mac platform
#else
#define PLATFORM_UNKNOWN 1
#endif

template <class T>
void clearVector(std::vector<T> &vec)
{
    std::vector<T>().swap(vec);
}

template <class T>
void clearPointerVector(std::vector<T*> &vec)
{
    for (auto &pvec : vec)
    {
        SAFE_DELETE(pvec);
    }
    std::vector<T*>().swap(vec);
}
//special part q value
const int QVALUE_GINGIVA = 0;
const int QVALUE_ACCESSORY = -1;
const int QVALUE_TOW_ACCESSORY = -2;
const int QVALUE_PRESSURE_ACCESSORY = -3;
const int QVALUE_ARTIFICIAL_TOOTH = -4;
const int QVALUE_COMPAREDOBJECT = -5;
const int QVALUE_QRCODE = -6;
const int QVALUE_TOOLING = -7;

// color
const vcg::Color4b CCOLOR_TOOTH(255, 255, 239, 255);
const vcg::Color4b CCOLOR_TOOTH_FIXED(0, 232, 232, 255);
const vcg::Color4b CCOLOR_GINGIVA(175, 51, 61, 255);

const vcg::Color4b CCOLOR_COMPARED_OGJECT(22, 132, 218, 200);

enum eProjAnalyserStage
{
    E_ProjAnaStageNone = 0,
    E_ProjAnaStageImport,    // Import model process
    E_ProjAnaStageSignalComplete,  // After single jaw landmarking completed
    E_ProjAnaStageSignalNotComplete,  // After single jaw landmarking not completed
    E_ProjAnaStageDualComplete, // After dual jaw landmarking completed
    E_ProjAnaStageDualNotComplete,  // After dual jaw landmarking not completed
    E_ProjAnaStageAnalyser,  // Analysis process
    E_ProjAnaStageMissUpper,  // Upper jaw missing tooth marking
    E_ProjAnaStageFixUpper,  // Upper jaw landmarking marking
    E_ProjAnaStageMissLower,  // Lower jaw or single jaw missing tooth marking
    E_ProjAnaStageFixLower,  // Lower jaw or single jaw landmarking marking
    E_ProjAnaStageComplete   // Landmarking completed

};
// proj stage
enum eProjStage
{
	E_ProjStageError = -1,
    E_ProjStageNone = 0,
    E_ProjStageStart = 0,             // Start
	E_ProjStageGetInfo = 0x003,
	E_ProjStageCreateProj = 0x0004,
};

enum eEditPluginFactoryType
{
    E_EditPluginMeasureAnaType
};

enum ePluginOrder
{
    E_PluginOrderOverlay,
    E_PluginOrderMeasureAnalyis,
    E_PluginOrderModelMark
};

enum eProgramType
{
    E_ProgramAnalyser = 0x01 << 4
};

#if PLATFORM_WINDOWS
// Windows platform
enum EditItemIndex {
    NONE = -1,
    MISSING_TOOTH, // Missing tooth position
    TEETH_WIDTH,// Tooth width measurement
    CUR_LENGTH_OF_DENTAL_ARCH,// Current dental arch length
    BOLTON_COMPLETELY,// Bolton overall ratio
    SPEE_CURVE_DEPTH,// Spee curve depth
    MOLAR_RELATIONSHIP,
    MIDLINE_RELATIONSHIP,
    ARCH_WIDTH,
    ARCH_LENGTH,
    GNATHOTECTUM_HEIGHT,
    OVERLAY_ANALYSIS,
    OCCLUSION_COLORING,
    DOCTOR_REMARK,
    BASAL_BONE_ARCH
};

#elif PLATFORM_IOS

enum EditItemIndex: int {
    NONE = -1,
    MISSING_TOOTH, // Missing tooth position
    TEETH_WIDTH,// Tooth width measurement
    CUR_LENGTH_OF_DENTAL_ARCH,// Current dental arch length
    BOLTON_COMPLETELY,// Bolton overall ratio
    SPEE_CURVE_DEPTH,// Spee curve depth
    MOLAR_RELATIONSHIP,
    MIDLINE_RELATIONSHIP,

    ARCH_WIDTH,
    ARCH_LENGTH,
    GNATHOTECTUM_HEIGHT,
    OVERLAY_ANALYSIS,
    OCCLUSION_COLORING,
    DOCTOR_REMARK,
    BASAL_BONE_ARCH
};

#endif

// Analysis tool report item translation mapping
#define  ITEM_MISSING_TOOTH         QObject::tr("Missing tooth")  // Missing tooth position record
#define  ITEM_TOOTH_WIDTH           QObject::tr("Tooth Width")    // Tooth width
#define  ITEM_CROWDING              QObject::tr("Crowding")       // Crowding degree
#define  ITEM_BOLTON_RATIO          QObject::tr("Bolton Ratio")   // Bolton ratio
#define  ITEM_SPEE                  QObject::tr("Spee")           // Spee curve
#define  ITEM_MOLAR_REL_RATIO       QObject::tr("Molar REL")      // Molar relationship
#define  ITEM_CENTERLINE_REL        QObject::tr("Centerline REL") // Midline relationship
#define  ITEM_ARCH_WIDTH            QObject::tr("Arch Width")     // Dental arch width
#define  ITEM_ARCH_LENGTH           QObject::tr("Arch Length")    // Dental arch length
#define  ITEM_BASAL_BONE            QObject::tr("Basal bone")     // Basal bone measurement
#define  ITEM_PALATAL_HEIGHT        QObject::tr("Palatal height") // Palatal height
#define  ITEM_OJ_OB                 QObject::tr("OJ/OB")         // Overjet/Overbite

#define CIRCLE_DIVIDE_NUMBER 360
#define PI_2X_ANGLE 360
#define PI 3.1415926
#define PI_2X  6.283185
#define STEP_RADS  0.0174532 /**((2*PI)/(CIRCLE_DIVIDE_NUMBER))*/
#define UP_TOOTHROOT_LENGHT 4.5f
#define LOW_TOOTHROOT_LENGHT 5.0f

//const QString DECORATE_PLUGIN_INFO = QObject::tr("decorate model");
#define  EDIT_DECORATE_PLUGIN_INFO  QObject::tr("decorate model")
#define  EDIT_DECORATE_ACTION        QObject::tr("decorate model")
#define  EDIT_OVERLAY_PLUGIN        QObject::tr("Overlay Analysis")
#define  EDIT_OVERLAY_ACTION        QObject::tr("Overlay Analysis")
#define  EDIT_TREATMENT_PLUGIN      QObject::tr("Generate treatment models")
#define  EDIT_TREATMENT_ACTION      QObject::tr("Generate treatment models")

#define EDIT_SEGMENT_ACTION_NAME         QObject::tr("segment")
#define EDIT_MODEL_MARKING_ACTION_NAME         QObject::tr("ModelMark")

#ifdef Q_OS_WIN
#define FONT_FAMILY_CURRENT_PLATFORM QString("Microsoft YaHei")
#endif
#ifdef Q_OS_MAC
#define FONT_FAMILY_CURRENT_PLATFORM  QString("PingFangSC-Regular")
#endif

// Right-click menu stylesheet
const QString StyleMenuPop("QMenu{background-color:rgba(250,251,254,1);color: #717377;font-family:Microsoft YaHei;font-size:14px;}"
	"QMenu::item{height:28px;font-family:Microsoft YaHei;font-size:14px;color: #717377;padding:4px 14px;margin:0px,4px;}"
	"QMenu::item:selected{background-color:rgb(236,250,253);}"
	"QMenu::item:hover{background-color:rgb(236, 250, 253);}"
);

#define EDIT_TOOLBUTTON_WIDTH 50
#define EDIT_TOOLBUTTON_HEIGHT 50

#define EDIT_PLUGIN_DIALOG_POS QPoint(70, 20)
#define EDIT_PLUGIN_ANALYSE_DIALOG_POS QPoint(0, 0)
#define EDIT_PLUGIN_DIALOG_WIDTH 370
#define EDIT_TOOLBAR_POS QPoint(10, 20)
#define EDIT_TOOLBAR_SEG_ADJ_POS  QPoint(10, 160)
#define EDIT_TOOLBAR_EXPRORT_POS QPoint(10, 100)
#define EDIT_OVERLAY_POS QPoint(0, 20)

#define FUSIONANALYSIER_PATH "/FusionAnalyser/"

#define SHOW_OCCLUSAL_ACT_OBJ_NAME QString("OCCLUSION")
#define SHOW_ACCESSORY_ACT_OBJ_NAME QString("show accessory act")
#define SHOW_REALROOT_ACT_OBJ_NAME QString("show tooth real tooth root act")
#define SHOW_JAWBONE_ACT_OBJ_NAME QString("show tooth jaw bone act")
#define SHOW_REAL_GINGIVA_ACT_NAME QString("show real gingiva")
#define SHOW_FDI_ACT_NAME QString("show fdi")
#define SHOW_OCCLUSION_ACT_NAME QString("show overlap")

#define EDIT_TOOTH_WIDTH_ANA_ACT_NAME  QString("teeth width ana")

const QString  defaultSkinName = "default";
const QString darkSkinName = "dark";
const QString MACHINECODEKEY = "machineID";

const QString UpdateHostUrl = "updateHost";

const QString AppLocalConfig = "appLocalConfig";

enum eLanguageType
{
    E_LAN_NONE=-1,
	E_CHINESE = 0,
	E_ENGLISH
};

enum eBoltonComputeType
{
    E_BOLOTON_NONE = -1,
    E_BOLOTON_SIX,
    E_BOLOTON_SEVEN
};

const QString LanguageTypeKey = "language";
const QString BoltonComputeKey = "boltonComputeKey";
const QString sDeviceHostKEY = "device_host";
const QString MisstoothGif = "misstoothgif";
const QString FixtoothGif = "fixtoothgif";
const QString OverlayGif = "overlaygif";
const QString TeethWidthGif = "teethwidthgif";
const QString NotTip = "tipsBtn";
const QString FusionAnalyserPrjSuffix = ".fa";
#endif // _CONFIG_H

