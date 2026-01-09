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

#ifndef EDITMODELMARKPLUGIN_H
#define EDITMODELMARKPLUGIN_H

#define TOOTH_WIDTH_MEASURE false

#include "EditModelMark_global.h"

#include<QObject>
#include"common/interfaces.h"
#include "data/fusionaligndata.h"
#include <common_ext/util/VectorAssist.h>
#include <UI_Common/gifViewLabel/GifViewLabel.h>
#include <UI_Common/gifViewLabel/gifviewgui.h>
#include <UI_Common/redoUndogui/redoUndoGui.h>
#include <QTimer>
#include <common_ext/commandmode/commandmanager.h>

// Undo point structure
struct FeaturePointData
{
public:
	FeaturePointData() {}

	FeaturePointData(QString _fdi, std::pair<QString, Point3f> _p)
	{
		setData(_fdi, _p);
	}

	void setData(QString _fdi, std::pair<QString, Point3f> _p)
	{
		this->label_fdi = _fdi;
		this->feature_point = _p;
	}

	FeaturePointData& operator=(const FeaturePointData& d)
	{
		this->label_fdi = d.label_fdi;
		this->feature_point = d.feature_point;
		return *this;
	}

public:
	QString label_fdi; // Tooth position number
	std::pair<QString, Point3f> feature_point; // Crown feature marking point position
};

// Data structure for picking crown feature points

namespace FeaturePointsMarking
{
	struct ToothPickedInfomation
	{
	public:
		ToothPickedInfomation() {}

		// Whether tooth position exists
		ToothPickedInfomation(const QString& fdi, bool is_existed)
		{
			this->label_fdi = fdi;
			this->is_existed = is_existed;
		}

		// Clear marking points and other information
		void clearFeaturePointsData() { feature_points.clear(); }

		// Clear border information
		void clearBorderInfo() { border_info.clearData(); }

		// Set whether this tooth position exists
		void setIsExisted(bool _is_existed) { this->is_existed = _is_existed; }

		// Clear data, mark tooth as non-existent and clear marking points
		void clearData() { setIsExisted(false); clearFeaturePointsData(); }

	public:
		bool is_existed = false; // Whether crown exists marker
		QString label_fdi; // Tooth position number
		std::map<QString, Point3f> feature_points; // Crown feature marking point positions
		// Crown border information, such as split line control points, bezier split lines
		AllToothBorderClosedContinuousIndex border_info;
	};
}

struct ToothWidthTool
{
public:
	ToothWidthTool() {}
	ToothWidthTool(QString _fdi, Point3f *_p_node_a, Point3f* _p_node_b, Point3m _view_direct)
	{
		fdi_ = _fdi;
		node_a_ = _p_node_a;
		node_b_ = _p_node_b;
		update(_view_direct);
	}

    bool operator==(const ToothWidthTool& rhs) const
	{
		return (fdi_ == rhs.fdi_);
	}

	void update(Point3m _view_direct)
	{
		if (node_a_ != nullptr && node_b_ != nullptr)
		{
			if (direct_mode_)
			{
				plane_normal_ = (*node_b_ - *node_a_).Normalize();
				Point3m axis_x = (plane_normal_ ^ Point3m(plane_normal_.X() + 1, plane_normal_.Y(), plane_normal_.Z())).Normalize();
				Point3m axis_y = (plane_normal_ ^ axis_x).Normalize();

				cut_face_a_.initFace2(*node_a_, plane_normal_, axis_x, axis_y, 5);
				cut_face_a_.setFaceColor(Point4m(52 / 255.f, 187 / 255.f, 219 / 255.f, 0.5f));
				cut_face_b_.initFace2(*node_b_, plane_normal_, axis_x, axis_y, 5);
				cut_face_b_.setFaceColor(Point4m(52 / 255.f, 187 / 255.f, 219 / 255.f, 0.5f));
			}
			else
			{
				Point3m vec_ab = (*node_b_ - *node_a_).Normalize();
				Point3m temp_vec = (_view_direct ^ vec_ab).Normalize();
				plane_normal_ = (temp_vec ^ _view_direct).Normalize();
				cut_face_a_.initFace2(*node_a_, plane_normal_, temp_vec, _view_direct, 10);
				cut_face_a_.setFaceColor(Point4m(52 / 255.f, 187 / 255.f, 219 / 255.f, 0.5f));
				cut_face_b_.initFace2(*node_b_, plane_normal_, temp_vec, _view_direct, 10);
				cut_face_b_.setFaceColor(Point4m(52 / 255.f, 187 / 255.f, 219 / 255.f, 0.5f));
			}
			updateWidth();
		}
	}

	void updateWidth()
	{
		if (node_a_ != nullptr && node_b_ != nullptr)
		{
			if (direct_mode_)
			{
				width_edge_ = FEdge(*node_a_, *node_b_);
				tooth_width_ = width_edge_.calculateLength();
			}
			else
			{
				Point3m a = *node_a_;
				Point3m b = UtilityTools::getInstance()->getProjPointOnPlane(a, *node_b_, -plane_normal_);
				width_edge_ = FEdge(a, b);
				tooth_width_ = width_edge_.calculateLength();
			}
		}
	}

	void updateVisibleDirectWithThirdNode(Point3m _third_node)
	{
		int dental_sign = fdi_.toInt() / 10;
		int tooth_sign = fdi_.toInt() % 10;
		if (node_a_ != nullptr && node_b_ != nullptr)
		{
			Point3m vec_ab = (*node_b_ - *node_a_).Normalize();
			Point3m vec_ac = (_third_node - *node_a_).Normalize();

			if (4 <= tooth_sign && tooth_sign <= 8)
			{
				Point3m vec0 = (vec_ab ^ vec_ac).Normalize();
				Point3m vec1 = (vec0 ^ vec_ab).Normalize();
				visible_direct_ = vec1;
			}
			else
			{
				Point3m vec0 = (vec_ac ^ vec_ab).Normalize();
				Point3m vec1 = (vec_ab ^ vec0).Normalize();
				visible_direct_ = vec1;
			}
		}
	}

	bool isUpperPart()
	{
		int dental_sign = fdi_.toInt() / 10;
		if (dental_sign == 1 || dental_sign == 2)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void setHoveringNode(Point3m *_p)
	{
		if (_p == nullptr)
		{
			node_a_hovering_ = false;
			node_b_hovering_ = false;
			return;
		}

		if (node_a_ == _p)
		{
			node_a_hovering_ = true;
			node_b_hovering_ = false;
			return;
		}

		if (node_b_ == _p)
		{
			node_b_hovering_ = true;
			node_a_hovering_ = false;
			return;
		}
	}

	void draw(QPainter* _painter, Point3m _camera_view_direct, MeshModel* _p_model, CloudOctree *_p_tree, bool _adjusting = false);
	void drawWidthLineSeg(QPainter* _painter, Point3m _camera_view_direct);
	bool mousePress(int _mouse_x, int _mouse_y);
	bool mouseMove(int _mouse_x, int _mouse_y);
	bool mouseRelease(int _mouse_x, int _mouse_y);
	void updateVisible(Point3m _camera_view_direct);

	bool need_update_cut_outline_ = false;
	bool direct_mode_ = true;
	bool node_a_hovering_ = false, node_b_hovering_ = false;
	Point3f* node_a_ = nullptr, *node_b_ = nullptr;
	Point3f plane_normal_, visible_direct_ = Point3m(0,0,0), inital_normal_;
	CutFace cut_face_a_, cut_face_b_;
	FEdge width_edge_;
	float tooth_width_;
	bool visible_ = true;
	QString fdi_;
};

class FeatureMarkGui;

class  EditModelMarkPlugin : public QObject, public MeshEditInterface
{
	Q_OBJECT
		Q_INTERFACES(MeshEditInterface)
public:
	EditModelMarkPlugin(QObject* parent = 0);
	EditModelMarkPlugin(QAction* pAct, QObject* parent = 0);
	virtual ~EditModelMarkPlugin();

public:
	virtual const QString Info() { return QString(); };
	void suggestedRenderingData(MeshModel& m, MLRenderingData& dt);
	virtual bool StartEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	virtual void EndEdit(MeshModel& m, GLArea* parent, MLSceneGLSharedDataContext* cont);
	void Decorate(MeshModel& m, GLArea* parent, QPainter* p);
	void keyPressEvent(QKeyEvent* event, MeshModel& m, GLArea* parent);
	void mousePressEvent(QMouseEvent* event, MeshModel& m, GLArea* parent);
	void mouseMoveEvent(QMouseEvent* event, MeshModel& m, GLArea* parent);
	void mouseReleaseEvent(QMouseEvent* event, MeshModel& m, GLArea* parent);
	void mouseDoubleClickEvent(QMouseEvent* event, MeshModel& m, GLArea* parent);
	void updateUI(bool visible = true);
	redoUndoGui* getRedoUndoGui() const { return p_redoUndo_ui; }
	// Create and close UI
	void createGui();
	void closeGui();

signals:
	// Tell navigation map the currently operating tooth position
	void currentWorkingToothSignal(const QString& label_fdi);
	// Tell navigation map existence/missing information for all tooth positions
	void initToothExistMarkSignal(const std::vector<std::pair<QString, bool>>& fdi_existed_map);
	// Tell navigation map whether to respond to right-click menu
	void setRightMouseMenuEnabledStatusSignal(bool bEnabled);

public slots:
	// Receive currently operating tooth position from navigation map
	void whichFdiToothLostSlot(const QString& fdi_tooth);
	// Receive tooth position to mark as missing from navigation map
	void undoWhichFdiToothLostSlot(const QString& fdi_tooth);
	// Receive command from navigation map to re-mark current tooth position
	void reLocatedTheFdiToothSlot(const QString& fdi_tooth);
	// Receive command from navigation map to re-mark all tooth positions
	void reLocatedAllToothSetCommand();
	// Receive currently operating tooth position from navigation map
	void currentWorkingToothSlot(const QString& label_fdi);

	// Switch upper/lower jaw event
	void tabUpperOrLowerSelectedSlot(int dentalType);
	void setGifSlot(bool is_marking_status);
	// Set whether current state is marking state
	void setCurrentMarkingStatusSlot(bool is_marking);
	// Receive signal to show GIF image
	void showGifSlot(bool openGif);

	// Save data
	void saveFeaturePointsSlot();

private:
	// Update data based on passed mesh
	void initData(MeshModel* _mesh);

	// Read marking data information from project or intelligent segmentation data, returns false on failure meaning only manual marking is possible
	bool getSegmentResultDataFormProjectOrJsonFile(bool is_upper_dental);

	// Get marking points from intelligent segmentation data
	void autoSegmentResultReadAndStatusInit(bool is_upper, const std::map<QString, CrownInfoSegmentedIntelligent>& data_crown_segmented);

	// Get project or create a project data object
	void getSegmentDataMemory(bool is_upper_dental, SegmentedStatusInfo*& segmented_data_);

	// Read marking segmentation information from project
	bool readSegmentStatusInfo(const SegmentedStatusInfo* segmentStatus, bool is_upper);

	// Update and calculate split line boundary
	void generateAndUpdateCrownBorderCurve();

	// Save project information
	bool saveSegmentStatusInfo(SegmentedStatusInfo* segmentStatus);

	// Initialize tooth position table based on upper/lower jaw prompt
	void initToothFdiTable(bool is_upper_dental, const std::vector<int>& is_existed_map = std::vector<int>());

	// Tell navigation map tooth position existence/missing information [adapt to results read from mlp project and navigation tooth position missing initialization]
	void tellFdiGuideToothExistedInfo();

	// Set wisdom teeth as missing
	void setFdiOpsigenesMissing();

	// Determine if it's a new project
	bool isEmptyProject()const;

	// Feature point marking and picking event handler
	void featurePointsMarkedEvent(MeshModel& m, GLArea* gla);

	// Feature point selection event
	void featurePointsPickedEvent(MeshModel& m, GLArea* gla);

	// Check if mouse hover stays on feature point
	bool featurePointsMouseHoveringEvent(MeshModel& m, GLArea* gla);

	// Selected feature point changes with mouse movement and finally settles
	void featurePointsMovingEvent(MeshModel& m, GLArea* gla);

	// Render and draw marked feature points
	void drawFeaturePoints(MeshModel& m, GLArea* parent, QPainter* p);

	// Draw 2D label
	void draw2DLabel(GLArea* gla, QPainter* painter);

	// Draw crown split line curve
	void drawCrownBorderCurve();

	// Initialize and construct corresponding tooth position feature point descriptions
	void initFdiAllFeaturesNames();
	// Return corresponding index based on FDI
	int getIndexToothFromFdi(const QString& label_fdi)const;

	// Return currently operating tooth position number ID
	int getCurrentToothFDIWorking()const;

	// Notify navigation map and plugin of currently operating tooth position
	void tellProgramCurrentToothFdiWorking();

	// Notify currently operating tooth position through known FDI
	void tellProgramCurrentToothFdiWorking(const QString& label_fdi);

	// Initialize some operation information
	void initStatusOperating();

	// Update feature point hint information for currently operating tooth position
	void updateHintCurrentFdiFeatureNames();

	// Determine if marking is complete
	bool judgeFeaturePointsMarked(const SegmentedStatusInfo* segmented_info)const;
	bool judgeFeaturePointsMarked();

private:
	GLArea* parent = nullptr;
	MeshModel* mesh_ = nullptr;

	// Whether current state is marking state
	bool is_marking_status_ = false;

	bool b_operate_upper_ = true;
	// Project information data
	SegmentedStatusInfo* segmented_data_ = nullptr;

	// Feature point marking interaction interface
	FeatureMarkGui* feature_marking_gui_ = nullptr;
	Gifviewgui* gif_view_ui = nullptr;
	Gifviewgui* gif_viewfix_ui = nullptr;
	GifViewLabel* gif_view_label_ui = nullptr;
	// Redo/undo interface
	redoUndoGui* p_redoUndo_ui = nullptr;
	// Data object supporting interaction
	std::vector<FeaturePointsMarking::ToothPickedInfomation*> tooth_picked_info_;

	// Tooth width measurement tool
	ToothWidthTool* cur_tooth_width_tool_ = nullptr;
	std::vector<ToothWidthTool> tooth_width_tool_list_;
	ToothWidthTool* getToothWidthToolWith(QString _fdi, Point3f* _p_node_a, Point3f* _p_node_b);
	bool getToothWidthToolVisibleWith(QString _fdi);
	bool deleteToothWidthToolWith(QString _fdi);
	void createMissGif();
	void createFixGif();
	void createRedoUndoGui();

	// Current mouse point position
	QPoint currentScreenPoint;
	// Index corresponding to currently operating/selected tooth position, initialized to -1 for no selected tooth position
	int current_operator_tooth_fdi_ = -1; // [0,15]

	// Hint feature point information for currently operating tooth position, each string corresponds to one line of text hint (bottom left corner)
	std::vector<std::shared_ptr<QString>> hint_fdi_feature_names_;
	// Hint a feature point of currently operating tooth position to be adjusted or picked... (mouse position)
	std::shared_ptr<QString> hint_feature_name_;
	// Feature sequence number for mouse hint
	int index_feature_mouse = -1;

	// Previous feature point position of currently mouse-sliding modified feature point
	FeaturePointData feature_point_last_;
	// Currently mouse-sliding modified feature point
	FeaturePointData feature_point_current_;
	// Name of picked feature point
	QString feature_point_name_picked_;
	// Tooth position record corresponding to feature point where mouse hovers
	int fdi_mouse_hovering_get_feature_point_ = -1;

	// Whether to render feature points
	bool b_show_feature_points_ = true;

	// Whether to render crown border line (toggle hide/show with shortcut key)
	bool b_show_crown_border_ = false;

	// Whether ready to pick feature points (on mouse double click)
	bool b_pick_feature_points_ready_ = false;

	// Whether left click or hover triggers selection of a feature point to be adjusted
	bool is_picking_ = false;

	// Whether a feature point is selected
	bool is_picked_ = false;

	// Whether mouse is sliding
	bool is_mouse_hovering = false;

	// Whether feature points have been modified
	bool is_modify_feature_points_ = false;

	// Timer to delay mouse sliding when double-clicking to pick feature points
	QTimer* timer_hovering_start = nullptr;
	// Enable feature point hover state on mouse sliding
	bool is_enable_mouse_hovering = true;

	// Feature point descriptions corresponding to tooth positions
	std::map<QString, std::vector<QString>> fdi_feature_points_names_;

	// Undo command...
private:
	CommandManager* command_manager_ = nullptr;
	void clearCommand();
    std::atomic<bool> task_end_;

	// Feature point marking record
	void featurePointsRecord(const FeaturePointData& data_record);
	void featurePointsRecordUndo(const FeaturePointData& data_record);
	void featurePointsRecordRedo(const FeaturePointData& data_record);
	// Feature point modification command
	void featurePointsModifyRecord(const FeaturePointData& last_data_record, const FeaturePointData& data_record);
	void featurePointsModifyRecordUndo(const FeaturePointData& data_record);
	void featurePointsModifyRecordRedo(const FeaturePointData& data_record) { featurePointsModifyRecordUndo(data_record); }
	// Feature point deletion command (delete points on one tooth position)
	void featurePointsClearRecord(const FeaturePointsMarking::ToothPickedInfomation& last_data_record);
	void featurePointsClearRecordUndo(const FeaturePointsMarking::ToothPickedInfomation& data_record);
	void featurePointsClearRecordRedo(const FeaturePointsMarking::ToothPickedInfomation& data_record);
	// Feature point deletion command (delete points on all tooth positions)
	void featurePointsAllClearRecord(const std::vector<FeaturePointsMarking::ToothPickedInfomation>& last_data_record);
	void featurePointsAllClearRecordUndo(const std::vector<FeaturePointsMarking::ToothPickedInfomation>& data_record);
	void featurePointsAllClearRecordRedo(const std::vector<FeaturePointsMarking::ToothPickedInfomation>& data_record);

private slots:
	void unDo();
	void reDo();
};

#endif // EDITMODELMARKPLUGIN_H
