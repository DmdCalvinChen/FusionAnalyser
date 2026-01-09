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

#include "decoratefusionplugin.h"
#include <wrap/qt/gl_label.h>
#include <QDesktopWidget>
#include <QDockWidget>
#include <QMainWindow>
#include "common_base/SignalManager.h"

#include "common_ext/bolton/BoltonAna.h"
#include "UI_Common/boltonAna/BoltonAnaDlg.h"
#include "wrap/gl/addons.h"
#include "FusionAnalyser/glarea.h"
using namespace common_ext;

DecorateFusionPlugin::DecorateFusionPlugin()
{
	if (translator_ == nullptr)
		translator_ = new QTranslator;

	if (PFusionAppData->getAppLanguage() == E_CHINESE)
		translator_->load(":/decorateFusionPlugin_ch.qm");
	else if (PFusionAppData->getAppLanguage() == E_ENGLISH)
		translator_->load("decorateFusionPlugin_en.ts");

		qApp->installTranslator(translator_);

	typeList <<
		DP_SHOW_MEASUREGRID <<
		DP_SHOW_MEASURE <<
		DP_SHOW_TOOTHFDI <<
		DP_SHOW_TOOTHOCCLUSION <<
		DP_SHOW_TOOTHBOLTON;
	this->setObjectName(tr("decorate fusion plugin"));
	FilterIDType tt;
	iconName.append("archlength");
	iconName.append("archwide");
	iconName.append("bollton_ana");
	iconName.append("Bolton");
	iconName.append("crowding");
	iconName.append("fdi");
	iconName.append("grid");
	iconName.append("measure");
	iconName.append("midline");
	iconName.append("molar");
	iconName.append("overlap");
	iconName.append("palateheight");
	iconName.append("report");
	iconName.append("spee");
	iconName.append("toothOcclusion");
	for (int num = 0; num < iconName.size(); num++)
	{
		QPixmap enabled_icon(QString(":/%1/images/%1/toolbarSvg/%2_normal.svg").arg(PFusionAppData->getSkinName()).arg(iconName[num]));
		QPixmap disabled_icon(QString(":/%1/images/%1/toolbarSvg/%2_disabled.svg").arg(PFusionAppData->getSkinName()).arg(iconName[num]));
		QIcon icon(enabled_icon);
		icon.addPixmap(disabled_icon, QIcon::Disabled);
		iconChange[iconName[num]] = icon;
	}
	foreach(tt, types())
	{
		QAction* pAct = new QAction(decorationName(tt), this);
		actionList << pAct;
		pAct->setToolTip(decorationInfo(tt));
		switch (tt)
		{
		case DP_SHOW_MEASUREGRID:
			pAct->setIcon(iconChange["grid"]);
			pAct->setObjectName("grid");
			pAct->setData(E_ProgramAnalyser);
			break;
		case DP_SHOW_MEASURE:
			pAct->setIcon(iconChange["measure"]);
			pAct->setData(E_ProgramAnalyser);
			pAct->setObjectName("measure");
			break;
		case DP_SHOW_TOOTHFDI:
			pAct->setIcon(iconChange["fdi"]);
			pAct->setData(E_ProgramAnalyser);
			pAct->setObjectName(SHOW_FDI_ACT_NAME);
			break;

		case DP_SHOW_TOOTHBOLTON:
			pAct->setIcon(iconChange["bollton_ana"]);
			pAct->setData(E_ProgramAnalyser);
			pAct->setObjectName("Bolton");
			break;

		default:
			break;
		}
	}

	QAction* ap = nullptr;
	foreach(ap, actionList)
	{
		ap->setCheckable(true);
	}

	connect(PSIGNALMANAGER, &SignalManager::transmitPickedPosSignal, this, &DecorateFusionPlugin::mousePickedVertexReceivedSlot);
}

DecorateFusionPlugin::~DecorateFusionPlugin()
{
	if (!translator_)
	{
		qApp->removeTranslator(translator_);
		SAFE_DELETE(translator_);
	}
}

void DecorateFusionPlugin::updateAnalyzerAvaliableItemActionsSlot(std::vector<EditItemIndex> _avaliable_item_indexes)
{
	for (auto& action : this->actionList)
	{
		switch (ID(action))
		{
		case DP_SHOW_TOOTHOCCLUSION:
		{
//			action->setIcon(iconChange["toothOcclusion"]);
			action->setEnabled(false);
			for (auto& item_index : _avaliable_item_indexes)
			{
				switch (item_index)
				{
                case EditItemIndex(OCCLUSION_COLORING):
					action->setEnabled(true);
					break;
				default:
					break;
				}
			}
			break;
		}

		default:
			break;
		}
	}
	return;
}

bool DecorateFusionPlugin::startDecorate(QAction* act, MeshModel& m, RichParameterSet* rm, GLArea* glarea)
{
	width_ = glarea->width();
	height_ = glarea->height();
	switch (ID(act))
	{
	default:
		break;
	}
	return true;
}

bool DecorateFusionPlugin::startDecorate(QAction* act, MeshDocument& m, RichParameterSet*, GLArea* gla)
{
	assert(act);
	assert(gla);
    if (!act->isChecked())
        return true;
	if (parent_ == nullptr)
	{
		parent_ = gla;
	}

	width_ = gla->width();
	height_ = gla->height();
	switch (ID(act))
	{
	case DP_SHOW_MEASUREGRID:
		if (!m.meshList.isEmpty())
		{
			act->setIcon(QIcon(QPixmap(QString(":/%1/images/%1/grid_pressed.svg").arg(PFusionAppData->getSkinName()))));
//			act->setIcon(QIcon(":/svg/images/grid_pressed.svg"));
			//gla->tabPerspectiveAndOrthographicProjection = false;
			gla->update();
		}
		break;
	case DP_SHOW_MEASURE:
		if (!m.meshList.isEmpty())
		{
			act->setIcon(QIcon(QPixmap(QString(":/%1/images/%1/measure_pressed.svg").arg(PFusionAppData->getSkinName()))));
			// Clear old measurement data
			pos_picked_for_measure_.clear();
			gla->update();
		}
	break;
    case DP_SHOW_TOOTHFDI:
		act->setIcon(QIcon(QPixmap(QString(":/%1/images/%1/fdi_pressed.svg").arg(PFusionAppData->getSkinName()))));
//        act->setIcon(QIcon(":/svg/images/fdi_pressed.svg"));
    break;
	case DP_SHOW_TOOTHBOLTON:
		act->setIcon(QIcon(QPixmap(QString(":/%1/images/%1/bollton_ana_pressed.svg").arg(PFusionAppData->getSkinName()))));
//		act->setIcon(QIcon(":/svg/images/bollton_ana_pressed.svg"));
		showBoltonAnaResult(m, gla, act);
		break;
    case DP_SHOW_TOOTHOCCLUSION:
		{
			static bool updating_occlusion_color = false;
			if (updating_occlusion_color)
			{
				return false;
			}

			act->setIcon(QIcon(QPixmap(QString(":/%1/images/%1/toothOcclusion_pressed.svg").arg(PFusionAppData->getSkinName()))));
			CMeshO* upper_mesh = nullptr, * lower_mesh = nullptr;
			if (PFusionAlignData->upper_dental_features_ != nullptr && PFusionAlignData->lower_dental_features_ != nullptr)
			{
				upper_mesh = &PFusionAlignData->upper_dental_features_->mesh_->original_mesh_->cm;
				lower_mesh = &PFusionAlignData->lower_dental_features_->mesh_->original_mesh_->cm;
			}
			else
			{
				for (auto& mesh_model : parent_->md()->meshList)
				{
					if (mesh_model->upperOrLowerToothModelMark == UpperToothModel)
					{
						upper_mesh = &mesh_model->cm;
					}
					else if (mesh_model->upperOrLowerToothModelMark == LowerToothModel)
					{
						lower_mesh = &mesh_model->cm;
					}
				}
				if (upper_mesh != nullptr && lower_mesh != nullptr)
				{
					vcg::tri::UpdateBounding<CMeshO>::Box(*upper_mesh);
					vcg::tri::UpdateBounding<CMeshO>::Box(*lower_mesh);
				}
			}

			if (upper_mesh != nullptr && lower_mesh != nullptr && !updating_occlusion_color)
			{
				act->setEnabled(false);
				QPixmap enabled_icon(QString(":/%1/images/%1/toothOcclusion_pressed.svg").arg(PFusionAppData->getSkinName()));
				QPixmap disabled_icon(QString(":/%1/images/%1/toothOcclusion_pressed.svg").arg(PFusionAppData->getSkinName()));
				QIcon icon(enabled_icon);
				icon.addPixmap(disabled_icon, QIcon::Disabled);
				act->setIcon(icon);
				updating_occlusion_color = true;
				UtilityTools::getInstance()->occlusionColoringTwoMesh(upper_mesh, lower_mesh);
				updating_occlusion_color = false;
				act->setEnabled(true);
			}
		}

        break;
	default:
		break;
	}
	return true;
}

void DecorateFusionPlugin::decorateMesh(QAction* a, MeshModel& mm, RichParameterSet*, GLArea* gla, QPainter* painter, GLLogStream& _log)
{
	switch (ID(a))
	{
	default:
		break;
	}
}

void DecorateFusionPlugin::decoratePostDoc(QAction* a, MeshDocument& mm, RichParameterSet*, GLArea* gla, QPainter* painter, GLLogStream& _log)
{
	int width = gla->width();
	int height = gla->height();

	if (width_ != width || height_ != height)
	{
		width_ = width;
		height_ = height;
		allocateOverlapBuffer();
	}
	switch (ID(a))
	{
	case DP_SHOW_TOOTHOCCLUSION:
		{
			if (bEnableDisplayDentalOverlapArea && !parent_->bRolling)
			{
				glPushAttrib(GL_ALL_ATTRIB_BITS);
				glEnable(GL_COLOR_MATERIAL);
				glEnable(GL_LIGHTING);
				drawDentalOverlapArea(painter, gla);
				glPopAttrib();
				glDisable(GL_LIGHTING);
			}
		}
		break;
	}
}

bool DecorateFusionPlugin::decorateDoc(QAction* a, MeshDocument& md, RichParameterSet*, GLArea* gla, QPainter* painter, GLLogStream& _log)
{
	if (!a->isChecked())
		return false;
	gla->makeCurrent();

	switch (ID(a))
	{
	case DP_SHOW_MEASUREGRID:
	{
		if (!md.meshList.isEmpty())
		{
			float maxDistance = 250.0f;
			int  step = 500 * gla->trackball.track.sca;

			step = step < 1 ? 1 : step;
			step = step > 100 ? 100 : step;
			step = (100 - step) / 10;
			if (step < 1)
			{
				step = 1;
			}

			if (step < 7)
			{
				step = 1;
			}
			else
			{
				step = 5;
			}

			drawBackGrid(gla, painter, maxDistance, maxDistance, step, Color4b(12, 110, 180, 255));
		}

	}
	break;
    case DP_SHOW_TOOTHFDI:
        drawToothFDILabel(painter, *md.mm(), gla);
        break;
	case DP_SHOW_MEASURE:
		if (!md.meshList.isEmpty())
		{
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glEnable(GL_COLOR_MATERIAL);
			glEnable(GL_LIGHTING);
			drawMeasureResult(gla, painter);
			glPopAttrib();
			glDisable(GL_LIGHTING);
		}
        break;

    case DP_SHOW_TOOTHOCCLUSION:
		break;
	default:
		break;
	}

	return true;
}

void DecorateFusionPlugin::endDecorate(QAction* act/*mode*/, MeshModel&/*m*/, RichParameterSet*, GLArea* /*parent*/)
{
	switch (ID(act))
	{
	default:
		break;
	}
}

void DecorateFusionPlugin::endDecorate(QAction* act, MeshDocument& m, RichParameterSet*, GLArea* gla)
{
    if (act->isChecked())
        return;
	switch (ID(act))
	{
    case DP_SHOW_TOOTHFDI:
		act->setIcon(iconChange["fdi"]);
        break;
	case DP_SHOW_MEASUREGRID:
		act->setIcon(iconChange["grid"]);
		gla->update();
	break;

	case DP_SHOW_MEASURE:
		act->setIcon(iconChange["measure"]);
		gla->update();
	break;

	case DP_SHOW_TOOTHBOLTON:
		act->setIcon(iconChange["bollton_ana"]);
		if (p_bolton_dlg_)
		{
			p_bolton_dlg_->hide();
		}
		break;

    case DP_SHOW_TOOTHOCCLUSION:
		bEnableDisplayDentalOverlapArea = false;

		{
			act->setIcon(iconChange["toothOcclusion"]);
			CMeshO* upper_mesh = nullptr, * lower_mesh = nullptr;
			if (PFusionAlignData->upper_dental_features_ != nullptr && PFusionAlignData->lower_dental_features_ != nullptr)
			{
				upper_mesh = &PFusionAlignData->upper_dental_features_->mesh_->original_mesh_->cm;
				lower_mesh = &PFusionAlignData->lower_dental_features_->mesh_->original_mesh_->cm;
			}
			else
			{
				for (auto& mesh_model : parent_->md()->meshList)
				{
					if (mesh_model->upperOrLowerToothModelMark == UpperToothModel)
					{
						upper_mesh = &mesh_model->cm;
					}
					else if (mesh_model->upperOrLowerToothModelMark == LowerToothModel)
					{
						lower_mesh = &mesh_model->cm;
					}
				}
				if (upper_mesh != nullptr && lower_mesh != nullptr)
				{
					vcg::tri::UpdateBounding<CMeshO>::Box(*upper_mesh);
					vcg::tri::UpdateBounding<CMeshO>::Box(*lower_mesh);
				}
			}

			if (upper_mesh != nullptr && lower_mesh != nullptr)
			{
				for (int i = 0; i < upper_mesh->vert.size(); ++i)
				{
					upper_mesh->vert[i].C() = vcg::Color4b(255, 255, 255, 255);
				}
				for (int i = 0; i < lower_mesh->vert.size(); ++i)
				{
					lower_mesh->vert[i].C() = vcg::Color4b(255, 255, 255, 255);
				}
				emit PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
			}
		}

		emit PSIGNALMANAGER->occlusalColoringInProgress(bEnableDisplayDentalOverlapArea);
        break;
	default:
		break;
	}
}

void DecorateFusionPlugin::allocateOverlapBuffer()
{
	SAFE_DELETE_ARRAY(p_overlap_depth_buffer_);
	p_overlap_depth_buffer_ = new GL_TYPE_NM<Scalarm>::ScalarType[width_ * height_];
	SAFE_DELETE_ARRAY(p_up_mesh_v_buffer_);
	p_up_mesh_v_buffer_ = new GL_TYPE_NM<Scalarm>::ScalarType[width_ * height_];
	SAFE_DELETE_ARRAY(p_down_mesh_v_buffer_);
	p_down_mesh_v_buffer_ = new GL_TYPE_NM<Scalarm>::ScalarType[width_ * height_];
	SAFE_DELETE_ARRAY(p_up_mesh_nv_buffer_);
	p_up_mesh_nv_buffer_ = new GL_TYPE_NM<Scalarm>::ScalarType[width_ * height_];
	SAFE_DELETE_ARRAY(p_down_mesh_nv_buffer_);
	p_down_mesh_nv_buffer_ = new GL_TYPE_NM<Scalarm>::ScalarType[width_ * height_];
	SAFE_DELETE_ARRAY(p_up_mesh_buffer_);
	p_up_mesh_buffer_ = new GL_TYPE_NM<Scalarm>::ScalarType[width_ * height_];
	SAFE_DELETE_ARRAY(p_down_mesh_buffer_);
	p_down_mesh_buffer_ = new GL_TYPE_NM<Scalarm>::ScalarType[width_ * height_];
	SAFE_DELETE_ARRAY(p_overlap_pix_buffer_);
	p_overlap_pix_buffer_ = new GLfloat[width_ * height_ * 4];

	SAFE_DELETE_ARRAY(buffer_U_V);
	SAFE_DELETE_ARRAY(buffer_L_V);
	SAFE_DELETE_ARRAY(buffer_U_NV);
	SAFE_DELETE_ARRAY(buffer_L_NV);
	SAFE_DELETE_ARRAY(buffer_depth);
	buffer_U_V = new GL_TYPE_NM<Scalarm>::ScalarType[width_ * height_];
	buffer_L_V = new GL_TYPE_NM<Scalarm>::ScalarType[width_ * height_];
	buffer_U_NV = new GL_TYPE_NM<Scalarm>::ScalarType[width_ * height_];
	buffer_L_NV = new GL_TYPE_NM<Scalarm>::ScalarType[width_ * height_];
	buffer_depth = new GL_TYPE_NM<Scalarm>::ScalarType[width_ * height_];
}

void DecorateFusionPlugin::deleteOverlapBuffer()
{
	this->toothCP.clear();
	SAFE_DELETE_ARRAY(buffer_U_V);
	SAFE_DELETE_ARRAY(buffer_L_V);
	SAFE_DELETE_ARRAY(buffer_U_NV);
	SAFE_DELETE_ARRAY(buffer_L_NV);
	SAFE_DELETE_ARRAY(buffer_depth);
	SAFE_DELETE_ARRAY(p_overlap_depth_buffer_);
	SAFE_DELETE_ARRAY(p_up_mesh_v_buffer_);
	SAFE_DELETE_ARRAY(p_down_mesh_v_buffer_);
	SAFE_DELETE_ARRAY(p_up_mesh_nv_buffer_);
	SAFE_DELETE_ARRAY(p_down_mesh_nv_buffer_);
	SAFE_DELETE_ARRAY(p_up_mesh_buffer_);
	SAFE_DELETE_ARRAY(p_down_mesh_buffer_);
	SAFE_DELETE_ARRAY(p_overlap_pix_buffer_);
}

bool DecorateFusionPlugin::startPostDecorate(QAction* act, MeshDocument& md, RichParameterSet* rps, GLArea* glarea)
{
	width_ = glarea->width();
	height_ = glarea->height();
	switch (ID(act))
	{
	case DP_SHOW_TOOTHOCCLUSION:
		act->setIcon(QIcon(QPixmap(QString(":/%1/images/%1/toothOcclusion_pressed.svg").arg(PFusionAppData->getSkinName()))));

		break;
	default:
		break;
	}

	return true;
}
void DecorateFusionPlugin::endPostDecorate(QAction* act, MeshDocument& md, RichParameterSet* rps, GLArea* glarea)
{
	switch (ID(act))
	{
	case DP_SHOW_TOOTHOCCLUSION:
//		act->setIcon(QIcon(QPixmap(QString(":/%1/images/%1/toothOcclusion.svg").arg(PFusionAppData->getSkinName()))));
		act->setIcon(iconChange["toothOcclusion"]);

		break;
	default:
		break;
	}
}

bool DecorateFusionPlugin::isDecorationApplicable(QAction* action, const MeshModel& m, QString& ErrorMessage) const
{
	return true;
}

int DecorateFusionPlugin::getDecorationClass(QAction* action) const
{
	if (ID(action) == DP_SHOW_TOOTHOCCLUSION)
	{
		emit PSIGNALMANAGER->occlusionShowSignal();
	}
	switch (ID(action))
	{
	case DP_SHOW_MEASUREGRID:
	case DP_SHOW_MEASURE:
	case DP_SHOW_TOOTHBOLTON:
    case DP_SHOW_TOOTHFDI:
	case DP_SHOW_TOOTHOCCLUSION:
		return DecorateFusionPlugin::PerDocument;

	}
	assert(0);
	return 0;
}

bool DecorateFusionPlugin::judgeFdiLabelIsInView(CMeshO* mesh, const Point3f& fdiShowPos, const Point3f& normal, GLArea* gla)
{
	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLdouble worldX, worldY, worldZ;

	// Calculate mouse position in world coordinate system
	glPushMatrix();
	//glMultMatrix(mesh->Tr);
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
	GLfloat winX, winY, winZ;

	winX = gla->width() / 2;
	winY = gla->height() / 2;

	gluUnProject(winX, winY, 0, mvmatrix, projmatrix, viewport, &worldX,
		&worldY, &worldZ);

	glPopMatrix();

	Point3f endVertex(worldX, worldY, worldZ);
	Point3f direction = (endVertex - fdiShowPos).normalized();
	Point3f beginVertex = fdiShowPos + direction * 1.0f;

	float dot = normal.dot(direction);

	bool bSucceed = dot > 0.0 ? true : false;

	return bSucceed;
}

void DecorateFusionPlugin::drawBackGrid(GLArea* gla, QPainter* painter, float maxX, float maxY, float step,
	Color4b lineColor)
{
	//painter->save();
	//painter->beginNativePainting();
	if (!gla)
	{
		return;
	}
	gla->makeCurrent();

	float screenRatio = float(gla->width()) / float(gla->height());
	//set orthogonal view
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	float scale = /*gla->viewRatio()*/1.75;
	gluOrtho2D(-screenRatio * scale, screenRatio * scale, -scale, scale);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPushMatrix();
	glScalef(gla->trackball.track.sca, gla->trackball.track.sca, gla->trackball.track.sca);
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDepthMask(false);
	// IN FRONT OF SURFACE
	glDepthFunc(GL_LESS);

	/////////////////////////////////////////////////////////////////////////////////
	Color4b majorColor = lineColor;

	//majorColor[3] = 255;
	Color4b minorColor, halfMinorColor;

	halfMinorColor[0] = 26;
	halfMinorColor[1] = 252;
	halfMinorColor[2] = 255;
	halfMinorColor[3] = 180;

	minorColor = halfMinorColor;

	int numTickOnXAxis = maxX / step, numTickOnYAxis = maxY / step;
	float startX = -numTickOnXAxis * step;
	float endX = numTickOnXAxis * step;
	float startY = numTickOnYAxis * step;
	float endY = -numTickOnYAxis * step;

	Point3m p1(0.0f, 0.0f, 0.0f), p2(0.0f, 0.0f, 0.0f);

	// Draw the axis
	glColor(minorColor);
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	p1[1] = startY;
	p2[1] = endY;
	p1[0] = p2[0] = 0;
	glVertex(p1);
	glVertex(p2);
	p1[0] = startX;
	p2[0] = endX;
	p1[1] = p2[1] = 0;
	glVertex(p1);
	glVertex(p2);
	glEnd();

	// -->draw lines on X axis
	p1[1] = startY;
	p2[1] = endY;

	int divide = -step + 11;
	glLineWidth(0.5f);
	glBegin(GL_LINES);
	for (int i = -numTickOnXAxis; i <= numTickOnXAxis; ++i)
	{
		if (i % divide == 0 && i != 0)
		{
			glColor(halfMinorColor);
		}
		else
		{
			glColor(majorColor);
			//glColor4f(200 / 255.0f, 200 / 255.0f, 200 / 255.0f, 255 / 255.0f);
		}
		if (i == 0)
		{
			continue;
		}
		p1[0] = p2[0] = step * i;
		glVertex(p1); glVertex(p2);
	}
	// -->draw lines on Y axis
	p1[0] = startX;
	p2[0] = endX;
	for (int i = -numTickOnYAxis; i <= numTickOnYAxis; ++i)
	{
		if (i % divide == 0 && i != 0)
		{
			glColor(halfMinorColor);
		}
		else
		{
			glColor(majorColor);
		}
		if (i == 0)
		{
			continue;
		}
		p1[1] = p2[1] = step * i;
		glVertex(p1); glVertex(p2);
	}
	glEnd();

	// Draw scale marks
	int unitTextPosX = -28, unitTextPosY = -28;

	float startXOfScalePlate = step * unitTextPosX * gla->trackball.track.sca;

	while (startXOfScalePlate < -screenRatio * scale * 0.85)
	{
		unitTextPosX += 1;
		startXOfScalePlate = step * unitTextPosX * gla->trackball.track.sca;
	}

	float startYOfScalePlate = step * unitTextPosY * gla->trackball.track.sca;

	while (startYOfScalePlate < -1.2f)
	{
		unitTextPosY += 1;
		startYOfScalePlate = step * unitTextPosY * gla->trackball.track.sca;
	}

	float startPosOnX = step * unitTextPosX;
	float startPosOnY = step * unitTextPosY;

	glColor4f(1 / 255.f, 142 / 255.f, 237 / 255.f, 1.0f);

	glLineWidth(4.0f);
	glBegin(GL_LINE_STRIP);
	p1[0] = startPosOnX;
	p1[1] = startPosOnY + step / 3.0;
	p2[0] = startPosOnX;
	p2[1] = startPosOnY;
	glVertex(p1);
	glVertex(p2);
	p2[0] += step;
	glVertex(p2);
	p2[1] += step / 3.0f;
	glVertex(p2);
	glEnd();
	glLineWidth(2.0f);

	// Draw scale text
	startPosOnX = step * unitTextPosX;
	startPosOnY = step * unitTextPosY;
	p1[0] = startPosOnX;
	p1[1] = startPosOnY - step / 3.0;
	QString str = QString("%1").arg(step) + QString("mm");

	// some control size change as the screen scale changing...
	QDesktopWidget* desktopWidget = QApplication::desktop(); // Get available desktop size
	QRect deskRect = desktopWidget->availableGeometry();  // Get device screen size
	float scaleResolutionDpi = deskRect.width() / 1920.0f;

	//vcg::glLabel::Mode pen(vcg::Color4b::White);
	vcg::glLabel::Mode pen(vcg::Color4b(1, 142, 237, 255));

	pen.qFont.setFamily("Microsoft YaHei UI");
	pen.qFont.setPixelSize(20 * scaleResolutionDpi);
	pen.qFont.setStyleStrategy(QFont::PreferAntialias);

	int sizeFont = 300 * gla->trackball.track.sca;

	sizeFont = sizeFont < 20 ? 20 : sizeFont;
	sizeFont = sizeFont > 40 ? 40 : sizeFont;

	pen.qFont.setPixelSize(sizeFont);

	vcg::glLabel::render(painter, p1, str, pen);
	/////////////////////////////////////////////////////////////////////////////////

	glMatrixMode(GL_PROJECTION);
	glPopAttrib();
	glEnable(GL_LIGHTING);
	glPopMatrix();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	//restore view
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	//painter->endNativePainting();
	//painter->restore();
}

void DecorateFusionPlugin::drawMeasureResult(GLArea* gla, QPainter* painter)
{
	//painter->save();
	//painter->beginNativePainting();
	gla->makeCurrent();
	vcg::glLabel::Mode pen(vcg::Color4b::Black);
	pen.qFont.setFamily("Microsoft YaHei UI");
	pen.qFont.setStyleStrategy(QFont::PreferAntialias);
	int sizeFont = 800 * gla->trackball.track.sca;

	sizeFont = sizeFont < 30 ? 30 : sizeFont;
	sizeFont = sizeFont > 150 ? 150 : sizeFont;

	pen.qFont.setPixelSize(sizeFont);

	float radius_point = 0.25f;

	int iVertex = 0;
	glEnable(GL_LIGHTING);
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	for (auto v : pos_picked_for_measure_)
	{
		if (iVertex % 2 == 0)
		{
			glColor3f(0.0f, 1.0f, 0.0f);
		}
		else if (iVertex % 2 == 1)
		{
			glColor3f(0.0f, 0.0f, 1.0f);
		}
		else
		{
			glColor3f(1.0f, 0.0f, 0.0f);
		}
		++iVertex;
		vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(v, radius_point, 40, 40);
	}

	glPopAttrib();
	glDisable(GL_LIGHTING);

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_MULTISAMPLE);
	glDisable(GL_LIGHTING);
	// Enable anti-aliasing
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);  // Antialias the lines
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glLineWidth(2.0f);
	glBegin(GL_LINES);

	for (int i_ver = 0; i_ver < pos_picked_for_measure_.size(); ++i_ver)
	{
		bool is_even = i_ver % 2 == 0;

		if (is_even)
		{
			glColor3f(0.0f, 1.0f, 0.0f);
		}
		else
		{
			glColor3f(0.0f, 0.0f, 1.0f);
		}

		auto& v = pos_picked_for_measure_.at(i_ver);
		glVertex3f(v.X(), v.Y(), v.Z());

	}
	glEnd();
	glLineWidth(1.0f);
	glDisable(GL_BLEND);
	glDisable(GL_MULTISAMPLE);
	glEnable(GL_LIGHTING);

	for (int i_ver = 0; i_ver < pos_picked_for_measure_.size(); ++i_ver)
	{
		if (i_ver % 2 != 0)
		{
			auto vA = pos_picked_for_measure_.at(i_ver - 1);
			auto vB = pos_picked_for_measure_.at(i_ver);
	 		auto vCenterLine = (vA + vB) / 2.0f;
			float distance3D = (vA - vB).Norm();
	 		pen.qFont.setPixelSize(sizeFont / 4);
			vcg::glLabel::render(painter, vCenterLine, QString::number(distance3D, 'f', 2) + " mm", pen);
		}
	}

// 	if (pos_picked_for_measure_.size() >= 2)
// 	{
//
// 		glVertex3f(vA.X(), vA.Y(), vA.Z());
// 		glColor3f(0.0f, 0.0f, 1.0f);
// 		glVertex3f(vB.X(), vB.Y(), vB.Z());
//
// 		if (bNeedCalculateAngle)
// 		{
// 			glColor3f(1.0f, 0.0f, 0.0f);
// 			glVertex3f(vC.X(), vC.Y(), vC.Z());
//
// 			vector<Point3m> arc_vert_list;
// 			vector<Point3m> arc_vert_color_list;
// 			Point3m va = (vA - vB).Normalize() * 2.5f;
// 			Point3m vb = (vC - vB).Normalize() * 2.5f;
//
// 			for (int i = 0; i < 100; ++i)
// 			{
// 				float t = i / 100.0f;
// 				Point3m vc = (va * t + vb * (1 - t)).Normalize();
// 				Point3m vert = vB + vc * 2.5f;
// 				arc_vert_list.push_back(vert);
//
// 				if (0 <= t && t < 0.5f)
// 				{
// 					t = 1 - (t / 0.5f);
// 					Point3m va_color = Point3m(1, 0, 0);
// 					Point3m vb_color = Point3m(0, 0, 1);
// 					Point3m vc_color = va_color * t + vb_color * (1 - t);
// 					arc_vert_color_list.push_back(vc_color);
// 				}
// 				else
// 				{
// 					t = 1 - ((t - 0.5f) / 0.5f);
// 					Point3m va_color = Point3m(0, 0, 1);
// 					Point3m vb_color = Point3m(0, 1, 0);
// 					Point3m vc_color = va_color * t + vb_color * (1 - t);
// 					arc_vert_color_list.push_back(vc_color);
// 				}
// 			}
//
// 			for (int i = 0; i < arc_vert_list.size() - 1; ++i)
// 			{
// 				glLineWidth(2.0f);
// 				glBegin(GL_LINE_STRIP);
// 				glColor3f(arc_vert_color_list[i].X(), arc_vert_color_list[i].Y(), arc_vert_color_list[i].Z());
// 				glVertex3f(arc_vert_list[i].X(), arc_vert_list[i].Y(), arc_vert_list[i].Z());
// 				glColor3f(arc_vert_color_list[i + 1].X(), arc_vert_color_list[i + 1].Y(), arc_vert_color_list[i + 1].Z());
// 				glVertex3f(arc_vert_list[i + 1].X(), arc_vert_list[i + 1].Y(), arc_vert_list[i + 1].Z());
// 			}
// 		}
//
// 		glEnd();
// 		glLineWidth(1.0f);
// 		glDisable(GL_BLEND);
// 		glDisable(GL_MULTISAMPLE);
// 		glEnable(GL_LIGHTING);
//
//
// 		auto vA = pos_picked_for_measure_[0];
// 		auto vB = pos_picked_for_measure_[1];
// 		auto vCenterLine = (vA + vB) / 2.0f;
// 		float distance3D = (vA - vB).Norm();
// 		pen.qFont.setPixelSize(sizeFont / 4);
// 		bool bNeedCalculateAngle = pos_picked_for_measure_.size() == 3 ? true : false;
//
// 		vcg::glLabel::render(painter, vCenterLine, QString::number(distance3D, 'f', 2) + " mm", pen);
//
// 		Point3m vC;
//
// 		if (bNeedCalculateAngle)
// 		{
// 			vC = pos_picked_for_measure_[2];
// 			vCenterLine = (vC + vB) / 2.0f;
// 			float distance3D = (vC - vB).Norm();
// 			vcg::glLabel::render(
// 				painter, vCenterLine, QString::number(distance3D, 'f', 2) + " mm", pen);
//
// 			float angle = vcg::Angle(vA - vB, vC - vB) / 3.1415926 * 180.0f;
//
// 			Point3m va = (vA - vB).Normalize() * 2.5f;
// 			Point3m vb = (vC - vB).Normalize() * 2.5f;
// 			Point3m vc = (va * 0.55f + vb * (1 - 0.55f)).Normalize();
// 			Point3m pos = vB + vc * 3.0f;
//
// 			vcg::glLabel::render(
// 				painter, pos, QString::number(angle, 'f', 1) + QStringLiteral(" °"), pen);
// 		}
//
//
// 		glDisable(GL_DEPTH_TEST);
//
// 		glEnable(GL_MULTISAMPLE);
// 		glDisable(GL_LIGHTING);
// 		glEnable(GL_LINE_SMOOTH);
// 		glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);  // Antialias the lines
// 		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
// 		glLineWidth(2.0f);
// 		glBegin(GL_LINE_STRIP);
// 		glColor3f(0.0f, 1.0f, 0.0f);
// 		glVertex3f(vA.X(), vA.Y(), vA.Z());
// 		glColor3f(0.0f, 0.0f, 1.0f);
// 		glVertex3f(vB.X(), vB.Y(), vB.Z());
//
// 		if (bNeedCalculateAngle)
// 		{
// 			glColor3f(1.0f, 0.0f, 0.0f);
// 			glVertex3f(vC.X(), vC.Y(), vC.Z());
//
// 			vector<Point3m> arc_vert_list;
// 			vector<Point3m> arc_vert_color_list;
// 			Point3m va = (vA - vB).Normalize() * 2.5f;
// 			Point3m vb = (vC - vB).Normalize() * 2.5f;
//
// 			for (int i = 0; i < 100; ++i)
// 			{
// 				float t = i / 100.0f;
// 				Point3m vc = (va * t + vb * (1 - t)).Normalize();
// 				Point3m vert = vB + vc * 2.5f;
// 				arc_vert_list.push_back(vert);
//
// 				if (0 <= t && t < 0.5f)
// 				{
// 					t = 1- (t / 0.5f);
// 					Point3m va_color = Point3m(1, 0, 0);
// 					Point3m vb_color = Point3m(0, 0, 1);
// 					Point3m vc_color = va_color * t + vb_color * (1 - t);
// 					arc_vert_color_list.push_back(vc_color);
// 				}
// 				else
// 				{
// 					t = 1- ((t - 0.5f) / 0.5f);
// 					Point3m va_color = Point3m(0, 0, 1);
// 					Point3m vb_color = Point3m(0, 1, 0);
// 					Point3m vc_color = va_color * t + vb_color * (1 - t);
// 					arc_vert_color_list.push_back(vc_color);
// 				}
// 			}
//
// 			for (int i = 0; i < arc_vert_list.size() - 1; ++i)
// 			{
// 				glLineWidth(2.0f);
// 				glBegin(GL_LINE_STRIP);
// 				glColor3f(arc_vert_color_list[i].X(), arc_vert_color_list[i].Y(), arc_vert_color_list[i].Z());
// 				glVertex3f(arc_vert_list[i].X(), arc_vert_list[i].Y(), arc_vert_list[i].Z());
// 				glColor3f(arc_vert_color_list[i + 1].X(), arc_vert_color_list[i + 1].Y(), arc_vert_color_list[i + 1].Z());
// 				glVertex3f(arc_vert_list[i + 1].X(), arc_vert_list[i + 1].Y(), arc_vert_list[i + 1].Z());
// 			}
// 		}
//
// 		glEnd();
// 		glLineWidth(1.0f);
// 		glDisable(GL_BLEND);
// 		glDisable(GL_MULTISAMPLE);
// 		glEnable(GL_LIGHTING);
//
// 	}
	//painter->endNativePainting();
	//painter->restore();
}

void DecorateFusionPlugin::mousePressEvent(QAction* pAct, QMouseEvent* event, MeshModel&/*m*/, GLArea*)
{
	// EnamelReductManager removed - file deleted
	// EnamelReductManager removed - file deleted
	if (false) // enamel_recution_manager_ == nullptr || !p_enamel_act_->isChecked()
	{
		return;
	}
	if (event->button() == Qt::LeftButton && pAct == p_enamel_act_)
	{
		QPoint cursor = event->pos();
	}
}

void DecorateFusionPlugin::mouseReleaseEvent(QMouseEvent* event, MeshModel&/*m*/, GLArea*)
{
	if (event->button() == Qt::RightButton && !pos_picked_for_measure_.empty())
	{
		if (judgeActionIsWorkingStatus(DP_SHOW_MEASURE))
		{
			pos_picked_for_measure_.pop_back();
		}
	}
}

void DecorateFusionPlugin::mousePickedVertexReceivedSlot(float x, float y, float z)
{
	if (judgeActionIsWorkingStatus(DP_SHOW_MEASURE))
	{
		pos_picked_for_measure_.push_back(Point3m(x, y, z));
// 		if (pos_picked_for_measure_.size() >= 2)
// 		{
// 			//pos_picked_for_measure_.back() = Point3m(x, y, z);
// 			clearVector(pos_picked_for_measure_);
// 			pos_picked_for_measure_.push_back(Point3m(x, y, z));
// 		}
// 		else
// 		{
// 			pos_picked_for_measure_.push_back(Point3m(x, y, z));
// 		}
	}

}

bool DecorateFusionPlugin::judgeActionIsWorkingStatus(DecorateToolsEnum _tool)
{
	for (auto action : actionList)
	{
		if (ID(action) == _tool)
		{
			if (action->isChecked())
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	return false;
}

void DecorateFusionPlugin::drawToothFDILabel(QPainter* painter, MeshModel& md, GLArea* gla)
{
	gla->makeCurrent();
	//painter->save();
	//painter->beginNativePainting();
	//painter->resetMatrix();
	MeshModel* mesh = &md;
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	vcg::glLabel::Mode pen(Color4b(30, 30, 30, 255));
	pen.qFont.setFamily("Microsoft YaHei UI");
	pen.qFont.setStyleStrategy(QFont::PreferAntialias);

	int sizeFont = gla->trackball.track.sca * 500;
	sizeFont = sizeFont > 100 ? 100 : sizeFont;
	sizeFont = sizeFont < 6 ? 6 : sizeFont;
	pen.qFont.setPixelSize(sizeFont + 5);

	if (PFusionAlignData->upperManager() || PFusionAlignData->lowerManager())
	{
		if (PFusionAlignData->upperManager() && PFusionAlignData->upperManager()->bVisible_)
		{
			for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
			{
				if (PFusionAlignData->upperManager()->cDental.bToothExist[i])
				{
					if (!PFusionAlignData->upperManager()->cDental.teeth[i].bPulled_out_)
					{
						vcg::Matrix44f cur_matrix = PFusionAlignData->upperManager()->cDental.teeth[i].p_mesh_->Tr;
						Point3f normal = PFusionAlignData->upperManager()->cDental.teeth[i].realTimeAxis.axisZVector;
						Point3f local_show_pos = PFusionAlignData->upperManager()->cDental.teeth[i].localAxis.centerPoint;
						local_show_pos += PFusionAlignData->upperManager()->cDental.teeth[i].localAxis.axisZVector * ((PFusionAlignData->upperManager()->cDental.teeth[i].localBoundbox.fWidth / 2.0f) + 0.1f);
						Point3f show_pos = cur_matrix * local_show_pos;
						if (judgeFdiLabelIsInView(&mesh->cm,
							show_pos, normal, gla))
						{
							vcg::glLabel::render(painter, show_pos,
								PFusionAlignData->upperManager()->cDental.teeth[i].strFDI, pen);
						}
					}
				}
			}
		}
		if (PFusionAlignData->lowerManager() && PFusionAlignData->lowerManager()->bVisible_)
		{
			for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
			{
				if (PFusionAlignData->lowerManager()->cDental.bToothExist[i])
				{
					if (!PFusionAlignData->lowerManager()->cDental.teeth[i].bPulled_out_)
					{
						vcg::Matrix44f cur_matrix = PFusionAlignData->lowerManager()->cDental.teeth[i].p_mesh_->Tr;
						Point3f normal = PFusionAlignData->lowerManager()->cDental.teeth[i].realTimeAxis.axisZVector;
						Point3f local_show_pos = PFusionAlignData->lowerManager()->cDental.teeth[i].localAxis.centerPoint;
						local_show_pos += PFusionAlignData->lowerManager()->cDental.teeth[i].localAxis.axisZVector * ((PFusionAlignData->lowerManager()->cDental.teeth[i].localBoundbox.fWidth / 2.0f) + 0.1f);
						Point3f show_pos = cur_matrix * local_show_pos;
						if (judgeFdiLabelIsInView(&mesh->cm,
							show_pos, normal, gla))
						{
							vcg::glLabel::render(painter, show_pos,
								PFusionAlignData->lowerManager()->cDental.teeth[i].strFDI, pen);
						}
					}
				}
			}
		}
	}
	else if (PFusionAlignData->upper_dental_features_ || PFusionAlignData->lower_dental_features_)
	{
		Point3m view_direct, cam_pos;
		UtilityTools::getInstance()->getCurrentCameraState(nullptr, gla->width(), gla->height(), view_direct, cam_pos);
		if (PFusionAlignData->upper_dental_features_  && PFusionAlignData->upper_dental_features_->bVisible_)
		{
			bool always_visible = false;
			if (PFusionAlignData->upper_dental_features_->base_plane_ready_ &&
				PFusionAlignData->upper_dental_features_->base_plane_.axisZV * view_direct > 0.5f)
			{
				always_visible = true;
			}

			for (auto& tooth : PFusionAlignData->upper_dental_features_->teeth_list_)
			{
				Point3f normal = tooth.fdi_show_normal_;
				Point3f show_pos = tooth.fdi_show_pos_;
				if (always_visible || judgeFdiLabelIsInView(&mesh->cm,
					show_pos, normal, gla))
				{
					vcg::glLabel::render(painter, show_pos,
						QString::number(tooth.fdi_), pen);
				}
			}
		}

		if (PFusionAlignData->lower_dental_features_ && PFusionAlignData->lower_dental_features_->bVisible_)
		{
			bool always_visible = false;
			if (PFusionAlignData->lower_dental_features_->base_plane_ready_ &&
				PFusionAlignData->lower_dental_features_->base_plane_.axisZV * view_direct > 0.5f)
			{
				always_visible = true;
			}

			for (auto& tooth : PFusionAlignData->lower_dental_features_->teeth_list_)
			{
				Point3f normal = tooth.fdi_show_normal_;
				Point3f show_pos = tooth.fdi_show_pos_;
				if (always_visible || judgeFdiLabelIsInView(&mesh->cm,
					show_pos, normal, gla))
				{
					vcg::glLabel::render(painter, show_pos,
						QString::number(tooth.fdi_), pen);
				}
			}
		}
	}
	glPopAttrib();
	//painter->endNativePainting();
	//painter->restore();
	return;
}

bool DecorateFusionPlugin::prepareFbo(unsigned int& textureColorbuffer, unsigned int& framebuffer, unsigned int& rbo, int width, int height)
{
	// framebuffer configuration
	// -------------------------
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	// create a color attachment texture
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

void DecorateFusionPlugin::drawDentalOverlapArea(QPainter* p, GLArea* gla)
{

}

void DecorateFusionPlugin::drawDentalOverlapAreaPerTooth(QPainter* p, DentalManager* curMeshMode, DentalManager* meshMode_U, DentalManager* meshMode_L)
{
	Point3m vert1, vert2, vert3;

	// Get current tooth pair index
	if (tooth_cp_.empty())
	{
		vector<FEdge> intersectRecords;
		meshMode_U->cDental.computeIntersectedToothCPwithAnohterDental(intersectRecords, &meshMode_L->cDental);
		for (int i = 0; i < intersectRecords.size(); ++i)
		{
			tooth_cp_.push_back(Point2i(intersectRecords[i].indexA, intersectRecords[i].indexB));
		}
		intersectRecords.clear();
	}

	vector<FFace> faceList_U_V, faceList_U_NV, faceList_L_V, faceList_L_NV;
	for (auto cp : tooth_cp_)
	{
		collectTooth2DProjFacesIndexes(cp.X(), meshMode_U, camera_pos_, view_dir_, faceList_U_V, faceList_U_NV);
		collectTooth2DProjFacesIndexes(cp.Y(), meshMode_L, camera_pos_, view_dir_, faceList_L_V, faceList_L_NV);
	}

	////generate fbo
	GLenum error = glewInit();
	if (error != GLEW_OK)
		assert(0);
	unsigned int textureColorbuffer, framebuffer, rbo;
	prepareFbo(textureColorbuffer, framebuffer, rbo, width_, height_);

	//GL_TYPE_NM<Scalarm>::ScalarType *buffer_U_V, *buffer_U_NV, *buffer_L_V, *buffer_L_NV;
	//Upper Visible Faces
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glColor4f(0, 0, 0, 0);
	glBegin(GL_TRIANGLES);
	for (auto face : faceList_U_V)
	{
		glVertex3f(face.v1.X(), face.v1.Y(), face.v1.Z());
		glVertex3f(face.v2.X(), face.v2.Y(), face.v2.Z());
		glVertex3f(face.v3.X(), face.v3.Y(), face.v3.Z());
	}
	glEnd();
	glColor3f(1, 1, 1);
	glDisable(GL_BLEND);
	glPopMatrix();

	//buffer_U_V = new GL_TYPE_NM<Scalarm>::ScalarType[width_ * height_];
	memset(p_up_mesh_v_buffer_, 0, width_*height_);
	glReadPixels(0, 0, width_, height_, GL_DEPTH_COMPONENT, GL_TYPE_NM<Scalarm>::SCALAR(), p_up_mesh_v_buffer_);
	faceList_U_V.clear();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Upper UnVisible Faces
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glColor4f(0, 0, 0, 0);
	glBegin(GL_TRIANGLES);
	for (auto face : faceList_U_NV)
	{
		glVertex3f(face.v1.X(), face.v1.Y(), face.v1.Z());
		glVertex3f(face.v2.X(), face.v2.Y(), face.v2.Z());
		glVertex3f(face.v3.X(), face.v3.Y(), face.v3.Z());
	}
	glEnd();
	glColor3f(1, 1, 1);
	glDisable(GL_BLEND);
	glPopMatrix();

	//buffer_U_NV = new GL_TYPE_NM<Scalarm>::ScalarType[width_ * height_];
	memset(p_up_mesh_nv_buffer_, 0, width_*height_);
	glReadPixels(0, 0, width_, height_, GL_DEPTH_COMPONENT, GL_TYPE_NM<Scalarm>::SCALAR(), p_up_mesh_nv_buffer_);
	faceList_U_NV.clear();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Lower Visible Faces
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glColor4f(0, 0, 0, 0);
	glBegin(GL_TRIANGLES);
	for (auto face : faceList_L_V)
	{
		glVertex3f(face.v1.X(), face.v1.Y(), face.v1.Z());
		glVertex3f(face.v2.X(), face.v2.Y(), face.v2.Z());
		glVertex3f(face.v3.X(), face.v3.Y(), face.v3.Z());
	}
	glEnd();
	glColor3f(1, 1, 1);
	glDisable(GL_BLEND);
	glPopMatrix();

	//buffer_L_V = new GL_TYPE_NM<Scalarm>::ScalarType[width_ * height_];
	memset(p_down_mesh_v_buffer_, 0, width_*height_);
	glReadPixels(0, 0, width_, height_, GL_DEPTH_COMPONENT, GL_TYPE_NM<Scalarm>::SCALAR(), p_down_mesh_v_buffer_);
	faceList_L_V.clear();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Lower UnVisible Faces
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glColor4f(0, 0, 0, 0);
	glBegin(GL_TRIANGLES);
	for (auto face : faceList_L_NV)
	{
		glVertex3f(face.v1.X(), face.v1.Y(), face.v1.Z());
		glVertex3f(face.v2.X(), face.v2.Y(), face.v2.Z());
		glVertex3f(face.v3.X(), face.v3.Y(), face.v3.Z());
	}
	glEnd();
	glColor3f(1, 1, 1);
	glDisable(GL_BLEND);
	glPopMatrix();

	memset(p_down_mesh_nv_buffer_, 0, width_*height_);
	//buffer_L_NV = new GL_TYPE_NM<Scalarm>::ScalarType[width_ * height_];
	glReadPixels(0, 0, width_, height_, GL_DEPTH_COMPONENT, GL_TYPE_NM<Scalarm>::SCALAR(), p_down_mesh_nv_buffer_);
	faceList_L_NV.clear();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteTextures(1, &textureColorbuffer);
	glDeleteRenderbuffers(1, &rbo);

	float depth_U_V, depth_U_NV, depth_L_V, depth_L_NV;
	for (int x = 0; x < height_; ++x)
	{
		for (int y = 0; y < width_; ++y)
		{
			depth_U_V = p_up_mesh_v_buffer_[x * width_ + y];
			depth_U_NV = p_up_mesh_nv_buffer_[x * width_ + y];
			depth_L_V = p_down_mesh_v_buffer_[x * width_ + y];
			depth_L_NV = p_down_mesh_nv_buffer_[x * width_ + y];

			if (depth_U_V != 1 && depth_L_V != 1)//overlap Area
			{
				if (depth_U_V < depth_L_V)//Upper in front of Lower
				{
					if (depth_U_NV != 1 && depth_L_V != 1)
					{
						if (depth_U_NV >= depth_L_V)
						{
							Point3d vert1 = projectPoint(Point3m(y, x, depth_U_NV), curMeshMode);
							Point3d vert2 = projectPoint(Point3m(y, x, depth_L_V), curMeshMode);

							float diff = sqrtf((vert1.X() - vert2.X()) * (vert1.X() - vert2.X()) + (vert1.Y() - vert2.Y()) * (vert1.Y() - vert2.Y()) + (vert1.Z() - vert2.Z()) * (vert1.Z() - vert2.Z()));
							p_overlap_depth_buffer_[x * width_ + y] = diff;
						}
						else
						{
							p_overlap_depth_buffer_[x * width_ + y] = -1;
						}
					}
					else
					{
						p_overlap_depth_buffer_[x * width_ + y] = -1;
					}
				}
				else if (depth_U_V > depth_L_V)//Lower in front of Upper
				{
					if (depth_U_V != 1 && depth_L_NV != 1)
					{
						if (depth_U_V <= depth_L_NV)
						{
							Point3d vert1 = projectPoint(Point3m(y, x, depth_L_NV), curMeshMode);
							Point3d vert2 = projectPoint(Point3m(y, x, depth_U_V), curMeshMode);

							float diff = sqrtf((vert1.X() - vert2.X()) * (vert1.X() - vert2.X()) + (vert1.Y() - vert2.Y()) * (vert1.Y() - vert2.Y()) + (vert1.Z() - vert2.Z()) * (vert1.Z() - vert2.Z()));
							p_overlap_depth_buffer_[x * width_ + y] = diff;
						}
						else
						{
							p_overlap_depth_buffer_[x * width_ + y] = -1;
						}
					}
					else
					{
						p_overlap_depth_buffer_[x * width_ + y] = -1;
					}
				}
				else
				{
					p_overlap_depth_buffer_[x * width_ + y] = -1;
				}
			}
			else
			{
				p_overlap_depth_buffer_[x * width_ + y] = -1;
			}
		}
	}

	drawOverlapPixels(p_overlap_depth_buffer_);

	vector<FFace>().swap(faceList_U_V);
	vector<FFace>().swap(faceList_U_NV);
	vector<FFace>().swap(faceList_L_V);
	vector<FFace>().swap(faceList_L_NV);
	return;

}

void DecorateFusionPlugin::drawDentalOverlapAreaMode2(QPainter* p, GLArea* gla)
{
	Point3m upper_baseplane_normal;
	CMeshO* upper_mesh = nullptr, *lower_mesh = nullptr;
	if (PFusionAlignData->upper_dental_features_ != nullptr && PFusionAlignData->lower_dental_features_ != nullptr)
	{
		upper_mesh = &PFusionAlignData->upper_dental_features_->mesh_->original_mesh_->cm;
		lower_mesh = &PFusionAlignData->lower_dental_features_->mesh_->original_mesh_->cm;
		upper_baseplane_normal = PFusionAlignData->upper_dental_features_->base_plane_.axisZV;
	}
	else
	{
		for (auto& mesh_model : parent_->md()->meshList)
		{
			if (mesh_model->upperOrLowerToothModelMark == UpperToothModel)
			{
				upper_mesh = &mesh_model->cm;
			}
			else if (mesh_model->upperOrLowerToothModelMark == LowerToothModel)
			{
				lower_mesh = &mesh_model->cm;
			}
		}
		if (upper_mesh != nullptr && lower_mesh != nullptr)
		{
			vcg::tri::UpdateBounding<CMeshO>::Box(*upper_mesh);
			vcg::tri::UpdateBounding<CMeshO>::Box(*lower_mesh);
			upper_baseplane_normal = (upper_mesh->bbox.Center() - lower_mesh->bbox.Center()).Normalize();
		}
	}

	if(upper_mesh != nullptr && lower_mesh != nullptr)
	{
		Point3m vert1, vert2, vert3;
		this->updateCoverage(upper_mesh, lower_mesh);

		UtilityTools::getInstance()->getCurrentCameraState(nullptr, width_, height_, viewDirect, cameraPosition);
		this->view_dir_ = viewDirect;
		this->camera_pos_ = cameraPosition;

		Point3d projWorldCenter = UtilityTools::getInstance()->unProjectPoint(Point3m(0, 0, 0));
		float actualDistance = abs(-cameraPosition * viewDirect);
		double curDistanceT = actualDistance / projWorldCenter.Z();

		float viewPlaneAngle = this->viewDirect * upper_baseplane_normal;

		bool bUpperDentalVisible;
		if (viewPlaneAngle > 0)
		{
			bUpperDentalVisible = true;
		}
		else
		{
			bUpperDentalVisible = false;
		}

		vector<FFace> proj2DFacesUpper;
		collectTooth2DProjFaces(upper_mesh, bUpperDentalVisible, cameraPosition, viewDirect, proj2DFacesUpper);

		vector<FFace> proj2DFacesLower;
		collectTooth2DProjFaces(lower_mesh, !bUpperDentalVisible, cameraPosition, viewDirect, proj2DFacesLower);

		////generate fbo
		GLenum error = glewInit();
		if (error != GLEW_OK)
			assert(0);
		unsigned int textureColorbuffer, framebuffer, rbo;
		prepareFbo(textureColorbuffer, framebuffer, rbo, gla->width(), gla->height());
		//1.
		glBindFramebufferEXT(GL_FRAMEBUFFER, framebuffer);
		glClear(GL_DEPTH_BUFFER_BIT);
		glPushMatrix();
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glColor4f(0, 0, 0, 0);
		glBegin(GL_TRIANGLES);
		for (auto face : proj2DFacesUpper)
		{
			vert1 = face.v1;
			vert2 = face.v2;
			vert3 = face.v3;

			glVertex3f(vert1.X(), vert1.Y(), vert1.Z());
			glVertex3f(vert2.X(), vert2.Y(), vert2.Z());
			glVertex3f(vert3.X(), vert3.Y(), vert3.Z());
		}
		glEnd();
		glColor3f(1, 1, 1);
		glDisable(GL_BLEND);
		glPopMatrix();
		vector<FFace>().swap(proj2DFacesUpper);
		memset(buffer_U_V, 1, coverageWidth * coverageHeight);
		glReadPixels(coverageVert.X(), coverageVert.Y(), coverageWidth, coverageHeight, GL_DEPTH_COMPONENT, GL_TYPE_NM<Scalarm>::SCALAR(), buffer_U_V);
		glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

		//2.
		glBindFramebufferEXT(GL_FRAMEBUFFER, framebuffer);
		glClear(GL_DEPTH_BUFFER_BIT);
		glPushMatrix();
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glColor4f(0, 0, 0, 0);
		glBegin(GL_TRIANGLES);
		for (auto face : proj2DFacesLower)
		{
			vert1 = face.v1;
			vert2 = face.v2;
			vert3 = face.v3;

			glVertex3f(vert1.X(), vert1.Y(), vert1.Z());
			glVertex3f(vert2.X(), vert2.Y(), vert2.Z());
			glVertex3f(vert3.X(), vert3.Y(), vert3.Z());
		}
		glEnd();
		glColor3f(1, 1, 1);
		glDisable(GL_BLEND);
		glPopMatrix();
		vector<FFace>().swap(proj2DFacesLower);
		memset(buffer_L_V, 1, coverageWidth * coverageHeight);
		glReadPixels(coverageVert.X(), coverageVert.Y(), coverageWidth, coverageHeight, GL_DEPTH_COMPONENT, GL_TYPE_NM<Scalarm>::SCALAR(), buffer_L_V);
		glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffersEXT(1, &framebuffer);
		glDeleteTextures(1, &textureColorbuffer);
		glDeleteRenderbuffersEXT(1, &rbo);

		//3.
		memset(buffer_depth, -1, gla->width() * gla->height() * sizeof(GL_TYPE_NM<Scalarm>::ScalarType));
		//for (int x = 0; x < this->height(); ++x)
		int i = 0;
		for (int x = coverageVert.Y(); x < coverageVert.Y() + coverageHeight; ++x, ++i)
		{
			int j = 0;
			for (int y = coverageVert.X(); y < coverageVert.X() + coverageWidth; ++y, ++j)
			{
				float depth1 = buffer_U_V[i * coverageWidth + j];
				float depth2 = buffer_L_V[i * coverageWidth + j];

				if (depth1 != 1 && depth2 != 1)
				{
					if (bUpperDentalVisible && depth1 <= depth2)
					{
						float diff = depth2 - depth1;
						buffer_depth[x * gla->width() + y] = diff * curDistanceT;
					}
					else if (!bUpperDentalVisible && depth1 >= depth2)
					{
						float diff = depth1 - depth2;
						buffer_depth[x * gla->width() + y] = diff * curDistanceT;
					}
				}
			}
		}

		drawOverlapPixels(buffer_depth);
	}
	emit PSIGNALMANAGER->updateMeshVertexInfoBuffer_PP();
}

void DecorateFusionPlugin::drawOverlapPixels(GL_TYPE_NM<Scalarm>::ScalarType* buffer)
{
	if (buffer == nullptr)
	{
		return;
	}
	memset(p_overlap_pix_buffer_, 0, width_ * height_ * 4);
	glReadPixels(0, 0, width_, height_, GL_RGBA, GL_FLOAT, p_overlap_pix_buffer_);
	glClear(GL_DEPTH_BUFFER_BIT);
	for (int x = 0; x < height_; ++x)
	{
		for (int y = 0; y < width_; ++y)
		{
			float depth = buffer[x * width_ + y];
			if (depth >= 0)
			{
				float t;
				float actualDepthDiff = depth;
				int index = x * width_ + y;
				int indexOfRed = index * 4;

				if (actualDepthDiff >= 1.0f)
				{
					p_overlap_pix_buffer_[indexOfRed] = 1;
					p_overlap_pix_buffer_[indexOfRed + 1] = 0;
					p_overlap_pix_buffer_[indexOfRed + 2] = 0;
					p_overlap_pix_buffer_[indexOfRed + 3] = 1;
				}
				else
				{
					t = actualDepthDiff / 1.0f;
					p_overlap_pix_buffer_[indexOfRed] = t;
					p_overlap_pix_buffer_[indexOfRed + 1] = (1 - t);
					p_overlap_pix_buffer_[indexOfRed + 2] = 0;
					p_overlap_pix_buffer_[indexOfRed + 3] = 1;
				}
			}
		}
	}
	glDrawPixels(width_, height_, GL_RGBA, GL_FLOAT, p_overlap_pix_buffer_);
}

void DecorateFusionPlugin::collectTooth2DProjFaces(CMeshO* _mesh, bool getVisibleFace, Point3m cameraPos, Point3m viewDirect, vector<FFace>& faces)
{
	if (_mesh == nullptr)
	{
		return;
	}

	Point3m v1, v2, v3;
	vcg::Matrix44f trans_matrix = _mesh->Tr;
	//collect useful faces Indexes
	for (int index = 0; index < _mesh->face.size() /*- (curManager->cDental.teeth[iTooth].boundaryIndex.size() * 40 * 2)*/; ++index)
	{
		if (getVisibleFace && (_mesh->face[index].N()) * viewDirect < 0)
		{
			v1 = trans_matrix * _mesh->face[index].V(0)->P();
			v2 = trans_matrix * _mesh->face[index].V(1)->P();
			v3 = trans_matrix * _mesh->face[index].V(2)->P();

			faces.push_back(FFace(v1, v2, v3));
		}
		else if (!getVisibleFace && (_mesh->face[index].N()) * viewDirect > 0)
		{
			v1 = trans_matrix * _mesh->face[index].V(0)->P();
			v2 = trans_matrix * _mesh->face[index].V(1)->P();
			v3 = trans_matrix * _mesh->face[index].V(2)->P();

			faces.push_back(FFace(v1, v2, v3));
		}
	}
	return;
}

void DecorateFusionPlugin::collectTooth2DProjFaces(int iTooth, DentalManager *curManager, bool getVisibleFace, Point3m cameraPos, Point3m viewDirect, vector<FFace> &faces)
{
	if (iTooth < 0)
	{
		return;
	}

	Point3m v1, v2, v3;
	vcg::Matrix44f trans_matrix = curManager->cDental.teeth[iTooth].p_mesh_->Tr;
	//collect useful faces Indexes
	for (int index = 0; index < curManager->cDental.teeth[iTooth].p_mesh_->face.size() /*- (curManager->cDental.teeth[iTooth].boundaryIndex.size() * 40 * 2)*/; ++index)
	{
		if (getVisibleFace && (curManager->cDental.teeth[iTooth].p_mesh_->face[index].N()) * viewDirect < 0)
		{
			v1 = trans_matrix * curManager->cDental.teeth[iTooth].p_mesh_->face[index].V(0)->P();
			v2 = trans_matrix * curManager->cDental.teeth[iTooth].p_mesh_->face[index].V(1)->P();
			v3 = trans_matrix * curManager->cDental.teeth[iTooth].p_mesh_->face[index].V(2)->P();
			faces.push_back(FFace(v1, v2, v3));
		}
		else if (!getVisibleFace && (curManager->cDental.teeth[iTooth].p_mesh_->face[index].N()) * viewDirect > 0)
		{
			v1 = trans_matrix * curManager->cDental.teeth[iTooth].p_mesh_->face[index].V(0)->P();
			v2 = trans_matrix * curManager->cDental.teeth[iTooth].p_mesh_->face[index].V(1)->P();
			v3 = trans_matrix * curManager->cDental.teeth[iTooth].p_mesh_->face[index].V(2)->P();
			faces.push_back(FFace(v1, v2, v3));
		}
	}
	return;
}

void DecorateFusionPlugin::collectTooth2DProjFacesIndexes(int iTooth, DentalManager* curManager, Point3m cameraPos, Point3m viewDirect, vector<FFace>& faceList_V, vector<FFace>& faceList_NV)
{
	 	if (iTooth < 0)
	 	{
	 		return;
	 	}

		vcg::Matrix44f trans_matrix = curManager->cDental.teeth[iTooth].p_mesh_->Tr;
		Point3m v1, v2, v3;
	 	float cosValue;
	 	//collect useful faces Indexes
	 	for (int index = 0; index < curManager->cDental.teeth[iTooth].p_mesh_->face.size(); ++index)
	 	{
	 		cosValue = curManager->cDental.teeth[iTooth].p_mesh_->face[index].N() * viewDirect;
	 		if (cosValue < 0)
	 		{
				v1 = trans_matrix * curManager->cDental.teeth[iTooth].p_mesh_->face[index].V(0)->P();
				v2 = trans_matrix * curManager->cDental.teeth[iTooth].p_mesh_->face[index].V(1)->P();
				v3 = trans_matrix * curManager->cDental.teeth[iTooth].p_mesh_->face[index].V(2)->P();
				faceList_V.push_back(FFace(v1, v2, v3));
	 		}
	 		else if (cosValue > 0)
	 		{
				v1 = trans_matrix * curManager->cDental.teeth[iTooth].p_mesh_->face[index].V(0)->P();
				v2 = trans_matrix * curManager->cDental.teeth[iTooth].p_mesh_->face[index].V(1)->P();
				v3 = trans_matrix * curManager->cDental.teeth[iTooth].p_mesh_->face[index].V(2)->P();
				faceList_NV.push_back(FFace(v1, v2, v3));
	 		}
	 	}
	 	return;
}

vcg::Point3d DecorateFusionPlugin::projectPoint(Point3m p, DentalManager* curMeshModel)
{
	double mm[16], mp[16];
	GLint vp[4];
	glPushMatrix();
	glMultMatrix(curMeshModel->cDental.p_mesh_->Tr);

	glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	glGetDoublev(GL_PROJECTION_MATRIX, mp);
	glGetIntegerv(GL_VIEWPORT, vp);
	Point3d result(0, 0, 0);
	gluUnProject(p[0], p[1], p[2], mm, mp, vp, &result[0], &result[1], &result[2]);
	glPopMatrix();

	return result;
}

vcg::Point3d DecorateFusionPlugin::unProjectPoint(Point3m p, MeshModel* curMeshModel)
{
	// Get gl state values
	double mm[16], mp[16];
	GLint vp[4];
	glPushMatrix();
	glMultMatrix(curMeshModel->cm.Tr);

	glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	glGetDoublev(GL_PROJECTION_MATRIX, mp);
	glGetIntegerv(GL_VIEWPORT, vp);
	Point3d result(0, 0, 0);
	gluProject(p[0], p[1], p[2], mm, mp, vp, &result[0], &result[1], &result[2]);
	glPopMatrix();

	return result;
}

//void DecorateFusionPlugin::getCurrentCameraState(MeshModel* curMesh)
//{
//	Point3d tempProjPoint1 = projectPoint(Point3m(width_ / 2.0f, height_ / 2.0f, 0), curMesh);
//	Point3d tempProjPoint2 = projectPoint(Point3m(width_ / 2.0f, height_ / 2.0f, 1), curMesh);
//	Point3d dir = tempProjPoint2 - tempProjPoint1;
//	this->view_dir_ = Point3m(dir.X(), dir.Y(), dir.Z()).normalized();
//	this->camera_pos_ = Point3m(tempProjPoint1.X(), tempProjPoint1.Y(), tempProjPoint1.Z());
//}

QString DecorateFusionPlugin::decorationName(FilterIDType filter) const
{
	switch (filter)
	{
	case DP_SHOW_MEASUREGRID:
		return tr("Grid");
	case DP_SHOW_MEASURE:
		return tr("Meas");
	case DP_SHOW_TOOTHFDI:
		return tr("Num");
	case DP_SHOW_TOOTHOCCLUSION:
		return tr("Occlus");
	case DP_SHOW_TOOTHBOLTON:
		return tr("bolton analysis");
	default:
		break;
	}
}

QString DecorateFusionPlugin::decorationInfo(FilterIDType filter) const
{
	switch (filter)
	{
	case DP_SHOW_MEASUREGRID:
		return tr("Grid");
	case DP_SHOW_MEASURE:
		return tr("Meas.");
	case DP_SHOW_TOOTHFDI:
		return tr("Tooth No.");
	case DP_SHOW_TOOTHOCCLUSION:
		return tr("Occlus");
	case DP_SHOW_TOOTHBOLTON:
		return tr("show tooth bolton analysis");
	default:
		break;
	}
}

void DecorateFusionPlugin::showBoltonAnaResult(MeshDocument& md, GLArea* gla, QAction* act)
{
	if (p_bolton_ana_ == nullptr)
		p_bolton_ana_ = new BoltonAna(&md);
	else
		p_bolton_ana_->updateMeshDoc(&md);

	if (p_bolton_dlg_ == nullptr)
	{
		p_bolton_dlg_ = new ui_common::BoltonAnaDlg(p_bolton_ana_, gla->window());
		connect(p_bolton_dlg_, &ui_common::BoltonAnaDlg::closedSignal, act, &QAction::trigger);
	}
	else
		p_bolton_dlg_->updateUI(p_bolton_ana_);

	p_bolton_dlg_->show();
}

void DecorateFusionPlugin::updateCoverage(CMeshO* _meshU, CMeshO* _meshL)
{
	if (_meshU == nullptr || _meshL == nullptr)
	{
		coverageVert = Point2i(-1, -1);
		coverageHeight = -1;
		coverageWidth = -1;
		return;
	}

	Point2i vertMinU(-1, -1), vertMaxU(-1, -1), vertMinL(-1, -1), vertMaxL(-1, -1);
	int heightU = 0, widthU = 0, heightL = 0, widthL = 0;
	updateMeshModelCoverage(_meshU, vertMinU, vertMaxU);
	updateMeshModelCoverage(_meshL, vertMinL, vertMaxL);
	int minX, maxX, minY, maxY;
	minX = vertMinU.X() < vertMinL.X() ? vertMinU.X() : vertMinL.X();
	maxX = vertMaxU.X() > vertMaxL.X() ? vertMaxU.X() : vertMaxL.X();
	minY = vertMinU.Y() < vertMinL.Y() ? vertMinU.Y() : vertMinL.Y();
	maxY = vertMaxU.Y() > vertMaxL.Y() ? vertMaxU.Y() : vertMaxL.Y();

	this->coverageVert = Point2i(minX, minY);
	this->coverageHeight = maxY - minY;
	this->coverageWidth = maxX - minX;
	return;
}

void DecorateFusionPlugin::updateCoverage(DentalManager *modeU, DentalManager *modeL)
{
	if (modeU == nullptr || modeL == nullptr)
	{
		coverageVert = Point2i(-1, -1);
		coverageHeight = -1;
		coverageWidth = -1;
		return;
	}

	Point2i vertMinU(-1, -1), vertMaxU(-1, -1), vertMinL(-1, -1), vertMaxL(-1, -1);
	int heightU = 0, widthU = 0, heightL = 0, widthL = 0;
	updateMeshModelCoverage(modeU, vertMinU, vertMaxU);
	updateMeshModelCoverage(modeL, vertMinL, vertMaxL);
	int minX, maxX, minY, maxY;
	minX = vertMinU.X() < vertMinL.X() ? vertMinU.X() : vertMinL.X();
	maxX = vertMaxU.X() > vertMaxL.X() ? vertMaxU.X() : vertMaxL.X();
	minY = vertMinU.Y() < vertMinL.Y() ? vertMinU.Y() : vertMinL.Y();
	maxY = vertMaxU.Y() > vertMaxL.Y() ? vertMaxU.Y() : vertMaxL.Y();

	this->coverageVert = Point2i(minX, minY);
	this->coverageHeight = maxY - minY;
	this->coverageWidth = maxX - minX;
	return;
}

void DecorateFusionPlugin::updateMeshModelCoverage(CMeshO* _curMesh, Point2i& _vertMin, Point2i& _vertMax)
{
	if (_curMesh == nullptr)
	{
		return;
	}

	vector<Point2i>().swap(projBoxVerts);
	vector<Point3m> boxVerts;

	if (_curMesh->bbox.IsEmpty())
	{
		vcg::tri::UpdateBounding<CMeshO>::Box(*_curMesh);
	}
	vcg::Matrix44f trans_matrix = vcg::Matrix44f::Identity();
	boxVerts.push_back(trans_matrix * (_curMesh->bbox.min));
	boxVerts.push_back(trans_matrix * (_curMesh->bbox.min + Point3m(0, 1, 0) * _curMesh->bbox.DimY()));
	boxVerts.push_back(trans_matrix * (_curMesh->bbox.min + Point3m(0, 1, 0) * _curMesh->bbox.DimY() + Point3m(1, 0, 0) * _curMesh->bbox.DimX()));
	boxVerts.push_back(trans_matrix * (_curMesh->bbox.max));
	boxVerts.push_back(trans_matrix * (_curMesh->bbox.min + Point3m(0, 1, 0) * _curMesh->bbox.DimY() + Point3m(0, 0, 1) * _curMesh->bbox.DimZ()));
	boxVerts.push_back(trans_matrix * (_curMesh->bbox.min + Point3m(1, 0, 0) * _curMesh->bbox.DimX() + Point3m(0, 0, 1) * _curMesh->bbox.DimZ()));
	boxVerts.push_back(trans_matrix * (_curMesh->bbox.min + Point3m(1, 0, 0) * _curMesh->bbox.DimX()));
	boxVerts.push_back(trans_matrix * (_curMesh->bbox.min + Point3m(0, 0, 1) * _curMesh->bbox.DimZ()));

	for (int i = 0; i < boxVerts.size(); ++i)
	{
		Point3d projV = UtilityTools::getInstance()->unProjectPoint(boxVerts[i]);
		tailorProjcetion(projV);
		projBoxVerts.push_back(Point2i(projV.X(), projV.Y()));
	}

	int minX, maxX, minY, maxY;
	minX = projBoxVerts[0].X();
	minY = projBoxVerts[0].Y();
	maxX = projBoxVerts[0].X();
	maxY = projBoxVerts[0].Y();
	for (int i = 1; i < projBoxVerts.size(); ++i)
	{
		if (projBoxVerts[i].X() < minX)
		{
			minX = projBoxVerts[i].X();
		}
		if (projBoxVerts[i].Y() < minY)
		{
			minY = projBoxVerts[i].Y();
		}

		if (projBoxVerts[i].X() > maxX)
		{
			maxX = projBoxVerts[i].X();
		}
		if (projBoxVerts[i].Y() > maxY)
		{
			maxY = projBoxVerts[i].Y();
		}
	}

	_vertMin = Point2i(minX, minY);
	_vertMax = Point2i(maxX, maxY);
}

void DecorateFusionPlugin::updateMeshModelCoverage(DentalManager *curMode, Point2i &_vertMin, Point2i &_vertMax)
{
	if (curMode == nullptr)
	{
		return;
	}

	vector<Point2i>().swap(projBoxVerts);
	vector<Point3m> boxVerts;
	for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (!curMode->cDental.bToothExist[i])
		{
			continue;
		}
		if (curMode->cDental.teeth[i].p_mesh_->bbox.IsEmpty())
		{
			vcg::tri::UpdateBounding<CMeshO>::Box(*curMode->cDental.teeth[i].p_mesh_);
		}
		vcg::Matrix44f trans_matrix = curMode->cDental.teeth[i].transformMatrix_;
		boxVerts.push_back(trans_matrix * (curMode->cDental.teeth[i].p_mesh_->bbox.min));
		boxVerts.push_back(trans_matrix * (curMode->cDental.teeth[i].p_mesh_->bbox.min + Point3m(0, 1, 0) * curMode->cDental.teeth[i].p_mesh_->bbox.DimY()));
		boxVerts.push_back(trans_matrix * (curMode->cDental.teeth[i].p_mesh_->bbox.min + Point3m(0, 1, 0) * curMode->cDental.teeth[i].p_mesh_->bbox.DimY() + Point3m(1, 0, 0) * curMode->cDental.teeth[i].p_mesh_->bbox.DimX()));
		boxVerts.push_back(trans_matrix * (curMode->cDental.teeth[i].p_mesh_->bbox.max));
		boxVerts.push_back(trans_matrix * (curMode->cDental.teeth[i].p_mesh_->bbox.min + Point3m(0, 1, 0) * curMode->cDental.teeth[i].p_mesh_->bbox.DimY() + Point3m(0, 0, 1) * curMode->cDental.teeth[i].p_mesh_->bbox.DimZ()));
		boxVerts.push_back(trans_matrix * (curMode->cDental.teeth[i].p_mesh_->bbox.min + Point3m(1, 0, 0) * curMode->cDental.teeth[i].p_mesh_->bbox.DimX() + Point3m(0, 0, 1) * curMode->cDental.teeth[i].p_mesh_->bbox.DimZ()));
		boxVerts.push_back(trans_matrix * (curMode->cDental.teeth[i].p_mesh_->bbox.min + Point3m(1, 0, 0) * curMode->cDental.teeth[i].p_mesh_->bbox.DimX()));
		boxVerts.push_back(trans_matrix * (curMode->cDental.teeth[i].p_mesh_->bbox.min + Point3m(0, 0, 1) * curMode->cDental.teeth[i].p_mesh_->bbox.DimZ()));
	}
	for (int i = 0; i < boxVerts.size(); ++i)
	{
		Point3d projV = UtilityTools::getInstance()->unProjectPoint(boxVerts[i], curMode->cDental.p_model_);
		tailorProjcetion(projV);
		projBoxVerts.push_back(Point2i(projV.X(), projV.Y()));
	}

	int minX, maxX, minY, maxY;
	minX = projBoxVerts[0].X();
	minY = projBoxVerts[0].Y();
	maxX = projBoxVerts[0].X();
	maxY = projBoxVerts[0].Y();
	for (int i = 1; i < projBoxVerts.size(); ++i)
	{
		if (projBoxVerts[i].X() < minX)
		{
			minX = projBoxVerts[i].X();
		}
		if (projBoxVerts[i].Y() < minY)
		{
			minY = projBoxVerts[i].Y();
		}

		if (projBoxVerts[i].X() > maxX)
		{
			maxX = projBoxVerts[i].X();
		}
		if (projBoxVerts[i].Y() > maxY)
		{
			maxY = projBoxVerts[i].Y();
		}
	}

	_vertMin = Point2i(minX, minY);
	_vertMax = Point2i(maxX, maxY);
}

void DecorateFusionPlugin::tailorProjcetion(Point3d &v)
{
	if (v.X() < 0)
	{
		v.X() = 0;
	}
	if (v.X() > width_)
	{
		v.X() = width_;
	}
	if (v.Y() < 0)
	{
		v.Y() = 0;
	}
	if (v.Y() > height_)
	{
		v.Y() = height_;
	}
}

