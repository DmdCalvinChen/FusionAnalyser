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

#include "meshmodel.h"
#include "dental.h"
#include "meshlabdocumentxml.h"
#include <wrap/qt/shot_qt.h>
#include <../common_ext/data/fusionaligndata.h>

QDomElement VertexsPickedForLocateToXML(std::vector<std::pair<Point3f, Point3f>>& vertexIndexPicked, QDomDocument& doc);
QDomElement VertexsPickedForLocateToXML(std::vector<std::pair<int, int>>& vertexIndexPicked, QDomDocument& doc);
QDomElement ToothExistMarkToXML(std::vector<int>& toothExistMark, QDomDocument& doc);
QDomElement AxisToothInfoToXML(std::vector<Axis*>& toothAxis, QDomDocument& doc);
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

QDomElement OrderIDToXML(QString id, QDomDocument& doc);
QDomElement BiteplaneToXML(CustomPlane bitePlane, QDomDocument& doc);
QDomElement WidthAndDepthToXML(float width, float depth, QDomDocument& doc);
QDomElement PulloutPlanToXML(std::vector<int> pulloutList, QDomDocument& doc);
QDomElement CureInfoToXML(std::vector<CureInfom>* stageList, QDomDocument& doc);
QDomElement CureStageToXML(const CureInfom& stage, QDomDocument& doc);
QDomElement SubStageIndexToXML(int stageIndex, QDomDocument& doc);
QDomElement FatherStageSignToXML(bool bSign, QDomDocument& doc);
QDomElement BraceNumberToXML(int num, QDomDocument& doc);
QDomElement ToothArrayInfoToXML(const vector<HexaVec>& array, QDomDocument& doc);
QDomElement ToothArrayInfo_RootToXML(const vector<HexaVec>& array, QDomDocument& doc);
QDomElement ToothHexaVecToXML(const HexaVec record, QDomDocument& doc);
QDomElement SonCureInfoListToXML(const vector<CureInfom>& sonCureInfoList, QDomDocument& doc);
// AccessoryTool removed - files deleted, AccessoryType moved from treatmentInfoRecord.h
QDomElement MoveDragRecordsToXML(std::vector<FEdge>* recordList, QDomDocument& doc);
QDomElement RotateDragRecordsToXML(std::vector<FEdge>* recordList, QDomDocument& doc);
QDomElement SynDragRecordCPToXML(std::vector<vcg::Point2i>* cpList, QDomDocument& doc);
QDomElement SliceCutRecordListToXML(std::vector<SliceCutRecord>* recordList, QDomDocument& doc);
QDomElement SliceCutRecordToXML(SliceCutRecord* record, QDomDocument& doc);
QDomElement EachToothConvexVertsToXML(Dental* dental, QDomDocument& doc);
QDomElement ToothConvexVertsToXML(std::vector<Point3m> verts, QDomDocument& doc);

void ParseCureInfoList(const QDomNodeList& cureInfoList, vector<CureInfom>& cureInfoListRecord);
void ParseCureInfo(const QDomNode& cureInfo, CureInfom& cureStage);
void ParseHexaVectorValue(const QDomNode& hexaVNode, HexaVec& hexaValue);
// AccessoryTool removed - files deleted
void ParseMoveRecords(QDomNode moveLinks, vector<FEdge>& moveRecords);
void ParseRotateRecords(QDomNode rotateLinks, vector<FEdge>& rotateRecords);
void ParseSynCPsRecords(QDomNode moveLinks, vector<Point2i>& synCPRecords);
void ParseSliceCutRecordList(QDomNode recordNodes, vector<SliceCutRecord>& sliceCutRecordList);
void ParseSliceCutRecord(QDomNode recordNode, SliceCutRecord& sliceCutRecord);
void ParseEachToothConvexVerts(QDomNode eachToothIndexesNode, vector<vector<Point3m>>& eachToothVerts);
void ParseToothConvexVerts(QDomNode toothIndexesNode, vector<Point3m>& toothVerts);

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
	//Devices
	while (!node.isNull()) {
		if (QString::compare(node.nodeName(), "MeshGroup") == 0)
		{
			QDomNode mesh; QString filen, label;
			QString upperOrLowerJaw;
			mesh = node.firstChild();
			while (!mesh.isNull()) {
				//return true;
				filen = mesh.attributes().namedItem("filename").nodeValue();
				label = mesh.attributes().namedItem("label").nodeValue();
				upperOrLowerJaw = mesh.attributes().namedItem("UpperOrLowerJaw").nodeValue();

				if (!filen.isEmpty())
				{
					/*MeshModel* mm = */md.addNewMesh(filen, label);
				}

				SegmentedStatusInfo* segmented_data = nullptr;

				if (upperOrLowerJaw.compare("UpperJaw") == 0)
				{
					md.mm()->upperOrLowerToothModelMark = UpperToothModel;

					FusionAlignData::getInstance()->segmneted_info_upper_ = new SegmentedStatusInfo;
					segmented_data = FusionAlignData::getInstance()->segmneted_info_upper_;
				}
				else if (upperOrLowerJaw.compare("LowerJaw") == 0)
				{
					md.mm()->upperOrLowerToothModelMark = LowerToothModel;

					FusionAlignData::getInstance()->segmneted_info_lower_ = new SegmentedStatusInfo;
					segmented_data = FusionAlignData::getInstance()->segmneted_info_lower_;
				}

				int numTooth = mesh.attributes().namedItem("numtoothsegmented").nodeValue().toInt();

				if (numTooth >= 0)
				{
					segmented_data->clearData();
					segmented_data->numToothSegmented = numTooth;
				}

				QString cbct_model_file_name = mesh.attributes().namedItem("cbct_model_file_name").nodeValue();
				segmented_data->cbct_model_file_name = cbct_model_file_name;

				QString root_model_file_name = mesh.attributes().namedItem("root_model_file_name").nodeValue();
				segmented_data->root_model_file_name = root_model_file_name;

				//QDomNode tr = mesh.firstChild();
				QDomNodeList list = mesh.childNodes(); //获得元素e的所有子节点的列表
				for (int a = 0; a < list.count(); a++) //遍历该列表
				{
					QDomNode node = list.at(a);

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
								Axis* axis = new Axis;

								axis->axisXVector = Point3f(values[index].toFloat(), values[index + 1].toFloat(), values[index + 2].toFloat());
								index += 3;
								axis->axisYVector = Point3f(values[index].toFloat(), values[index + 1].toFloat(), values[index + 2].toFloat());
								index += 3;
								axis->axisZVector = Point3f(values[index].toFloat(), values[index + 1].toFloat(), values[index + 2].toFloat());
								index += 3;
								axis->centerPoint = Point3f(values[index].toFloat(), values[index + 1].toFloat(), values[index + 2].toFloat());

								segmented_data->axisTooth.push_back(axis);
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
								// 牙龈无标记
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

							if (segmented_data->bitePlaneGenerated)
							{
								delete segmented_data->bitePlane;
								segmented_data->bitePlane = NULL;
								segmented_data->bitePlaneGenerated = false;
							}
							if (!segmented_data->bitePlaneGenerated)
							{
								segmented_data->bitePlane = new CustomPlane();
								segmented_data->bitePlane->center = center;
								segmented_data->bitePlane->axisXV = axisX;
								segmented_data->bitePlane->axisYV = axisY;
								segmented_data->bitePlane->axisZV = axisZ;
								segmented_data->bitePlaneGenerated = true;
							}

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

							if (segmented_data->bitePlane != NULL)
							{
								segmented_data->fPlaneWidth = width;
								segmented_data->fPlaneDepth = depth;
							}
						}
					}

					if (node.nodeName() == "PullOutList")
					{
						if (node.childNodes().size() == 1)
						{
							QStringList values = node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
							vector<int> pullOutList;
							for (auto value : values)
							{
								pullOutList.push_back(value.toInt());
							}

							if (segmented_data->bitePlane != NULL)
							{
								segmented_data->pullOutList = pullOutList;
							}
						}
					}

					if (node.nodeName() == "CureInfoList")
					{
						vector<CureInfom> cureInfoListRecord;
						QDomNodeList cureInfoList = node.childNodes();
						ParseCureInfoList(cureInfoList, cureInfoListRecord);

						if (!cureInfoListRecord.empty())
						{
							segmented_data->cureInfoListRecordPointer = new vector<CureInfom>;
							for (int i = 0; i < cureInfoListRecord.size(); ++i)
							{
								segmented_data->cureInfoListRecordPointer->push_back(cureInfoListRecord.at(i));
							}
						}
					}

					// AccessoryTool removed - files deleted

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

					if (node.nodeName() == "SliceCutRecordList")
					{
						ParseSliceCutRecordList(node, segmented_data->sliceCutRecords);
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

void ParseCureInfoList(const QDomNodeList& cureInfoList, vector<CureInfom>& cureInfoListRecord)
{
	for (int i = 0; i < cureInfoList.size(); ++i)
	{
		CureInfom cureStage;
		ParseCureInfo(cureInfoList.at(i), cureStage);

		cureInfoListRecord.push_back(cureStage);
	}
}

void ParseCureInfo(const QDomNode& cureInfo, CureInfom& cureStage)
{
	// Treatment functionality removed - iSub, bStage, iBraceNum, sonCureInfo, accInfo, baseBraceInfo no longer exist
	vector<HexaVec> toothArrayInfo, toothArrayInfo_Root;
	// vector<CureInfom> sonCureInfoRecordList;

	QDomNodeList cureInfoDetails = cureInfo.childNodes();
	for (int i = 0; i < cureInfoDetails.size(); ++i)
	{
		QDomNode node = cureInfoDetails.at(i);

		// StageIndex, FatherStageSign, BraceNumber removed - members no longer exist in CureInfom

		// if (node.nodeName() == "FatherStageSign")
		// {
		// 	if (node.childNodes().size() == 1)
		// 	{
		// 		QStringList values = node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
		// 		bIsFatherStage = values[0].toFloat() == 0 ? false : true;
		// 		cureStage.bStage = bIsFatherStage;
		// 	}
		// }

		// if (node.nodeName() == "BraceNumber")
		// {
		// 	if (node.childNodes().size() == 1)
		// 	{
		// 		QStringList values = node.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
		// 		iBraceNum = values[0].toFloat();
		// 		cureStage.iBraceNum = iBraceNum;
		// 	}
		// }

		if (node.nodeName() == "ToothArrayInfo")
		{
			QDomNodeList arrayList = node.childNodes();
			for (int i = 0; i < arrayList.size(); ++i)
			{
				HexaVec tempHexVec;
				ParseHexaVectorValue(arrayList.at(i), tempHexVec);
				toothArrayInfo.push_back(tempHexVec);
			}
			cureStage.teethInfo = toothArrayInfo;
		}

		if (node.nodeName() == "ToothArrayInfo_Root")
		{
			QDomNodeList arrayList = node.childNodes();
			for (int i = 0; i < arrayList.size(); ++i)
			{
				HexaVec tempHexVec;
				ParseHexaVectorValue(arrayList.at(i), tempHexVec);
				toothArrayInfo_Root.push_back(tempHexVec);
			}
			cureStage.teethInfo_Root = toothArrayInfo_Root;
		}

		// SonCureInfoList removed - sonCureInfo member no longer exists
	}
	// AccessoryTool and BaseBrace removed - files deleted

	toothArrayInfo.clear();
	// sonCureInfoRecordList.clear();
}

void ParseHexaVectorValue(const QDomNode& hexaVNode, HexaVec& hexaValue)
{
	float fVX, fVY, fVZ;
	float fA, fB, fG;

	if (hexaVNode.childNodes().size() == 1)
	{
		QStringList values = hexaVNode.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
		fVX = values[0].toFloat();
		fVY = values[1].toFloat();
		fVZ = values[2].toFloat();
		fA = values[3].toFloat();
		fB = values[4].toFloat();
		fG = values[5].toFloat();
		hexaValue.Inital(fVX, fVY, fVZ, fA, fB, fG);
	}
}

// AccessoryTool removed - files deleted
// void ParseAccessoryInfoMatrix(QDomNode nodeMatrix, vector<vector<AccessoryTool*>>& accInfoRecord)

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

void ParseSliceCutRecordList(QDomNode recordNodes, vector<SliceCutRecord>& sliceCutRecordList)
{
	vector<SliceCutRecord>().swap(sliceCutRecordList);

	QDomNodeList nodeList = recordNodes.childNodes();
	for (int i = 0; i < nodeList.size(); ++i)
	{
		if (nodeList.at(i).nodeName() == "SliceCutRecord")
		{
			SliceCutRecord record;
			ParseSliceCutRecord(nodeList.at(i), record);
			sliceCutRecordList.push_back(record);
		}
	}
}

void ParseSliceCutRecord(QDomNode node, SliceCutRecord& sliceCutRecord)
{
	if (node.childNodes().size() == 1)
	{
		QStringList values = node.firstChild().nodeValue().split("\n", QString::SkipEmptyParts);
		QStringList numList = values.at(0).split(" ", QString::SkipEmptyParts);
		int iToothA = numList[0].toInt();
		int iToothB = numList[1].toInt();
		int iStep = numList[2].toInt();
		float fCutDepth = numList[3].toFloat();
		sliceCutRecord.initial(iToothA, iToothB, fCutDepth, iStep);
	}
}

// AccessoryTool removed - files deleted
// void ParseAccessoryInfoList(QDomNode node, vector<AccessoryTool*>& accInfoListRecord)
// AccessoryTool* ParseAccessoryInfo(const QDomNode& nodeFather, AccessoryTool*& accInfo)
// 			case 0:
// 			case 1:
// 			case 2:
// 			case 3:
// 			case 4:
// 			case 5:
// 			case 6:
// 			case 7:
// 			case 8:
// 			case 9:
// 			else
// 			else
// 			else

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

QDomElement MeshModelToXML(MeshModel* mp, QDomDocument& doc, bool bIsSegmentProject)
{
	QDomElement meshElem = doc.createElement("MLMesh");

	SegmentedStatusInfo* segmented_data = nullptr;

	if (mp->upperOrLowerToothModelMark == UpperToothModel || mp->upperOrLowerToothModelMark == UpperSingleDental)
	{
		if (FusionAlignData::getInstance()->segmneted_info_upper_)
		{
			segmented_data = FusionAlignData::getInstance()->segmneted_info_upper_;
		}
	}
	else if (mp->upperOrLowerToothModelMark == LowerToothModel || mp->upperOrLowerToothModelMark == LowerSingleDental)
	{
		if (FusionAlignData::getInstance()->segmneted_info_lower_)
		{
			segmented_data = FusionAlignData::getInstance()->segmneted_info_lower_;
		}
	}

	if (bIsSegmentProject)
	{
		meshElem.setAttribute("label", mp->label());
		meshElem.setAttribute("filename", mp->relativePathName());
		meshElem.setAttribute("cbct_model_file_name", segmented_data->cbct_model_file_name);
	}
	else
	{
		meshElem.setAttribute("label", mp->_labelSegmentMark);
		meshElem.setAttribute("filename", mp->_fullPathFileNameSegment);
		meshElem.setAttribute("root_model_file_name", segmented_data->root_model_file_name);
	}

	DentalManager* cur_manager = nullptr;
	if (mp->upperOrLowerToothModelMark == UpperToothModel || mp->upperOrLowerToothModelMark == UpperSingleDental)
	{
		meshElem.setAttribute("UpperOrLowerJaw", "UpperJaw");
		cur_manager = PFusionAlignData->upperManager();
	}
	else
	{
		meshElem.setAttribute("UpperOrLowerJaw", "LowerJaw");
		cur_manager = PFusionAlignData->lowerManager();
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
			meshElem.appendChild(OrderIDToXML(
				cur_manager->cDental.strOrderID, doc));
			meshElem.appendChild(BiteplaneToXML(
				cur_manager->cDental.basePlane, doc));
			meshElem.appendChild(WidthAndDepthToXML(
				cur_manager->cDental.fWidth, cur_manager->cDental.fDepth, doc));
			meshElem.appendChild(PulloutPlanToXML(
				cur_manager->cDental.pullOutToothList, doc));
			meshElem.appendChild(EachToothConvexVertsToXML
			(&cur_manager->cDental, doc));
			meshElem.appendChild(FixtureCSYSToXML(
				cur_manager->cDental.fixtureCSYS, doc));
			meshElem.appendChild(AimArchBezierCtrlNodes2DToXML(
				cur_manager->cDental.archBezierCtrlNodes2D, doc));
		}
		if (cur_manager->cDental.bCureModelGenerated)
		{
			meshElem.appendChild(CureInfoToXML(
				cur_manager->cDental.stageInfoList, doc));

			// AccessoryTool removed - files deleted
			// meshElem.appendChild(AccessoryInfoMatrixToXML(

			meshElem.appendChild(MoveDragRecordsToXML(
				&cur_manager->cDental.moveLinkRecords, doc));
			meshElem.appendChild(RotateDragRecordsToXML(
				&cur_manager->cDental.rotateLinkRecords, doc));
			meshElem.appendChild(SynDragRecordCPToXML(
				&cur_manager->cDental.synLinkCPRecords, doc));
			meshElem.appendChild(SliceCutRecordListToXML(
				&cur_manager->cDental.sliceCutRecords, doc));
		}
	}
	return meshElem;
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

QDomElement AxisToothInfoToXML(std::vector<Axis*>& toothAxis, QDomDocument& doc)
{
	QDomElement matrixElem = doc.createElement("ToothAxis");
	QString str;
	for (auto axis : toothAxis)
	{
		str += QString("%1 %2 %3 \n").arg(axis->axisXVector.X()).arg(axis->axisXVector.Y()).arg(axis->axisXVector.Z());
		str += QString("%1 %2 %3 \n").arg(axis->axisYVector.X()).arg(axis->axisYVector.Y()).arg(axis->axisYVector.Z());
		str += QString("%1 %2 %3 \n").arg(axis->axisZVector.X()).arg(axis->axisZVector.Y()).arg(axis->axisZVector.Z());
		str += QString("%1 %2 %3 \n").arg(axis->centerPoint.X()).arg(axis->centerPoint.Y()).arg(axis->centerPoint.Z());
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
	for (auto loopContour : controlsIndexMark)
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

QDomElement PulloutPlanToXML(std::vector<int> pulloutList, QDomDocument& doc)
{
	QDomElement pullOutListElement = doc.createElement("PullOutList");
	QString str;
	for (auto index : pulloutList)
	{
		str += QString("%1 ").arg(index);
	}
	str += QString('\n');

	QDomText nd = doc.createTextNode("\n" + str);
	pullOutListElement.appendChild(nd);
	return pullOutListElement;
}

QDomElement CureInfoToXML(std::vector<CureInfom>* stageList, QDomDocument& doc)
{
	QDomElement cureInfoList = doc.createElement("CureInfoList");
	for (int i = 0; i < stageList->size(); ++i)
	{
		QDomElement cureStage = CureStageToXML(stageList->at(i), doc);
		cureInfoList.appendChild(cureStage);
	}
	return cureInfoList;
}

QDomElement CureStageToXML(const CureInfom& stage, QDomDocument& doc)
{
	QDomElement cureStage = doc.createElement("CureStage");

	QDomElement subStageIndex = SubStageIndexToXML(stage.iSub, doc);
	cureStage.appendChild(subStageIndex);
	QDomElement fatherStageSign = FatherStageSignToXML(stage.bStage, doc);
	cureStage.appendChild(fatherStageSign);
	QDomElement braceNum = BraceNumberToXML(stage.iBraceNum, doc);
	cureStage.appendChild(braceNum);
	QDomElement toothArrayInfo = ToothArrayInfoToXML(stage.teethInfo, doc);
	cureStage.appendChild(toothArrayInfo);
	QDomElement toothArrayInfo_Root = ToothArrayInfo_RootToXML(stage.teethInfo_Root, doc);
	cureStage.appendChild(toothArrayInfo_Root);
	if (stage.bStage)
	{

		QDomElement accInfoList = AccessoryInfoListToXML(*stage.accInfo, doc);
		cureStage.appendChild(accInfoList);
	}
	QDomElement sonCureInfo = SonCureInfoListToXML(stage.sonCureInfo, doc);
	cureStage.appendChild(sonCureInfo);

	return cureStage;
}

QDomElement SubStageIndexToXML(int stageIndex, QDomDocument& doc)
{
	QDomElement stageIndexElement = doc.createElement("StageIndex");
	QString str;
	str += QString("%1 \n").arg(stageIndex);
	QDomText iSub = doc.createTextNode(str);
	stageIndexElement.appendChild(iSub);
	return stageIndexElement;
}

QDomElement FatherStageSignToXML(bool bSign, QDomDocument& doc)
{
	QDomElement fatherStageSignElement = doc.createElement("FatherStageSign");
	QString str;
	int iSign = bSign ? 1 : 0;
	str += QString("%1 \n").arg(iSign);
	QDomText sign = doc.createTextNode(str);
	fatherStageSignElement.appendChild(sign);
	return fatherStageSignElement;
}

QDomElement BraceNumberToXML(int num, QDomDocument& doc)
{
	QDomElement braceNumElement = doc.createElement("BraceNumber");
	QString str;
	str += QString("%1 \n").arg(num);
	QDomText sign = doc.createTextNode(str);
	braceNumElement.appendChild(sign);
	return braceNumElement;
}

QDomElement ToothArrayInfoToXML(const vector<HexaVec>& array, QDomDocument& doc)
{
	QDomElement toothArrayElement = doc.createElement("ToothArrayInfo");

	for (int i = 0; i < array.size(); ++i)
	{
		QDomElement toothHexaVec = ToothHexaVecToXML(array[i], doc);
		toothArrayElement.appendChild(toothHexaVec);
	}

	return toothArrayElement;
}

QDomElement ToothArrayInfo_RootToXML(const vector<HexaVec>& array, QDomDocument& doc)
{
	QDomElement toothArrayElement = doc.createElement("ToothArrayInfo_Root");

	for (int i = 0; i < array.size(); ++i)
	{
		QDomElement toothHexaVec = ToothHexaVecToXML(array[i], doc);
		toothArrayElement.appendChild(toothHexaVec);
	}

	return toothArrayElement;
}

QDomElement ToothHexaVecToXML(const HexaVec record, QDomDocument& doc)
{
	QDomElement toothHexaVecElement = doc.createElement("ToothHexaVec");

	QString str;
	str += QString("%1 %2 %3 \n").arg(record.fVX).arg(record.fVY).arg(record.fVZ);
	str += QString("%1 %2 %3 \n").arg(record.fA).arg(record.fB).arg(record.fG);
	QDomText sign = doc.createTextNode(str);
	toothHexaVecElement.appendChild(sign);

	return toothHexaVecElement;
}

QDomElement SonCureInfoListToXML(const vector<CureInfom>& sonCureInfoList, QDomDocument& doc)
{
	QDomElement sonCureInfoListElement = doc.createElement("SonCureInfoList");

	for (int i = 0; i < sonCureInfoList.size(); ++i)
	{
		QDomElement sonCureInfoElement = CureStageToXML(sonCureInfoList[i], doc);
		sonCureInfoListElement.appendChild(sonCureInfoElement);
	}

	return sonCureInfoListElement;
}

// AccessoryTool removed - files deleted

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

QDomElement SliceCutRecordListToXML(std::vector<SliceCutRecord>* recordList, QDomDocument& doc)
{
	QDomElement sliceCutRecordsElement = doc.createElement("SliceCutRecordList");
	for (int i = 0; i < recordList->size(); ++i)
	{
		sliceCutRecordsElement.appendChild(SliceCutRecordToXML(&recordList->at(i), doc));
	}
	return sliceCutRecordsElement;
}

QDomElement SliceCutRecordToXML(SliceCutRecord* record, QDomDocument& doc)
{
	QDomElement sliceCutRecordElement = doc.createElement("SliceCutRecord");
	QString str;
	str += QString("%1 %2 %3 %4 \n").arg(record->iToothA_Index).arg(record->iToothB_Index).arg(record->iInitialStep).arg(record->fSliceCutAmount);
	QDomText sign = doc.createTextNode(str);
	sliceCutRecordElement.appendChild(sign);
	return sliceCutRecordElement;
}

// AccessoryTool removed - files deleted
// QDomElement AccessoryInfoListToXML(vector<AccessoryTool*> accPointerList, QDomDocument& doc)
// QDomElement AccessortyInfoToXML(const AccessoryTool* accToolPointer, QDomDocument& doc)
// QDomElement AccessoryIndexToXML(const int iNo, QDomDocument& doc)
// QDomElement AccessoryDigoutToXML(const bool digout, QDomDocument& doc)
// 	else
// QDomElement AccessoryLeftHookToXML(const bool leftHook, QDomDocument& doc)
// 	else

QDomDocument MeshDocumentToXML(MeshDocument& md, bool onlyVisibleLayers, bool bIsSegmentProject)
{
	QDomDocument ddoc("MeshLabDocument");

	QDomElement root = ddoc.createElement("MeshLabProject");
	ddoc.appendChild(root);
	QDomElement mgroot = ddoc.createElement("MeshGroup");

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
