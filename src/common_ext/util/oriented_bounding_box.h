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

#ifndef COMMON_ORIENTED_BOUNDING_BOX
#define COMMON_ORIENTED_BOUNDING_BOX
#define INV_SQRT_TWO 0.70710678f

#include "common/ml_mesh_type.h"
#include "util/assist_geometry.h"
#include "common_ext_global.h"

using namespace std;
using namespace vcg;

class COMMON_EXT_EXPORT OrientedBoundingBox
{
public:
	OrientedBoundingBox();
	~OrientedBoundingBox();

public:
	Point3f center, axisX, axisY, axisZ;
	float halfLengthX, halfLengthY, halfLengthZ;
	Point3f position;

	vector<FFace> faceList;
	vector<Point3f> topLoop, bottomLoop;
	Point2i collideObjIndexCP;

	vector<Point3m> vertsList;

public:
	Matrix33f computeCovarianceMatrix(vector<Point3m> pVertices);
	//void jacobiSolver(Matrix33f matrix, vector<float> &eValue, vector<Point3f> &eVectors);
	bool jacbiCor(double * pMatrix, int nDim, double *pdblVects, double *pdbEigenValues, double dbEps, int nJt);
	void schmidtOrthogonal(Point3f &v0, Point3f &v1, Point3f &v2);
	void computeCenterAndHalfLength(Point3f &center, float &halfX, float &halfY, float &halfZ, vector<Point3m> vertices);
	void constructOBB_BoundBox(vector<Point3f> vertices);
	void constructOBB_BoundBox(vector<FFace> faces);
	void getCollideIndexCP(Point2i cp);
	vector<int> getAimToothVertsIndexList(CMeshO* curMesh, vector<int> vertIndexs);
	void collectVerts();
	vector<Point3m> adjAimVertsByGaussianWay(vector<int> aimAdjVertsList, CMeshO *curMesh, bool useCurCoordianteSys);
	vector<Point3m> worldToLocalCoordinates(Point3m curCenter, Point3m curAxisX, Point3m curAxisY, Point3m curAxisZ, vector<Point3m> sourceVertList, vector<int> &delList);
	vector<Point3m> localToWorldCoordinates(Point3m curCenter, Point3m curAxisX, Point3m curAxisY, Point3m curAxisZ, vector<Point3m> sourceVertList);
	bool analysisVertsGetParameters(float &fPeakValue, float &fXo, float &fYo, float &fSigmaX, float &fSigmaY, vector<Point3m> sourceVertLocalList);
	vector<Point3m> gaussianAdjustVerts(float fPeakValue, float fXo, float fYo, float fSigmaX, float fSigmaY, vector<Point3m> sourceVertLocalList);
	float computeVariance(vector<float> numList);
	float guassianFunction(float fPeakValue, float fXo, float fYo, float fSigmaX, float fSigmaY, float x, float y);
	bool ifNeedChangeAxisZDirection(Point3m curCenter, Point3m curAxisZ, vector<Point3m> sourceVertList);
};

#endif // !COMMON_ORIENTED_BOUNDING_BOX
