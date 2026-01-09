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

#ifndef SEARCHER_H
#define SEARCHER_H

#include<QString>
#include<QMap>
#include<QList>
#include<QAction>
#include<QRegExp>
#include<QVector>
#include<QSet>

class WordActionsMap
{
public:
	WordActionsMap();
	void addWordsPerAction(QAction& act,const QStringList& words);
	void removeActionReferences(QAction& act);
	bool getActionsPerWord( const QString& word,QList<QAction*>& res ) const;
private:
    QMap<QString,QList<QAction*> > wordacts;
};

class RankedMatches;

class WordActionsMapAccessor
{
public:
	WordActionsMapAccessor();
	void addWordsPerAction(QAction& act,const QString& st);
	inline void removeActionReferences(QAction& act) {map.removeActionReferences(act);}
	inline void setSeparator(const QRegExp& sep) {sepexp = sep;}
	inline void setIgnoredWords(const QRegExp& ign) {ignexp = ign;}
	int rankedMatchesPerInputString(const QString& input,RankedMatches& rm) const;
	inline QRegExp separtor() const {return sepexp;}
	inline QRegExp ignored() const {return ignexp;}

private:
	void purifiedSplit(const QString& input,QStringList& res) const;
	void addSubStrings(QStringList& res) const;
	WordActionsMap map;
	QRegExp sepexp;
	QRegExp ignexp;
};

class RankedMatches
{
public:
	RankedMatches();
	void getActionsWithNMatches(const int n,QList<QAction*>& res);
private:
	friend int WordActionsMapAccessor::rankedMatchesPerInputString(const QString& input,RankedMatches& rm) const;
	int computeRankedMatches(const QStringList& inputst,const WordActionsMap& map,bool matchesontitlearemoreimportant = true);
    QVector<QList<QAction*> > ranking;
};

#endif
