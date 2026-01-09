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

#include "mlapplication.h"
#include "mlexception.h"
#include <QMessageBox>
#include <vcg/complex/complex.h>
#include "../common_base/logsingleton.h"

#include <../common_base/SignalManager.h>
#include "../common_base/util/uitools.h"

QTranslator MeshLabApplication::translator_;
bool MeshLabApplication::translator_loaded_ = false;

QString MeshLabApplication::app_name_ = "FusionAnalyser";
bool MeshLabApplication::notify(QObject* rec, QEvent* ev)
{
	try
	{
		return QApplication::notify(rec, ev);
	}
	catch (MLException& e)
	{
		qCritical("%s", e.what());
	}
	catch (vcg::MissingComponentException& e)
	{
		qCritical("%s", e.what());
		abort();
	}
	catch (...)
	{
		qCritical("Something really bad happened!!!!!!!!!!!!!");
		emit PSIGNALMANAGER->saveFeaturePointsSignal();
		emit PSIGNALMANAGER->saveProjectSignal();// 程序异常崩溃前发射保存当前工程信号
		QString str = QObject::tr("A serious error occurred in the program");
		UiUtilityTools::getInstance()->showInfoMessageBox(QObject::tr("error"), str);

		SPDLOG->error(str);
		SPDLOG->flush();
	}
	return false;
}

void MeshLabApplication::loadTranslator(int type)
{
	if (!translator_loaded_)
	{
		if (type == 0)
		{
			translator_.load(":/common_ch.qm");
		}
		else if (type == 1)
			translator_.load(":/common_en.qm");
		qApp->installTranslator(&translator_);
		translator_loaded_ = true;
	}
}
