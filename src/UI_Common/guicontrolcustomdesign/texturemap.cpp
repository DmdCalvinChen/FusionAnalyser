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

#include "texturemap.h"

#include <wrap/qt/col_qt_convert.h>
// #include <qgl.h>
#include <QGLWidget>

namespace ui_common
{
TextureMap::TextureMap(const QString& _imageFilePath)
{
    targetTex = 0;
    opacity = 0.5;

    if (imageBackground.load(_imageFilePath))
    {
        bLoadImageSucceed = true;

        setTarget(imageBackground);
        setUVCoordinateMap(vcg::Point3f(-1.0f, 1.0f, 0.0f), vcg::Point3f(-1.0f, -1.0f, 0.0f),
                           vcg::Point3f(1.0, 1.0, 0.0f), vcg::Point3f(1.0f, -1.0f, 0.0f));
    }
    else
    {
        bLoadImageSucceed = false;
    }
}

TextureMap::~TextureMap()
{
    if (bLoadImageSucceed)
    {
        if (targetTex)
            glDeleteTextures(1, &targetTex);
    }
}

void TextureMap::setTarget(QImage& image)
{
    if (image.isNull())
        return;
    if (targetTex)
    {
        glDeleteTextures(1, &targetTex);
        targetTex = 0;
    }
    // create texture
    glGenTextures(1, &targetTex);
    QImage tximg = QGLWidget::convertToGLFormat(image);
    glBindTexture(GL_TEXTURE_2D, targetTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, tximg.width(), tximg.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 tximg.bits());

    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureMap::setScreenRatio(float _screenRatio)
{
    this->screenRatio = _screenRatio;
}

bool TextureMap::getLoadImageIsWhetherSucceed() const
{
    return bLoadImageSucceed;
}

void TextureMap::setUVCoordinateMap(const vcg::Point3f& _leftTopVertex2DUVCoordinate,
                                    const vcg::Point3f& _leftBottomVertex2DUVCoordinate,
                                    const vcg::Point3f& _rightTopVertex2DUVCoordinate,
                                    const vcg::Point3f& _rightBottomVertex2DUVCoordinate)
{
    this->leftTopVertex2DUVCoordinate = _leftTopVertex2DUVCoordinate;
    this->leftBottomVertex2DUVCoordinate = _leftBottomVertex2DUVCoordinate;
    this->rightTopVertex2DUVCoordinate = _rightTopVertex2DUVCoordinate;
    this->rightBottomVertex2DUVCoordinate = _rightBottomVertex2DUVCoordinate;
}

void TextureMap::draw(float _screenRatio)
{
    if (!targetTex || !bLoadImageSucceed)
        return;

    glColor4f(1, 1, 1, opacity);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, targetTex);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);    // first point
    glVertex3f(leftBottomVertex2DUVCoordinate.X(), leftBottomVertex2DUVCoordinate.Y(),
               leftBottomVertex2DUVCoordinate.Z());
    glTexCoord2f(1.0f, 0.0f);    // second point
    glVertex3f(rightBottomVertex2DUVCoordinate.X(), rightBottomVertex2DUVCoordinate.Y(),
               rightBottomVertex2DUVCoordinate.Z());
    glTexCoord2f(1.0f, 1.0f);    // third point
    glVertex3f(rightTopVertex2DUVCoordinate.X(), rightTopVertex2DUVCoordinate.Y(),
               rightTopVertex2DUVCoordinate.Z());
    glTexCoord2f(0.0f, 1.0f);    // fourth point
    glVertex3f(leftTopVertex2DUVCoordinate.X(), leftTopVertex2DUVCoordinate.Y(),
               leftTopVertex2DUVCoordinate.Z());
    glEnd();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void TextureMap::drawBackground(float _screenRatio)
{
    if (!targetTex || !bLoadImageSucceed)
        return;

    QImage& curImg = imageBackground;
    float imageRatio = float(curImg.width()) / float(curImg.height());

    // set orthogonal view
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-1.0f, 1.0f, -1, 1);
    glColor4f(1, 1, 1, opacity);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, targetTex);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);    // first point
    glVertex3f(leftBottomVertex2DUVCoordinate.X(), leftBottomVertex2DUVCoordinate.Y(),
               leftBottomVertex2DUVCoordinate.Z());
    glTexCoord2f(1.0f, 0.0f);    // second point
    glVertex3f(rightBottomVertex2DUVCoordinate.X(), rightBottomVertex2DUVCoordinate.Y(),
               rightBottomVertex2DUVCoordinate.Z());
    glTexCoord2f(1.0f, 1.0f);    // third point
    glVertex3f(rightTopVertex2DUVCoordinate.X(), rightTopVertex2DUVCoordinate.Y(),
               rightTopVertex2DUVCoordinate.Z());
    glTexCoord2f(0.0f, 1.0f);    // fourth point
    glVertex3f(leftTopVertex2DUVCoordinate.X(), leftTopVertex2DUVCoordinate.Y(),
               leftTopVertex2DUVCoordinate.Z());
    glEnd();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    // restore view
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
}    // namespace ui_common
