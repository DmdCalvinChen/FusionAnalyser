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

#include "searcher.h"
#include "mlexception.h"

#include <cmath>

WordActionsMap::WordActionsMap()
:wordacts()
{

}

void WordActionsMap::addWordsPerAction(QAction& act,const QStringList& words)
{
	foreach(QString word,words)
		wordacts[word].push_back(&act);
}

void WordActionsMap::removeActionReferences(QAction& act )
{
    for(QMap<QString,QList<QAction*> >::iterator it  = wordacts.begin();it != wordacts.end();++it)
		it.value().removeAll(&act);
}

bool WordActionsMap::getActionsPerWord( const QString& word,QList<QAction*>& res ) const
{
    QMap< QString,QList<QAction*> >::const_iterator it = wordacts.find(word);
	if (it != wordacts.end())
	{
		res = it.value();
		return true;
	}
	return false;
}

WordActionsMapAccessor::WordActionsMapAccessor()
:map(),sepexp(),ignexp()
{
	sepexp.setPattern("\\W+");
	ignexp.setPattern("\\b(an|the|of|it|as|in|by|and|or|for)\\b|\\b[a-z]\\b|'s\\b|\\.|<[^>]*>");
}

void WordActionsMapAccessor::addWordsPerAction(QAction& act,const QString& st )
{
	QStringList wlist;
	purifiedSplit(st,wlist);
	addSubStrings(wlist);
	map.addWordsPerAction(act,wlist);
}

int WordActionsMapAccessor::rankedMatchesPerInputString( const QString& input,RankedMatches& rm ) const
{
	QStringList inputlist;
	purifiedSplit(input,inputlist);
	return rm.computeRankedMatches(inputlist,map);
}

void WordActionsMapAccessor::purifiedSplit( const QString& input,QStringList& res ) const
{
	res.clear();
	QString tmp = input;
	tmp = tmp.toLower();
	tmp.remove(ignexp);
	res = tmp.split(sepexp,QString::SkipEmptyParts);
	res.removeDuplicates();
}

void WordActionsMapAccessor::addSubStrings( QStringList& res ) const
{
	QStringList resWithPrefix;
	foreach(QString str, res)
	{
		QString strPref = str;
		resWithPrefix.push_back(strPref);
		for(int i=0;i<str.length()-3;++i)
		{
			strPref.chop(1);
			resWithPrefix.push_back(strPref);
		}
	}
	resWithPrefix.removeDuplicates();
	res = resWithPrefix;
}

RankedMatches::RankedMatches()
:ranking()
{
}

int RankedMatches::computeRankedMatches( const QStringList& inputst,const WordActionsMap& map, bool matchesontitlearemoreimportant)
{
	QMap<QAction*, float> wordmatchesperaction;
	ranking.clear();
	int inputstsize = inputst.size();
	ranking.resize(inputstsize);
	float bonuspertitleword = 0.0f;
	if (matchesontitlearemoreimportant)
		bonuspertitleword = 1.0f / std::pow(10,inputstsize);
	foreach(const QString& st,inputst)
	{
		QList<QAction*> res;
		bool found = map.getActionsPerWord(st,res);
		if (found)
		{
			foreach(QAction* act, res)
			{
				++wordmatchesperaction[act];
				QString title = act->text().toLower().trimmed();
				if (title.contains(st))
					wordmatchesperaction[act] = wordmatchesperaction[act] + bonuspertitleword;
			}
		}
	}

	QMap<float, QList<QAction*> > rankedmatches;
	for (QMap<QAction*, float>::iterator it = wordmatchesperaction.begin(); it != wordmatchesperaction.end(); ++it)
		rankedmatches[it.value()].push_back(it.key());

	int maxindex = -1;
	for(QMap<float,QList<QAction*> >::iterator it = rankedmatches.end() - 1;it != rankedmatches.begin() - 1;--it)
	{
		int index = std::floor(it.key()) - 1;
		if (index >= ranking.size())
		{
			throw InvalidInvariantException("WARNING! Index contained in wordmatchesperaction it's out-of-bound.");
			return 0;
		}
		if (index > maxindex)
			maxindex = index;
		ranking[index].append(it.value());
	}
	return maxindex + 1;
}

void RankedMatches::getActionsWithNMatches( const int n,QList<QAction*>& res )
{
	res.clear();
	int index = n -1;
	if ((index >= ranking.size()) || (n < 1))
	{
		throw InvalidInvariantException(QString("WARNING! Parameter n MUST be in the range [1..") + QString::number(ranking.size()) + "].");
		return;
	}
	res = ranking[index];
}

