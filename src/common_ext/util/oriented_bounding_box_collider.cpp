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

#include "oriented_bounding_box_collider.h"

OBBox::OBBox(Point3m _center, Point3m _axisX, Point3m _axisY, Point3m _axisZ, float _lenX, float _lenY, float _lenZ)
{
	center_ = _center;
	axis_[0] = _axisX;
	axis_[1] = _axisY;
	axis_[2] = _axisZ;
	extents_[0] = _lenX;
	extents_[1] = _lenY;
	extents_[2] = _lenZ;
	matrix_.SetIdentity();
}

vector<Point3m> OBBox::getCurrentConners()
{
    vector<Point3m> conners;
    conners.push_back(matrix_ * (center_ + axis_[0] * extents_[0] + axis_[1] * extents_[1] + axis_[2] * extents_[2]));
    conners.push_back(matrix_ * (center_ + axis_[0] * extents_[0] + axis_[1] * extents_[1] - axis_[2] * extents_[2]));
    conners.push_back(matrix_ * (center_ + axis_[0] * extents_[0] - axis_[1] * extents_[1] + axis_[2] * extents_[2]));
    conners.push_back(matrix_ * (center_ + axis_[0] * extents_[0] - axis_[1] * extents_[1] - axis_[2] * extents_[2]));
    conners.push_back(matrix_ * (center_ - axis_[0] * extents_[0] + axis_[1] * extents_[1] + axis_[2] * extents_[2]));
    conners.push_back(matrix_ * (center_ - axis_[0] * extents_[0] + axis_[1] * extents_[1] - axis_[2] * extents_[2]));
    conners.push_back(matrix_ * (center_ - axis_[0] * extents_[0] - axis_[1] * extents_[1] + axis_[2] * extents_[2]));
    conners.push_back(matrix_ * (center_ - axis_[0] * extents_[0] - axis_[1] * extents_[1] - axis_[2] * extents_[2]));
    return conners;
}

vector<Point3m> OBBox::getCurrentAxises()
{
    vector<Point3m> axises;
    Point3m center = center_;
    Point3m axisnode0 = center + axis_[0];
    Point3m axisnode1 = center + axis_[1];
    Point3m axisnode2 = center + axis_[2];

    center = matrix_ * center;
    axisnode0 = matrix_ * axisnode0;
    axisnode1 = matrix_ * axisnode1;
    axisnode2 = matrix_ * axisnode2;
    axises.push_back((axisnode0 - center).Normalize());
    axises.push_back((axisnode1 - center).Normalize());
    axises.push_back((axisnode2 - center).Normalize());
    return axises;
}

void OBBox::getInterval(const vector<Point3m>& _conners, const Point3m& _axis, float& _min, float& _max)
{
    float temp_proj;
    _min = _max = _conners[0] * _axis;
    for (int i = 1; i < _conners.size(); ++i)
    {
        temp_proj = _conners[i] * _axis;
        if (_min > temp_proj)
        {
            _min = temp_proj;
        }
        if (_max < temp_proj)
        {
            _max = temp_proj;
        }
    }
}

bool OBBox::intersectWithBox(OBBox _box)
{
    vector<Point3m> conners_a = getCurrentConners();
    vector<Point3m> conners_b = _box.getCurrentConners();
    vector<Point3m> axises_a = getCurrentAxises();
    vector<Point3m> axises_b = _box.getCurrentAxises();

    Point3m axis;
    float min_a, max_a, min_b, max_b;
    for (int i = 0; i < 3; ++i)
    {
        axis = axises_a[i];
        getInterval(conners_a, axis, min_a, max_a);
        getInterval(conners_b, axis, min_b, max_b);
        if (max_a < min_b || max_b < min_a)
        {
            return false;
        }
    }
    for (int i = 0; i < 3; ++i)
    {
        axis = axises_b[i];
        getInterval(conners_a, axis, min_a, max_a);
        getInterval(conners_b, axis, min_b, max_b);
        if (max_a < min_b || max_b < min_a)
        {
            return false;
        }
    }
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            axis = (axises_a[i] ^ axises_b[j]).Normalize();
            getInterval(conners_a, axis, min_a, max_a);
            getInterval(conners_b, axis, min_b, max_b);
            if (max_a < min_b || max_b < min_a)
            {
                return false;
            }
        }
    }
    return true;
}

void OBBox::draw(Point4m _color)
{
    vector<Point3m> verts = getCurrentConners();
    vector<FFace> faces;
    faces.push_back(FFace(verts[0], verts[1], verts[2]));
    faces.push_back(FFace(verts[1], verts[3], verts[2]));
    faces.push_back(FFace(verts[4], verts[6], verts[5]));
    faces.push_back(FFace(verts[5], verts[6], verts[7]));
    faces.push_back(FFace(verts[0], verts[2], verts[4]));
    faces.push_back(FFace(verts[4], verts[2], verts[6]));
    faces.push_back(FFace(verts[1], verts[5], verts[3]));
    faces.push_back(FFace(verts[5], verts[7], verts[3]));
    faces.push_back(FFace(verts[1], verts[5], verts[4]));
    faces.push_back(FFace(verts[1], verts[4], verts[0]));
    faces.push_back(FFace(verts[3], verts[6], verts[7]));
    faces.push_back(FFace(verts[6], verts[3], verts[2]));

    for (auto& face : faces)
    {
        glPushMatrix();
        glDisable(GL_LIGHTING);
        glColor4f(_color.X(), _color.Y(), _color.Z(), _color.W());
        glBegin(GL_TRIANGLES);
        glVertex3f(face.v1.X(), face.v1.Y(), face.v1.Z());
        glVertex3f(face.v3.X(), face.v3.Y(), face.v3.Z());
        glVertex3f(face.v2.X(), face.v2.Y(), face.v2.Z());
        glEnd();
        glColor3f(1, 1, 1);
        glPopMatrix();
    }
    vector<FFace>().swap(faces);
}

void OBBox::exportBoxStl(QString _filename)
{
    vector<Point3m> verts = getCurrentConners();
    vector<FFace> faces;
    faces.push_back(FFace(verts[0], verts[1], verts[2]));
    faces.push_back(FFace(verts[1], verts[3], verts[2]));
    faces.push_back(FFace(verts[4], verts[6], verts[5]));
    faces.push_back(FFace(verts[5], verts[6], verts[7]));
    faces.push_back(FFace(verts[0], verts[2], verts[4]));
    faces.push_back(FFace(verts[4], verts[2], verts[6]));
    faces.push_back(FFace(verts[1], verts[5], verts[3]));
    faces.push_back(FFace(verts[5], verts[7], verts[3]));
    faces.push_back(FFace(verts[1], verts[5], verts[4]));
    faces.push_back(FFace(verts[1], verts[4], verts[0]));
    faces.push_back(FFace(verts[3], verts[6], verts[7]));
    faces.push_back(FFace(verts[6], verts[3], verts[2]));

    CMeshO mesh;
    for (int i = 0; i < faces.size(); ++i)
    {
        vcg::tri::Allocator<CMeshO>::AddFace(mesh, faces[i].v1, faces[i].v2, faces[i].v3);
    }
    vcg::tri::Clean<CMeshO>::RemoveDuplicateVertex(mesh);
    tri::Allocator<CMeshO>::CompactEveryVector(mesh);
    MeshGenerator mesh_maker;
    mesh_maker.updateModel(&mesh);
    vcg::tri::io::ExporterSTL<CMeshO>::Save(mesh, _filename.toStdString().c_str());
}
