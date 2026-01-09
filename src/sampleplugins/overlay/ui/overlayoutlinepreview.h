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

#ifndef OVERLAYOUTLINEPREVIEW_H
#define OVERLAYOUTLINEPREVIEW_H

#include <common_ext/data/fusionaligndata.h>
#include <QGLWidget>
#include <QScreen>
#include <qcursor.h>
#include <qevent.h>
#include <math.h>
#include <common/ml_mesh_type.h>
//#include <GL/GLU.h>
#include <GL/glew.h>
#include <wrap/qt/gl_label.h>
#include <common_ext/util/assist_geometry.h>

using namespace std;

struct UdCamera {
	float x, y, z;
	float headX, headY, headZ;

	Point3m camPos, starePos;
	Point3m frontMoveVec, rightMoveVec, upMoveVec;
	float fHorizontalDegree, fVerticalDegree;
	UdCamera()
	{
		x = 0; y = 0; z = 50;
		headX = 0; headY = 1; headZ = 0;
	}
	UdCamera(float _x, float _y, float _z, float hx, float hy, float hz)
	{
		x = _x; y = _y; z = _z;
		headX = hx; headY = hy; headZ = hz;
	}
	UdCamera(Point3m _camPos, Point3m _starePos)
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

struct VertCP {
	vcg::Color4b color_;
	Point3m a, b;
	int type_;

	VertCP() {}
	VertCP(Point3m _a, Point3m _b, int _type)
	{
		a = _a;
		b = _b;
		color_ = vcg::Color4b::White;
		type_ = _type;
	}
	VertCP(const VertCP &_cp)
	{
		a = _cp.a;
		b = _cp.b;
		color_ = _cp.color_;
		type_ = _cp.type_;
	}

	void Init(Point3m _a, Point3m _b, vcg::Color4b _color, int _type)
	{
		a = _a;
		b = _b;
		color_ = _color;
		type_ = _type;
	}
};

class OverlayOutlinePreview : public QGLWidget
{
	Q_OBJECT

public:
	enum  CheekSide{RIGHT_SIDE, LEFT_SIDE};

	OverlayOutlinePreview(QWidget *parent);
	~OverlayOutlinePreview();
	void resizeGL(float w, float h);

protected:
	void initializeGL();
	void paintGL();
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent * event);
	void keyReleaseEvent(QKeyEvent * e);
	void keyPressEvent(QKeyEvent * e);
	void leaveEvent(QEvent *e);

	void DrawVertexCouple();
	void DrawColoredLineSegment(Point3m p1, Point3m p2, float red, float green, float blue, float size, bool bDrawStipple);
	void DrawRect(Point3m p, float _w, float _h, float red, float green, float blue);
	void DrawLabel(Point3m p, QString _str, float red, float green, float blue);
	void DrawColoredPoint(Point3m p, float red, float green, float blue, float size);
	void DrawCoordinate();
	void DrawCursorLine();
	void DrawCurrentVertCP();

	Point3m getMixedPointFromScreenToWorld(float mX, float mY, Point3m nV, Point3m hP);
	vcg::Point3d projectPoint(Point3m p);
	vcg::Point3d unProjectPoint(Point3m p);
	bool computeCrossPoint(Point3m startP, Point3m endP, Point3m cutFaceNormalV, Point3m cutFacePosP, Point3m &CrossPoint);
	void computeCurrentToothLength();
	void computeOverlayStateParameters(bool _clear_cur_side = false);

	bool bGetCurrentVertCP;
	bool translateCamera = false;
	bool bPickCursorVert = false;
	QPoint curCursorQPos;
	vcg::Point3d startP, endP;
	Point3m cursorPoint;
	int iPickSituation;
	bool bSwitchCheekSideJustNow = false;

public:
	vector<VertCP> vertCP; // Accepted tooth outline
	UdCamera *camera;  // Camera coordinates
	float fHorizontalWidth, fVerticalHeight; // Current distance analysis result
	Point3m pa, pb; // Two points to be stored in currentCP
	VertCP *left_measure_edge_ = nullptr, *right_measure_edge_ = nullptr;
	bool b_pickon_upper_a_ = false, b_pickon_upper_b_ = false;
	Point3m undetermindPoint;
	bool bGetUndetermindPoint;
	int undetermind_point_type_;
	bool bFirstMoveMouse;
	VertCP *currentCP; // Current distance analysis variable
	float upper_tooth_length_, lower_tooth_length_;

	float overjet_left_value_ = 0, overbite_left_value_ = 0, overjet_right_value_ = 0, overbite_right_value_ = 0;
	float overjet_value_ = 0, overbite_value_ = 0;
	QString overjet_rank_, overbite_rank_;
	CheekSide cheek_side_ = RIGHT_SIDE;
	float cur_dpi_ = 1;

	void GetCurrentVertCouple(float x, float y);
	bool GetCurrentPoint3m(float x, float y, Point3m &point);
	void AnalysisVertCouple();

signals:
	void updateOverlayStateSignal(float _overjet_value, QString _overjet_rank, Point3m _overjet_color, float _overbite_value, QString _overbite_rank, Point3m _overbite_color);
	void setCheekSideSignal(CheekSide);
	void updateLeftAndRightParameters(float _left_overbite, float _left_overjet, float _right_overbite, float _right_overjet);

public slots:
	void getNewVertCouple(const vector<VertCP> &cpList);
	void setCheekSide(CheekSide _side);
	void loadLeftMeasureRecord(FEdge  _edge);
	void loadRighttMeasureRecord(FEdge  _edge);
};

#endif // !OVERLAYOUTLINEPREVIEW_H
