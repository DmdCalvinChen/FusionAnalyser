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

#include "melkman_convexhull2D.h"

void MelkmanConvexhull2D::swap(int i, int j)
{
    MPoint tempPoint;
    tempPoint = points_[j];
    points_[j] = points_[i];
    points_[i] = tempPoint;
}

// Quick sort partition function
int MelkmanConvexhull2D::loc(int top, int bot)
{
    float x = points_[top].angle;
    int j, k;
    j = top + 1;
    k = bot;
    while (true)
    {
        while (j < bot && points_[j].angle < x)
            j++;
        while (k > top && points_[k].angle > x)
            k--;
        if (j >= k)
            break;
        swap(j, k);
    }
    swap(top, k);
    return k;
}

// Quick sort implementation
void MelkmanConvexhull2D::quickSort(int top, int bot)
{
    int pos;
    if (top < bot)
    {
        pos = loc(top, bot);
        quickSort(top, pos - 1);
        quickSort(pos + 1, bot);
    }
}

float MelkmanConvexhull2D::isLeft(MPoint o, MPoint a, MPoint b)
{
    float aoX = a.x - o.x;
    float aoY = a.y - o.y;
    float baX = b.x - a.x;
    float baY = b.y - a.y;
    return aoX * baY - aoY * baX;
}

vector<Point2m> MelkmanConvexhull2D::getResults(vector<Point2m> &_pointlist)
{
    int PointsNum = _pointlist.size();

    vector<int> index((PointsNum + 10) * 2, 0); // Double-ended queue

    vector<MPoint>().swap(points_);
    for (auto& p : _pointlist)
    {
        points_.push_back(MPoint(p));
    }

    int k = 0;
    MPoint Ymin = points_[k];
    for (int i = 1; i < PointsNum; i++)
    {
        if (points_[i].y < Ymin.y)
        {
            Ymin.x = points_[i].x;
            Ymin.y = points_[i].y;
            k = i;
        }

        if (points_[i].y == Ymin.y)
        {
            if (points_[i].x < Ymin.x)
            {
                Ymin.x = points_[i].x;
                Ymin.y = points_[i].y;
                k = i;
            }
        }
    }
    swap(0, k);

    // Calculate angle from point with minimum y to x-axis
    for (int i = 1; i < PointsNum; i++)
    {
        if (points_[i].x == points_[0].x)
        {
            float y_dvalue = points_[i].y - points_[0].y;
            if (y_dvalue == 0)
            {
                points_[i].angle = 0;
            }
            else if(y_dvalue < 0)
            {
                points_[i].angle = -PI / 2.0f;
            }
            else
            {
                points_[i].angle = PI / 2.0f;
            }
        }
        else
        {
            float t = (points_[i].y - points_[0].y) / abs(points_[i].x - points_[0].x);
            points_[i].angle = atan(t);
        }
    }
    // Sort by angle
    quickSort(1, PointsNum - 1);

    int bot = PointsNum - 1;
    int top = PointsNum;
    index[top++] = 0;//index[PointsNum]=0;
    index[top++] = 1;//index[PointsNum+1]=1;top=PointsNum+2;
    int i;
    for (i = 2; i < PointsNum; i++)
    {
        // Find first 3 non-collinear points
        if (isLeft(points_[index[top - 2]], points_[index[top - 1]], points_[i]) != 0)
        {
            break;
        }
        // Collinear points, update last index
        index[top - 1] = i;
    }
    index[bot--] = i;
    index[top++] = i;

    // Ensure counter-clockwise orientation
    int t;
    if (isLeft(points_[index[PointsNum]], points_[index[PointsNum + 1]], points_[index[PointsNum + 2]]) < 0)
    {
        // Swap if clockwise, ensure counter-clockwise
        t = index[PointsNum];
        index[PointsNum] = index[PointsNum + 1];
        index[PointsNum + 1] = t;
    }

    for (i++; i < PointsNum; i++)
    {
        // Skip if point is inside current hull
        if (isLeft(points_[index[top - 2]], points_[index[top - 1]], points_[i]) > 0 && isLeft(points_[index[bot + 1]], points_[index[bot + 2]], points_[i]) > 0)
            continue;
        // Pop from top stack
        while (isLeft(points_[index[top - 2]], points_[index[top - 1]], points_[i]) <= 0)
            top--;
        index[top++] = i;

        // Pop from bottom stack
        while (isLeft(points_[index[bot + 1]], points_[index[bot + 2]], points_[i]) <= 0)
            bot++;
        index[bot--] = i;
    }
    // Convex hull points are between bot+1 and top-1
    int index1 = 0;
    vector<Point2m> results;
    for (i = bot + 1; i < top - 1; i++)
    {
        results.push_back(Point2m(points_[index[i]].x, points_[index[i]].y));
    }
    return results;
}
