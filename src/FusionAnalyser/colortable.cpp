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

#include "colortable.h"
#include "ui_colortable.h"

ColorTable::ColorTable(QWidget *parent) :
    QGLWidget(parent),
    ui(new Ui::ColorTable)
{
    ui->setupUi(this);
	x = -0.39;
	y = 0;
}

ColorTable::~ColorTable()
{
    delete ui;
}

void ColorTable::initializeGL()
{
	//widgetͳߴ
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(29 / 255.0f, 35 / 255.0f, 73 / 255.0f, 1);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);//͸У
	cursorPoint.X() = 0;
	cursorPoint.Y() = 0;
	cursorPoint.Z() = 0;
	camera = new UdCameraA(77, 45, 250, 0, 1, 0);

	this->topColor = updateTopColor();
	updateNumberSign();
}

void ColorTable::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(25, 1920.0f / 1080.0f, 0.1, 500);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camera->x, camera->y, camera->z, camera->x, camera->y, 0, camera->headX, camera->headY, camera->headZ);

	Point3m p1, p2;
	float t = 0;
	p1 = Point3m(0, 0, 0);
	p2 = Point3m(0, 100, 0);
	if (this->fRatio <= 0.5f)
	{
		glPushMatrix();
		float unitAdd = 0.00001;
		for (int i = 0; i < 100; i++)
		{
			glBegin(GL_LINES);
			glLineWidth(1.0f);
			glColor3f(1, 0, 0);
			glVertex3f(p1.X(), p1.Y(), p1.Z());
			glColor3f(topColor.X(), topColor.Y(), topColor.Z());
			glVertex3f(p2.X(), p2.Y(), p2.Z());
			glLineWidth(1.0f);
			glEnd();

			p1.X() -= 0.1;
			p2.X() -= 0.1;
		}
		glPopMatrix();
	}
	else
	{
		float P = 0.5f / this->fRatio;
		Point3m pYellow = Point3m(0,100 * P, 0);
		glPushMatrix();
		float unitAdd = 0.00001;
		for (int i = 0; i < 100; i++)
		{
			glBegin(GL_LINES);
			glLineWidth(1.0f);
			glColor3f(1, 0, 0);
			glVertex3f(p1.X(), p1.Y(), p1.Z());
			glColor3f(1, 1, 0);
			glVertex3f(pYellow.X(), pYellow.Y(), pYellow.Z());
			glLineWidth(1.0f);
			glEnd();

			glBegin(GL_LINES);
			glLineWidth(1.0f);
			glColor3f(1, 1, 0);
			glVertex3f(pYellow.X(), pYellow.Y(), pYellow.Z());
			glColor3f(topColor.X(), topColor.Y(), topColor.Z());
			glVertex3f(p2.X(), p2.Y(), p2.Z());
			glLineWidth(1.0f);
			glEnd();

			p1.X() -= 0.1;
			pYellow.X() -= 0.1;
			p2.X() -= 0.1;
		}
		glPopMatrix();
	}

	QFont labelFont("Verdana");
	labelFont.setBold(true);
	labelFont.setPixelSize(25);
	labelFont.setStyleStrategy(QFont::NoAntialias);

	glPushMatrix();
	glColor3f(1, 1, 1);
	renderText(165, 733, QString::number(scals[0]), labelFont);
	renderText(165, 567, QString::number(scals[1]), labelFont);
	renderText(165, 406, QString::number(scals[2]), labelFont);
	renderText(165, 244, QString::number(scals[3]), labelFont);
	renderText(165, 83,  QString::number(scals[4]), labelFont);

	renderText(10, 10, "Max: " + QString::number(fMaxium));
	renderText(10, 30, "Rat: " + QString::number(fRatio));
	renderText(10, 50, "TopNum " + QString::number(fTopNum));
	renderText(10, 70, "X: " + QString::number(cursor.x()));
	renderText(10, 90, "Y: " + QString::number(cursor.y()));
	glPopMatrix();
}

void ColorTable::resizeGL(float w, float h)
{
	if (h == 0)
		h = 1;
	glViewport(0, 0, 1920, 1080);
}

void ColorTable::mousePressEvent(QMouseEvent *event)
{
	event->accept();
	setFocus();
	cursor = event->pos();
	getColorFromColorTableSlot(0, colorPicked);

	updateGL();
}
void ColorTable::mouseMoveEvent(QMouseEvent *event)
{
	event->accept();
	cursor = event->pos();

	updateGL();
}

void ColorTable::mouseReleaseEvent(QMouseEvent *event)
{

}

void ColorTable::wheelEvent(QWheelEvent * event)
{
	event->accept();
	setFocus();
	cursor = event->pos();

	if (cursor.x() > 156)
	{
		//ֵ
		if (event->delta() > 0)
		{
			this->fMaxium += 0.1f;
		}
		else
		{
			this->fMaxium -= 0.1f;
			if (fMaxium < fTopNum)
			{
				fMaxium = fTopNum;
			}
		}
	}
	else
	{
		//ǰ
		if (event->delta() > 0)
		{
			this->fTopNum += 0.025;
			if (fTopNum >= fMaxium)
			{
				fTopNum = fMaxium;
			}
		}
		else
		{
			this->fTopNum -= 0.025;
			if (fTopNum <= 0)
			{
				fTopNum = 0;
			}
		}
	}

	this->topColor = updateTopColor();
	updateNumberSign();
	emit setNewTopDistance(fTopNum, fMaxium, topColor);

	updateGL();
}

void ColorTable::keyReleaseEvent(QKeyEvent * e)
{

}

void  ColorTable::keyPressEvent(QKeyEvent * e)
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
	if (e->key() == Qt::Key_E)
		camera->z += addValue;
	if (e->key() == Qt::Key_Q)
		camera->z -= addValue;

	if (e->key() == Qt::Key_Left)
		cursorPoint.X() -= addValue;
	if (e->key() == Qt::Key_Right)
		cursorPoint.X() += addValue;
	if (e->key() == Qt::Key_Up)
		cursorPoint.Y() += addValue;
	if (e->key() == Qt::Key_Down)
		cursorPoint.Y() -= addValue;

	updateGL();
}

void ColorTable::getColorTableParams(float fenshu, float max)
{
	this->fRatio = fenshu;
	this->fMaxium = max;
}

Point3m ColorTable::computeColorByDistance(float fDistance)
{
	float red, green, blue;

	float fLevel;
	fLevel = float(fDistance / this->fMaxium);
	if (fLevel > 1)
		fLevel = 1;
	if (fLevel < 0)
		fLevel = 0;

	if (fLevel < 0.5f)
		red = 255.0f;
	else
		red = -510.0f * fLevel + 510.0f;

	if (fLevel < 0.5f)
		green = 2 * 255.0f * fLevel;
	else
		green = -510.0f * fLevel + 510.0f;

	blue = 255.0f * fLevel;

	return Point3m(red, green, blue);
}

Point3m ColorTable::updateTopColor()
{
	this->fRatio = fTopNum / fMaxium;
	float red, green, blue;
	float P;
	if (fRatio <= 0.5f)
	{
		P = fRatio / 0.5f;
		red = 1;
		green = P;
		blue = 0;
	}
	else
	{
		P = (fRatio - 0.5f) / 0.5f;
		red = 1 - P;
		green = 1 - P;
		blue = P;
	}

	return Point3m(red, green, blue);
}

void ColorTable::updateNumberSign()
{
	this->fTopNum = fMaxium * fRatio;
	float step = fTopNum / 4.0f;
	this->scals.clear();

	for (int i = 0; i < 5; i++)
	{
		scals.push_back(i * step);
	}
}

void ColorTable::getColorFromColorTableSlot(float radio, Point3m& color)
{
	float *record = new float[3];
	Point3m node(0, 100 * radio, 0);
	//glReadPixels(node.X(), node.Y(), 1, 1, GL_RGB, GL_BYTE, record);
	glReadPixels(cursor.x(), cursor.y(), 1, 1, GL_RGB, GL_FLOAT, record);

	color.X() = record[2];
	color.Y() = record[1];
	color.Z() = record[0];
}
