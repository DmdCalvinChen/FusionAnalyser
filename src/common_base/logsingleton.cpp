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

#include "logsingleton.h"

#include <chrono>
#include <QCoreApplication>
#include <QDir>

LogSingleton::Garbo  LogSingleton::garbo;

void LogSingleton::init()
{
	try
	{

		QString appName = QCoreApplication::applicationName();
		QString path = qApp->applicationDirPath();
		if (path == "")
			path = QDir::currentPath();
		path += "/logs";
		QDir dir(path);
		if (!dir.exists())
		{
			dir.mkdir(path);
		}

		QSettings setting(path+"/log.ini", QSettings::IniFormat);

		QString content("trace, debug, info, warning, error, critical, off");
		if (setting.value("All-Level-parameter-Of-Log").toString().isEmpty())
		{
			setting.setValue(QString("All-Level-parameter-Of-Log"), content);
		}

		int numMaxLogFileExist = setting.value("NumOldLogFileExist").toInt();
		if (numMaxLogFileExist <= 0)
		{
			setting.setValue("NumOldLogFileExist", 1000);
			numMaxLogFileExist = 1000;
		}
		QDir fromDir(path);

		QStringList filters;
		filters.append("*.txt");

		// Level of log set from ini
		QString levelLogStr = setting.value("LevelOfLog").toString();
		if (levelLogStr.isEmpty())
		{
			setting.setValue("LevelOfLog", "trace");
			levelLogStr = "trace";
		}
		std::string level = levelLogStr.toStdString();
		auto log_level = spdlog::level::from_str(level);

		QDateTime currentDateTime = QDateTime::currentDateTime();
		QString fileName = QString(path+"/") + appName + QString("_log") + QString(".txt");
		QByteArray cdata = fileName.toLocal8Bit();
		std::string  stdFileName(cdata);
		if (log_level != spdlog::level::off)
		{
		}
		log = spdlog::rotating_logger_mt("logger", stdFileName, 1024 * 1024 * 5, 5);
		log->set_level(log_level);
		log->flush_on(log_level);
		spdlog::register_logger(log);
		spdlog::flush_every(std::chrono::seconds(60));
	}
	catch (const spdlog::spdlog_ex& ex)
	{
	}
}

LogSingleton* LogSingleton::p_log_instance_ = nullptr;

LogSingleton::LogSingleton()
{

}

void LogSingleton::removeOldLogFile(const QDir &fromDir, const QStringList &filters, int numMaxFileShouldDelete)
{
	QDir *dir = new QDir(fromDir);
	dir->setNameFilters(filters);
	QList<QFileInfo> fileInfo = QList<QFileInfo>(dir->entryInfoList(filters));

	if (fileInfo.size() > numMaxFileShouldDelete)
	{
		QList<QDateTime> fileTime;
		for (int i = 0; i < fileInfo.size(); i++) {
			fileTime.append(fileInfo.at(i).lastModified());
		}

		qSort(fileTime.begin(), fileTime.end());

		for (int i = 0; i < (fileTime.size() - numMaxFileShouldDelete); i++) {
			for (int j = 0; j < fileTime.size(); j++) {
				if (fileInfo.at(j).lastModified() == fileTime.at(i))
					dir->remove(fileInfo.at(j).fileName());
			}
		}
	}
    delete dir;
}

