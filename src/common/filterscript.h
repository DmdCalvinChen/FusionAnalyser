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

#ifndef FILTERSCRIPT_H
#define FILTERSCRIPT_H

#include "filterparameter.h"

#include <QPair>

class QDomElement;

class FilterNameParameterValuesPair
{
public:
  virtual QString filterName() const = 0;
  virtual bool isXMLFilter() const = 0;
  virtual ~FilterNameParameterValuesPair(){}
};

class XMLFilterNameParameterValuesPair : public FilterNameParameterValuesPair
{
public:
  ~XMLFilterNameParameterValuesPair(){}
  bool isXMLFilter() const {return true;}
  QString filterName() const {return pair.first;}
  QPair< QString , QMap<QString,QString> > pair;
};

class OldFilterNameParameterValuesPair : public FilterNameParameterValuesPair
{
public:
  ~OldFilterNameParameterValuesPair(){}
  bool isXMLFilter() const {return false;}
  QString filterName() const {return pair.first;}
  QPair< QString , RichParameterSet > pair;
};

/*
The filterscipt class abstract the concept of history of processing.
It is simply a list of all the performed actions
Each action is a pair <filtername, parameters>
*/

class FilterScript : public QObject
{
    Q_OBJECT
public:
    FilterScript();
    ~FilterScript();
    bool open(QString filename);
    bool save(QString filename);
    QDomDocument xmlDoc();

    QList< FilterNameParameterValuesPair* > filtparlist;
    typedef QList< FilterNameParameterValuesPair* >::iterator iterator;

public slots:
    void addExecutedXMLFilter(const QString& name,const QMap<QString,QString>& parvalue);
};

#endif
