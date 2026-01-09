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

#include "overlayoutlinepreview.h"
#include <QGuiApplication>

OverlayOutlinePreview::OverlayOutlinePreview(QWidget *parent)
	: QGLWidget(parent)
{
	bGetCurrentVertCP = false;
	bFirstMoveMouse = true;
	bGetUndetermindPoint = false;
}

OverlayOutlinePreview::~OverlayOutlinePreview()
{
}

void OverlayOutlinePreview::initializeGL()
{
	glClearColor(55 /255.0f, 55 / 255.0f, 55 / 255.0f, 0);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
#ifdef Q_OS_WIN
	HDC desk = GetDC(NULL);   // Tested and works, measure DPI
	float hDPI = GetDeviceCaps(desk, LOGPIXELSX);
	float vDPI = GetDeviceCaps(desk, LOGPIXELSY);
	int dpi = (hDPI + vDPI) / 2;
	cur_dpi_ = dpi / 96.0f;
#elif defined(Q_OS_MAC)
	// macOS: use Qt to get screen DPI
	QScreen *screen = QGuiApplication::primaryScreen();
	int dpi = screen->logicalDotsPerInch();
	cur_dpi_ = dpi / 96.0f;
#else
	// Linux and other platforms
	cur_dpi_ = 1.0f;
#endif
	qDebug() << "Overlay Dialog Width : " << width() << endl;
	qDebug() << "Overlay Dialog Height : " << height() << endl;
	if (cur_dpi_ >= 2)
	{
		glViewport(-width() * (cur_dpi_ * 0.5f - 0.5f), height() * (cur_dpi_ * 0.5f - 0.5f), this->width() * cur_dpi_, this->height() * cur_dpi_);
	}
	else
	{
		glViewport(0, 0, this->width(), this->height());
	}
	cursorPoint.X() = 0;
	cursorPoint.Y() = 0;
	cursorPoint.Z() = 0;
	iPickSituation = 0;
	camera = new UdCamera(0, 0, 50, 0, 1, 0);
}

void OverlayOutlinePreview::paintGL()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(25, (GLdouble)this->width() / (GLdouble)this->height(), 0.1, 5000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camera->x, camera->y, camera->z, camera->x, camera->y, 0, camera->headX, camera->headY, camera->headZ);

	if (this->bPickCursorVert)
	{
		cursorPoint = getMixedPointFromScreenToWorld(this->curCursorQPos.x(), this->curCursorQPos.y(), Point3m(0, 0, 1), Point3m(0, 0, 0));
		this->bPickCursorVert = false;
	}

	Point3d screen_proj_origin = unProjectPoint(Point3m(0, 0, 0));
	screen_proj_origin.X() = QTDeviceToLogical(this, screen_proj_origin.X());
	screen_proj_origin.Y() = QTDeviceToLogical(this, screen_proj_origin.Y());

	DrawVertexCouple();
	DrawCoordinate();
	DrawCurrentVertCP();
	DrawColoredPoint(Point3m(0, 0, 0), 255.0f, 255.0f, 255.0f, 5.0f);
	if (bGetUndetermindPoint)
	{
		DrawColoredPoint(undetermindPoint, 0.0f, 0.0f, 255.0f, 10.0f);
	}
	glPopMatrix();
	glPopAttrib();
}

void OverlayOutlinePreview::resizeGL(float w, float h)
{
	setFixedSize(w,h);
	initializeGL();
	this->update();
}

void OverlayOutlinePreview::mousePressEvent(QMouseEvent *event)
{
	event->accept();
	setFocus();
	curCursorQPos = QPoint(QTLogicalToDevice(this, event->pos().x()), QTLogicalToDevice(this, this->height() - event->pos().y()));
	this->bPickCursorVert = true;

	if (bFirstMoveMouse)
	{
		setMouseTracking(true);
		bFirstMoveMouse = false;
	}

	this->setCursor(QCursor(Qt::ClosedHandCursor));
	translateCamera = true;
	startP = vcg::Point3d(event->pos().x(), event->pos().y(), 0.5f);

	updateGL();
}

void OverlayOutlinePreview::mouseMoveEvent(QMouseEvent *event)
{
	event->accept();
	setFocus();
	curCursorQPos = QPoint(QTLogicalToDevice(this, event->pos().x()), QTLogicalToDevice(this, this->height() - event->pos().y()));
	this->bPickCursorVert = true;

	if (translateCamera)
	{
		endP = vcg::Point3d(event->pos().x(), event->pos().y(), 0.5f);
		vcg::Point3d tempTranslateVector = endP - startP;
		camera->x -= tempTranslateVector.X() / 10.0f;
		camera->y += tempTranslateVector.Y() / 10.0f;
		startP = endP;
	}
	else
	{
		if (GetCurrentPoint3m(cursorPoint.X(), cursorPoint.Y(), undetermindPoint))
		{
			this->setCursor(QCursor(Qt::BlankCursor));
		}
		else
		{
			this->setCursor(QCursor(Qt::OpenHandCursor));
		}
	}
	updateGL();
}

void OverlayOutlinePreview::mouseReleaseEvent(QMouseEvent *event)
{
	this->setCursor(QCursor(Qt::OpenHandCursor));
	translateCamera = false;
}

void OverlayOutlinePreview::mouseDoubleClickEvent(QMouseEvent *event)
{
	event->accept();
	setFocus();
	curCursorQPos = QPoint(QTLogicalToDevice(this, event->pos().x()), QTLogicalToDevice(this, this->height() - event->pos().y()));
	this->bPickCursorVert = true;

	if (event->button() == Qt::LeftButton)
	{
		if (!bFirstMoveMouse)//Pick Point
		{
			bGetCurrentVertCP = false;
			GetCurrentVertCouple(cursorPoint.X(), cursorPoint.Y());
		}
	}
	updateGL();
}

void OverlayOutlinePreview::wheelEvent(QWheelEvent * event)
{
	event->accept();
	Point3m tempVector;
	tempVector = cursorPoint - Point3m(camera->x, camera->y, camera->z);
	tempVector = tempVector.Normalize();

	if (event->delta() > 0)
	{
		/*camera->x += tempVector.X() * 3.0f;
		camera->y += tempVector.Y() * 3.0f;*/
		camera->z += tempVector.Z() * 3.0f;
	}
	else
	{
		/*camera->x -= tempVector.X() * 3.0f;
		camera->y -= tempVector.Y() * 3.0f;*/
		camera->z -= tempVector.Z() * 3.0f;
	}
	if (camera->z < 25)
	{
		camera->z = 25;
	}
	else if (camera->z > 495) {
		camera->z = 495;
	}
	cursorPoint = getMixedPointFromScreenToWorld(event->pos().x(), this->height() - event->pos().y(), Point3m(0, 0, 1), Point3m(0, 0, 0));

	updateGL();
}

void OverlayOutlinePreview::keyReleaseEvent(QKeyEvent * e)
{

}

void OverlayOutlinePreview::keyPressEvent(QKeyEvent * e)
{
	e->ignore();
	float addValue = 0.5f;
	if (e->key() == Qt::Key_A)
		camera->x -= addValue;
	if (e->key() == Qt::Key_D)
		camera->x += addValue;
	if (e->key() == Qt::Key_W)
		camera->y += addValue;
	if (e->key() == Qt::Key_S)
		camera->y -= addValue;

	if (e->key() == Qt::Key_Left)
		cursorPoint.X() -= addValue;
	if (e->key() == Qt::Key_Right)
		cursorPoint.X() += addValue;
	if (e->key() == Qt::Key_Up)
		cursorPoint.Y() += addValue;
	if (e->key() == Qt::Key_Down)
		cursorPoint.Y() -= addValue;

	if (e->key() == Qt::Key_I)
		GetCurrentVertCouple(cursorPoint.X(), cursorPoint.Y());

	if (e->key() == Qt::Key_1)
	{
		setCheekSide(RIGHT_SIDE);
	}
	if (e->key() == Qt::Key_2)
	{
		setCheekSide(LEFT_SIDE);
	}

	updateGL();
}

void OverlayOutlinePreview::DrawVertexCouple()
{
	for (int i = 0; i < vertCP.size(); i++)
	{
		DrawColoredLineSegment(vertCP[i].a, vertCP[i].b, vertCP[i].color_.X(), vertCP[i].color_.Y(), vertCP[i].color_.Z(), 2.0f, false);
	}
}

void OverlayOutlinePreview::DrawColoredLineSegment(Point3m p1, Point3m p2, float red, float green, float blue, float size, bool bDrawStipple)
{
	glPushMatrix();
	glLineWidth(size);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	if (bDrawStipple)
	{
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(2.0f, 0x0F0F);
		glLineWidth(size);
		glBegin(GL_LINES);
		glColor3f(red / 255.0f, green / 255.0f, blue / 255.0f);
		glVertex3f(p1.X(), p1.Y(), p1.Z());
		glVertex3f(p2.X(), p2.Y(), p2.Z());
		glEnd();
		glColor3f(1, 1, 1);
		glLineWidth(1.0f);
		glDisable(GL_LINE_STIPPLE);
		glPopMatrix();
	}
	else
	{
		glBegin(GL_LINE_LOOP);
		glColor3f(red / 255.0f, green / 255.0f, blue / 255.0f);
		glVertex3f(p1.X(), p1.Y(), p1.Z());
		glVertex3f(p2.X(), p2.Y(), p2.Z());
		glEnd();
		glColor3f(1, 1, 1);
		glLineWidth(1.0f);
		glPopMatrix();
	}
}

void OverlayOutlinePreview::DrawLabel(Point3m p, QString _str, float red, float green, float blue)
{
	makeCurrent();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	QFont font("Microsoft YaHei", 10);
	setFont(font);
	vcg::Point3d tempProjPoint;
	Point3m nearP, farP;
	tempProjPoint = unProjectPoint(p);
	glColor3f(red / 255.0f, green / 255.0f, blue / 255.0f);
	if (cur_dpi_ >= 2)
	{
		renderText(QTDeviceToLogical(this, tempProjPoint.X()), QTDeviceToLogical(this, this->height() * cur_dpi_ - tempProjPoint.Y()), _str);
	}
	else
	{
		renderText(tempProjPoint.X(), this->height() - tempProjPoint.Y(), _str);
	}
	glPopAttrib();
}

void OverlayOutlinePreview::DrawRect(Point3m p, float _w, float _h, float red, float green, float blue)
{
	makeCurrent();
	Point3m v[4];
	p.Y() -= 0.1f;
	v[0] = p;
	v[1] = p + Point3m(_w, 0, 0);
	v[2] = p + Point3m(_w, _h, 0);
	v[3] = p + Point3m(0, _h, 0);

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glColor3f(red / 255.0f, green / 255.0f, blue / 255.0f);
	glBegin(GL_TRIANGLES);
	glVertex3f(v[0].X(), v[0].Y(), v[0].Z());
	glVertex3f(v[1].X(), v[1].Y(), v[1].Z());
	glVertex3f(v[2].X(), v[2].Y(), v[2].Z());
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex3f(v[0].X(), v[0].Y(), v[0].Z());
	glVertex3f(v[3].X(), v[3].Y(), v[3].Z());
	glVertex3f(v[2].X(), v[2].Y(), v[2].Z());
	glEnd();
	glColor3f(1, 1, 1);
	glPopAttrib();
}

void OverlayOutlinePreview::DrawColoredPoint(Point3m p, float red, float green, float blue, float size)
{
	glPushMatrix();
	glPointSize(size);
	glBegin(GL_POINTS);
	glColor3f(red / 255.0f, green / 255.0f, blue / 255.0f);
	glVertex3f(p.X(), p.Y(), p.Z());
	glEnd();
	glColor3f(1.0f, 1.0f, 1.0f);
	glPointSize(1.0f);
	glPopMatrix();
}

void OverlayOutlinePreview::DrawCoordinate()
{
	float coorLength = 200.0f;
	Point3m right(coorLength, 0, 0), left(-coorLength, 0, 0), up(0, coorLength, 0), down(0, -coorLength, 0);

	Point3m color = Point3m(0.5f * 255.0f, 0.5f * 255.0f, 0.5f * 255.0f);

	DrawColoredLineSegment(up, down, color.X(), color.Y(), color.Z(), 1.0f, false);
	DrawColoredLineSegment(left, right, color.X(), color.Y(), color.Z(), 1.0f, false);

	Point3m pUp(200, 0.5, 0), pDown(200, -0.5, 0), pUpp(195, 0.25, 0), pDownn(195, -0.25, 0);
	for (int i = 0; i < 40; i++)
	{
		DrawColoredLineSegment(pUp, pDown, color.X(), color.Y(), color.Z(), 1.0f, false);
		pUp.X() -= 10.0f;
		pDown.X() -= 10.0f;
	}
	for (int i = 0; i < 39; i++)
	{
		DrawColoredLineSegment(pUpp, pDownn, color.X(), color.Y(), color.Z(), 1.0f, false);
		pUpp.X() -= 10.0f;
		pDownn.X() -= 10.0f;
	}

	Point3m pR(0.5, 200, 0), pL(-0.5, 200, 0), pRr(0.25f, 195, 0), pLl(-0.25f, 195, 0);
	for (int i = 0; i < 40; i++)
	{
		DrawColoredLineSegment(pR, pL, color.X(), color.Y(), color.Z(), 1.0f, false);
		pR.Y() -= 10.0f;
		pL.Y() -= 10.0f;
	}
	for (int i = 0; i < 39; i++)
	{
		DrawColoredLineSegment(pRr, pLl, color.X(), color.Y(), color.Z(), 1.0f, false);
		pRr.Y() -= 10.0f;
		pLl.Y() -= 10.0f;
	}
}

void OverlayOutlinePreview::DrawCursorLine()
{
	Point3m tp1, tp2;
	tp1.X() = cursorPoint.X() - 100.0f; tp1.Y() = cursorPoint.Y(); tp1.Z() = 0;
	tp2.X() = cursorPoint.X() + 100.0f; tp2.Y() = cursorPoint.Y(); tp2.Z() = 0;
	DrawColoredLineSegment(tp1, tp2, 230.0f, 255.0f, 0, 1.0f, true);
	tp1.X() = cursorPoint.X(); tp1.Y() = cursorPoint.Y() + 100; tp1.Z() = 0;
	tp2.X() = cursorPoint.X(); tp2.Y() = cursorPoint.Y() - 100; tp2.Z() = 0;
	DrawColoredLineSegment(tp1, tp2, 230.0f, 255.0f, 0, 1.0f, true);
}

void OverlayOutlinePreview::DrawCurrentVertCP()
{
	if (iPickSituation == 1)
	{
		if (bGetUndetermindPoint)
		{
			DrawColoredLineSegment(pa, undetermindPoint, 130.0f, 200.0f, 255.0f, 1.0f, true);
		}
		else
		{
			DrawColoredLineSegment(pa, cursorPoint, 130.0f, 200.0f, 255.0f, 1.0f, true);
		}
	}
	if (bGetCurrentVertCP)
	{
		bool uptilted = true;
		float x, y, z, mid_x, mid_y;
		if (pa.X() > pb.X())
		{
			x = pa.X();
			y = pb.Y();

			if (y > pa.Y())
			{
				uptilted = true;
			}
			else
			{
				uptilted = false;
			}
		}
		else
		{
			x = pb.X();
			y = pa.Y();

			if (y > pb.Y())
			{
				uptilted = true;
			}
			else
			{
				uptilted = false;
			}
		}
		z = 0;
		mid_x = (pa.X() + pb.X()) * 0.5f;
		mid_y = (pa.Y() + pb.Y()) * 0.5f;

		DrawColoredLineSegment(pa, pb, 130.0f, 200.0f, 255.0f, 3.0f, false);
		AnalysisVertCouple();
		if (fVerticalHeight - (int)fVerticalHeight == 0)
		{
			renderText(10, 30, tr("overbite: ") + QString::number(fVerticalHeight) + ".00mm");
		}
		else
		{
			renderText(10, 30, tr("overbite: ") + QString::number(fVerticalHeight) + "mm");
		}
		if (fHorizontalWidth - (int)fHorizontalWidth == 0)
		{
			renderText(10, 60,tr("overjet: ")+ QString::number(fHorizontalWidth) + ".00mm");
		}
		else
		{
			renderText(10, 60, tr("overjet: ") + QString::number(fHorizontalWidth) + "mm");
		}

		DrawColoredLineSegment(pa, Point3m(x, y, z), 66.0f, 204.0f, 82.0f, 2.0f, false);
		DrawColoredLineSegment(pb, Point3m(x, y, z), 66.0f, 204.0f, 82.0f, 2.0f, false);

		glPushAttrib(GL_ALL_ATTRIB_BITS);
		vcg::Point3d screen_pa;
		screen_pa = unProjectPoint(Point3m(pa.X(), pa.Y(), pa.Z()));
		glPopAttrib();

		float adj_value = 0.5f;
		QFont font("Microsoft YaHei", 10);
		QFontMetrics fm(font);
		float screen_z = screen_pa.Z();
		if (uptilted)
		{
			QRect rect = fm.boundingRect(QString::number(fHorizontalWidth).append("mm"));
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			vcg::Point3d tempProjPoint;
			tempProjPoint = projectPoint(Point3m(QTLogicalToDevice(this, rect.width()), QTLogicalToDevice(this, rect.height()), screen_z));
			vcg::Point3d tempProjPoint2;
			tempProjPoint2 = projectPoint(Point3m(0, 0, screen_z));
			glPopAttrib();

			float width = abs(tempProjPoint.X() - tempProjPoint2.X());
			float height = abs(tempProjPoint.Y() - tempProjPoint2.Y());

			DrawRect(Point3m(mid_x - width / 2.0f, y + adj_value, z), width, height, 55, 55, 55);
			DrawLabel(Point3m(mid_x - width / 2.0f, y + adj_value, z), QString::number(fHorizontalWidth).append("mm"), 66.0f, 204.0f, 82.0f);
		}
		else
		{
			QRect rect = fm.boundingRect(QString::number(fHorizontalWidth).append("mm"));
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			vcg::Point3d tempProjPoint;
			tempProjPoint = projectPoint(Point3m(QTLogicalToDevice(this, rect.width()), QTLogicalToDevice(this, rect.height()), screen_z));
			vcg::Point3d tempProjPoint2;
			tempProjPoint2 = projectPoint(Point3m(0, 0, screen_z));
			glPopAttrib();

			float width = abs(tempProjPoint.X() - tempProjPoint2.X());
			float height = abs(tempProjPoint.Y() - tempProjPoint2.Y());

			DrawRect(Point3m(mid_x - width / 2.0f, y - height - adj_value, z), width, height, 55, 55, 55);
			DrawLabel(Point3m(mid_x - width / 2.0f, y - height - adj_value, z), QString::number(fHorizontalWidth).append("mm"), 66.0f, 204.0f, 82.0f);
		}
		QRect rect = fm.boundingRect(QString::number(fVerticalHeight).append("mm"));
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		vcg::Point3d tempProjPoint;
		tempProjPoint = projectPoint(Point3m(QTLogicalToDevice(this, rect.width()), QTLogicalToDevice(this, rect.height()), screen_z));
		vcg::Point3d tempProjPoint2;
		tempProjPoint2 = projectPoint(Point3m(0, 0, screen_z));
		glPopAttrib();

		float width = abs(tempProjPoint.X() - tempProjPoint2.X());
		float height = abs(tempProjPoint.Y() - tempProjPoint2.Y());

		DrawRect(Point3m(x + adj_value, mid_y - height / 2.0f, z), width, height, 55, 55, 55);
		DrawLabel(Point3m(x + adj_value, mid_y - height / 2.0f, z), QString::number(fVerticalHeight).append("mm"), 66.0f, 204.0f, 82.0f);
	}
	else
	{
		renderText(10, 30, tr("Please double click LMB"));
		renderText(10, 60, tr("on the section  of maxillaryand mandible"));
	}
}

void OverlayOutlinePreview::leaveEvent(QEvent* e)
{
	bGetUndetermindPoint = false;
	if (iPickSituation == 1)
	{
		iPickSituation = 0;
		bGetCurrentVertCP = false;
		pa = Point3m(0, 0, 0);
		pb = Point3m(0, 0, 0);
	}
	this->update();
}

void OverlayOutlinePreview::GetCurrentVertCouple(float x, float y)
{
	if (iPickSituation == 0)
	{
		if (bGetUndetermindPoint)
		{
			pa.X() = undetermindPoint.X();
			pa.Y() = undetermindPoint.Y();
			pa.Z() = 0;

			if (undetermind_point_type_ == 4 || undetermind_point_type_ == 0)
			{
				b_pickon_upper_a_ = true;
			}
			else if (undetermind_point_type_ == 5 || undetermind_point_type_ ==  1)
			{
				b_pickon_upper_a_ = false;
			}
			iPickSituation = 1;
			bGetCurrentVertCP = false;

			computeOverlayStateParameters(true);
		}
	}
	else if (iPickSituation == 1)
	{
		if (bGetUndetermindPoint)
		{
			pb.X() = undetermindPoint.X();
			pb.Y() = undetermindPoint.Y();
			pb.Z() = 0;

			if (undetermind_point_type_ == 4 || undetermind_point_type_ == 0)
			{
				b_pickon_upper_b_ = true;
			}
			else if (undetermind_point_type_ == 5 || undetermind_point_type_ == 1)
			{
				b_pickon_upper_b_ = false;
			}

			iPickSituation = 0;
			bGetCurrentVertCP = true;

			if (b_pickon_upper_a_ != b_pickon_upper_b_)
			{
				computeOverlayStateParameters();
			}
		}
	}
}

bool OverlayOutlinePreview::GetCurrentPoint3m(float x, float y, Point3m &point)
{
	float pickRadius = 1.0f;
	Point3m temp_undermind_vert;
	vector<std::pair<float, Point3m>> candidate_points;
	vector<int> candidate_types;

	for (int i = 0; i < vertCP.size(); i++)
	{
		int type = vertCP[i].type_;

		if (iPickSituation == 1)
		{
			if (b_pickon_upper_a_)
			{
				if (type != 5 && type != 1)
				{
					continue;
				}
			}
			else
			{
				if (type != 4 && type != 0)
				{
					continue;
				}
			}
		}
		else
		{
			if (type != 4 && type != 5 && type != 0 && type != 1)
			{
				continue;
			}
		}

		float xRight, xLeft, yUp, yDown;
		float x1, y1, x2, y2;
		float a, b;

		x1 = vertCP[i].a.X();
		y1 = vertCP[i].a.Y();
		x2 = vertCP[i].b.X();
		y2 = vertCP[i].b.Y();

		if (vertCP[i].a.X() < vertCP[i].b.X())
		{
			xRight = vertCP[i].b.X();
			xLeft = vertCP[i].a.X();
		}
		else
		{
			xLeft = vertCP[i].b.X();
			xRight = vertCP[i].a.X();
		}
		if (vertCP[i].a.Y() < vertCP[i].b.Y())
		{
			yUp = vertCP[i].b.Y();
			yDown = vertCP[i].a.Y();
		}
		else
		{
			yDown = vertCP[i].b.Y();
			yUp = vertCP[i].a.Y();
		}

		if (x2 - x1 == 0)
		{
			if (y <= yUp && y >= yDown)
			{
				float standerdX = x2;
				temp_undermind_vert.X() = standerdX;
				temp_undermind_vert.Y() = y;
				temp_undermind_vert.Z() = 0;
				if (abs(standerdX - x) < pickRadius)
				{
					candidate_points.push_back(make_pair(abs(standerdX - x), temp_undermind_vert));
					candidate_types.push_back(type);
				}
			}
		}
		else
		{
			a = (y2 - y1) / (x2 - x1);
			b = y1 - a * x1;

			if (abs(a) <= 1.0f)
			{
				if (x <= xRight && x >= xLeft)
				{
					float standerdY;
					standerdY = a * x + b;

					temp_undermind_vert.X() = x;
					temp_undermind_vert.Y() = standerdY;
					temp_undermind_vert.Z() = 0;
					if (abs(standerdY - y) < pickRadius)
					{
						candidate_points.push_back(make_pair(abs(standerdY - y), temp_undermind_vert));
						candidate_types.push_back(type);
					}
				}
			}
			else
			{
				if (y <= yUp && y >= yDown)
				{
					float standerdX;
					standerdX = (y - b) / a;

					temp_undermind_vert.X() = standerdX;
					temp_undermind_vert.Y() = y;
					temp_undermind_vert.Z() = 0;
					if (abs(standerdX - x) < pickRadius)
					{
						candidate_points.push_back(make_pair(abs(standerdX - x), temp_undermind_vert));
						candidate_types.push_back(type);
					}
				}
			}
		}
	}

	if (!candidate_points.empty())
	{
		float min_distance = FLT_MAX;
		Point3m min_point;
		int min_index = 0;

		int i = 0;
		for (auto& data : candidate_points)
		{
			if (data.first < min_distance)
			{
				min_distance = data.first;
				min_point = data.second;
				min_index = i;
			}
			++i;
		}
		this->undetermindPoint = min_point;
		this->undetermind_point_type_ = candidate_types[min_index];
		bGetUndetermindPoint = true;
		return true;
	}
	else
	{
		bGetUndetermindPoint = false;
		return false;
	}
}

void OverlayOutlinePreview::AnalysisVertCouple()
{
	fHorizontalWidth = abs(pa.X() - pb.X());
	fVerticalHeight = abs(pa.Y() - pb.Y());

	int tempNum;
	tempNum = (int)(fHorizontalWidth * 100);
	fHorizontalWidth = tempNum / 100.0f;

	tempNum = (int)(fVerticalHeight * 100);
	fVerticalHeight = tempNum / 100.0f;
}

void OverlayOutlinePreview::getNewVertCouple(const vector<VertCP> &cpList)
{
	if (bSwitchCheekSideJustNow)
	{
		if (cheek_side_ == RIGHT_SIDE && this->right_measure_edge_ == nullptr)
		{
			bGetCurrentVertCP = false;
		}
		else if (cheek_side_ == LEFT_SIDE && this->left_measure_edge_ == nullptr)
		{
			bGetCurrentVertCP = false;
		}
		bSwitchCheekSideJustNow = false;
	}
	else
	{
		bGetCurrentVertCP = false;
		this->pa = Point3m(0, 0, 0);
		this->pb = Point3m(0, 0, 0);
	}

	vector<VertCP>().swap(vertCP);
	vertCP = cpList;
	computeCurrentToothLength();
	computeOverlayStateParameters();
	this->update();
}

void OverlayOutlinePreview::setCheekSide(CheekSide _side)
{
	this->cheek_side_ = _side;
	emit setCheekSideSignal(_side);
	bSwitchCheekSideJustNow = true;
	b_pickon_upper_a_ = true;
	if (_side == RIGHT_SIDE)
	{
		if (right_measure_edge_ != nullptr)
		{
			pa = right_measure_edge_->a;
			pb = right_measure_edge_->b;
			bGetCurrentVertCP = true;
		}
		else
		{
			pa = Point3m(0, 0, 0);
			pb = Point3m(0, 0, 0);
			bGetCurrentVertCP = false;
		}
	}
	else if (_side == LEFT_SIDE)
	{
		if (left_measure_edge_ != nullptr)
		{
			pa = left_measure_edge_->a;
			pb = left_measure_edge_->b;
			bGetCurrentVertCP = true;
		}
		else
		{
			pa = Point3m(0, 0, 0);
			pb = Point3m(0, 0, 0);
			bGetCurrentVertCP = false;
		}
	}

	Point3m pcenter = (pa + pb) * 0.5f;
	camera->x = pcenter.X();
	camera->y = pcenter.Y();

	computeOverlayStateParameters();
	updateGL();
}

void OverlayOutlinePreview::computeCurrentToothLength()
{
	float min_y, max_y;
	vector<VertCP> upper_tooth_cps, lower_tooth_cps;
	for (int i = 0; i < vertCP.size(); ++i)
	{
		if (vertCP[i].type_ == 4 || vertCP[i].type_ == 0)
		{
			upper_tooth_cps.push_back(vertCP[i]);
		}
		if (vertCP[i].type_ == 5 || vertCP[i].type_ == 1)
		{
			lower_tooth_cps.push_back(vertCP[i]);
		}
	}
	if (!upper_tooth_cps.empty())
	{
		min_y = upper_tooth_cps[0].a.Y();
		max_y = upper_tooth_cps[0].a.Y();
		for (auto& cp : upper_tooth_cps)
		{
			if (min_y > cp.a.Y())
			{
				min_y = cp.a.Y();
			}
			if (min_y > cp.b.Y())
			{
				min_y = cp.b.Y();
			}
			if (max_y < cp.a.Y())
			{
				max_y = cp.a.Y();
			}
			if (max_y < cp.b.Y())
			{
				max_y = cp.b.Y();
			}
		}
		upper_tooth_length_ = abs(max_y - min_y);
	}

	if (!lower_tooth_cps.empty())
	{
		min_y = lower_tooth_cps[0].a.Y();
		max_y = lower_tooth_cps[0].a.Y();
		for (auto& cp : lower_tooth_cps)
		{
			if (min_y > cp.a.Y())
			{
				min_y = cp.a.Y();
			}
			if (min_y > cp.b.Y())
			{
				min_y = cp.b.Y();
			}
			if (max_y < cp.a.Y())
			{
				max_y = cp.a.Y();
			}
			if (max_y < cp.b.Y())
			{
				max_y = cp.b.Y();
			}
		}
		lower_tooth_length_ = abs(max_y - min_y);
	}
}

void OverlayOutlinePreview::computeOverlayStateParameters(bool _clear_cur_side /*=false*/)
{
	Point3m upper_vert, lower_vert;
	Point3m overjet_color(255, 0, 0);
	Point3m overbite_color(255, 0, 0);
	if (b_pickon_upper_a_)
	{
		upper_vert = pa;
		lower_vert = pb;
	}
	else
	{
		upper_vert = pb;
		lower_vert = pa;
	}
	Point3m vec;
	if (_clear_cur_side)
	{
		vec = Point3m(0, 0, 0);
	}
	else
	{
		vec = upper_vert - lower_vert;
	}

	if (cheek_side_ == RIGHT_SIDE)
	{
		overjet_right_value_ = vec.X();
		overbite_right_value_ = vec.Y();
		if (right_measure_edge_ != nullptr)
		{
			delete right_measure_edge_;
			right_measure_edge_ = nullptr;
		}
		if (vec.X() != 0 || vec.Y() != 0)
		{
			this->right_measure_edge_ = new VertCP(upper_vert, lower_vert, RIGHT_SIDE);
		}
	}
	else if (cheek_side_ == LEFT_SIDE)
	{
		overjet_left_value_ = vec.X();
		overbite_left_value_ = vec.Y();
		if (left_measure_edge_ != nullptr)
		{
			delete left_measure_edge_;
			left_measure_edge_ = nullptr;
		}
		if (vec.X() != 0 || vec.Y() != 0)
		{
			this->left_measure_edge_ = new VertCP(upper_vert, lower_vert, LEFT_SIDE);
		}
	}

	//OVERJET
	float acutal_overjet_value = 0;
	if (overjet_left_value_ != 0 && overjet_right_value_ != 0)
	{
		float cur_overjet_vlaue = (overjet_left_value_ + overjet_right_value_) * 0.5f;
		int temp_value = (cur_overjet_vlaue * 100) / (int)1;
		cur_overjet_vlaue = temp_value / 100.0f;

		acutal_overjet_value = cur_overjet_vlaue;
		overjet_value_ = abs(cur_overjet_vlaue);
	}
	else if (overjet_left_value_ != 0 && overjet_right_value_ == 0)
	{
		float cur_overjet_vlaue = overjet_left_value_;
		int temp_value = (cur_overjet_vlaue * 100) / (int)1;
		cur_overjet_vlaue = temp_value / 100.0f;

		acutal_overjet_value = cur_overjet_vlaue;
		overjet_value_ = abs(cur_overjet_vlaue);
	}
	else if (overjet_right_value_ != 0 && overjet_left_value_ == 0)
	{
		float cur_overjet_vlaue = overjet_right_value_;
		int temp_value = (cur_overjet_vlaue * 100) / (int)1;
		cur_overjet_vlaue = temp_value / 100.0f;

		acutal_overjet_value = cur_overjet_vlaue;
		overjet_value_ = abs(cur_overjet_vlaue);
	}
	else
	{
		acutal_overjet_value = -1;
		overjet_value_ = -1;
	}

	if (overjet_value_ <= 0)
	{
		overjet_rank_ = QString(tr(""));
		overjet_color = Point3m(255, 255, 0);
	}
	else if (0 < overjet_value_ && overjet_value_ <= 3.0f)
	{
		overjet_rank_ = QString("Normal overjet");
		overjet_color = Point3m(255, 255, 0);
	}
	else if (3.0f < overjet_value_ && overjet_value_ <= 5.0f)
	{
		overjet_rank_ = QString("I-degree overjet");
	}
	else if (5.0f < overjet_value_ && overjet_value_ <= 8.0f)
	{
		overjet_rank_ = QString("II-degree overjet");
	}
	else
	{
		overjet_rank_ = QString("III-degree overjet");
	}

	if (acutal_overjet_value > 0)
	{
		overjet_rank_ = QString("Crossbite");
		overjet_color = Point3m(255, 0, 0);
	}

	//OVERBITE
	float actual_overbite_value = 0;
	if (overbite_left_value_ != 0 && overbite_right_value_ != 0)
	{
		float cur_overbite_vlaue = (overbite_left_value_ + overbite_right_value_) * 0.5f;
		int temp_value = (cur_overbite_vlaue * 100) / (int)1;
		cur_overbite_vlaue = temp_value / 100.0f;

		actual_overbite_value = cur_overbite_vlaue;
		overbite_value_ = abs(cur_overbite_vlaue);
	}
	else if (overbite_left_value_ != 0 && overbite_right_value_ == 0)
	{
		float cur_overbite_vlaue = overbite_left_value_;
		int temp_value = (cur_overbite_vlaue * 100) / (int)1;
		cur_overbite_vlaue = temp_value / 100.0f;

		actual_overbite_value = cur_overbite_vlaue;
		overbite_value_ = abs(cur_overbite_vlaue);
	}
	else if (overbite_right_value_ != 0 && overbite_left_value_ == 0)
	{
		float cur_overbite_vlaue = overbite_right_value_;
		int temp_value = (cur_overbite_vlaue * 100) / (int)1;
		cur_overbite_vlaue = temp_value / 100.0f;

		actual_overbite_value = cur_overbite_vlaue;
		overbite_value_ = abs(cur_overbite_vlaue);
	}
	else
	{
		actual_overbite_value = 0;
		overbite_value_ = 0;
		overbite_color = Point3m(255, 255, 0);
	}

	{
		if (actual_overbite_value > 0)
		{
			overbite_rank_ = QString("Open bite");
			overbite_color = Point3m(255, 0, 0);
		}
		else if (actual_overbite_value < 0)
		{
			if (acutal_overjet_value > 0)
			{
				overbite_rank_ = QString("Crossbite");
				overbite_color = Point3m(255, 0, 0);
			}
			else
			{
				overbite_rank_ = QString("Normal overbite");
				overbite_color = Point3m(255, 255, 0);
			}
		}
		else
		{
			overbite_value_ = -1;
			overbite_rank_ = QString(tr(""));
		}
	}

	updateLeftAndRightParameters(overbite_left_value_, overjet_left_value_, overbite_right_value_, overjet_right_value_);
	updateOverlayStateSignal(overjet_value_, overjet_rank_, overjet_color, overbite_value_, overbite_rank_, overbite_color);
}

void OverlayOutlinePreview::loadLeftMeasureRecord(FEdge  _edge)
{
	b_pickon_upper_a_ = true;
	cheek_side_ = LEFT_SIDE;
	pa = _edge.vertA;
	pb = _edge.vertB;
	computeOverlayStateParameters();
}

void OverlayOutlinePreview::loadRighttMeasureRecord(FEdge  _edge)
{
	b_pickon_upper_a_ = true;
	cheek_side_ = RIGHT_SIDE;
	pa = _edge.vertA;
	pb = _edge.vertB;
	computeOverlayStateParameters();
}

vcg::Point3d OverlayOutlinePreview::projectPoint(Point3m p)
{
	// Get gl state values
	double mm[16], mp[16];
	GLint vp[4];
	glPushMatrix();

	glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	glGetDoublev(GL_PROJECTION_MATRIX, mp);
	glGetIntegerv(GL_VIEWPORT, vp);
	vcg::Point3d result(0, 0, 0);
	gluUnProject(p[0], p[1], p[2], mm, mp, vp, &result[0], &result[1], &result[2]);
	glPopMatrix();

	return result;
}

vcg::Point3d OverlayOutlinePreview::unProjectPoint(Point3m p)
{
	// Get gl state values
	double mm[16], mp[16];
	GLint vp[4];
	glPushMatrix();

	glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	glGetDoublev(GL_PROJECTION_MATRIX, mp);
	glGetIntegerv(GL_VIEWPORT, vp);
	vcg::Point3d result(0, 0, 0);
	gluProject(p[0], p[1], p[2], mm, mp, vp, &result[0], &result[1], &result[2]);
	glPopMatrix();

	return result;
}

bool OverlayOutlinePreview::computeCrossPoint(Point3m startP, Point3m endP, Point3m cutFaceNormalV, Point3m cutFacePosP, Point3m &CrossPoint)
{
	// 1. Check if two endpoints of line segment are on opposite sides of plane, if yes continue calculation, otherwise return false
	float fPointMulit;
	Point3m sideVector1, sideVector2;
	sideVector1 = startP - cutFacePosP;
	sideVector2 = endP - cutFacePosP;
	if ((sideVector1 * cutFaceNormalV) * (sideVector2 * cutFaceNormalV) > 0) // Two points on the same side of plane
		return false;

	Point3m cs, se;
	cs = startP - cutFacePosP;
	se = endP - startP;
	float base = se *cutFaceNormalV;
	if (base == 0)
		return false;

	float t = -(cs * cutFaceNormalV) / base;
	if (0 <= t && t <= 1)
	{
		CrossPoint = startP + se * t;
		return true;
	}
	return false;
}

Point3m OverlayOutlinePreview::getMixedPointFromScreenToWorld(float mX, float mY, Point3m nV, Point3m hP)
{
	Point3m resultP;
	vcg::Point3d tempProjPoint;
	Point3m nearP, farP;

	tempProjPoint = projectPoint(Point3m(mX, mY, 0));
	nearP.X() = tempProjPoint.X();
	nearP.Y() = tempProjPoint.Y();
	nearP.Z() = tempProjPoint.Z();
	tempProjPoint = projectPoint(Point3m(mX, mY, 1));
	farP.X() = tempProjPoint.X();
	farP.Y() = tempProjPoint.Y();
	farP.Z() = tempProjPoint.Z();

	if (computeCrossPoint(nearP, farP, nV, hP, resultP))
		return resultP;
	else
		return hP;
}
