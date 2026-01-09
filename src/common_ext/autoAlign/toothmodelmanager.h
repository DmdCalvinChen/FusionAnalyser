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

#ifndef TOOTH_MODEL_MANAGER_H
#define TOOTH_MODEL_MANAGER_H

#include "meshExt/Tooth.h"
#include "meshExt/Dental.h"
#include "wrap/io_trimesh/export_stl.h"
#include "qdom.h"
#include "common/meshmodel.h"

// Manager for loading and saving tooth model files (STL mesh + XML data)
class ToothModelManager
{
public:
	ToothModelManager();
	~ToothModelManager();

	void addToothModelToDental(int _i_fdi, Dental *_dental, MeshDocument* _md);
	void loadToothModel(int _i_fdi, int i_tooth, Dental *_dental, MeshDocument *_md);
	bool parseToothModelData(QString _file_name, vector<int> &_boundary, vector<Point3m> &_convexhull);
	void dismantlingDental(Dental *_dental);
	void extractToothData(Tooth *_tooth, QString _folder);
	void saveToothMeshFile(Tooth *_tooth, QString _path, vcg::Matrix44f &_trans_matrix);
	void saveToothData(Tooth *_tooth, QString _path, const vcg::Matrix44f &_trans_matrix);
	void clearFiles(const QString &folderFullPath);
	bool ToothToXMLFile(Tooth *_tooth, QString _filename, const vcg::Matrix44f &_trans_matrix);
	QDomDocument collectToothDataDomDoc(Tooth *_tooth, const vcg::Matrix44f &_trans_matrix);
	QDomElement intToXML(const int num, QString node_name, QDomDocument& doc);
	QDomElement boundaryIndexListToXML(const vector<int> &_boundary, QDomDocument &doc);
	vector<int> parseBoundaryIndexList(QDomNode _node);
	QDomElement convexHullPointListToXML(const vector<Point3m> &_points, QDomDocument &doc, const vcg::Matrix44f &_trans_matrix);
	vector<Point3m> parseConvexHullPointsList(QDomNode _node);
};

#endif // TOOTH_MODEL_MANAGER_H
