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

#include <QString>
#include <QtGlobal>
#include <QFileInfo>
#include <QtXml>

#include "common/meshmodel.h"
#include "meshExt/dental.h"
#include "fusionProjectParser.h"
#include <wrap/qt/shot_qt.h>
#include "data/fusionaligndata.h"
#include "data/dentalanalysisdata.h"

// Convert feature point information to XML
QDomElement FeaturePointsToXML(const std::map<QString, CrownInfoSegmentedIntelligent>& _feature_points_vec, QDomDocument& doc);
// Parse feature point information from XML
void ParseFeaturePointsFromXML(const QDomNode& node, std::map<QString, CrownInfoSegmentedIntelligent>& _feature_points_vec);
// Parse individual feature point from XML
bool ParseEachFdiFeaturePointFromXML(const QDomNode& node, CrownInfoSegmentedIntelligent& _feature_point);
QDomElement Point3fToXML(const QString& name_node, const Point3f& p, QDomDocument& doc);

QDomElement VertexsPickedForLocateToXML(std::vector<std::pair<Point3f, Point3f>>& vertexIndexPicked, QDomDocument& doc);
QDomElement VertexsPickedForLocateToXML(std::vector<std::pair<int, int>>& vertexIndexPicked, QDomDocument& doc);
QDomElement ToothExistMarkToXML(std::vector<int>& toothExistMark, QDomDocument& doc);
QDomElement AxisToothInfoToXML(std::vector<Axis>& toothAxis, QDomDocument& doc);
QDomElement ToothRootIntervalListToXML(std::vector<Point2i>& root_interval_list, QDomDocument& doc);
QDomElement GingivaControlPointsIndexMarkToXML(std::vector<int>& controlsIndexMark, QDomDocument& doc);
QDomElement ToothControlPointsIndexMarkToXML(std::vector<std::vector<int>>& controlsIndexMark, QDomDocument& doc);
QDomElement BorderToothPointsIndexMarkToXML(std::vector<std::vector<int>>& borderVertexIndexMark, QDomDocument& doc);
QDomElement ToothLabelFDIToXML(std::vector<QString>& labelToothFDI, QDomDocument& doc);
QDomElement ToothLabelFDIPosToXML(std::vector<Point3f>& posLabelToothFDI, QDomDocument& doc);
QDomElement ToothLabelFDIToXML(std::vector<ToothFDIInfo>& toothFDI, QDomDocument& doc);
void ParseToothLabelFDIPosToXML(const QDomNodeList& nodeList, std::vector<ToothFDIInfo>& posLabelToothFDI);
QDomElement OcclusalPlaneToXML(CustomPlane occlusalPlane, QDomDocument& doc);
QDomElement FixtureCSYSToXML(Axis csys, QDomDocument& doc);
QDomElement AimArchBezierCtrlNodes2DToXML(vector<Point3m> nodes, QDomDocument& doc);
QDomElement NeighborTeethVecListToXML(vector<pair<int, HexaVec>> _list, QDomDocument& doc);
void ParseNeighborTeethVecList(const QDomNode& _node, vector<pair<int, HexaVec>>& _list);
void ParseNeighborToothVec(const QDomNode& _node, pair<int, HexaVec>& _element);
QDomElement NeighborToothVecToXML(pair<int, HexaVec> _vec, QDomDocument& doc);
template<typename T> QDomElement vectorToXML(vector<T> _list, QString _node_name, QDomDocument& doc);
template<typename T> void ParseVector(const QDomNode& _node, vector<T>& _list);
QDomElement ArtificialRecordListToXML(vector < pair<int, bool >> _list, QDomDocument& doc);
void ParseArtificialRecordList(const QDomNode& _node, vector < pair<int, bool >>& _list);
QDomElement ArtificialRecordToXML(pair<int, bool > _record, QDomDocument& doc);
void ParseArtificialRecord(const QDomNode& _node, pair<int, bool >& _element);

QDomElement DentalnitialRealtimeAxisToXML(Axis _axis, QDomDocument& doc);
QDomElement OrderIDToXML(QString id, QDomDocument& doc);
QDomElement BiteplaneToXML(CustomPlane bitePlane, QDomDocument& doc);
QDomElement WidthAndDepthToXML(float width, float depth, QDomDocument& doc);
QDomElement FixedToothIndexListToXML(std::vector<int> _index_list, QDomDocument& doc);
QDomElement ArtificialTeethInfoListToXML(std::vector<ArtificialToothRecord> _arti_list, QDomDocument& doc);

QDomElement ToothArrayInfoToXML(const vector<HexaVec>& array, QDomDocument& doc);
QDomElement ToothArrayInfo_RootToXML(const vector<HexaVec>& array, QDomDocument& doc);
QDomElement hexaVecToXML(const HexaVec record, QString node_name, QDomDocument& doc);
QDomElement intToXML(const int num, QString node_name, QDomDocument& doc);

QDomElement MoveDragRecordsToXML(std::vector<FEdge>* recordList, QDomDocument& doc);
QDomElement RotateDragRecordsToXML(std::vector<FEdge>* recordList, QDomDocument& doc);
QDomElement SynDragRecordCPToXML(std::vector<vcg::Point2i>* cpList, QDomDocument& doc);
QDomElement EachToothConvexVertsToXML(Dental* dental, QDomDocument& doc);
QDomElement ToothConvexVertsToXML(std::vector<Point3m> verts, QDomDocument& doc);
QDomElement EachToothFeatureMarksToXML(Dental* dental, QDomDocument& doc);
QDomElement ToothFeatureMarksToXML(std::vector<Point3m> verts, QDomDocument& doc);
QDomElement DentalAnalysisDataToXML(DentalAnalysisData *_data, QDomDocument& doc);
QDomElement PointVectorToXML(QString _title, std::vector<Point3m> _verts, QDomDocument& doc);
QDomElement EdgeVectorToXML(QString _title, std::vector<FEdge> _edges, QDomDocument& doc);
QDomElement EdgeToXML(QString _title, FEdge _edge, QDomDocument& doc);
QDomElement CutFaceToXML(QString _title, CutFace * _face, QDomDocument& doc);
QDomElement FloatToXML(QString _title, float _number, QDomDocument& doc);
QDomElement BoolToXML(QString _title, bool _value, QDomDocument& doc);
QDomElement StringToXML(QString _title, QString _context, QDomDocument& doc);
QDomElement StringVectorToXML(QString _title, vector<QString> _contexts, QDomDocument& doc);

// Parse hexa vector value from XML node
void ParseHexaVectorValue(const QDomNode& hexaVNode, HexaVec& hexaValue);

void ParseMoveRecords(QDomNode moveLinks, vector<FEdge>& moveRecords);
void ParseRotateRecords(QDomNode rotateLinks, vector<FEdge>& rotateRecords);
void ParseSynCPsRecords(QDomNode moveLinks, vector<Point2i>& synCPRecords);

void ParseEachToothConvexVerts(QDomNode eachToothIndexesNode, vector<vector<Point3m>>& eachToothVerts);
void ParseToothConvexVerts(QDomNode toothIndexesNode, vector<Point3m>& toothVerts);
void ParseEachToothFeatureMarks(QDomNode eachToothIndexesNode, vector<vector<Point3m>>& eachToothVerts);
void ParseToothFeatureMarks(QDomNode toothIndexesNode, vector<Point3m>& toothVerts);
void ParsePointVectorInfo(QDomNode _node, vector<Point3m>& _verts);
void ParseEdgeVectorInfo(QDomNode _node, vector<FEdge>& _edges);
void ParseEdgeInfo(QDomNode _node, FEdge& _edges);
void ParseCutFaceInfo(QDomNode _node, CutFace*&  _face);
void ParseFloatInfo(QDomNode _node, float& _num);
void ParseBoolInfo(QDomNode _node, bool& _value);
void ParseStringInfo(QDomNode _node, QString& _context);
void ParseStringInfoNoSpace(QDomNode _node, QString& _context);
void ParseStringVectorInfo(QDomNode _node, vector<QString>& _contexts);

bool has_recored_analyzer_data_ = false;

// Save MeshDocument to XML file
bool MeshDocumentToXMLFile(MeshDocument& md, QString filename, bool onlyVisibleLayers, bool bIsSegmentProject)
{
	md.setFileName(filename);
	QFileInfo fi(filename);
	QDir tmpDir = QDir::current();
	QDir::setCurrent(fi.absoluteDir().absolutePath());
	QDomDocument doc = MeshDocumentToXML(md, onlyVisibleLayers, bIsSegmentProject);
	QFile file(filename);
	file.open(QIODevice::WriteOnly);
	QTextStream qstream(&file);
	doc.save(qstream, 1);
	file.close();
	QDir::setCurrent(tmpDir.absolutePath());
	return true;
}

// Load MeshDocument from XML file
bool MeshDocumentFromXML(MeshDocument& md, QString filename)
{
	QFile qf(filename);
	QFileInfo qfInfo(filename);
	QDir tmpDir = QDir::current();
	QDir::setCurrent(qfInfo.absoluteDir().absolutePath());
	if (!qf.open(QIODevice::ReadOnly))
		return false;

	QString project_path = qfInfo.absoluteFilePath();

	QString errorMsg;
	QDomDocument doc("MeshLabDocument");    //It represents the XML document
	if (!doc.setContent(&qf, &errorMsg))
		return false;
	QDomElement root = doc.documentElement();

	QDomNode node = root.firstChild();

	while (!node.isNull())
	{
		if (root.nodeName() == "MeshLabProject")
		{
			break;
		}
		node = node.nextSiblingElement();
	}
	while (!node.isNull()) {
		if (QString::compare(node.nodeName(), "MeshGroup") == 0)
		{
			QDomNode mesh; QString filen, label;
			QString upperOrLowerJaw;
			mesh = node.firstChild();
			while (!mesh.isNull()) {
				filen = mesh.attributes().namedItem("filename").nodeValue();
				label = mesh.attributes().namedItem("label").nodeValue();
				upperOrLowerJaw = mesh.attributes().namedItem("UpperOrLowerJaw").nodeValue();

				if (!filen.isEmpty())
				{
					md.addNewMesh(filen, label);
				}

				SegmentedStatusInfo* segmented_data = nullptr;
				bool is_upper_tooth = true;

				if (upperOrLowerJaw.compare("UpperJaw") == 0)
				{
					md.mm()->upperOrLowerToothModelMark = UpperToothModel;
					is_upper_tooth = true;
				}
				else if (upperOrLowerJaw.compare("LowerJaw") == 0)
				{
					md.mm()->upperOrLowerToothModelMark = LowerToothModel;
					is_upper_tooth = false;
				}
				segmented_data = new SegmentedStatusInfo;

				int numTooth = mesh.attributes().namedItem("numtoothsegmented").nodeValue().toInt();

				if (numTooth >= 0)
				{
					segmented_data->clearData();
					segmented_data->numToothSegmented = numTooth;
				}

				//QDomNode tr = mesh.firstChild();
				QDomNodeList list = mesh.childNodes(); //获得元素e的所有子节点的列表
				for (int a = 0; a < list.count(); a++) //遍历该列表
				{
					QDomNode node = list.at(a);

					qDebug() << "parse node : --->" << node.nodeName() << endl;

					if (node.nodeName() == "MLMatrix44")
					{
						QDomNode tr = mesh.firstChild();
						vcg::Matrix44f trm;
						if (tr.childNodes().size() == 1)
						{
							QStringList values = tr.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
							for (int y = 0; y < 4; y++)
								for (int x = 0; x < 4; x++)
									md.mm()->cm.Tr[y][x] = values[x + 4 * y].toFloat();
						}

					}

					if (node.nodeName() == "VertexsPickedForLocate")
					{
						if (node.childNodes().size() == 1)
						{
							QStringList values = node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
							int numPair = (values.size() - 1) / 6;
							for (int i = 0; i < numPair; ++i)
							{
								int index = i * 6;
								Point3f v1(values[index].toFloat(),
									values[index + 1].toFloat(),
									values[index + 2].toFloat());
								index += 3;
								Point3f v2(values[index].toFloat(),
									values[index + 1].toFloat(),
									values[index + 2].toFloat());

								segmented_data->posVertexPickedLocated.push_back(std::make_pair(v1, v2));
							}
						}
					}

					if (node.nodeName() == "ToothExistMark")
					{
						if (node.childNodes().size() == 1)
						{
							QStringList values = node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
							int num = values.size() - 1;
							for (int i = 0; i < num; ++i)
							{
								segmented_data->bIsLossTooth.push_back(values[i].toUInt());
							}
						}
					}

					if (node.nodeName() == "ToothAxis")
					{
						if (node.childNodes().size() == 1)
						{
							QStringList values = node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
							int num = (values.size() - 1) / 12;
							for (int i = 0; i < num; ++i)
							{
								int index = 12 * i;
								Axis axis;

								axis.axisXVector = Point3f(values[index].toFloat(), values[index + 1].toFloat(), values[index + 2].toFloat());
								index += 3;
								axis.axisYVector = Point3f(values[index].toFloat(), values[index + 1].toFloat(), values[index + 2].toFloat());
								index += 3;
								axis.axisZVector = Point3f(values[index].toFloat(), values[index + 1].toFloat(), values[index + 2].toFloat());
								index += 3;
								axis.centerPoint = Point3f(values[index].toFloat(), values[index + 1].toFloat(), values[index + 2].toFloat());

								segmented_data->axisTooth.push_back(axis);
							}
						}
					}

					if (node.nodeName() == "RootIntervalList")
					{
						if (node.childNodes().size() == 1)
						{
							QStringList values = node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
							int num = (values.size() - 1) / 2;
							for (int i = 0; i < num; ++i)
							{
								int index = 2 * i;
								Point2i interval = Point2i(values[index].toInt(), values[index + 1].toInt());

								segmented_data->root_interval_list_.push_back(interval);
							}
						}
					}

					if (node.nodeName() == "ToothControlsIndex")
					{
						if (node.childNodes().size() == 1)
						{
							QStringList values = node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
							int num = values.size() - 1;
							std::vector<int> eachToothVertexsContour;
							for (int i = 0; i < num; ++i)
							{
								int value = values[i].toInt();
								if (value < 0 && !eachToothVertexsContour.empty())
								{
									segmented_data->contourVertexIndexTooth.push_back(eachToothVertexsContour);

									std::vector<int>().swap(eachToothVertexsContour);
								}
								else
								{
									eachToothVertexsContour.push_back(value);
								}
							}
						}
					}

					if (node.nodeName() == "BorderIndexEachTooth")
					{
						if (node.childNodes().size() == 1)
						{
							QStringList values = node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
							int num = values.size() - 1;
							std::vector<int> eachToothVertexsBorder;
							for (int i = 0; i < num; ++i)
							{
								int value = values[i].toInt();
								if (value < 0 && !eachToothVertexsBorder.empty())
								{
									segmented_data->borderVertexIndexEachTooth.push_back(eachToothVertexsBorder);

									std::vector<int>().swap(eachToothVertexsBorder);
								}
								else
								{
									eachToothVertexsBorder.push_back(value);
								}
							}

							if (!segmented_data->borderVertexIndexEachTooth.empty())
							{
								std::vector<std::vector<int>> toothBorderCopy = segmented_data->borderVertexIndexEachTooth;
								segmented_data->borderVertexIndexEachTooth.clear();

								int numLossToothMark = segmented_data->bIsLossTooth.size();
								int existToothNum = 0;
								int borderToothNum = toothBorderCopy.size();
								// Gingiva has no mark
								segmented_data->borderVertexIndexEachTooth.push_back(vector<int>());

								for (int i = 1; i < numLossToothMark; ++i)
								{
									if (segmented_data->bIsLossTooth[i] || existToothNum >= borderToothNum)
									{
										segmented_data->borderVertexIndexEachTooth.push_back(vector<int>());
									}
									else
									{
										segmented_data->borderVertexIndexEachTooth.push_back(toothBorderCopy[existToothNum++]);
									}
								}
							}

						}
					}

					if (node.nodeName() == "ToothLabelFDI")
					{
						QDomNodeList toothFDI = node.childNodes();
						ParseToothLabelFDIPosToXML(toothFDI, segmented_data->toothFDIInfo);
					}

					if (node.nodeName() == "FeaturePointsGroup")
					{
						ParseFeaturePointsFromXML(node, segmented_data->feature_points_mark);
					}

					if (node.nodeName() == "OcclusalPlane")
					{
						if (node.childNodes().size() == 1)
						{
							QStringList values = node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);

							Point3f center(values[0].toFloat(), values[1].toFloat(), values[2].toFloat());

							Point3f axisX(values[3].toFloat(), values[4].toFloat(), values[5].toFloat());

							Point3f axisY(values[6].toFloat(), values[7].toFloat(), values[8].toFloat());

							Point3f axisZ(values[9].toFloat(), values[10].toFloat(), values[11].toFloat());

							if (segmented_data->occlusalPlane)
							{
								delete segmented_data->occlusalPlane;
								segmented_data->occlusalPlane = nullptr;
							}

							segmented_data->occlusalPlane = new CustomPlane();
							segmented_data->occlusalPlane->center = center;
							segmented_data->occlusalPlane->axisXV = axisX;
							segmented_data->occlusalPlane->axisYV = axisY;
							segmented_data->occlusalPlane->axisZV = axisZ;
						}
					}

					if (node.nodeName() == "FixtureCSYS")
					{
						if (node.childNodes().size() == 1)
						{
							QStringList values = node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);

							if (values.empty())
							{
								delete segmented_data->fixtureCSys;
								segmented_data->fixtureCSys = nullptr;
							}
							else
							{
								Point3f center(values[0].toFloat(), values[1].toFloat(), values[2].toFloat());

								Point3f axisX(values[3].toFloat(), values[4].toFloat(), values[5].toFloat());

								Point3f axisY(values[6].toFloat(), values[7].toFloat(), values[8].toFloat());

								Point3f axisZ(values[9].toFloat(), values[10].toFloat(), values[11].toFloat());

								if (segmented_data->fixtureCSys)
								{
									delete segmented_data->fixtureCSys;
									segmented_data->fixtureCSys = nullptr;
								}

								segmented_data->fixtureCSys = new Axis(center, axisX, axisY, axisZ);
							}
						}
					}

					if (node.nodeName() == "AimArchBezierCtrlNodes2D")
					{
						if (node.childNodes().size() == 1)
						{
							QStringList values = node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
							values.pop_back();

							if (values.empty() || values.size() % 3 != 0)
							{
								vector<Point3m>().swap(segmented_data->archCtrlNodes);
							}
							else
							{
								vector<Point3m>().swap(segmented_data->archCtrlNodes);

								Point3m tempNode;
								for (int i = 0; i < values.size(); ++i)
								{
									if (i % 3 == 0)
									{
										tempNode.X() = values[i].toFloat();
									}
									if (i % 3 == 1)
									{
										tempNode.Y() = values[i].toFloat();
									}
									if (i % 3 == 2)
									{
										tempNode.Z() = values[i].toFloat();
										segmented_data->archCtrlNodes.push_back(tempNode);
									}
								}
							}

						}
					}

					if (node.nodeName() == "DentalInitialRealtimeAxis")
					{
						if (node.childNodes().size() == 1)
						{
							QStringList values = node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);

							if (values.empty())
							{
								delete segmented_data->dentalInitialCSys;
								segmented_data->dentalInitialCSys = nullptr;
							}
							else
							{
								Point3f center(values[0].toFloat(), values[1].toFloat(), values[2].toFloat());

								Point3f axisX(values[3].toFloat(), values[4].toFloat(), values[5].toFloat());

								Point3f axisY(values[6].toFloat(), values[7].toFloat(), values[8].toFloat());

								Point3f axisZ(values[9].toFloat(), values[10].toFloat(), values[11].toFloat());

								if (segmented_data->dentalInitialCSys)
								{
									delete segmented_data->dentalInitialCSys;
									segmented_data->dentalInitialCSys = nullptr;
								}

								segmented_data->dentalInitialCSys = new Axis(center, axisX, axisY, axisZ);
							}
						}
					}

					if (node.nodeName() == "OrderID")
					{
						if (node.childNodes().size() == 1)
						{
							QString values = node.firstChild().nodeValue();

							segmented_data->orderID = values;
						}
					}

					if (node.nodeName() == "Biteplane")
					{
						if (node.childNodes().size() == 1)
						{
							QStringList values = node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
							Point3f center(values[0].toFloat(),
								values[1].toFloat(),
								values[2].toFloat());

							Point3f axisX(values[3].toFloat(),
								values[4].toFloat(),
								values[5].toFloat());

							Point3f axisY(values[6].toFloat(),
								values[7].toFloat(),
								values[8].toFloat());

							Point3f axisZ(values[9].toFloat(),
								values[10].toFloat(),
								values[11].toFloat());
							if (segmented_data->occlusalPlane)
							{
								delete segmented_data->occlusalPlane;
								segmented_data->occlusalPlane = nullptr;
							}

							segmented_data->occlusalPlane = new CustomPlane();
							segmented_data->occlusalPlane->center = center;
							segmented_data->occlusalPlane->axisXV = axisX;
							segmented_data->occlusalPlane->axisYV = axisY;
							segmented_data->occlusalPlane->axisZV = axisZ;

						}
					}

					if (node.nodeName() == "WidthAndDepth")
					{
						if (node.childNodes().size() == 1)
						{
							QStringList values = node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
							float width, depth;
							width = values[0].toFloat();
							depth = values[1].toFloat();

							segmented_data->fPlaneWidth = width;
							segmented_data->fPlaneDepth = depth;
						}
					}

					if (node.nodeName() == "TeethWidthDefVertsList")
					{
						vector<Point3m> verts_list;
						ParsePointVectorInfo(node, verts_list);
						segmented_data->width_def_verts_list_.clear();
						for (int i = 0; i < verts_list.size(); i += 3)
						{
							vector<Point3m> temp_list;
							temp_list.push_back(verts_list[i]);
							temp_list.push_back(verts_list[i + 1]);
							temp_list.push_back(verts_list[i + 2]);
							segmented_data->width_def_verts_list_.push_back(temp_list);
						}
					}

					if (node.nodeName() == "DentalAnalysisData")
					{
						segmented_data->have_analyzer_measure_record_ = true;

						QDomNodeList infoList = node.childNodes();
						for (int i = 0; i < infoList.size(); ++i)
						{
							QDomNode node = infoList.at(i);

							if (node.nodeName() == "TeethWidthEdgesUpper")
							{
								ParseEdgeVectorInfo(node, segmented_data->tooth_width_edges_upper_);
							}
							if (node.nodeName() == "TeethWidthEdgesLower")
							{
								ParseEdgeVectorInfo(node, segmented_data->tooth_width_edges_lower_);
							}

							if (node.nodeName() == "CurLenArchNodesUpper")
							{
								ParsePointVectorInfo(node, segmented_data->cur_length_arch_ctrlnodes_upper_);
							}
							if (node.nodeName() == "CurLenArchNodesLower")
							{
								ParsePointVectorInfo(node, segmented_data->cur_length_arch_ctrlnodes_lower_);
							}

							if (node.nodeName() == "CurLenArchNodesUpper55")
							{
								ParsePointVectorInfo(node, segmented_data->crowding_arch_5_5_upper_);
							}
							if (node.nodeName() == "CurLenArchNodesLower55")
							{
								ParsePointVectorInfo(node, segmented_data->crowding_arch_5_5_lower_);
							}

							if (node.nodeName() == "CurLenArchNodesUpper77")
							{
								ParsePointVectorInfo(node, segmented_data->crowding_arch_7_7_upper_);
							}
							if (node.nodeName() == "CurLenArchNodesLower77")
							{
								ParsePointVectorInfo(node, segmented_data->crowding_arch_7_7_lower_);
							}

							if (node.nodeName() == "SpeeNodes")
							{
								ParsePointVectorInfo(node, segmented_data->spee_ctrlnodes_);
							}

							if (node.nodeName() == "MolarEdgesUpper")
							{
								ParseEdgeVectorInfo(node, segmented_data->molar_ctrledges_upper_);
							}
							if (node.nodeName() == "MolarEdgesLower")
							{
								ParseEdgeVectorInfo(node, segmented_data->molar_ctrledges_lower_);
							}
							if (node.nodeName() == "MidlineEdgesUpper")
							{
								ParseEdgeVectorInfo(node, segmented_data->midline_ctrledges_upper_);
							}
							if (node.nodeName() == "MidlineEdgesLower")
							{
								ParseEdgeVectorInfo(node, segmented_data->midline_ctrledges_lower_);
							}
							if (node.nodeName() == "ArchWidthEdgesUpper")
							{
								ParseEdgeVectorInfo(node, segmented_data->arch_width_ctrledges_upper_);
							}
							if (node.nodeName() == "ArchWidthEdgesLower")
							{
								ParseEdgeVectorInfo(node, segmented_data->arch_width_ctrledges_lower_);
							}
							if (node.nodeName() == "ArchLengthEdgesUpper")
							{
								ParseEdgeVectorInfo(node, segmented_data->arch_len_ctrledges_upper_);
							}
							if (node.nodeName() == "ArchLengthEdgesLower")
							{
								ParseEdgeVectorInfo(node, segmented_data->arch_len_ctrledges_lower_);
							}

							if (node.nodeName() == "BasalBoneArchEdgesUpper")
							{
								ParseEdgeVectorInfo(node, segmented_data->basal_bone_arch_ctrledges_upper_);
							}
							if (node.nodeName() == "BasalBoneArchEdgesLower")
							{
								ParseEdgeVectorInfo(node, segmented_data->basal_bone_arch_ctrledges_lower_);
							}

							if (node.nodeName() == "GnaHeightEdgesUpper")
							{
								ParseEdgeVectorInfo(node, segmented_data->gna_height_ctrledges_upper_);
							}

							if (node.nodeName() == "OverlayLeftCutFace")
							{
								ParseCutFaceInfo(node, segmented_data->left_overlay_cut_face_);
							}
							if (node.nodeName() == "OverlayRightCutFace")
							{
								ParseCutFaceInfo(node, segmented_data->right_overlay_cut_face_);
							}
							if (node.nodeName() == "OverlayLeftMeasureEdge")
							{
								ParseEdgeInfo(node, segmented_data->left_overlay_measure_edge_);
							}
							if (node.nodeName() == "OverlayRightMeasureEdge")
							{
								ParseEdgeInfo(node, segmented_data->right_overlay_measure_edge_);
							}

							if (node.nodeName() == "OverbiteValue")
							{
								ParseFloatInfo(node, segmented_data->overbite_value_);
							}
							if (node.nodeName() == "OverbiteRank")
							{
								ParseStringInfo(node, segmented_data->overbite_rank_);
							}
							if (node.nodeName() == "OverjetValue")
							{
								ParseFloatInfo(node, segmented_data->overjet_value_);
							}
							if (node.nodeName() == "OverjetRank")
							{
								ParseStringInfo(node, segmented_data->overjet_rank_);
							}

							if (node.nodeName() == "CrowdingAnalysis6To6")
							{
								bool six_to_six;
								ParseBoolInfo(node, six_to_six);
								DentalAnalysisData::setCrowdingAnalysisMode(six_to_six);
								emit PSIGNALMANAGER->setSixToSixModeCheckedStateSignal(six_to_six);
							}

							if (node.nodeName() == "ConfirmMissingFDI")
							{
								ParseBoolInfo(node, segmented_data->confirm_missing_fdi_);
							}
							if (node.nodeName() == "ConfirmToothWidth")
							{
								ParseBoolInfo(node, segmented_data->confirm_tooth_width_);
							}
							if (node.nodeName() == "ConfirmAnteriroParam")
							{
								ParseBoolInfo(node, segmented_data->confirm_anterior_parameter_);
							}
							if (node.nodeName() == "ConfirmArchLenParam")
							{
								ParseBoolInfo(node, segmented_data->confirm_arch_length_);
							}
							if (node.nodeName() == "ConfirmArchWidthParam")
							{
								ParseBoolInfo(node, segmented_data->confirm_arch_width_);
							}
							if (node.nodeName() == "ConfirmBoltonParam")
							{
								ParseBoolInfo(node, segmented_data->confirm_bolton_);
							}
							if (node.nodeName() == "ConfirmCrowdingParam")
							{
								ParseBoolInfo(node, segmented_data->confirm_crowding_);
							}
							if (node.nodeName() == "ConfirmBasalBoneArch")
							{
								ParseBoolInfo(node, segmented_data->confirm_basal_bone_arch_);
							}
							if (node.nodeName() == "ConfirmOverlay")
							{
								ParseBoolInfo(node, segmented_data->confirm_overlay_);
							}
							if (node.nodeName() == "ConfirmGnathotectumParam")
							{
								ParseBoolInfo(node, segmented_data->confirm_gnathotectum_height_);
							}
							if (node.nodeName() == "ConfirmMolarParam")
							{
								ParseBoolInfo(node, segmented_data->confirm_molar_);
							}
							if (node.nodeName() == "ConfirmSpeeParam")
							{
								ParseBoolInfo(node, segmented_data->confirm_spee_);
							}
							if (node.nodeName() == "BrowsedItemsParam")
							{
								ParseStringVectorInfo(node, segmented_data->browsed_item_prompt_);
							}
							if (node.nodeName() == "DoctorRemarks")
							{
								ParseStringInfoNoSpace(node, segmented_data->remark_);
							}
							if (node.nodeName() == "DoctorName")
							{
								ParseStringInfoNoSpace(node, segmented_data->doctor_name_);
							}
							if (node.nodeName() == "PatientName")
							{
								ParseStringInfoNoSpace(node, segmented_data->patient_name_);
							}
						}
					}

					if (node.nodeName() == "MoveLinkRecords")
					{
						vector<FEdge> moveRecords;
						ParseMoveRecords(node, moveRecords);

						if (!moveRecords.empty())
						{
							segmented_data->moveRecords.clear();
							segmented_data->moveRecords = moveRecords;
						}
					}

					if (node.nodeName() == "RotateLinkRecords")
					{
						vector<FEdge> rotateRecords;
						ParseRotateRecords(node, rotateRecords);

						if (!rotateRecords.empty())
						{
							segmented_data->rotateRecords.clear();
							segmented_data->rotateRecords = rotateRecords;
						}
					}

					if (node.nodeName() == "SynLinkRecordCPs")
					{
						vector<Point2i> curSynCPs;
						ParseSynCPsRecords(node, curSynCPs);

						if (!curSynCPs.empty())
						{
							segmented_data->synCPs.clear();
							segmented_data->synCPs = curSynCPs;
						}
					}

					if (node.nodeName() == "EachToothConvexVerts")
					{
						vector<vector<Point3m>> eachToothConvexVerts;
						ParseEachToothConvexVerts(node, eachToothConvexVerts);

						if (!eachToothConvexVerts.empty())
						{
							segmented_data->eachToothConvexVerts.clear();
							segmented_data->eachToothConvexVerts = eachToothConvexVerts;
						}
						else
						{
							segmented_data->eachToothConvexVerts.clear();
						}
					}

					if (node.nodeName() == "EachToothFeatureMarks")
					{
						vector<vector<Point3m>> eachToothMarks;
						ParseEachToothFeatureMarks(node, eachToothMarks);

						if (!eachToothMarks.empty())
						{
							segmented_data->eachToothFeatureMarks.clear();
							segmented_data->eachToothFeatureMarks = eachToothMarks;
						}
						else
						{
							segmented_data->eachToothFeatureMarks.clear();
						}
					}

					if (node.nodeName() == "FixedToothIndexList")
					{
						if (node.childNodes().size() == 1)
						{
							QStringList values = node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
							for (auto value : values)
							{
								segmented_data->fixed_tooth_list_.push_back(value.toInt());
							}
						}
					}
				}

				mesh = mesh.nextSibling();
			}
		}

		node = node.nextSibling();
	}

	QDir::setCurrent(tmpDir.absolutePath());
	qf.close();
	return true;
}

// Get mesh file names and occlusal plane from XML
bool GetMeshFileNameFromXML(QString filename, QString& _upper_file, QString& _lower_file, CustomPlane& _upper_occlusal_plane)
{
	QFile qf(filename);
	QFileInfo qfInfo(filename);
	QDir tmpDir = QDir::current();
	QDir::setCurrent(qfInfo.absoluteDir().absolutePath());
	if (!qf.open(QIODevice::ReadOnly))
	{
		return false;
	}

	QString project_path = qfInfo.absoluteFilePath();
	QString errorMsg;
	QDomDocument doc("MeshLabDocument");    //It represents the XML document
	if (!doc.setContent(&qf, &errorMsg))
	{
		return false;
	}

	QDomElement root = doc.documentElement();
	QDomNode node = root.firstChild();
	while (!node.isNull())
	{
		if (root.nodeName() == "MeshLabProject")
		{
			break;
		}
		node = node.nextSiblingElement();
	}
	while (!node.isNull()) {
		if (QString::compare(node.nodeName(), "MeshGroup") == 0)
		{
			QDomNode mesh; QString filen, label;
			QString upperOrLowerJaw;
			mesh = node.firstChild();
			while (!mesh.isNull())
			{
				filen = mesh.attributes().namedItem("filename").nodeValue();
				label = mesh.attributes().namedItem("label").nodeValue();
				upperOrLowerJaw = mesh.attributes().namedItem("UpperOrLowerJaw").nodeValue();
				if (upperOrLowerJaw.compare("UpperJaw") == 0)
				{
					_upper_file = filen;
				}
				else if (upperOrLowerJaw.compare("LowerJaw") == 0)
				{
					_lower_file = filen;
				}

				if (upperOrLowerJaw.compare("UpperJaw") == 0)
				{
					QDomNodeList list = mesh.childNodes(); //获得元素e的所有子节点的列表
					for (int a = 0; a < list.count(); a++) //遍历该列表
					{
						QDomNode node = list.at(a);

						qDebug() << "parse node : --->" << node.nodeName() << endl;

						if (node.nodeName() == "OcclusalPlane")
						{
							if (node.childNodes().size() == 1)
							{
								QStringList values = node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);

								Point3f center(values[0].toFloat(), values[1].toFloat(), values[2].toFloat());

								Point3f axisX(values[3].toFloat(), values[4].toFloat(), values[5].toFloat());

								Point3f axisY(values[6].toFloat(), values[7].toFloat(), values[8].toFloat());

								Point3f axisZ(values[9].toFloat(), values[10].toFloat(), values[11].toFloat());

								_upper_occlusal_plane.center = center;
								_upper_occlusal_plane.axisXV = axisX;
								_upper_occlusal_plane.axisYV = axisY;
								_upper_occlusal_plane.axisZV = axisZ;
							}
						}
					}
				}

				mesh = mesh.nextSibling();
			}
		}
		node = node.nextSibling();
	}

	QDir::setCurrent(tmpDir.absolutePath());
	qf.close();
	return true;
}

// Load historical comparison document from XML
bool MeshHistoricalComparsionDocumentFromXML(MeshDocument& md, QString filename)
{
	QFile qf(filename);
	QFileInfo qfInfo(filename);
	QDir tmpDir = QDir::current();
	QDir::setCurrent(qfInfo.absoluteDir().absolutePath());
	if (!qf.open(QIODevice::ReadOnly))
		return false;

	QString project_path = qfInfo.absoluteFilePath();

	QString errorMsg;
	QDomDocument doc("MeshLabDocument");    //It represents the XML document
	if (!doc.setContent(&qf, &errorMsg))
		return false;
	QDomElement root = doc.documentElement();

	QDomNode node = root.firstChild();

	while (!node.isNull())
	{
		if (root.nodeName() == "MeshLabProject")
		{
			break;
		}
		node = node.nextSiblingElement();
	}
	while (!node.isNull()) {
		if (QString::compare(node.nodeName(), "MeshGroup") == 0)
		{
			QDomNode mesh; QString filen, label;
			QString upperOrLowerJaw;
			mesh = node.firstChild();
			while (!mesh.isNull()) {
				filen = mesh.attributes().namedItem("filename").nodeValue();
				label = mesh.attributes().namedItem("label").nodeValue();
				upperOrLowerJaw = mesh.attributes().namedItem("UpperOrLowerJaw").nodeValue();

				if (!filen.isEmpty())
				{
					md.addNewMesh(filen, label);
				}

				SegmentedStatusInfo* segmented_data = nullptr;
				bool is_upper_tooth = true;

				if (upperOrLowerJaw.compare("UpperJaw") == 0)
				{
					md.mm()->upperOrLowerToothModelMark = UpperToothModel;
					is_upper_tooth = true;
				}
				else if (upperOrLowerJaw.compare("LowerJaw") == 0)
				{
					md.mm()->upperOrLowerToothModelMark = LowerToothModel;
					is_upper_tooth = false;
				}
				segmented_data = new SegmentedStatusInfo;

				int numTooth = mesh.attributes().namedItem("numtoothsegmented").nodeValue().toInt();

				if (numTooth >= 0)
				{
					segmented_data->clearData();
					segmented_data->numToothSegmented = numTooth;
				}

				mesh = mesh.nextSibling();
			}
		}

		node = node.nextSibling();
	}

	QDir::setCurrent(tmpDir.absolutePath());
	qf.close();
	return true;
}

QDomElement Matrix44mToXML(Matrix44m& m, QDomDocument& doc)
{
	QDomElement matrixElem = doc.createElement("MLMatrix44");
	QString Row[4];
	for (int i = 0; i < 4; ++i)
		Row[i] = QString("%1 %2 %3 %4 \n").arg(m[i][0]).arg(m[i][1]).arg(m[i][2]).arg(m[i][3]);

	QDomText nd = doc.createTextNode("\n" + Row[0] + Row[1] + Row[2] + Row[3]);
	matrixElem.appendChild(nd);

	return matrixElem;
}

// Parse hexa vector value from XML node
void ParseHexaVectorValue(const QDomNode& hexaVNode, HexaVec& hexaValue)
{
	float fVX, fVY, fVZ;
	float fA, fB, fG;
	float fZX, fZY, fZZ;

	if (hexaVNode.childNodes().size() == 1)
	{
		QStringList values = hexaVNode.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
		fVX = values[0].toFloat();
		fVY = values[1].toFloat();
		fVZ = values[2].toFloat();
		fA = values[3].toFloat();
		fB = values[4].toFloat();
		fG = values[5].toFloat();
		if (values.size() == 9)
		{
			fZX = values[6].toFloat();
			fZY = values[7].toFloat();
			fZZ = values[8].toFloat();
		}
		else
		{
			fZX = 1;
			fZY = 1;
			fZZ = 1;
		}
		hexaValue.Inital(fVX, fVY, fVZ, fA, fB, fG);
		hexaValue.setZoomPart(Point3m(fZX, fZY, fZZ));
	}
}

void ParseMoveRecords(QDomNode node, vector<FEdge>& moveRecords)
{
	if (node.childNodes().size() == 1)
	{
		QStringList values = node.firstChild().nodeValue().split("\n", QString::SkipEmptyParts);
		for (auto value : values)
		{
			QStringList numList = value.split(" ", QString::SkipEmptyParts);
			int x1 = numList[0].toInt();
			int y1 = numList[1].toInt();
			int x2 = numList[2].toInt();
			int y2 = numList[3].toInt();
			Point3m vertA(x1, y1, 0);
			Point3m vertB(x2, y2, 0);

			moveRecords.push_back(FEdge(vertA, vertB));
		}
	}
}

void ParseRotateRecords(QDomNode node, vector<FEdge>& rotateRecords)
{
	if (node.childNodes().size() == 1)
	{
		QStringList values = node.firstChild().nodeValue().split("\n", QString::SkipEmptyParts);
		for (auto value : values)
		{
			QStringList numList = value.split(" ", QString::SkipEmptyParts);
			int x1 = numList[0].toInt();
			int y1 = numList[1].toInt();
			int x2 = numList[2].toInt();
			int y2 = numList[3].toInt();
			Point3m vertA(x1, y1, 0);
			Point3m vertB(x2, y2, 0);

			rotateRecords.push_back(FEdge(vertA, vertB));
		}
	}
}

void ParseSynCPsRecords(QDomNode node, vector<Point2i>& synCPRecords)
{
	if (node.childNodes().size() == 1)
	{
		QStringList values = node.firstChild().nodeValue().split("\n", QString::SkipEmptyParts);
		for (auto value : values)
		{
			QStringList numList = value.split(" ", QString::SkipEmptyParts);
			int x1 = numList[0].toInt();
			int y1 = numList[1].toInt();
			Point2i cp(x1, y1);

			synCPRecords.push_back(cp);
		}
	}
}

void ParseEachToothConvexVerts(QDomNode eachToothIndexesNode, vector<vector<Point3m>>& eachToothVerts)
{
	eachToothVerts.clear();
	QDomNodeList nodeList = eachToothIndexesNode.childNodes();
	for (int i = 0; i < nodeList.size(); ++i)
	{
		if (nodeList.at(i).nodeName() == "ToothConvexVerts")
		{
			vector<Point3m> toothConvexVertsList;
			ParseToothConvexVerts(nodeList.at(i), toothConvexVertsList);
			if (!toothConvexVertsList.empty())
			{
				eachToothVerts.push_back(toothConvexVertsList);
			}
		}
	}
	return;
}

void ParseToothConvexVerts(QDomNode toothIndexesNode, vector<Point3m>& toothVerts)
{
	QStringList values = toothIndexesNode.firstChild().nodeValue().split("\n", QString::SkipEmptyParts);

	toothVerts.clear();
	int index;
	float fX, fY, fZ;
	for (auto value : values)
	{
		QStringList numList = value.split(" ", QString::SkipEmptyParts);
		if (numList.size() == 3)
		{
			fX = numList[0].toFloat();
			fY = numList[1].toFloat();
			fZ = numList[2].toFloat();
			toothVerts.push_back(Point3m(fX, fY, fZ));
		}
	}
	return;
}

void ParseEachToothFeatureMarks(QDomNode _eachToothIndexesNode, vector<vector<Point3m>>& _eachToothMarks)
{
	_eachToothMarks.clear();
	QDomNodeList nodeList = _eachToothIndexesNode.childNodes();
	for (int i = 0; i < nodeList.size(); ++i)
	{
		if (nodeList.at(i).nodeName() == "ToothFeatureMarks")
		{
			vector<Point3m> toothMarksList;
			ParseToothFeatureMarks(nodeList.at(i), toothMarksList);
			_eachToothMarks.push_back(toothMarksList);
		}
	}
	return;
}

void ParsePointVectorInfo(QDomNode _node, vector<Point3m>& _verts)
{
	QStringList values = _node.firstChild().nodeValue().split("\n", QString::SkipEmptyParts);
	if (!values.empty())
	{
		_verts.clear();
		int index;
		float fX, fY, fZ;
		for (auto value : values)
		{
			QStringList numList = value.split(" ", QString::SkipEmptyParts);
			if (numList.size() == 3)
			{
				fX = numList[0].toFloat();
				fY = numList[1].toFloat();
				fZ = numList[2].toFloat();
				_verts.push_back(Point3m(fX, fY, fZ));
			}
		}
	}
	return;
}

void ParseEdgeVectorInfo(QDomNode _node, vector<FEdge>& _edges)
{
	QStringList values = _node.firstChild().nodeValue().split("\n", QString::SkipEmptyParts);
	if (!values.empty())
	{
		_edges.clear();
		int index;
		float fX, fY, fZ;
		Point3m a, b;
		for (auto value : values)
		{
			QStringList numList = value.split(" ", QString::SkipEmptyParts);
			if (numList.size() == 6)
			{
				fX = numList[0].toFloat();
				fY = numList[1].toFloat();
				fZ = numList[2].toFloat();
				a = Point3m(fX, fY, fZ);

				fX = numList[3].toFloat();
				fY = numList[4].toFloat();
				fZ = numList[5].toFloat();
				b = Point3m(fX, fY, fZ);
				_edges.push_back(FEdge(a, b));
			}
		}
	}
	return;
}

void ParseEdgeInfo(QDomNode _node, FEdge& _edges)
{
	QStringList values = _node.firstChild().nodeValue().split("\n", QString::SkipEmptyParts);
	if (!values.empty())
	{
		float fX, fY, fZ;
		Point3m a, b;
		QStringList numList = values[0].split(" ", QString::SkipEmptyParts);
		if (numList.size() == 6)
		{
			fX = numList[0].toFloat();
			fY = numList[1].toFloat();
			fZ = numList[2].toFloat();
			a = Point3m(fX, fY, fZ);

			fX = numList[3].toFloat();
			fY = numList[4].toFloat();
			fZ = numList[5].toFloat();
			b = Point3m(fX, fY, fZ);
			_edges = FEdge(a, b);
		}
	}
	return;
}

void ParseCutFaceInfo(QDomNode _node, CutFace*& _face)
{
	if (_node.childNodes().size() == 1)
	{
		QStringList values = _node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
		Point3f center(values[0].toFloat(),
			values[1].toFloat(),
			values[2].toFloat());

		Point3f axisX(values[3].toFloat(),
			values[4].toFloat(),
			values[5].toFloat());

		Point3f axisY(values[6].toFloat(),
			values[7].toFloat(),
			values[8].toFloat());

		Point3f axisZ(values[9].toFloat(),
			values[10].toFloat(),
			values[11].toFloat());

		if (_face)
		{
			delete _face;
			_face = nullptr;
		}
		_face = new CutFace(center, axisZ, axisX, axisY);
	}
}

void ParseFloatInfo(QDomNode _node, float& _num)
{
	QStringList values = _node.firstChild().nodeValue().split("\n", QString::SkipEmptyParts);
	if (!values.empty())
	{
		QStringList numList = values[0].split(" ", QString::SkipEmptyParts);
		_num = numList[0].toFloat();
	}
	return;
}

void ParseBoolInfo(QDomNode _node, bool& _value)
{
	QStringList values = _node.firstChild().nodeValue().split("\n", QString::SkipEmptyParts);
	if (!values.empty())
	{
		QStringList numList = values[0].split(" ", QString::SkipEmptyParts);
		if (numList[0].toInt() > 0)
		{
			_value = true;
		}
		else
		{
			_value = false;
		}
	}
	return;
}

void ParseStringInfo(QDomNode _node, QString& _context)
{
	QStringList values = _node.firstChild().nodeValue().split("\n", QString::SkipEmptyParts);
	if (!values.empty())
	{
		_context = values[0];
	}
	return;
}

void ParseStringInfoNoSpace(QDomNode _node, QString& _context)
{
	QString value = _node.firstChild().nodeValue();
	_context = value;
	return;
}

void ParseStringVectorInfo(QDomNode _node, vector<QString>& _contexts)
{
	QStringList values = _node.firstChild().nodeValue().split("\n", QString::SkipEmptyParts);
	if (!values.empty())
	{
		for (auto& value : values)
		{
			_contexts.push_back(value);
		}
	}
	return;
}

void ParseToothFeatureMarks(QDomNode toothIndexesNode, vector<Point3m>& toothVerts)
{
	QStringList values = toothIndexesNode.firstChild().nodeValue().split("\n", QString::SkipEmptyParts);

	toothVerts.clear();
	int index;
	float fX, fY, fZ;
	for (auto value : values)
	{
		QStringList numList = value.split(" ", QString::SkipEmptyParts);
		if (numList.size() == 3)
		{
			fX = numList[0].toFloat();
			fY = numList[1].toFloat();
			fZ = numList[2].toFloat();
			toothVerts.push_back(Point3m(fX, fY, fZ));
		}
	}
	return;
}

QDomElement MeshModelToXML(MeshModel* mp, QDomDocument& doc, bool bIsSegmentProject)
{
	QDomElement meshElem = doc.createElement("MLMesh");

	SegmentedStatusInfo* segmented_data = nullptr;
	if (segmented_data == nullptr)
	{
		segmented_data = new SegmentedStatusInfo;
	}

	meshElem.setAttribute("label", mp->_labelSegmentMark);
	meshElem.setAttribute("filename", mp->_fullPathFileNameSegment);

	DentalManager* cur_manager = nullptr;
	DentalAnalysisData* cur_analysisdata = nullptr;
	if (mp->upperOrLowerToothModelMark == UpperToothModel || mp->upperOrLowerToothModelMark == UpperSingleDental)
	{
		meshElem.setAttribute("UpperOrLowerJaw", "UpperJaw");
		cur_manager = PFusionAlignData->upperManager();
		if (!has_recored_analyzer_data_)
		{
			cur_analysisdata = PFusionAlignData->analysisData();
			has_recored_analyzer_data_ = true;
		}
		else
		{
			cur_analysisdata = nullptr;
		}
	}
	else
	{
		meshElem.setAttribute("UpperOrLowerJaw", "LowerJaw");
		cur_manager = PFusionAlignData->lowerManager();
		if (!has_recored_analyzer_data_)
		{
			cur_analysisdata = PFusionAlignData->analysisData();
			has_recored_analyzer_data_ = true;
		}
		else
		{
			cur_analysisdata = nullptr;
		}
	}

	QString str = QString("%1").arg(segmented_data->numToothSegmented);

	meshElem.setAttribute("numtoothsegmented", str);

	meshElem.appendChild(Matrix44mToXML(mp->cm.Tr, doc));

	meshElem.appendChild(VertexsPickedForLocateToXML(
		segmented_data->posVertexPickedLocated, doc));
	meshElem.appendChild(ToothExistMarkToXML(
		segmented_data->bIsLossTooth, doc));
	meshElem.appendChild(AxisToothInfoToXML(
		segmented_data->axisTooth, doc));
	meshElem.appendChild(ToothControlPointsIndexMarkToXML(
		segmented_data->contourVertexIndexTooth, doc));
	meshElem.appendChild(BorderToothPointsIndexMarkToXML(
		segmented_data->borderVertexIndexEachTooth, doc));
	meshElem.appendChild(ToothLabelFDIToXML(
		segmented_data->toothFDIInfo, doc));

	meshElem.appendChild(FeaturePointsToXML(
		segmented_data->feature_points_mark, doc));

	meshElem.appendChild(ToothRootIntervalListToXML(
		segmented_data->root_interval_list_, doc));

	if (segmented_data->occlusalPlane)
	{
		meshElem.appendChild(OcclusalPlaneToXML(
			*segmented_data->occlusalPlane, doc));
		meshElem.appendChild(WidthAndDepthToXML(segmented_data->fPlaneWidth,
			segmented_data->fPlaneDepth, doc));
	}

	if (cur_manager != nullptr && cur_manager->bGotDental)
	{
		if (cur_manager->cDental.bBasalPlaneIsReady)
		{
			if (cur_manager->cDental.bUpperDental)
			{
				qDebug() << "SAVE Upper jaw" << endl;
			}
			else
			{
				qDebug() << "SAVE Lower jaw" << endl;
			}
			qDebug() << "SAVE --Local Axis--" << endl;
			qDebug() << cur_manager->cDental.localAxis.axisXVector.X() << ", " << cur_manager->cDental.localAxis.axisXVector.Y() << ", " << cur_manager->cDental.localAxis.axisXVector.Z() << endl;
			qDebug() << cur_manager->cDental.localAxis.axisYVector.X() << ", " << cur_manager->cDental.localAxis.axisYVector.Y() << ", " << cur_manager->cDental.localAxis.axisYVector.Z() << endl;
			qDebug() << cur_manager->cDental.localAxis.axisZVector.X() << ", " << cur_manager->cDental.localAxis.axisZVector.Y() << ", " << cur_manager->cDental.localAxis.axisZVector.Z() << endl;
			qDebug() << "SAVE --Realtime Axis--" << endl;
			qDebug() << cur_manager->cDental.realTimeAxis.axisXVector.X() << ", " << cur_manager->cDental.realTimeAxis.axisXVector.Y() << ", " << cur_manager->cDental.realTimeAxis.axisXVector.Z() << endl;
			qDebug() << cur_manager->cDental.realTimeAxis.axisYVector.X() << ", " << cur_manager->cDental.realTimeAxis.axisYVector.Y() << ", " << cur_manager->cDental.realTimeAxis.axisYVector.Z() << endl;
			qDebug() << cur_manager->cDental.realTimeAxis.axisZVector.X() << ", " << cur_manager->cDental.realTimeAxis.axisZVector.Y() << ", " << cur_manager->cDental.realTimeAxis.axisZVector.Z() << endl;

			meshElem.appendChild(DentalnitialRealtimeAxisToXML(
				cur_manager->cDental.realTimeAxis, doc));
			meshElem.appendChild(OrderIDToXML(
				cur_manager->cDental.strOrderID, doc));
			meshElem.appendChild(OcclusalPlaneToXML(
				cur_manager->cDental.basePlane, doc));
			meshElem.appendChild(WidthAndDepthToXML(
				cur_manager->cDental.fWidth, cur_manager->cDental.fDepth, doc));
			meshElem.appendChild(EachToothConvexVertsToXML
			(&cur_manager->cDental, doc));
			meshElem.appendChild(FixtureCSYSToXML(
				cur_manager->cDental.fixtureCSYS, doc));
			meshElem.appendChild(AimArchBezierCtrlNodes2DToXML(
				cur_manager->cDental.archBezierCtrlNodes2D, doc));
			meshElem.appendChild(EachToothFeatureMarksToXML
			(&cur_manager->cDental, doc));
		}
		if (cur_manager->cDental.bCureModelGenerated)
		{
			meshElem.appendChild(FixedToothIndexListToXML(
				cur_manager->fixed_tooth_list_, doc));

			meshElem.appendChild(MoveDragRecordsToXML(
				&cur_manager->cDental.moveLinkRecords, doc));
			meshElem.appendChild(RotateDragRecordsToXML(
				&cur_manager->cDental.rotateLinkRecords, doc));
			meshElem.appendChild(SynDragRecordCPToXML(
				&cur_manager->cDental.synLinkCPRecords, doc));
		}
		if (!cur_manager->cDental.teeth_width_def_verts_list_.empty())
		{
			vector<Point3m> verts_list;
			for (auto& temp_list : cur_manager->cDental.teeth_width_def_verts_list_)
			{
				verts_list.insert(verts_list.end(), temp_list.begin(), temp_list.end());
			}
			meshElem.appendChild(PointVectorToXML(QString("TeethWidthDefVertsList"), verts_list, doc));
		}
	}
	if (cur_analysisdata != nullptr)
	{
		meshElem.appendChild(DentalAnalysisDataToXML(
			cur_analysisdata, doc));
	}
	return meshElem;
}

QDomElement FeaturePointsToXML(const std::map<QString, CrownInfoSegmentedIntelligent>& _feature_points_vec, QDomDocument& doc)
{
	QDomElement feature_group = doc.createElement("FeaturePointsGroup");

	for (auto feature_point : _feature_points_vec)
	{
		QDomElement fdi_element = doc.createElement("FDI");
		fdi_element.setAttribute("label", feature_point.first);

		for (auto& it : feature_point.second.landmarks)
		{
			auto element = Point3fToXML(it.first, it.second, doc);
			fdi_element.appendChild(element);
		}

		feature_group.appendChild(fdi_element);
	}

	return feature_group;
}

QDomElement Point3fToXML(const QString& name_node, const Point3f& p, QDomDocument& doc)
{
	QDomElement node = doc.createElement(name_node);
	node.appendChild(doc.createTextNode(QString("%1 %2 %3").arg(p.X()).arg(p.Y()).arg(p.Z())));
	return node;
}

void ParseFeaturePointsFromXML(const QDomNode& node, std::map<QString, CrownInfoSegmentedIntelligent>& _feature_points_vec)
{
	_feature_points_vec.clear();

	QDomNode feature_point_info_node = node.firstChild();

	while (!feature_point_info_node.isNull())
	{

		CrownInfoSegmentedIntelligent feature_point;

		QString fdi = feature_point_info_node.attributes().namedItem("label").nodeValue();

		if (ParseEachFdiFeaturePointFromXML(feature_point_info_node, feature_point))
		{
			_feature_points_vec[fdi] = feature_point;

		}
		feature_point_info_node = feature_point_info_node.nextSibling();
	}
}

bool ParseEachFdiFeaturePointFromXML(const QDomNode& node, CrownInfoSegmentedIntelligent& _feature_point)
{

	QDomNode firstNode = node.firstChild();

	Point3f p;

	_feature_point.landmarks.clear();

	while (!firstNode.isNull())
	{
		QString nodeName = firstNode.nodeName();

		QStringList text = firstNode.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);

		p.X() = text[0].toFloat();

		p.Y() = text[1].toFloat();

		p.Z() = text[2].toFloat();

		_feature_point.landmarks[nodeName] = p;

		firstNode = firstNode.nextSibling();
	}

	return true;
}

QDomElement VertexsPickedForLocateToXML(std::vector<std::pair<Point3f, Point3f>>& vertexIndexPicked, QDomDocument& doc)
{
	QDomElement matrixElem = doc.createElement("VertexsPickedForLocate");
	QString str;
	for (auto v : vertexIndexPicked)
	{
		str += QString("%1 %2 %3 \n").arg(v.first.X()).arg(v.first.Y()).arg(v.first.Z());
		str += QString("%1 %2 %3 \n").arg(v.second.X()).arg(v.second.Y()).arg(v.second.Z());
	}
	QDomText nd = doc.createTextNode("\n" + str);
	matrixElem.appendChild(nd);
	return matrixElem;
}

QDomElement ToothExistMarkToXML(std::vector<int>& toothExistMark, QDomDocument& doc)
{
	QDomElement matrixElem = doc.createElement("ToothExistMark");
	QString str;
	for (auto v : toothExistMark)
	{
		str += QString("%1 ").arg(v);
	}
	QDomText nd = doc.createTextNode("\n" + str + " \n");
	matrixElem.appendChild(nd);
	return matrixElem;
}

QDomElement AxisToothInfoToXML(std::vector<Axis>& toothAxis, QDomDocument& doc)
{
	QDomElement matrixElem = doc.createElement("ToothAxis");
	QString str;
	for (auto axis : toothAxis)
	{
		str += QString("%1 %2 %3 \n").arg(axis.axisXVector.X()).arg(axis.axisXVector.Y()).arg(axis.axisXVector.Z());
		str += QString("%1 %2 %3 \n").arg(axis.axisYVector.X()).arg(axis.axisYVector.Y()).arg(axis.axisYVector.Z());
		str += QString("%1 %2 %3 \n").arg(axis.axisZVector.X()).arg(axis.axisZVector.Y()).arg(axis.axisZVector.Z());
		str += QString("%1 %2 %3 \n").arg(axis.centerPoint.X()).arg(axis.centerPoint.Y()).arg(axis.centerPoint.Z());
	}
	QDomText nd = doc.createTextNode("\n" + str);
	matrixElem.appendChild(nd);
	return matrixElem;
}

QDomElement ToothRootIntervalListToXML(std::vector<Point2i>& root_interval_list, QDomDocument& doc)
{
	QDomElement matrixElem = doc.createElement("RootIntervalList");
	QString str;
	for (auto info : root_interval_list)
	{
		str += QString("%1 %2 \n").arg(info.X()).arg(info.Y());
	}
	QDomText nd = doc.createTextNode("\n" + str);
	matrixElem.appendChild(nd);
	return matrixElem;
}

QDomElement GingivaControlPointsIndexMarkToXML(std::vector<int>& controlsIndexMark, QDomDocument& doc)
{
	QDomElement matrixElem = doc.createElement("GingivaControlsIndex");
	QString str;
	int numElementEachRow = 10;
	bool endLastLoop = false;
	int iVertex = 0;
	for (auto v : controlsIndexMark)
	{
		str += QString("%1 ").arg(v);
		endLastLoop = false;
		if (++iVertex % numElementEachRow == 0)
		{
			str += "\n";
			endLastLoop = true;
		}
	}
	QDomText nd;
	if (endLastLoop)
	{
		nd = doc.createTextNode("\n" + str);
	}
	else
	{
		nd = doc.createTextNode("\n" + str + "\n");
	}
	matrixElem.appendChild(nd);
	return matrixElem;
}

QDomElement ToothControlPointsIndexMarkToXML(std::vector<std::vector<int>>& controlsIndexMark, QDomDocument& doc)
{
	std::vector<int> allControlPoints;
	for (auto& loopContour : controlsIndexMark)
	{
		allControlPoints.insert(allControlPoints.end(), loopContour.begin(), loopContour.end());
		allControlPoints.push_back(-1);
	}
	QDomElement matrixElem = doc.createElement("ToothControlsIndex");
	QString str;
	int numElementEachRow = 10;
	bool endLastLoop = false;
	int iVertex = 0;
	for (auto v : allControlPoints)
	{
		str += QString("%1 ").arg(v);
		endLastLoop = false;
		if (++iVertex % numElementEachRow == 0)
		{
			str += "\n";
			endLastLoop = true;
		}
	}
	QDomText nd;
	if (endLastLoop)
	{
		nd = doc.createTextNode("\n" + str);
	}
	else
	{
		nd = doc.createTextNode("\n" + str + "\n");
	}
	matrixElem.appendChild(nd);
	return matrixElem;
}

QDomElement BorderToothPointsIndexMarkToXML(std::vector<std::vector<int>>& borderVertexIndexMark, QDomDocument& doc)
{
	std::vector<int> allBorderVertexIndex;
	for (auto loopContour : borderVertexIndexMark)
	{
		if (loopContour.empty())
		{
			continue;
		}
		allBorderVertexIndex.insert(allBorderVertexIndex.end(), loopContour.begin(), loopContour.end());
		allBorderVertexIndex.push_back(-1);
	}
	QDomElement matrixElem = doc.createElement("BorderIndexEachTooth");
	QString str;
	int numElementEachRow = 10;
	bool endLastLoop = false;
	int iVertex = 0;
	for (auto v : allBorderVertexIndex)
	{
		str += QString("%1 ").arg(v);
		endLastLoop = false;
		if (++iVertex % numElementEachRow == 0)
		{
			str += "\n";
			endLastLoop = true;
		}
	}
	QDomText nd;
	if (endLastLoop)
	{
		nd = doc.createTextNode("\n" + str);
	}
	else
	{
		nd = doc.createTextNode("\n" + str + "\n");
	}
	matrixElem.appendChild(nd);
	return matrixElem;
}

QDomElement ToothLabelFDIToXML(std::vector<ToothFDIInfo>& toothFDI, QDomDocument& doc)
{
	QDomElement toothLabelFDIElem = doc.createElement("ToothLabelFDI");

	QDomElement labelToothFDI = doc.createElement("LabelFDI");
	{
		QString str;
		for (auto fdi : toothFDI)
		{
			str += QString("%1 ").arg(fdi.labelFDI);
		}
		QDomText nd = doc.createTextNode("\n" + str + "\n");
		labelToothFDI.appendChild(nd);
	}

	toothLabelFDIElem.appendChild(labelToothFDI);

	return toothLabelFDIElem;
}

QDomElement ToothLabelFDIToXML(std::vector<QString>& labelToothFDI, QDomDocument& doc)
{
	QDomElement toothLabelFDIElem = doc.createElement("ToothLabelFDI");
	QString str;
	for (auto labelFDI : labelToothFDI)
	{
		str += QString("%1 ").arg(labelFDI);
	}
	QDomText nd = doc.createTextNode("\n" + str + " \n");
	toothLabelFDIElem.appendChild(nd);
	return toothLabelFDIElem;
}

QDomElement ToothLabelFDIPosToXML(std::vector<Point3f>& posLabelToothFDI, QDomDocument& doc)
{
	QDomElement posLabelFDIElem = doc.createElement("PosToothLabelFDI");

	QString str;
	for (auto v : posLabelToothFDI)
	{
		str += QString("%1 %2 %3 \n").arg(v.X()).arg(v.Y()).arg(v.Z());
	}
	QDomText nd = doc.createTextNode("\n" + str);
	posLabelFDIElem.appendChild(nd);
	return posLabelFDIElem;
}

void ParseToothLabelFDIPosToXML(const QDomNodeList& toothFDIInfo, std::vector<ToothFDIInfo>& toothFDI)
{
	vector<QString> labelFDI;

	for (int i = 0; i < toothFDIInfo.size(); ++i)
	{
		QDomNode node = toothFDIInfo.at(i);

		if (node.nodeName() == "LabelFDI")
		{
			labelFDI.clear();
			if (node.childNodes().size() == 1)
			{
				QStringList values = node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
				int num = values.size() - 1;
				for (int i = 0; i < num; ++i)
				{
					labelFDI.push_back(values[i].trimmed());
				}
			}
		}
	}

	vector<ToothFDIInfo>().swap(toothFDI);
	for (int i = 0; i < labelFDI.size(); ++i)
	{
		toothFDI.push_back(ToothFDIInfo(labelFDI[i]));
	}
}

QDomElement OcclusalPlaneToXML(CustomPlane occlusalPlane, QDomDocument& doc)
{
	QDomElement biteplaneElement = doc.createElement("OcclusalPlane");
	QString str;
	str += QString("%1 %2 %3 \n").arg(occlusalPlane.center.X()).arg(occlusalPlane.center.Y()).arg(occlusalPlane.center.Z());
	str += QString("%1 %2 %3 \n").arg(occlusalPlane.axisXV.X()).arg(occlusalPlane.axisXV.Y()).arg(occlusalPlane.axisXV.Z());
	str += QString("%1 %2 %3 \n").arg(occlusalPlane.axisYV.X()).arg(occlusalPlane.axisYV.Y()).arg(occlusalPlane.axisYV.Z());
	str += QString("%1 %2 %3 \n").arg(occlusalPlane.axisZV.X()).arg(occlusalPlane.axisZV.Y()).arg(occlusalPlane.axisZV.Z());
	QDomText nd = doc.createTextNode("\n" + str);
	biteplaneElement.appendChild(nd);
	return biteplaneElement;
}

QDomElement FixtureCSYSToXML(Axis csys, QDomDocument& doc)
{
	QDomElement fixtureCSYSElement = doc.createElement("FixtureCSYS");
	QString str;
	str += QString("%1 %2 %3 \n").arg(csys.centerPoint.X()).arg(csys.centerPoint.Y()).arg(csys.centerPoint.Z());
	str += QString("%1 %2 %3 \n").arg(csys.axisXVector.X()).arg(csys.axisXVector.Y()).arg(csys.axisXVector.Z());
	str += QString("%1 %2 %3 \n").arg(csys.axisYVector.X()).arg(csys.axisYVector.Y()).arg(csys.axisYVector.Z());
	str += QString("%1 %2 %3 \n").arg(csys.axisZVector.X()).arg(csys.axisZVector.Y()).arg(csys.axisZVector.Z());
	QDomText nd = doc.createTextNode("\n" + str);
	fixtureCSYSElement.appendChild(nd);
	return fixtureCSYSElement;
}

QDomElement AimArchBezierCtrlNodes2DToXML(vector<Point3m> nodes, QDomDocument& doc)
{
	QDomElement ctrlNodesElement = doc.createElement("AimArchBezierCtrlNodes2D");
	QString str;
	for (auto node : nodes)
	{
		str += QString("%1 %2 %3 \n").arg(node.X()).arg(node.Y()).arg(node.Z());
	}
	QDomText nd = doc.createTextNode("\n" + str);
	ctrlNodesElement.appendChild(nd);
	return ctrlNodesElement;
}

QDomElement DentalnitialRealtimeAxisToXML(Axis csys, QDomDocument& doc)
{
	QDomElement initialAxisElement = doc.createElement("DentalInitialRealtimeAxis");
	QString str;
	str += QString("%1 %2 %3 \n").arg(csys.centerPoint.X()).arg(csys.centerPoint.Y()).arg(csys.centerPoint.Z());
	str += QString("%1 %2 %3 \n").arg(csys.axisXVector.X()).arg(csys.axisXVector.Y()).arg(csys.axisXVector.Z());
	str += QString("%1 %2 %3 \n").arg(csys.axisYVector.X()).arg(csys.axisYVector.Y()).arg(csys.axisYVector.Z());
	str += QString("%1 %2 %3 \n").arg(csys.axisZVector.X()).arg(csys.axisZVector.Y()).arg(csys.axisZVector.Z());
	QDomText nd = doc.createTextNode("\n" + str);
	initialAxisElement.appendChild(nd);
	return initialAxisElement;
}

QDomElement OrderIDToXML(QString id, QDomDocument& doc)
{
	QDomElement orderIDElement = doc.createElement("OrderID");
	QDomText nd = doc.createTextNode(id);
	orderIDElement.appendChild(nd);
	return orderIDElement;
}

QDomElement BiteplaneToXML(CustomPlane bitePlane, QDomDocument& doc)
{
	QDomElement biteplaneElement = doc.createElement("Biteplane");
	QString str;
	str += QString("%1 %2 %3 \n").arg(bitePlane.center.X()).arg(bitePlane.center.Y()).arg(bitePlane.center.Z());
	str += QString("%1 %2 %3 \n").arg(bitePlane.axisXV.X()).arg(bitePlane.axisXV.Y()).arg(bitePlane.axisXV.Z());
	str += QString("%1 %2 %3 \n").arg(bitePlane.axisYV.X()).arg(bitePlane.axisYV.Y()).arg(bitePlane.axisYV.Z());
	str += QString("%1 %2 %3 \n").arg(bitePlane.axisZV.X()).arg(bitePlane.axisZV.Y()).arg(bitePlane.axisZV.Z());
	QDomText nd = doc.createTextNode("\n" + str);
	biteplaneElement.appendChild(nd);
	return biteplaneElement;
}

QDomElement WidthAndDepthToXML(float width, float depth, QDomDocument& doc)
{
	QDomElement widthAndDepthElement = doc.createElement("WidthAndDepth");
	QString str;
	str += QString("%1 %2 \n").arg(width).arg(depth);
	QDomText nd = doc.createTextNode("\n" + str);
	widthAndDepthElement.appendChild(nd);
	return widthAndDepthElement;
}

QDomElement ArtificialTeethInfoListToXML(std::vector<ArtificialToothRecord> _arti_list, QDomDocument& doc)
{
	QDomElement pullOutListElement = doc.createElement("ArtificialToothList");
	QString str;
	for (auto element : _arti_list)
	{
		if (0 < element.index_ && element.index_ < STANDARD_TOOTH_SUM)
		{
			str += QString("%1 %2 \n").arg(element.index_).arg((int)element.b_visible_);
		}
	}
	QDomText nd = doc.createTextNode("\n" + str);
	pullOutListElement.appendChild(nd);
	return pullOutListElement;
}

QDomElement PulloutPlanToXML(std::vector<int> pulloutList, QDomDocument& doc)
{
	QDomElement pullOutListElement = doc.createElement("PullOutList");
	QString str;
	for (auto index : pulloutList)
	{
		if (0 < index && index < STANDARD_TOOTH_SUM)
		{
			str += QString("%1 ").arg(index);
		}
	}

	QDomText nd = doc.createTextNode(str);
	pullOutListElement.appendChild(nd);
	return pullOutListElement;
}

QDomElement FixedToothIndexListToXML(std::vector<int> _index_list, QDomDocument& doc)
{
	QDomElement fixedToothListElement = doc.createElement("FixedToothIndexList");
	QString str;
	for (auto index : _index_list)
	{
		if (0 < index && index < STANDARD_TOOTH_SUM)
		{
			str += QString("%1 ").arg(index);
		}
	}
	str += QString('\n');

	QDomText nd = doc.createTextNode("\n" + str);
	fixedToothListElement.appendChild(nd);
	return fixedToothListElement;
}

QDomElement DentalAnalysisDataToXML(DentalAnalysisData* _data, QDomDocument& doc)
{
	QDomElement dataList = doc.createElement("DentalAnalysisData");

	QDomElement teethWidthEdgesUpperStage = EdgeVectorToXML(QString("TeethWidthEdgesUpper"), _data->tooth_width_edges_upper_, doc);
	dataList.appendChild(teethWidthEdgesUpperStage);
	QDomElement teethWidthEdgesLowerStage = EdgeVectorToXML(QString("TeethWidthEdgesLower"), _data->tooth_width_edges_lower_, doc);
	dataList.appendChild(teethWidthEdgesLowerStage);

	QDomElement curLenArchNodesUpperStage = PointVectorToXML(QString("CurLenArchNodesUpper"), _data->cur_length_arch_ctrlnodes_upper_, doc);
	dataList.appendChild(curLenArchNodesUpperStage);
	QDomElement curLenArchNodesUpper55Stage = PointVectorToXML(QString("CurLenArchNodesUpper55"), _data->crowding_arch_5_5_upper_, doc);
	dataList.appendChild(curLenArchNodesUpper55Stage);
	QDomElement curLenArchNodesUpper77Stage = PointVectorToXML(QString("CurLenArchNodesUpper77"), _data->crowding_arch_7_7_upper_, doc);
	dataList.appendChild(curLenArchNodesUpper77Stage);
	QDomElement curLenArchNodesLowerStage = PointVectorToXML(QString("CurLenArchNodesLower"), _data->cur_length_arch_ctrlnodes_lower_, doc);
	dataList.appendChild(curLenArchNodesLowerStage);
	QDomElement curLenArchNodesLower55Stage = PointVectorToXML(QString("CurLenArchNodesLower55"), _data->crowding_arch_5_5_lower_, doc);
	dataList.appendChild(curLenArchNodesLower55Stage);
	QDomElement curLenArchNodesLower77Stage = PointVectorToXML(QString("CurLenArchNodesLower77"), _data->crowding_arch_7_7_lower_, doc);
	dataList.appendChild(curLenArchNodesLower77Stage);
	QDomElement speeNodesStage = PointVectorToXML(QString("SpeeNodes"), _data->spee_ctrlnodes_, doc);
	dataList.appendChild(speeNodesStage);

	QDomElement molarEdgesUpperStage = EdgeVectorToXML(QString("MolarEdgesUpper"), _data->molar_ctrledges_upper_, doc);
	dataList.appendChild(molarEdgesUpperStage);
	QDomElement molarEdgesLowerStage = EdgeVectorToXML(QString("MolarEdgesLower"), _data->molar_ctrledges_lower_, doc);
	dataList.appendChild(molarEdgesLowerStage);

	QDomElement midlineEdgesUpperStage = EdgeVectorToXML(QString("MidlineEdgesUpper"), _data->midline_ctrledges_upper_, doc);
	dataList.appendChild(midlineEdgesUpperStage);
	QDomElement midlineEdgesLowerStage = EdgeVectorToXML(QString("MidlineEdgesLower"), _data->midline_ctrledges_lower_, doc);
	dataList.appendChild(midlineEdgesLowerStage);

	QDomElement archWidthEdgesUpperStage = EdgeVectorToXML(QString("ArchWidthEdgesUpper"), _data->arch_width_ctrledges_upper_, doc);
	dataList.appendChild(archWidthEdgesUpperStage);
	QDomElement archWidthEdgesLowerStage = EdgeVectorToXML(QString("ArchWidthEdgesLower"), _data->arch_width_ctrledges_lower_, doc);
	dataList.appendChild(archWidthEdgesLowerStage);

	QDomElement archLengthEdgesUpperStage = EdgeVectorToXML(QString("ArchLengthEdgesUpper"), _data->arch_len_ctrledges_upper_, doc);
	dataList.appendChild(archLengthEdgesUpperStage);
	QDomElement archLengthEdgesLowerStage = EdgeVectorToXML(QString("ArchLengthEdgesLower"), _data->arch_len_ctrledges_lower_, doc);
	dataList.appendChild(archLengthEdgesLowerStage);

	QDomElement gnaHeightEdgesUpperStage = EdgeVectorToXML(QString("GnaHeightEdgesUpper"), _data->gna_height_ctrledges_upper_, doc);
	dataList.appendChild(gnaHeightEdgesUpperStage);

	if (_data->left_overlay_cut_face_ != nullptr)
	{
		QDomElement LeftOverlayCutFaceStage = CutFaceToXML(QString("OverlayLeftCutFace"), _data->left_overlay_cut_face_, doc);
		dataList.appendChild(LeftOverlayCutFaceStage);
		QDomElement LeftOverlayMeasureEdgeStage = EdgeToXML(QString("OverlayLeftMeasureEdge"), _data->left_overlay_measure_edge_, doc);
		dataList.appendChild(LeftOverlayMeasureEdgeStage);
	}
	if (_data->right_overlay_cut_face_ != nullptr)
	{
		QDomElement RightOverlayCutFaceStage = CutFaceToXML(QString("OverlayRightCutFace"), _data->right_overlay_cut_face_, doc);
		dataList.appendChild(RightOverlayCutFaceStage);
		QDomElement RightOverlayMeasureEdgeStage = EdgeToXML(QString("OverlayRightMeasureEdge"), _data->right_overlay_measure_edge_, doc);
		dataList.appendChild(RightOverlayMeasureEdgeStage);
	}

	QDomElement basalBoneArchEdgesUpperStage = EdgeVectorToXML(QString("BasalBoneArchEdgesUpper"), _data->basal_bone_arch_ctrledges_upper_, doc);
	dataList.appendChild(basalBoneArchEdgesUpperStage);
	QDomElement basalBoneArchEdgesLowerStage = EdgeVectorToXML(QString("BasalBoneArchEdgesLower"), _data->basal_bone_arch_ctrledges_lower_, doc);
	dataList.appendChild(basalBoneArchEdgesLowerStage);

	QDomElement OverbiteValueStage = FloatToXML(QString("OverbiteValue"), _data->overbite_value_, doc);
	dataList.appendChild(OverbiteValueStage);
	QDomElement OverbiteRankStage = StringToXML(QString("OverbiteRank"), _data->overbite_rank_, doc);
	dataList.appendChild(OverbiteRankStage);
	QDomElement OverjetValueStage = FloatToXML(QString("OverjetValue"), _data->overjet_value_, doc);
	dataList.appendChild(OverjetValueStage);
	QDomElement OverjetRankStage = StringToXML(QString("OverjetRank"), _data->overjet_rank_, doc);
	dataList.appendChild(OverjetRankStage);

	QDomElement CrowdingAnalysis6To6Stage = BoolToXML(QString("CrowdingAnalysis6To6"), DentalAnalysisData::crowding_analysis_6to6_mode_, doc);
	dataList.appendChild(CrowdingAnalysis6To6Stage);

	QDomElement ConfirmMissingFdiStage = BoolToXML(QString("ConfirmMissingFDI"), _data->confirm_missing_fdi_, doc);
	dataList.appendChild(ConfirmMissingFdiStage);
	QDomElement ConfirmToothWidthStage = BoolToXML(QString("ConfirmToothWidth"), _data->confirm_tooth_width_, doc);
	dataList.appendChild(ConfirmToothWidthStage);
	QDomElement ConfirmAnteriorParamStage = BoolToXML(QString("ConfirmAnteriroParam"), _data->confirm_anterior_parameter_, doc);
	dataList.appendChild(ConfirmAnteriorParamStage);
	QDomElement ConfirmArchLenStage = BoolToXML(QString("ConfirmArchLenParam"), _data->confirm_arch_length_, doc);
	dataList.appendChild(ConfirmArchLenStage);
	QDomElement ConfirmArchWidthStage = BoolToXML(QString("ConfirmArchWidthParam"), _data->confirm_arch_width_, doc);
	dataList.appendChild(ConfirmArchWidthStage);
	QDomElement ConfirmBoltonStage = BoolToXML(QString("ConfirmBoltonParam"), _data->confirm_bolton_, doc);
	dataList.appendChild(ConfirmBoltonStage);
	QDomElement ConfirmCrowdingStage = BoolToXML(QString("ConfirmCrowdingParam"), _data->confirm_crowding_, doc);
	dataList.appendChild(ConfirmCrowdingStage);
	QDomElement ConfirmGnathotectumStage = BoolToXML(QString("ConfirmGnathotectumParam"), _data->confirm_gnathotectum_height_, doc);
	dataList.appendChild(ConfirmGnathotectumStage);
	QDomElement ConfirmMolarStage = BoolToXML(QString("ConfirmMolarParam"), _data->confirm_molar_, doc);
	dataList.appendChild(ConfirmMolarStage);
	QDomElement ConfirmSpeeStage = BoolToXML(QString("ConfirmSpeeParam"), _data->confirm_spee_, doc);
	dataList.appendChild(ConfirmSpeeStage);
	QDomElement ConfirmBasalBoneArchStage = BoolToXML(QString("ConfirmBasalBoneArch"), _data->confirm_basal_bone_arch_, doc);
	dataList.appendChild(ConfirmBasalBoneArchStage);
	QDomElement ConfirmOverlayStage = BoolToXML(QString("ConfirmOverlay"), _data->confirm_overlay_, doc);
	dataList.appendChild(ConfirmOverlayStage);
	QDomElement BrowsedItemsStage = StringVectorToXML(QString("BrowsedItemsParam"), _data->browsed_item_prompt_, doc);
	dataList.appendChild(BrowsedItemsStage);

	QDomElement DoctorRemarksStage = StringToXML(QString("DoctorRemarks"), _data->remark_, doc);
	dataList.appendChild(DoctorRemarksStage);

	QDomElement DoctorNameStage = StringToXML(QString("DoctorName"), _data->doctor_name_, doc);
	dataList.appendChild(DoctorNameStage);
	QDomElement PatientNameStage = StringToXML(QString("PatientName"), _data->patient_name_, doc);
	dataList.appendChild(PatientNameStage);

	return dataList;
}

QDomElement PointVectorToXML(QString _title, std::vector<Point3m> _verts, QDomDocument& doc)
{
	QDomElement element = doc.createElement(_title);
	QString str;
	for (auto p : _verts)
	{
		str += QString("%1 %2 %3\n").arg(p.X()).arg(p.Y()).arg(p.Z());
	}

	QDomText nd = doc.createTextNode("\n" + str);
	element.appendChild(nd);
	return element;
}

QDomElement EdgeVectorToXML(QString _title, std::vector<FEdge> _edges, QDomDocument& doc)
{
	QDomElement element = doc.createElement(_title);
	QString str;
	for (auto edge : _edges)
	{
		str += QString("%1 %2 %3 %4 %5 %6\n").arg(edge.vertA.X()).arg(edge.vertA.Y()).arg(edge.vertA.Z()).arg(edge.vertB.X()).arg(edge.vertB.Y()).arg(edge.vertB.Z());
	}

	QDomText nd = doc.createTextNode("\n" + str);
	element.appendChild(nd);
	return element;
}

QDomElement EdgeToXML(QString _title, FEdge _edge, QDomDocument& doc)
{
	QDomElement element = doc.createElement(_title);
	QString str;
	str += QString("%1 %2 %3 %4 %5 %6\n").arg(_edge.vertA.X()).arg(_edge.vertA.Y()).arg(_edge.vertA.Z()).arg(_edge.vertB.X()).arg(_edge.vertB.Y()).arg(_edge.vertB.Z());

	QDomText nd = doc.createTextNode("\n" + str);
	element.appendChild(nd);
	return element;
}

QDomElement CutFaceToXML(QString _title, CutFace* _face, QDomDocument& doc)
{
	QDomElement element = doc.createElement(_title);
	QString str;
	str += QString("%1 %2 %3 \n").arg(_face->axis_.centerPoint.X()).arg(_face->axis_.centerPoint.Y()).arg(_face->axis_.centerPoint.Z());
	str += QString("%1 %2 %3 \n").arg(_face->axis_.axisXVector.X()).arg(_face->axis_.axisXVector.Y()).arg(_face->axis_.axisXVector.Z());
	str += QString("%1 %2 %3 \n").arg(_face->axis_.axisYVector.X()).arg(_face->axis_.axisYVector.Y()).arg(_face->axis_.axisYVector.Z());
	str += QString("%1 %2 %3 \n").arg(_face->axis_.axisZVector.X()).arg(_face->axis_.axisZVector.Y()).arg(_face->axis_.axisZVector.Z());
	QDomText nd = doc.createTextNode("\n" + str);
	element.appendChild(nd);
	return element;
}

QDomElement FloatToXML(QString _title, float _number, QDomDocument& doc)
{
	QDomElement element = doc.createElement(_title);
	QString str;
	str += QString("%1\n").arg(_number);

	QDomText nd = doc.createTextNode("\n" + str);
	element.appendChild(nd);
	return element;
}

QDomElement BoolToXML(QString _title, bool _value, QDomDocument& doc)
{
	QDomElement element = doc.createElement(_title);
	QString str;
	if (_value)
	{
		str += QString("%1\n").arg(1);
	}
	else
	{
		str += QString("%1\n").arg(0);
	}
	QDomText nd = doc.createTextNode("\n" + str);
	element.appendChild(nd);
	return element;
}

QDomElement StringToXML(QString _title, QString _context, QDomDocument& doc)
{
	QDomElement element = doc.createElement(_title);
	QString str = _context;

	QDomText nd = doc.createTextNode(str + "\n");
	element.appendChild(nd);
	return element;
}

QDomElement StringVectorToXML(QString _title, vector<QString> _contexts, QDomDocument& doc)
{
	QDomElement element = doc.createElement(_title);
	QString str;
	for (auto context : _contexts)
	{
		str += QString("%1\n").arg(context);
	}

	QDomText nd = doc.createTextNode("\n" + str);
	element.appendChild(nd);
	return element;
}

QDomElement ToothArrayInfoToXML(const vector<HexaVec>& array, QDomDocument& doc)
{
	QDomElement toothArrayElement = doc.createElement("ToothArrayInfo");

	for (int i = 0; i < array.size(); ++i)
	{
		QDomElement toothHexaVec = hexaVecToXML(array[i], QString("HexaVec"), doc);
		toothArrayElement.appendChild(toothHexaVec);
	}

	return toothArrayElement;
}

QDomElement ToothArrayInfo_RootToXML(const vector<HexaVec>& array, QDomDocument& doc)
{
	QDomElement toothArrayElement = doc.createElement("ToothArrayInfo_Root");

	for (int i = 0; i < array.size(); ++i)
	{
		QDomElement toothHexaVec = hexaVecToXML(array[i], QString("HexaVec"), doc);
		toothArrayElement.appendChild(toothHexaVec);
	}

	return toothArrayElement;
}

QDomElement hexaVecToXML(const HexaVec record, QString node_name, QDomDocument& doc)
{
	QDomElement toothHexaVecElement = doc.createElement(node_name);

	QString str;
	str += QString("%1 %2 %3 \n").arg(record.fVX).arg(record.fVY).arg(record.fVZ);
	str += QString("%1 %2 %3 \n").arg(record.fA).arg(record.fB).arg(record.fG);
	str += QString("%1 %2 %3 \n").arg(record.fZX).arg(record.fZY).arg(record.fZZ);
	QDomText sign = doc.createTextNode(str);
	toothHexaVecElement.appendChild(sign);

	return toothHexaVecElement;
}

QDomElement intToXML(const int num, QString node_name, QDomDocument& doc)
{
	QDomElement element = doc.createElement(node_name);

	QString str;
	str += QString("%1 \n").arg(num);
	QDomText sign = doc.createTextNode(str);
	element.appendChild(sign);
	return element;
}

QDomElement NeighborTeethVecListToXML(vector<pair<int, HexaVec>> _list, QDomDocument& doc)
{
	QDomElement vecListElement = doc.createElement("NeighborTeethVecList");
	for (int i = 0; i < _list.size(); ++i)
	{
		vecListElement.appendChild(NeighborToothVecToXML(_list[i], doc));
	}
	return vecListElement;
}

void ParseNeighborTeethVecList(const QDomNode& _node, vector<pair<int, HexaVec>>& _list)
{
	QDomNodeList node_list = _node.childNodes();
	for (int i = 0; i < node_list.size(); ++i)
	{
		if (node_list.at(i).nodeName() == QString("NeighborToothVec"))
		{
			pair<int, HexaVec> element;
			ParseNeighborToothVec(node_list.at(i), element);
			_list.push_back(element);
		}
	}
}

QDomElement NeighborToothVecToXML(pair<int, HexaVec> _vec, QDomDocument& doc)
{
	QDomElement toothHexaVecElement = doc.createElement("NeighborToothVec");
	toothHexaVecElement.appendChild(intToXML(_vec.first, QString("NeighToothIndex"), doc));
	toothHexaVecElement.appendChild(hexaVecToXML(_vec.second, QString("NeighborToothVector"), doc));
	return toothHexaVecElement;
}

void ParseNeighborToothVec(const QDomNode& _node, pair<int, HexaVec>& _element)
{
	QDomNodeList node_list = _node.childNodes();
	for (int i = 0; i < node_list.size(); ++i)
	{
		if (node_list.at(i).nodeName() == QString("NeighToothIndex"))
		{
			_element.first = node_list.at(i).firstChild().nodeValue().toInt();
		}
		if (node_list.at(i).nodeName() == QString("NeighborToothVector"))
		{
			ParseHexaVectorValue(node_list.at(i).firstChild(), _element.second);
		}
	}
	return;
}

template<typename T> QDomElement vectorToXML(vector<T> _list, QString _node_name, QDomDocument& doc)
{
	QDomElement vectorElement = doc.createElement(_node_name);
	QString str;
	for (auto t : _list)
	{
		str += QString("%1 ").arg(t);
	}
	str += QString("\n");
	QDomText sign = doc.createTextNode(str);
	vectorElement.appendChild(sign);
	return vectorElement;
}

template<typename T> void ParseVector(const QDomNode& _node, vector<T>& _list)
{
	QStringList values = _node.firstChild().nodeValue().split("\n", QString::SkipEmptyParts);
	if (values.empty())
	{
		return;
	}
	QStringList numList = values.at(0).split(" ", QString::SkipEmptyParts);
	for (int i = 0; i < numList.size(); ++i)
	{
		_list.push_back(numList.at(i).toInt());
	}
}

QDomElement ArtificialRecordListToXML(vector < pair<int, bool >> _list, QDomDocument& doc)
{
	QDomElement recordListElement = doc.createElement("ArtificialRecordList");
	for (int i = 0; i < _list.size(); ++i)
	{
		recordListElement.appendChild(ArtificialRecordToXML(_list[i], doc));
	}
	return recordListElement;
}

void ParseArtificialRecordList(const QDomNode& _node, vector < pair<int, bool >>& _list)
{
	QDomNodeList node_list = _node.childNodes();
	if (node_list.isEmpty())
	{
		return;
	}
	for (int i = 0; i < node_list.size(); ++i)
	{
		if (node_list.at(i).nodeName() == QString("ArtificialRecord"))
		{
			pair<int, bool> element;
			ParseArtificialRecord(node_list.at(i), element);
			_list.push_back(element);
		}
	}
}

QDomElement ArtificialRecordToXML(pair<int, bool > _record, QDomDocument& doc)
{
	QDomElement recordElement = doc.createElement(QString("ArtificialRecord"));

	QString str;
	str += QString("%1 %2 \n").arg(_record.first).arg(_record.second);
	QDomText sign = doc.createTextNode(str);
	recordElement.appendChild(sign);

	return recordElement;
}

void ParseArtificialRecord(const QDomNode& _node, pair<int, bool >& _element)
{
	QStringList values = _node.firstChild().nodeValue().split("\n", QString::SkipEmptyParts);
	QStringList numList = values.at(0).split(" ", QString::SkipEmptyParts);
	_element.first = numList.at(0).toInt();
	_element.second = numList.at(1).toInt();
}

QDomElement MoveDragRecordsToXML(std::vector<FEdge>* recordList, QDomDocument& doc)
{
	QDomElement moveLinkRecordsElement = doc.createElement("MoveLinkRecords");
	QString str;
	for (int i = 0; i < recordList->size(); ++i)
	{
		int x1, y1, x2, y2;
		x1 = recordList->at(i).vertA.X();
		y1 = recordList->at(i).vertA.Y();
		x2 = recordList->at(i).vertB.X();
		y2 = recordList->at(i).vertB.Y();
		str += QString("%1 %2 %3 %4 \n").arg(x1).arg(y1).arg(x2).arg(y2);
	}
	QDomText sign = doc.createTextNode(str);
	moveLinkRecordsElement.appendChild(sign);
	return moveLinkRecordsElement;
}

QDomElement RotateDragRecordsToXML(std::vector<FEdge>* recordList, QDomDocument& doc)
{
	QDomElement rotateLinkRecordsElement = doc.createElement("RotateLinkRecords");
	QString str;
	for (int i = 0; i < recordList->size(); ++i)
	{
		int x1, y1, x2, y2;
		x1 = recordList->at(i).vertA.X();
		y1 = recordList->at(i).vertA.Y();
		x2 = recordList->at(i).vertB.X();
		y2 = recordList->at(i).vertB.Y();
		str += QString("%1 %2 %3 %4 \n").arg(x1).arg(y1).arg(x2).arg(y2);
	}
	QDomText sign = doc.createTextNode(str);
	rotateLinkRecordsElement.appendChild(sign);
	return rotateLinkRecordsElement;
}

QDomElement SynDragRecordCPToXML(std::vector<vcg::Point2i>* cpList, QDomDocument& doc)
{
	QDomElement synCPsElement = doc.createElement("SynLinkRecordCPs");
	QString str;
	for (int i = 0; i < cpList->size(); ++i)
	{
		int x1, y1, x2, y2;
		x1 = cpList->at(i).X();
		y1 = cpList->at(i).Y();
		str += QString("%1 %2 \n").arg(x1).arg(y1);
	}
	QDomText sign = doc.createTextNode(str);
	synCPsElement.appendChild(sign);
	return synCPsElement;
}

QDomDocument MeshDocumentToXML(MeshDocument& md, bool onlyVisibleLayers, bool bIsSegmentProject)
{
	QDomDocument ddoc("MeshLabDocument");

	QDomElement root = ddoc.createElement("MeshLabProject");
	ddoc.appendChild(root);
	QDomElement mgroot = ddoc.createElement("MeshGroup");
	has_recored_analyzer_data_ = false;
	foreach(MeshModel * mmp, md.meshList)
	{
		if ((!onlyVisibleLayers) || (mmp->visible))
		{
			if (mmp->upperOrLowerToothModelMark == UpperToothModel || mmp->upperOrLowerToothModelMark == UpperSingleDental ||
				mmp->upperOrLowerToothModelMark == LowerToothModel || mmp->upperOrLowerToothModelMark == LowerSingleDental)
			{
				QDomElement meshElem = MeshModelToXML(mmp, ddoc, bIsSegmentProject);
				mgroot.appendChild(meshElem);
			}
		}
	}
	root.appendChild(mgroot);
	return ddoc;
}

QDomElement EachToothConvexVertsToXML(Dental* dental, QDomDocument& doc)
{
	QDomElement eachToothConvexVertsElement = doc.createElement("EachToothConvexVerts");

	bool canConstruct = true;
	for (auto i : dental->toothSortIndexList)
	{
		if (dental->teeth[i].frozenConvexVerts.empty())
		{
			canConstruct = false;
			break;
		}
	}

	if (canConstruct)
	{
		for (auto i : dental->toothSortIndexList)
		{
			QDomElement toothConvexVertsIndexesElement = ToothConvexVertsToXML(dental->teeth[i].frozenConvexVerts, doc);
			eachToothConvexVertsElement.appendChild(toothConvexVertsIndexesElement);
		}
	}

	return eachToothConvexVertsElement;
}

QDomElement ToothConvexVertsToXML(std::vector<Point3m> verts, QDomDocument& doc)
{
	QDomElement toothConvexVertsElement = doc.createElement("ToothConvexVerts");
	QString str;
	for (auto p : verts)
	{
		str += QString("%1 %2 %3\n").arg(p.X()).arg(p.Y()).arg(p.Z());
	}

	QDomText nd = doc.createTextNode("\n" + str);
	toothConvexVertsElement.appendChild(nd);
	return toothConvexVertsElement;
}

QDomElement EachToothFeatureMarksToXML(Dental* dental, QDomDocument& doc)
{
	QDomElement eachToothFeatureMarksElement = doc.createElement("EachToothFeatureMarks");

	for (int i = 0; i < STANDARD_TOOTH_SUM; i++)
	{
		if (dental->bToothExist[i])
		{
			if (!dental->teeth[i].feature_marks_.empty())
			{
				QDomElement toothMarksElement = ToothFeatureMarksToXML(dental->teeth[i].feature_marks_, doc);
				eachToothFeatureMarksElement.appendChild(toothMarksElement);
			}
			else
			{
				QDomElement toothMarksElement = ToothFeatureMarksToXML(vector<Point3m>(), doc);
				eachToothFeatureMarksElement.appendChild(toothMarksElement);
			}
		}
		else
		{
			QDomElement toothMarksElement = ToothFeatureMarksToXML(vector<Point3m>(), doc);
			eachToothFeatureMarksElement.appendChild(toothMarksElement);
		}
	}
	return eachToothFeatureMarksElement;
}

QDomElement ToothFeatureMarksToXML(std::vector<Point3m> verts, QDomDocument& doc)
{
	QDomElement toothConvexVertsElement = doc.createElement("ToothFeatureMarks");
	QString str;
	for (auto p : verts)
	{
		str += QString("%1 %2 %3\n").arg(p.X()).arg(p.Y()).arg(p.Z());
	}

	QDomText nd = doc.createTextNode("\n" + str);
	toothConvexVertsElement.appendChild(nd);
	return toothConvexVertsElement;
}
