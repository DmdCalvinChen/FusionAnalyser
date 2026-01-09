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

#ifndef  CLOUDSHADER_H
#define CLOUDSHADER_H

#include <gl/glew.h>
#include <qopengl.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "meshExt/absmesh.h"
#include "meshExt/tooth.h"

class CloudShader
{
public:
	// Program ID
	unsigned int ID;

	// Constructor reads and builds shader
	CloudShader(const GLchar* vertexPath, const GLchar* fragmentPath);
	// Use/activate program
	void use();
	// Uniform utility functions
	 void setBool(const std::string &name, bool value) const;
	 void setInt(const std::string &name, int value) const;
	 void setFloat(const std::string &name, float value) const;
	 void setVec3(const std::string &name, vcg::Point3f &value) const;
	 void setVec3(const std::string &name, float x, float y, float z) const;
	 void setMat4(const std::string &name, const GLfloat* mat) const;
 private:
	 void checkCompileErrors(unsigned int shader, std::string type);
 };
#endif // ! CLOUDSHADER_H
