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

#include "triangleintersectmachine.h"

TriangleIntersectMachine::TriangleIntersectMachine()
{
}

TriangleIntersectMachine::~TriangleIntersectMachine()
{
}

//ά㿽Ϊά
void TriangleIntersectMachine::copy_point(point& p, Point3m f)
{
	p.x = f[0];
	p.y = f[1];
}

//ĵʽ
inline float TriangleIntersectMachine::get_vector4_det(Point3m v1, Point3m v2, Point3m v3, Point3m v4)
{
	float a[3][3];
	for (int i = 0; i != 3; ++i)
	{
		a[0][i] = v1[i] - v4[i];
		a[1][i] = v2[i] - v4[i];
		a[2][i] = v3[i] - v4[i];
	}

	return a[0][0] * a[1][1] * a[2][2]
		+ a[0][1] * a[1][2] * a[2][0]
		+ a[0][2] * a[1][0] * a[2][1]
		- a[0][2] * a[1][1] * a[2][0]
		- a[0][1] * a[1][0] * a[2][2]
		- a[0][0] * a[1][2] * a[2][1];
}

inline double TriangleIntersectMachine::direction(point p1, point p2, point p)
{
	return (p.x - p1.x) * (p2.y - p1.y) - (p2.x - p1.x) * (p.y - p1.y);
}

//ȷ߶p1p2ߵĵpǷ߶p1p2
inline int TriangleIntersectMachine::on_segment(point p1, point p2, point p)
{
	double max = p1.x > p2.x ? p1.x : p2.x;
	double min = p1.x < p2.x ? p1.x : p2.x;
	double max1 = p1.y > p2.y ? p1.y : p2.y;
	double min1 = p1.y < p2.y ? p1.y : p2.y;
	if (p.x >= min && p.x <= max && p.y >= min1 && p.y <= max1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

//ж߶p1p2߶p3p4Ƿཻ
inline int TriangleIntersectMachine::segments_intersert(point p1, point p2, point p3, point p4)
{
	double d1, d2, d3, d4;
	d1 = this->direction(p3, p4, p1);
	d2 = this->direction(p3, p4, p2);
	d3 = this->direction(p1, p2, p3);
	d4 = this->direction(p1, p2, p4);
	if (d1 * d2 < 0 && d3 * d4 < 0)
	{
		return 1;
	}
	else if (d1 == 0 && on_segment(p3, p4, p1) == 1)
	{
		return 1;
	}
	else if (d2 == 0 && on_segment(p3, p4, p2) == 1)
	{
		return 1;
	}
	else if (d3 == 0 && on_segment(p1, p2, p3) == 1)
	{
		return 1;
	}
	else if (d4 == 0 && on_segment(p1, p2, p4) == 1)
	{
		return 1;
	}
	return 0;
}

//жͬһƽֱߺǷཻ
inline bool TriangleIntersectMachine::line_triangle_intersert_inSamePlane(FFace *tri, Point3m f1, Point3m f2)
{
	point p1, p2, p3, p4;

	copy_point(p1, f1);

	copy_point(p2, f2);

	copy_point(p3, tri->v1);

	copy_point(p4, tri->v2);

	if (segments_intersert(p1, p2, p3, p4))
	{
		return true;
	}

	copy_point(p3, tri->v2);

	copy_point(p4, tri->v3);

	if (segments_intersert(p1, p2, p3, p4))
	{
		return true;
	}

	copy_point(p3, tri->v1);

	copy_point(p4, tri->v3);

	if (segments_intersert(p1, p2, p3, p4))
	{
		return true;
	}

	return false;
}

inline bool TriangleIntersectMachine::triangle_intersert_inSamePlane(FFace *tri1, FFace *tri2)
{
	if (line_triangle_intersert_inSamePlane(tri2, tri1->v1, tri1->v2))
	{
		return true;
	}
	else if (line_triangle_intersert_inSamePlane(tri2, tri1->v2, tri1->v3))
	{
		return true;
	}
	else if (line_triangle_intersert_inSamePlane(tri2, tri1->v1, tri1->v3))
	{
		return true;
	}
	else
	{
		return false;
	}
}

//ķжϵǷڲ
inline bool TriangleIntersectMachine::is_point_within_triangle(FFace *tri, Point3m p)
{
	Point3m v0;
	v0 = tri->v3 - tri->v1;
    Point3m v1;
	v1 = tri->v2 - tri->v1;
	Point3m v2;
	v2 = p - tri->v1;
	float dot00 = v0 * v0;
	float dot01 = v0 * v1;
	float dot02 = v0 * v2;
	float dot11 = v1 * v1;
	float dot12 = v1 * v2;
	float inverDeno = 1 / (dot00* dot11 - dot01* dot01);
	float u = (dot11* dot02 - dot01* dot12) * inverDeno;
	if (u < 0 || u > 1) // if u out of range, return directly
	{
		return false;
	}
	float v = (dot00* dot12 - dot01* dot02) * inverDeno;
	if (v < 0 || v > 1) // if v out of range, return directly
	{
		return false;
	}
	return u + v <= 1;
}

//Devillers㷨
TopologicalStructure TriangleIntersectMachine::judge_triangle_topologicalStructure(FFace *tri1, FFace *tri2)
{
	//tri1ڵƽΪp1,tri2ڵƽΪp2
	float p1_tri2_vertex1 = get_vector4_det(tri1->v1, tri1->v2, tri1->v3, tri2->v1);

	float p1_tri2_vertex2 = get_vector4_det(tri1->v1, tri1->v2, tri1->v3, tri2->v2);

	float p1_tri2_vertex3 = get_vector4_det(tri1->v1, tri1->v2, tri1->v3, tri2->v3);

	if (p1_tri2_vertex1 > 0 && p1_tri2_vertex2 > 0 && p1_tri2_vertex3 > 0)
	{
		return NONINTERSECT;
	}

	if (p1_tri2_vertex1 < 0 && p1_tri2_vertex2 < 0 && p1_tri2_vertex3 < 0)
	{
		return NONINTERSECT;
	}

	if (p1_tri2_vertex1 == 0 && p1_tri2_vertex2 == 0 && p1_tri2_vertex3 == 0)
	{
		if (triangle_intersert_inSamePlane(tri1, tri2))
		{
			return INTERSECT;
		}
		else
		{
			return NONINTERSECT;
		}
	}

	if (p1_tri2_vertex1 == 0 && p1_tri2_vertex2 * p1_tri2_vertex3 > 0)
	{
		if (is_point_within_triangle(tri1, tri2->v1))
		{
			return INTERSECT;
		}
		else
		{
			return NONINTERSECT;
		}
	}
	else if (p1_tri2_vertex2 == 0 && p1_tri2_vertex1 * p1_tri2_vertex3 > 0)
	{
		if (is_point_within_triangle(tri1, tri2->v2))
		{
			return INTERSECT;
		}
		else
		{
			return NONINTERSECT;
		}
	}
	else if (p1_tri2_vertex3 == 0 && p1_tri2_vertex1 * p1_tri2_vertex2 > 0)
	{
		if (is_point_within_triangle(tri1, tri2->v3))
		{
			return INTERSECT;
		}
		else
		{
			return NONINTERSECT;
		}
	}

	float p2_tri1_vertex1 = get_vector4_det(tri2->v1, tri2->v2, tri2->v3, tri1->v1);
	float p2_tri1_vertex2 = get_vector4_det(tri2->v1, tri2->v2, tri2->v3, tri1->v2);
	float p2_tri1_vertex3 = get_vector4_det(tri2->v1, tri2->v2, tri2->v3, tri1->v3);

	if (p2_tri1_vertex1 > 0 && p2_tri1_vertex2 > 0 && p2_tri1_vertex3 > 0)
	{
		return NONINTERSECT;
	}

	if (p2_tri1_vertex1 < 0 && p2_tri1_vertex2 < 0 && p2_tri1_vertex3 < 0)
	{
		return NONINTERSECT;
	}

	if (p2_tri1_vertex1 == 0 && p2_tri1_vertex2 * p2_tri1_vertex3 > 0)
	{
		if (is_point_within_triangle(tri2, tri1->v1))
		{
			return INTERSECT;
		}
		else
		{
			return NONINTERSECT;
		}
	}

	if (p2_tri1_vertex2 == 0 && p2_tri1_vertex1 * p2_tri1_vertex3 > 0)
	{
		if (is_point_within_triangle(tri2, tri1->v2))
		{
			return INTERSECT;
		}
		else
		{
			return NONINTERSECT;
		}
	}

	if (p2_tri1_vertex3 == 0 && p2_tri1_vertex1 * p2_tri1_vertex2 > 0)
	{
		if (is_point_within_triangle(tri2, tri1->v3))
		{
			return INTERSECT;
		}
		else
		{
			return NONINTERSECT;
		}
	}

	Point3m tri1_a = tri1->v1, tri1_b = tri1->v2, tri1_c = tri1->v3
		, tri2_a = tri2->v1, tri2_b = tri2->v2, tri2_c = tri2->v3;

	Point3m m;

	float im;

	if (p2_tri1_vertex2 * p2_tri1_vertex3 >= 0 && p2_tri1_vertex1 != 0)
	{
		if (p2_tri1_vertex1 < 0)
		{
			m = tri2_b;
			tri2_b = tri2_c;
			tri2_c = m;

			im = p1_tri2_vertex2;
			p1_tri2_vertex2 = p1_tri2_vertex3;
			p1_tri2_vertex3 = im;
		}
	}
	else if (p2_tri1_vertex1 * p2_tri1_vertex3 >= 0 && p2_tri1_vertex2 != 0)
	{
		m = tri1_a;
		tri1_a = tri1_b;
		tri1_b = tri1_c;
		tri1_c = m;

		if (p2_tri1_vertex2 < 0)
		{
			m = tri2_b;
			tri2_b = tri2_c;
			tri2_c = m;

			im = p1_tri2_vertex2;
			p1_tri2_vertex2 = p1_tri2_vertex3;
			p1_tri2_vertex3 = im;
		}
	}
	else if (p2_tri1_vertex1 * p2_tri1_vertex2 >= 0 && p2_tri1_vertex3 != 0)
	{
		m = tri1_a;

		tri1_a = tri1_c;

		tri1_c = tri1_b;

		tri1_b = m;

		if (p2_tri1_vertex3 < 0)
		{
			m = tri2_b;
			tri2_b = tri2_c;
			tri2_c = m;

			im = p1_tri2_vertex2;
			p1_tri2_vertex2 = p1_tri2_vertex3;
			p1_tri2_vertex3 = im;
		}
	}

	if (p1_tri2_vertex2 * p1_tri2_vertex3 >= 0 && p1_tri2_vertex1 != 0)
	{
		if (p1_tri2_vertex1 < 0)
		{
			m = tri1_b;
			tri1_b = tri1_c;
			tri1_c = m;
		}
	}
	else if (p1_tri2_vertex1 * p1_tri2_vertex3 >= 0 && p1_tri2_vertex2 != 0)
	{
		m = tri2_a;

		tri2_a = tri2_b;

		tri2_b = tri2_c;

		tri2_c = m;

		if (p1_tri2_vertex2 < 0)
		{
			m = tri1_b;
			tri1_b = tri1_c;
			tri1_c = m;
		}
	}
	else if (p1_tri2_vertex1 * p1_tri2_vertex2 >= 0 && p1_tri2_vertex3 != 0)
	{
		m = tri2_a;

		tri2_a = tri2_c;

		tri2_c = tri2_b;

		tri2_b = m;

		if (p1_tri2_vertex3 < 0)
		{
			m = tri1_b;
			tri1_b = tri1_c;
			tri1_c = m;
		}
	}

	if (get_vector4_det(tri1_a, tri1_b, tri2_a, tri2_b) <= 0 && get_vector4_det(tri1_a, tri1_c, tri2_c, tri2_a) <= 0)
	{
		return INTERSECT;
	}
	else
	{
		return NONINTERSECT;
	}
}
