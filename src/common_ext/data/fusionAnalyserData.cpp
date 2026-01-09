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

#include "fusionAnalyserData.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFileInfo>
#include <QCoreApplication>
#include <QSettings>
#include "common/config.h"

ToothFeatureConfig::ToothFeatureConfig(QObject* parent )
{

}

ToothFeatureConfig::~ToothFeatureConfig()
{

}

void ToothFeatureConfig::readConfig(QString ConfigFilePath, bool bAll)
{
	QFile configFile(ConfigFilePath);
	if (!configFile.open(QIODevice::ReadOnly))
	{
		return;
	}

	QByteArray config = configFile.readAll();

	QJsonParseError parseError;
	QJsonDocument configDoc = QJsonDocument::fromJson(config, &parseError);

	if (parseError.error != QJsonParseError::NoError)
	{
		return;
	}

	QJsonObject obj = configDoc.object();

	type_ = obj["type"].toInt();

	QJsonArray markArray = obj["landmarks"].toArray();

	for (auto toothMark : markArray)
	{
		QJsonObject toothMarkObj = toothMark.toObject();

		QJsonArray fdiArray = toothMarkObj["fdi"].toArray();
		QJsonArray marksArray = toothMarkObj["marks"].toArray();

		std::vector<sToothFeatureConfigData::sFeatureNameConfig> marks;
		for (auto mark : marksArray)
		{
			QJsonObject markObj = mark.toObject();
			sToothFeatureConfigData::sFeatureNameConfig featureName;
			featureName.name_ = markObj["name"].toString();
			featureName.mark_name_ = markObj["markName"].toString();
			featureName.type_ = markObj["type"].toInt();
			featureName.index_ = markObj["index"].toInt();

			marks.push_back(featureName);
		}
		for (auto fdi : fdiArray)
		{
			sToothFeatureConfigData feature;
			feature.fdi_ = fdi.toInt();
			feature.marks_ = marks;
			if (bAll)
				all_mark_configs_.insert(std::map<int, ToothFeatureConfig::sToothFeatureConfigData>::value_type(feature.fdi_, feature));
			else
				mark_configs_.insert(std::map<int, ToothFeatureConfig::sToothFeatureConfigData>::value_type(feature.fdi_, feature));
		}
	}
}

QString ToothFeatureConfig::getMarkName(int fdi, QString& name, bool bAll)
{
	std::map<int, sToothFeatureConfigData>::iterator mark;
	if (bAll)
	{
		mark = all_mark_configs_.find(fdi);
		if (mark == all_mark_configs_.end())
			return "";
	}
	else
	{
		mark = mark_configs_.find(fdi);
		if (mark == mark_configs_.end())
			return "";
	}

	auto markFeature = mark->second;

	for (auto& feature : markFeature.marks_)
	{
		if (feature.name_ == name)
			return feature.mark_name_;
	}

	return "";
}

QString ToothFeatureConfig::getName(int fdi, QString& markName, bool bAll)
{
	std::map<int, sToothFeatureConfigData>::iterator mark;
	if (bAll)
	{
		mark = all_mark_configs_.find(fdi);
		if (mark == all_mark_configs_.end())
			return "";
	}
	else
	{
		mark = mark_configs_.find(fdi);
		if (mark == mark_configs_.end())
			return "";
	}

	auto markFeature = mark->second;

	for (auto& feature : markFeature.marks_)
	{
		if (feature.mark_name_ == markName)
			return feature.name_;
	}

	return "";
}

int ToothFeatureConfig::getIndex(int fdi, QString name, bool bAll)
{
	std::map<int, sToothFeatureConfigData>::iterator mark;
	if (bAll)
	{
		mark = all_mark_configs_.find(fdi);
		if (mark == all_mark_configs_.end())
			return -1;
	}
	else
	{
		mark = mark_configs_.find(fdi);
		if (mark == mark_configs_.end())
			return -1;
	}

	auto markFeature = mark->second;

	for (auto& feature : markFeature.marks_)
	{
		if (feature.mark_name_ == name || feature.name_ == name)
			return feature.index_;
	}
	return -1;
}

bool sFusionAnalyserData::getIsBmu()
{
	// BMU customization removed - always return false for standard version
	return false;
}
