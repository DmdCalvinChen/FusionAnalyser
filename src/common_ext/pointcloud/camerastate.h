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

#ifndef CAMERASTATE_H
#define CAMERASTATE_H

struct CameraState
{
	CameraState() {}
    void update(float _width, float _height, Point3m _view_vec, Point3m _right_vec, Point3m _camera_pos/*, GLdouble *_mm, GLdouble *_pm, GLint *_viewport*/)
	{
		glGetIntegerv(GL_VIEWPORT, this->view_prot_);
		glGetFloatv(GL_MODELVIEW_MATRIX, this->mm_);
		glGetFloatv(GL_PROJECTION_MATRIX, this->pm_);

		screen_width_ = _width;
		screen_height_ = _height;
		view_vec_ = _view_vec;
		right_vec_ = _right_vec;
		camera_pos_ = _camera_pos;
		model_ = vcg::Matrix44f(mm_);
		project_ = vcg::Matrix44f(pm_);
		model_ = model_.transpose();
		project_ = project_.transpose();

#if PERSPECTIVE_STATE
		// Perspective projection
		n_ = project_[2][3] / (project_[2][2] - 1);
		f_ = project_[2][3] / (project_[2][2] + 1);
		r_ = n_ / project_[0][0];
		l_ = -r_;
		t_ = n_ / project_[1][1];
		b_ = -t_;
#else
		// Orthographic projection
		n_ = (project_[2][3] + 1) / project_[2][2];
		f_ = (project_[2][3] - 1) / project_[2][2];
		r_ = n_ / project_[0][0];
		l_ = -r_;
		t_ = n_ / project_[1][1];
		b_ = -t_;
#endif
	}

public:
	Point3m view_vec_, right_vec_, camera_pos_;
	vcg::Matrix44f model_, project_, view_;
	float r_, l_, t_, b_, n_, f_;//right,left,top,below,near,far
	float screen_width_, screen_height_;

	GLfloat mm_[16], pm_[16];
	GLint view_prot_[4];
};

#endif // !CAMERASTATE_H
