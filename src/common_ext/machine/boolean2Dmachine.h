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

#ifndef COMMON_BOOLEAN2DMACHINE_H
#define COMMON_BOOLEAN2DMACHINE_H

#include "common/ml_mesh_type.h"
#include "machine/delaunaymachine.h"
#include "util/assist_geometry.h"
#include "util/mesh_bounding_box.h"

#include "common_ext_global.h"

class COMMON_EXT_EXPORT Boolean2DMachine
{
public:
	Boolean2DMachine();
	Boolean2DMachine(CMeshO *_meshA, CMeshO *_meshB, vector<Point2m> cplist);
	~Boolean2DMachine();

	void initial(CMeshO *_meshA, CMeshO *_meshB, vector<Point2m> cp);
	void initial(CMeshO *_meshA, CMeshO *_meshB, vector<int> AFaceIndexs, vector<int> BFaceIndexs);
	bool boolean2DCompute(int iOperateKind, vector<FFace>& result);//------------------>2D布尔运算入口函数，输入布尔运算的操作类型，输出经布尔运算的结果面集。

	//第一步：获得操作二维平面信息，建立在其上的坐标系
	void construct2DCoordinateSystem();

	//第二步：根据输入数据，提取得到contourA，contourB
	vector<WordCircle> extractOrderedContourLoopsFrom(CMeshO *mesh, vector<int> faceIndexs);
	vector<WordCircle> getContourCircles(vector<FEdge> edgelist);
	void recognizeOutsideOrInside(vector<WordCircle> &contourCircles);
	Point3f pickOneVertOutofAllBox(WordCircle circle, int I, vector<WordCircle> contourCircles);
	int getIntersectNumWithOtherCircle(int I, Point3f A, Point3f B, vector<WordCircle> contourCircles);
	int computSumIntersectNumWithCircle(Point3f A, Point3f B, WordCircle circle);
	void adjustLoopDirect(vector<WordCircle> &contourCircles);

	//第三步：求交两组轮廓线A和B，对各自的交点标记出入属性。并进行区域划分和收集整理
	void splitAndSignContourLoop(vector<WordCircle> &A, vector<WordCircle> &B);
	void computeAllIntersectionWith(WordEdge & edge, vector<WordCircle> &AimCircle);
	void getOutAndInsideEdgeFromCircles(vector<WordCircle> circles, vector<WordCircle> &outsideCircles, vector<WordCircle> &insideCircles);
	void getOutAndInsideEdgeFromOneCircle(WordCircle circle, vector<WordCircle> &outsideCircles, vector<WordCircle> &insideCircles);

	//第四步：依据内外包含属性拆分了曲线之后，根据二维布尔运算类型进行最终轮廓确定。
	void getFinalWantedContourCircles(int iKind);
	vector<WordCircle> autoCombineAndLinkCircles(vector<WordCircle> A, vector<WordCircle> B);

	//第五步：根据外轮廓进行剖分和剔除得到最终的面结果
	bool  generateAimFaces(vector<FFace>& result);
	FFace getSuperTriangle(vector<Point3f> v);
	void GetMaximumIn(vector<Point3f> vertexs, float &maxX, float &minX, float &maxY, float &minY);

public:
	CMeshO *meshA = NULL, *meshB = NULL;//操作对象源模型A,B（输入数据）
	vector<int> sourceFaceIndexs_A, sourceFaceIndexs_B;//操作面分别在对应模型A，B上的面索引。（输入数据）
	vector<WordCircle> contourA, contourB;//经分析提取得到的可用于2D布尔运算的对象，两组面片的有序外轮廓边集。

	vector<WordCircle> AoutsideB, AinsideB, BoutsideA, BinsideA;
	vector<WordCircle> FinalContourCircles;

	vector<FEdge> mustExitEdges;
	vector<vector<FFace>> steps;
	vector<Point3m> embedVerts;

	Point3m coordinateCenterPos, coordinateAxisX, coordinateAxisY, coordinateAxisZ;
public:
	enum {A_UNION_B = 1, A_INTERSECT_B, A_MINUS_B, B_MINUS_A};
};

#endif // !COMMON_BOOLEAN2DMACHINE_H
