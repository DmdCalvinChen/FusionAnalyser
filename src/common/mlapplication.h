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

#ifndef ML_APPLICATION_H
#define ML_APPLICATION_H
#include <QTranslator>
#include <QApplication>
#include <QString>
#include <wrap/gl/gl_mesh_attributes_info.h>
#include "ml_mesh_type.h"

class MeshLabApplication : public QApplication
{
public:
    enum HW_ARCHITECTURE {HW_32BIT = 32,HW_64BIT = 64};
    MeshLabApplication(int &argc, char *argv[]):QApplication(argc,argv)
    {

    }

public:
    static void  loadTranslator(int type);

    ~MeshLabApplication()
    {
        removeTranslator(&translator_);
    }
    bool notify(QObject * rec, QEvent * ev);
    static const QString appName() { return app_name_; }
    static void  setAppName(QString appName) { app_name_ = appName; }
    static const QString architecturalSuffix(const HW_ARCHITECTURE hw) {return "";}
    static const QString appArchitecturalName(const HW_ARCHITECTURE hw) {return appName();}
    static const QString appVer() {return tr(""); }
	static const QString shortName() { return appName(); }
    static const QString completeName(const HW_ARCHITECTURE hw){return appArchitecturalName(hw); }
    static const QString organization(){return tr("AI-Align");}
    static const QString organizationHost() {return tr("https://www.ai-align.cn/");}
    static const QString webSite() {return tr("https://www.ai-align.cn/");}
    static const QString downloadSite() {return tr("https://www.ai-align.cn/");}

    static const QString pluginsPathRegisterKeyName() {return tr("pluginsPath");}
    static const QString versionRegisterKeyName() {return tr("version");}
    static const QString wordSizeKeyName() {return tr("wordSize");}

private:
    static QString app_name_;
    static QTranslator translator_;
    static bool  translator_loaded_;

};

#endif
