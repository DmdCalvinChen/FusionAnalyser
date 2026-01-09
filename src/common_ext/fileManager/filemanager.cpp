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

#include "filemanager.h"

#include <QDir>
#include <QList>
#include <QDateTime>
#include <QDirIterator>

bool FileManager::deleteFileOrFolder(const QString& strPath)
{
	if (strPath.isEmpty() || !QDir().exists(strPath))
		return false;

	QFileInfo FileInfo(strPath);

	if (FileInfo.isFile())
		QFile::remove(strPath);
	else if (FileInfo.isDir())
	{
		QDir qDir(strPath);
		qDir.removeRecursively();
	}
	return true;
}

QList<QFileInfo> FileManager::getDirNameList(const QString& strDirpath)
{
	QDir dir(strDirpath);
	QFileInfoList Info_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
	QListIterator<QFileInfo> Iterator(Info_list);
	QList<QFileInfo> strDirNameList;
	QFileInfo Info;
	while (Iterator.hasNext())
	{
		Info = Iterator.next();
		if (Info.isDir())
		{
			strDirNameList << Info;
		}
	}
	return strDirNameList;
}

void FileManager::removeOldFolders(const QString& strDirpath, int num_old_file_save)
{
	QList<QFileInfo> foldername = getDirNameList(strDirpath);
	if (foldername.size() > num_old_file_save)
	{
		QList<QDateTime> fileTime;
		for (int i = 0; i < foldername.size(); i++) {
			fileTime.append(foldername.at(i).lastModified());
		}

		qSort(fileTime.begin(), fileTime.end());

		for (int i = 0; i < (fileTime.size() - num_old_file_save); i++)
		{
			for (int j = 0; j < fileTime.size(); j++)
			{
				if (foldername.at(j).lastModified() == fileTime.at(i))
					deleteFileOrFolder(foldername.at(j).absoluteFilePath());
			}
		}
	}
}

bool FileManager::copyDirectoryFiles(const QString& from_dir, const QString& to_dir, bool cover_file_If_exist)
{
	QDir sourceDir(from_dir);
	QDir targetDir(to_dir);
	if (!targetDir.exists()) {
		// Create target directory if it doesn't exist
		if (!targetDir.mkdir(targetDir.absolutePath()))
			return false;
	}

	QFileInfoList fileInfoList = sourceDir.entryInfoList();
	foreach(QFileInfo fileInfo, fileInfoList)
	{
		if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
			continue;

		if (fileInfo.isDir())
		{
			// Recursively copy directory
			if (!copyDirectoryFiles(fileInfo.filePath(),
				targetDir.filePath(fileInfo.fileName()),
				cover_file_If_exist))
				return false;
		}
		else
		{
			// Remove old file if cover is allowed
			if (cover_file_If_exist && targetDir.exists(fileInfo.fileName()))
			{
				targetDir.remove(fileInfo.fileName());
			}

			// Copy file
			if (!QFile::copy(fileInfo.filePath(),
				targetDir.filePath(fileInfo.fileName()))) {
				return false;
			}
		}
	}
	return true;
}

bool FileManager::isFileExist(const QString& full_file_name)
{
	QFileInfo fileInfo(full_file_name);
	return fileInfo.isFile();
}

// Check if file exists and is being used. Returns true if file exists and is occupied
bool FileManager::isFileUsed(QString fpath)
{
	bool isUsed = false;
	QString fpathx = fpath + "x";
	QFile file(fpath);
	bool isExist = file.exists();

	if(isExist == true)
	{
		bool isCanRename = file.rename(fpath,fpathx);
		if(isCanRename == false)
		{
			isUsed = true;
		}
		else
		{
			file.rename(fpathx,fpath);
		}
	}
	file.close();
	return isUsed;
}
