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

#include "oriented_bounding_box.h"

#include "vcg/complex/algorithms/update/topology.h"
#include "vcg/complex/algorithms/update/normal.h"

OrientedBoundingBox::OrientedBoundingBox()
{
}

OrientedBoundingBox::~OrientedBoundingBox()
{
}

Matrix33f OrientedBoundingBox::computeCovarianceMatrix(vector<Point3m> pVertices)
{
	Matrix33f covariance;
	vector<Point3f> pVectors;
	int numVertices = pVertices.size();

	//Compute the average x,y,z
	Point3f avg(0.0f, 0.0f, 0.0f);
	for (auto vert : pVertices) {
		pVectors.push_back(vert);
		avg += vert;
	}
	avg /= (float)numVertices;
	this->position = avg;

	for (int i = 0; i < numVertices; i++)
	{
		pVectors[i] -= avg;
	}

	//Compute the covariance
	for (int c = 0; c<3; c++)
	{
		float data[3];
		for (int r = c; r<3; r++)
		{
			covariance[c][r] = 0;

			float acc = 0.0f;
			//cov(X,Y)=E[(X-x)(Y-y)]
			for (int i = 0; i<numVertices; i++)
			{
				data[0] = pVectors[i].X();
				data[1] = pVectors[i].Y();
				data[2] = pVectors[i].Z();
				acc += data[c] * data[r];
			}

			acc /= (float)numVertices;
			covariance[c][r] = acc;
			//symmetric
			covariance[r][c] = acc;
		}

	}
	return covariance;
	}

bool OrientedBoundingBox::jacbiCor(double * pMatrix, int nDim, double *pdblVects, double *pdbEigenValues, double dbEps, int nJt)
{
	for (int i = 0; i < nDim; i++)
	{
		pdblVects[i*nDim + i] = 1.0f;
		for (int j = 0; j < nDim; j++)
		{
			if (i != j)
				pdblVects[i*nDim + j] = 0.0f;
		}
	}

	int nCount = 0;     //迭代次数
	while (1)
	{
		//在pMatrix的非对角线上找到最大元素
		double dbMax = pMatrix[1];
		int nRow = 0;
		int nCol = 1;
		for (int i = 0; i < nDim; i++)          //行
		{
			for (int j = 0; j < nDim; j++)      //列
			{
				double d = fabs(pMatrix[i*nDim + j]);

				if ((i != j) && (d> dbMax))
				{
					dbMax = d;
					nRow = i;
					nCol = j;
				}
			}
		}

		if (dbMax < dbEps)     //精度符合要求
			break;

		if (nCount > nJt)       //迭代次数超过限制
			break;

		nCount++;

		double dbApp = pMatrix[nRow*nDim + nRow];
		double dbApq = pMatrix[nRow*nDim + nCol];
		double dbAqq = pMatrix[nCol*nDim + nCol];

		//计算旋转角度
		double dbAngle = 0.5*atan2(-2 * dbApq, dbAqq - dbApp);
		double dbSinTheta = sin(dbAngle);
		double dbCosTheta = cos(dbAngle);
		double dbSin2Theta = sin(2 * dbAngle);
		double dbCos2Theta = cos(2 * dbAngle);

		pMatrix[nRow*nDim + nRow] = dbApp*dbCosTheta*dbCosTheta +
			dbAqq*dbSinTheta*dbSinTheta + 2 * dbApq*dbCosTheta*dbSinTheta;
		pMatrix[nCol*nDim + nCol] = dbApp*dbSinTheta*dbSinTheta +
			dbAqq*dbCosTheta*dbCosTheta - 2 * dbApq*dbCosTheta*dbSinTheta;
		pMatrix[nRow*nDim + nCol] = 0.5*(dbAqq - dbApp)*dbSin2Theta + dbApq*dbCos2Theta;
		pMatrix[nCol*nDim + nRow] = pMatrix[nRow*nDim + nCol];

		for (int i = 0; i < nDim; i++)
		{
			if ((i != nCol) && (i != nRow))
			{
				int u = i*nDim + nRow;  //p
				int w = i*nDim + nCol;  //q
				dbMax = pMatrix[u];
				pMatrix[u] = pMatrix[w] * dbSinTheta + dbMax*dbCosTheta;
				pMatrix[w] = pMatrix[w] * dbCosTheta - dbMax*dbSinTheta;
			}
		}

		for (int j = 0; j < nDim; j++)
		{
			if ((j != nCol) && (j != nRow))
			{
				int u = nRow*nDim + j;  //p
				int w = nCol*nDim + j;  //q
				dbMax = pMatrix[u];
				pMatrix[u] = pMatrix[w] * dbSinTheta + dbMax*dbCosTheta;
				pMatrix[w] = pMatrix[w] * dbCosTheta - dbMax*dbSinTheta;
			}
		}

		//计算特征向量
		for (int i = 0; i < nDim; i++)
		{
			int u = i*nDim + nRow;      //p
			int w = i*nDim + nCol;      //q
			dbMax = pdblVects[u];
			pdblVects[u] = pdblVects[w] * dbSinTheta + dbMax*dbCosTheta;
			pdblVects[w] = pdblVects[w] * dbCosTheta - dbMax*dbSinTheta;
		}

	}

	//对特征值进行排序以及重新排列特征向量,特征值即pMatrix主对角线上的元素
	std::map<double, int> mapEigen;
	for (int i = 0; i < nDim; i++)
	{
		pdbEigenValues[i] = pMatrix[i*nDim + i];

		mapEigen.insert(make_pair(pdbEigenValues[i], i));
	}

	double *pdbTmpVec = new double[nDim*nDim];
	std::map<double, int>::reverse_iterator iter = mapEigen.rbegin();
	for (int j = 0; iter != mapEigen.rend(), j < nDim; ++iter, ++j)
	{
		for (int i = 0; i < nDim; i++)
		{
			pdbTmpVec[i*nDim + j] = pdblVects[i*nDim + iter->second];
		}

		//特征值重新排列
		pdbEigenValues[j] = iter->first;
	}

	//设定正负号
	for (int i = 0; i < nDim; i++)
	{
		double dSumVec = 0;
		for (int j = 0; j < nDim; j++)
			dSumVec += pdbTmpVec[j * nDim + i];
		if (dSumVec<0)
		{
			for (int j = 0; j < nDim; j++)
				pdbTmpVec[j * nDim + i] *= -1;
		}
	}

	memcpy(pdblVects, pdbTmpVec, sizeof(double)*nDim*nDim);
	delete[]pdbTmpVec;

	return 1;
}

void OrientedBoundingBox::schmidtOrthogonal(Point3f &v0, Point3f &v1, Point3f &v2)
{
	v0 = v0.Normalize();

	//v1-=(v1*v0)*v0;
	v1 -= v0 * (v1 * v0);
	v1 = v1.Normalize();
	v2 = v0 ^ v1;
	v2 = v2.Normalize();
	return;
}

void OrientedBoundingBox::computeCenterAndHalfLength(Point3f &center, float &halfX, float &halfY, float &halfZ, vector<Point3m> vertices)
{
	Point3f minExtents(FLT_MAX, FLT_MAX, FLT_MAX);
	Point3f maxExtents(FLT_MIN, FLT_MIN, FLT_MIN);

	for (int index = 0; index < vertices.size(); index++)
	{
		Point3f vec = vertices[index];
		Point3f displacement = vec - position;

		minExtents.X() = fminf(minExtents.X(), displacement * axisX);
		minExtents.Y() = fminf(minExtents.Y(), displacement * axisY);
		minExtents.Z() = fminf(minExtents.Z(), displacement * axisZ);

		maxExtents.X() = fmaxf(maxExtents.X(), displacement * axisX);
		maxExtents.Y() = fmaxf(maxExtents.Y(), displacement * axisY);
		maxExtents.Z() = fmaxf(maxExtents.Z(), displacement * axisZ);
	}

	//offset = (maxExtents-minExtents)/2.0f+minExtents
	Point3f offset = (maxExtents - minExtents) / 2.0f + minExtents;
	position += axisX * offset.X();
	position += axisY * offset.Y();
	position += axisZ * offset.Z();
	center = position;
	//半长度
	halfX = (maxExtents.X() - minExtents.X()) / 2.0f;
	halfY = (maxExtents.Y() - minExtents.Y()) / 2.0f;
	halfZ = (maxExtents.Z() - minExtents.Z()) / 2.0f;
}

void OrientedBoundingBox::constructOBB_BoundBox(vector<Point3f> vertices)
{
	this->vertsList = vertices;
	Matrix33f covarianceMatrix = this->computeCovarianceMatrix(vertices);
	vector<float> eValues;
	vector<Point3f> eVectors;
	//this->jacobiSolver(covarianceMatrix, eValues, eVectors);
	/*this->axisX = eVectors[0];
	this->axisY = eVectors[1];
	this->axisZ = eVectors[2];*/

	//求雅克比特征向量
	double dbEps = 0.000001f;
	int nJt = 100;
	int nDim = 3;
	double *pMatrix = new double[nDim * nDim];
	double *pdblVects = new double[nDim * nDim];
	double *pdbEigenValues = new double[nDim];
	for (int i = 0; i < nDim; ++i)
	{
		for (int j = 0; j < nDim; ++j)
		{
			pMatrix[i * nDim + j] = covarianceMatrix[i][j];
		}
	}
	this->jacbiCor(pMatrix, nDim, pdblVects, pdbEigenValues, dbEps, nJt);
	this->axisX = Point3m(pdblVects[0], pdblVects[3], pdblVects[6]);
	this->axisY = Point3m(pdblVects[1], pdblVects[4], pdblVects[7]);
	this->axisZ = Point3m(pdblVects[2], pdblVects[5], pdblVects[8]);
	delete[] pMatrix;
	delete[] pdblVects;
	delete[] pdbEigenValues;

	this->schmidtOrthogonal(axisX, axisY, axisZ);
	this->computeCenterAndHalfLength(this->center, this->halfLengthX, this->halfLengthY, this->halfLengthZ, vertices);

	this->topLoop.push_back(center + (axisX * -halfLengthX + axisY * halfLengthY + axisZ * -halfLengthZ));
	this->topLoop.push_back(center + (axisX * halfLengthX + axisY * halfLengthY + axisZ * -halfLengthZ));
	this->topLoop.push_back(center + (axisX * halfLengthX + axisY * halfLengthY + axisZ * halfLengthZ));
	this->topLoop.push_back(center + (axisX * -halfLengthX + axisY * halfLengthY + axisZ * halfLengthZ));

	this->bottomLoop.push_back(center + (axisX * -halfLengthX - axisY * halfLengthY + axisZ * -halfLengthZ));
	this->bottomLoop.push_back(center + (axisX * halfLengthX - axisY * halfLengthY + axisZ * -halfLengthZ));
	this->bottomLoop.push_back(center + (axisX * halfLengthX - axisY * halfLengthY + axisZ * halfLengthZ));
	this->bottomLoop.push_back(center + (axisX * -halfLengthX - axisY * halfLengthY + axisZ * halfLengthZ));
}

void OrientedBoundingBox::constructOBB_BoundBox(vector<FFace> faces)
{
	bool exited;
	vector<Point3f> vertList;
	for (auto face : faces)
	{
		exited = false;
		for (auto vert : vertList)
		{
			if (vert == face.v1)
			{
				exited = true;
				break;
			}
		}
		if (!exited)
		{
			vertList.push_back(face.v1);
		}

		exited = false;
		for (auto vert : vertList)
		{
			if (vert == face.v2)
			{
				exited = true;
				break;
			}
		}
		if (!exited)
		{
			vertList.push_back(face.v2);
		}

		exited = false;
		for (auto vert : vertList)
		{
			if (vert == face.v3)
			{
				exited = true;
				break;
			}
		}
		if (!exited)
		{
			vertList.push_back(face.v3);
		}
	}

	this->faceList = faces;
	this->constructOBB_BoundBox(vertList);
	return;
}

void OrientedBoundingBox::getCollideIndexCP(Point2i cp)
{
	this->collideObjIndexCP = cp;
}

vector<int> OrientedBoundingBox::getAimToothVertsIndexList(CMeshO* curMesh, vector<int> vertIndexs)
{
	vector<int> result;
	for (auto vert : this->vertsList)
	{
		for (auto i : vertIndexs)
		{
			if (!curMesh->vert[i].IsD())
			{
				Point3m compareVert = curMesh->vert[i].P();
				Point3m minus = compareVert - vert;
				//if (curMesh->vert[i].P() == vert)
				if (abs(minus.X()) < 1e-3
					&&
					abs(minus.Y()) < 1e-3
					&&
					abs(minus.Z()) < 1e-3)
				{
					result.push_back(i);
					break;
				}
			}
		}
	}
	return result;
}

void OrientedBoundingBox::collectVerts()
{
	this->vertsList.clear();
	bool exited = false;
	for (auto face : this->faceList)
	{
		exited = false;
		for (auto vert : this->vertsList)
		{
			if (vert == face.v1)
			{
				exited = true;
				break;
			}
		}
		if (!exited)
		{
			this->vertsList.push_back(face.v1);
		}

		exited = false;
		for (auto vert : this->vertsList)
		{
			if (vert == face.v2)
			{
				exited = true;
				break;
			}
		}
		if (!exited)
		{
			this->vertsList.push_back(face.v2);
		}

		exited = false;
		for (auto vert : this->vertsList)
		{
			if (vert == face.v3)
			{
				exited = true;
				break;
			}
		}
		if (!exited)
		{
			this->vertsList.push_back(face.v3);
		}
	}

	return;
}

vector<Point3m> OrientedBoundingBox::adjAimVertsByGaussianWay(vector<int> aimAdjVertsList, CMeshO *curMesh, bool useCurCoordianteSys)
{
	vector<Point3m> sourceVertList, aimVertList;
	vector<Point3m> sourceVertLocalList, aimVertLocalList;
	//1.收集sourceVertList
	for (auto index : aimAdjVertsList)
	{
		sourceVertList.push_back(curMesh->vert[index].P());
	}
	//2.转换sourceVertList到sourceVertLocalList
	Point3m curCenter, curAxisX, curAxisY, curAxisZ;
	curCenter = this->center;
	curAxisX = this->axisX;
	curAxisY = this->axisY;
	useCurCoordianteSys = this->ifNeedChangeAxisZDirection(curCenter, this->axisZ, sourceVertList);
	if (!useCurCoordianteSys)
	{
		curAxisZ = this->axisZ * -1.0f;
	}
	else
	{
		curAxisZ = this->axisZ;
	}
	vector<int> delList;
	sourceVertLocalList = this->worldToLocalCoordinates(curCenter, curAxisX, curAxisY, curAxisZ, sourceVertList, delList);
	////2.2剔除所有sourceVertLocalListZ值小于0的点...
	if (!delList.empty())
	{
		vector<int> newAdjVertIndexlist;

		for (int i = 0; i < aimAdjVertsList.size(); ++i)
		{
			bool needDel = false;
			for (auto index : delList)
			{
				if (i == index)
				{
					needDel = true;
					break;
				}
			}
			if (!needDel)
			{
				newAdjVertIndexlist.push_back(aimAdjVertsList[i]);
			}
		}
		aimAdjVertsList.clear();
		aimAdjVertsList = newAdjVertIndexlist;
		newAdjVertIndexlist.clear();
	}
	//3.分析sourceVertLocalList得出峰值，中心点xy坐标，xy方差值
	float fPeakValue, fXo, fYo, fSigmaX, fSigmaY;
	this->analysisVertsGetParameters(fPeakValue, fXo, fYo, fSigmaX, fSigmaY, sourceVertLocalList);
	//4.根据分析的参数构建特殊高斯调整方程，带入sourceVertLocalList，获得aimVertLocalList
	aimVertLocalList = this->gaussianAdjustVerts(fPeakValue, fXo, fYo, fSigmaX, fSigmaY, sourceVertLocalList);
	//5.将aimVertLocalList进行坐标转化得到最终的调整结果
	aimVertList = this->localToWorldCoordinates(curCenter, curAxisX, curAxisY, curAxisZ, aimVertLocalList);
	//6将aimVertList按照赋值给原始网格
	if (aimVertList.size() == aimAdjVertsList.size())
	{
		int curIndex;
		for (int i = 0; i < aimVertList.size(); ++i)
		{
			curIndex = aimAdjVertsList[i];
			curMesh->vert[curIndex].P() = aimVertList[i];
		}
		vcg::tri::Allocator<CMeshO>::CompactEveryVector(*curMesh);

		//update topology
		assert(tri::HasFFAdjacency(*curMesh) == false);
		curMesh->face.EnableFFAdjacency();
		assert(tri::HasFFAdjacency(*curMesh) == true);

		assert(tri::HasVFAdjacency(*curMesh) == false);
		curMesh->vert.EnableVFAdjacency();
		curMesh->face.EnableVFAdjacency();
		assert(tri::HasVFAdjacency(*curMesh) == true);
		vcg::tri::UpdateTopology<CMeshO>::VertexFace(*curMesh);
		vcg::tri::UpdateTopology<CMeshO>::FaceFace(*curMesh);

		// update Normals
		vcg::tri::UpdateNormal<CMeshO>::PerVertexNormalizedPerFace(*curMesh);
		vcg::tri::UpdateNormal<CMeshO>::PerFaceNormalized(*curMesh);
	}

	return aimVertList;
}

vector<Point3m> OrientedBoundingBox::worldToLocalCoordinates(Point3m curCenter, Point3m curAxisX, Point3m curAxisY, Point3m curAxisZ, vector<Point3m> sourceVertList, vector<int> &delList)
{
	vector<Point3m> result;
	float x, y, z;
	Point3m tempVector;
	int i = 0;
	for (auto vert : sourceVertList)
	{
		tempVector = vert - curCenter;
		x = tempVector * curAxisX;
		y = tempVector * curAxisY;
		z = tempVector * curAxisZ;
		if (z < 0)
		{
			delList.push_back(i);
		}
		else
		{
			result.push_back(Point3m(x, y, z));
		}

		i++;
	}
	return result;
}

vector<Point3m> OrientedBoundingBox::localToWorldCoordinates(Point3m curCenter, Point3m curAxisX, Point3m curAxisY, Point3m curAxisZ, vector<Point3m> sourceVertList)
{
	vector<Point3m> result;
	for (auto vert : sourceVertList)
	{
		result.push_back(curCenter + curAxisX * vert.X() + curAxisY * vert.Y() + curAxisZ * vert.Z());
	}
	return result;
}

bool OrientedBoundingBox::analysisVertsGetParameters(float &fPeakValue, float &fXo, float &fYo, float &fSigmaX, float &fSigmaY, vector<Point3m> sourceVertLocalList)
{
	if (sourceVertLocalList.empty())
	{
		return false;
	}
	//获得峰值和峰值点对应的x,y值
	fPeakValue = sourceVertLocalList[0].Z();
	fXo = sourceVertLocalList[0].X();
	fYo = sourceVertLocalList[0].Y();
	for (auto vert : sourceVertLocalList)
	{
		if (vert.Z() > fPeakValue)
		{
			fPeakValue = vert.Z();
			fXo = vert.X();
			fYo = vert.Y();
		}
	}
	//分别计算x，y集的方差
	vector<float> tempXList, tempYList;
	for (auto vert : sourceVertLocalList)
	{
		tempXList.push_back(vert.X());
		tempYList.push_back(vert.Y());
	}
	fSigmaX = computeVariance(tempXList);
	fSigmaY = computeVariance(tempYList);
	tempXList.clear();
	tempYList.clear();
	return true;
}

float OrientedBoundingBox::computeVariance(vector<float> numList)
{
	float aveValue = 0.0f;
	for (auto n : numList)
	{
		aveValue += n;
	}
	aveValue /= (float)(numList.size());

	float variance = 0.0f;
	for (auto n : numList)
	{
		variance += (n - aveValue) * (n - aveValue);
	}
	variance /= (float)(numList.size());

	return variance;
}

vector<Point3m> OrientedBoundingBox::gaussianAdjustVerts(float fPeakValue, float fXo, float fYo, float fSigmaX, float fSigmaY, vector<Point3m> sourceVertLocalList)
{
	vector<Point3m> result;
	for (auto vert : sourceVertLocalList)
	{
		//float tempZ = this->guassianFunction(fPeakValue, fXo, fYo, fSigmaX, fSigmaY, vert.X(), vert.Y());
		float tempZ = -0.0001;
		result.push_back(Point3m(vert.X(), vert.Y(), tempZ));
	}
	return result;
}

float OrientedBoundingBox::guassianFunction(float fPeakValue, float fXo, float fYo, float fSigmaX, float fSigmaY, float x, float y)
{
	if (fSigmaX == 0 || fSigmaY == 0)
	{
		return 0;
	}
	float result;
	float fExponent1, fExponent2;
	fExponent1 = -1 * (pow(x - fXo, 2) / (2 * pow(fSigmaX, 2) + pow(y - fYo, 2) / (2 * pow(fSigmaY, 2))));
	fExponent2 = -1 * (pow(x - fXo, 2) / (2 * pow(fSigmaX * 20.0f, 2) + pow(y - fYo, 2) / (2 * pow(fSigmaY * 20.0f, 2))));
	result = fPeakValue * expf(fExponent1) * (1 - (2 - exp(fExponent2)));

	return result;
}

bool OrientedBoundingBox::ifNeedChangeAxisZDirection(Point3m curCenter, Point3m curAxisZ, vector<Point3m> sourceVertList)
{
	float sameDirect = 0, diffDirect = 0, z;
	Point3m tempVector;
	for (auto vert : sourceVertList)
	{
		tempVector = vert - curCenter;
		z = tempVector * curAxisZ;
		if (z < 0)
		{
			diffDirect += 1;
		}
		else
		{
			sameDirect += 1;
		}
	}
	if (sameDirect > diffDirect)
	{
		return true;
	}
	else
	{
		return false;
	}
}
