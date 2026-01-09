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

#ifndef PATIENTINFO_H
#define PATIENTINFO_H

class PatientInfo
{
public:
	PatientInfo()
	{

	}
	~PatientInfo()
	{

	}

private:
	QString name;
	QString orderId;
	QString fileFolder;

public:
	QString gettFileFolder()const
	{
		return fileFolder;
	}
	QString getName()const
	{
		return name;
	}
	QString getOrderId() const
	{
		return orderId;
	}
	void setName(QString _name)
	{
		name = _name;
	}
	void setOrderId(QString _orderId)
	{
		orderId = _orderId;
	}
	void setFileFolder(QString _fileFolder)
	{
		fileFolder = _fileFolder;
	}

public:
	QFileInfoList fileInfoList;
	QStringList fileNameList;
	QString nameMark = "-p";
	QString orderIdMark = "-o";
	QString fileFolderMark = "-f";
	bool bHaveName = false;
	bool bHaveOrderId = false;
	bool bHaveFileFolder = false;
	void clearData()
	{
		bHaveName = false;
		bHaveOrderId = false;
		bHaveFileFolder = false;
		fileInfoList.clear();
		fileNameList.clear();
		name.clear();
		orderId.clear();
		fileNameList.clear();
	}

};

#endif // PATIENTINFO_H
