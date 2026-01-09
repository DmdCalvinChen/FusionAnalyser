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

#ifndef TOWABLECTRLSYS_H
#define TOWABLECTRLSYS_H

#include <qobject.h>
#include <GL/glew.h>
#include "../common/ml_mesh_type.h"
#include "util/assist_geometry.h"
#include "util/utility_tools.h"
#include "common_ext_global.h"
using namespace std;

class TowableCtrlNode;
class TowableCtrlEdge
{
public:
    TowableCtrlEdge(){}
    TowableCtrlEdge(TowableCtrlNode* _a, TowableCtrlNode* _b, float _screen_height, float _screen_width)
    {
        a_ = _a;
        b_ = _b;
        screen_height_ = _screen_height;
        screen_width_ = _screen_width;
    }

    void draw();
    bool mousePress(int _mouse_x, int _mouse_y);
    bool mouseRelease(int _mouse_x, int _mouse_y);
    bool mouseMove(int _mouse_x, int _mouse_y);

public:
    float screen_height_, screen_width_;
    bool picked_up_ = false;
    TowableCtrlNode* a_ = nullptr, *b_ = nullptr;
    bool visible_ = true;
    Point3m picked_pos_;
};

class TowableCtrlNode
{
public:
    TowableCtrlNode() {}
    TowableCtrlNode(Point3m _p, float _screen_height, float _screen_width)
    {
        p_ = _p;
        screen_height_ = _screen_height;
        screen_width_ = _screen_width;
    }

    void setArrowMode(bool _state);

    inline void setAdjointPoint(TowableCtrlNode* _node) { adjoint_node_ = _node; }
    inline void setSpaceConstraints(pair<bool, pair<Point3m, Point3m>> _data) { spce_constraints_ = _data; }

    virtual void draw();
    bool mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::NoButton);
    bool mouseRelease(int _mouse_x, int _mouse_y);
    bool spaceConstraintsJudge(Point3m _p);

    virtual bool mouseMove(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::LeftButton) { return false; }

public:
    pair<bool, pair<Point3m, Point3m>> spce_constraints_ = make_pair(false, make_pair(Point3m(0, 0, 0), Point3m(0, 0, 0)));
    float screen_height_, screen_width_;
    bool picked_up_ = false;
    Point3m p_;
    bool visible_ = true;
    TowableCtrlNode* adjoint_node_ = nullptr;
    bool arrow_mode_ = false;
    bool draw_guidelines_ = false;
};

class TowableCtrlNodeOnMesh : public TowableCtrlNode
{
public:
    TowableCtrlNodeOnMesh() {}
    TowableCtrlNodeOnMesh(Point3m _p, vector<CMeshO*>_mesh_list, float _screen_height, float _screen_width, bool _cloud_pick_unvisible_faces = false) : TowableCtrlNode(_p, _screen_height, _screen_width)
    {
        mesh_list_ = _mesh_list;
        cloud_pick_unvisible_faces_ = _cloud_pick_unvisible_faces;
    }
  bool mouseMove(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::LeftButton);

private:
    bool cloud_pick_unvisible_faces_ = false;
    vector<CMeshO*> mesh_list_;
};

class TowableCtrlNodeOnPlane : public TowableCtrlNode
{
public:
    TowableCtrlNodeOnPlane(){}
    TowableCtrlNodeOnPlane(Point3m _p, Point3m _plane_pos, Point3m _plane_normal, bool _plane_normal_is_view, float _screen_height, float _screen_width) : TowableCtrlNode(_p, _screen_height, _screen_width)
    {
        plane_pos_ = _plane_pos;
        plane_normal_ = _plane_normal;
        plane_normal_is_view_direct_ = _plane_normal_is_view;
    }
   bool mouseMove(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::LeftButton);
   void draw();

private:
    Point3m plane_pos_, plane_normal_;
    bool plane_normal_is_view_direct_ = false;
};

class COMMON_EXT_EXPORT TowableCtrlSystem :
    public QObject
{
    Q_OBJECT
public:
    TowableCtrlSystem(){}
    TowableCtrlSystem(const TowableCtrlSystem &_sys);
    TowableCtrlSystem(int _ino, vector<pair<Point3m, vector<CMeshO*>>> _node_info_list, bool _fitting_curve = false, bool _cloud_pick_unvisible_face = false);
    TowableCtrlSystem(int _ino, Point3m _node_a, CMeshO* _mesh_a, Point3m _node_b, CMeshO* _mesh_b, bool _fitting_curve = false, bool _arrow_mode = false);
    TowableCtrlSystem(int _ino, vector<Point3m> _points, CMeshO* _mesh, bool _normal_is_view_direct = false, bool _fitting_curve = false, bool _arrow_mode = false);
    TowableCtrlSystem(int _ino, Point3m _node_a, vector<CMeshO*> _mesh_a_list, Point3m _node_b, vector<CMeshO*> _mesh_b_list, bool _fitting_curve = false);
    TowableCtrlSystem(int _ino, vector<Point3m> _points, Point3m _plane_pos, Point3m _plane_normal, bool _normal_is_view_direct = false, bool _fitting_curve = false, bool _arrow_mode = false);

    TowableCtrlSystem& operator=(const TowableCtrlSystem& _sys)
    {
        iNo_ = _sys.iNo_;
        length_ = _sys.length_;
        fitting_curve_ = _sys.fitting_curve_;
        edges_ = _sys.edges_;
        mesh_list_ = _sys.mesh_list_;
        picked_ctrl_node_ = _sys.picked_ctrl_node_;
        ctrl_nodes_ = _sys.ctrl_nodes_;
        return *this;
    }

    void update();
    void draw();
    void drawAuxiliary();
    void clear();
    inline void setVisible(bool _visible) { visible_ = _visible; }
    inline void setAuxiliary(bool _auxiliary) { auxiliary_ = _auxiliary; update(); }
    void setVisualAngleDirect(Point3m _direct);
    void setScreenWidthAndHeight(int _width, int _height);
    void updateBoundBall();
    void initialTowableCtrlEdges();
    void udpateCtrlNodePos(const vector<Point3m> &_pos_list);
    void setSpaceConstraints(pair<bool, pair<Point3m, Point3m>> _data);

    bool mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::NoButton);
    bool mouseMove(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::LeftButton);
    bool mouseRelease(int _mouse_x, int _mouse_y);
    bool pickedHandle();
    bool judgeVisibleByVisualAngle(Point3m &_visible_direct);
public:
    bool visible_ = true, auxiliary_ = false;
    int iNo_ = 0;
    float length_ = 0.0f;
    bool fitting_curve_ = false;
    vector<FEdge> edges_;
    vector<CMeshO*> mesh_list_;
    std::pair<bool, Point3m> visual_angle_ctrl_;
    float screen_height_, screen_width_;

    Point3m center_;
    float radius_;

    TowableCtrlNode* picked_ctrl_node_ = nullptr;
    vector<TowableCtrlNode*> ctrl_nodes_;

    TowableCtrlEdge* picked_ctrl_edge_ = nullptr;
    vector<TowableCtrlEdge*> ctrl_edges_;
};

#endif
