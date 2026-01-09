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

#ifndef LOGSINGLETON_H
#define LOGSINGLETON_H

#include <QTime>
#include <QSettings>
#include <QDir>
#include <QString>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include "common_base_global.h"

// Log: use log to record some thing

class COMMON_BASESHARED_EXPORT LogSingleton
{
private:
	LogSingleton();
	~LogSingleton(){
	}

	static LogSingleton *p_log_instance_;
	void removeOldLogFile(const QDir &fromDir, const QStringList &filters, int numMaxFileShouldDelete);
    bool b_init_ = false;

public:
    class Garbo
    {
    public:
        ~Garbo()
        {
            if (LogSingleton::p_log_instance_)
            {
                p_log_instance_->log->flush();
                delete LogSingleton::p_log_instance_;
            }
        }
    };
    static Garbo garbo;
public:
	std::shared_ptr<spdlog::logger> log;

	static LogSingleton* getInstance()
	{
        if (p_log_instance_ == nullptr)
            p_log_instance_ = new LogSingleton;
        return p_log_instance_;
	}

    void init();

    void trace(const std::string &msg)      {if(!b_init_) {init();b_init_ =true;} log->trace(msg.c_str()); }
    void trace(const char* msg)             {if(!b_init_) {init();b_init_ =true;} log->trace(msg); }
    void trace(const QString &msg)          {if(!b_init_) {init();b_init_ =true;} log->trace(msg.toLocal8Bit()); }

    void debug(const std::string &msg)      {if(!b_init_) {init();b_init_ =true;} log->debug(msg.c_str()); }
    void debug(const char* msg)             {if(!b_init_) {init();b_init_ =true;} log->debug(msg); }
    void debug(const QString &msg)          {if(!b_init_) {init();b_init_ =true;} log->debug(msg.toLocal8Bit()); }

    void info(const std::string &msg)       {if(!b_init_) {init();b_init_ =true;} log->info(msg.c_str()); }
    void info(const char* msg)              {if(!b_init_) {init();b_init_ =true;} log->info(msg); }
    void info(const QString& msg)           {if(!b_init_) {init();b_init_ =true;} log->info(msg.toLocal8Bit()); }

    void warn(const std::string &msg)       {if(!b_init_) {init();b_init_ =true;} log->warn(msg.c_str()); }
    void warn(const char* msg)              {if(!b_init_) {init();b_init_ =true;} log->warn(msg); }
    void warn(const QString &msg)           {if(!b_init_) {init();b_init_ =true;} log->warn(msg.toLocal8Bit()); }

    void error(const std::string &msg)      {if(!b_init_) {init();b_init_ =true;} log->error(msg.c_str()); }
    void error(const char* msg)             {if(!b_init_) {init();b_init_ =true;} log->error(msg); }
    void error(const QString &msg)          {if(!b_init_) {init();b_init_ =true;} log->error(msg.toLocal8Bit()); }

    void critical(const std::string &msg)   {if(!b_init_) {init();b_init_ =true;} log->critical(msg.c_str()); }
    void critical(const char* msg)          {if(!b_init_) {init();b_init_ =true;} log->critical(msg); }
    void critical(const QString &msg)       {if(!b_init_) {init();b_init_ =true;} log->critical(msg.toLocal8Bit()); }

    void flush()                    { log->flush(); }
};

#ifdef Q_OS_WIN
#define  SPDLOG LogSingleton::getInstance()
#else
#define SPDLOG LogSingleton::getInstance()
#endif

#define  SPDLOG_FUNC LogSingleton::getInstance()->info(__FUNCTION__);
#define  SPDLOG_LINE LogSingleton::getInstance()->trace(QString(__FILE__) + QString(" %1").arg(__LINE__));
#endif // !LOGSINGLETON_H

