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

#ifndef  MELKMANCONVEXHULL2D_H
#define MELKMANCONVEXHULL2D_H
#include "common/ml_mesh_type.h"
#include "util/assist_geometry.h"
#include "common_ext_global.h"
using namespace std;

struct MPoint
{
    float x;
    float y;
    float angle;
    MPoint()
    {
        x = 0;
        y = 0;
        angle = 0;
    }
    MPoint(Point2m _p)
    {
        x = _p.X();
        y = _p.Y();
        angle = 0;
    }
};

class COMMON_EXT_EXPORT MelkmanConvexhull2D
{
public:
    MelkmanConvexhull2D() {}
    void swap(int i, int j);
    int loc(int top, int bot);
    void quickSort(int top, int bot);
    float isLeft(MPoint o, MPoint a, MPoint b);
    vector<Point2m> getResults(vector<Point2m>& _pointlist);

    vector<MPoint> points_;
};

#endif // ! MELKMANCONVEXHULL2D_H

