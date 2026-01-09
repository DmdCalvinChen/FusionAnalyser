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

#ifndef DENTALANALYSISDATA_H
#define DENTALANALYSISDATA_H

#include <qobject.h>
#include <qapplication.h>
#include <qclipboard.h>
#include "qpainter.h"
#include "util/utility_tools.h"
#include "data/fusionaligndata.h"
#include "towablectrlsystem.h"
#include "interactive2Dlabel.h"
#include "interactive2Dframe.h"
#include "common_ext_global.h"
#include "CutFace.h"
#include "common/config.h"
#include <wrap/qt/device_to_logical.h>

class EditItem :
    public QObject
{
    Q_OBJECT
public:
    EditItem() {}
    EditItem(EditItemIndex _index, QString *_context)
    {
        index_ = _index;
        context_ = _context;
    }

    inline QString context() { if (context_ != nullptr) { return *context_; } else { return QString(); } }
    inline QString prompt() { return prompt_context_; }
    inline float value() { return value_; }
    inline EditItemIndex itemIndex() { return index_; }
    inline void setContext(QString _context) { if (context_ != nullptr) { *context_ = _context; } }
    inline void setPrompt(QString _context) { prompt_context_ = _context; }
    inline void setVisible(bool _visible) { visible_ = _visible; }
    inline bool visible() { return visible_; }
    inline void setFeatureMode(bool _state) { feature_mode_ = _state; }
    inline void setSpecialSign(bool _state) { special_sign_ = _state; }

    virtual void draw(QPainter *_p) {}
    virtual void update() {}
    virtual void updateFeatures() {}
    virtual bool mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::NoButton) { return false; }
    virtual void initialUpdate() {}
    virtual void initialUpdateFeatures() {}
    virtual void updateCtrlParts() {}
    virtual void startEdit(){}
    virtual bool availabilityJudgment() { return true; }

    bool mouseMove(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::NoButton);
    bool mouseRelease(int _mouse_x, int _mouse_y);
    void drawCrevicePlane(TowableCtrlSystem& _sys, bool &_need_update_cut_outline, MeshModel *_p_model = nullptr, CloudOctree *_tree = nullptr);
    void drawCrevicePolygonPlane(vector<Point3m>& _verts);
    void drawEdge(FEdge &_edge, Point3m _color_a = Point3m(40.f / 255.f, 220.f / 255.f, 120.f / 255.f), Point3m _color_b = Point3m(40.f / 255.f, 220.f / 255.f, 120.f / 255.f));
    void setScreenWidthAndHeight(float _width, float _height);
    void setCurrentItemVisibleMode(Interactive2DLabel::AccompanyMotionId _id);
    void addToObjectList(TowableCtrlSystem* _obj);
    void updateEachCtrlObjVisibleByView(vector<TowableCtrlSystem> &_obj_list);

    float screen_width_, screen_height_;
    TowableCtrlSystem* p_cur_obj_ = nullptr;
    vector<TowableCtrlSystem*> obj_list_;
    vector<Point3m>* ctrlnodes_ = nullptr, *nodes_5_5_ = nullptr, *nodes_7_7_ = nullptr;
    vector<FEdge>* ctrledges_ = nullptr;
    bool feature_mode_ = false;
    bool special_sign_ = true;
    Interactive2DLabel::AccompanyMotionId first_enable_item_index_ = Interactive2DLabel::DEFAULT_VIEW;
    bool mouse_pressed_ = false;

private:
    QString *context_ = nullptr;
    QString prompt_context_;
    float value_ = 0;
#if PLATFORM_IOS
    EditItemIndex index_;
#else
    EditItemIndex index_ = NONE;
#endif
    bool visible_ = true;
};

class MissingToothItem : public EditItem
{
public:
    MissingToothItem(Dental* _dental, vector<int>& _fdi_list, QString* _context);
    MissingToothItem(DentalFeatures* _dental, vector<int> &_fdi_list, QString* _context);
};

class TeethWidthItem : public EditItem
{
public:
    TeethWidthItem(Dental* _dental, QString* _context);
    TeethWidthItem(DentalFeatures *_dental_feature, QString *_context, QString* _context_first_three, QString* _context_first_six, vector<FEdge> *_ctrl_edges, bool _six_to_six);

    void draw(QPainter *_p);
    void update();
    void updateFeatures();
    bool mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::NoButton);
    void initialUpdate();
    void initialUpdateFeatures();
    void updateCtrlParts();
    void startEdit();
    bool availabilityJudgment() { return true; }

public:
    Dental* dental_ = nullptr;
    DentalFeatures* dental_feature_ = nullptr;
    vector<TowableCtrlSystem> ctrl_obj_list_;
    vector<FEdge>* ctrl_edges_ = nullptr;
    bool six_to_six_ = true;
    QString* context_first_three_ = nullptr, *context_first_six_ = nullptr;
};

class CurrentArchLengthItem : public EditItem
{
public:
    CurrentArchLengthItem(Dental* _dental, QString* _context, vector<Point3m>* _ctrlnodes);
    CurrentArchLengthItem(DentalFeatures* _dental_features, QString* _context, vector<Point3m>* _ctrlnodes, vector<Point3m>* _5_5_nodes, vector<Point3m>* _7_7_nodes, QString* _context_complete, bool _six_to_six);
    ~CurrentArchLengthItem();

    void draw(QPainter *_p);
    void update();
    void updateFeatures();
    bool mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::NoButton);
    void initialUpdate() {}
    void initialUpdateFeatures() {}
    void updateCtrlParts();
    void startEdit();
    bool availabilityJudgment();

public:
    Dental* dental_ = nullptr;
    DentalFeatures* dental_feature_ = nullptr;
    TowableCtrlSystem *ctrl_obj_ = nullptr, *auxiliary_obj1_ = nullptr, * auxiliary_obj2_ = nullptr;
    QString* context_complete_ = nullptr;
    bool six_to_six_ = true;
};

class BoltonItem : public EditItem
{
public:
    BoltonItem(Dental* _dental, QString* _context_anterior, QString* _context, QString* _context_complete, EditItemIndex _item_index, int _tooth_sign);
    BoltonItem(DentalFeatures* _dental_features, QString* _context_anterior, QString* _context, QString* _context_complete, EditItemIndex _item_index, int _tooth_sign, bool _six_to_six);

    void initialUpdate();
    void initialUpdateFeatures();
    void draw(QPainter *_p);
    void update();
    void updateFeatures();
    bool mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::NoButton);
    void updateCtrlParts() {}
    void startEdit();
    bool availabilityJudgment();

public:
    QString* context_anterior_ = nullptr;
    QString* context_complete_ = nullptr;
    Dental* dental_ = nullptr;
    DentalFeatures* dental_feature_ = nullptr;
    vector<TowableCtrlSystem> ctrl_obj_list_;
    int tooth_sign_range_;
    bool six_to_six_ = true;
};

class SpeeCurveDepthItem : public EditItem
{
public:
    SpeeCurveDepthItem(Dental* _dental, float* _left_deepth, float* _right_deepth, QString* _context, vector<Point3m>* _ctrlnodes);
    SpeeCurveDepthItem(DentalFeatures* _dental_features, float *_left_deepth, float *_right_deepth, QString* _context, vector<Point3m>* _ctrlnodes);
    ~SpeeCurveDepthItem();
    void draw(QPainter *_p);
    void update();
    void updateFeatures();
    bool mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::NoButton);
    void initialUpdate() {}
    void initialUpdateFeatures() {}
    void updateCtrlParts();
    void startEdit();
    bool availabilityJudgment();
    FEdge fliterDeepestEdge(float &_depth, vector<FEdge> &_edges, const Point3m &_pos, const Point3m &_axis_x, const Point3m &_axis_y);

public:
    Dental* dental_ = nullptr;
    DentalFeatures* dental_feature_ = nullptr;
    TowableCtrlSystem* left_ope_obj_ = nullptr, *right_ope_obj_ = nullptr;
    Point3m left_vec_, right_vec_;
    FEdge left_deep_edge_, right_deep_edge_;
    CutFace spee_plane_;
    float* left_deepth_ = nullptr, * right_deepth_ = nullptr;
};

class MolarRelationshipItem : public EditItem
{
public:
    MolarRelationshipItem(QString* _context, Point3m* _left_node, Point3m* _right_node, DentalManager* _dental_manager, vector<FEdge>* _ctrledges);
    MolarRelationshipItem(QString* _context, Point3m *_left_node, Point3m *_right_node, DentalFeatures *_dental_features, vector<FEdge>* _ctrledges);
    ~MolarRelationshipItem();
    void draw(QPainter *_p);
    void update();
    void updateFeatures();
    bool mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::NoButton);
    void initialUpdate() {}
    void initialUpdateFeatures() {}
    void updateCtrlParts();
    void startEdit();
    bool availabilityJudgment();
public:
    DentalFeatures* dental_feature_ = nullptr;
    TowableCtrlSystem* left_obj_ = nullptr, * right_obj_ = nullptr;
    Point3m* left_node_ = nullptr, * right_node_ = nullptr;
    Point3m left_direct_, right_direct_;
    Point3m left_view_direct_, right_view_direct_;
    FEdge left_edge_, right_edge_;
};

class MidlineRelationshipItem : public EditItem
{
public:
    MidlineRelationshipItem(QString* _context, DentalManager* _dental_manager, Axis* _plane, vector<FEdge>* _ctrledges);
    MidlineRelationshipItem(QString* _context, DentalFeatures* _dental_features, Axis* _plane, vector<FEdge>* _ctrledges);
    ~MidlineRelationshipItem();
    void draw(QPainter *_p);
    void update();
    void updateFeatures();
    bool mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::NoButton);
    void updatePlane(TowableCtrlSystem* _obj, Axis *_axis, vector<Point3m> &_plane_verts);
    void initialUpdate() {}
    void initialUpdateFeatures() {}
    void updateCtrlParts();
    void startEdit();
    bool availabilityJudgment();
public:
    DentalFeatures* dental_feature_ = nullptr;
    TowableCtrlSystem* obj_ = nullptr;
    Axis *plane_ = nullptr;
    vector<Point3m> plane_verts_;
    bool first_initial_ = true;
    Point3m first_initial_direct_;
    Point3m occlusal_plane_normal;
};

class ArchWidthItem : public EditItem
{
public:
    ArchWidthItem(Dental* _detnal, float* _front_value, float* _middle_value, float* _rear_value, float* _basalbone_value, QString* _context, vector<FEdge>* _ctrledges);
    ArchWidthItem(DentalFeatures *_detnal_features, float *_front_value, float* _middle_value, float* _rear_value, float* _basalbone_value, QString* _context, vector<FEdge>* _ctrledges);
    ~ArchWidthItem();
    void draw(QPainter *_p);
    void update();
    void updateFeatures();
    bool mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::NoButton);
    void initialUpdate() {}
    void initialUpdateFeatures() {}
    void updateCtrlParts();
    void startEdit();
    bool availabilityJudgment();
public:
    Dental* dental_ = nullptr;
    DentalFeatures *dental_feature_ = nullptr;
    TowableCtrlSystem* front_obj_ = nullptr, *middle_obj_ = nullptr, *rear_obj_ = nullptr, *basalbone_obj_ = nullptr;
    float* front_value_ = nullptr, * middle_value_ = nullptr, * rear_value_ = nullptr, * basalbone_value_ = nullptr;
    FEdge basalbone_edge_;
    bool front_edge_enable_ = true, middle_edge_enable_ = true, rear_edge_enable_ = true;
};

class ArchLengthItem : public EditItem
{
public:
    ArchLengthItem(Dental* _dental, float* _front_value, float* _middle_value, float* _rear_value, float* _whole_value, QString* _context, vector<FEdge>* _ctrledges);
    ArchLengthItem(DentalFeatures *_dental_features, float* _front_value, float* _middle_value, float* _rear_value, float* _whole_value, QString* _context, vector<FEdge>* _ctrledges);
    ~ArchLengthItem();
    void draw(QPainter *_p);
    void update();
    void updateFeatures();
    bool mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::NoButton);
    void initialUpdate() {}
    void initialUpdateFeatures() {}
    void updateCtrlParts();
    void startEdit();
    bool availabilityJudgment();
    bool getIntersectPoint3m(Point3m _pa1, Point3m _pa2, Point3m _pb1, Point3m _pb2, Point3m &_result);
public:
    Dental* dental_ = nullptr;
    DentalFeatures* dental_feature_ = nullptr;
    Point3m anterior_tangent_vert;
    FEdge front_edge_, middle_edge_, rear_edge_, total_edge_;
    TowableCtrlSystem* front_obj_ = nullptr, * middle_obj_ = nullptr, * rear_obj_ = nullptr;
    float* front_value_ = nullptr, * middle_value_ = nullptr, * rear_value_ = nullptr, * whole_value_ = nullptr;
};

class BasalBoneArchItem : public EditItem
{
public:
    BasalBoneArchItem(DentalFeatures* _dental_features, float* _basalbone_len_value, float* _basalbone_wid_value, QString* _context, vector<FEdge>* _ctrledges);
    ~BasalBoneArchItem();
    void draw(QPainter* _p);
    void update();
    void updateFeatures();
    bool mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::NoButton);
    void initialUpdate() {}
    void initialUpdateFeatures() {}
    void updateCtrlParts();
    void startEdit();
    bool availabilityJudgment();
public:
    DentalFeatures* dental_feature_ = nullptr;
    Point3m anterior_tangent_vert, gum_vert_, proj_gum_vert_;
    FEdge basalbone_len_edge_, gum_len_edge_, basalbone_wid_edge_;
    TowableCtrlSystem* basalbone_len_obj_ = nullptr, * gum_vert_obj_ = nullptr, * basalbone_wid_obj_ = nullptr;
    float* basalbone_len_value_ = nullptr, * basalbone_wid_value_ = nullptr;
};

class GnathotectumHeightItem : public EditItem
{
public:
    GnathotectumHeightItem(Dental* _dental, float* _p_value, QString* _context);
    GnathotectumHeightItem(DentalFeatures *_dental_features, float* _p_value, QString* _context, vector<FEdge>* _ctrledges);
    ~GnathotectumHeightItem();
    void draw(QPainter *_p);
    void update();
    void updateFeatures();
    bool mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::NoButton);
    void initialUpdate() {}
    void initialUpdateFeatures() {}
    void updateCtrlParts() {}
    void startEdit();
    bool availabilityJudgment();
public:
    float* p_height_value_ = nullptr;
    Dental* dental_ = nullptr;
    DentalFeatures* dental_feature_ = nullptr;
    CMeshO* gum_mesh_ = nullptr;
    TowableCtrlSystem *tooth_ope_obj_ = nullptr, *gum_ope_obj_ = nullptr;
    FEdge height_edge_;
};

class COMMON_EXT_EXPORT DentalAnalysisData :
    public QObject
{
    Q_OBJECT
public:
    DentalAnalysisData();
    DentalAnalysisData(const DentalAnalysisData& _info);
    DentalAnalysisData(DentalManager* _upper_dental, DentalManager* _lower_dental);
    DentalAnalysisData(DentalFeatures *_upper_dental, DentalFeatures*_lower_dental);
    ~DentalAnalysisData();

    DentalAnalysisData& operator=(const DentalAnalysisData& _data);

    void initialAnalysisData(DentalManager* _upper_dental, DentalManager* _lower_dental);
    void initialAnalysisData(DentalFeatures* _upper_dental, DentalFeatures* _lower_dental);
    void switchEditItem(EditItemIndex _item, bool _b_upper = true);
    void switchEditItemFeatureMode(EditItemIndex _item, bool _b_upper = true);
    void setContext(QString _context);
    void updateSeverityOfCrowding();
    void updateBoltonAnterior();
    void updateBoltonCompletely();
    void udpateMidlineRelationship();
    void updateMolarRelationship();
    void updateSpeedDeepth();
    void updateTeethWidthList();
    void updateTeethWidthEdgesList();
    void clearItems();
    bool searchItemInList(EditItemIndex _item_index, vector<EditItem*> &_item_list, EditItem *&_result);
    bool deleteItemInList(EditItemIndex _item_index, vector<EditItem*>& _item_list);
    void unFocusAllFrames();
    void redoCrowdingAnalysis();
    static void setCrowdingAnalysisMode(bool _six_to_six_mode);
    void addBrowsedPrompt(QString _prompt);
    void alignFramesSize(vector<Interactive2DFrame> &_frames);

    vector<std::pair<QString, bool>> getEachItemConfirmState();

    void draw(QPainter *_p);
    void update();
    void updateLabels(EditItemIndex _item_index);
    bool mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::NoButton);
    bool mouseMove(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn = Qt::NoButton);
    bool mouseRelease(int _mouse_x, int _mouse_y);

    bool picked();
    void setScreenWidthAndHeight(float _width, float _height);
    void setOverjetValue(float _value);
    void setOverbiteValue(float _value);
    void setOverjetRank(QString _rank);
    void setOverbiteRank(QString _rank);
    void setRemark(QString _remark);
    void setPatientName(QString _name);
    void setDoctorName(QString _name);
    void autoPickonTopInteractiveFrame();
    void startEdit();

    QString getMark()const { return remark_; }
    QString getPatientName()const { return patient_name_; }
    QString getDoctorName()const { return doctor_name_; }

    bool mouse_pressed_ = false;
    float screen_width_, screen_height_;
    vector<Interactive2DLabel> labels_;
    vector<Interactive2DFrame> frames_;
    vector<EditItemIndex> avaliable_item_index_list_;
    Interactive2DLabel::AccompanyMotionId upper_arch_len_motion_ = Interactive2DLabel::FRONT_SHOW_ONLY_UPPER;
    Interactive2DLabel::AccompanyMotionId lower_arch_len_motion_ = Interactive2DLabel::FRONT_SHOW_ONLY_LOWER;
    Interactive2DLabel::AccompanyMotionId upper_basal_bone_motion_ = Interactive2DLabel::BASALBONE_WID_SHOW_ONLY_UPPER;
    Interactive2DLabel::AccompanyMotionId lower_basal_bone_motion_ = Interactive2DLabel::BASALBONE_WID_SHOW_ONLY_LOWER;
    Interactive2DLabel::AccompanyMotionId corwding_compute_mode_motion_ = Interactive2DLabel::CROWDING_SWITCH_TO_5_5;

    public slots:
    void copyToClipboard();
    void generateCopyContext(QString &_context);
    void setConfirmState(QString _item_name, bool _state);
    void updateLabelsSlot(int _width, int _height);

signals:

public:
    vector<FEdge> tooth_width_edges_upper_, tooth_width_edges_lower_;
    vector<Point3m> cur_length_arch_ctrlnodes_upper_, cur_length_arch_ctrlnodes_lower_;
    vector<Point3m> crowding_arch_5_5_upper_, crowding_arch_7_7_upper_, crowding_arch_5_5_lower_, crowding_arch_7_7_lower_;
    vector<Point3m> spee_ctrlnodes_;
    vector <FEdge> molar_ctrledges_upper_, molar_ctrledges_lower_;
    vector <FEdge> midline_ctrledges_upper_, midline_ctrledges_lower_;
    vector <FEdge> arch_width_ctrledges_upper_, arch_width_ctrledges_lower_;
    vector <FEdge> arch_len_ctrledges_upper_, arch_len_ctrledges_lower_;
    vector <FEdge> basal_bone_arch_ctrledges_upper_, basal_bone_arch_ctrledges_lower_;
    vector<FEdge> gna_height_ctrledges_upper_;
    CutFace* left_overlay_cut_face_ = nullptr, * right_overlay_cut_face_ = nullptr;
    FEdge left_overlay_measure_edge_, right_overlay_measure_edge_;

public:
    static bool crowding_analysis_6to6_mode_;
#if PLATFORM_IOS
    EditItemIndex cur_mode_;
#else
    EditItemIndex cur_mode_ = NONE;
#endif

    DentalManager* upper_dental_manager_ = nullptr, * lower_dental_manager_ = nullptr;
    DentalFeatures* upper_dental_features_ = nullptr, * lower_dental_features_ = nullptr;
    EditItem* cur_ope_item_ = nullptr;
    EditItem *cur_upper_item_ = nullptr, *cur_lower_item_ = nullptr;
    vector<EditItem*> upper_item_list_, lower_item_list_;

    Axis upper_plane_, lower_plane_;
    Point3m left_molar_node_upper_, right_molar_node_upper_, left_molar_node_lower_, right_molar_node_lower_;
    bool confirm_missing_fdi_ = true;
    bool confirm_tooth_width_ = true;
    bool confirm_crowding_ = true;
    bool confirm_bolton_ = true;
    bool confirm_anterior_parameter_ = true;
    bool confirm_molar_ = true;
    bool confirm_spee_ = true;
    bool confirm_arch_width_ = true;
    bool confirm_arch_length_ = true;
    bool confirm_basal_bone_arch_ = true;
    bool confirm_gnathotectum_height_ = true;
    bool confirm_overlay_ = true;
    vector<QString> browsed_item_prompt_;

    float left_spee_depth_ = 0, right_spee_deepth_ = 0;
    float arch_width_front_upper_= 0, arch_width_middle_upper_= 0, arch_width_rear_upper_= 0, arch_width_basalbone_upper_ = 0;
    float arch_width_front_lower_= 0, arch_width_middle_lower_= 0, arch_width_rear_lower_= 0, arch_width_basalbone_lower_ = 0;
    float arch_len_front_upper_= -1, arch_len_middle_upper_= -1, arch_len_rear_upper_= -1, arch_len_whole_upper_ = -1, arch_len_basalbone_upper_ = -1;
    float arch_len_front_lower_= -1, arch_len_middle_lower_= -1, arch_len_rear_lower_= -1, arch_len_whole_lower_ = -1, arch_len_basalbone_lower_ = -1;
    float overbite_value_ = -1;
    float overjet_value_ = -1;
    float gnathotectum_height_value_upper_;
    vector<int> missing_tooth_upper_, missing_tooth_lower_;
    QString space_requared_upper_, space_requared_lower_;
    QString cur_length_arch_upper_, cur_length_arch_lower_;
    QString crowding_severity_upper_, crowding_severity_lower_;
    QString sum_anterior_width_upper_, sum_anterior_width_lower_;
    QString sum_whole_width_upper_, sum_whole_width_lower_;
    QString bolton_anterior_larger_describe_;
    QString bolton_anterior_larger_value_;
    QString bolton_anterior_discrepancy_;
    QString bolton_anterior_;
    QString bolton_completely_larger_describe_;
    QString bolton_completely_larger_value_;
    QString bolton_completely_discrepancy_;
    QString bolton_completely_;
    QString needed_crevice_;
    QString left_molar_relationship_, right_molar_relationship_, bilateral_molar_relationship_;
    QString midline_relationship_;
    QString gnathotectum_height_upper_;
    QString overbite_rank_, overbite_rank_translated_;
    QString overjet_rank_, overjet_rank_translated_;
    vector<QString> tooth_width_list_right_upper;
    vector<QString> tooth_width_list_left_upper;
    vector<QString> tooth_width_list_right_lower;
    vector<QString> tooth_width_list_left_lower;

    QColor crowding_severity_upper_color_, crowding_severity_lower_color_;
    QColor bolton_completely_color_, bolton_anterior_color_;
    QColor left_molar_relationship_color_, right_molar_relationship_color_, bilateral_molar_relationship_color_;

    QString patient_name_, doctor_name_;
    QString remark_;
};

#endif
