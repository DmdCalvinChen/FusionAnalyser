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

#ifndef TEXTUREMAP_H
#define TEXTUREMAP_H
#include <QImage>
#include <QString>

#include <GL/glew.h>
#include <vcg/space/point3.h>

#include "../uicommon_global.h"

/**
 *     texture load for 2D control designed using.  Designed by liuchuang on 29/7/2019
 */

namespace ui_common
{
class UI_COMMONSHARED_EXPORT TextureMap
{
public:
    TextureMap(const QString& _imageFilePath);
    ~TextureMap();
    void setTarget(QImage& image);
    void setUVCoordinateMap(const vcg::Point3f& _leftTopVertex2DUVCoordinate,
                            const vcg::Point3f& _leftBottomVertex2DUVCoordinate,
                            const vcg::Point3f& _rightTopVertex2DUVCoordinate,
                            const vcg::Point3f& _rightBottomVertex2DUVCoordinate);
    void draw(float _screenRatio = 1.0f);
    void drawBackground(float _screenRatio);
    void setScreenRatio(float _screenRatio);

    bool getLoadImageIsWhetherSucceed() const;

public:
private:
    bool bLoadImageSucceed = false;
    float screenRatio;
    float opacity;
    QImage imageBackground;
    GLuint targetTex;

    vcg::Point3f leftTopVertex2DUVCoordinate;
    vcg::Point3f leftBottomVertex2DUVCoordinate;
    vcg::Point3f rightTopVertex2DUVCoordinate;
    vcg::Point3f rightBottomVertex2DUVCoordinate;
};
}    // namespace ui_common

#endif
