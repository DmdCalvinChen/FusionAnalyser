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

#ifndef FILTERTHREAD_H
#define FILTERTHREAD_H

#include <QThread>
#include "mainwindow.h"

class FilterThread : public QThread
{
    Q_OBJECT
public:
    //Ideally PluginManager parameter should be const
    //and MainWindow parameter should not exist!
	FilterThread(const QString& fname,const QMap<QString,QString>& parexpval,PluginManager& pm, MeshDocument& md,MainWindow* mw);
    ~FilterThread();
    inline bool succeed() const {return _success;}
    inline QString filterName() const {return _fname;}
    static bool localCallBack(const int pos, const char * str);

protected:
    void run();

signals:
    void threadCB(const int pos,const QString& str);
private:
    static FilterThread* _cur;
    QString _fname;
    QMap<QString,QString> _parexpval;
    //ideally this should be const
    PluginManager& _pm;
    MeshDocument& _md;
    QGLWidget* _glwid;
    bool _success;

	//WARNING!!!!!!!!!!!!!!!MainWindow parameter should not exist at all! it has been introduced only to give access to global parameters to the script environment
    MainWindow* _mw;
	/*****************************************************************************************************************************************************************/
};

#endif // FILTERTHREAD_H
