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

#ifndef LONGAXISLOCATIONTOOL_H
#define LONGAXISLOCATIONTOOL_H
#define CTRL_NODE_WIDTH 0.5f

#include <qobject.h>
#include "util/assist_geometry.h"
#include "util/utility_tools.h"
#include "vcg/space/index/kdtree/kdtree.h"
#include "common_ext_global.h"

// Tool for locating and adjusting tooth long axis
class COMMON_EXT_EXPORT LongAxisLocationTool :
    public QObject
{
    enum {NONE_PICKED = -1, TOP_PICKED, BOTTOM_PICKED};
public:
    LongAxisLocationTool(){}
    LongAxisLocationTool(Axis* _aim_axis, CMeshO* _cur_mesh, vcg::Matrix44f _matrix, int _fdi);
    LongAxisLocationTool(Axis* _aim_axis, CMeshO* _cur_mesh, vcg::Matrix44f _matrix, int _fdi, int _range);
    LongAxisLocationTool(Axis* _aim_axis, CMeshO* _cur_mesh, vcg::Matrix44f _matrix, int _fdi, QString &_context);
    void initalTool();
    void getOriginNode();
    Point3m centerOfBoundary();
    void updateProfile();
    bool computeCrossSegmentByFace(CFaceO* face, Point3m cutFaceNormalV, Point3m cutFacePosP, vcg::Matrix44f _matrix, FColorEdge &_edge);
    void updateProfileColor(vector<FColorEdge> &_edges);
    void initalKdTree();
    Point3m adjustMeshOrigin();
    void updateEnableFaceIndexes();
    void initalCtrlNode(vector<FColorEdge>& _edges);
    void adjustAction();
    void setFaceRange(int _range);
    void setModelMatrix(vcg::Matrix44f _matrix);
    void forceInital();
    void initalDirectly(Axis* _aim_axis, CMeshO* _cur_mesh, vcg::Matrix44f _matrix, int _fdi);

    void draw();
    void drawCtrlNodes();
    void drawProfile();
    void drawContext();

    bool mousePressAction(int _mousex, int _mousey);
    bool mouseMoveAction(int _mousex, int _mousey);
    bool mouseReleaseAction(int _mousex, int _mousey);
public:
    Axis* aim_axis_ = nullptr;
    CMeshO* cur_mesh_ = nullptr;
    vector<FColorEdge> profile_;
    Point3m top_node_, bottom_node_, origin_node_;
    vector<int> enable_faces_;
    QString display_context_;
    vcg::Matrix44f  model_matrix_;
    int picked_state_ = NONE_PICKED;
    int face_range_;
    Point3m focus_center_;
    KdTree<float>* kt_ = nullptr;
    int fdi_;
    float znode_t_ = 0.5f;
};

#endif //LONGAXISLOCATIONTOOL_H
