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

#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <QObject>
#include <QList>
#include <QFileInfo>

#include "common_ext_global.h"

/**
 * FileManager: File and folder management utilities
 * Handles folder operations, deletion of old folders at specified paths
 * Provides file existence checking logic
 */

class COMMON_EXT_EXPORT FileManager : public QObject
{
	Q_OBJECT
public:
	static bool deleteFileOrFolder(const QString& strPath);
	static QList<QFileInfo> getDirNameList(const QString& strDirpath);
	/**
	 * Remove old folders
	 * @param strDirpath Folder path
	 * @param num_old_file_save Maximum number of old files to keep
	 */
	static void removeOldFolders(const QString& strDirpath, int num_old_file_save = 3);

	// Copy directory
	static bool copyDirectoryFiles(const QString& from_dir, const QString& to_dir, bool cover_file_If_exist = true);

	// Check if file exists, parameter is full path file name
	static bool isFileExist(const QString& full_file_name);

	// Check if file is being used. Returns true if file exists and is occupied
	static bool isFileUsed(QString fpath);
};

#endif
