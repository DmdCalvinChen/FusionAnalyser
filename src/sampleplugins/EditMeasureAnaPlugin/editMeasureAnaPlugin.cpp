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

#include "editMeasureAnaPlugin.h"

enum EditItemIndex : int ;

EditMeasureAnaPlugin::EditMeasureAnaPlugin(QObject* parent /*= 0*/)
{
    this->edit_item_index_ = EditItemIndex();
}

EditMeasureAnaPlugin::EditMeasureAnaPlugin(QAction* pAct /*= 0*/, QObject* parent /*= 0*/)
{
	p_act_ = pAct;
}

EditMeasureAnaPlugin::~EditMeasureAnaPlugin()
{
	if (PFusionAlignData->dental_analysis_)
	{
		if (PFusionAlignData->upperManager() || PFusionAlignData->lowerManager())
		{
            PFusionAlignData->dental_analysis_->switchEditItem(EditItemIndex(NONE));
		}
		if (PFusionAlignData->upper_dental_features_ || PFusionAlignData->lower_dental_features_)
		{
            PFusionAlignData->dental_analysis_->switchEditItemFeatureMode(EditItemIndex(NONE));
		}
	}
}

void EditMeasureAnaPlugin::suggestedRenderingData(MeshModel& m, MLRenderingData& dt)
{

}

bool EditMeasureAnaPlugin::StartEdit(MeshModel& m, GLArea* gla, MLSceneGLSharedDataContext* cont)
{

	gla->makeCurrent();
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		return false;
	}
	if (parent_ != gla)
	{
		parent_ = gla;
	}

	if (PFusionAlignData->dental_analysis_)
	{
		if (PFusionAlignData->upperManager() || PFusionAlignData->lowerManager())
		{
			PFusionAlignData->dental_analysis_->switchEditItem(this->edit_item_index_);
		}
		if (PFusionAlignData->upper_dental_features_ || PFusionAlignData->lower_dental_features_)
		{
			PFusionAlignData->dental_analysis_->switchEditItemFeatureMode(this->edit_item_index_);
		}

		PFusionAlignData->dental_analysis_->startEdit();
	}

	parent_->update();
	return true;
}

void EditMeasureAnaPlugin::EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	if (PFusionAlignData->dental_analysis_)
	{
		if (PFusionAlignData->upperManager() || PFusionAlignData->lowerManager())
		{
            PFusionAlignData->dental_analysis_->switchEditItem(EditItemIndex(NONE));
		}
		if (PFusionAlignData->upper_dental_features_ || PFusionAlignData->lower_dental_features_)
		{
            PFusionAlignData->dental_analysis_->switchEditItemFeatureMode(EditItemIndex(NONE));
		}
	}
}

void EditMeasureAnaPlugin::Decorate(MeshModel& m, GLArea* parent, QPainter* p)
{
	if (PFusionAlignData->dental_analysis_ != nullptr)
	{
		if (dental_analysis_press_)
		{
			if (PFusionAlignData->dental_analysis_->mousePress(mouseX_, mouseY_, mouse_btn_))
			{
				parent_->bWhetherOpenGlobalTrackball = false;
			}
			dental_analysis_press_ = false;
		}
		if (dental_analysis_move_)
		{
			if (PFusionAlignData->dental_analysis_->mouseMove(mouseX_, mouseY_, mouse_btn_))
			{
				parent_->bWhetherOpenGlobalTrackball = false;
			}
			dental_analysis_move_ = false;
		}
		if (dental_analysis_release_)
		{
			PFusionAlignData->dental_analysis_->mouseRelease(mouseX_, mouseY_);
			dental_analysis_release_ = false;
		}
	}

	if (PFusionAlignData->dental_analysis_ != nullptr)
	{
		PFusionAlignData->dental_analysis_->setScreenWidthAndHeight(parent->width(), parent->height());

		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_LIGHTING);
		PFusionAlignData->dental_analysis_->draw(p);
		glPopAttrib();
	}
	parent->update();
}

void EditMeasureAnaPlugin::keyPressEvent(QKeyEvent* event, MeshModel& m, GLArea* parent)
{
}

void EditMeasureAnaPlugin::mousePressEvent(QMouseEvent* event, MeshModel& m, GLArea* parent)
{
	if (PFusionAlignData->dental_analysis_ != nullptr && parent_ != nullptr)
	{
		mouseX_ = QTLogicalToDevice(parent_, event->pos().x());
		mouseY_ = QTLogicalToDevice(parent_, parent_->height() - event->pos().y());
		mouse_btn_ = event->button();
		dental_analysis_press_ = true;
	}
}

void EditMeasureAnaPlugin::mouseMoveEvent(QMouseEvent* event, MeshModel& m, GLArea* parent)
{
	if (PFusionAlignData->dental_analysis_ != nullptr && parent_ != nullptr)
	{
		mouseX_ = QTLogicalToDevice(parent_, event->pos().x());
		mouseY_ = QTLogicalToDevice(parent_, parent_->height() - event->pos().y());
		//mouse_btn_ = event->button();
		dental_analysis_move_ = true;
	}
}

void EditMeasureAnaPlugin::mouseReleaseEvent(QMouseEvent* event, MeshModel& m, GLArea* parent)
{
	if (PFusionAlignData->dental_analysis_ != nullptr && parent_ != nullptr)
	{
		mouseX_ = QTLogicalToDevice(parent_, event->pos().x());
		mouseY_ = QTLogicalToDevice(parent_, parent_->height() - event->pos().y());
		mouse_btn_ = event->button();
		dental_analysis_release_ = true;
		parent_->bWhetherOpenGlobalTrackball = true;
	}
}

void EditMeasureAnaPlugin::mouseDoubleClickEvent(QMouseEvent* event, MeshModel& m, GLArea* parent)
{

}

void EditMeasureAnaPlugin::updateUI(bool visible /*= true*/)
{

}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//

MissingToothAnaPlugin::MissingToothAnaPlugin(QObject* parent) : EditMeasureAnaPlugin(parent)
{
    this->edit_item_index_ = EditItemIndex(MISSING_TOOTH);
}

MissingToothAnaPlugin::~MissingToothAnaPlugin()
{

}

const QString MissingToothAnaPlugin::Info()
{
	return tr("missing tooth analysis plugin");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//

TeethWidthAnaPlugin::TeethWidthAnaPlugin(QObject* parent /*= 0*/) : EditMeasureAnaPlugin(parent)
{
    this->edit_item_index_ = EditItemIndex(TEETH_WIDTH);
	connect(PSIGNALMANAGER, &SignalManager::showTeethWidthSignal, this, &TeethWidthAnaPlugin::showTeethWidthGifSlot);
}

TeethWidthAnaPlugin::TeethWidthAnaPlugin(QAction* pAct /*= nullptr*/, QObject* parent /*= 0*/)
	: EditMeasureAnaPlugin(pAct, parent)
{
    this->edit_item_index_ = EditItemIndex(TEETH_WIDTH);
	connect(PSIGNALMANAGER, &SignalManager::showTeethWidthSignal, this, &TeethWidthAnaPlugin::showTeethWidthGifSlot);
}

bool TeethWidthAnaPlugin::StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	GLA = parent;
	if (p_act_)
		p_act_->setIcon(QIcon(":/res/images/icon/toothwidth_pressed.svg"));
	if (TeethWidthtipBtn == nullptr)
	{
		TeethWidthtipBtn = new tipsWidget(parent, parent);
		TeethWidthtipBtn->open_gif_type = TeethWidthGif;
		TeethWidthtipBtn->show();
	}
	else
	{
		TeethWidthtipBtn->show();
	}
	QSettings settings(QCoreApplication::applicationDirPath() + "./configs/config.ini", QSettings::IniFormat);
	if (gif_TeethWidth_ui == nullptr)
	{
		bool showBtn = true;
		if (!settings.value(TeethWidthGif).isValid())
			settings.value(TeethWidthGif) = 0;
		if (settings.value(TeethWidthGif) == 1)
			showBtn = false;
		QMainWindow* pMainWindow = UtilityTools::getInstance()->getSubMainWindow(parent->window());
		gif_TeethWidth_ui = new Gifviewgui(showBtn, parent);
		gif_TeethWidth_ui->gifType = TeethWidthGif;
		QList<std::pair<QString, QString>> imagepairlst;
		if (PFusionAppData->getAppLanguage() == E_CHINESE)
		{
			imagepairlst = { std::make_pair(":/res/gifs/TeethWidthLeft.gif", ""),
				std::make_pair(":/res/gifs/TeethWidthRight.gif", "")
			};
		}
		else if (PFusionAppData->getAppLanguage() == E_ENGLISH)
		{
			imagepairlst = { std::make_pair(":/res/gifs/TeethWidthLeft_EN.gif", ""),
				std::make_pair(":/res/gifs/TeethWidthRight_EN.gif", "")
			};
		}
		gif_TeethWidth_ui->showGif(imagepairlst);
		gif_TeethWidth_ui->setGeometry(parent->window()->width() - 400, 60,
			gif_TeethWidth_ui->width(), gif_TeethWidth_ui->height());
	}
	if (settings.value(TeethWidthGif) != 1)
	{
		gif_TeethWidth_ui->show();
	}
	return EditMeasureAnaPlugin::StartEdit(m, parent, cont);
}

void TeethWidthAnaPlugin::showTeethWidthGifSlot()
{
	if (gif_TeethWidth_ui && gif_TeethWidth_ui->isVisible())
	{
		gif_TeethWidth_ui->close();
	}
	else if (gif_TeethWidth_ui && !gif_TeethWidth_ui->isVisible())
	{
		gif_TeethWidth_ui->show();
	}
	qDebug() << gif_TeethWidth_ui->isVisible();
}

void TeethWidthAnaPlugin::updateUI(bool visible /* = true */)
{
	QSettings settings(QCoreApplication::applicationDirPath() + "./configs/config.ini", QSettings::IniFormat);
	if (gif_TeethWidth_ui)
	{
		if (settings.value(TeethWidthGif) == 1)
		{
			gif_TeethWidth_ui->showButton(false);
		}
		gif_TeethWidth_ui->setGeometry(GLA->window()->width() - 400, 60,
			gif_TeethWidth_ui->width(), gif_TeethWidth_ui->height());
//		gif_TeethWidth_ui->setVisible(visible);
	}
	if (TeethWidthtipBtn)
	{
		QPoint pointOri = TeethWidthtipBtn->p_gla_->mapToGlobal(QPoint(0, 0));
		TeethWidthtipBtn->setGeometry(pointOri.x() + TeethWidthtipBtn->p_gla_->width() - TeethWidthtipBtn->width() / 1.2,
			pointOri.y(),
			TeethWidthtipBtn->width(), TeethWidthtipBtn->height());
		TeethWidthtipBtn->setVisible(visible);
	}
}
void TeethWidthAnaPlugin::EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	QPixmap enabled_icon(":/res/images/icon/toolbarSVG/bollton_ana_normal.svg");
	QPixmap disabled_icon(":/res/images/icon/toolbarSVG/bollton_ana_disabled.svg");
	QIcon icon(enabled_icon);
	icon.addPixmap(disabled_icon, QIcon::Disabled);
	if (p_act_)
		p_act_->setIcon(icon);
	if (TeethWidthtipBtn)
	{
		//TeethWidthtipBtn->close();
		TeethWidthtipBtn->hide();
	}
	if (gif_TeethWidth_ui)
	{
		//gif_TeethWidth_ui->close();
		gif_TeethWidth_ui->hide();
	}
	return EditMeasureAnaPlugin::EndEdit(m, parent, cont);
}

TeethWidthAnaPlugin::~TeethWidthAnaPlugin()
{

}

const QString TeethWidthAnaPlugin::Info()
{
	return tr("Teeth width analysis plugin");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//

CrowdingDegreeAnaPlugin::CrowdingDegreeAnaPlugin(QObject* parent /*= nullptr*/) : EditMeasureAnaPlugin(parent)
{
    this->edit_item_index_ = EditItemIndex(CUR_LENGTH_OF_DENTAL_ARCH);
}

CrowdingDegreeAnaPlugin::CrowdingDegreeAnaPlugin(QAction* pAct /*= nullptr*/, QObject* parent /*= 0*/)
	: EditMeasureAnaPlugin(pAct, parent)
{
    this->edit_item_index_ = EditItemIndex(CUR_LENGTH_OF_DENTAL_ARCH);
}

bool CrowdingDegreeAnaPlugin::StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	if (p_act_)
		p_act_->setIcon(QIcon(":/res/images/icon/crowding_pressed.svg"));
	return EditMeasureAnaPlugin::StartEdit(m, parent, cont);
}

void CrowdingDegreeAnaPlugin::EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	QPixmap enabled_icon(":/res/images/icon/toolbarSVG/crowding_normal.svg");
	QPixmap disabled_icon(":/res/images/icon/toolbarSVG/crowding_disabled.svg");
	QIcon icon(enabled_icon);
	icon.addPixmap(disabled_icon, QIcon::Disabled);
	if (p_act_)
		p_act_->setIcon(icon);
	return EditMeasureAnaPlugin::EndEdit(m, parent, cont);
}
CrowdingDegreeAnaPlugin::~CrowdingDegreeAnaPlugin()
{

}

const QString CrowdingDegreeAnaPlugin::Info()
{
	return tr("crowding degree analysis plugin");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//

BoltonAnaPlugin::BoltonAnaPlugin(QObject* parent /*= nullptr*/) : EditMeasureAnaPlugin(parent)
{
    this->edit_item_index_ = EditItemIndex(BOLTON_COMPLETELY);
}

BoltonAnaPlugin::BoltonAnaPlugin(QAction* pAct /*= nullptr*/, QObject* parent /*= 0*/)
	: EditMeasureAnaPlugin(pAct, parent)
{
    this->edit_item_index_ = EditItemIndex(BOLTON_COMPLETELY);
}

bool BoltonAnaPlugin::StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	if (p_act_)
		p_act_->setIcon(QIcon(":/res/images/icon/bolton_pressed.svg"));
	return EditMeasureAnaPlugin::StartEdit(m, parent, cont);
}

void BoltonAnaPlugin::EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	QPixmap enabled_icon(":/res/images/icon/toolbarSVG/Bolton_normal.svg");
	QPixmap disabled_icon(":/res/images/icon/toolbarSVG/Bolton_disabled.svg");
	QIcon icon(enabled_icon);
	icon.addPixmap(disabled_icon, QIcon::Disabled);
	if (p_act_)
		p_act_->setIcon(icon);
	return EditMeasureAnaPlugin::EndEdit(m, parent, cont);
}
BoltonAnaPlugin::~BoltonAnaPlugin()
{

}

const QString BoltonAnaPlugin::Info()
{
	return tr("Bolton analysis plugin");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//

SpeeCurveDepthAnaPlugin::SpeeCurveDepthAnaPlugin(QObject* parent /*= nullptr*/) : EditMeasureAnaPlugin(parent)
{
    this->edit_item_index_ = EditItemIndex(SPEE_CURVE_DEPTH);
}

SpeeCurveDepthAnaPlugin::SpeeCurveDepthAnaPlugin(QAction* pAct /*= nullptr*/, QObject* parent /*= 0*/)
	: EditMeasureAnaPlugin(pAct, parent)
{
    this->edit_item_index_ = EditItemIndex(SPEE_CURVE_DEPTH);
}

bool SpeeCurveDepthAnaPlugin::StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	if (p_act_)
		p_act_->setIcon(QIcon(":/res/images/icon/spee_pressed.svg"));
	return EditMeasureAnaPlugin::StartEdit(m, parent, cont);
}

void SpeeCurveDepthAnaPlugin::EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	QPixmap enabled_icon(":/res/images/icon/toolbarSVG/spee_normal.svg");
	QPixmap disabled_icon(":/res/images/icon/toolbarSVG/spee_disabled.svg");
	QIcon icon(enabled_icon);
	icon.addPixmap(disabled_icon, QIcon::Disabled);
	if (p_act_)
		p_act_->setIcon(icon);
	return EditMeasureAnaPlugin::EndEdit(m, parent, cont);
}
SpeeCurveDepthAnaPlugin::~SpeeCurveDepthAnaPlugin()
{

}

const QString SpeeCurveDepthAnaPlugin::Info()
{
	return tr("spee curve analysis plugin");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//

MolarRelationshipAnaPlugin::MolarRelationshipAnaPlugin(QObject* parent /*= nullptr*/) : EditMeasureAnaPlugin(parent)
{
    this->edit_item_index_ = EditItemIndex(MOLAR_RELATIONSHIP);
}

MolarRelationshipAnaPlugin::MolarRelationshipAnaPlugin(QAction* pAct /*= nullptr*/, QObject* parent /*= 0*/)
	: EditMeasureAnaPlugin(pAct, parent)
{
    this->edit_item_index_ = EditItemIndex(MOLAR_RELATIONSHIP);
}

bool MolarRelationshipAnaPlugin::StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	if (p_act_)
		p_act_->setIcon(QIcon(":/res/images/icon/molar_pressed.svg"));
	return EditMeasureAnaPlugin::StartEdit(m, parent, cont);
}

void MolarRelationshipAnaPlugin::EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	QPixmap enabled_icon(":/res/images/icon/toolbarSVG/molar_normal.svg");
	QPixmap disabled_icon(":/res/images/icon/toolbarSVG/molar_disabled.svg");
	QIcon icon(enabled_icon);
	icon.addPixmap(disabled_icon, QIcon::Disabled);
	if (p_act_)
		p_act_->setIcon(icon);
	return EditMeasureAnaPlugin::EndEdit(m, parent, cont);
}
MolarRelationshipAnaPlugin::~MolarRelationshipAnaPlugin()
{

}

const QString MolarRelationshipAnaPlugin::Info()
{
	return tr("molar relation analysis plugin");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//

MidlineRelationshipAnaPlugin::MidlineRelationshipAnaPlugin(QObject* parent /*= nullptr*/) : EditMeasureAnaPlugin(parent)
{
    this->edit_item_index_ = EditItemIndex(MIDLINE_RELATIONSHIP);
}

MidlineRelationshipAnaPlugin::MidlineRelationshipAnaPlugin(QAction* pAct /*= nullptr*/, QObject* parent /*= 0*/)
	: EditMeasureAnaPlugin(pAct, parent)
{
    this->edit_item_index_ = EditItemIndex(MIDLINE_RELATIONSHIP);
}

bool MidlineRelationshipAnaPlugin::StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	if (p_act_)
		p_act_->setIcon(QIcon(":/res/images/icon/midline_pressed.svg"));
	return EditMeasureAnaPlugin::StartEdit(m, parent, cont);
}

void MidlineRelationshipAnaPlugin::EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	QPixmap enabled_icon(":/res/images/icon/toolbarSVG/midline_normal.svg");
	QPixmap disabled_icon(":/res/images/icon/toolbarSVG/midline_disabled.svg");
	QIcon icon(enabled_icon);
	icon.addPixmap(disabled_icon, QIcon::Disabled);
	if (p_act_)
		p_act_->setIcon(icon);
	return EditMeasureAnaPlugin::EndEdit(m, parent, cont);
}
MidlineRelationshipAnaPlugin::~MidlineRelationshipAnaPlugin()
{

}

const QString MidlineRelationshipAnaPlugin::Info()
{
	return tr("midline relation analysis plugin");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//

ArchWidthAnaPlugin::ArchWidthAnaPlugin(QObject* parent /*= nullptr*/) : EditMeasureAnaPlugin(parent)
{
    this->edit_item_index_ = EditItemIndex(ARCH_WIDTH);
}

ArchWidthAnaPlugin::ArchWidthAnaPlugin(QAction* pAct /*= nullptr*/, QObject* parent /*= 0*/)
	: EditMeasureAnaPlugin(pAct, parent)
{
    this->edit_item_index_ = EditItemIndex(ARCH_WIDTH);
}

bool ArchWidthAnaPlugin::StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	if (p_act_)
		p_act_->setIcon(QIcon(":/res/images/icon/archwide_pressed.svg"));
	return EditMeasureAnaPlugin::StartEdit(m, parent, cont);
}

void ArchWidthAnaPlugin::EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	QPixmap enabled_icon(":/res/images/icon/toolbarSVG/archwide_normal.svg");
	QPixmap disabled_icon(":/res/images/icon/toolbarSVG/archwide_disabled.svg");
	QIcon icon(enabled_icon);
	icon.addPixmap(disabled_icon, QIcon::Disabled);
	if (p_act_)
		p_act_->setIcon(icon);
	return EditMeasureAnaPlugin::EndEdit(m, parent, cont);
}
ArchWidthAnaPlugin::~ArchWidthAnaPlugin()
{

}

const QString ArchWidthAnaPlugin::Info()
{
	return tr("the width of dental arch");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//

ArchLengthAnaPlugin::ArchLengthAnaPlugin(QObject* parent /*= nullptr*/) : EditMeasureAnaPlugin(parent)
{
    this->edit_item_index_ = EditItemIndex(ARCH_LENGTH);
}

ArchLengthAnaPlugin::ArchLengthAnaPlugin(QAction* pAct /*= nullptr*/, QObject* parent /*= 0*/)
	: EditMeasureAnaPlugin(pAct, parent)
{
    this->edit_item_index_ = EditItemIndex(ARCH_LENGTH);
}

bool ArchLengthAnaPlugin::StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	if (p_act_)
		p_act_->setIcon(QIcon(":/res/images/icon/archlength_pressed.svg"));
	return EditMeasureAnaPlugin::StartEdit(m, parent, cont);
}

void ArchLengthAnaPlugin::EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	QPixmap enabled_icon(":/res/images/icon/toolbarSVG/archlength_normal.svg");
	QPixmap disabled_icon(":/res/images/icon/toolbarSVG/archlength_disabled.svg");
	QIcon icon(enabled_icon);
	icon.addPixmap(disabled_icon, QIcon::Disabled);
	if (p_act_)
		p_act_->setIcon(icon);
	return EditMeasureAnaPlugin::EndEdit(m, parent, cont);
}
ArchLengthAnaPlugin::~ArchLengthAnaPlugin()
{

}

const QString ArchLengthAnaPlugin::Info()
{
	return tr("the length of dental arch");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//

GnathotectumHeightAnaPlugin::GnathotectumHeightAnaPlugin(QObject* parent /*= nullptr*/) : EditMeasureAnaPlugin(parent)
{
    this->edit_item_index_ = EditItemIndex(GNATHOTECTUM_HEIGHT);
}

GnathotectumHeightAnaPlugin::GnathotectumHeightAnaPlugin(QAction* pAct /*= nullptr*/, QObject* parent /*= 0*/)
	: EditMeasureAnaPlugin(pAct, parent)
{
    this->edit_item_index_ = EditItemIndex(GNATHOTECTUM_HEIGHT);
}

bool GnathotectumHeightAnaPlugin::StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	if (p_act_)
		p_act_->setIcon(QIcon(":/res/images/icon/palateheight_pressed.svg"));
	return EditMeasureAnaPlugin::StartEdit(m, parent, cont);
}

void GnathotectumHeightAnaPlugin::EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	QPixmap enabled_icon(":/res/images/icon/toolbarSVG/palateheight_normal.svg");
	QPixmap disabled_icon(":/res/images/icon/toolbarSVG/palateheight_disabled.svg");
	QIcon icon(enabled_icon);
	icon.addPixmap(disabled_icon, QIcon::Disabled);
	if (p_act_)
		p_act_->setIcon(icon);
	return EditMeasureAnaPlugin::EndEdit(m, parent, cont);
}
GnathotectumHeightAnaPlugin::~GnathotectumHeightAnaPlugin()
{

}

const QString GnathotectumHeightAnaPlugin::Info()
{
	return tr("Gnathotect height analysis plugin");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//

BasalBoneArchAnaPlugin ::BasalBoneArchAnaPlugin (QObject* parent /*= nullptr*/) : EditMeasureAnaPlugin(parent)
{
    this->edit_item_index_ = EditItemIndex(BASAL_BONE_ARCH);
}

BasalBoneArchAnaPlugin ::BasalBoneArchAnaPlugin (QAction* pAct /*= nullptr*/, QObject* parent /*= 0*/)
	: EditMeasureAnaPlugin(pAct, parent)
{
    this->edit_item_index_ = EditItemIndex(BASAL_BONE_ARCH);
}

bool BasalBoneArchAnaPlugin ::StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	if (p_act_)
		p_act_->setIcon(QIcon(":/res/images/icon/basalbonearch_pressed.svg"));
	return EditMeasureAnaPlugin::StartEdit(m, parent, cont);
}

void BasalBoneArchAnaPlugin ::EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	QPixmap enabled_icon(":/res/images/icon/toolbarSVG/basalbonearch_normal.svg");
	QPixmap disabled_icon(":/res/images/icon/toolbarSVG/basalbonearch_disabled.svg");
	QIcon icon(enabled_icon);
	icon.addPixmap(disabled_icon, QIcon::Disabled);
	if (p_act_)
		p_act_->setIcon(icon);
	return EditMeasureAnaPlugin::EndEdit(m, parent, cont);
}
BasalBoneArchAnaPlugin ::~BasalBoneArchAnaPlugin ()
{

}

const QString BasalBoneArchAnaPlugin ::Info()
{
	return tr("Basal arch measurement");
}
