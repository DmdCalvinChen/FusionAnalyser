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

#include <FusionAnalyser/glarea.h>
#include "editModelMarkPlugin.h"
#include "ui/FeatureMarkGui/featuremarkgui.h"
#include <UI_Common/toothfdiguidegui/toothfdiguidegui.h>
#include <common_ext/commandmode/commandcommon.h>
#include <QMainWindow>
#include <QSettings>
#include <logsingleton.h>
#define COLOR_NUM 16

static Point3i color_feature[COLOR_NUM] = {
Point3i(160, 82, 45),
Point3i(36,66,138),
Point3i(208, 17, 43),
Point3i(40, 220, 120),
Point3i(255, 255, 0),
Point3i(0, 255, 255),
Point3i(255, 0, 255),
Point3i(255, 128, 0),
Point3i(128, 0, 255),
Point3i(128, 0, 0),
Point3i(178, 178, 0),
Point3i(97, 128, 255),
Point3i(197, 68, 215),
Point3i(216, 173, 87),
Point3i(7, 172, 28),
Point3i(255, 0, 0) };

void ToothWidthTool::draw(QPainter* _painter, Point3m _camera_view_direct, MeshModel* _p_model, CloudOctree* _p_tree, bool _adjusting)
{
	if (node_a_ == nullptr || node_b_ == nullptr)
	{
		return;
	}
	if (need_update_cut_outline_ || _adjusting)
	{
		this->cut_face_a_.cutMeshModel(_p_model, _p_tree);
		this->cut_face_b_.cutMeshModel(_p_model, _p_tree);
		need_update_cut_outline_ = false;
	}
	this->cut_face_a_.drawCutFace();
	this->cut_face_b_.drawCutFace();
}

void ToothWidthTool::drawWidthLineSeg(QPainter* _painter, Point3m _camera_view_direct)
{
	if (node_a_ == nullptr || node_b_ == nullptr)
	{
		return;
	}

	updateVisible(_camera_view_direct);
	if (!visible_)
	{
		return;
	}

	Point4m node_a_color, node_b_color;
	if (this->node_a_hovering_)
	{
		node_a_color = Point4m(129 / 255.f, 156 / 255.f, 222 / 255.f, 0.9f);
	}
	else
	{
		node_a_color = Point4m(color_feature[1].X() / 255.f, color_feature[1].Y() / 255.f, color_feature[1].Z() / 255.f, 0.9f);
	}
	if (this->node_b_hovering_)
	{
		node_b_color = Point4m( 243/ 255.f, 118/ 255.f, 134/ 255.f, 0.9f);
	}
	else
	{
		node_b_color = Point4m(color_feature[2].X() / 255.f, color_feature[2].Y() / 255.f, color_feature[2].Z() / 255.f, 0.9f);
	}

	Point3m vec_ab = (width_edge_.vertB - width_edge_.vertA).Normalize();
	Point3m axis_y = (vec_ab ^ _camera_view_direct).Normalize();

	float arrow_len = 0.5f;
	Point3m arrow_a_node0, arrow_a_node1;
	arrow_a_node0 = width_edge_.vertA + (vec_ab * cos(PI / 6.0f) + axis_y * sin(PI / 6.0f)).Normalize() * arrow_len;
	arrow_a_node1 = width_edge_.vertA + (vec_ab * cos(PI / 6.0f) - axis_y * sin(PI / 6.0f)).Normalize() * arrow_len;
	Point3m arrow_b_node0, arrow_b_node1;
	arrow_b_node0 = width_edge_.vertB + (-vec_ab * cos(PI / 6.0f) + axis_y * sin(PI / 6.0f)).Normalize() * arrow_len;
	arrow_b_node1 = width_edge_.vertB + (-vec_ab * cos(PI / 6.0f) - axis_y * sin(PI / 6.0f)).Normalize() * arrow_len;

	Point3m drawable_node_a = width_edge_.vertA + vec_ab * arrow_len * cos(PI / 6.0f);
	Point3m drawable_node_b = width_edge_.vertB - vec_ab * arrow_len * cos(PI / 6.0f);

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glLineWidth(5.0f);
	glBegin(GL_LINES);
	glColor4f(node_a_color.X(), node_a_color.Y(), node_a_color.Z(), node_a_color.W());
	glVertex3f(drawable_node_a.X(), drawable_node_a.Y(), drawable_node_a.Z());
	glColor4f(node_b_color.X(), node_b_color.Y(), node_b_color.Z(), node_b_color.W());
	glVertex3f(drawable_node_b.X(), drawable_node_b.Y(), drawable_node_b.Z());
	glEnd();
	/*glColor4f(0, 0, 0, 0.9f);
	glBegin(GL_LINES);
	glVertex3f(middle.X(), middle.Y(), middle.Z());
	glVertex3f(visible_direct_node.X(), visible_direct_node.Y(), visible_direct_node.Z());
	glEnd();*/

	glLineWidth(1.0f);
	glColor4f(node_a_color.X(), node_a_color.Y(), node_a_color.Z(), node_a_color.W());
	glBegin(GL_TRIANGLES);
	glVertex3f(arrow_a_node0.X(), arrow_a_node0.Y(), arrow_a_node0.Z());
	glVertex3f(width_edge_.vertA.X(), width_edge_.vertA.Y(), width_edge_.vertA.Z());
	glVertex3f(arrow_a_node1.X(), arrow_a_node1.Y(), arrow_a_node1.Z());
	glEnd();

	glColor4f(node_b_color.X(), node_b_color.Y(), node_b_color.Z(), node_b_color.W());
	glBegin(GL_TRIANGLES);
	glVertex3f(arrow_b_node1.X(), arrow_b_node1.Y(), arrow_b_node1.Z());
	glVertex3f(width_edge_.vertB.X(), width_edge_.vertB.Y(), width_edge_.vertB.Z());
	glVertex3f(arrow_b_node0.X(), arrow_b_node0.Y(), arrow_b_node0.Z());
	glEnd();

	glLineWidth(1.0f);
	glColor3f(1, 1, 1);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glPopAttrib();
}

void ToothWidthTool::updateVisible(Point3m _camera_view_direct)
{
	if (visible_direct_ * _camera_view_direct > 0)
	{
		visible_ = false;
	}
	else
	{
		visible_ = true;
	}
}

bool ToothWidthTool::mousePress(int _mouse_x, int _mouse_y)
{
	return false;
}

bool ToothWidthTool::mouseMove(int _mouse_x, int _mouse_y)
{
	return false;
}

bool ToothWidthTool::mouseRelease(int _mouse_x, int _mouse_y)
{
	return false;
}

EditModelMarkPlugin::EditModelMarkPlugin(QObject* parent /*= 0*/)
{
	timer_hovering_start = new QTimer(this);
	timer_hovering_start->setInterval(1000);
	connect(timer_hovering_start, &QTimer::timeout, this, [&]() {
		is_enable_mouse_hovering = true;
		if (timer_hovering_start)
		{
			timer_hovering_start->stop();
		}
		});

	initFdiAllFeaturesNames();
//	createRedoUndoGui();
	connect(PSIGNALMANAGER, &SignalManager::setCurrentMarkingStatusSignal, this, &EditModelMarkPlugin::setGifSlot);
    //p_crown_border_tool_ = std::make_unique<ToothMeshSegment>();
}

void EditModelMarkPlugin::createRedoUndoGui()
{
	QMainWindow* pMainWindow = UtilityTools::getInstance()->getSubMainWindow(parent->window());
	p_redoUndo_ui = new redoUndoGui(pMainWindow);
	p_redoUndo_ui->hide();
}

void EditModelMarkPlugin::setGifSlot(bool is_marking_status)
{
	if (is_marking_status)
	{
		if (p_redoUndo_ui)
		{
			p_redoUndo_ui->show();
		}
		else
		{
			createRedoUndoGui();
			p_redoUndo_ui->show();
		}
		if (gif_view_ui)
		{
			gif_view_ui->hide();
		}
		QSettings settings(QCoreApplication::applicationDirPath() + "./configs/config.ini", QSettings::IniFormat);
		//		QSettings settings;
		updateUI(true);
		if (settings.value(FixtoothGif).isValid() && settings.value(FixtoothGif) == 1)
		{
			if (gif_viewfix_ui)
			{
				gif_viewfix_ui->hide();
			}
			return;
		}
		if (gif_viewfix_ui)
		{
			gif_viewfix_ui->show();
		}
		else {
			createFixGif();
			gif_viewfix_ui->show();
		}
	}
	else {
		if (p_redoUndo_ui)
		{
			p_redoUndo_ui->hide();
		}
		else
		{
			createRedoUndoGui();
			p_redoUndo_ui->hide();
		}
		if (gif_viewfix_ui)
		{
			gif_viewfix_ui->hide();
		}
		QSettings settings(QCoreApplication::applicationDirPath() + "./configs/config.ini", QSettings::IniFormat);
		//		QSettings settings;
		//		gif_view_ui->showButton(false);
		updateUI(true);
		if (settings.value(MisstoothGif).isValid() && settings.value(MisstoothGif) == 1)
		{
			if (gif_view_ui)
			{
				gif_view_ui->hide();
			}
			return;
		}
		if (gif_view_ui)
		{
			gif_view_ui->show();
		}
		else {
			createMissGif();
			gif_view_ui->show();
		}
	}
}

void EditModelMarkPlugin::initFdiAllFeaturesNames()
{

	ToothFeatureConfig& config = PFusionAlignData->getToothFeatureConfig();

	for (auto tooth : config.getMarkConfig())
	{
		std::vector<QString> name;
		for (auto feature : tooth.second.marks_)
		{
			name.push_back(feature.name_);
		}
		fdi_feature_points_names_[QString::number(tooth.first)] = name;
	}
}

EditModelMarkPlugin::EditModelMarkPlugin(QAction* pAct, QObject* parent /*= 0*/)
{

}

EditModelMarkPlugin::~EditModelMarkPlugin()
{

}

void EditModelMarkPlugin::suggestedRenderingData(MeshModel& m, MLRenderingData& dt)
{

}

void EditModelMarkPlugin::initData(MeshModel* _mesh)
{
	is_modify_feature_points_ = false;
	this->mesh_ = _mesh;

	createGui();

	bool is_upper_tooth = _mesh->upperOrLowerToothModelMark == UpperToothModel;

	// Clear data
#if TOOTH_WIDTH_MEASURE
	clearVector(tooth_width_tool_list_);
#endif

	// Read marking information from project or JSON string
	if (getSegmentResultDataFormProjectOrJsonFile(is_upper_tooth))
	{
	}
	else
	{
		// Manually mark feature points, initialize tooth position information defaulting all exist
		initToothFdiTable(is_upper_tooth);
	}

	initStatusOperating();

	if (isEmptyProject())
	{
		// Empty project without marking points defaults to wisdom teeth missing
		setFdiOpsigenesMissing();
	}

	// Notify navigation map to update tooth position existence/missing markers
	tellFdiGuideToothExistedInfo();

	tellProgramCurrentToothFdiWorking();

//	emit PSIGNALMANAGER->saveWhatProjectSignal(false);

	if (b_operate_upper_)
	{
		emit PSIGNALMANAGER->onlyShowUpperDentalSignal(true);
	}
	else
	{
		emit PSIGNALMANAGER->onlyShowLowerDentalSignal(true);
	}

	// Save information
	saveSegmentStatusInfo(segmented_data_);

	if (b_operate_upper_)
	{
		emit PSIGNALMANAGER->markingDoneSignal(judgeFeaturePointsMarked());
	}
	emit PSIGNALMANAGER->markingAnyOneDentalDoneSignal(judgeFeaturePointsMarked());
}

bool EditModelMarkPlugin::StartEdit(MeshModel& m, GLArea* _parent, MLSceneGLSharedDataContext* cont)
{
	this->parent = _parent;

	initData(&m);

	// Command memory allocation
	if (command_manager_ == nullptr)
	{
		command_manager_ = new CommandManager();
	}
	clearCommand();
	return true;
}

void EditModelMarkPlugin::EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont)
{
	parent->bWhetherOpenGlobalTrackball = true;
	clearCommand();
	closeGui();
	saveSegmentStatusInfo(segmented_data_);

	if (b_operate_upper_)
	{
		emit PSIGNALMANAGER->onlyShowUpperDentalSignal(false);
	}
	else
	{
		emit PSIGNALMANAGER->onlyShowLowerDentalSignal(false);
	}
}

void EditModelMarkPlugin::Decorate(MeshModel& m, GLArea* parent, QPainter* p)
{

	if (!is_marking_status_)
	{
		return;
	}

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	// Draw split line
	if (b_show_crown_border_)
	{
		drawCrownBorderCurve();
	}

	// Feature point picking event
	if (b_pick_feature_points_ready_)
	{
		featurePointsMarkedEvent(m, parent);
		b_pick_feature_points_ready_ = false;

		if (b_operate_upper_)
		{
			emit PSIGNALMANAGER->markingDoneSignal(judgeFeaturePointsMarked());
		}

		emit PSIGNALMANAGER->markingAnyOneDentalDoneSignal(judgeFeaturePointsMarked());

		if (timer_hovering_start)
		{
			is_enable_mouse_hovering = false;
			timer_hovering_start->start();
		}
	}

	// Mouse selection picking feature point event
	if (is_picking_)
	{
		featurePointsPickedEvent(m, parent);
		is_picking_ = false;
	}

	if (!is_picked_ && is_mouse_hovering)
	{
		if (!featurePointsMouseHoveringEvent(m, parent))
		{
			feature_point_name_picked_ = QString("");
			fdi_mouse_hovering_get_feature_point_ = -1;
		}
		is_mouse_hovering = false;
	}

	if (is_picked_)
	{
		featurePointsMovingEvent(m, parent);

		if (cur_tooth_width_tool_ != nullptr)
		{
			Point3m viewDirect, cameraPos;
			UtilityTools::getInstance()->getCurrentCameraState(&m, parent->width(), parent->height(), viewDirect, cameraPos);
			cur_tooth_width_tool_->update(viewDirect);
		}
	}

	// Draw marking points
	if (b_show_feature_points_)
	{
		drawFeaturePoints(m, parent, p);
	}

	draw2DLabel(parent, p);
	glPopAttrib();

#if TOOTH_WIDTH_MEASURE
	Point3m viewDirect, cameraPos;
	UtilityTools::getInstance()->getCurrentCameraState(&m, parent->width(), parent->height(), viewDirect, cameraPos);
	for (auto& tool : this->tooth_width_tool_list_)
	{
		if (b_operate_upper_ == tool.isUpperPart())
		{
			tool.drawWidthLineSeg(p, viewDirect);
		}
	}
	if (cur_tooth_width_tool_ != nullptr)
	{
		if (b_operate_upper_ == cur_tooth_width_tool_->isUpperPart())
		{
			cur_tooth_width_tool_->draw(p, viewDirect, &m, PFusionAlignData->mesh_cloud_Octree_[&m], !parent->bWhetherOpenGlobalTrackball);
		}
	}
#endif
}

void EditModelMarkPlugin::keyPressEvent(QKeyEvent* event, MeshModel& m, GLArea* parent)
{
	if (event->key() == Qt::Key_H && event->modifiers() == Qt::AltModifier)
	{
		b_show_crown_border_ = !b_show_crown_border_;
		if (parent)
		{
			parent->update();
		}
	}

	if (is_marking_status_)
	{
		if (event->key() == Qt::Key_Z && event->modifiers() == Qt::ControlModifier)
		{
			unDo();
		}

		if (event->key() == Qt::Key_Y && event->modifiers() == Qt::ControlModifier)
		{
			reDo();
		}
	}
}

void EditModelMarkPlugin::mousePressEvent(QMouseEvent* event, MeshModel& m, GLArea* parent)
{
	if (!is_marking_status_)
	{
		return;
	}

	currentScreenPoint = event->pos();

	// Right mouse button response to undo feature marking information of current tooth position
	/*if (event->button() == Qt::RightButton && current_operator_tooth_fdi_ >= 0 && current_operator_tooth_fdi_ < tooth_picked_info_.size())
	{
		int index_feature_name = tooth_picked_info_[current_operator_tooth_fdi_]->feature_points.size() - 1;
		QString fdi = tooth_picked_info_[current_operator_tooth_fdi_]->label_fdi;

		if (index_feature_name >= 0 && index_feature_name < fdi_feature_points_names_[fdi].size())
		{
			QString feature_name = PFusionAlignData->getToothFeatureConfig().getMarkName(fdi.toInt(), fdi_feature_points_names_[fdi][index_feature_name]);
			tooth_picked_info_[current_operator_tooth_fdi_]->feature_points.erase(feature_name);
		}
	}*/

	if (event->button() == Qt::LeftButton && !is_picked_)
	{
		if (!b_pick_feature_points_ready_)
		{
			is_picking_ = true;
		}
	}

	parent->update();
}

void EditModelMarkPlugin::mouseMoveEvent(QMouseEvent* event, MeshModel& m, GLArea* parent)
{
	if (!is_marking_status_)
	{
		return;
	}

	currentScreenPoint = event->pos();

	if (is_enable_mouse_hovering)
	{
		if (is_picked_)
		{
			// Disable global trackball
			parent->bWhetherOpenGlobalTrackball = false;
		}

		if (!b_pick_feature_points_ready_)
		{
			is_mouse_hovering = true;
		}
	}
	parent->update();

}

void EditModelMarkPlugin::mouseReleaseEvent(QMouseEvent* event, MeshModel& m, GLArea* parent)
{

	is_picking_ = false;

	if (is_picked_)
	{
		featurePointsModifyRecord(feature_point_last_, feature_point_current_);

		if (!judgeFeaturePointsMarked())
		{
			tellProgramCurrentToothFdiWorking();
		}
	}
	is_picked_ = false;

	currentScreenPoint = event->pos();
	// Restore global trackball
	parent->bWhetherOpenGlobalTrackball = true;

	parent->update();
}

void EditModelMarkPlugin::mouseDoubleClickEvent(QMouseEvent* event, MeshModel& m, GLArea* parent)
{
	if (!is_marking_status_)
	{
		return;
	}

	if (event->button() == Qt::LeftButton)
	{
		b_pick_feature_points_ready_ = true;
		// Disable global trackball
		parent->bWhetherOpenGlobalTrackball = false;
	}

	parent->update();
}

void EditModelMarkPlugin::updateUI(bool visible /*= true*/)
{
	if (feature_marking_gui_)
	{
		feature_marking_gui_->setVisible(visible);
		QPoint pos = QPoint(0, 0);
		//feature_marking_gui_->setFixedWidth(EDIT_PLUGIN_DIALOG_WIDTH);
		feature_marking_gui_->setGeometry(pos.x(), pos.y(),
			feature_marking_gui_->width(), feature_marking_gui_->height());

		feature_marking_gui_->update();

	}
	int width = parent->window()->width();
	int height = parent->window()->height();
	if (p_redoUndo_ui)
	{
		if(parent)
		{
			QPoint pointOri = parent->mapToGlobal(QPoint(0, 0));
			int posX = pointOri.x() + parent->window()->width() / 2 - p_redoUndo_ui->width() / 2;
			int posY = pointOri.y() + 5;
			p_redoUndo_ui->setGeometry(posX, posY,
				p_redoUndo_ui->width(), p_redoUndo_ui->height());
		}
		p_redoUndo_ui->update();
	}
	/*if (gif_view_label_ui)
	{
		gif_view_label_ui->setVisible(visible);
		gif_view_label_ui->setGeometry(parent->window()->width() - 400, 20,
			gif_view_label_ui->width(), gif_view_label_ui->height());
		qDebug() << parent->window()->width() - 400;
		gif_view_label_ui->update();
	}  */
	//PSIGNALMANAGER->updatePluginUISignal();
	if (gif_view_ui && !is_marking_status_)
	{
		QSettings settings(QCoreApplication::applicationDirPath() + "./configs/config.ini", QSettings::IniFormat);
		if (settings.value(MisstoothGif).isValid() && settings.value(MisstoothGif) == 1)
		{
			gif_view_ui->showButton(false);
		}
		gif_view_ui->setGeometry(parent->window()->width() - 400, 60,
			gif_view_ui->width(), gif_view_ui->height());
//		qDebug() << parent->window()->width() - 400;
		gif_view_ui->update();
	}
	if (gif_viewfix_ui && is_marking_status_)
	{
		QSettings settings(QCoreApplication::applicationDirPath() + "./configs/config.ini", QSettings::IniFormat);
		if (settings.value(FixtoothGif).isValid() && settings.value(FixtoothGif) == 1)
		{
			gif_viewfix_ui->showButton(false);
		}
		gif_viewfix_ui->setGeometry(parent->window()->width() - 400, 60,
			gif_viewfix_ui->width(), gif_viewfix_ui->height());
//		qDebug() << parent->window()->width() - 400;
		gif_viewfix_ui->update();
	}

}

void EditModelMarkPlugin::createGui()
{
	if (feature_marking_gui_ == nullptr)
	{
		QMainWindow* pMainWindow = UtilityTools::getInstance()->getSubMainWindow(parent->window());
		feature_marking_gui_ = new FeatureMarkGui(pMainWindow);
		//		feature_marking_gui_->setFixedHeight(200);
		//		feature_marking_gui_->setFixedWidth(200);
				// Add signals
		connect(feature_marking_gui_, &FeatureMarkGui::doneSignal, PSIGNALMANAGER, &SignalManager::endEditSignal);

		// Navigation map signals
		connect(this, &EditModelMarkPlugin::currentWorkingToothSignal,
			feature_marking_gui_->tooth_fdi_preview_ui_, &ui_common::ToothFdiGuideGui::setWhichToothWorkingSlot);

		connect(feature_marking_gui_->tooth_fdi_preview_ui_, &ui_common::ToothFdiGuideGui::setWhichToothWorkingSignal,
			this, &EditModelMarkPlugin::currentWorkingToothSlot);

		connect(this, &EditModelMarkPlugin::initToothExistMarkSignal,
			feature_marking_gui_->tooth_fdi_preview_ui_, &ui_common::ToothFdiGuideGui::setToothExistedMapSlot);

		connect(feature_marking_gui_->tooth_fdi_preview_ui_, &ui_common::ToothFdiGuideGui::setWhichToothLostSignal,
			this, &EditModelMarkPlugin::whichFdiToothLostSlot);

		connect(feature_marking_gui_->tooth_fdi_preview_ui_, &ui_common::ToothFdiGuideGui::setWhichToothExistSignal,
			this, &EditModelMarkPlugin::undoWhichFdiToothLostSlot);

		connect(feature_marking_gui_->tooth_fdi_preview_ui_, &ui_common::ToothFdiGuideGui::reMarkingCurrentToothSignal,
			this, &EditModelMarkPlugin::reLocatedTheFdiToothSlot);

		connect(feature_marking_gui_->tooth_fdi_preview_ui_, &ui_common::ToothFdiGuideGui::reMarkingAllToothSignal,
			this, &EditModelMarkPlugin::reLocatedAllToothSetCommand);

		connect(this, &EditModelMarkPlugin::setRightMouseMenuEnabledStatusSignal,
			feature_marking_gui_->tooth_fdi_preview_ui_, &ui_common::ToothFdiGuideGui::setRightMouseMenuEnabledStatusSlot);

		connect(PSIGNALMANAGER, &SignalManager::setCurrentMarkingStatusSignal, this, &EditModelMarkPlugin::setCurrentMarkingStatusSlot);

		connect(PSIGNALMANAGER, &SignalManager::showGifSignal, this, &EditModelMarkPlugin::showGifSlot);

		connect(PSIGNALMANAGER, &SignalManager::saveFeaturePointsSignal, this, &EditModelMarkPlugin::saveFeaturePointsSlot);

		connect(PSIGNALMANAGER, &SignalManager::unDoSignal, this, &EditModelMarkPlugin::unDo, Qt::QueuedConnection);
		connect(PSIGNALMANAGER, &SignalManager::reDoSignal, this, &EditModelMarkPlugin::reDo, Qt::QueuedConnection);
	}

	//if (gif_view_label_ui == nullptr)
	//{
	//	QMainWindow* pMainWindow = UtilityTools::getInstance()->getSubMainWindow(parent->window());
	//	gif_view_label_ui = new GifViewLabel(pMainWindow);
	//	gif_view_label_ui->setFixedHeight(350);
	//	gif_view_label_ui->setFixedWidth(350);
	//	gif_view_label_ui->addImage(imagepairlst);
	//	gif_view_label_ui->setGeometry(parent->window()->width() - 400, 20,
	//		gif_view_label_ui->width(), gif_view_label_ui->height());
	//}
	QSettings settings(QCoreApplication::applicationDirPath() + "./configs/config.ini", QSettings::IniFormat);
	//	QSettings settings;
	if (gif_view_ui == nullptr)
	{
		if (!settings.value(MisstoothGif).isValid())
			settings.setValue(MisstoothGif, 2);
		if (settings.value(MisstoothGif) != 1)
		{

			createMissGif();
		}
	}
	if (gif_viewfix_ui == nullptr)
	{
		if (!settings.value(FixtoothGif).isValid())
			settings.setValue(FixtoothGif, 2);
//		if (settings.value(FixtoothGif) != 1)
//		{
//			createFixGif();
//		}
	}
	if (feature_marking_gui_)
	{
		// Special signal connection - needs to be closed when exiting plugin
		connect(PSIGNALMANAGER, &SignalManager::setSelectDentalSignal, this, &EditModelMarkPlugin::tabUpperOrLowerSelectedSlot);

		feature_marking_gui_->show();
	}
	/*	if (gif_view_label_ui)
		{
			gif_view_label_ui->show();
		}  */
	if (gif_view_ui && settings.value(MisstoothGif) != 1)
	{
		gif_view_ui->show();
	}
}

void EditModelMarkPlugin::closeGui()
{
	if (feature_marking_gui_)
	{
		// Special signal connection - needs to be closed when exiting plugin
		disconnect(PSIGNALMANAGER, &SignalManager::setSelectDentalSignal, this, &EditModelMarkPlugin::tabUpperOrLowerSelectedSlot);
		feature_marking_gui_->hide();
		//gif_view_label_ui->hide();
	}
	if (gif_view_ui)
	{
		gif_view_ui->hide();
	}
	if (gif_viewfix_ui)
	{
		gif_viewfix_ui->hide();
	}
	if (p_redoUndo_ui)
	{
		p_redoUndo_ui->hide();
	}
}

void EditModelMarkPlugin::tabUpperOrLowerSelectedSlot(int dentalType)
{
	static bool flag = false;
	static QTimer* count_timer = nullptr;
	if (count_timer == nullptr)
	{
		count_timer = new QTimer(this);
		count_timer->setInterval(300);
		connect(count_timer, &QTimer::timeout, this, [&]() {
			flag = false; count_timer->stop(); });
	}
	if (flag)
	{
		return;
	}

	ToothModelType type_need_tab(ToothModelType(UpperToothModel + dentalType));
	// If upper or lower jaw to switch is same as current, return directly
	if (this->mesh_ && this->mesh_->upperOrLowerToothModelMark == type_need_tab)
		return;

	parent->bWhetherOpenGlobalTrackball = true;

	saveSegmentStatusInfo(segmented_data_);

	/*	if (!judgeFeaturePointsMarked(segmented_data_))
		{
			UiUtilityTools::getInstance()->showInfoMessageBox(tr("Info"), tr("all feature points marked not done."));

			flag = true;
			count_timer->start();

			// Restore originally selected mesh and only selected mesh
			if (this->mesh_ && this->mesh_->upperOrLowerToothModelMark == UpperToothModel)
			{
				emit PSIGNALMANAGER->setUpperDentalSelectedSignal();
				emit PSIGNALMANAGER->onlyShowUpperDentalSignal(true);
			}
			else
			{
				emit PSIGNALMANAGER->setLowerDentalSelectedSignal();
				emit PSIGNALMANAGER->onlyShowLowerDentalSignal(true);
			}
			return;
		}  */


	for (auto& it : parent->md()->meshList)
	{
		if (it->upperOrLowerToothModelMark == type_need_tab)
		{
			initData(it);
			break;
		}
	}
	if (segmented_data_)
	{
		segmented_data_->mark_point_modifyed_ = true;
	}
	parent->update();
}

void EditModelMarkPlugin::createMissGif()
{
	QSettings settings(QCoreApplication::applicationDirPath() + "./configs/config.ini", QSettings::IniFormat);
	//	QSettings settings;
	bool showBtn = true;
	if (settings.value(MisstoothGif).isValid() && settings.value(MisstoothGif) == 1)
		showBtn = false;
	QMainWindow* pMainWindow = UtilityTools::getInstance()->getSubMainWindow(parent->window());
	gif_view_ui = new Gifviewgui(showBtn, pMainWindow);
	gif_view_ui->gifType = MisstoothGif;
	QList<std::pair<QString, QString>> imagepairlst;
	if (PFusionAppData->getAppLanguage() == E_CHINESE)
	{
		imagepairlst = { std::make_pair(":/ui/gif/operation.gif", "") ,std::make_pair(":/ui/gif/misstooth.gif", "")
		};
	}
	else if (PFusionAppData->getAppLanguage() == E_ENGLISH)
	{
		imagepairlst = { std::make_pair(":/ui/gif/operation_ENG.gif", ""),std::make_pair(":/ui/gif/misstooth_ENG.gif", "")
		};
	}
	gif_view_ui->showGif(imagepairlst);
	gif_view_ui->setGeometry(parent->window()->width() - 400, 60,
		gif_view_ui->width(), gif_view_ui->height());
}

void EditModelMarkPlugin::createFixGif()
{
	QSettings settings(QCoreApplication::applicationDirPath() + "./configs/config.ini", QSettings::IniFormat);
	//	QSettings settings;
	bool showBtn = true;
	if (settings.value(FixtoothGif).isValid() && settings.value(FixtoothGif) == 1)
		showBtn = false;
	QMainWindow* pMainWindow = UtilityTools::getInstance()->getSubMainWindow(parent->window());
	gif_viewfix_ui = new Gifviewgui(showBtn, pMainWindow);
	gif_viewfix_ui->gifType = FixtoothGif;
	QList<std::pair<QString, QString>> imagepairlst;
	if (PFusionAppData->getAppLanguage() == E_CHINESE)
	{
		imagepairlst = { std::make_pair(":/ui/gif/fixtooth.gif", ""),
			std::make_pair(":/ui/gif/operation.gif", "")
		};
	}
	else if (PFusionAppData->getAppLanguage() == E_ENGLISH)
	{
		imagepairlst = { std::make_pair(":/ui/gif/fixtooth_ENG.gif", ""),
			std::make_pair(":/ui/gif/operation_ENG.gif", "")
		};
	}
	gif_viewfix_ui->showGif(imagepairlst);
	gif_viewfix_ui->setGeometry(parent->window()->width() - 400, 60,
		gif_viewfix_ui->width(), gif_viewfix_ui->height());
}
void EditModelMarkPlugin::showGifSlot(bool openGif)
{
	if (gif_view_ui && gif_view_ui->isVisible())
	{
		gif_view_ui->hide();
		return;
	}
	if (gif_viewfix_ui && gif_viewfix_ui->isVisible())
	{
		gif_viewfix_ui->hide();
		return;
	}
	if (is_marking_status_)
	{
		if (gif_view_ui)
		{
			gif_view_ui->hide();
		}
		if (!gif_viewfix_ui)
		{
			createFixGif();
		}
		gif_viewfix_ui->show();
	}
	else
	{
		if (gif_viewfix_ui)
		{
			gif_viewfix_ui->hide();
		}
		if (!gif_view_ui)
		{
			createMissGif();
		}
		gif_view_ui->show();
	}
}

void EditModelMarkPlugin::saveFeaturePointsSlot()
{
	bool cur_pro = false;
	if (PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageFixLower || PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageFixUpper || PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageMissLower || PFusionAlignData->getAnalyserData().cur_proj_stage_ == E_ProjAnaStageMissUpper)
	{
		cur_pro = true;
	}
	if (/*cur_pro &&*/ parent && parent->getCurrentEditAction() && parent->getCurrentEditAction()->property("Plugin").toString() == ("ModelMark"))
	{
		saveSegmentStatusInfo(segmented_data_);
	}
}

void EditModelMarkPlugin::setCurrentMarkingStatusSlot(bool is_marking)
{
	is_marking_status_ = is_marking;
	clearCommand();
}

bool EditModelMarkPlugin::judgeFeaturePointsMarked(const SegmentedStatusInfo* segmented_info)const
{
	if (segmented_info == nullptr)
	{
		return false;
	}

	const auto& feaConfig = PFusionAlignData->getToothFeatureConfig().getMarkConfig();
	if (segmented_info->toothFDIInfo.size() == segmented_info->feature_points_mark.size())
	{
		for (auto& mark : segmented_info->feature_points_mark)
		{
			auto it = feaConfig.find(mark.first.toInt());
			if (it != feaConfig.end())
			{
				if (mark.second.landmarks.size() < it->second.marks_.size())
				{
					return false;
				}
			}
		}
		return true;
	}
	return false;
}

bool EditModelMarkPlugin::judgeFeaturePointsMarked()
{
	for (auto& it : tooth_picked_info_)
	{
		if (it->is_existed)
		{
			int size_features = it->feature_points.size();
			QString fdi = it->label_fdi;

			if (size_features >= 0 && size_features >= fdi_feature_points_names_[fdi].size())
			{
			}
			else
			{
				return false;
			}
		}
	}
	return true;
}

void EditModelMarkPlugin::currentWorkingToothSlot(const QString& label_fdi)
{
	int index_tooth = getIndexToothFromFdi(label_fdi);

	if (index_tooth < 0 || index_tooth > 15)
	{
		cur_tooth_width_tool_ = nullptr;
		return;
	}

	this->current_operator_tooth_fdi_ = index_tooth;

	updateHintCurrentFdiFeatureNames();

	if (parent)
	{
		parent->update();
	}
}

void EditModelMarkPlugin::whichFdiToothLostSlot(const QString& fdi_tooth)
{
	int index_tooth = getIndexToothFromFdi(fdi_tooth);

	if (index_tooth >= 0)
	{
		tooth_picked_info_[index_tooth]->clearData();
		deleteToothWidthToolWith(fdi_tooth);

		tellProgramCurrentToothFdiWorking();

		is_modify_feature_points_ = true;

		if (b_operate_upper_)
		{
			emit PSIGNALMANAGER->markingDoneSignal(judgeFeaturePointsMarked());
		}

		emit PSIGNALMANAGER->markingAnyOneDentalDoneSignal(judgeFeaturePointsMarked());
	}

}

void EditModelMarkPlugin::undoWhichFdiToothLostSlot(const QString& fdi_tooth)
{
	int index_tooth = getIndexToothFromFdi(fdi_tooth);

	if (index_tooth >= 0)
	{
		if (segmented_data_)
		{
			segmented_data_->mark_point_modifyed_ = true;
		}

		tooth_picked_info_[index_tooth]->clearData();
		tooth_picked_info_[index_tooth]->setIsExisted(true);
		deleteToothWidthToolWith(fdi_tooth);

		tellProgramCurrentToothFdiWorking();

		is_modify_feature_points_ = true;

		if (b_operate_upper_)
		{
			emit PSIGNALMANAGER->markingDoneSignal(judgeFeaturePointsMarked());
		}
		emit PSIGNALMANAGER->markingAnyOneDentalDoneSignal(judgeFeaturePointsMarked());
	}
}

void EditModelMarkPlugin::reLocatedTheFdiToothSlot(const QString& fdi_tooth)
{
	int index_tooth = getIndexToothFromFdi(fdi_tooth);

	if (index_tooth >= 0)
	{
		if (segmented_data_)
		{
			segmented_data_->mark_point_modifyed_ = true;
		}

		// Record backup copy for undo
		featurePointsClearRecord(*tooth_picked_info_[index_tooth]);

		tooth_picked_info_[index_tooth]->clearFeaturePointsData();
		deleteToothWidthToolWith(fdi_tooth);
		tellProgramCurrentToothFdiWorking();

		is_modify_feature_points_ = true;

		if (b_operate_upper_)
		{
			emit PSIGNALMANAGER->markingDoneSignal(judgeFeaturePointsMarked());
		}
		emit PSIGNALMANAGER->markingAnyOneDentalDoneSignal(judgeFeaturePointsMarked());
	}
}

void EditModelMarkPlugin::reLocatedAllToothSetCommand()
{

	// Record backup copy for undo
	std::vector<FeaturePointsMarking::ToothPickedInfomation> data;
	for (auto& it : tooth_picked_info_)
	{
		data.push_back(*it);
	}
	featurePointsAllClearRecord(data);

	if (segmented_data_)
	{
		segmented_data_->mark_point_modifyed_ = true;
	}

	for (auto& info : tooth_picked_info_)
	{
		info->clearFeaturePointsData();
	}
#if TOOTH_WIDTH_MEASURE
	tooth_width_tool_list_.clear();
	cur_tooth_width_tool_ = nullptr;
#endif

	current_operator_tooth_fdi_ = -1;
	tellProgramCurrentToothFdiWorking();

	// Separation functionality removed - not used in FusionAnalyser
	// p_cur_mesh_ is no longer available, always emit markingDoneSignal
		emit PSIGNALMANAGER->markingDoneSignal(judgeFeaturePointsMarked());
	// }
	emit PSIGNALMANAGER->markingAnyOneDentalDoneSignal(judgeFeaturePointsMarked());

	is_modify_feature_points_ = true;
}

int EditModelMarkPlugin::getIndexToothFromFdi(const QString& label_fdi)const
{

	for (size_t index = 0; index < tooth_picked_info_.size(); ++index)
	{
		if (tooth_picked_info_[index]->label_fdi == label_fdi)
		{
			return index;
		}
	}

	return -1;
}

int EditModelMarkPlugin::getCurrentToothFDIWorking()const
{

	// Check if picking for currently operating tooth position is finished
	if (current_operator_tooth_fdi_ >= 0 && current_operator_tooth_fdi_ < tooth_picked_info_.size())
	{
		auto it = tooth_picked_info_[current_operator_tooth_fdi_];
		if (it->is_existed && fdi_feature_points_names_.find(it->label_fdi) != fdi_feature_points_names_.end())
		{
			if (it->feature_points.size() < fdi_feature_points_names_.at(it->label_fdi).size())
			{
				return current_operator_tooth_fdi_;
			}
		}
	}

	// If current tooth position satisfies picking, traverse from start to end to find which tooth position doesn't satisfy and update current operation
	for (int iTooth = 0; iTooth < tooth_picked_info_.size(); ++iTooth)
	{
		auto it = tooth_picked_info_[iTooth];

		if (it->is_existed && fdi_feature_points_names_.find(it->label_fdi) != fdi_feature_points_names_.end())
		{
			if (it->feature_points.size() < fdi_feature_points_names_.at(it->label_fdi).size())
			{
				return iTooth;
			}
		}

	}

	// Otherwise return first existing one (when all are marked)
	for (int iTooth = 0; iTooth < tooth_picked_info_.size(); ++iTooth)
	{
		if (tooth_picked_info_[iTooth]->is_existed)
		{
			return iTooth;
		}
	}

	// If no tooth positions exist, return -1
	return -1;
}

void EditModelMarkPlugin::initStatusOperating()
{
	current_operator_tooth_fdi_ = -1;

	// Name of picked feature point
	feature_point_name_picked_ = "";

	// Whether ready to pick feature points (on mouse double click)
	b_pick_feature_points_ready_ = false;

	// Whether left click or hover triggers selection of a feature point to be adjusted
	is_picking_ = false;

	// Whether a feature point is selected
	is_picked_ = false;

	// Whether mouse is sliding
	is_mouse_hovering = false;
}

void EditModelMarkPlugin::tellProgramCurrentToothFdiWorking()
{
	auto index_tooth_prepare_operating = getCurrentToothFDIWorking();

	current_operator_tooth_fdi_ = index_tooth_prepare_operating;

	if (current_operator_tooth_fdi_ >= 0 && current_operator_tooth_fdi_ < tooth_picked_info_.size())
	{
		emit currentWorkingToothSignal(tooth_picked_info_[current_operator_tooth_fdi_]->label_fdi);
	}

	updateHintCurrentFdiFeatureNames();

	if (parent)
	{
		parent->update();
	}
}

void EditModelMarkPlugin::tellProgramCurrentToothFdiWorking(const QString& label_fdi)
{
	auto index = getIndexToothFromFdi(label_fdi);

	if (index >= 0)
	{
		current_operator_tooth_fdi_ = index;

		if (current_operator_tooth_fdi_ >= 0 && current_operator_tooth_fdi_ < tooth_picked_info_.size())
		{
			emit currentWorkingToothSignal(tooth_picked_info_[current_operator_tooth_fdi_]->label_fdi);
		}

		updateHintCurrentFdiFeatureNames();

		if (parent)
		{
			parent->update();
		}
	}
}

bool EditModelMarkPlugin::getSegmentResultDataFormProjectOrJsonFile(bool is_upper_dental)
{

	getSegmentDataMemory(is_upper_dental, segmented_data_);

	// First check from project data if segmentation project data exists
	bool existed_project = readSegmentStatusInfo(segmented_data_, is_upper_dental);

	if (existed_project)
	{
		return true; // Data information read from project
	}

	// Separation functionality removed - not used in FusionAnalyser
	return false;
}

void EditModelMarkPlugin::autoSegmentResultReadAndStatusInit(bool is_upper, const std::map<QString, CrownInfoSegmentedIntelligent>& data_crown_segmented)
{

	initToothFdiTable(is_upper);

	// Assign coordinate axis information for existing tooth positions
	for (auto& tooth : tooth_picked_info_)
	{

		bool is_existed = data_crown_segmented.find(tooth->label_fdi) != data_crown_segmented.end();

		tooth->is_existed = is_existed;

		if (is_existed)
		{
			auto& data = data_crown_segmented.at(tooth->label_fdi);
			tooth->clearFeaturePointsData();
			// Store crown marking point records
			tooth->feature_points = data.landmarks;

			tooth->clearBorderInfo();
			if (!data.boundryVertexsCrown.empty())
			{
				auto border_sort = UtilityTools::getInstance()->sortBorderVertexs(data.boundryVertexsCrown);
				tooth->border_info.toothBorderClosedContinuousIndex = UtilityTools::getInstance()->matchPointsFromMesh(this->mesh_, border_sort);
			}
		}
	}

	// Update and calculate boundary bezier
	generateAndUpdateCrownBorderCurve();
}

void EditModelMarkPlugin::getSegmentDataMemory(bool is_upper_dental, SegmentedStatusInfo*& _segmented_data)
{
	// Separation functionality removed - not used in FusionAnalyser
	_segmented_data = new SegmentedStatusInfo;
	_segmented_data->mark_point_modifyed_ = false;
	b_operate_upper_ = is_upper_dental;

	// if (is_upper_dental && PFusionAlignData->segmneted_info_upper_)
	// {
	// 	_segmented_data = PFusionAlignData->segmneted_info_upper_;
	// }
	// else if (!is_upper_dental && PFusionAlignData->segmneted_info_lower_)
	// {
	// 	_segmented_data = PFusionAlignData->segmneted_info_lower_;
	// }
	// else
	// {
	// 	_segmented_data = new SegmentedStatusInfo;
	// 	if (is_upper_dental)
	// 	{
	// 		PFusionAlignData->segmneted_info_upper_ = _segmented_data;
	// 	}
	// 	else
	// 	{
	// 		PFusionAlignData->segmneted_info_lower_ = _segmented_data;
	// 	}
	// }
	// _segmented_data->mark_point_modifyed_ = false;
	// b_operate_upper_ = is_upper_dental;
}

bool EditModelMarkPlugin::readSegmentStatusInfo(const SegmentedStatusInfo* segmentStatus, bool is_upper)
{

	if (segmentStatus == nullptr || segmentStatus->bIsLossTooth.empty())
	{
		return false;
	}

	std::vector<int> fdi_existed_map(segmentStatus->bIsLossTooth.begin() + 1, segmentStatus->bIsLossTooth.end()); // Tooth position missing information

	bool is_existed = false;

	for (auto& it : fdi_existed_map) // Invert to get tooth position existence information
	{
		it = it ? 0 : 1;

		is_existed |= it;
	}

	initToothFdiTable(is_upper, fdi_existed_map);

	// Assign feature marking point information
	int num_tooth_existed_add = 0;
	for (auto& it : tooth_picked_info_)
	{
		if (!it->is_existed)
		{
			continue;
		}

		it->clearFeaturePointsData();
		if (segmentStatus->feature_points_mark.find(it->label_fdi) != segmentStatus->feature_points_mark.end())
		{
			it->feature_points = segmentStatus->feature_points_mark.at(it->label_fdi).landmarks;
		}

		it->clearBorderInfo();
		if (segmentStatus->contourVertexIndexTooth.size() > num_tooth_existed_add)
		{
			it->border_info.bezierControlPointsIndex = segmentStatus->contourVertexIndexTooth[num_tooth_existed_add];
		}

#if TOOTH_WIDTH_MEASURE
		if (it->feature_points.size() >= 2)
		{
			Point3m viewDirect, cameraPos;
			QString feature_name_a = PFusionAlignData->getToothFeatureConfig().getMarkName(it->label_fdi.toInt(), fdi_feature_points_names_[it->label_fdi][0]);
			QString feature_name_b = PFusionAlignData->getToothFeatureConfig().getMarkName(it->label_fdi.toInt(), fdi_feature_points_names_[it->label_fdi][1]);
			if (getToothWidthToolWith(it->label_fdi, &(it->feature_points[feature_name_a]), &(it->feature_points[feature_name_b])) == nullptr)
			{
				UtilityTools::getInstance()->getCurrentCameraState(nullptr, parent->width(), parent->height(), viewDirect, cameraPos);
				tooth_width_tool_list_.push_back(ToothWidthTool(
					it->label_fdi,
					&(it->feature_points[feature_name_a]),
					&(it->feature_points[feature_name_b]),
					viewDirect));
			}

			if (it->feature_points.size() == 3)
			{
				QString feature_name_c = PFusionAlignData->getToothFeatureConfig().getMarkName(it->label_fdi.toInt(), fdi_feature_points_names_[it->label_fdi][2]);
				ToothWidthTool* p_tool = getToothWidthToolWith(it->label_fdi,
					&(it->feature_points[feature_name_a]),
					&(it->feature_points[feature_name_b]));

				if (p_tool != nullptr)
				{
					p_tool->updateVisibleDirectWithThirdNode(it->feature_points[feature_name_c]);
				}
			}
		}
#endif

		++num_tooth_existed_add;
	}

	// Recalculate curve control points using provided boundary points when no control points exist
	for (int i_tooth = 0; i_tooth < tooth_picked_info_.size(); ++i_tooth)
	{
		auto it = tooth_picked_info_[i_tooth];
		if (!it->is_existed)
		{
			continue;
		}

		if (it->border_info.bezierControlPointsIndex.empty())
		{
			if (segmentStatus->borderVertexIndexEachTooth.size() > (i_tooth + 1))
			{
				it->border_info.toothBorderClosedContinuousIndex = segmentStatus->borderVertexIndexEachTooth[i_tooth + 1];
			}
		}

	}

	// Update and calculate boundary bezier
	generateAndUpdateCrownBorderCurve();

	return true;
}

void EditModelMarkPlugin::generateAndUpdateCrownBorderCurve()
{
	// Update and calculate boundary bezier
	// Note: toothmeshsegment functionality has been removed as segment module is not used
}

bool EditModelMarkPlugin::saveSegmentStatusInfo(SegmentedStatusInfo* segmentStatus)
{

	if (segmentStatus == nullptr)
	{
		return false;
	}

	segmentStatus->bIsLossTooth.clear();
	segmentStatus->bIsLossTooth.push_back(true);

	segmentStatus->toothFDIInfo.clear();

	//segmentStatus->feature_points_mark.clear();

	std::vector<Point3m> upperToothOcclusalPlaneControlPoints;

	for (int i = 0; i < tooth_picked_info_.size(); ++i)
	{
		auto tooth = tooth_picked_info_[i];

		segmentStatus->bIsLossTooth.push_back(!tooth->is_existed);

		if (tooth->is_existed)
		{
			segmentStatus->toothFDIInfo.push_back(ToothFDIInfo(tooth->label_fdi));

			CrownInfoSegmentedIntelligent feature_point;
			std::map<QString, vcg::Point3f> landmarks;
			std::map<QString, sFeaturePoint> land_marks_;

			//feature_point.landmarks = tooth->feature_points;
			for (auto& mark : tooth->feature_points)
			{
				landmarks[mark.first] = mark.second;
			}

			for (auto& mark : landmarks)
			{
				sFeaturePoint point;
				point.mark_name_ = mark.first;
				point.name_ = PFusionAlignData->getToothFeatureConfig().getName(tooth->label_fdi.toInt(), point.mark_name_);
				point.index_ = PFusionAlignData->getToothFeatureConfig().getIndex(tooth->label_fdi.toInt(), point.mark_name_);
				point.pt_ = mark.second;

				land_marks_[mark.first] = point;
			}

			segmentStatus->feature_points_mark[tooth->label_fdi].landmarks = landmarks;
			segmentStatus->feature_points_mark[tooth->label_fdi].land_marks_ = land_marks_;

		}
		else
		{
			if (segmentStatus->feature_points_mark.find(tooth->label_fdi) != segmentStatus->feature_points_mark.end())
			{
				segmentStatus->feature_points_mark.erase(tooth->label_fdi);
			}
		}
	}

	return true;
}

void EditModelMarkPlugin::initToothFdiTable(bool is_upper_dental, const std::vector<int>& is_existed_map)
{
	std::vector<QString> fdi_vec;

	if (is_upper_dental)
	{
		fdi_vec =
		{ "18","17","16","15","14","13","12","11","21","22","23","24", "25","26","27","28" };
	}
	else
	{
		fdi_vec =
		{ "48","47","46","45","44","43","42","41","31","32","33","34", "35","36","37","38" };
	}

	clearPointerVector(tooth_picked_info_);

	if (is_existed_map.empty())
	{
		for (auto& fdi : fdi_vec)
		{
			// Initialize non-existent tooth position
			tooth_picked_info_.push_back(new FeaturePointsMarking::ToothPickedInfomation(fdi, true));
		}
	}
	else
	{
		for (int i = 0; i < 16; ++i)
		{
			tooth_picked_info_.push_back(new FeaturePointsMarking::ToothPickedInfomation(fdi_vec[i], is_existed_map[i]));
		}
	}

}

void EditModelMarkPlugin::setFdiOpsigenesMissing()
{
	std::set<QString> fdi_missing{ "18","28","38","48" };
	for (auto& it : tooth_picked_info_)
	{
		if (it->is_existed && fdi_missing.find(it->label_fdi) != fdi_missing.end())
		{
			it->is_existed = !it->is_existed;
		}
	}
}

bool EditModelMarkPlugin::isEmptyProject()const
{
	for (auto& it : tooth_picked_info_)
	{
		if (it->is_existed && !it->feature_points.empty())
		{
			return false;
		}
	}

	return true;
}

void EditModelMarkPlugin::tellFdiGuideToothExistedInfo()
{
	std::vector<std::pair<QString, bool>> fdi_exist_map;

	for (auto& it : tooth_picked_info_)
	{
		fdi_exist_map.push_back(std::make_pair(it->label_fdi, it->is_existed));
	}
	emit initToothExistMarkSignal(fdi_exist_map);

	if (feature_marking_gui_)
	{
		feature_marking_gui_->repaint();
	}
}

void EditModelMarkPlugin::featurePointsMarkedEvent(MeshModel& m, GLArea* gla)
{

	Point3d pointPicked = UtilityTools::getInstance()->transformPointFrom2DscreenTo3Dworld(
		QTLogicalToDevice(gla, currentScreenPoint.x()),
		QTLogicalToDevice(gla, gla->height() - currentScreenPoint.y()), &m.cm);

	int vIdPicked = UtilityTools::getInstance()->getIndexPointClosestMesh(m, Point3m(pointPicked.X(), pointPicked.Y(), pointPicked.Z()));

	if (vIdPicked < 0)
	{
		cur_tooth_width_tool_ = nullptr;
		return;
	}

	if (current_operator_tooth_fdi_ >= 0 && current_operator_tooth_fdi_ < tooth_picked_info_.size())
	{

		int index_feature_name = tooth_picked_info_[current_operator_tooth_fdi_]->feature_points.size();
		QString fdi = tooth_picked_info_[current_operator_tooth_fdi_]->label_fdi;

		if (index_feature_name >= 0 && index_feature_name < fdi_feature_points_names_[fdi].size())
		{
			segmented_data_->mark_point_modifyed_ = true;
			Point3f p(pointPicked.X(), pointPicked.Y(), pointPicked.Z());
			//QString feature_name(QString("Pt") + QString::number(index_feature_name));

			QString feature_name = PFusionAlignData->getToothFeatureConfig().getMarkName(fdi.toInt(), fdi_feature_points_names_[fdi][index_feature_name]);
			tooth_picked_info_[current_operator_tooth_fdi_]->feature_points[feature_name] = p;

#if TOOTH_WIDTH_MEASURE
			if (tooth_picked_info_[current_operator_tooth_fdi_]->feature_points.size() == 2)
			{
				Point3m viewDirect, cameraPos;
				QString feature_name_a = PFusionAlignData->getToothFeatureConfig().getMarkName(fdi.toInt(), fdi_feature_points_names_[fdi][0]);
				QString feature_name_b = PFusionAlignData->getToothFeatureConfig().getMarkName(fdi.toInt(), fdi_feature_points_names_[fdi][1]);
				cur_tooth_width_tool_ = getToothWidthToolWith(tooth_picked_info_[current_operator_tooth_fdi_]->label_fdi,
					&(tooth_picked_info_[current_operator_tooth_fdi_]->feature_points[feature_name_a]),
					&(tooth_picked_info_[current_operator_tooth_fdi_]->feature_points[feature_name_b]));
				if (cur_tooth_width_tool_ == nullptr)
				{
					UtilityTools::getInstance()->getCurrentCameraState(&m, parent->width(), parent->height(), viewDirect, cameraPos);
					tooth_width_tool_list_.push_back(ToothWidthTool(
						tooth_picked_info_[current_operator_tooth_fdi_]->label_fdi,
						&(tooth_picked_info_[current_operator_tooth_fdi_]->feature_points[feature_name_a]),
						&(tooth_picked_info_[current_operator_tooth_fdi_]->feature_points[feature_name_b]),
						viewDirect));
					cur_tooth_width_tool_ = &tooth_width_tool_list_.back();
				}
			}
			else
			{
				cur_tooth_width_tool_ = nullptr;
			}

			if (tooth_picked_info_[current_operator_tooth_fdi_]->feature_points.size() == 3)
			{
				QString feature_name_a = PFusionAlignData->getToothFeatureConfig().getMarkName(fdi.toInt(), fdi_feature_points_names_[fdi][0]);
				QString feature_name_b = PFusionAlignData->getToothFeatureConfig().getMarkName(fdi.toInt(), fdi_feature_points_names_[fdi][1]);
				ToothWidthTool* p_tool = getToothWidthToolWith(tooth_picked_info_[current_operator_tooth_fdi_]->label_fdi,
					&(tooth_picked_info_[current_operator_tooth_fdi_]->feature_points[feature_name_a]),
					&(tooth_picked_info_[current_operator_tooth_fdi_]->feature_points[feature_name_b]));

				if (p_tool != nullptr)
				{
					p_tool->updateVisibleDirectWithThirdNode(p);
				}
			}
#endif

			tellProgramCurrentToothFdiWorking();
			is_modify_feature_points_ = true;

			// Record
			FeaturePointData feature_point_record(fdi, std::make_pair(feature_name, p));
			featurePointsRecord(feature_point_record);

		}
		else
		{
			UiUtilityTools::getInstance()->showInfoMessageBox(tr("Info"), tr("All current dental feature points have been marked."));
			cur_tooth_width_tool_ = nullptr;
		}
	}
}

void EditModelMarkPlugin::featurePointsPickedEvent(MeshModel& m, GLArea* gla)
{

	for (int index_fdi = 0; index_fdi < tooth_picked_info_.size(); ++index_fdi)
	{
		auto it = tooth_picked_info_[index_fdi];

		if (it->is_existed)
		{
			std::vector<QString> control_points_names;
			std::vector<Point3f> control_points, control_judge_vecs;
			std::vector<bool> control_cull_faces, interacive_list;

			/*for (auto& feature_point : it->feature_points)
			{
				control_points_names.push_back(feature_point.first);
				control_points.push_back(feature_point.second);
				control_judge_vecs.push_back(Point3m(0, 0, 0));
			}*/

			bool cur_visible = getToothWidthToolVisibleWith(it->label_fdi);

			bool hide_the_first_two = false;
			if (it->feature_points.size() >= 2)
			{
				hide_the_first_two = true;
			}
			for (auto& feature_point : it->feature_points)
			{
				control_points_names.push_back(feature_point.first);

				if (!hide_the_first_two)
				{
					control_points.push_back(feature_point.second);
					control_judge_vecs.push_back(Point3m(0, 0, 0));
					control_cull_faces.push_back(false);
					interacive_list.push_back(true);
				}
				else
				{
					int idx = PFusionAlignData->getToothFeatureConfig().getIndex(it->label_fdi.toInt(), feature_point.first);
					if (idx == 3)
					{
						control_points.push_back(feature_point.second);
						control_judge_vecs.push_back(Point3m(0, 0, 0));
						control_cull_faces.push_back(false);
						interacive_list.push_back(true);
					}
					else if (idx == 2)
					{
						Point3m oppe_node, adj_vec(0, 0, 0);
						for (auto& sub_point : it->feature_points)
						{
							idx = PFusionAlignData->getToothFeatureConfig().getIndex(it->label_fdi.toInt(), sub_point.first);
							if (idx == 1)
							{
								oppe_node = sub_point.second;
								adj_vec = -(oppe_node - feature_point.second).Normalize();
								break;
							}
						}
						control_points.push_back(feature_point.second);
						control_judge_vecs.push_back(adj_vec);
						control_cull_faces.push_back(true);
						interacive_list.push_back(cur_visible);
					}
					else if (idx == 1)
					{
						Point3m oppe_node, adj_vec(0, 0, 0);
						for (auto& sub_point : it->feature_points)
						{
							idx = PFusionAlignData->getToothFeatureConfig().getIndex(it->label_fdi.toInt(), sub_point.first);
							if (idx == 2)
							{
								oppe_node = sub_point.second;
								adj_vec = -(oppe_node - feature_point.second).Normalize();
								break;
							}
						}
						control_points.push_back(feature_point.second);
						control_judge_vecs.push_back(adj_vec);
						control_cull_faces.push_back(true);
						interacive_list.push_back(cur_visible);
					}
				}
			}

			if (!control_points.empty())
			{
				float radiusControlGeneral = 0.3f;
				int indexControlPicked = -1;

				bool is_picked = false;

				is_picked = UtilityTools::getInstance()->pickHandle(
					QTLogicalToDevice(gla, currentScreenPoint.x()),
					QTLogicalToDevice(gla, gla->height() - currentScreenPoint.y()),
					indexControlPicked, control_points, control_judge_vecs, control_cull_faces, radiusControlGeneral * 3);

				if (is_picked && interacive_list[indexControlPicked])
				{
					segmented_data_->mark_point_modifyed_ = true;

					is_picked_ = true;
					is_picking_ = false;

					current_operator_tooth_fdi_ = index_fdi;

					if (current_operator_tooth_fdi_ >= 0 && current_operator_tooth_fdi_ < tooth_picked_info_.size())
					{
						emit currentWorkingToothSignal(tooth_picked_info_[current_operator_tooth_fdi_]->label_fdi);
					}

					feature_point_name_picked_ = control_points_names[indexControlPicked];

					updateHintCurrentFdiFeatureNames();

					gla->bWhetherOpenGlobalTrackball = false;

					feature_point_last_.setData(tooth_picked_info_[current_operator_tooth_fdi_]->label_fdi,
						std::make_pair(feature_point_name_picked_, control_points[indexControlPicked]));

					break;
				}

			}
		}
	}

}

bool EditModelMarkPlugin::featurePointsMouseHoveringEvent(MeshModel& m, GLArea* gla)
{
#if TOOTH_WIDTH_MEASURE
	for (auto& tool : this->tooth_width_tool_list_)
	{
		tool.setHoveringNode(nullptr);
	}
#endif

	for (int index_fdi = 0; index_fdi < tooth_picked_info_.size(); ++index_fdi)
	{
		auto it = tooth_picked_info_[index_fdi];

		if (it->is_existed)
		{
			std::vector<QString> control_points_names;
			std::vector<Point3f> control_points, control_judge_vecs;
			std::vector<bool> control_cull_faces;

			/*for (auto& feature_point : it->feature_points)
			{
				control_points_names.push_back(feature_point.first);
				control_points.push_back(feature_point.second);
				control_judge_vecs.push_back(Point3m(0, 0, 0));
			}*/

			bool hide_the_first_two = false;
			if (it->feature_points.size() >= 2)
			{
				hide_the_first_two = true;
			}
			for (auto& feature_point : it->feature_points)
			{
				control_points_names.push_back(feature_point.first);

				if (!hide_the_first_two)
				{
					control_points.push_back(feature_point.second);
					control_judge_vecs.push_back(Point3m(0, 0, 0));
					control_cull_faces.push_back(false);
				}
				else
				{
					int idx = PFusionAlignData->getToothFeatureConfig().getIndex(it->label_fdi.toInt(), feature_point.first);
					if (idx == 3)
					{
						control_points.push_back(feature_point.second);
						control_judge_vecs.push_back(Point3m(0, 0, 0));
						control_cull_faces.push_back(false);
					}
					else if (idx == 2)
					{
						Point3m oppe_node, adj_vec(0, 0, 0);
						for (auto& sub_point : it->feature_points)
						{
							idx = PFusionAlignData->getToothFeatureConfig().getIndex(it->label_fdi.toInt(), sub_point.first);
							if (idx == 1)
							{
								oppe_node = sub_point.second;
								adj_vec = -(oppe_node - feature_point.second).Normalize();
								break;
							}
						}
						control_points.push_back(feature_point.second);
						control_judge_vecs.push_back(adj_vec);
						control_cull_faces.push_back(true);
					}
					else if (idx == 1)
					{
						Point3m oppe_node, adj_vec(0, 0, 0);
						for (auto& sub_point : it->feature_points)
						{
							idx = PFusionAlignData->getToothFeatureConfig().getIndex(it->label_fdi.toInt(), sub_point.first);
							if (idx == 2)
							{
								oppe_node = sub_point.second;
								adj_vec = -(oppe_node - feature_point.second).Normalize();
								break;
							}
						}
						control_points.push_back(feature_point.second);
						control_judge_vecs.push_back(adj_vec);
						control_cull_faces.push_back(true);
					}
				}
			}

			if (!control_points.empty())
			{
				float radiusControlGeneral = 0.3f;
				int indexControlPicked = -1;

				bool is_picked = false;

				is_picked = UtilityTools::getInstance()->pickHandle(
					QTLogicalToDevice(gla, currentScreenPoint.x()),
					QTLogicalToDevice(gla, gla->height() - currentScreenPoint.y()),
					indexControlPicked, control_points, control_judge_vecs, control_cull_faces, radiusControlGeneral * 3);

				if (is_picked)
				{
					// 					current_operator_tooth_fdi_ = index_fdi;
					//
					// 					if (current_operator_tooth_fdi_ >= 0 && current_operator_tooth_fdi_ < tooth_picked_info_.size())
					// 					{
					// 						emit currentWorkingToothSignal(tooth_picked_info_[current_operator_tooth_fdi_]->label_fdi);
					// 					}

					fdi_mouse_hovering_get_feature_point_ = index_fdi;
					feature_point_name_picked_ = control_points_names[indexControlPicked];

					ToothWidthTool* p_tool = getToothWidthToolWith(it->label_fdi, nullptr, nullptr);
					if (p_tool)
					{
						p_tool->setHoveringNode(&tooth_picked_info_[index_fdi]->feature_points.at(feature_point_name_picked_));
					}

					return true;
				}

			}
		}
	}

	return false;
}

void EditModelMarkPlugin::featurePointsMovingEvent(MeshModel& m, GLArea* gla)
{
	Point3d pointPicked = UtilityTools::getInstance()->transformPointFrom2DscreenTo3Dworld(
		QTLogicalToDevice(gla, currentScreenPoint.x()),
		QTLogicalToDevice(gla, gla->height() - currentScreenPoint.y()), &m.cm);

	int vIdPicked = UtilityTools::getInstance()->getIndexPointClosestMesh(m, Point3m(pointPicked.X(), pointPicked.Y(), pointPicked.Z()));

	if (vIdPicked < 0)
	{
		return;
	}

	Point3f p(pointPicked.X(), pointPicked.Y(), pointPicked.Z());

	if (current_operator_tooth_fdi_ >= 0 && current_operator_tooth_fdi_ < tooth_picked_info_.size() && !feature_point_name_picked_.isEmpty())
	{
		is_modify_feature_points_ = true;
		tooth_picked_info_[current_operator_tooth_fdi_]->feature_points[feature_point_name_picked_] = p;

		feature_point_current_.setData(tooth_picked_info_[current_operator_tooth_fdi_]->label_fdi, std::make_pair(feature_point_name_picked_, p));
	}

}

void EditModelMarkPlugin::drawFeaturePoints(MeshModel& m, GLArea* parent, QPainter* p)
{

	float radiusControlFeaturesPoints = 0.2f;
	int slices = 16;
	int stacks = 16;

	glPushAttrib(GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	// Draw feature point spheres
	int iToothPicked = -1;
	for (auto& tooth : tooth_picked_info_)
	{
		++iToothPicked;
		if (tooth->is_existed && !tooth->feature_points.empty())
		{

			int id_color = 0;
			bool hide_the_first_two = false;
#if TOOTH_WIDTH_MEASURE
			if (tooth->feature_points.size() >= 2)
			{
				hide_the_first_two = true;
			}
#endif

			for (auto& it : tooth->feature_points)
			{
				int idx = PFusionAlignData->getToothFeatureConfig().getIndex(tooth->label_fdi.toInt(), it.first);
				Point3i color = color_feature[(idx) % COLOR_NUM];
				if (idx != -1 && (!hide_the_first_two || idx >= 3))
				{
					glEnable(GL_LIGHTING);
					glColor3f(color.X() / 255.0f, color.Y() / 255.0f, color.Z() / 255.0f);
					vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(it.second, radiusControlFeaturesPoints, slices, stacks);
				}
			}

			if ((fdi_mouse_hovering_get_feature_point_ == iToothPicked) || ((current_operator_tooth_fdi_ == iToothPicked) && is_picked_))
			{
				// Draw mouse hover selected point, make it uppercase
				if (!feature_point_name_picked_.isEmpty() && tooth->feature_points.find(feature_point_name_picked_) != tooth->feature_points.end())
				{
					int id_color = 0;
					int idx = PFusionAlignData->getToothFeatureConfig().getIndex(tooth->label_fdi.toInt(), feature_point_name_picked_);
					if (idx != -1 && (!hide_the_first_two || idx >= 3))
					{
						Point3i color = color_feature[(idx) % COLOR_NUM];
						glEnable(GL_LIGHTING);
						glColor3f(color.X() / 255.0f, color.Y() / 255.0f, color.Z() / 255.0f);
						vcg::Add_Ons::glPoint<vcg::Add_Ons::DMSolid>(tooth->feature_points.at(feature_point_name_picked_), radiusControlFeaturesPoints * 2, slices, stacks);
					}
				}
			}
		}
	}

	int size_font = 25;
	if (parent)
	{
		size_font = 700 * parent->trackball.track.sca;
	}

	size_font = std::max(10, size_font);
	size_font = std::min(size_font, 40);

	vcg::glLabel::Mode pen;
	pen.qFont.setPixelSize(size_font);
	pen.qFont.setFamily("Microsoft YaHei UI");
	pen.qFont.setStyleStrategy(QFont::PreferAntialias);

	// Draw feature point text description, current tooth position
	iToothPicked = -1;
	for (auto& tooth : tooth_picked_info_)
	{
		++iToothPicked;

		if (tooth->is_existed && !tooth->feature_points.empty())
		{

			if (current_operator_tooth_fdi_ == iToothPicked)
			{

				for (auto& it : tooth->feature_points)
				{
					int idx = PFusionAlignData->getToothFeatureConfig().getIndex(tooth->label_fdi.toInt(), it.first);
					if (idx != -1)
					{
						Point3i color = color_feature[(idx) % COLOR_NUM];
						pen.color = vcg::Color4b(color.X(), color.Y(), color.Z(), 255);

						vcg::glLabel::render(p, it.second, QString("  ") + QString::number(idx), pen);
					}
				}

				// 				int num_features = tooth->feature_points.size();
				// 				if (num_features >= 2)
				// 				{
				// 					Point3m average_point(0, 0, 0);
				// 					for (auto v : tooth->feature_points)
				// 					{
				// 						average_point += v.second;
				// 					}
				//
				// 					average_point /= num_features;
				//
				// 					if (num_features > 2)
				// 					{
				// 						average_point += Point3m(0.5, 0.5, 0.5);
				// 					}
				//
				// 					Point3i color = color_feature[0];
				// 					pen.color = vcg::Color4b(color.X(), color.Y(), color.Z(), 255);
				//
				// 					vcg::glLabel::render(p, average_point, QString(" FDI:") + tooth->label_fdi, pen);
				// 				}

			}

		}

	}
	glPopAttrib();
}

void EditModelMarkPlugin::draw2DLabel(GLArea* gla, QPainter* painter)
{
	if (!gla)
	{
		return;
	}

	int font_size = 20;
	int height_text_each_row = font_size * 1.5;

	//static vcg::Color4b color_pen[15]; = { vcg::Color4b(color_feature[0].X(),color_feature[0].Y(),color_feature[0].Z(),255), vcg::Color4b(color_feature[1].X(),color_feature[1].Y(),color_feature[1].Z(),255),
	//	vcg::Color4b(color_feature[2].X(),color_feature[2].Y(),color_feature[2].Z(),255),vcg::Color4b(color_feature[3].X(),color_feature[3].Y(),color_feature[3].Z(),255) };

	vcg::glLabel::Mode pen;
	pen.qFont.setFamily("Microsoft YaHei UI");
	pen.qFont.setPixelSize(20);
	pen.qFont.setStyleStrategy(QFont::PreferAntialias);

	glPushAttrib(GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	painter->save();
	painter->endNativePainting();

	QPen textPen(QColor(255, 255, 255, 255));
	textPen.setWidthF(0.2f);
	painter->setPen(textPen);
	painter->setFont(pen.qFont);

	int num_row = hint_fdi_feature_names_.size();
	if (num_row > 0)
	{
		int start_x = 30;
		int start_y = parent->height() - height_text_each_row * (num_row + 1);
		int y = parent->height() - height_text_each_row;
		// Draw feature point information text hint for currently operating tooth position at bottom left corner -> changed to top right -> changed back to bottom left -> color changed from colored to white 2022.5.9
		for (int iRow = 0; iRow < num_row; ++iRow)
		{
			QString hint = *hint_fdi_feature_names_[iRow];
			int idx = hint.left(hint.indexOf(":")).toInt();

			//vcg::Color4b penColor(color_feature[idx % COLOR_NUM].X(), color_feature[idx % COLOR_NUM].Y(), color_feature[idx % COLOR_NUM].Z(), 255);
			//pen.color = color_pen[iRow % 4];
			//pen.color = penColor;

			//painter->setPen(QColor(color_feature[idx % COLOR_NUM].X(), color_feature[idx % COLOR_NUM].Y(), color_feature[idx % COLOR_NUM].Z(), 255));
			//			painter->drawText(QPoint(start_x, start_y + iRow * height_text_each_row), *hint_fdi_feature_names_[iRow]);
			if (iRow == 0)
			{
				painter->drawText(QPoint(start_x, parent->height() - 2 * height_text_each_row), *hint_fdi_feature_names_[iRow]);
				continue;
			}
			if (PFusionAppData->getAppLanguage() == E_CHINESE)
			{
				painter->drawText(QPoint(start_x + (iRow - 1) * 170, y), *hint_fdi_feature_names_[iRow]);
			}
			else if (PFusionAppData->getAppLanguage() == E_ENGLISH)
			{
				painter->drawText(QPoint(start_x + (iRow - 1) * 250, y), *hint_fdi_feature_names_[iRow]);
			}

			//vcg::glLabel::render2D(painter, Point2f(start_x, start_y - iRow * height_text_each_row), *hint_fdi_feature_names_[iRow], pen);
		}

		if (hint_feature_name_ && index_feature_mouse >= 0)
		{
			// Draw mouse hint content

			QString hint = *hint_feature_name_;
			int fdi = hint.left(hint.indexOf(" ")).toInt();
			QString featureName = hint.right(hint.size() - hint.indexOf(" ") - 2);
			int idx = PFusionAlignData->getToothFeatureConfig().getIndex(fdi, featureName);
			//vcg::Color4b penColor(color_feature[(index_feature_mouse + 1) % COLOR_NUM].X(), color_feature[(index_feature_mouse + 1) % COLOR_NUM].Y(), color_feature[(index_feature_mouse + 1) % COLOR_NUM].Z(), 255);
			if (idx != -1)
			{
				//vcg::Color4b penColor(color_feature[idx % COLOR_NUM].X(), color_feature[idx % COLOR_NUM].Y(), color_feature[idx % COLOR_NUM].Z(), 255);
				//pen.color = penColor;
				//vcg::glLabel::render2D(painter, Point2f(currentScreenPoint.x(), gla->height() - currentScreenPoint.y()), *hint_feature_name_, pen);

				int cursor_x = currentScreenPoint.x() + 50;
				int cursor_y = currentScreenPoint.y();

				QFontMetrics fm(pen.qFont);
				QRect rect = fm.boundingRect(*hint_feature_name_);
				int left_margin = -3;
				int right_margin = 3;
				int top_margin = 8;
				int bottom_margin = 6;
				QPoint topleft(cursor_x + left_margin, cursor_y - rect.height() + top_margin);
				QPoint bottomright(cursor_x + rect.width() + right_margin, cursor_y + bottom_margin);
				QRect background_rect(topleft, bottomright);
				painter->setPen(Qt::PenStyle::NoPen);
				//painter->setBrush(QBrush(QColor(255 - color_feature[idx % COLOR_NUM].X(), 255 - color_feature[idx % COLOR_NUM].Y(), 255 - color_feature[idx % COLOR_NUM].Z(), 200)));
				painter->setBrush(QBrush(QColor(255, 255, 255, 150)));
				painter->drawRoundedRect(background_rect, 5, 5);

				//painter->setPen(QColor(color_feature[idx % COLOR_NUM].X(), color_feature[idx % COLOR_NUM].Y(), color_feature[idx % COLOR_NUM].Z(), 255));
				painter->setPen(QColor(0, 0, 0, 255));
				painter->drawText(QPoint(cursor_x, cursor_y), *hint_feature_name_);
			}
		}
	}

	painter->beginNativePainting();
	painter->restore();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}

void EditModelMarkPlugin::drawCrownBorderCurve()
{

	vcg::Color4f clrControlSpline = vcg::Color4f::LightBlue;
	vcg::Color4f clrControlSplineInterval = vcg::Color4f::Blue;

	glPushAttrib(GL_ENABLE_BIT | GL_VIEWPORT_BIT);
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(3.0f);

	int iBorder = 0;
	int interval = 20;
	for (auto& tooth : tooth_picked_info_)
	{
		if (tooth->is_existed && !tooth->border_info.hadSmoothedVertexsPosInfo.empty())
		{
			if (iBorder++ % 2 == 0)
			{
				glColor3f(clrControlSpline.X(), clrControlSpline.Y(),
					clrControlSpline.Z());
			}
			else {
				glColor3f(clrControlSplineInterval.X(), clrControlSplineInterval.Y(),
					clrControlSplineInterval.Z());
			}

			int numVertexs = tooth->border_info.hadSmoothedVertexsPosInfo.size();

			glBegin(GL_LINE_STRIP);
			for (int iVertex = 0; iVertex + interval < numVertexs; iVertex += interval)
			{
				vcg::glVertex(tooth->border_info.hadSmoothedVertexsPosInfo[iVertex]);
			}
			glEnd();

		}

	}

	glEnable(GL_LIGHTING);
	glLineWidth(1.0f);
	glPopAttrib();
}

void EditModelMarkPlugin::updateHintCurrentFdiFeatureNames()
{
	std::vector<std::shared_ptr<QString>>().swap(hint_fdi_feature_names_);

	if (current_operator_tooth_fdi_ >= 0 && current_operator_tooth_fdi_ < tooth_picked_info_.size())
	{
		auto it = tooth_picked_info_[current_operator_tooth_fdi_];
		if (it->is_existed)
		{

			int index_feature_name = it->feature_points.size();
			QString fdi = it->label_fdi;

			if (index_feature_name >= 0 && index_feature_name < fdi_feature_points_names_[fdi].size())
			{
				hint_feature_name_ = std::make_shared<QString>(it->label_fdi + "  " + fdi_feature_points_names_[fdi][index_feature_name]);
				index_feature_mouse = index_feature_name;
			}
			else
			{
				index_feature_mouse = -1;
				hint_feature_name_ = std::make_shared<QString>();
			}

			hint_fdi_feature_names_.push_back(std::make_shared<QString>(QString("FDI: ") + fdi));
			for (auto& feature_name : fdi_feature_points_names_[it->label_fdi])
			{
				QString featureIndex = QString::number(PFusionAlignData->getToothFeatureConfig().getIndex(fdi.toInt(), feature_name));
				hint_fdi_feature_names_.push_back(std::make_shared<QString>(featureIndex + " : " + feature_name));
			}

#if TOOTH_WIDTH_MEASURE
			if (it->feature_points.size() >= 2)
			{
				Point3m viewDirect, cameraPos;
				QString feature_name_a = PFusionAlignData->getToothFeatureConfig().getMarkName(fdi.toInt(), fdi_feature_points_names_[fdi][0]);
				QString feature_name_b = PFusionAlignData->getToothFeatureConfig().getMarkName(fdi.toInt(), fdi_feature_points_names_[fdi][1]);
				cur_tooth_width_tool_ = getToothWidthToolWith(fdi, &(it->feature_points[feature_name_a]), &(it->feature_points[feature_name_b]));
				if (cur_tooth_width_tool_ == nullptr)
				{
					UtilityTools::getInstance()->getCurrentCameraState(nullptr, parent->width(), parent->height(), viewDirect, cameraPos);
					tooth_width_tool_list_.push_back(ToothWidthTool(
						tooth_picked_info_[current_operator_tooth_fdi_]->label_fdi,
						&(it->feature_points[feature_name_a]),
						&(it->feature_points[feature_name_b]),
						viewDirect));
					cur_tooth_width_tool_ = &tooth_width_tool_list_.back();
				}

				if (it->feature_points.size() == 3)
				{
					QString feature_name_c = PFusionAlignData->getToothFeatureConfig().getMarkName(fdi.toInt(), fdi_feature_points_names_[fdi][2]);
					ToothWidthTool* p_tool = getToothWidthToolWith(fdi, &(it->feature_points[feature_name_a]), &(it->feature_points[feature_name_b]));
					if (p_tool != nullptr)
					{
						p_tool->updateVisibleDirectWithThirdNode(it->feature_points[feature_name_c]);

						Point3m viewDirect, cameraPos;
						UtilityTools::getInstance()->getCurrentCameraState(nullptr, parent->width(), parent->height(), viewDirect, cameraPos);
						p_tool->updateVisible(viewDirect);
					}
				}
			}
			else
			{
				cur_tooth_width_tool_ = nullptr;
			}
#endif
		}
	}
}

ToothWidthTool* EditModelMarkPlugin::getToothWidthToolWith(QString _fdi, Point3f* _p_node_a, Point3f* _p_node_b)
{
#if TOOTH_WIDTH_MEASURE
	if (_p_node_a == nullptr && _p_node_b == nullptr)
	{
		for (auto& tool : this->tooth_width_tool_list_)
		{
			if (tool.fdi_ == _fdi)
			{
				return &tool;
			}
		}
		return nullptr;
	}

	for (auto& tool : this->tooth_width_tool_list_)
	{
		if (tool.fdi_ == _fdi)
		{
			if (tool.node_a_ != _p_node_a || tool.node_b_ != _p_node_b)
			{
				deleteToothWidthToolWith(_fdi);
				return nullptr;
			}
			else
			{
				tool.need_update_cut_outline_ = true;
				return &tool;
			}
		}
	}
#endif
	return nullptr;
}

bool EditModelMarkPlugin::getToothWidthToolVisibleWith(QString _fdi)
{
#if TOOTH_WIDTH_MEASURE
	for (auto& tool : this->tooth_width_tool_list_)
	{
		if (tool.fdi_ == _fdi)
		{
			return tool.visible_;
		}
	}
#endif
	return true;
}

bool EditModelMarkPlugin::deleteToothWidthToolWith(QString _fdi)
{
#if TOOTH_WIDTH_MEASURE
	if (tooth_width_tool_list_.empty())
	{
		return false;
	}

	auto ite = tooth_width_tool_list_.begin();
	while (ite != tooth_width_tool_list_.end())
	{
		if (ite->fdi_ == _fdi)
		{
			if (cur_tooth_width_tool_ != nullptr && cur_tooth_width_tool_->fdi_ == _fdi)
			{
				cur_tooth_width_tool_ = nullptr;
			}
			tooth_width_tool_list_.erase(ite);
			return true;
		}
		else
		{
			++ite;
		}
	}
#endif
	return false;
}

void EditModelMarkPlugin::featurePointsRecord(const FeaturePointData& data_record)
{
	if (command_manager_ == nullptr)
	{
		return;
	}

	CommandCommon<FeaturePointData>* dataCommand = new CommandCommon<FeaturePointData>(MODEL_MARK_POINTS_PICKED_COMMAND, data_record);

	CommandCommon<FeaturePointData>* copyDataCommand = new CommandCommon<FeaturePointData>(MODEL_MARK_POINTS_PICKED_COMMAND, data_record);

	command_manager_->addCommand(std::make_pair(dataCommand, copyDataCommand));

	PSIGNALMANAGER->canUndoOrRedoStatusSignal(command_manager_->canUndo(), command_manager_->canRedo());
}

void EditModelMarkPlugin::featurePointsModifyRecord(const FeaturePointData& last_data_record, const FeaturePointData& data_record)
{
	if (command_manager_ == nullptr)
	{
		return;
	}

	CommandCommon<FeaturePointData>* dataCommand = new CommandCommon<FeaturePointData>(MODEL_MARK_POINT_MODIFY_COMMAND, data_record);

	CommandCommon<FeaturePointData>* copyDataCommand = new CommandCommon<FeaturePointData>(MODEL_MARK_POINT_MODIFY_COMMAND, last_data_record);

	command_manager_->addCommand(std::make_pair(dataCommand, copyDataCommand));

	PSIGNALMANAGER->canUndoOrRedoStatusSignal(command_manager_->canUndo(), command_manager_->canRedo());
}

void EditModelMarkPlugin::featurePointsRecordUndo(const FeaturePointData& data_record)
{
	int index_tooth = getIndexToothFromFdi(data_record.label_fdi);

	if (index_tooth >= 0)
	{

		auto& it = tooth_picked_info_[index_tooth]->feature_points;
		if (!it.empty())
		{
			it.erase(data_record.feature_point.first);
		}

		if (it.size() < 2)
		{
			deleteToothWidthToolWith(data_record.label_fdi);
		}

		if (it.empty())
		{
			tellProgramCurrentToothFdiWorking();
		}
		else if (it.size() < 3)
		{
			tellProgramCurrentToothFdiWorking(data_record.label_fdi);
		}
		else if (it.size() >= 3)
		{
			tellProgramCurrentToothFdiWorking();
		}
	}
}

void EditModelMarkPlugin::featurePointsRecordRedo(const FeaturePointData& data_record)
{
	int index_tooth = getIndexToothFromFdi(data_record.label_fdi);
	QString fdi = tooth_picked_info_[index_tooth]->label_fdi;

	if (index_tooth >= 0)
	{
		auto& it = tooth_picked_info_[index_tooth]->feature_points;
		it[data_record.feature_point.first] = data_record.feature_point.second;

#if TOOTH_WIDTH_MEASURE
		// Update tooth width tool
		{
			if (tooth_picked_info_[index_tooth]->feature_points.size() >= 2)
			{
				Point3m viewDirect, cameraPos;
				QString feature_name_a = PFusionAlignData->getToothFeatureConfig().getMarkName(fdi.toInt(), fdi_feature_points_names_[fdi][0]);
				QString feature_name_b = PFusionAlignData->getToothFeatureConfig().getMarkName(fdi.toInt(), fdi_feature_points_names_[fdi][1]);
				cur_tooth_width_tool_ = getToothWidthToolWith(tooth_picked_info_[index_tooth]->label_fdi,
					&(tooth_picked_info_[index_tooth]->feature_points[feature_name_a]),
					&(tooth_picked_info_[index_tooth]->feature_points[feature_name_b]));
				if (cur_tooth_width_tool_ == nullptr)
				{
					UtilityTools::getInstance()->getCurrentCameraState(nullptr, parent->width(), parent->height(), viewDirect, cameraPos);
					tooth_width_tool_list_.push_back(ToothWidthTool(
						tooth_picked_info_[index_tooth]->label_fdi,
						&(tooth_picked_info_[index_tooth]->feature_points[feature_name_a]),
						&(tooth_picked_info_[index_tooth]->feature_points[feature_name_b]),
						viewDirect));
					cur_tooth_width_tool_ = &tooth_width_tool_list_.back();
				}
			}
			else
			{
				cur_tooth_width_tool_ = nullptr;
			}

			if (cur_tooth_width_tool_ != nullptr)
			{
				Point3m viewDirect, cameraPos;
				UtilityTools::getInstance()->getCurrentCameraState(nullptr, parent->width(), parent->height(), viewDirect, cameraPos);
				cur_tooth_width_tool_->update(viewDirect);
			}

			if (cur_tooth_width_tool_ != nullptr && tooth_picked_info_[index_tooth]->feature_points.size() == 3)
			{
				QString feature_name_c = PFusionAlignData->getToothFeatureConfig().getMarkName(fdi.toInt(), fdi_feature_points_names_[fdi][2]);
				cur_tooth_width_tool_->updateVisibleDirectWithThirdNode(tooth_picked_info_[index_tooth]->feature_points[feature_name_c]);
			}
		}
#endif

		if (it.empty())
		{
			tellProgramCurrentToothFdiWorking();
		}
		else if (it.size() < 3)
		{
			tellProgramCurrentToothFdiWorking(data_record.label_fdi);
		}
		else if (it.size() >= 3)
		{
			tellProgramCurrentToothFdiWorking();
		}
	}
	parent->update();
}

void EditModelMarkPlugin::featurePointsModifyRecordUndo(const FeaturePointData& data_record)
{
	int index_tooth = getIndexToothFromFdi(data_record.label_fdi);
	QString fdi = tooth_picked_info_[index_tooth]->label_fdi;

	if (index_tooth >= 0)
	{
		auto& it = tooth_picked_info_[index_tooth]->feature_points;
		if (!it.empty())
		{
			QString name_feature_point = data_record.feature_point.first;
			if (it.find(name_feature_point) != it.end())
			{
				it[name_feature_point] = data_record.feature_point.second;

#if TOOTH_WIDTH_MEASURE
				if (tooth_picked_info_[index_tooth]->feature_points.size() >= 2)
				{
					Point3m viewDirect, cameraPos;
					QString feature_name_a = PFusionAlignData->getToothFeatureConfig().getMarkName(fdi.toInt(), fdi_feature_points_names_[fdi][0]);
					QString feature_name_b = PFusionAlignData->getToothFeatureConfig().getMarkName(fdi.toInt(), fdi_feature_points_names_[fdi][1]);
					cur_tooth_width_tool_ = getToothWidthToolWith(tooth_picked_info_[index_tooth]->label_fdi,
						&(tooth_picked_info_[index_tooth]->feature_points[feature_name_a]),
						&(tooth_picked_info_[index_tooth]->feature_points[feature_name_b]));
					if (cur_tooth_width_tool_ == nullptr)
					{
						UtilityTools::getInstance()->getCurrentCameraState(nullptr, parent->width(), parent->height(), viewDirect, cameraPos);
						tooth_width_tool_list_.push_back(ToothWidthTool(
							tooth_picked_info_[index_tooth]->label_fdi,
							&(tooth_picked_info_[index_tooth]->feature_points[feature_name_a]),
							&(tooth_picked_info_[index_tooth]->feature_points[feature_name_b]),
							viewDirect));
						cur_tooth_width_tool_ = &tooth_width_tool_list_.back();
					}
				}
				else
				{
					cur_tooth_width_tool_ = nullptr;
				}

				if (cur_tooth_width_tool_ != nullptr)
				{
					Point3m viewDirect, cameraPos;
					UtilityTools::getInstance()->getCurrentCameraState(nullptr, parent->width(), parent->height(), viewDirect, cameraPos);
					cur_tooth_width_tool_->update(viewDirect);
				}

				if (cur_tooth_width_tool_ != nullptr && tooth_picked_info_[index_tooth]->feature_points.size() == 3)
				{
					QString feature_name_c = PFusionAlignData->getToothFeatureConfig().getMarkName(fdi.toInt(), fdi_feature_points_names_[fdi][2]);
					cur_tooth_width_tool_->updateVisibleDirectWithThirdNode(tooth_picked_info_[index_tooth]->feature_points[feature_name_c]);
				}
#endif
			}
		}

		tellProgramCurrentToothFdiWorking(data_record.label_fdi);
	}
}

void EditModelMarkPlugin::featurePointsClearRecord(const FeaturePointsMarking::ToothPickedInfomation& last_data_record)
{
	if (command_manager_ == nullptr)
	{
		return;
	}

	CommandCommon<FeaturePointsMarking::ToothPickedInfomation>* dataCommand = new CommandCommon<FeaturePointsMarking::ToothPickedInfomation>(MODEL_MARK_POINT_CLEAR_COMMAND, last_data_record);

	CommandCommon<FeaturePointsMarking::ToothPickedInfomation>* copyDataCommand = new CommandCommon<FeaturePointsMarking::ToothPickedInfomation>(MODEL_MARK_POINT_CLEAR_COMMAND, last_data_record);

	command_manager_->addCommand(std::make_pair(dataCommand, copyDataCommand));

	PSIGNALMANAGER->canUndoOrRedoStatusSignal(command_manager_->canUndo(), command_manager_->canRedo());
}

void EditModelMarkPlugin::featurePointsClearRecordUndo(const FeaturePointsMarking::ToothPickedInfomation& data_record)
{
	auto index_tooth = getIndexToothFromFdi(data_record.label_fdi);

	if (index_tooth >= 0)
	{

		SAFE_DELETE(tooth_picked_info_[index_tooth]);
		tooth_picked_info_[index_tooth] = new FeaturePointsMarking::ToothPickedInfomation(data_record);

		tellProgramCurrentToothFdiWorking();

	}
}

void EditModelMarkPlugin::featurePointsClearRecordRedo(const FeaturePointsMarking::ToothPickedInfomation& data_record)
{
	auto index_tooth = getIndexToothFromFdi(data_record.label_fdi);

	if (index_tooth >= 0)
	{
		tooth_picked_info_[index_tooth]->clearFeaturePointsData();
		deleteToothWidthToolWith(data_record.label_fdi);
		tellProgramCurrentToothFdiWorking();

	}
}

void EditModelMarkPlugin::featurePointsAllClearRecord(const std::vector<FeaturePointsMarking::ToothPickedInfomation>& last_data_record)
{
	if (command_manager_ == nullptr)
	{
		return;
	}

	CommandCommon<std::vector<FeaturePointsMarking::ToothPickedInfomation>>* dataCommand =
		new CommandCommon<std::vector<FeaturePointsMarking::ToothPickedInfomation>>(MODEL_MARK_POINT_ALL_CLEAR_COMMAND, last_data_record);

	CommandCommon<std::vector<FeaturePointsMarking::ToothPickedInfomation>>* copyDataCommand =
		new CommandCommon<std::vector<FeaturePointsMarking::ToothPickedInfomation>>(MODEL_MARK_POINT_ALL_CLEAR_COMMAND, last_data_record);

	command_manager_->addCommand(std::make_pair(dataCommand, copyDataCommand));
	PSIGNALMANAGER->canUndoOrRedoStatusSignal(command_manager_->canUndo(), command_manager_->canRedo());
}

void EditModelMarkPlugin::featurePointsAllClearRecordUndo(const std::vector<FeaturePointsMarking::ToothPickedInfomation>& data_record)
{
	for (auto& it : data_record)
	{
		featurePointsClearRecordUndo(it);
	}
}

void EditModelMarkPlugin::featurePointsAllClearRecordRedo(const std::vector<FeaturePointsMarking::ToothPickedInfomation>& data_record)
{
	for (auto& info : tooth_picked_info_)
	{
		info->clearFeaturePointsData();
	}

#if TOOTH_WIDTH_MEASURE
	tooth_width_tool_list_.clear();
	cur_tooth_width_tool_ = nullptr;
#endif

	current_operator_tooth_fdi_ = -1;
	tellProgramCurrentToothFdiWorking();
}

void EditModelMarkPlugin::clearCommand()
{
	if (command_manager_)
	{
		command_manager_->clearAllCommands();
	}
	PSIGNALMANAGER->canUndoOrRedoStatusSignal(command_manager_->canUndo(), command_manager_->canRedo());
}

void EditModelMarkPlugin::unDo()
{
	if (command_manager_ && !command_manager_->canUndo() && !task_end_)
	{
		return;
	}

	task_end_ = false;

	// Disable buttons before undo task to prevent performance issues causing task exception crash
	PSIGNALMANAGER->canUndoOrRedoStatusSignal(false, false);

	auto commandCombinew = command_manager_->unDo();
	auto newCommand = commandCombinew.getNewCommand();
	auto oldCommand = commandCombinew.getOldCommand();
	auto copyCommand = commandCombinew.getCopyCommand();

	bool modify_feature_points = false;
	// For deletion data, find current data
	if (copyCommand)
	{
		switch (copyCommand->getCommandType())
		{
		case MODEL_MARK_POINTS_PICKED_COMMAND:
		{
			auto feature_point = ((CommandCommon<FeaturePointData>*)copyCommand)->getData();
			SPDLOG->info("featurePointsRecordUndo begin");
			featurePointsRecordUndo(feature_point);
			modify_feature_points |= true;
		}
		break;
		case MODEL_MARK_POINT_MODIFY_COMMAND:
		{
			// Restore modification data to previous data
			auto feature_point = ((CommandCommon<FeaturePointData>*)copyCommand)->getData();
			SPDLOG->info("featurePointsModifyRecordUndo begin");
			featurePointsModifyRecordUndo(feature_point);
			modify_feature_points |= true;
		}
		break;
		case MODEL_MARK_POINT_CLEAR_COMMAND:
		{
			auto it = ((CommandCommon<FeaturePointsMarking::ToothPickedInfomation>*)copyCommand)->getData();
			SPDLOG->info("featurePointsClearRecordUndo begin");
			featurePointsClearRecordUndo(it);
			modify_feature_points |= true;
		}
		break;
		case MODEL_MARK_POINT_ALL_CLEAR_COMMAND:
		{
			auto it = ((CommandCommon<std::vector<FeaturePointsMarking::ToothPickedInfomation>>*)copyCommand)->getData();
			SPDLOG->info("featurePointsAllClearRecordUndo begin");
			featurePointsAllClearRecordUndo(it);
			modify_feature_points |= true;
		}
		break;

		default:
			break;
		}
	}

	if (modify_feature_points)
	{
		is_modify_feature_points_ = true;

		if (segmented_data_)
		{
			segmented_data_->mark_point_modifyed_ = true;
		}

		// Separation functionality removed - not used in FusionAnalyser
		// p_cur_mesh_ is no longer available, always emit markingDoneSignal
			emit PSIGNALMANAGER->markingDoneSignal(judgeFeaturePointsMarked());
		// }
		emit PSIGNALMANAGER->markingAnyOneDentalDoneSignal(judgeFeaturePointsMarked());
	}

	parent->update();

	PSIGNALMANAGER->canUndoOrRedoStatusSignal(command_manager_->canUndo(), command_manager_->canRedo());

	task_end_ = true;
	SPDLOG->info("Undo end");
}

void EditModelMarkPlugin::reDo()
{
	if (command_manager_ && !command_manager_->canRedo() && !task_end_)
	{
		return;
	}

	SPDLOG->info("reDo begin");
	task_end_ = false;

	// Disable buttons before undo task to prevent performance issues causing task exception crash
	PSIGNALMANAGER->canUndoOrRedoStatusSignal(false, false);

	auto commandCombinew = command_manager_->reDo();
	auto newCommand = commandCombinew.getNewCommand();

	bool modify_feature_points = false;
	// For deletion data, find current data
	if (newCommand)
	{
		switch (newCommand->getCommandType())
		{
		case MODEL_MARK_POINTS_PICKED_COMMAND:
		{
			auto feature_point = ((CommandCommon<FeaturePointData>*)newCommand)->getData();
			SPDLOG->info("featurePointsRecordRedo begin");
			featurePointsRecordRedo(feature_point);
			modify_feature_points |= true;
		}
		break;
		case MODEL_MARK_POINT_MODIFY_COMMAND:
		{
			// Restore modification data to previous data
			auto feature_point = ((CommandCommon<FeaturePointData>*)newCommand)->getData();
			SPDLOG->info("featurePointsModifyRecordRedo begin");
			featurePointsModifyRecordRedo(feature_point);
			modify_feature_points |= true;
		}
		break;
		case MODEL_MARK_POINT_CLEAR_COMMAND:
		{
			auto it = ((CommandCommon<FeaturePointsMarking::ToothPickedInfomation>*)newCommand)->getData();
			SPDLOG->info("featurePointsClearRecordRedo begin");
			featurePointsClearRecordRedo(it);
			modify_feature_points |= true;
		}
		break;
		case MODEL_MARK_POINT_ALL_CLEAR_COMMAND:
		{
			auto it = ((CommandCommon<std::vector<FeaturePointsMarking::ToothPickedInfomation>>*)newCommand)->getData();
			SPDLOG->info("featurePointsAllClearRecordRedo begin");
			featurePointsAllClearRecordRedo(it);
			modify_feature_points |= true;
		}
		break;

		default:
			break;
		}
	}

	if (modify_feature_points)
	{
		is_modify_feature_points_ = true;

		if (segmented_data_)
		{
			segmented_data_->mark_point_modifyed_ = true;
		}

		// Separation functionality removed - not used in FusionAnalyser
		// p_cur_mesh_ is no longer available, always emit markingDoneSignal
			emit PSIGNALMANAGER->markingDoneSignal(judgeFeaturePointsMarked());
		// }
		emit PSIGNALMANAGER->markingAnyOneDentalDoneSignal(judgeFeaturePointsMarked());
	}

	parent->update();

	PSIGNALMANAGER->canUndoOrRedoStatusSignal(command_manager_->canUndo(), command_manager_->canRedo());
	SPDLOG->info("reDo end");

	task_end_ = true;
}
