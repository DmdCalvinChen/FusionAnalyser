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

#include "meshshaderrender.h"
#include  "common_base/util/uitools.h"

MeshShaderRender::MeshShaderRender()
{

}

MeshShaderRender::~MeshShaderRender()
{

}

void MeshShaderRender::LoadShader()
{
	QDomDocument doc;

	QString exePath = qApp->applicationDirPath();
	//QFile file(":/shaders/phong.gdp");

	QFile file(exePath + "/shaders/phong.gdp");
	QDir shadersDir = QDir(qApp->applicationDirPath());
	shadersDir.cd("shaders");

	QString name_gdp("phong.gdp");
	if (file.open(QIODevice::ReadOnly))
	{
		if (doc.setContent(&file))
		{
			file.close();

			QDomElement root = doc.documentElement();
			if (root.nodeName() == "GLSLang")
			{
				ShaderInfo si;

				QDomElement elem;

				//Vertex program filename
				elem = root.firstChildElement("VPCount");
				if (!elem.isNull())
				{
					//first child of VPCount is "Filenames"
					QDomNode child = elem.firstChild();
					if (!child.isNull())
					{
						//first child of "Filenames" is "Filename0"
						child = child.firstChild();
						//si.vpFile = ":/shaders/phong.vert"/*shadersDir.absoluteFilePath((child.toElement()).attribute("VertexProgram", ""))*/;
						si.vpFile = shadersDir.absoluteFilePath((child.toElement()).attribute("VertexProgram", ""));
					}
				}

				//Fragment program filename
				elem = root.firstChildElement("FPCount");
				if (!elem.isNull())
				{
					//first child of FPCount is "Filenames"
					QDomNode child = elem.firstChild();
					if (!child.isNull())
					{
						//first child of "Filenames" is "Filename0"
						child = child.firstChild();
						//si.fpFile = ":/shaders/phong.frag"/*shadersDir.absoluteFilePath((child.toElement()).attribute("FragmentProgram", ""))*/;
						si.fpFile = shadersDir.absoluteFilePath((child.toElement()).attribute("FragmentProgram", ""));
					}
				}

				//Uniform Variables
				elem = root.firstChildElement("UniformVariables");
				if (!elem.isNull())
				{

					QDomNode unif = elem.firstChild();
					while (!unif.isNull())
					{

						UniformVariable uv;

						QDomElement unifElem = unif.toElement();
						QString unifVarName = unifElem.attribute("Name", "");

						uv.type = (unifElem.attribute("Type", "")).toInt();
						uv.widget = (unifElem.attribute("Widget", "")).toInt();
						uv.min = (unifElem.attribute("Min", "")).toFloat();
						uv.max = (unifElem.attribute("Max", "")).toFloat();
						uv.step = (unifElem.attribute("Step", "")).toFloat();

						QDomNode unifElemValue = unifElem.firstChild();

						if (!unifElemValue.isNull())
						{

							switch (uv.type)
							{
							case SINGLE_INT:
							{
								uv.ival[0] = unifElemValue.toElement().attribute("Value0", 0).toInt();
							} break;
							case SINGLE_FLOAT:
							{
								uv.fval[0] = unifElemValue.toElement().attribute("Value0", 0).toFloat();
							} break;
							case ARRAY_2_FLOAT:
							{
								uv.fval[0] = unifElemValue.toElement().attribute("Value0", 0).toFloat();
								uv.fval[1] = unifElemValue.toElement().attribute("Value1", 0).toFloat();
							} break;
							case ARRAY_3_FLOAT:
							{
								uv.fval[0] = unifElemValue.toElement().attribute("Value0", 0).toFloat();
								uv.fval[1] = unifElemValue.toElement().attribute("Value1", 0).toFloat();
								uv.fval[2] = unifElemValue.toElement().attribute("Value2", 0).toFloat();
							} break;
							case ARRAY_4_FLOAT:
							{
								uv.fval[0] = unifElemValue.toElement().attribute("Value0", 0).toFloat();
								uv.fval[1] = unifElemValue.toElement().attribute("Value1", 0).toFloat();
								uv.fval[2] = unifElemValue.toElement().attribute("Value2", 0).toFloat();
								uv.fval[3] = unifElemValue.toElement().attribute("Value3", 0).toFloat();
							} break;
							default:
							{

							} break;
							}

							si.uniformVars[unifVarName] = uv;
						}

						unif = unif.nextSibling();
					}
				}

				//OpenGL Status
				elem = root.firstChildElement("FragmentProcessor");
				if (!elem.isNull())
				{
					if (elem.hasAttribute("Shade"))					si.glStatus[SHADE] = elem.attribute("Shade", "0");
					if (elem.hasAttribute("AlphaTest"))			si.glStatus[ALPHA_TEST] = elem.attribute("AlphaTest", "False");
					if (elem.hasAttribute("AlphaFunc"))			si.glStatus[ALPHA_FUNC] = elem.attribute("AlphaFunc", "0");
					if (elem.hasAttribute("AlphaClamp"))		si.glStatus[ALPHA_CLAMP] = elem.attribute("AlphaClamp", "0");
					if (elem.hasAttribute("Blending"))			si.glStatus[BLENDING] = elem.attribute("Blending", "False");
					if (elem.hasAttribute("BlendFuncSRC"))	si.glStatus[BLEND_FUNC_SRC] = elem.attribute("BlendFuncSRC", "0");
					if (elem.hasAttribute("BlendFuncDST"))	si.glStatus[BLEND_FUNC_DST] = elem.attribute("BlendFuncDST", "0");
					if (elem.hasAttribute("BlendEquation")) si.glStatus[BLEND_EQUATION] = elem.attribute("BlendEquation", "0");
					if (elem.hasAttribute("DepthTest"))			si.glStatus[DEPTH_TEST] = elem.attribute("DepthTest", "False");
					if (elem.hasAttribute("DepthFunc"))			si.glStatus[DEPTH_FUNC] = elem.attribute("DepthFunc", "0");
					if (elem.hasAttribute("ClampNear"))			si.glStatus[CLAMP_NEAR] = elem.attribute("ClampNear", "0");
					if (elem.hasAttribute("ClampFar"))			si.glStatus[CLAMP_FAR] = elem.attribute("ClampFar", "0");
					if (elem.hasAttribute("ClearColorR"))		si.glStatus[CLEAR_COLOR_R] = elem.attribute("ClearColorR", "0");
					if (elem.hasAttribute("ClearColorG"))		si.glStatus[CLEAR_COLOR_G] = elem.attribute("ClearColorG", "0");
					if (elem.hasAttribute("ClearColorB"))		si.glStatus[CLEAR_COLOR_B] = elem.attribute("ClearColorB", "0");
					if (elem.hasAttribute("ClearColorA"))		si.glStatus[CLEAR_COLOR_A] = elem.attribute("ClearColorA", "0");
				}
				shaders[name_gdp] = si;
			}
		}
		else
		{
			file.close();
		}
	}
}

void MeshShaderRender::reLoadShader()
{
	//glDeleteShader()
}

void MeshShaderRender::Init()
{
	LoadShader();
	GLenum err = glewInit();
	if (GLEW_OK == err) {
		if (GLEW_ARB_vertex_program && GLEW_ARB_fragment_program) {

			if (!shaders.empty()) {

				supported = true;

				v = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
				f = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

				const char* vv;
				const char* ff;

				QFile fs_file(shaders["phong.gdp"].fpFile);
				bool succeed = fs_file.open(QIODevice::ReadOnly);
				std::string data_string_frag;
				if (succeed)
				{
					QByteArray fsData = fs_file.readAll();
					data_string_frag = std::string(fsData.constData(), fsData.size());
					ff = data_string_frag.data();
				}
				fs_file.close();

				QFile vs_file(shaders["phong.gdp"].vpFile);
				succeed = vs_file.open(QIODevice::ReadOnly);
				std::string data_string_vert;
				if (succeed)
				{
					QByteArray vsData = vs_file.readAll();
					data_string_vert = std::string(vsData.constData(), vsData.size());
					vv = data_string_vert.data();
				}
				vs_file.close();

				glShaderSourceARB(v, 1, &vv, NULL);
				glShaderSourceARB(f, 1, &ff, NULL);

				glCompileShaderARB(v);
				glCompileShaderARB(f);

				GLint statusV;
				GLint statusF;

				glGetObjectParameterivARB(v, GL_OBJECT_COMPILE_STATUS_ARB, &statusV);
				glGetObjectParameterivARB(f, GL_OBJECT_COMPILE_STATUS_ARB, &statusF);

				if (statusF && statusV) { //successful compile
					shaders[shaders.begin()->first].shaderProg = glCreateProgramObjectARB();
					glAttachObjectARB(shaders[shaders.begin()->first].shaderProg, v);
					glAttachObjectARB(shaders[shaders.begin()->first].shaderProg, f);
					glLinkProgramARB(shaders[shaders.begin()->first].shaderProg);

					GLint linkStatus;
					glGetObjectParameterivARB(shaders[shaders.begin()->first].shaderProg, GL_OBJECT_LINK_STATUS_ARB, &linkStatus);

					if (linkStatus) {
						map<QString, UniformVariable>::iterator i = shaders[shaders.begin()->first].uniformVars.begin();
						while (i != shaders[shaders.begin()->first].uniformVars.end()) {
							(shaders[shaders.begin()->first].uniformVars[i->first]).location =
								glGetUniformLocationARB(shaders[shaders.begin()->first].shaderProg,
								(i->first).toLocal8Bit().data());
							++i;
						}

					}
					else
					{
						QFile file("shaders.log");
						if (file.open(QFile::Append))
						{
							char proglog[2048];
							GLsizei length;
							QTextStream out(&file);

							glGetProgramiv(v, GL_LINK_STATUS, &statusV);
							glGetProgramInfoLog(v, 2048, &length, proglog);
							out << "VERTEX SHADER LINK INFO:" << endl;
							out << proglog << endl << endl;

							glGetProgramiv(f, GL_LINK_STATUS, &statusF);
							glGetProgramInfoLog(f, 2048, &length, proglog);
							out << "FRAGMENT SHADER LINK INFO:" << endl << endl;
							out << proglog << endl << endl;

							file.close();
						}

						//QMessageBox::critical(0, "Meshlab",
						//	QString("An error occurred during shader's linking.\n") +
						//	"See shaders.log for further details about this error.\n");
					}

					//Textures

					std::vector<TextureInfo>::iterator tIter = shaders[shaders.begin()->first].textureInfo.begin();
					while (tIter != shaders[shaders.begin()->first].textureInfo.end())
					{
						glEnable(tIter->Target);
						QImage img, imgScaled, imgGL;
						bool opened = img.load(tIter->path);
						if (!opened)
						{
							supported = false;
							return;
						}
						// image has to be scaled to a 2^n size. We choose the first 2^N <= picture size.
						int bestW = pow(2.0, floor(::log(double(img.width())) / ::log(2.0)));
						int bestH = pow(2.0, floor(::log(double(img.height())) / ::log(2.0)));
						imgScaled = img.scaled(bestW, bestH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
						imgGL = QGLWidget::convertToGLFormat(imgScaled);

						glGenTextures(1, &(tIter->tId));
						glBindTexture(tIter->Target, tIter->tId);
						glTexImage2D(tIter->Target, 0, 3, imgGL.width(), imgGL.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, imgGL.bits());
						glTexParameteri(tIter->Target, GL_TEXTURE_MIN_FILTER, tIter->MinFilter);
						glTexParameteri(tIter->Target, GL_TEXTURE_MAG_FILTER, tIter->MagFilter);
						glTexParameteri(tIter->Target, GL_TEXTURE_WRAP_S, tIter->WrapS);
						glTexParameteri(tIter->Target, GL_TEXTURE_WRAP_T, tIter->WrapT);
						glTexParameteri(tIter->Target, GL_TEXTURE_WRAP_R, tIter->WrapR);

						++tIter;
					}
				}
				else
				{
					QFile file("shaders.log");
					if (file.open(QFile::WriteOnly))
					{
						char shlog[2048];
						GLsizei length;
						QTextStream out(&file);

						glGetShaderiv(v, GL_COMPILE_STATUS, &statusV);
						glGetShaderInfoLog(v, 2048, &length, shlog);
						out << "VERTEX SHADER COMPILE INFO:" << endl << endl;
						out << shlog << endl << endl;

						glGetShaderiv(f, GL_COMPILE_STATUS, &statusF);
						glGetShaderInfoLog(f, 2048, &length, shlog);
						out << "FRAGMENT SHADER COMPILE INFO:" << endl << endl;
						out << shlog << endl << endl;

						file.close();
					}

					UiUtilityTools::getInstance()->showInfoMessageBox(QObject::tr("error"), QObject::tr("An error occurred during shader's compiling.\n"
						"See shaders.log for further details about this error."));
				}
			}
		}
	}

	// * clear the errors, if any
	glGetError();
}

void MeshShaderRender::Render()
{
	if (shaders.empty())
	{
		return;
	}

	if (supported) {

		ShaderInfo si = shaders[shaders.begin()->first];
		glUseProgramObjectARB(si.shaderProg);

		map<QString, UniformVariable>::iterator i = si.uniformVars.begin();
		while (i != si.uniformVars.end()) {
			switch (i->second.type) {
			case SINGLE_INT: {
				glUniform1iARB(i->second.location, i->second.ival[0]);
			} break;
			case SINGLE_FLOAT: {
				glUniform1fARB(i->second.location, i->second.fval[0]);
			} break;
			case ARRAY_2_FLOAT: {
				glUniform2fARB(i->second.location, i->second.fval[0], i->second.fval[1]);
			} break;
			case ARRAY_3_FLOAT: {
				glUniform3fARB(i->second.location, i->second.fval[0], i->second.fval[1], i->second.fval[2]);
			} break;
			case ARRAY_4_FLOAT: {
				glUniform4fARB(i->second.location, i->second.fval[0], i->second.fval[1], i->second.fval[2], i->second.fval[3]);
			} break;
			default: {} break;
			}
			++i;
		}
	}
}

