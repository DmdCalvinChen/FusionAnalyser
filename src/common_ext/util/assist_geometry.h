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

#ifndef ASSISTGEOMETRY_H
#define ASSISTGEOMETRY_H
#define PI 3.141592653
#include "common/ml_mesh_type.h"
using namespace std;
using namespace vcg;
enum SPACE_ENUM { ON_LEFT, ON_MIXED, ON_RIGHT };

struct HexaVec {
	float fVX, fVY, fVZ;
	float fA, fB, fG;
	float fZX = 1, fZY = 1, fZZ = 1;

	HexaVec()
	{
		fVX = 0; fVY = 0; fVZ = 0;
		fA = 0;  fB = 0;  fG = 0;
		fZX = 1; fZY = 1; fZZ = 1;
	}
	HexaVec(float x, float y, float z, float a, float b, float g) {
		fVX = x; fVY = y; fVZ = z;
		fA = a;  fB = b;  fG = g;
		fZX = 1; fZY = 1; fZZ = 1;
	}
	HexaVec(float x, float y, float z, float a, float b, float g, float zx, float zy, float zz)
	{
		fVX = x; fVY = y; fVZ = z;
		fA = a;  fB = b;  fG = g;
		fZX = zx; fZY = zy; fZZ = zz;
	}
	HexaVec(Point3m move, Point3m rotate) {
		fVX = move.X(); fVY = move.Y(); fVZ = move.Z();
		fA = rotate.X();  fB = rotate.Y();  fG = rotate.Z();
		fZX = 1; fZY = 1; fZZ = 1;
	}
	HexaVec(Point3m move, Point3m rotate, Point3m zoom) {
		fVX = move.X(); fVY = move.Y(); fVZ = move.Z();
		fA = rotate.X();  fB = rotate.Y();  fG = rotate.Z();
		fZX = zoom.X(); fZY = zoom.Y(); fZZ = zoom.Z();
	}
	HexaVec(const HexaVec &e)
	{
		this->fVX = e.fVX;
		this->fVY = e.fVY;
		this->fVZ = e.fVZ;
		this->fA = e.fA;
		this->fB = e.fB;
		this->fG = e.fG;
		this->fZX = e.fZX;
		this->fZY = e.fZY;
		this->fZZ = e.fZZ;
	}
	void Inital(float x, float y, float z, float a, float b, float g)
	{
		fVX = x; fVY = y; fVZ = z;
		fA = a;  fB = b;  fG = g;
		fZX = 1; fZY = 1; fZZ = 1;
	}
	bool IsZerO()
	{
		if (fVX == 0 && fVY == 0 && fVZ == 0 && fA == 0 && fB == 0 && fG == 0)
			return true;
		else
			return false;
	}
	void setZoomPart(Point3m _value)
	{
		this->fZX = _value.X();
		this->fZY = _value.Y();
		this->fZZ = _value.Z();
	}

	HexaVec& operator=(const  HexaVec& e)
	{
		this->fVX = e.fVX;
		this->fVY = e.fVY;
		this->fVZ = e.fVZ;
		this->fA = e.fA;
		this->fB = e.fB;
		this->fG = e.fG;
		this->fZX = e.fZX;
		this->fZY = e.fZY;
		this->fZZ = e.fZZ;
		return *this;
	}

	HexaVec& operator+(const HexaVec& e)
	{
		HexaVec result;
		result.fVX = this->fVX + e.fVX;
		result.fVY = this->fVY + e.fVY;
		result.fVZ = this->fVZ + e.fVZ;
		result.fA = this->fA + e.fA;
		result.fB = this->fB + e.fB;
		result.fG = this->fG + e.fG;
		return result;
	}

	HexaVec& operator-(const HexaVec& e)
	{
		HexaVec result;
		result.fVX = this->fVX - e.fVX;
		result.fVY = this->fVY - e.fVY;
		result.fVZ = this->fVZ - e.fVZ;
		result.fA = this->fA - e.fA;
		result.fB = this->fB - e.fB;
		result.fG = this->fG - e.fG;
		return result;
	}

	HexaVec& operator/(const float e)
	{
		this->fVX = this->fVX / e;
		this->fVY = this->fVY / e;
		this->fVZ = this->fVZ / e;
		this->fA = this->fA / e;
		this->fB = this->fB / e;
		this->fG = this->fG / e;
		return *this;
	}

	HexaVec& operator*(const float e)
	{
		this->fVX = this->fVX * e;
		this->fVY = this->fVY * e;
		this->fVZ = this->fVZ * e;
		this->fA = this->fA * e;
		this->fB = this->fB * e;
		this->fG = this->fG * e;
		return *this;
	}

	bool operator==(const HexaVec& e)
	{
		if (e.fVX == this->fVX &&
			e.fVY == this->fVY &&
			e.fVZ == this->fVZ &&
			e.fA == this->fA &&
			e.fB == this->fB &&
			e.fG == this->fG &&
			e.fZX == this->fZX &&
			e.fZY == this->fZY &&
			e.fZZ == this->fZZ)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool operator!=(const HexaVec& e)
	{
		if (e.fVX != this->fVX ||
			e.fVY != this->fVY ||
			e.fVZ != this->fVZ ||
			e.fA != this->fA ||
			e.fB != this->fB ||
			e.fG != this->fG ||
			e.fZX != this->fZX ||
			e.fZY != this->fZY ||
			e.fZZ != this->fZZ)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};

struct Axis
{
	Axis()
	{
		this->centerPoint = Point3m(0, 0, 0);
		this->axisXVector = Point3m(1, 0, 0);
		this->axisYVector = Point3m(0, 1, 0);
		this->axisZVector = Point3m(0, 0, 1);
	}
	Axis(Point3m _center, Point3m axisX, Point3m axisY, Point3m axisZ)
	{
		this->centerPoint = _center;
		this->axisXVector = axisX.Normalize();
		this->axisYVector = axisY.Normalize();
		this->axisZVector = axisZ.Normalize();
	}

	void setCenter(const Point3m& center)
	{
		this->centerPoint = center;
	}

	void setVector(const Point3m& axisX, const Point3m& axisY, const Point3m& axisZ)
	{
		this->axisXVector = axisX;
		this->axisYVector = axisY;
		this->axisZVector = axisZ;
	}

	Point4m computeQuaternion(Axis aimAxis)
	{
		Point3m sCenter, sX, sY, sZ;
		Point3m eCenter, eX, eY, eZ;
		float x1, x2, x3, y1, y2, y3, z1, z2, z3;

		sX = this->axisXVector;
		sY = this->axisYVector;
		sZ = this->axisZVector;
		eX = aimAxis.axisXVector;
		eY = aimAxis.axisYVector;
		eZ = aimAxis.axisZVector;

		x1 = eX * sX;
		x2 = eY * sX;
		x3 = eZ * sX;
		y1 = eX * sY;
		y2 = eY * sY;
		y3 = eZ * sY;
		z1 = eX * sZ;
		z2 = eY * sZ;
		z3 = eZ * sZ;

		float q1 = sqrtf(x1 + y2 + z3 + 1) / 2.0f;
		float q2 = sqrtf(x1 - y2 - z3 + 1) / 2.0f;
		float q3 = sqrtf(y2 - x1 - z3 + 1) / 2.0f;
		float q4 = sqrtf(z3 - x1 - y2 + 1) / 2.0f;
		if (q2 * (y3 - z2) < 0)
		{
			q2 *= -1.0f;
		}
		if (q3 * (z1 - x3) < 0)
		{
			q3 *= -1.0f;
		}
		if (q4 * (x2 - y1) < 0)
		{
			q4 *= -1.0f;
		}

		if (isnan(q1))
			q1 = 0.0f;

		if (isnan(q2))
			q2 = 0.0f;

		if (isnan(q3))
			q3 = 0.0f;

		if (isnan(q4))
			q4 = 0.0f;

		return Point4m(q1, q2, q3, q4);
	}

	void computeTransformVectors(Axis aimAxis, Point3m &move, Point3m &rotate)
	{
		vcg::Matrix44f transformValue, rotMatLocal;
		rotMatLocal.SetIdentity();

		Point3m sCenter, sX, sY, sZ;
		Point3m eCenter, eX, eY, eZ;
		float l1, l2, l3, m1, m2, m3, n1, n2, n3;
		float A, B, C; // Rotation angles for x, y, z axes
		float a, b, c; // Translation amounts for x, y, z axes
		float F = PI / 180.0f, D = PI;

		sX = this->axisXVector;
		sY = this->axisYVector;
		sZ = this->axisZVector;
		eX = aimAxis.axisXVector;
		eY = aimAxis.axisYVector;
		eZ = aimAxis.axisZVector;
		Point3m rotateEX = sX;

		l1 = eX * sX;
		l2 = eY * sX;
		l3 = eZ * sX;
		m1 = eX * sY;
		m2 = eY * sY;
		m3 = eZ * sY;
		n1 = eX * sZ;
		n2 = eY * sZ;
		n3 = eZ * sZ;

		sCenter = this->centerPoint;
		eCenter = aimAxis.centerPoint;

		Point3m translateV = eCenter - sCenter;
		a = translateV * sX;
		if (abs(a) < 0.001)
		{
			a = 0;
		}
		b = translateV * sY;
		if (abs(b) < 0.001)
		{
			b = 0;
		}
		c = translateV * sZ;
		if (abs(c) < 0.001)
		{
			c = 0;
		}

		if (n3 == 0)
		{
			n3 = 0.00001f;
		}

		float m, n; // Projection of endZAxis on W plane, components on sY and sZ axes
		m = m3;
		n = n3;
		A = atan(m / n);
		A = A / F;
		if (abs(A) < 0.001)
		{
			A = 0;
		}
		transformValue.SetRotateDeg(-A, sX);
		rotMatLocal *= transformValue;

		if (math::IsNAN(A))
		{
			bool stop = true;
		}

		if ((l3 - 1.0f) > 0.000000001f)
		{
			l3 = 1.0f;
		}
		else if ((-1.0f - l3) > 0.000000001f)
		{
			l3 = -1.0f;
		}
		B = acos(l3);
		B /= F;
		if (eZ * sZ < 0)
		{
			B += 90;
			B *= -1;
		}
		else
		{
			B -= 90;
		}

		if (isnan(B))
		{
			bool stop = true;
		}
		if (abs(B) < 0.001)
		{
			B = 0;
		}
		transformValue.SetRotateDeg(-B, sY);
		rotMatLocal *= transformValue;

		rotateEX = rotMatLocal * rotateEX;
		rotateEX = rotateEX.Normalize();
		float temp = rotateEX * eX;

		/*if (abs(temp - 1) < 1e-5)
		{
			temp = 1;
		}*/
		if (temp > 0 && abs(1 - temp) < 0.001)
		{
			temp = 1;
		}
		else if (temp < 0 && abs(-1 - temp) < 0.001)
		{
			temp = -1;
		}
		C = acos(temp);
		C /= F;
		if (isnan(C))
		{
			C = 0;
		}
		if ((rotateEX ^ eX) * eZ > 0)
		{
			C *= -1;
		}

		if (math::IsNAN(a) || math::IsNAN(b) || math::IsNAN(c))
		{
			bool stop = 0;
		}
		if (math::IsNAN(A) || math::IsNAN(B) || math::IsNAN(C))
		{
			bool stop = 0;
		}
		move = Point3m(a, b, c);
		rotate = Point3m(-A, -B, -C);
	}

	void rotate(float degree, Point3m axis)
	{
		vcg::Matrix44f transMat;
		vcg::Matrix44f transform, transformInverse;
		transform.SetIdentity();
		transform.SetTranslate(this->centerPoint);
		transformInverse.SetTranslate(-this->centerPoint);

		vcg::Matrix44f transformValue, rotMatLocal;
		transMat.SetIdentity();
		rotMatLocal.SetIdentity();

		transformValue.SetRotateDeg(degree, axis);
		rotMatLocal *= transformValue;

		transform *= rotMatLocal * transMat;
		transform *= transformInverse;

		Point3m axisVert_Center, axisVert_X, axisVert_Y, axisVert_Z;
		axisVert_Center = this->centerPoint;
		axisVert_X = this->centerPoint + this->axisXVector;
		axisVert_Y = this->centerPoint + this->axisYVector;
		axisVert_Z = this->centerPoint + this->axisZVector;

		axisVert_Center = transform * axisVert_Center;
		axisVert_X = transform * axisVert_X;
		axisVert_Y = transform * axisVert_Y;
		axisVert_Z = transform * axisVert_Z;

		this->centerPoint = axisVert_Center;
		this->axisXVector = (axisVert_X - axisVert_Center).Normalize();
		this->axisYVector = (axisVert_Y - axisVert_Center).Normalize();
		this->axisZVector = (axisVert_Z - axisVert_Center).Normalize();
	}

	Axis transformAccording(HexaVec info)
	{
        vcg::Matrix44f transform = constructTransformMatrix(info);

		Point3m axisVert_Center = this->centerPoint;
		Point3m axisVert_X = this->centerPoint + this->axisXVector;
		Point3m axisVert_Y = this->centerPoint + this->axisYVector;
		Point3m axisVert_Z = this->centerPoint + this->axisZVector;

		axisVert_Center = transform * axisVert_Center;
		axisVert_X = transform * axisVert_X;
		axisVert_Y = transform * axisVert_Y;
		axisVert_Z = transform * axisVert_Z;

		Axis result;
		result.centerPoint = axisVert_Center;
		result.axisXVector = (axisVert_X - axisVert_Center).Normalize();
		result.axisYVector = (axisVert_Y - axisVert_Center).Normalize();
		result.axisZVector = (axisVert_Z - axisVert_Center).Normalize();
		return result;
	}

	Point3m getTransformVertAccording(HexaVec info, Point3m aimVert)
	{
		vcg::Matrix44f transform = constructTransformMatrix(info);
		return transform * aimVert;
	}

	vcg::Matrix44f constructTransformMatrix(HexaVec info)
	{
		vcg::Matrix44f transform, transformInverse;
		transform.SetIdentity();
		transform.SetTranslate(this->centerPoint);
		transformInverse.SetTranslate(-this->centerPoint);

		vcg::Matrix44f transformValue, transMat, rotMat;
		transMat.SetIdentity();
		rotMat.SetIdentity();

		transformValue.SetTranslate(this->axisXVector * info.fVX);
		transMat *= transformValue;
		transformValue.SetTranslate(this->axisYVector * info.fVY);
		transMat *= transformValue;
		transformValue.SetTranslate(this->axisZVector * info.fVZ);
		transMat *= transformValue;

		transformValue.SetRotateDeg(info.fA, this->axisXVector);
		rotMat *= transformValue;
		transformValue.SetRotateDeg(info.fB, this->axisYVector);
		rotMat *= transformValue;
		transformValue.SetRotateDeg(info.fG, this->axisZVector);
		rotMat *= transformValue;

		Matrix44f zoom_matrix;
		zoom_matrix.SetIdentity();
		if (info.fZX != 1 ||
			info.fZY != 1 ||
			info.fZZ != 1)
		{
			zoom_matrix = getZoomMatrixAccordZoomVec(info.fZX, info.fZY, info.fZZ);
		}

		transform *= transMat * rotMat * zoom_matrix;
		transform *= transformInverse;

 		return transform;
	}

	vcg::Matrix44f getZoomMatrixAccordZoomVec(const float& _x, const float& _y, const float& _z)
	{
		Matrix44f zoomMatLocal;
		zoomMatLocal.SetIdentity();
		zoomMatLocal *= this->setZoomDeg(_x, axisXVector);
		zoomMatLocal *= this->setZoomDeg(_y, axisYVector);
		zoomMatLocal *= this->setZoomDeg(_z, axisZVector);
		return zoomMatLocal;
	}

	vcg::Matrix44f setZoomDeg(float value, Point3m axis)
	{
		float rotateValue;
		Point3m A, B, rotateAxis;
		A = axis.Normalize();
		B = Point3m(1, 0, 0);
		rotateAxis = (B ^ A).Normalize();
		float dotMulit = A * B;
		if (abs(dotMulit) > 1)
		{
			if (dotMulit > 0)
			{
				dotMulit = 1.0f;
			}
			else
			{
				dotMulit = -1.0f;
			}
		}
		rotateValue = acosf(dotMulit);
		rotateValue = (rotateValue * 180.0f) / PI;

		vcg::Matrix44f rotMatLocal, rotMatReturn, zoomLocal;
		rotMatLocal.SetRotateDeg(rotateValue, rotateAxis);
		rotMatReturn.SetRotateDeg(-rotateValue, rotateAxis);
		zoomLocal.SetScale(value, 1, 1);
		return rotMatLocal * zoomLocal * rotMatReturn;
	}

	void keepThreeDecimalPlaces()
	{
		int h;
		h = axisXVector.X() * 1000 / (int)1;
		axisXVector.X() = h / 1000.0f;
		h = axisXVector.Y() * 1000 / (int)1;
		axisXVector.Y() = h / 1000.0f;
		h = axisXVector.Z() * 1000 / (int)1;
		axisXVector.Z() = h / 1000.0f;

		h = axisYVector.X() * 1000 / (int)1;
		axisYVector.X() = h / 1000.0f;
		h = axisYVector.Y() * 1000 / (int)1;
		axisYVector.Y() = h / 1000.0f;
		h = axisYVector.Z() * 1000 / (int)1;
		axisYVector.Z() = h / 1000.0f;

		h = axisZVector.X() * 1000 / (int)1;
		axisZVector.X() = h / 1000.0f;
		h = axisZVector.Y() * 1000 / (int)1;
		axisZVector.Y() = h / 1000.0f;
		h = axisZVector.Z() * 1000 / (int)1;
		axisZVector.Z() = h / 1000.0f;
	}

	Point3m           axisXVector;
	Point3m           axisYVector;
	Point3m           axisZVector;
	Point3m           centerPoint;
};

struct FPoint {
public:
	Point3f point;
	int n;
	bool bSalient;
	FPoint() {}
	FPoint(Point3f p, int _n)
	{
		this->point = p;
		this->n = _n;
		bSalient = false;
	}
	FPoint(Point3f _p, int _n, bool _state)
	{
		this->point = _p;
		this->n = _n;
		bSalient = _state;
	}
	void Inital(Point3f p, int _n)
	{
		this->point = p;
		this->n = _n;
		bSalient = false;
	}

	bool almost_equal(const FPoint &p)
	{
		return almost_equalNum(point.X(), p.point.X()) && almost_equalNum(point.X(), p.point.X());
	}

	bool almost_equalNum(const float x, const float y, int ulp = 2)
	{
		// the machine epsilon has to be scaled to the magnitude of the values used
		// and multiplied by the desired precision in ULPs (units in the last place)
		return std::abs(x - y) <= std::numeric_limits<float>::epsilon() * std::abs(x + y) * static_cast<float>(ulp)
			// unless the result is subnormal
			|| std::abs(x - y) < std::numeric_limits<float>::min();
	}

	bool operator==(const FPoint p)
	{
		if (this->n == p.n && this->bSalient == p.bSalient)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	FPoint& operator=(const FPoint &p)
	{
		this->point = p.point;
		this->n = p.n;
		this->bSalient = p.bSalient;
		return *this;
	}
};

struct FColorEdge
{
public:
	Point3m vertA, vertB;
	Point3m colorA, colorB;
public:
	FColorEdge()
	{
		vertA = Point3m(0, 0, 0);
		vertB = Point3m(0, 0, 0);
		colorA = Point3m(0, 0, 0);
		colorB = Point3m(0, 0, 0);
	}
	FColorEdge(Point3f a, Point3f b)
	{
		vertA = a;
		vertB = b;
		colorA = Point3m(0, 0, 0);
		colorB = Point3m(0,0,0);
	}
	FColorEdge(Point3f a, Point3f b, Point3m _color_a, Point3m _color_b)
	{
		vertA = a;
		vertB = b;
		colorA = _color_a;
		colorB = _color_b;
	}

	void setColor(const Point3m &_color_a, const Point3m &_color_b)
	{
		colorA = _color_a;
		colorB = _color_b;
	}

	FColorEdge& operator=(const FColorEdge& p)
	{
		this->vertA = p.vertA;
		this->vertB = p.vertB;
		this->colorA = p.colorA;
		this->colorB = p.colorB;
		return *this;
	}
};

struct FEdge {
public:
	Point3f vertA, vertB;
	int indexA = -1, indexB = -1;
	int iTriNo = -1;
	int faceA = -1, faceB = -1;
	float fLength = 0;
	bool bBad = false;
public:
	FEdge() {}
	~FEdge() {}
	FEdge(Point3f a, Point3f b)
	{
		vertA = a;
		vertB = b;
		this->initalFaceIndexs();
		this->bBad = false;
	}
	FEdge(int indexa, int indexb)
	{
		indexA = indexa;
		indexB = indexb;
		faceA = -1;
		faceB = -1;
	}
	FEdge(Point3f a, Point3f b, int indexa, int indexb)
	{
		vertA = a;
		vertB = b;
		indexA = indexa;
		indexB = indexb;
		faceA = -1;
		faceB = -1;
		this->initalFaceIndexs();
	}
	FEdge(Point3f a, Point3f b, int indexa, int indexb, int _faceA, int _faceB)
	{
		vertA = a;
		vertB = b;
		indexA = indexa;
		indexB = indexb;
		faceA = _faceA;
		faceB = _faceB;
	}
	void InitEdge(Point3f a, Point3f b)
	{
		vertA = a;
		vertB = b;
	}
	void InitEdge(Point3f a, Point3f b, int iNo)
	{
		vertA = a;
		vertB = b;
		iTriNo = iNo;
	}
	void InitEdge(int _indexA, int _indexB, int _faceA, int _faceB, vector<Point3m> nodelist)
	{
		vertA = nodelist[_indexA];
		vertB = nodelist[_indexB];
		indexA = _indexA;
		indexB = _indexB;
		faceA = _faceA;
		faceB = _faceB;
	}

    float calculateLength()
	{
		this->fLength = sqrtf((vertB - vertA) * (vertB - vertA));
		return fLength;
	}

	void initalFaceIndexs()
	{
		this->faceA = -1;
		this->faceB = -1;
	}

	void giveFaceIndex(int newFaceIndex)
	{
		if (this->faceA == -1)
		{
			faceA = newFaceIndex;
			return;
		}
		else if (this->faceB == -1)
		{
			faceB = newFaceIndex;
			return;
		}
		else
		{
			return;
		}
	}

	void exchangeItself()
	{
		Point3f tempVert;
		tempVert = vertA;
		vertA = vertB;
		vertB = tempVert;

		int tempIndex;
		tempIndex = indexA;
		indexA = indexB;
		indexB = tempIndex;
	}

	bool lineIntersectSide(Point3f A, Point3f B, Point3f C, Point3f D)
	{
		Point3f v1, v2;
		v1 = B - A;
		v2 = D - C;
		if ((v1.X() * v2.Y() - v1.Y() * v2.X()) == 0)
		{
			return false;
		}
		else
		{
			float fC = (C.Y() - A.Y()) * (A.X() - B.X()) - (C.X() - A.X()) * (A.Y() - B.Y());
			float fD = (D.Y() - A.Y()) * (A.X() - B.X()) - (D.X() - A.X()) * (A.Y() - B.Y());

			if (fC * fD > 0)
			{
				return false;
			}
			return true;
		}
	}

	bool intersectWith(FEdge e)
	{
		if (!lineIntersectSide(this->vertA, this->vertB, e.vertA, e.vertB))
		{
			return false;
		}
		if (!lineIntersectSide(e.vertA, e.vertB, this->vertA, this->vertB))
		{
			return false;
		}
		return true;
	}

	bool intersection(FEdge l2)
	{

		// Quick rejection test
		if((vertA.X() > vertB.X() ? vertA.X() : vertB.X()) < (l2.vertA.X() < l2.vertB.X() ? l2.vertA.X() : l2.vertB.X()) ||
			(vertA.Y() >vertB.Y() ? vertA.Y() :vertB.Y()) < (l2.vertA.Y() < l2.vertB.Y() ? l2.vertA.Y() : l2.vertB.Y()) ||
			(l2.vertA.X() > l2.vertB.X() ? l2.vertA.X() : l2.vertB.X()) < (vertA.X() < vertB.X() ? vertA.X() : vertB.X()) ||
			(l2.vertA.Y() > l2.vertB.Y() ? l2.vertA.Y() : l2.vertB.Y()) < (vertA.Y() <vertB.Y() ? vertA.Y() :vertB.Y()))
		{
			return false;
		}
		// Straddle test
		if ((((vertA.X() - l2.vertA.X())*(l2.vertB.Y() - l2.vertA.Y()) - (vertA.Y() - l2.vertA.Y())*(l2.vertB.X() - l2.vertA.X()))*
			((vertB.X() - l2.vertA.X())*(l2.vertB.Y() - l2.vertA.Y()) - (vertB.Y() - l2.vertA.Y())*(l2.vertB.X() - l2.vertA.X()))) > 0 ||
			(((l2.vertA.X() - vertA.X())*(vertB.Y() - vertA.Y()) - (l2.vertA.Y() - vertA.Y())*(vertB.X() - vertA.X()))*
			((l2.vertB.X() - vertA.X())*(vertB.Y() - vertA.Y()) - (l2.vertB.Y() - vertA.Y())*(vertB.X() - vertA.X()))) > 0)
		{
			return false;
		}
		return true;
	}

	bool IsReplicateWith(FEdge e)
	{
		if (this->vertA == e.vertA && this->vertB == e.vertB)
			return true;
		else if (this->vertA == e.vertB && this->vertB == e.vertA)
			return true;
		else
			return false;
	}

	void Dereplication(vector<FEdge> &edgeList)
	{
		vector<FEdge> delList;
		for (int i = 0; i < edgeList.size(); i++)
		{
			FEdge edgeC = edgeList[i];
			for (int j = i + 1; j < edgeList.size(); j++)
			{
				if (edgeC.IsReplicateWith(edgeList[j]))
				{
					bool bRe = false;
					for (int k = 0; k < delList.size(); k++)
					{
						if (edgeC.IsReplicateWith(delList[k]))
						{
							bRe = true;
							break;
						}
					}

					if (!bRe)
						delList.push_back(edgeC);

					break;
				}
			}
		}

		vector<FEdge> tempV;
		for (int i = 0; i < edgeList.size(); i++)
		{
			FEdge edgeC = edgeList[i];
			bool PushThis = true;
			for (int j = 0; j < delList.size(); j++)
			{
				if (edgeC.IsReplicateWith(delList[j]))
				{
					PushThis = false;
					break;
				}
			}

			if (PushThis)
				tempV.push_back(edgeC);
		}
		edgeList.clear();
		for (int i = 0; i < tempV.size(); i++)
			edgeList.push_back(tempV[i]);
	}

	bool isSameAs(const FEdge e)
	{
		if (e.indexA == indexA && e.indexB == indexB)
		{
			return true;
		}
		else if (e.indexA == indexB && e.indexB == indexA)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void adjustNeighborFaceIndex(FEdge obstacle, int neighborIndex, int canKao1, int canKao2)
	{
		if (!this->isSameAs(obstacle))
		{
			if (this->faceA == canKao1 || this->faceA == canKao2)
			{
				this->faceA = neighborIndex;
			}
			else if (this->faceB == canKao1 || this->faceB == canKao2)
			{
				this->faceB = neighborIndex;
			}
		}
	}

	bool intersectWithOn2DSpace(FEdge aimEdge, Point3m &intersection)
	{
		FEdge tempEdgeSelf, tempEdgeAim;
		tempEdgeSelf = *this;
		tempEdgeAim = aimEdge;

		Point3m translateVector = -tempEdgeSelf.vertA;

		tempEdgeSelf.vertA += translateVector;
		tempEdgeSelf.vertB += translateVector;
		tempEdgeAim.vertA += translateVector;
		tempEdgeAim.vertB += translateVector;

		Point3m edgeVector = (tempEdgeSelf.vertB - tempEdgeSelf.vertA).Normalize();
		float rotateAngle = -acosf(edgeVector * Point3m(1, 0, 0));

		vcg::Matrix44f transMat;
		vcg::Matrix44f transform, transformInverse;
		transform.SetIdentity();
		transform.SetTranslate(Point3m(0, 0, 0));
		transformInverse.SetTranslate(-Point3m(0, 0, 0));

		vcg::Matrix44f transformValue, rotMatLocal, rotMatLocal2;
		transMat.SetIdentity();
		rotMatLocal.SetIdentity();
		rotMatLocal2.SetIdentity();

		transformValue.SetRotateDeg(rotateAngle, Point3m(0, 0, 1));
		rotMatLocal *= transformValue;


		transform *= rotMatLocal * transMat;
		transform *= transformInverse;

		Point3m hh = transform * Point3m(1, 1, 0);

		tempEdgeSelf.vertA = transform * tempEdgeSelf.vertA;
		tempEdgeSelf.vertB = transform * tempEdgeSelf.vertB;
		tempEdgeAim.vertA = transform * tempEdgeAim.vertA;
		tempEdgeAim.vertB = transform * tempEdgeAim.vertB;

		if (tempEdgeAim.vertA.Y() * tempEdgeAim.vertB.Y() > 0)
		{
			return false;
		}

		float absYA, absYB;
		absYA = abs(tempEdgeAim.vertA.Y());
		absYB = abs(tempEdgeAim.vertB.Y());
		float t = absYA / (absYA + absYB);

		float zeroX = tempEdgeAim.vertA.X() + (tempEdgeAim.vertB.X() - tempEdgeAim.vertA.X()) * t;

		if (0 <= zeroX && zeroX <= tempEdgeSelf.vertB.X())
		{
			intersection = aimEdge.vertA + (aimEdge.vertB - aimEdge.vertA) * t;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool almost_equal(const FEdge &e)
	{
		FPoint vA(vertA, 0);
		FPoint vB(vertB, 0);
		FPoint eA(e.vertA, 0);
		FPoint eB(e.vertB, 0);
		return	(vA.almost_equal(eA) && vB.almost_equal(eB)) ||
			(vA.almost_equal(eB) && vB.almost_equal(eA));
	}

	bool operator!=(const FEdge e)
	{
		if (indexA != e.indexA && indexA != e.indexB && indexB != e.indexA && indexB != e.indexB)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool operator==(const FEdge e)
	{
		if (e.vertA == vertA && e.vertB == vertB)
		{
			return true;
		}
		else if (e.vertA == vertB && e.vertB == vertA)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool operator==(const Point2i e)
	{
		if (e.X() == indexA && e.Y() == indexB)
		{
			return true;
		}
		else if (e.X() == indexB && e.Y() == indexA)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	FEdge& operator=(const FEdge &p)
	{
		this->vertA = p.vertA;
		this->vertB = p.vertB;
		this->indexA = p.indexA;
		this->indexB = p.indexB;
		this->faceA = p.faceA;
		this->faceB = p.faceB;
		this->iTriNo = p.iTriNo;
		this->fLength = p.fLength;
		this->bBad = p.bBad;
		return *this;
	}
};

struct FFace {
public:
	Point3f v1, v2, v3;
	int iSpaceAttribute1, iSpaceAttribute2, iSpaceAttribute3;
	int n1, n2, n3;
	Point3f normalVector;
	CVertexO *vertP1, *vertP2, *vertP3;
	Point2i edge1, edge2, edge3;
	int edge1Index, edge2Index, edge3Index;
	vcg::Color4b color1, color2, color3;
	Point3m infoV1, infoV2, infoV3;
	bool bBad = false;

	FFace& operator=(const FFace &p)
	{
		this->v1 = p.v1;
		this->v2 = p.v2;
		this->v3 = p.v3;

		this->n1 = p.n1;
		this->n2 = p.n2;
		this->n3 = p.n3;

		this->edge1 = p.edge1;
		this->edge2 = p.edge2;
		this->edge3 = p.edge3;

		this->edge1Index = p.edge1Index;
		this->edge2Index = p.edge2Index;
		this->edge3Index = p.edge3Index;

		this->iSpaceAttribute1 = p.iSpaceAttribute1;
		this->iSpaceAttribute2 = p.iSpaceAttribute2;
		this->iSpaceAttribute3 = p.iSpaceAttribute3;

		this->vertP1 = p.vertP1;
		this->vertP2 = p.vertP2;
		this->vertP3 = p.vertP3;

		this->color1 = p.color1;
		this->color2 = p.color2;
		this->color3 = p.color3;

		this->infoV1 = p.infoV1;
		this->infoV2 = p.infoV2;
		this->infoV3 = p.infoV3;

		this->normalVector = p.normalVector;
		this->bBad = p.bBad;
		return *this;
	}

	FFace() { }
	~FFace() { }
	FFace(int _n1, int _n2, int _n3)
	{
		n1 = _n1;
		n2 = _n2;
		n3 = _n3;
		this->initalEdge();
	}
	FFace(Point3i edgeIndexs)
	{
		this->edge1Index = edgeIndexs.X();
		this->edge2Index = edgeIndexs.Y();
		this->edge3Index = edgeIndexs.Z();
	}
	FFace(Point3f _v1, Point3f _v2, Point3f _v3)
	{
		v1 = _v1;
		v2 = _v2;
		v3 = _v3;
		bBad = false;
	}
	FFace(Point3f _v1, Point3f _v2, Point3f _v3, Point3m _infoV1, Point3m _infoV2, Point3m _infoV3)
	{
		v1 = _v1;
		v2 = _v2;
		v3 = _v3;
		infoV1 = _infoV1;
		infoV2 = _infoV2;
		infoV3 = _infoV3;
	}
	FFace(Point3f _v1, Point3f _v2, Point3f _v3, Point3f n)
	{
		v1 = _v1;
		v2 = _v2;
		v3 = _v3;
		normalVector = n;
	}
	FFace(Point3f _v1, Point3f _v2, Point3f _v3, int sign1, int sign2, int sign3)
	{
		v1 = _v1;
		v2 = _v2;
		v3 = _v3;
		iSpaceAttribute1 = sign1;
		iSpaceAttribute2 = sign2;
		iSpaceAttribute3 = sign3;
	}
	FFace(Point3f _v1, Point3f _v2, Point3f _v3, vcg::Color4b c1, vcg::Color4b c2, vcg::Color4b c3)
	{
		v1 = _v1;
		v2 = _v2;
		v3 = _v3;
		color1 = c1;
		color2 = c2;
		color3 = c3;
	}
	FFace(Point3f _v1, Point3f _v2, Point3f _v3, vcg::Color4b c1, vcg::Color4b c2, vcg::Color4b c3, int _n1, int _n2, int _n3)
	{
		v1 = _v1;
		v2 = _v2;
		v3 = _v3;
		color1 = c1;
		color2 = c2;
		color3 = c3;
		n1 = _n1;
		n2 = _n2;
		n3 = _n3;
	}
	FFace(Point3f _v1, Point3f _v2, Point3f _v3, int sign1, int sign2, int sign3, Point3f n)
	{
		v1 = _v1;
		v2 = _v2;
		v3 = _v3;
		iSpaceAttribute1 = sign1;
		iSpaceAttribute2 = sign2;
		iSpaceAttribute3 = sign3;
		normalVector = n;
	}
	FFace(int _edge1Index, int _edge2Index, int _edge3Index, int vert1Index, int vert2Index, int vert3Index, vector<FEdge> edgelist, vector<Point3f> nodelist)
	{
		edge1Index = _edge1Index;
		edge2Index = _edge2Index;
		edge3Index = _edge3Index;
		n1 = vert1Index;
		n2 = vert2Index;
		n3 = vert3Index;

		edge1 = Point2i(edgelist[edge1Index].indexA, edgelist[edge1Index].indexB);
		edge2 = Point2i(edgelist[edge2Index].indexA, edgelist[edge2Index].indexB);
		edge3 = Point2i(edgelist[edge3Index].indexA, edgelist[edge3Index].indexB);
		v1 = nodelist[n1];
		v2 = nodelist[n2];
		v3 = nodelist[n3];
	}

	void InitFFace(Point3f _v1, Point3f _v2, Point3f _v3)
	{
		v1 = _v1;
		v2 = _v2;
		v3 = _v3;
	}

	void InitFFace(int _n1, int _n2, int _n3)
	{
		n1 = _n1;
		n2 = _n2;
		n3 = _n3;
		this->initalEdge();
	}

	void InitFFace(int _n1, int _n2, int _n3, CVertexO *vp1, CVertexO *vp2, CVertexO *vp3)
	{
		n1 = _n1;
		n2 = _n2;
		n3 = _n3;
		this->vertP1 = vp1;
		this->vertP2 = vp2;
		this->vertP3 = vp3;
		this->initalEdge();
	}

	void InitVertexsPointers(CVertexO *vp1, CVertexO *vp2, CVertexO *vp3)
	{
		this->vertP1 = vp1;
		this->vertP2 = vp2;
		this->vertP3 = vp3;
	}

	bool IsSameAs(FFace f)
	{
		if (this->v1 == f.v1 && this->v2 == f.v2 && this->v3 == f.v3)
			return true;
		else
			return false;
	}

	Point3f GetInterCenterPoint()
	{
		float a, b, c;
		a = sqrt((v2.X() - v3.X()) * (v2.X() - v3.X()) + (v2.Y() - v3.Y()) * (v2.Y() - v3.Y()));
		b = sqrt((v3.X() - v1.X()) * (v3.X() - v1.X()) + (v3.Y() - v1.Y()) * (v3.Y() - v1.Y()));
		c = sqrt((v1.X() - v2.X()) * (v1.X() - v2.X()) + (v1.Y() - v2.Y()) * (v1.Y() - v2.Y()));

		float X, Y;
		X = (a * v1.X() + b * v2.X() + c * v3.X()) / (a + b + c);
		Y = (a * v1.Y() + b * v2.Y() + c * v3.Y()) / (a + b + c);

		return Point3f(X, Y, 0);
	}

	Point3f GetAInnerPoint(Point3f V1, Point3f V2, Point3f V3)
	{
		Point3f vec12, vec13;
		vec12.X() = V2.X() - V1.X();
		vec12.Y() = V2.Y() - V1.Y();
		vec12.Z() = 0;
		vec13.X() = V3.X() - V1.X();
		vec13.Y() = V3.Y() - V1.Y();
		vec13.Z() = 0;

		return Point3f(V1.X() + (vec12.X() + vec13.X()) / 4.0f, V1.Y() + (vec12.Y() + vec13.Y()) / 4.0f, 0);
	}

	void initalEdge()
	{
		this->edge1 = Point2i(n1, n2);
		this->edge2 = Point2i(n2, n3);
		this->edge3 = Point2i(n3, n1);
	}

	void adjustNormalVector(Point3m standNormal)
	{
		Point3m vector1, vector2;
		vector1 = this->v2 - this->v1;
		vector2 = this->v3 - this->v2;
		Point3m tempNormal = vector1 ^ vector2;
		if (tempNormal * standNormal < 0)
		{
			Point3m tempPoint;
			tempPoint = v3;
			v3 = v2;
			v2 = tempPoint;

			int tempIndex;
			tempIndex = n3;
			n3 = n2;
			n2 = tempIndex;
		}
	}

	void getIntersectionWithEdgeOn2DSpace(FEdge aimEdge, vector<Point3m> &intersections)
	{
		FEdge tempEdge;
		intersections.clear();

		Point3m intersection1(0, 0, 0);
		tempEdge = FEdge(this->v1, this->v2);
		if (tempEdge.intersectWithOn2DSpace(aimEdge, intersection1))
		{
			intersections.push_back(intersection1);
		}

		Point3m intersection2(0, 0, 0);
		tempEdge = FEdge(this->v2, this->v3);
		if (tempEdge.intersectWithOn2DSpace(aimEdge, intersection2))
		{
			intersections.push_back(intersection2);
		}

		Point3m intersection3(0, 0, 0);
		tempEdge = FEdge(this->v3, this->v1);
		if (tempEdge.intersectWithOn2DSpace(aimEdge, intersection3))
		{
			intersections.push_back(intersection3);
		}

		return;
	}

	bool ifVertInsideTriangleOn2DSpace(Point3m p)
	{
		vector<Point3m> polygon;
		polygon.push_back(this->v1);
		polygon.push_back(this->v2);
		polygon.push_back(this->v3);

		float sum = 0;
		float px = p.X(), py = p.Y();

		for (int i = 0, l = polygon.size(), j = l - 1; i < l; j = i, i++)
		{
			float sx = polygon[i].X(), sy = polygon[i].Y();
			float tx = polygon[j].X(), ty = polygon[j].Y();

			// Point coincides with polygon vertex or is on polygon edge
			if ((sx - px) * (px - tx) >= 0 && (sy - py) * (py - ty) >= 0 && (px - sx) * (ty - sy) == (py - sy) * (tx - sx))
			{
				return true;
			}

			// Angle between point and adjacent vertex connection
			float angle = atan2(sy - py, sx - px) - atan2(ty - py, tx - px);

			// Ensure angle is within range (-π to π)
			if (angle >= PI)
			{
				angle = angle - PI * 2;
			}
			else if (angle <= -PI)
			{
				angle = angle + PI * 2;
			}

			sum += angle;
		}

		// Calculate winding number and determine geometric relationship between point and polygon
		return roundf(sum / PI) == 0 ? false : true;
	}

	bool circumCircleContains(const Point3m &v) const
	{
		const float ab = v1 * v1;
		const float cd = v2 * v2;
		const float ef = v3* v3;

		const float ax = v1.X();
		const float ay = v1.Y();
		const float bx = v2.X();
		const float by = v2.Y();
		const float cx = v3.X();
		const float cy = v3.Y();

		const double circum_x = (ab * (cy - by) + cd * (ay - cy) + ef * (by - ay)) / (ax * (cy - by) + bx * (ay - cy) + cx * (by - ay));
		const double circum_y = (ab * (cx - bx) + cd * (ax - cx) + ef * (bx - ax)) / (ay * (cx - bx) + by * (ax - cx) + cy * (bx - ax));

		const Point3m circum((circum_x * 0.5f), (circum_y * 0.5f), 0);
		const double circum_radius = (v1 - circum) * (v1 - circum);
		const double dist = (v - circum) * (v - circum);
		return dist <= circum_radius;
	}

	bool containsVertex(const Point3m &v) const
	{
		return v1 == v || v2 == v || v3 == v;
	}
};

struct TraceablePoint
{
	TraceablePoint() {}
	TraceablePoint(Point3m p, int iedge)
	{
		this->point = p;
		this->edgeIndex = iedge;
	}

	bool operator==(const TraceablePoint p)
	{
		if (this->point == p.point && this->edgeIndex == p.edgeIndex)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

public:
	Point3m point;
	int edgeIndex;
	float fDistance = 0;
};

struct RichIntersection
{
public:
	RichIntersection(){}
	RichIntersection(Point3m _intersection, bool _EnterPot)
	{
		this->intersection = _intersection;
		this->isEnterPot = _EnterPot;
	}

public:
	Point3m intersection;
	bool isEnterPot;
};

struct WordEdge
{
public:
	WordEdge() {}
	WordEdge(FEdge edgeInput)
	{
		this->vertA = edgeInput.vertA;
		this->vertB = edgeInput.vertB;
		this->indexA = -1;
		this->indexB = -1;
		this->bNeedProjectA = true;
		this->bNeedProjectB = true;
		bHaveIntersection = false;
	}
	WordEdge(Point3f _a, Point3f _b)
	{
		this->vertA = _a;
		this->vertB = _b;
		this->indexA = -1;
		this->indexB = -1;
		this->bNeedProjectA = true;
		this->bNeedProjectB = true;
		bHaveIntersection = false;
	}
	WordEdge(Point3f _a, Point3f _b, int _A, int _B)
	{
		this->vertA = _a;
		this->vertB = _b;
		this->indexA = _A;
		this->indexB = _B;
		this->bNeedProjectA = true;
		this->bNeedProjectB = true;
		bHaveIntersection = false;
	}
	WordEdge(WordEdge _edge, int _attriA, int _attriB)
	{
		this->vertA = _edge.vertA;
		this->vertB = _edge.vertB;
		this->indexA = _edge.indexA;
		this->indexB = _edge.indexB;
		this->attributeA = _attriA;
		this->attributeB = _attriB;
	}
	WordEdge(Point3f _a, Point3f _b, int _A, int _B, int _attriA, int _attriB)
	{
		this->vertA = _a;
		this->vertB = _b;
		this->indexA = _A;
		this->indexB = _B;
		this->attributeA = _attriA;
		this->attributeB = _attriB;
	}
	WordEdge(Point3f _a, Point3f _b, int _A, int _B, bool _needProjA, bool _needProjB)
	{
		this->vertA = _a;
		this->vertB = _b;
		this->indexA = _A;
		this->indexB = _B;
		this->bNeedProjectA = _needProjA;
		this->bNeedProjectB = _needProjB;
		bHaveIntersection = false;
	}

	void exchange()
	{
		int tempIndex;
		Point3f tempVert, tempProjVert, temp2D;
		bool tempNeed;

		tempIndex = indexA;
		indexA = indexB;
		indexB = tempIndex;

		tempVert = vertA;
		vertA = vertB;
		vertB = tempVert;

		tempProjVert = projVertA;
		projVertA = projVertB;
		projVertB = tempProjVert;

		temp2D = vertA_2D;
		vertA_2D = vertB_2D;
		vertB_2D = temp2D;

		tempNeed = bNeedProjectA;
		bNeedProjectA = bNeedProjectB;
		bNeedProjectB = tempNeed;
	}

	bool sameAs(WordEdge edge)
	{
		if (this->vertA == edge.vertA && this->vertB == edge.vertB)
		{
			return true;
		}
		else if (this->vertA == edge.vertB && this->vertB == edge.vertA)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	float getLength()
	{
		Point3m edge = this->vertB - this->vertA;
		return sqrtf(edge.X() * edge.X() + edge.Y() * edge.Y() + edge.Z() * edge.Z());
	}

	bool intersectWith(const WordEdge &edge, Point3m normalV, Point3m &intersection)
	{
		//仅限于两边都处在以normalV为法向共面的两边求交
		Point3m tempCenter, tempAxisX, tempAxisY;
		this->getVector();
		tempCenter = this->vertA;
		tempAxisX = this->vector;
		tempAxisY = (normalV ^ tempAxisX).Normalize();

		float projX, projY;
		Point3m tempVector;
		float edgeLength = this->getLength();
		Point3m projA1(0, 0, 0), projB1(edgeLength, 0, 0), projA2, projB2;
		tempVector = edge.vertA - tempCenter;
		projX = tempVector * tempAxisX;
		projY = tempVector * tempAxisY;
		projA2 = Point3m(projX, projY, 0);
		tempVector = edge.vertB - tempCenter;
		projX = tempVector * tempAxisX;
		projY = tempVector * tempAxisY;
		projB2 = Point3m(projX, projY, 0);

		float a, b;
		float interProjX;
		float diffX = projA2.X() - projB2.X();
		if (diffX == 0 && projA2.Y() * projB2.Y() < 0)
		{
			interProjX = projA2.X();
			if (interProjX > 0 && interProjX < edgeLength)
			{
				intersection = tempCenter + tempAxisX * interProjX;
				return true;
			}
			return false;
		}
		else
		{
			a = (projA2.Y() - projB2.Y()) / diffX;
			b = projA2.Y() - a * projA2.X();
			if (a == 0)
			{
				return false;
			}

			interProjX = -b / a;
			if (interProjX > 0 && interProjX < edgeLength)
			{
				float minX, maxX;
				if (projA2.X() < projB2.X())
				{
					minX = projA2.X();
					maxX = projB2.X();
				}
				else
				{
					minX = projB2.X();
					maxX = projA2.X();
				}

				if (interProjX > minX && interProjX < maxX)
				{
					intersection = tempCenter + tempAxisX * interProjX;
					return true;
				}
			}
		}
		return false;
	}

	void getVector()
	{
		this->vector = vertB - vertA;
		this->vector = vector.Normalize();
	}

	void getNormalV(Point3f signNormalV)
	{
		Point3m tempNormalV;
		this->getVector();
		tempNormalV = signNormalV ^ this->vector;
		tempNormalV = tempNormalV.Normalize();
		this->normalV = tempNormalV;
	}

	//重新按照WordEdge的方向顺序排列分割点
	void reSortSplitVerts()
	{
		this->getVector();
		Point3m standardVector = this->vector;

		Point3m tempEdge;
		std::vector<float> distanceList;
		for (auto splitVert : this->splitVerts)
		{
			tempEdge = splitVert.intersection - this->vertB;
			distanceList.push_back(tempEdge * standardVector);
		}

		int i, j, temp;
		RichIntersection tempSplitVert;
		for (i = distanceList.size() - 1; 0 < i; i--)
		{
			for (j = 0; j < i; j++)
			{
				if (distanceList[j] > distanceList[j + 1]) {
					temp = distanceList[j];
					distanceList[j] = distanceList[j + 1];
					distanceList[j + 1] = temp;

					tempSplitVert = splitVerts[j];
					splitVerts[j] = splitVerts[j + 1];
					splitVerts[j + 1] = tempSplitVert;
				}
			}
		}
	}

public:
	enum {NORMALPOINT = 0, OUTPOINT, ENTERPOINT};
	int indexA, indexB;
	int attributeA, attributeB;
	Point3f vertA, vertB;
	Point3f projVertA, projVertB;
	Point3f vertA_2D, vertB_2D;
	Point3f vector;
	Point3f normalV;
	bool bNeedProjectA = true, bNeedProjectB = true;

	bool bHaveIntersection;
	Point3f mixP;
	int indexOfMixP;
	std::vector<RichIntersection> splitVerts;
};

struct WordCircle
{
public:
	WordCircle() {}
	WordCircle(vector<WordEdge> _list)
	{
		bOutsideCircle = true;
		bIsClockWise = true;
		this->edgeList = _list;
		headIndex = edgeList[0].indexA;
		endIndex = edgeList[edgeList.size() - 1].indexB;
	}
	WordCircle(WordEdge edge)
	{
		bOutsideCircle = true;
		bIsClockWise = true;
		this->edgeList.push_back(edge);
		headIndex = edge.indexA;
		endIndex = edge.indexB;
	}

	void pushCircleBack(WordCircle &newcircle)
	{
		for (auto c : newcircle.edgeList)
		{
			this->edgeList.push_back(c);
		}
		this->endIndex = newcircle.endIndex;
		newcircle.edgeList.clear();
		newcircle.headIndex = -1;
		newcircle.endIndex = -1;
	}

	void pushCircleFront(WordCircle &newcircle)
	{
		for (auto c : this->edgeList)
		{
			newcircle.edgeList.push_back(c);
		}
		this->edgeList.clear();
		this->edgeList = newcircle.edgeList;
		this->headIndex = newcircle.headIndex;

		newcircle.edgeList.clear();
		newcircle.headIndex = -1;
		newcircle.endIndex = -1;
	}

	void putUpsideDown()
	{
		int temp;
		temp = this->headIndex;
		this->headIndex = this->endIndex;
		this->endIndex = temp;

		vector<WordEdge> newList;
		for (int i = edgeList.size() - 1; i >= 0; i--)
		{
			newList.push_back(edgeList[i]);
		}
		for (int i = 0; i < newList.size(); i++)
		{
			newList[i].exchange();
		}
		this->edgeList.clear();
		this->edgeList = newList;
		newList.clear();
	}

	void simplify()
	{
		/// compute each directVector
		for (int i = 0; i < edgeList.size(); i++)
		{
			edgeList[i].getVector();
		}

		int Length;
		vector<WordEdge> newList;
		vector<WordEdge> holder;
		int I;
		Point3f compareV;

		Length = edgeList.size();
		compareV = edgeList[0].vector;
		holder.clear();
		I = 0;
		while (I < Length)
		{
			Point3m vector1, vector2;
			vector1 = edgeList[I].vector;
			vector2 = compareV;
			vector1 = vector1.Normalize();
			vector2 = vector2.Normalize();
			float cosAngle = vector1 * vector2;

			//if (abs(1 - cosAngle) < 0.1f && cosAngle > 0)
			if (edgeList[I].vector == compareV)
			{
				holder.push_back(edgeList[I]);
			}
			else
			{
				if (holder.size() <= 1)
				{
					newList.push_back(holder[0]);
				}
				else
				{
					Point3f a, b;
					int     index_a, index_b;
					a = holder[0].vertA;
					b = holder[holder.size() - 1].vertB;
					index_a = holder[0].indexA;
					index_b = holder[holder.size() - 1].indexB;
					newList.push_back(WordEdge(a, b, index_a, index_b));
				}

				holder.clear();
				holder.push_back(edgeList[I]);
				compareV = edgeList[I].vector;
			}

			I++;
			if (I >= Length)
			{
				Point3f a, b;
				int     index_a, index_b;
				a = holder[0].vertA;
				b = holder[holder.size() - 1].vertB;
				index_a = holder[0].indexA;
				index_b = holder[holder.size() - 1].indexB;
				newList.push_back(WordEdge(a, b, index_a, index_b));
			}
		}

 		this->edgeList.clear();
		this->edgeList = newList;
		newList.clear();
		this->headIndex = edgeList[0].indexA;
		this->endIndex = edgeList[edgeList.size() - 1].indexB;
	}

	void recognizeClockWise(Point3f normalV)
	{
		float maxX = this->edgeList[0].vertA.X();
		int index = 0;
		for (int i = 0; i < this->edgeList.size(); i++)
		{
			if (this->edgeList[i].vertA.X() > maxX)
			{
				maxX = edgeList[i].vertA.X();
				index = i;
			}
		}

		Point3f a, b, c;
		a = edgeList[index].vertA;
		b = edgeList[index].vertB;
		if (index == 0)
		{
			c = edgeList[edgeList.size() - 1].vertA;
		}
		else
		{
			c = edgeList[index - 1].vertA;
		}
		Point3f v1, v2, tempNV;
		v1 = a - c;
		v2 = b - a;
		tempNV = v1 ^ v2;

		if (tempNV == Point3m(0, 0, 0))
		{
			if (index == edgeList.size() - 1)
			{
				c = edgeList[0].vertB;
			}
			else
			{
				c = edgeList[index + 1].vertB;
			}

			v1 = b - a;
			v2 = c - b;
			tempNV = v1 ^ v2;
		}

		if (tempNV * normalV > 0)
		{
			bIsClockWise = false;
		}
		else
		{
			bIsClockWise = true;
		}
	}

	void recognizeClockWise2(Point3f normalV)
	{
		float maxX = this->edgeList[0].vertA_2D.X();
		int index = 0;
		for (int i = 0; i < this->edgeList.size(); i++)
		{
			if (this->edgeList[i].vertA_2D.X() > maxX)
			{
				maxX = edgeList[i].vertA_2D.X();
				index = i;
			}
		}

		Point3f a, b, c;
		a = edgeList[index].vertA_2D;
		b = edgeList[index].vertB_2D;
		if (index == 0)
		{
			c = edgeList[edgeList.size() - 1].vertA_2D;
		}
		else
		{
			c = edgeList[index - 1].vertA_2D;
		}
		Point3f v1, v2, tempNV;
		v1 = a - c;
		v2 = b - a;
		tempNV = v1 ^ v2;

		if (tempNV == Point3m(0, 0, 0))
		{
			if (index == edgeList.size() - 1)
			{
				c = edgeList[0].vertB_2D;
			}
			else
			{
				c = edgeList[index + 1].vertB_2D;
			}

			v1 = b - a;
			v2 = c - b;
			tempNV = v1 ^ v2;
		}

		if (tempNV * normalV > 0)
		{
			bIsClockWise = false;
		}
		else
		{
			bIsClockWise = true;
		}
	}

	void initalBoundingBox()
	{
		if (edgeList.size() == 0)
		{
			return;
		}

		vector<Point3f> verts;
		for (auto edge : edgeList)
		{
			verts.push_back(edge.vertA);
		}

		float maxX, minX, maxY, minY;
		maxX = verts[0].X();
		minX = verts[0].X();
		maxY = verts[0].Y();
		minY = verts[0].Y();
		for (auto vert : verts)
		{
			if (vert.X() > maxX)
			{
				maxX = vert.X();
			}
			if (vert.X() < minX)
			{
				minX = vert.X();
			}

			if (vert.Y() > maxY)
			{
				maxY = vert.Y();
			}
			if (vert.Y() < minY)
			{
				minY = vert.Y();
			}
		}

		Point3f  uL(minX, maxY, 0), uR(maxX, maxY, 0), dR(maxX, minY, 0), dL(minX, minY, 0);
		boundingBoxEdgeList.push_back(WordEdge(uL, uR));
		boundingBoxEdgeList.push_back(WordEdge(uR, dR));
		boundingBoxEdgeList.push_back(WordEdge(dR, dL));
		boundingBoxEdgeList.push_back(WordEdge(dL, uL));
	}

	void initalBoundingBox2()
	{
		if (edgeList.size() == 0)
		{
			return;
		}

		vector<Point3f> verts;
		for (auto edge : edgeList)
		{
			verts.push_back(edge.vertA_2D);
		}

		float maxX, minX, maxY, minY;
		maxX = verts[0].X();
		minX = verts[0].X();
		maxY = verts[0].Y();
		minY = verts[0].Y();
		for (auto vert : verts)
		{
			if (vert.X() > maxX)
			{
				maxX = vert.X();
			}
			if (vert.X() < minX)
			{
				minX = vert.X();
			}

			if (vert.Y() > maxY)
			{
				maxY = vert.Y();
			}
			if (vert.Y() < minY)
			{
				minY = vert.Y();
			}
		}

		Point3f  uL(minX, maxY, 0), uR(maxX, maxY, 0), dR(maxX, minY, 0), dL(minX, minY, 0);
		boundingBoxEdgeList.push_back(WordEdge(uL, uR));
		boundingBoxEdgeList.push_back(WordEdge(uR, dR));
		boundingBoxEdgeList.push_back(WordEdge(dR, dL));
		boundingBoxEdgeList.push_back(WordEdge(dL, uL));
	}

	bool thisVertInsideBoundBox(Point3m vert)
	{
		Point3f upRightP, downLeftP;
		upRightP = boundingBoxEdgeList[0].vertB;
		downLeftP = boundingBoxEdgeList[2].vertB;

		if (downLeftP.X() <= vert.X() && vert.X() <= upRightP.X()
			&&
			downLeftP.Y() <= vert.Y() && vert.Y() <= upRightP.Y())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool thisVertInsideBoundBox2(Point3m vert)
	{
		Point3f upRightP, downLeftP;
		upRightP = boundingBoxEdgeList[0].vertB_2D;
		downLeftP = boundingBoxEdgeList[2].vertB_2D;

		if (downLeftP.X() <= vert.X() && vert.X() <= upRightP.X()
			&&
			downLeftP.Y() <= vert.Y() && vert.Y() <= upRightP.Y())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void reconstructItself()
	{
		vector<WordEdge> newEdgeList;
		for (int i = 0; i < edgeList.size(); ++i)
		{
			if (edgeList[i].splitVerts.empty())
			{
				newEdgeList.push_back(WordEdge(edgeList[i], WordEdge::NORMALPOINT, WordEdge::NORMALPOINT));
			}
			else
			{
				edgeList[i].reSortSplitVerts();
				vector<int> splitAttributeList;
				for (auto splitVert : edgeList[i].splitVerts)
				{
					if (splitVert.isEnterPot)
					{
						splitAttributeList.push_back(WordEdge::ENTERPOINT);
					}
					else
					{
						splitAttributeList.push_back(WordEdge::OUTPOINT);
					}
				}

				int lastIndex = splitAttributeList.size() - 1;
				//head
				newEdgeList.push_back(WordEdge(edgeList[i].vertA, edgeList[i].splitVerts[0].intersection, edgeList[i].indexA, -1, WordEdge::NORMALPOINT, splitAttributeList[0]));
				//body
				for (int j = 0; j < splitAttributeList.size() - 1; ++j)
				{
					newEdgeList.push_back(WordEdge(edgeList[i].splitVerts[j].intersection, edgeList[i].splitVerts[j + 1].intersection, -1, -1,
												splitAttributeList[j], splitAttributeList[j + 1]));
				}
				//tail
				newEdgeList.push_back(WordEdge(edgeList[i].splitVerts[lastIndex].intersection, edgeList[i].vertB, -1, edgeList[i].indexB, splitAttributeList[lastIndex], WordEdge::NORMALPOINT));
			}
		}

		this->edgeList.clear();
		this->edgeList = newEdgeList;
		newEdgeList.clear();
	}

public:
	int headIndex;
	int endIndex;
	bool bOutsideCircle = true;
	bool bIsClockWise = true;
	vector<WordEdge> edgeList;
	vector<WordEdge> boundingBoxEdgeList;
	vector<Point3f> loop;
};

struct CommonFuncSet
{
public:
	CommonFuncSet(){}

	FPoint findMinimumDiversionPointFrom(FPoint vertO, Point3m judgeVector, vector<FPoint> &vertSet, Axis axis)
	{
		//y = ax + b
		vector<int> considerScope;
		for (int i = 0; i < vertSet.size(); ++i)
		{
			if (vertSet[i] == vertO)
			{
				continue;
			}
			considerScope.push_back(i);
		}
		if (considerScope.empty())
		{
			return vertO;
		}

		float fMaxDotProduct = -2.0f;
		float tempDotProduct;
		int wantedIndex = -1;
		for (auto i : considerScope)
		{
			Point3m tempV = (vertSet[i].point - vertO.point).Normalize();
			if ((judgeVector ^ tempV) * axis.axisZVector > 0)
			{
				tempDotProduct = tempV * judgeVector;
				if (fMaxDotProduct < tempDotProduct)
				{
					fMaxDotProduct = tempDotProduct;
					wantedIndex = i;
				}
			}
		}

		vector<int>().swap(considerScope);
		if (wantedIndex != -1)
		{
			return vertSet[wantedIndex];
		}
		else
		{
			return vertO;
		}
	}

	pair<FPoint, FPoint> findCommonTangentLineFrom(vector<FPoint> &vertSet, Axis axis)
	{
		Point3m judgeVector;
		judgeVector = (axis.axisXVector * -1 + axis.axisYVector * -0.2f).Normalize();

		float maxProjValue = FLT_MIN, maxProjA = FLT_MIN, maxProjB = FLT_MIN;
		Point3m edge;
		FPoint start, end, frozenStart, frozenVertA, frozenVertB;
		for (auto vert : vertSet)
		{
			edge = vert.point - axis.centerPoint;
			float tempProjValue = edge * judgeVector;
			if (maxProjValue < tempProjValue)
			{
				maxProjValue = tempProjValue;
				start = vert;
			}

			if (vert.bSalient)
			{
				if (maxProjA < tempProjValue)
				{
					maxProjA = tempProjValue;
					frozenVertA = vert;
				}
			}
			else
			{
				if (maxProjB < tempProjValue)
				{
					maxProjB = tempProjValue;
					frozenVertB = vert;
				}
			}
		}

		bool bStop = false, bFound = false;
		while (!bStop)
		{
			end = findMinimumDiversionPointFrom(start, judgeVector, vertSet, axis);
			if (frozenStart == end || start == end)
			{
				bStop = true;
				bFound = false;
			}

			if (start.bSalient != end.bSalient)
			{
				bStop = true;
				bFound = true;
			}
			else
			{
				judgeVector = (end.point - start.point).Normalize();
				start = end;
			}
		}

		pair<FPoint, FPoint> tangentLine;
		if (bFound)
		{
			if (start.bSalient)
			{
				tangentLine.first = start;
				tangentLine.second = end;
			}
			else
			{
				tangentLine.first = end;
				tangentLine.second = start;
			}
		}
		else
		{
			tangentLine.first = frozenVertA;
			tangentLine.second = frozenVertB;
		}

		return tangentLine;
	}
};

struct NeighboringTeethStateRecord
{
public:
	int pullout_tooth_index;
	int start_index_;
	int tooth_num_;
	vector<pair<int, HexaVec>> teeth_vec_list_;
	vector<int> pull_teeth_list_;
	vector<pair<int, bool>> artificial_teeth_list_;
public:
	NeighboringTeethStateRecord(){}
	NeighboringTeethStateRecord(int _tooth_index, int _istart, int _tooth_num, vector<pair<int, HexaVec>> _teeth_list, vector<int> _pull_list, vector<pair<int, bool>> _artificial_list)
	{
		pullout_tooth_index = _tooth_index;
		start_index_ = _istart;
		tooth_num_ = _tooth_num;
		teeth_vec_list_ = _teeth_list;
		pull_teeth_list_ = _pull_list;
		artificial_teeth_list_ = _artificial_list;
	}
	void initial(int _tooth_index, int _istart, int _tooth_num, vector<pair<int, HexaVec>> _teeth_list, vector<int> _pull_list, vector<pair<int, bool>> _artificial_list)
	{
		pullout_tooth_index = _tooth_index;
		start_index_ = _istart;
		tooth_num_ = _tooth_num;
		teeth_vec_list_ = _teeth_list;
		pull_teeth_list_ = _pull_list;
		artificial_teeth_list_ = _artificial_list;
	}
	~NeighboringTeethStateRecord()
	{
		vector<pair<int, HexaVec>>().swap(teeth_vec_list_);
		vector<int>().swap(pull_teeth_list_);
		vector<pair<int, bool>>().swap(artificial_teeth_list_);
	}
};

struct ArtificialToothRecord
{
public:
	int index_;
	bool b_visible_ = true;
public:
	ArtificialToothRecord() {}
	ArtificialToothRecord(int _index)
	{
		this->index_ = _index;
		this->b_visible_ = true;
	}
	ArtificialToothRecord(pair<int, bool> _v)
	{
		this->index_ = _v.first;
		this->b_visible_ = _v.second;
	}
	void setVisible(bool _state)
	{
		b_visible_ = _state;
	}

	ArtificialToothRecord& operator=(const ArtificialToothRecord &e)
	{
		this->index_ = e.index_;
		this->b_visible_ = e.b_visible_;
		return *this;
	}

	bool operator==(const ArtificialToothRecord &e)
	{
		if (this->index_ == e.index_ && this->b_visible_ == e.b_visible_)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};

#endif // !ASSISTGEOMETRY_H

