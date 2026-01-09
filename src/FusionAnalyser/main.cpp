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

#include <common/mlapplication.h>
#include <common_base/logsingleton.h>
#include <QMessageBox>
#include <QString>
#include <clocale>
#include <QPixmap>
#include <QDesktopWidget>

#include "mainwindow.h"
#ifdef Q_OS_WIN
// Breakpad crash reporting - Windows only
#include "client/windows/crash_generation/client_info.h"
#include "client/windows/crash_generation/crash_generation_server.h"
#include "client/windows/handler/exception_handler.h"
#include "client/windows/common/ipc_protocol.h"
#endif

#include "UI_Common/uicommon.h"
#include "common_ext/util/utility_tools.h"
#include "common_base/util/uitools.h"
#include "common_base/common_base.h"
#include "common_ext/common_ext.h"
#include "ui_common/UserCenterUI/workDirSetingDlg.h"
#ifdef VLD_CHECK_MEMORY
#define VLD_FORCE_ENABLE
#include "vld.h"
#endif

#ifdef Q_OS_WIN
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"wmainCRTStartup\"" )
#endif

#ifdef Q_OS_WIN
namespace
{
	static bool callback(const wchar_t* dump_path, const wchar_t* id,
		void* context, EXCEPTION_POINTERS* exinfo,
		MDRawAssertionInfo* assertion,
		bool succeeded)
	{
		if (succeeded)
		{
			printf("dump guid is %ws\n", id);
		}
		else
		{
			printf("dump failed\n");
		}
		fflush(stdout);
		return succeeded;
	}
}
#endif

#ifdef Q_OS_WIN
int wmain(int argc, wchar_t* wargv[])
{
	char** argv = new char* [argc];
	for (int i = 0; i < argc; ++i)
	{
		std::string arg = UtilityTools::getInstance()->wstring2string(std::wstring(wargv[i]));
		argv[i] = new char[arg.size() + 1];
		strcpy(argv[i], arg.c_str());
	}
#else
int main(int argc, char** argv)
{
#endif
	char* netParam = nullptr;
	if (argc == 2)
	{
		qDebug() << argv[1];
		if(strlen(argv[1])>7)
		{
			netParam = argv[1] + 7;
		}
		qDebug() << netParam;
	}

    Q_INIT_RESOURCE(common);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
#endif

	LogSingleton* logInstance = LogSingleton::getInstance();
	FusionAlignData::getInstance();

	cout << "2";

    MeshLabApplication app(argc, argv);
	MeshLabApplication::setAppName("FusionAnalyser");
	QString appName = QCoreApplication::applicationName();
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
	QCoreApplication::setOrganizationName(MeshLabApplication::organization());

	SPDLOG->info("============ init_log ==========");

    QTranslator translator;
	if (PFusionAppData->getAppSettingLanguage() == E_CHINESE)
	{
		translator.load(":/FusionAnalyser_ch.qm");
	}
	else if(PFusionAppData->getAppSettingLanguage() == E_ENGLISH)
	{
		translator.load(":/FusionAnalyser_en.qm");
	}

	app.installTranslator(&translator);
	// Load common library translation before related UI startup
	ui_common::UI_Common::loadTranslator(PFusionAppData->getAppSettingLanguage());

	MeshLabApplication::loadTranslator(PFusionAppData->getAppSettingLanguage());
	Common_base::loadTranslator(PFusionAppData->getAppSettingLanguage());
	Common_ext::loadTranslator(PFusionAppData->getAppSettingLanguage());

	QString tmp = MeshLabApplication::appArchitecturalName(
		MeshLabApplication::HW_ARCHITECTURE(QSysInfo::WordSize));
	QCoreApplication::setApplicationName(
		MeshLabApplication::appArchitecturalName(MeshLabApplication::HW_ARCHITECTURE(QSysInfo::WordSize)));

	QDesktopWidget* desktopWidget = QApplication::desktop(); // Get available desktop size
	QRect deskRect = desktopWidget->availableGeometry();  // Get device screen size
	QRect screenRect = desktopWidget->screenGeometry();
	qDebug() << deskRect.height();
	qDebug() << deskRect.width();
	PFusionAppData->setSkinName(darkSkinName);

	QFile file;
#ifdef Q_OS_WIN
	HDC desk = GetDC(NULL);
	float hDPI = GetDeviceCaps(desk, LOGPIXELSX);
	float vDPI = GetDeviceCaps(desk, LOGPIXELSY);
	int dpi = (hDPI + vDPI) / 2;
	qDebug() << dpi;
	if ((deskRect.height() >= 1100 && dpi < 150) || (deskRect.height() < 1100 && deskRect.height() >= 1000 && dpi < 140))
	{
		file.setFileName(":/qss/res/default/images/fusionAnalyser_dark2k.css");
	}
	else if ((deskRect.height() >= 1100 && dpi < 180 && dpi >= 150) || (deskRect.height() < 1100 && deskRect.height() >= 1000 && dpi >= 140 && dpi < 180))
	{
		file.setFileName(":/qss/res/default/images/fusionAnalyser_dark2k175.css");
	}
	else {
		file.setFileName(":/qss/res/default/images/fusionAnalyser_dark.css");
	}
#else
	// On macOS, use a simpler approach based on screen height
	if (deskRect.height() >= 1100)
	{
		file.setFileName(":/qss/res/default/images/fusionAnalyser_dark2k.css");
	}
	else if (deskRect.height() >= 1000)
	{
		file.setFileName(":/qss/res/default/images/fusionAnalyser_dark2k175.css");
	}
	else {
		file.setFileName(":/qss/res/default/images/fusionAnalyser_dark.css");
	}
#endif
	if (file.open(QIODevice::ReadOnly))
	{
		auto qssText = file.readAll();
		qApp->setStyleSheet(qssText);
	}

	file.close();
	if(PFusionAlignData->getAnalyserData().getIsBmu())
	{
		QSettings workSettings;
		if (!workSettings.value("FusionAnalyser WorkSpace").isValid())
		{
			WorkDirSetingDlg dlg;
			if (QDialog::Rejected == dlg.exec())
				return 0;
		}
	}

    MainWindow window;

	QString fileVersion = UtilityTools::getInstance()->GetFileVersion(qApp->applicationFilePath());

	QString token, fileName;
	if (argc == 2)
	{
		if(PFusionAlignData->getAnalyserData().getIsBmu())
		{
			int length = strlen(netParam);
			QString parmstr = QString(netParam);
			int index = parmstr.indexOf("/");
			parmstr = parmstr.left(index);
			QString param1 = UtilityTools::getInstance()->getCorrectUnicode(QByteArray(netParam, index));
			param1 = param1.left(param1.indexOf("\r"));
			QString param = QByteArray::fromBase64(param1.toLocal8Bit());

			qDebug() << param << "\n";

			SPDLOG->info(param);
			QStringList paraList = param.split("|");
			qDebug() << (paraList.size());

			if (paraList.size() <= 4)
			{

				UiUtilityTools::getInstance()->showInfoMessageBox(QObject::tr("error"), QObject::tr("net param error"));
				return 0;
			}
			else
			{
				PFusionAlignData->getAnalyserData().cur_stage_id_ = paraList[1];
				PFusionAlignData->getAnalyserData().cur_stage_name_ = paraList[2];
				PFusionAlignData->getAnalyserData().patient_id = paraList[3];
			}
		}
		else
		{
#ifdef Q_OS_WIN
			fileName = QString::fromWCharArray(wargv[1]);
#else
			fileName = QString::fromLocal8Bit(argv[1]);
#endif
			window.showMaximized();
			window.quickOpenFile(fileName);
		}
	}
	else
	{
		window.showMaximized();
	}

	window.getPatientFiles();

	logInstance->info(QObject::tr("window start..."));

	FileOpenEater *filterObj = new FileOpenEater(&window);
	app.installEventFilter(filterObj);
	app.processEvents();
    delete filterObj;
#ifdef Q_OS_WIN
	for (int i = 0; i < argc; ++i)
	{
		delete[]argv[i];
	}
	delete[] argv;
#endif

    int ret = -1;
    try
    {
        ret = app.exec();
    }
    catch (...)
    {
    }
    return ret;
}
