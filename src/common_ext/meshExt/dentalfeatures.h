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

#ifndef  DENTALFEATURES_H
#define DENTALFEATURES_H

#include <QObject>
#include "../common/ml_mesh_type.h"
#include "util/assist_geometry.h"
#include "util/custom_plane.h"
#include "meshExt/AbsMesh.h"
#include "common_ext_global.h"
#include "common_ext/data/CutFace.h"
#include "data/separationmanager.h"

class COMMON_EXT_EXPORT  ToothFeatures
{
public:
    ToothFeatures(int _fdi, vector<Point3m> &_marks, vector<Point3m> &_boundary);
    ToothFeatures(int _fdi);
    ToothFeatures(int _fdi, std::map<QString, sFeaturePoint>&marks);
    void initialWithThreePoint(Point3m _near_midpoint, Point3m _far_midpoint, Point3m _midpoint);
    void initalWithPointList(vector<pair<int, Point3m>> &_point_list);
    void supplyEstimateMarks();
    void setWidthEdge(Point3m &_a, Point3m &_b);
    void setAccompanyEdgePointer(FEdge* _edge);
    void updateWidthEdge();
    bool boundaryIntersectionWithXYPlane(Point3m _pos, Point3m _normal, Point3m _va, Point3m _vb, Point3m& _v1, Point3m& _v2);
    Point3m findNearestPoint(Point3m _p, vector<Point3m>& _vert_list);
    void computeAxis();
    void analysisIncisorAndCannies();
    void analysisPremolars();
    void analysisMolars();
    void initialFdiShowParameters(Point3m &_dental_center, Point3m &_dental_plane_normal);
public:
    int fdi_;
    vector<Point3m> marks_;
    vector<Point3m> boundary_;
    FEdge width_edge_;
    FEdge* p_accompany_edge_ = nullptr;
    float f_tooth_width_ = 0;
    Axis axis_;
    std::map<QString, sFeaturePoint> feature_marks_;
    bool all_mark_ready_ = false;
    bool axis_ready_ = false;
    Point3m fdi_show_normal_, fdi_show_pos_;
};

class COMMON_EXT_EXPORT  DentalFeatures :
    public QObject
{
    Q_OBJECT
public:
    DentalFeatures();
    DentalFeatures(bool _b_upper, SeparationManager* _mesh, vector<int>& _fdi_list, vector<vector<Point3m>>& _mark_list, vector<vector<Point3m>>& _boundary_list);
    DentalFeatures(bool _b_upper, SeparationManager* _mesh, vector<int>& _fdi_list, vector<vector<pair<int, Point3m>>> &_mark_list);
    DentalFeatures(bool _b_upper, SeparationManager* _mesh, vector<int>& _fdi_list);
    DentalFeatures(bool isUpper, std::map<QString, CrownInfoSegmentedIntelligent>& marks, SeparationManager *pMesh = nullptr);

    void constructOcclusalPlaneAxis();
    void constructSpeePlane();
    void setVisible(bool _visible);
    void setDentalArch(float _length, vector<Point3m> &_arch);
    bool eachToothMarksReady();
    bool getPreferViewDirect(Point3m &_left_direct, Point3m& _right_direct);

public:
    bool b_upper_;
    bool bVisible_ = true;
    bool base_plane_ready_ = false;
    CustomPlane base_plane_;
    CutFace spee_plane_;
    SeparationManager* mesh_ = nullptr;
    vector<ToothFeatures> teeth_list_;
    vector<Point3m> dentalArch_;
    float fDentalArchLength_ = 0;
};

#endif // ! DENTALFEATURES_H
