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

#include "toothmodelmanager.h"

ToothModelManager::ToothModelManager()
{
}

ToothModelManager::~ToothModelManager()
{
}

void ToothModelManager::addToothModelToDental(int _i_fdi, Dental *_dental, MeshDocument* _md)
{
	if (!_dental || !_md)
	{
		return;
	}
	int tooth_index = _dental->getToothIndexAccording(_i_fdi);
	if (_dental->bToothExist[tooth_index])
	{
		return;
	}

	loadToothModel(_i_fdi, tooth_index, _dental, _md);
}

void ToothModelManager::loadToothModel(int _i_fdi, int i_tooth, Dental *_dental, MeshDocument *_md)
{
	QString folder_path = QCoreApplication::applicationDirPath() + QString::fromStdString("/teethmodels");
	QString file_name = folder_path + QString("/") + QString::number(_i_fdi);
	_dental->bToothExist[i_tooth] = true;

	// Determine if upper or lower tooth based on FDI notation
	bool bUpper = false;
	if ((_i_fdi / 10) == 1 || (_i_fdi / 10) == 2)
	{
		bUpper = true;
	}

	// Load STL mesh file
	QString meshFileName = file_name + QString(".stl");
	MeshModel* new_model = _md->addNewMesh("", TOOTHRD_LABEL);
	if (bUpper)
	{
		new_model->upperOrLowerToothModelMark = UpperSingleTooth;
	}
	else
	{
		new_model->upperOrLowerToothModelMark = LowerSingleTooth;
	}
	int err, loadmask;
	err = vcg::tri::io::ImporterSTL<CMeshO>::Open(new_model->cm, (meshFileName.toStdString()).c_str(), loadmask);
	if (err != 0)
	{
		const char* errmsg = vcg::tri::io::ImporterSTL<CMeshO>::ErrorMsg(err);
		return;
	}
	_dental->teeth[i_tooth].InitalTooth(&new_model->cm, i_tooth, bUpper, Axis(), QString::number(_i_fdi), false);
	_dental->teeth[i_tooth].p_model_ = new_model;

	// Load XML data file (boundary and convex hull)
	QString dataFileName = file_name + QString(".xml");
	vector<int> boundary_list;
	vector<Point3m> convex_hull_verts;
	parseToothModelData(dataFileName, boundary_list, convex_hull_verts);
	_dental->teeth[i_tooth].boundaryIndex = boundary_list;
	_dental->teeth[i_tooth].frozenConvexVerts = convex_hull_verts;
	_dental->teeth[i_tooth].convexVerts = convex_hull_verts;
	_dental->teeth[i_tooth].frozenConvexHullCenter = _dental->teeth[i_tooth].localAxis.centerPoint;
	_dental->teeth[i_tooth].convexHullCenter = _dental->teeth[i_tooth].localAxis.centerPoint;
}

bool ToothModelManager::parseToothModelData(QString _file_name, vector<int> &_boundary, vector<Point3m> &_convexhull)
{
	QFile qf(_file_name);
	QFileInfo qfInfo(_file_name);
	QDir tmpDir = QDir::current();
	QDir::setCurrent(qfInfo.absoluteDir().absolutePath());
	if (!qf.open(QIODevice::ReadOnly))
	{
		return false;
	}
	QString errorMsg;
	QDomDocument doc("MeshLabDocument");
	if (!doc.setContent(&qf, &errorMsg))
	{
		return false;
	}
	QDomElement root = doc.documentElement();
	QDomNode node = root.firstChild();
	int tooth_index, tooth_fdi;
	while (!node.isNull())
	{
		if (node.nodeName() == QString("ToothIndex"))
		{
			tooth_index = node.firstChild().nodeValue().toInt();
		}

		if (node.nodeName() == QString("ToothFDI"))
		{
			tooth_fdi = node.firstChild().nodeValue().toInt();
		}

		if (node.nodeName() == QString("BoundaryIndexes"))
		{
			_boundary = parseBoundaryIndexList(node);
		}

		if (node.nodeName() == QString("ConvecHullPoints"))
		{
			_convexhull = parseConvexHullPointsList(node);
		}
		node = node.nextSiblingElement();
	}
	return true;
}

void ToothModelManager::dismantlingDental(Dental *_dental)
{
	if (!_dental)
	{
		return;
	}
	// Clear model file folder and extract tooth data
	QString folder_path = QCoreApplication::applicationDirPath() + QString::fromStdString("/teethmodels");
	clearFiles(folder_path);

	for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (_dental->bToothExist[i])
		{
			extractToothData(&_dental->teeth[i], folder_path);
		}
	}
}

void ToothModelManager::extractToothData(Tooth *_tooth, QString _folder)
{
	QString file_path = _folder + QString("/") + _tooth->strFDI;
	vcg::Matrix44f trans_matrix;
	saveToothMeshFile(_tooth, file_path, trans_matrix);
	saveToothData(_tooth, file_path, trans_matrix);
}

void ToothModelManager::saveToothMeshFile(Tooth *_tooth, QString _path, vcg::Matrix44f &_trans_matrix)
{
	// Compute transformation matrix from local axis
	Axis new_local_axis;
	Point3m move, rotate;
	_tooth->localAxis.computeTransformVectors(new_local_axis, move, rotate);
	HexaVec transVec(move, rotate);
	_trans_matrix = _tooth->localAxis.constructTransformMatrix(transVec);

	// Transform mesh vertices and save to STL file
	CMeshO save_mesh;
	vcg::tri::Append<CMeshO, CMeshO>::MeshCopy(save_mesh, *_tooth->p_mesh_);
	for (int i = 0; i < save_mesh.vert.size(); ++i)
	{
		save_mesh.vert[i].P() = _trans_matrix * save_mesh.vert[i].P();
	}
	_tooth->updateModel(&save_mesh);

	QString tempFileName = _path + QString::fromStdString(".stl");
	string fileName = QFile::encodeName(tempFileName).constData();
	vcg::tri::io::ExporterSTL<CMeshO>::Save(save_mesh, fileName.c_str(), true, 0, "STL generated by Fusionalign", false);
}

void ToothModelManager::saveToothData(Tooth *_tooth, QString _path, const vcg::Matrix44f &_trans_matrix)
{
	QString tempFileName = _path + QString::fromStdString(".xml");
	ToothToXMLFile(_tooth, tempFileName, _trans_matrix);
}

void ToothModelManager::clearFiles(const QString &folderFullPath)
{
	QDir dir(folderFullPath);
	dir.setFilter(QDir::Files);
	int fileCount = dir.count();
	for (int i = 0; i < fileCount; i++)
	{
		dir.remove(dir[i]);
	}
}

bool ToothModelManager::ToothToXMLFile(Tooth *_tooth, QString _filename, const vcg::Matrix44f &_trans_matrix)
{
	QFileInfo fi(_filename);
	QDir tmpDir = QDir::current();
	QDir::setCurrent(fi.absoluteDir().absolutePath());
	QDomDocument doc = collectToothDataDomDoc(_tooth, _trans_matrix);
	QFile file(_filename);
	file.open(QIODevice::WriteOnly);
	QTextStream qstream(&file);
	doc.save(qstream, 1);
	file.close();
	QDir::setCurrent(tmpDir.absolutePath());
	return true;
}

QDomDocument ToothModelManager::collectToothDataDomDoc(Tooth *_tooth, const vcg::Matrix44f &_trans_matrix)
{
	QDomDocument ddoc("ToothModelData");
	ddoc.appendChild(intToXML(_tooth->iLabel, QString("ToothIndex"), ddoc));
	ddoc.appendChild(intToXML(_tooth->strFDI.toInt(), QString("ToothFDI"), ddoc));
	ddoc.appendChild(boundaryIndexListToXML(_tooth->boundaryIndex, ddoc));
	ddoc.appendChild(convexHullPointListToXML(_tooth->convexVerts, ddoc, _trans_matrix));
	return ddoc;
}

QDomElement ToothModelManager::intToXML(const int num, QString node_name, QDomDocument& doc)
{
	QDomElement element = doc.createElement(node_name);

	QString str;
	str += QString("%1 \n").arg(num);
	QDomText sign = doc.createTextNode(str);
	element.appendChild(sign);
	return element;
}

QDomElement ToothModelManager::boundaryIndexListToXML(const vector<int> &_boundary, QDomDocument &doc)
{
	QDomElement element = doc.createElement(QString("BoundaryIndexes"));

	QString str;
	for (auto index : _boundary)
	{
		str += QString("%1 ").arg(index);
	}
	QDomText sign = doc.createTextNode(str);
	element.appendChild(sign);
	return element;
}

vector<int> ToothModelManager::parseBoundaryIndexList(QDomNode _node)
{
	QStringList values = _node.firstChild().nodeValue().split("\n", QString::SkipEmptyParts);
	if (values.empty())
	{
		return vector<int>();
	}
	QStringList numList = values.at(0).split(" ", QString::SkipEmptyParts);
	vector<int> result;
	for (int i = 0; i < numList.size(); ++i)
	{
		result.push_back(numList.at(i).toInt());
	}
	return result;
}

QDomElement ToothModelManager::convexHullPointListToXML(const vector<Point3m> &_points, QDomDocument &doc, const vcg::Matrix44f &_trans_matrix)
{
	QDomElement element = doc.createElement(QString("ConvecHullPoints"));

	QString str;
	for (auto p : _points)
	{
		p = _trans_matrix * p;
		str += QString("%1 %2 %3 \n").arg(p.X()).arg(p.Y()).arg(p.Z());
	}
	QDomText sign = doc.createTextNode(str);
	element.appendChild(sign);
	return element;
}

vector<Point3m> ToothModelManager::parseConvexHullPointsList(QDomNode _node)
{
	QStringList values = _node.firstChild().nodeValue().split("\n", QString::SkipEmptyParts);
	if (values.empty())
	{
		return vector<Point3m>();
	}
	vector<Point3m> result;
	for (auto value : values)
	{
		QStringList numList = value.split(" ", QString::SkipEmptyParts);
		if (numList.size() >= 3)
		{
			float x = numList.at(0).toFloat();
			float y = numList.at(1).toFloat();
			float z = numList.at(2).toFloat();
			result.push_back(Point3m(x, y, z));
		}
	}
	return result;
}
