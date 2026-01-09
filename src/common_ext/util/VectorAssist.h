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

#ifndef VECTORASSIST_H
#define VECTORASSIST_H

#include <vector>
#include "common/config.h"
#include "util/assist_geometry.h"
#include "autoAlign/longaxislocationtool.h"

/*
*  all vertex adjacency map information etc
*/
struct VertexTopology
{
	std::vector<int> vertexAdjacencyIndex;
	VertexTopology() {}
};

enum ColorMark
{
	DEFAULT = 0,
	RED,/** real border vertex*/
	YELLOW,/** center vertex*/
	GREEN, /** disk vertex ALL tooths ,increased as the tooth increase*/
	BLUE, /** disk vertex gingiva / all gingiva*/
	TOOTH1,/**mark tooth from one to ...*/
	TOOTH2,
	TOOTH3,
	TOOTH4,
	TOOTH5,
	TOOTH6,
	TOOTH7,
	TOOTH8,
	TOOTH9,
	TOOTH10,
	TOOTH11,
	TOOTH12,
	TOOTH13,
	TOOTH14,
	TOOTH15,
	TOOTH16, // 最多牙齿从1-16
	TOOTH17,
	TOOTH18,
	TOOTH19,
	TOOTH20
};

/** seem as the struct of VertexTopology*/
struct VertexBorderMapInfo
{
	bool bWhetherIsBorder = false;
	bool bHaveMarked;
	bool bFeauture;
	bool bCuttingPoints = false;
	int vertexId;
	int markedId;
	ColorMark color;
	ColorMark tooth2toothBorder;
	ColorMark beforeBecomeSingleBorderColor;
	ColorMark finalVertexMarked;
	ColorMark segmentOrigion;
	ColorMark leftToothClr, rightToothClr;
	VertexBorderMapInfo() {
		this->bWhetherIsBorder = false;
		this->bHaveMarked = false;
		this->bFeauture = false;
		this->markedId = -1;
		this->color = DEFAULT;
		this->finalVertexMarked = DEFAULT;
	}
	VertexBorderMapInfo(int vertexId, bool bWhetherIsBorder = false, bool bHaveMarked = false,
		bool bFeauture = false, int markedId = -1, ColorMark color = DEFAULT)
	{
		this->vertexId = vertexId;
		this->bWhetherIsBorder = bWhetherIsBorder;
		this->bHaveMarked = bHaveMarked;
		this->bFeauture = bFeauture;
		this->markedId = markedId;
		this->color = color;
		this->finalVertexMarked = color;
	}
};

struct MarkedStatistics
{
	int markedId;
	float colorMarked;
	Point3m center;
	std::vector<int>allSameMarkedIdVertexId;
};

struct DataStatistics
{
	int vertexId;
	Point3f projectPos;
	int vertexIdB;
	float distance;
	DataStatistics()
	{

	}
	DataStatistics(int vertexId,
		Point3f pro)
	{
		this->vertexId = vertexId;
		this->projectPos = pro;
	}
	DataStatistics(int vertexId,
		float distance)
	{
		this->vertexId = vertexId;
		this->distance = distance;
	}
	DataStatistics(int vertexId, int vertexIdB, float distance)
	{
		this->vertexId = vertexId;
		this->vertexIdB = vertexIdB;
		this->distance = distance;
	}
};

/** the Continuous vertexs for smooth
*/
struct AllToothBorderClosedContinuousIndex
{
	std::vector<int> toothBorderClosedContinuousIndex;
	std::vector<int> bezierControlPointsIndex;
	std::vector<int> prepareWhetherAsSmoothedVertexIndex;
	std::vector<int> smoothVertexIndexSingleBorderOnMesh;
	std::vector<vcg::Point3f> hadSmoothedVertexsPosInfo;
	std::vector<vcg::Point3f> hadSmoothedVertexsPosRealInfo;
	std::vector<pair<Point3f, Point3f>> cuttingPoints;
	std::vector<Point3m> pointsAMB;

	void clearData() {
		clearVector(toothBorderClosedContinuousIndex);
		clearVector(bezierControlPointsIndex);
		clearVector(prepareWhetherAsSmoothedVertexIndex);
		clearVector(smoothVertexIndexSingleBorderOnMesh);
		clearVector(hadSmoothedVertexsPosInfo);
		clearVector(hadSmoothedVertexsPosRealInfo);
		clearVector(cuttingPoints);
		clearVector(pointsAMB);
	}
};

/** we make tooth many parts for tooth segmenting in order to improve performance
*/
struct ToothParticalData
{
	int beginVecId;
	int endVecId;
	float radiusRange;
	std::vector<int> vecOldId;
	std::map<int, int> vecNewIdMap;
	std::vector<int> forwardFeautureOldId;
	std::vector<int> backFeautureOldId;
	ToothParticalData() {}
	ToothParticalData(
		const int beginVecId, const int endVecId,
		const float radiusRange,
		std::vector<int>& vecOldId,
		std::map<int, int>& vecNewIdMap,
		std::vector<int>& forwardFeautureOldId,
		std::vector<int>& backFeautureOldId
	)
	{
		this->beginVecId = beginVecId;
		this->endVecId = endVecId;
		this->radiusRange = radiusRange;
		this->vecOldId = vecOldId;
		this->vecNewIdMap = vecNewIdMap;
		this->forwardFeautureOldId = forwardFeautureOldId;
		this->backFeautureOldId = backFeautureOldId;
	}
};

struct ToothSmoothArea
{
	int centerVertexIndex;
	std::vector<int> vecOldId;
	std::map<int, int> vecNewIdMap;
	std::vector<int> fixAnchorOldVertexId;
	std::vector<int> moveAnchorOldVertexId;
	ToothSmoothArea() {}
	ToothSmoothArea(
		int _centerVertexIndex,
		std::vector<int>& _vecOldId,
		std::map<int, int>& _vecNewIdMap,
		std::vector<int>& _fixAnchorOldVertexId,
		std::vector<int>& _moveAnchorOldVertexId
	)
	{
		this->centerVertexIndex = _centerVertexIndex;
		this->vecOldId = _vecOldId;
		this->vecNewIdMap = _vecNewIdMap;
		this->fixAnchorOldVertexId = _fixAnchorOldVertexId;
		this->moveAnchorOldVertexId = _moveAnchorOldVertexId;
	}
};

struct SegmentResult
{
	std::vector<double> resultData;
};

struct CuttingPoints
{
	pair<ColorMark, ColorMark> toothBelongTo;
	int index;
	Point3f pos;
	CuttingPoints(pair<ColorMark, ColorMark> cuttingPoint, int index)
	{
		toothBelongTo = cuttingPoint;
		this->index = index;
	}

};
struct CuttingPointsPair
{
	pair<ColorMark, ColorMark> toothBelongTo;
	pair<int, int> indexOnMeshPair;
	pair<Point3f, Point3f> posOnMeshPair;
	CuttingPointsPair(pair<ColorMark, ColorMark> cuttingPoint, pair<int, int> indexPair, pair<Point3f, Point3f> posPair)
	{
		toothBelongTo = cuttingPoint;
		indexOnMeshPair = indexPair;
		posOnMeshPair = posPair;
	}
	pair<int, int> cutOffIndexPair;
	pair<Point3m, Point3m> cutOffPosPair;
	std::vector<int> redundantAreaOnMesh;
	std::set<int> deleteRedundantFace;
	std::set<int> deleteRedundantVer;
};

struct SingleToothSegmentedInfo
{
	bool bHaveCuttingPoints = false;
	std::vector<pair<Point3m, Point3m>> cuttingPointsPair;
	SingleToothSegmentedInfo()
	{
	}
	SingleToothSegmentedInfo(std::vector<pair<Point3m, Point3m>>& _cuttingPointsPair)
	{
		cuttingPointsPair = _cuttingPointsPair;
	}
};

struct ToothPickedInfomation
{
private:
	bool bHadLocated = false;

public:
	bool bIsExisted = false;
	QString labelToothFdi;
	Axis* axisTooth = nullptr;

	ToothParticalData* tooth_partical_data = nullptr;

	std::vector<Point3m> nearAndFarPoints;

	int seed_vertex_index;// 对应牙位上的标记索引,为分割区域增长提供种子索引
	Point3m seed_vertex;// 作为种子点的参考点（测试显示使用）

	std::vector<Point3m> contour_line;
	Point3m center_contour_line;
	CMeshO* mesh_crown = nullptr;
	LongAxisLocationTool* long_axis_tool_ = nullptr;
	std::map<QString, vcg::Point3f> landmarks;// 牙冠标记点  std::map[fdi_label,value]

	ToothPickedInfomation() {}

	~ToothPickedInfomation()
	{
		clearAllInfoStatus();
	}

	ToothPickedInfomation(bool _bIsExisted, QString _labelToothFdi)
	{
		bIsExisted = _bIsExisted;
		labelToothFdi = _labelToothFdi;
	}

	ToothPickedInfomation(const ToothPickedInfomation& _toothPickedInfo)
	{
		bIsExisted = _toothPickedInfo.bIsExisted;
		labelToothFdi = _toothPickedInfo.labelToothFdi;
		axisTooth = _toothPickedInfo.axisTooth;
		nearAndFarPoints = _toothPickedInfo.nearAndFarPoints;
		tooth_partical_data = _toothPickedInfo.tooth_partical_data;
	}

	ToothPickedInfomation(
		bool _bIsExisted,
		QString _labelToothFdi,
		Axis* _axisTooth)
	{
		bIsExisted = _bIsExisted;
		labelToothFdi = _labelToothFdi;
		axisTooth = _axisTooth;
	}

	bool getPickedLocatedStatus()const
	{
		if (nearAndFarPoints.size() == 2)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void clearAllInfoStatus()
	{
		bHadLocated = false;
		bIsExisted = false;
		labelToothFdi = -1;
		SAFE_DELETE(axisTooth);
		SAFE_DELETE(mesh_crown);
		clearVector(nearAndFarPoints);
		clearVector(contour_line);
		SAFE_DELETE(tooth_partical_data);
		landmarks.clear();
	}

	void clearSomeInfoStatus()
	{
		SAFE_DELETE(axisTooth);
		clearVector(nearAndFarPoints);
		clearVector(contour_line);
		SAFE_DELETE(tooth_partical_data);
		landmarks.clear();
	}

};


struct sFeaturePoint
{
	QString name_;
	QString mark_name_;
	Point3m pt_;
	int index_ = -1;
};
// 智能分割结果牙冠数据结构定义
struct CrownInfoSegmentedIntelligent
{
	std::vector<vcg::Point3f> boundryVertexsCrown; // 牙冠边界顶点
	Axis axis;// 牙冠坐标系
	std::map<QString, vcg::Point3f> landmarks;// 牙冠标记点  std::map[fdi_label,value]

	std::map<QString, sFeaturePoint> land_marks_;
};
#endif
