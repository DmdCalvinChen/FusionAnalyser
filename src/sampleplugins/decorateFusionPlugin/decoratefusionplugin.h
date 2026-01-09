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

#ifndef DECORATEFUSIONPLUGIN_H_
#define DECORATEFUSIONPLUGIN_H_

#include "GL/glew.h"
#include "wrap/gl/trimesh.h"
#include "common/interfaces.h"
#include <common/config.h>
#include "common/ml_mesh_type.h"
#include "common_ext/util/utility_tools.h"

#include "vcg/space/index/kdtree/kdtree.h"
#include "common_ext/data/dentalmanager.h"
#include "common_ext/data/fusionaligndata.h"
#include "common_ext/meshExt/dentalfeatures.h"

enum EditItemIndex:int;

class QDockWidget;
class QTranslator;

namespace ui_common
{
	class BoltonAnaDlg;
}

namespace fusionAlign
{
	struct DataContentColumnFormat;
	struct ToothMoveTable;
}

namespace common_ext
{
    class BoltonAna;
}

class DecorateFusionPlugin : public QObject, public MeshDecorateInterface
{

	Q_OBJECT
		MESHLAB_PLUGIN_IID_EXPORTER(MESH_DECORATE_INTERFACE_IID)
		Q_INTERFACES(MeshDecorateInterface)

		QString decorationName(FilterIDType filter) const;
	QString decorationInfo(FilterIDType filter) const;

	enum DecorateToolsEnum
	{
		DP_SHOW_MEASUREGRID,     // Measurement grid
		DP_SHOW_TOOTHFDI,        // Tooth position number
		DP_SHOW_TOOTHOCCLUSION,  // Occlusion contact
		DP_SHOW_TOOTHBOLTON,     // Tooth position measurement
		DP_SHOW_MEASURE,          // Measurement
	};

public:
	DecorateFusionPlugin();
	~DecorateFusionPlugin();

	bool startDecorate(QAction * /*mode*/, MeshModel &/*m*/, RichParameterSet *, GLArea * /*parent*/);
	bool startDecorate(QAction * /*mode*/, MeshDocument &/*m*/, RichParameterSet *, GLArea * /*parent*/);
    bool startPostDecorate(QAction*act, MeshDocument &md, RichParameterSet*rps, GLArea*glarea);
	void decorateMesh(QAction *a, MeshModel &md, RichParameterSet *, GLArea *gla, QPainter *painter, GLLogStream &_log);
    void decoratePostDoc(QAction *a, MeshDocument &md, RichParameterSet *, GLArea *gla, QPainter *painter, GLLogStream &_log);
	bool decorateDoc(QAction *a, MeshDocument &md, RichParameterSet *, GLArea *gla, QPainter *painter, GLLogStream &_log);
	void endDecorate(QAction * /*mode*/, MeshModel &/*m*/, RichParameterSet *, GLArea * /*parent*/);
	void endDecorate(QAction *, MeshDocument &, RichParameterSet *, GLArea *);
    void endPostDecorate(QAction*act, MeshDocument &md, RichParameterSet*rps, GLArea*glarea);

	bool isDecorationApplicable(QAction *action, const MeshModel& m, QString &ErrorMessage) const;
	int getDecorationClass(QAction */*action*/) const;
	//void initGlobalParameterSet(QAction *, RichParameterSet &/*globalparam*/);

	void mousePressEvent(QAction* a, QMouseEvent* event, MeshModel&/*m*/, GLArea*);
	void mouseReleaseEvent(QMouseEvent* event, MeshModel&/*m*/, GLArea*);
	bool judgeActionIsWorkingStatus(DecorateToolsEnum _tool);
	std::map<QString, QIcon> iconChange;
	QStringList iconName;
public slots:
	void mousePickedVertexReceivedSlot(float x, float y, float z);
	void updateAnalyzerAvaliableItemActionsSlot(std::vector<EditItemIndex> _avaliable_item_indexes);

protected:
	// fdi
	std::map<QString, vcg::KdTree<float>> searchTreeMesh;
	// accessory
	vcg::GlTrimesh<CMeshO> glWrap, glWrapLower;
	bool b_show_upper_tooth_ = true;
	bool b_show_lower_tooth_ = true;

	// occlusal contact
	Point3m camera_pos_, view_dir_;
	std::vector<vcg::Point2i> tooth_cp_;

	//glarea width height
	int width_ = 0, height_ = 0;

protected:
	// measure grid
	void drawBackGrid(GLArea* gla, QPainter* painter, float maxX, float maxY, float step, vcg::Color4b lineColor);

	// Draw angle and distance measurement results
	void drawMeasureResult(GLArea* gla, QPainter* painter);

	// fdi
	bool judgeFdiLabelIsInView(CMeshO* mesh, const vcg::Point3f& fdiShowPos, const vcg::Point3f& normal, GLArea *gla);
	//void drawToothFDILabel(QPainter* panter, MeshDocument &md, GLArea *gla);
    void drawToothFDILabel(QPainter* panter, MeshModel &md, GLArea *gla);

	//display dental overlap area
	bool b_mouse_interactive_ = false;
	bool bEnableDisplayDentalOverlapArea = false, bEnableOverlap = true;
	Point3m cameraPosition, viewDirect, view_RH_Direct;
	vector<Point2i> toothCP;
	Point2i coverageVert;
	vector<Point2i> projBoxVerts;
	int coverageHeight = 0, coverageWidth = 0;
	GL_TYPE_NM<Scalarm>::ScalarType *buffer_U_V=nullptr, *buffer_U_NV=nullptr, *buffer_L_V=nullptr, *buffer_L_NV=nullptr, *buffer_depth=nullptr;

	void drawDentalOverlapArea(QPainter* p, GLArea* gla);
	void drawDentalOverlapAreaMode2(QPainter* p, GLArea* gla);
	void drawDentalOverlapAreaPerTooth(QPainter* p, DentalManager *curMeshMode, DentalManager *meshMode_U, DentalManager *meshMode_L);
	void drawOverlapPixels(vcg::GL_TYPE_NM<Scalarm>::ScalarType *buffer);
	void updateCoverage(DentalManager* modeU, DentalManager* modeL);
	void updateCoverage(CMeshO *_meshU, CMeshO *_meshL);
	void updateMeshModelCoverage(DentalManager* curMode, Point2i& _vertMin, Point2i& _vertMax);
	void updateMeshModelCoverage(CMeshO *_curMesh, Point2i &_vertMin, Point2i &_vertMax);
	void tailorProjcetion(Point3d &v);
	void collectTooth2DProjFaces(int iTooth, DentalManager* curManager, bool getVisibleFace, Point3m cameraPos, Point3m viewDirect, vector<FFace>& faces);
	void collectTooth2DProjFaces(CMeshO *_mesh, bool getVisibleFace, Point3m cameraPos, Point3m viewDirect, vector<FFace> &faces);
	void collectTooth2DProjFacesIndexes(int iTooth, DentalManager* curMesh, Point3m cameraPos, Point3m viewDirect, std::vector<FFace> &faceList_V, std::vector<FFace> &faceList_NV);
	vcg::Point3d projectPoint(Point3m p, DentalManager* curMeshModel);
	vcg::Point3d unProjectPoint(Point3m p, MeshModel * curMeshModel);
	bool prepareFbo(unsigned int& textId, unsigned int& fboId, unsigned int &rboId, int width, int height);
	//void getCurrentCameraState(MeshModel *curMesh);
	vcg::GL_TYPE_NM<Scalarm>::ScalarType *p_overlap_depth_buffer_ = nullptr;
    vcg::GL_TYPE_NM<Scalarm>::ScalarType *p_up_mesh_buffer_ = nullptr;
    vcg::GL_TYPE_NM<Scalarm>::ScalarType *p_up_mesh_v_buffer_ = nullptr;
    vcg::GL_TYPE_NM<Scalarm>::ScalarType *p_up_mesh_nv_buffer_ = nullptr;
    vcg::GL_TYPE_NM<Scalarm>::ScalarType *p_down_mesh_buffer_ = nullptr;
    vcg::GL_TYPE_NM<Scalarm>::ScalarType *p_down_mesh_v_buffer_ = nullptr;
    vcg::GL_TYPE_NM<Scalarm>::ScalarType *p_down_mesh_nv_buffer_ = nullptr;
    vcg::GL_TYPE_NM<Scalarm>::ScalarType *p_overlap_pix_buffer_ = nullptr;

    void allocateOverlapBuffer();         // Allocate memory
	void deleteOverlapBuffer();

	// bolton ana
	void showBoltonAnaResult(MeshDocument &md, GLArea *gla, QAction *act);

private:
	GLArea *parent_ = nullptr;
	QTranslator *translator_ = nullptr;
	ui_common::BoltonAnaDlg* p_bolton_dlg_ = nullptr;
	common_ext::BoltonAna* p_bolton_ana_ = nullptr;
	std::vector<Point3m> pos_picked_for_measure_;

	QAction* p_enamel_act_ = nullptr;
	QAction* p_overlap_act_ = nullptr;
};
#endif // DECORATEFUSIONPLUGIN_H
