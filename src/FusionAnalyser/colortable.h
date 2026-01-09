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

#ifndef COLORTABLE_H
#define COLORTABLE_H
//#include <GL/GLU.h>
#include <GL/glew.h>
#include <QWidget>
#include <QGLWidget>
#include <qcursor.h>
#include <qevent.h>
#include <math.h>
#include <common/ml_mesh_type.h>

using namespace std;

struct UdCameraA {
	float x, y, z;
	float headX, headY, headZ;

	Point3m camPos, starePos;
	Point3m frontMoveVec, rightMoveVec, upMoveVec;
	float fHorizontalDegree, fVerticalDegree;
	UdCameraA()
	{
		x = 0; y = 0; z = 50;
		headX = 0; headY = 1; headZ = 0;
	}
	UdCameraA(float _x, float _y, float _z, float hx, float hy, float hz)
	{
		x = _x; y = _y; z = _z;
		headX = hx; headY = hy; headZ = hz;
	}
	UdCameraA(Point3m _camPos, Point3m _starePos)
	{
		camPos = _camPos;
		starePos = _starePos;
		UpdateVectors();
	}
	void Inital(Point3m _camPos, Point3m _starePos)
	{
		camPos = _camPos;
		starePos = _starePos;
		UpdateVectors();
	}
	void UpdateVectors()
	{
		frontMoveVec = starePos - camPos;
		if ((camPos.Y() - starePos.Y()) != 0)
		{
			upMoveVec.X() = starePos.X() - camPos.X();
			upMoveVec.Z() = starePos.Z() - camPos.Z();
			upMoveVec.Y() = (float)(((camPos.X() - starePos.X())*(camPos.X() - starePos.X()) + (camPos.Z() - starePos.Z())*(camPos.Z() - starePos.Z())) / (camPos.Y() - starePos.Y()));
		}
		else
		{
			upMoveVec.X() = 0;
			upMoveVec.Z() = 0;
			upMoveVec.Y() = 1;
		}
		float l, m, n, o, p, q;
		l = upMoveVec.X(); m = upMoveVec.Y(); n = upMoveVec.Z();
		o = frontMoveVec.X(); p = frontMoveVec.Y(); q = frontMoveVec.Z();
		rightMoveVec = Point3m(m*q - n*p, n*o - l*q, l*p - m*o);

		frontMoveVec = frontMoveVec.Normalize();
		upMoveVec = upMoveVec.Normalize();
		rightMoveVec = rightMoveVec.Normalize();
	}
};

namespace Ui {
class ColorTable;
}

class ColorTable : public QGLWidget
{
    Q_OBJECT

public:
    explicit ColorTable(QWidget *parent = 0);
    ~ColorTable();

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(float w, float h);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent * event);
	void keyReleaseEvent(QKeyEvent * e);
	void keyPressEvent(QKeyEvent * e);

	Point3m computeColorByDistance(float fDistance);
	Point3m updateTopColor();
	void updateNumberSign();

	bool translateCamera = false;
	vcg::Point3d startP, endP;
	Point3m cursorPoint;
	UdCameraA *camera;  // 相机坐标
signals:
	void setNewTopDistance(float num, float sum, Point3m color);

public slots:
	void getColorTableParams(float fenshu, float max);
	void getColorFromColorTableSlot(float radio, Point3m& color);
private:
    Ui::ColorTable *ui;

private:
	float fRatio = 1;
	float fMaxium = 5;
	float fTopNum = 5;
	QPoint cursor;
	float x = 0, y = 0;
	Point3m topColor = Point3m(0, 0, 1);
	vector<float> scals;
	Point3m colorPicked;
};

#endif // COLORTABLE_H
