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

#include "dentalanalysisdata.h"

bool DentalAnalysisData::crowding_analysis_6to6_mode_ = true;

void EditItem::drawCrevicePlane(TowableCtrlSystem& _sys, bool& _need_update_cut_outline, MeshModel* _p_model, CloudOctree* _p_tree)
{
	Point3m v, v_another;
	if (_sys.ctrl_nodes_[0]->picked_up_)
	{
		v = _sys.ctrl_nodes_[0]->p_;
		v_another = _sys.ctrl_nodes_[1]->p_;
	}
	else if (_sys.ctrl_nodes_[1]->picked_up_)
	{
		v = _sys.ctrl_nodes_[1]->p_;
		v_another = _sys.ctrl_nodes_[0]->p_;
	}
	else
	{
		return;
	}

	Point3m v0 = _sys.ctrl_nodes_[0]->p_;
	Point3m v1 = _sys.ctrl_nodes_[1]->p_;

	float fRadius = 10.0f;
	Point3m normal = (v1 - v0).Normalize();

	Point3m axis_x, axis_y;
	Point3m vx = v + Point3m(0, 1, 0);
	vx = UtilityTools::getInstance()->getProjPointOnPlane(vx, v, -normal);
	axis_x = (vx - v).Normalize();
	axis_y = (axis_x ^ normal).Normalize();

	CutFace face_a, face_b;
	face_a.initFace2(v, normal, axis_x, axis_y, 7);
	face_a.setFaceColor(Point4m(52 / 255.f, 187 / 255.f, 219 / 255.f, 0.7f));
	face_b.initFace2(v_another, normal, axis_x, axis_y, 7);
	face_b.setFaceColor(Point4m(52 / 255.f, 187 / 255.f, 219 / 255.f, 0.7f));

	if (_need_update_cut_outline && _p_model != nullptr && _p_tree != nullptr)
	{
		face_a.cutMeshModel(_p_model, _p_tree);
		face_b.cutMeshModel(_p_model, _p_tree);
	}

	face_a.drawCutFace();
	face_b.drawCutFace();
}

void EditItem::drawCrevicePolygonPlane(vector<Point3m>& _verts)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glColor4f(150 / 255.0f, 175 / 255.0f, 219 / 255.0f, 0.5f);
	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f(_verts[0].X(), _verts[0].Y(), _verts[0].Z());
	glVertex3f(_verts[1].X(), _verts[1].Y(), _verts[1].Z());
	glVertex3f(_verts[2].X(), _verts[2].Y(), _verts[2].Z());
	glEnd();
	glColor3f(1, 1, 1);
	glPopMatrix();

	glPushMatrix();
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glColor4f(150 / 255.0f, 175 / 255.0f, 219 / 255.0f, 0.5f);
	glBegin(GL_TRIANGLES);
	glVertex3f(_verts[2].X(), _verts[2].Y(), _verts[2].Z());
	glVertex3f(_verts[3].X(), _verts[3].Y(), _verts[3].Z());
	glVertex3f(_verts[0].X(), _verts[0].Y(), _verts[0].Z());
	glEnd();
	glColor3f(1, 1, 1);
	glPopMatrix();
	glPopAttrib();
}

void EditItem::drawEdge(FEdge& _edge, Point3m _color_a , Point3m _color_b )
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glLineWidth(4.0f);
	glColor4f(1, 1, 0, 1);
	glBegin(GL_LINES);
	glColor4f(_color_a.X(), _color_a.Y(), _color_a.Z(), 1);
	glVertex3f(_edge.vertA.X(), _edge.vertA.Y(), _edge.vertA.Z());
	glColor4f(_color_b.X(), _color_b.Y(), _color_b.Z(), 1);
	glVertex3f(_edge.vertB.X(), _edge.vertB.Y(), _edge.vertB.Z());
	glEnd();
	glLineWidth(1.0f);
	glColor3f(1, 1, 1);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glPopAttrib();
}

void EditItem::setScreenWidthAndHeight(float _width, float _height)
{
	this->screen_width_ = _width;
	this->screen_height_ = _height;
}

void EditItem::setCurrentItemVisibleMode(Interactive2DLabel::AccompanyMotionId _id)
{
	if (obj_list_.empty())
	{
		return;
	}

	int index = -1;
	if (Interactive2DLabel::FRONT_SHOW_ONLY_UPPER <= _id && _id <= Interactive2DLabel::BASALBONE_SHOW_ONLY_LOWER)
	{
		index = _id - Interactive2DLabel::FRONT_SHOW_ONLY_UPPER;
		index = index % 4;
	}
	else if (Interactive2DLabel::BASALBONE_WID_SHOW_ONLY_UPPER <= _id && _id <= Interactive2DLabel::BASALBONE_LEN_SHOW_ONLY_LOWER)
	{
		index = _id - Interactive2DLabel::BASALBONE_WID_SHOW_ONLY_UPPER;
		index = index % 2;
	}

	if (0 <= index && index < obj_list_.size())
	{
		for (int i = 0; i < obj_list_.size(); ++i)
		{
			if (obj_list_[i] != nullptr)
			{
				if (i == index)
				{
					obj_list_[i]->setVisible(true);
				}
				else
				{
					obj_list_[i]->setVisible(false);
				}
			}
		}
	}
}

void EditItem::addToObjectList(TowableCtrlSystem* _obj)
{
	this->obj_list_.push_back(_obj);
}

bool EditItem::mouseMove(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn )
{
	if (p_cur_obj_ != nullptr)
	{
		bool value = p_cur_obj_->mouseMove(_mouse_x, _mouse_y, _mouse_btn);
		if (feature_mode_)
		{
			updateFeatures();
		}
		else
		{
			update();
		}

		return value;
	}
	return false;
}

bool EditItem::mouseRelease(int _mouse_x, int _mouse_y)
{
	if (p_cur_obj_ != nullptr)
	{
		mouse_pressed_ = false;
		bool value = p_cur_obj_->mouseRelease(_mouse_x, _mouse_y);
		p_cur_obj_ = nullptr;
		return value;
	}
	return false;
}

MissingToothItem::MissingToothItem(Dental* _dental, vector<int>& _fdi_list, QString* _context)
    : EditItem(EditItemIndex(MISSING_TOOTH), _context)
{
	if (_dental->bUpperDental)
	{
		_fdi_list = {18, 17, 16, 15, 14, 13, 12, 11, 21, 22, 23, 24, 25, 26, 27, 28};
	}
	else
	{
		_fdi_list = { 48, 47, 46, 45, 44, 43, 42, 41, 31, 32, 33, 34, 35, 36, 37, 38 };
	}

	for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (_dental->bToothExist[i])
		{
			int fdi = _dental->teeth[i].strFDI.toInt();
			vector<int>::iterator ite = find(_fdi_list.begin(), _fdi_list.end(), fdi);
			if (ite != _fdi_list.end())
			{
				_fdi_list.erase(ite);
			}
		}
	}
	this->setPrompt(QString("Missing tooth"));
}

TeethWidthItem::TeethWidthItem(Dental* _dental, QString* _context)
    : EditItem(EditItemIndex(TEETH_WIDTH), _context)
{
	dental_ = _dental;
	initialUpdate();
	update();
	this->setPrompt(QString("Tooth Width"));
}

void TeethWidthItem::initialUpdate()
{
	vector<TowableCtrlSystem>().swap(ctrl_obj_list_);
	if (dental_ != nullptr)
	{
		for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
		{
			if (dental_->bToothExist[i])
			{
				int fdi = dental_->teeth[i].strFDI.toInt();
				if (fdi % 10 <= 8)
				{
					Point3m a, b, c, plane_normal;
					a = dental_->teeth[i].width_definition_point_a_;
					b = dental_->teeth[i].width_definition_point_b_;
					c = dental_->teeth[i].width_definition_point_c_;
					plane_normal = dental_->teeth[i].realTimeAxis.axisYVector;

					vector<Point3m> vert_list;
					vert_list.push_back(a);
					vert_list.push_back(b);

					Point3m visual_direct = (a + b) / 2.0f;
					visual_direct = (visual_direct - dental_->basePlane.center).Normalize();
					visual_direct -= dental_->basePlane.axisZV;
					visual_direct /= 2.0f;
					visual_direct = visual_direct.Normalize();

					TowableCtrlSystem ctrl_sys(i, vert_list, c, plane_normal, true);
					ctrl_sys.setScreenWidthAndHeight(screen_width_, screen_height_);
					ctrl_sys.setVisualAngleDirect(visual_direct);
					this->ctrl_obj_list_.push_back(ctrl_sys);
				}
			}
		}
	}
}

void TeethWidthItem::draw(QPainter *_p)
{
	if (!ctrl_obj_list_.empty())
	{
		for (int i = 0; i < ctrl_obj_list_.size(); ++i)
		{
			ctrl_obj_list_[i].draw();

			MeshModel* p_model = this->dental_feature_->mesh_->original_mesh_;
			if (ctrl_obj_list_[i].pickedHandle())
			{
				drawCrevicePlane(ctrl_obj_list_[i], mouse_pressed_, p_model, PFusionAlignData->mesh_cloud_Octree_[p_model]);
			}
		}
	}
}

void TeethWidthItem::update()
{
	if (p_cur_obj_ != nullptr && dental_ != nullptr)
	{
		p_cur_obj_->update();

		int itooth = p_cur_obj_->iNo_;
		Point3m a = p_cur_obj_->ctrl_nodes_[0]->p_;
		Point3m b = p_cur_obj_->ctrl_nodes_[1]->p_;
		Point3m c = dental_->teeth[itooth].width_definition_point_c_;
		dental_->setToothWidthDefVerts(itooth, a, b, c);
	}

	float length = 0;
	for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (dental_->bToothExist[i])
		{
			int fdi = dental_->teeth[i].strFDI.toInt();
			if (fdi % 10 <= 7)
			{
				length += dental_->teeth[i].fToothWidth;
			}
		}
	}
	int len = (length * 100.0f) / (int)1;
	float len2 = len / 100.0f;
	setContext(QString::number(len2));
}

bool TeethWidthItem::mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn )
{
	for (int i = 0; i < ctrl_obj_list_.size(); ++i)
	{
		if (ctrl_obj_list_[i].mousePress(_mouse_x, _mouse_y, _mouse_btn))
		{
			mouse_pressed_ = true;
			p_cur_obj_ = &ctrl_obj_list_[i];
			return true;
		}
	}
	p_cur_obj_ = nullptr;
	return false;
}

void TeethWidthItem::startEdit()
{
}

CurrentArchLengthItem::CurrentArchLengthItem(Dental* _dental, QString* _context, vector<Point3m>* _ctrlnodes)
    : EditItem(EditItemIndex(CUR_LENGTH_OF_DENTAL_ARCH), _context)
{
	dental_ = _dental;
	ctrlnodes_ = _ctrlnodes;
	bool got_tooth_marks = dental_->gotFeatureMarks();
	vector<Point3m> verts, verts2;

	if (ctrlnodes_ != nullptr && !ctrlnodes_->empty())
	{
		for (int i = 0; i < ctrlnodes_->size(); ++i)
		{
			verts.push_back(ctrlnodes_->at(i));
		}
	}
	else
	{
		int fdi_list[6];
		if (_dental->bUpperDental)
		{
			fdi_list[0] = 17;
			fdi_list[1] = 13;
			fdi_list[2] = 11;
			fdi_list[3] = 21;
			fdi_list[4] = 23;
			fdi_list[5] = 27;
		}
		else
		{
			fdi_list[0] = 37;
			fdi_list[1] = 33;
			fdi_list[2] = 31;
			fdi_list[3] = 41;
			fdi_list[4] = 43;
			fdi_list[5] = 47;
		}
		if (got_tooth_marks)
		{
			for (int ii = 0; ii < 6; ++ii)
			{
				int cur_fdi = fdi_list[ii];
				for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
				{
					if (_dental->bToothExist[i] && _dental->teeth[i].strFDI.toInt() == cur_fdi)
					{
						Point3m vert;
						Axis local_axis = _dental->teeth[i].localAxis;
						if (cur_fdi == 16 || cur_fdi == 26)
						{
							vert = _dental->teeth[i].feature_marks_[10];
						}
						else if (cur_fdi == 36 || cur_fdi == 46)
						{
							vert = _dental->teeth[i].feature_marks_[2];
						}
						else if (cur_fdi == 17 || cur_fdi == 27 || cur_fdi == 37 || cur_fdi == 47)
						{
							vert = _dental->teeth[i].feature_marks_[10];
						}
						else if (cur_fdi == 13 || cur_fdi == 23 || cur_fdi == 33 || cur_fdi == 43)
						{
							vert = _dental->teeth[i].feature_marks_[0];
						}
						else if (cur_fdi == 11 || cur_fdi == 21 || cur_fdi == 31 || cur_fdi == 41)
						{
							vert = _dental->teeth[i].feature_marks_[0];
						}
						verts.push_back(vert);
					}
				}
			}
		}
		else
		{
			for (int ii = 0; ii < 6; ++ii)
			{
				int cur_fdi = fdi_list[ii];
				for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
				{
					if (_dental->bToothExist[i] && _dental->teeth[i].strFDI.toInt() == cur_fdi)
					{
						Point3m vert;
						Axis local_axis = _dental->teeth[i].localAxis;
						if (cur_fdi == 16 || cur_fdi == 36 || cur_fdi == 17 || cur_fdi == 37)
						{
							vert = _dental->teeth[i].getDirectionalPeak(local_axis.axisXVector, local_axis.axisYVector, Point3m(0, 0, 0));
						}
						else if (cur_fdi == 26 || cur_fdi == 46 || cur_fdi == 27 || cur_fdi == 47)
						{
							vert = _dental->teeth[i].getDirectionalPeak(-local_axis.axisXVector, local_axis.axisYVector, Point3m(0, 0, 0));
						}
						else
						{
							vert = _dental->teeth[i].getDirectionalPeak(Point3m(0, 0, 0), local_axis.axisYVector, Point3m(0, 0, 0));
						}
						verts.push_back(vert);
					}
				}
			}
		}
	}

	if (verts.size() == 6)
	{
		verts2.push_back(verts[0]);
		verts2.push_back(verts[1]);
		verts2.push_back((verts[2] + verts[3]) * 0.5f);
		verts2.push_back(verts[4]);
		verts2.push_back(verts[5]);
		verts = verts2;
	}

	Point3m plane_pos = _dental->basePlane.center;
	Point3m plane_normal = _dental->basePlane.axisZV;
	vector<Point3m> proj_verts;
	for (int i = 0; i < verts.size(); ++i)
	{
		proj_verts.push_back(UtilityTools::getInstance()->getProjPointOnPlane(verts[i], plane_pos, -plane_normal));
	}

	this->ctrl_obj_ = new TowableCtrlSystem(0, proj_verts, plane_pos, plane_normal, false, true);
	this->setPrompt(QString("Crowding"));
	update();
}

CurrentArchLengthItem::~CurrentArchLengthItem()
{
	if (ctrl_obj_ != nullptr)
	{
		delete ctrl_obj_;
		ctrl_obj_ = nullptr;
	}
	p_cur_obj_ = nullptr;
}

void CurrentArchLengthItem::draw(QPainter *_p)
{
	if (ctrl_obj_ != nullptr)
	{
		ctrl_obj_->draw();
	}
	if (auxiliary_obj1_)
	{
		auxiliary_obj1_->draw();
	}

}

void CurrentArchLengthItem::update()
{
	if (ctrl_obj_)
	{
		ctrl_obj_->update();
	}

	int len = (ctrl_obj_->length_ * 100.0f) / (int)1;
	float len2 = len / 100.0f;
	setContext(QString::number(len2));
}

bool CurrentArchLengthItem::mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn )
{
	if (ctrl_obj_ != nullptr && ctrl_obj_->mousePress(_mouse_x, _mouse_y))
	{
		p_cur_obj_ = ctrl_obj_;
		return true;
	}
	p_cur_obj_ = nullptr;
	return false;
}

void CurrentArchLengthItem::updateCtrlParts()
{
	if (ctrlnodes_ != nullptr)
	{
		ctrlnodes_->clear();
		if (ctrl_obj_ != nullptr)
		{
			for (int i = 0; i < ctrl_obj_->ctrl_nodes_.size(); ++i)
			{
				ctrlnodes_->push_back(ctrl_obj_->ctrl_nodes_[i]->p_);
			}
		}
	}

	if (nodes_5_5_ != nullptr && six_to_six_)
	{
		nodes_5_5_->clear();
		if (ctrl_obj_ != nullptr)
		{
			for (int i = 0; i < ctrl_obj_->ctrl_nodes_.size(); ++i)
			{
				nodes_5_5_->push_back(ctrl_obj_->ctrl_nodes_[i]->p_);
			}
		}
	}
	if (nodes_7_7_ != nullptr && !six_to_six_)
	{
		nodes_7_7_->clear();
		if (ctrl_obj_ != nullptr)
		{
			for (int i = 0; i < ctrl_obj_->ctrl_nodes_.size(); ++i)
			{
				nodes_7_7_->push_back(ctrl_obj_->ctrl_nodes_[i]->p_);
			}
		}
	}
}

void CurrentArchLengthItem::startEdit()
{
}

BoltonItem::BoltonItem(Dental* _dental, QString* _context_anterior, QString* _context, QString* _context_complete, EditItemIndex _item_index, int _tooth_sign)
	: EditItem(_item_index, _context)
{
	dental_ = _dental;
	context_anterior_ = _context_anterior;
	context_complete_ = _context_complete;
	tooth_sign_range_ = _tooth_sign;
	initialUpdate();
	update();
	this->setPrompt(QString("Bolton Ratio"));
}

void BoltonItem::initialUpdate()
{
	if (dental_ == nullptr)
	{
		return;
	}
	vector<TowableCtrlSystem>().swap(ctrl_obj_list_);
	for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (dental_->bToothExist[i])
		{
			int fdi = dental_->teeth[i].strFDI.toInt();
			if (fdi % 10 <= tooth_sign_range_)
			{
				Point3m a, b, c, plane_normal;
				a = dental_->teeth[i].width_definition_point_a_;
				b = dental_->teeth[i].width_definition_point_b_;
				c = dental_->teeth[i].width_definition_point_c_;
				plane_normal = dental_->teeth[i].realTimeAxis.axisYVector;

				vector<Point3m> vert_list;
				vert_list.push_back(a);
				vert_list.push_back(b);

				Point3m visual_direct = (a + b) / 2.0f;
				visual_direct = (visual_direct - dental_->basePlane.center).Normalize();
				visual_direct -= dental_->basePlane.axisZV;
				visual_direct /= 2.0f;
				visual_direct = visual_direct.Normalize();

				TowableCtrlSystem ctrl_sys(i, vert_list, c, plane_normal, true);
				ctrl_sys.setScreenWidthAndHeight(screen_width_, screen_height_);
				ctrl_sys.setVisualAngleDirect(visual_direct);

				this->ctrl_obj_list_.push_back(ctrl_sys);
			}
		}
	}
}

void BoltonItem::draw(QPainter *_p)
{
	if (!ctrl_obj_list_.empty())
	{
		for (int i = 0; i < ctrl_obj_list_.size(); ++i)
		{
			ctrl_obj_list_[i].draw();

			MeshModel* p_model = this->dental_feature_->mesh_->original_mesh_;
			if (ctrl_obj_list_[i].pickedHandle())
			{
				drawCrevicePlane(ctrl_obj_list_[i], mouse_pressed_, p_model, PFusionAlignData->mesh_cloud_Octree_[p_model]);
			}
		}
	}
}

void BoltonItem::update()
{
	if (p_cur_obj_ != nullptr)
	{
		p_cur_obj_->update();

		int itooth = p_cur_obj_->iNo_;
		if (p_cur_obj_->ctrl_nodes_.size() >= 2)
		{
			Point3m a = p_cur_obj_->ctrl_nodes_[0]->p_;
			Point3m b = p_cur_obj_->ctrl_nodes_[1]->p_;
			Point3m c = dental_->teeth[itooth].width_definition_point_c_;
			dental_->setToothWidthDefVerts(itooth, a, b, c);
		}
		else
		{
			qDebug() << "error:ctrl node defect" << endl;
		}
	}

	float length = 0;
	for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (dental_->bToothExist[i])
		{
			int fdi = dental_->teeth[i].strFDI.toInt();
			if (fdi % 10 <= 6)
			{
				length += dental_->teeth[i].fToothWidth;
			}
		}
	}
	int len = (length * 100) / (int)1;
	float len2 = len / 100.0f;
	setContext(QString::number(len2));

	length = 0;
	for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (dental_->bToothExist[i])
		{
			int fdi = dental_->teeth[i].strFDI.toInt();
			if (fdi % 10 <= 7)
			{
				length += dental_->teeth[i].fToothWidth;
			}
		}
	}
	len = (length * 100) / (int)1;
	len2 = len / 100.0f;
	if (context_complete_ != nullptr)
	{
		*context_complete_ = QString::number(len2);
	}

	length = 0;
	for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (dental_->bToothExist[i])
		{
			int fdi = dental_->teeth[i].strFDI.toInt();
			if (fdi % 10 <= 3)
			{
				length += dental_->teeth[i].fToothWidth;
			}
		}
	}
	len = (length * 100.0f) / (int)1;
	len2 = len / 100.0f;
	if (context_anterior_ != nullptr)
	{
		*context_anterior_ = QString::number(len2);
	}
}

bool BoltonItem::mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn )
{
	for (int i = 0; i < ctrl_obj_list_.size(); ++i)
	{
		if (ctrl_obj_list_[i].mousePress(_mouse_x, _mouse_y))
		{
			mouse_pressed_ = true;
			p_cur_obj_ = &ctrl_obj_list_[i];
			return true;
		}
	}
	p_cur_obj_ = nullptr;
	return false;
}

void BoltonItem::startEdit()
{
}

SpeeCurveDepthItem::SpeeCurveDepthItem(Dental* _dental, float* _left_deepth, float* _right_deepth, QString* _context, vector<Point3m>* _ctrlnodes)
    :EditItem(EditItemIndex(SPEE_CURVE_DEPTH), _context)
{
	dental_ = _dental;
	ctrlnodes_ = _ctrlnodes;
	left_deepth_ = _left_deepth;
	right_deepth_ = _right_deepth;
	bool got_tooth_marks = dental_->gotFeatureMarks();

	vector<pair<Point3m, vector<CMeshO*>>> left_node_infos, right_node_infos;
	Point3m vert;
	int cur_fdi, tooth_sign;

	int left_indexes[3] = { 31, 34, 37 };
	for (int ii = 0; ii < 3; ++ii)
	{
		cur_fdi = left_indexes[ii];
		tooth_sign = cur_fdi % 10;

		if (ctrlnodes_ != nullptr && !ctrlnodes_->empty())
		{
			for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
			{
				if (_dental->bToothExist[i] && _dental->teeth[i].strFDI.toInt() == cur_fdi)
				{
					vert = ctrlnodes_->at(ii);
					vector<CMeshO*> mesh_list;
					mesh_list.push_back(_dental->teeth[i].p_mesh_);
					if (tooth_sign == 4)
					{
						for (int ii = 1; ii < STANDARD_TOOTH_SUM; ++ii)
						{
							if (_dental->bToothExist[ii] && _dental->teeth[ii].strFDI.toInt() == 35)
							{
								mesh_list.push_back(_dental->teeth[ii].p_mesh_);
							}
						}
					}

					left_node_infos.push_back(make_pair(vert, mesh_list));
				}
			}
		}
		else
		{
			for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
			{
				if (_dental->bToothExist[i] && _dental->teeth[i].strFDI.toInt() == cur_fdi)
				{
					if (!got_tooth_marks)
					{
						if (tooth_sign == 1 || tooth_sign == 2 || tooth_sign == 3 || tooth_sign == 4 || tooth_sign == 5)
						{
							vert = _dental->teeth[i].getDirectionalPeak(Point3m(0, 0, 0), _dental->teeth[i].localAxis.axisYVector, _dental->teeth[i].localAxis.axisZVector);
						}
						else if (tooth_sign == 6 || tooth_sign == 7)
						{
							vert = _dental->teeth[i].getDirectionalPeak(_dental->teeth[i].localAxis.axisXVector, _dental->teeth[i].localAxis.axisYVector, _dental->teeth[i].localAxis.axisZVector);
						}
					}
					else
					{
						if (tooth_sign == 1 || tooth_sign == 2 || tooth_sign == 3 || tooth_sign == 4 || tooth_sign == 5)
						{
							vert = _dental->teeth[i].feature_marks_[0];
						}
						else if (tooth_sign == 6 || tooth_sign == 7)
						{
							vert = _dental->teeth[i].feature_marks_[1];
						}
					}

					vector<CMeshO*> mesh_list;
					mesh_list.push_back(_dental->teeth[i].p_mesh_);
					if (tooth_sign == 4)
					{
						for (int ii = 1; ii < STANDARD_TOOTH_SUM; ++ii)
						{
							if (_dental->bToothExist[ii] && _dental->teeth[ii].strFDI.toInt() == 35)
							{
								mesh_list.push_back(_dental->teeth[ii].p_mesh_);
							}
						}
					}

					left_node_infos.push_back(make_pair(vert, mesh_list));
				}
			}
		}
	}
	this->left_ope_obj_ = new TowableCtrlSystem(0, left_node_infos, true);
	this->left_ope_obj_->setScreenWidthAndHeight(screen_width_, screen_height_);
	this->left_ope_obj_->setVisualAngleDirect(_dental->basePlane.axisXV);
	this->left_vec_ = _dental->basePlane.axisXV;

	int right_indexes[3] = { 41, 44, 47 };
	for (int ii = 0; ii < 3; ++ii)
	{
		cur_fdi = right_indexes[ii];
		tooth_sign = cur_fdi % 10;

		if (ctrlnodes_ != nullptr && !ctrlnodes_->empty())
		{
			for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
			{
				if (_dental->bToothExist[i] && _dental->teeth[i].strFDI.toInt() == cur_fdi)
				{
					vert = ctrlnodes_->at(ii + 3);
					vector<CMeshO*> mesh_list;
					mesh_list.push_back(_dental->teeth[i].p_mesh_);
					if (tooth_sign == 4)
					{
						for (int ii = 1; ii < STANDARD_TOOTH_SUM; ++ii)
						{
							if (_dental->bToothExist[ii] && _dental->teeth[ii].strFDI.toInt() == 45)
							{
								mesh_list.push_back(_dental->teeth[ii].p_mesh_);
							}
						}
					}

					right_node_infos.push_back(make_pair(vert, mesh_list));
				}
			}
		}
		else
		{
			for (int i = 1; i < STANDARD_TOOTH_SUM; ++i)
			{
				if (_dental->bToothExist[i] && _dental->teeth[i].strFDI.toInt() == cur_fdi)
				{
					if (!got_tooth_marks)
					{
						if (tooth_sign == 1 || tooth_sign == 2 || tooth_sign == 3 || tooth_sign == 4 || tooth_sign == 5)
						{
							vert = _dental->teeth[i].getDirectionalPeak(Point3m(0, 0, 0), _dental->teeth[i].localAxis.axisYVector, _dental->teeth[i].localAxis.axisZVector);
						}
						else if (tooth_sign == 6 || tooth_sign == 7)
						{
							vert = _dental->teeth[i].getDirectionalPeak(-_dental->teeth[i].localAxis.axisXVector, _dental->teeth[i].localAxis.axisYVector, _dental->teeth[i].localAxis.axisZVector);
						}
					}
					else
					{
						if (tooth_sign == 1 || tooth_sign == 2 || tooth_sign == 3 || tooth_sign == 4 || tooth_sign == 5)
						{
							vert = _dental->teeth[i].feature_marks_[0];
						}
						else if (tooth_sign == 6 || tooth_sign == 7)
						{
							vert = _dental->teeth[i].feature_marks_[1];
						}
					}

					vector<CMeshO*> mesh_list;
					mesh_list.push_back(_dental->teeth[i].p_mesh_);
					if (tooth_sign == 4)
					{
						for (int ii = 1; ii < STANDARD_TOOTH_SUM; ++ii)
						{
							if (_dental->bToothExist[ii] && _dental->teeth[ii].strFDI.toInt() == 45)
							{
								mesh_list.push_back(_dental->teeth[ii].p_mesh_);
							}
						}
					}

					right_node_infos.push_back(make_pair(vert, mesh_list));
				}
			}
		}
	}
	this->right_ope_obj_ = new TowableCtrlSystem(0, right_node_infos, true);
	this->right_ope_obj_->setScreenWidthAndHeight(screen_width_, screen_height_);
	this->right_ope_obj_->setVisualAngleDirect(-_dental->basePlane.axisXV);
	this->right_vec_ = _dental->basePlane.axisXV;
	this->setPrompt(QString("Spee"));
	update();
}

SpeeCurveDepthItem::~SpeeCurveDepthItem()
{
	if (left_ope_obj_ != nullptr)
	{
		delete left_ope_obj_;
		left_ope_obj_ = nullptr;
	}
	if (right_ope_obj_ != nullptr)
	{
		delete right_ope_obj_;
		right_ope_obj_ = nullptr;
	}
}

void SpeeCurveDepthItem::draw(QPainter *_p)
{
	if (left_ope_obj_ != nullptr)
	{
		left_ope_obj_->draw();

		if (left_ope_obj_->visible_)
		{
			drawEdge(left_deep_edge_, Point3m(0.48f, 0.48f, 0.48f), Point3m(0.48f, 0.48f, 0.48f));
		}
	}

	if (right_ope_obj_ != nullptr)
	{
		right_ope_obj_->draw();

		if (right_ope_obj_->visible_)
		{
			drawEdge(right_deep_edge_, Point3m(0.48f, 0.48f, 0.48f), Point3m(0.48f, 0.48f, 0.48f));
		}
	}

	spee_plane_.drawCutFace();
}

void SpeeCurveDepthItem::update()
{
	int deep = 0;
	float deep_value = 0, deep2 = 0;
	if (left_ope_obj_ != nullptr && left_ope_obj_->ctrl_nodes_.size() >= 3)
	{
		left_ope_obj_->update();
		Point3m pos = (left_ope_obj_->ctrl_nodes_[0]->p_ + left_ope_obj_->ctrl_nodes_[2]->p_) * 0.5f;
		Point3m vec_x = (left_ope_obj_->ctrl_nodes_[0]->p_ - left_ope_obj_->ctrl_nodes_[2]->p_).Normalize();
		left_deep_edge_ = fliterDeepestEdge(deep_value, left_ope_obj_->edges_, pos, vec_x, left_vec_);
	}
	deep = (deep_value * 100.0f) / (int)1;
	deep2 = deep / 100.0f;
	if (left_deepth_ != nullptr)
	{
		*left_deepth_ = deep2;
	}

	if (right_ope_obj_ != nullptr && right_ope_obj_->ctrl_nodes_.size() >= 3)
	{
		right_ope_obj_->update();
		Point3m pos = (right_ope_obj_->ctrl_nodes_[0]->p_ + right_ope_obj_->ctrl_nodes_[2]->p_) * 0.5f;
		Point3m vec_x = (right_ope_obj_->ctrl_nodes_[0]->p_ - right_ope_obj_->ctrl_nodes_[2]->p_).Normalize();
		right_deep_edge_ = fliterDeepestEdge(deep_value, right_ope_obj_->edges_, pos, vec_x, right_vec_);
	}
	deep = (deep_value * 100.0f) / (int)1;
	deep2 = deep / 100.0f;
	if (right_deepth_ != nullptr)
	{
		*right_deepth_ = deep2;
	}
}

bool SpeeCurveDepthItem::mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn )
{
	if (left_ope_obj_ != nullptr && left_ope_obj_->mousePress(_mouse_x, _mouse_y))
	{
		p_cur_obj_ = left_ope_obj_;
		return true;
	}
	if (right_ope_obj_ != nullptr && right_ope_obj_->mousePress(_mouse_x, _mouse_y))
	{
		p_cur_obj_ = right_ope_obj_;
		return true;
	}
	p_cur_obj_ = nullptr;
	return false;
}

FEdge SpeeCurveDepthItem::fliterDeepestEdge(float& _depth, vector<FEdge>& _edges, const Point3m& _pos, const Point3m& _axis_x, const Point3m& _axis_y)
{
	Point3m normal = (_axis_x ^ _axis_y).Normalize();
	float deepest_value = 0;
	FEdge deepest_edge;
	for (auto& edge : _edges)
	{
		Point3m vert = edge.vertA;
		Point3m proj_vert = UtilityTools::getInstance()->getProjPointOnPlane(vert, _pos, -normal);
		FEdge cur_edge(vert, proj_vert);
		Point3m cur_vec = proj_vert - vert;
		float length = cur_vec * normal;
		if (abs(length) > deepest_value)
		{
			deepest_value = abs(length);
			deepest_edge = cur_edge;
			_depth = length;
		}
	}
	return deepest_edge;
}

void SpeeCurveDepthItem::updateCtrlParts()
{
	if (ctrlnodes_ != nullptr)
	{
		ctrlnodes_->clear();
		if (left_ope_obj_ != nullptr && right_ope_obj_ != nullptr)
		{
			for (int i = 0; i < left_ope_obj_->ctrl_nodes_.size(); ++i)
			{
				ctrlnodes_->push_back(left_ope_obj_->ctrl_nodes_[i]->p_);
			}
			for (int i = 0; i < right_ope_obj_->ctrl_nodes_.size(); ++i)
			{
				ctrlnodes_->push_back(right_ope_obj_->ctrl_nodes_[i]->p_);
			}
		}
	}
}

void SpeeCurveDepthItem::startEdit()
{
}

MolarRelationshipItem::MolarRelationshipItem(QString* _context, Point3m* _left_node, Point3m* _right_node, DentalManager* _dental_manager, vector<FEdge>* _ctrledges)
    :EditItem(EditItemIndex(MOLAR_RELATIONSHIP), _context)
{
	left_node_ = _left_node;
	right_node_ = _right_node;
	ctrledges_ = _ctrledges;
	bool got_tooth_marks = _dental_manager->cDental.gotFeatureMarks();
	vector<Point3m> vert_list;
	vector<CMeshO*> mesh_list;
	Tooth* temp_tooth = nullptr;
	if (_dental_manager->cDental.bUpperDental)
	{
		for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
		{
			if (_dental_manager->cDental.bToothExist[i])
			{
				Point3m peak_node, gum_node;
				int fdi = _dental_manager->cDental.teeth[i].strFDI.toInt();
				if (fdi == 26)
				{
					temp_tooth = &_dental_manager->cDental.teeth[i];

					if(ctrledges_ && !ctrledges_->empty())
					{
						peak_node = ctrledges_->at(0).vertA;
						gum_node = ctrledges_->at(0).vertB;
					}
					else
					{
						Axis local_axis = temp_tooth->localAxis;
						if (got_tooth_marks)
						{
							peak_node = temp_tooth->feature_marks_[0];
							gum_node = temp_tooth->feature_marks_[7];
						}
						else
						{
							peak_node = temp_tooth->getDirectionalPeak(local_axis.axisXVector, local_axis.axisYVector, local_axis.axisZVector, false);
							gum_node = temp_tooth->getDirectionalPeak(local_axis.axisXVector, Point3m(0, 0, 0), local_axis.axisZVector);
						}
					}

					vector<pair<Point3m, vector<CMeshO*>>> node_info;
					vector<CMeshO*> mesh_list;
					mesh_list.push_back(temp_tooth->p_mesh_);
					node_info.push_back(make_pair(peak_node, mesh_list));
					left_obj_ = new TowableCtrlSystem(0, node_info);
					left_direct_ = -temp_tooth->localAxis.axisYVector;
					left_view_direct_ = temp_tooth->localAxis.axisZVector;
				}
				else if (fdi == 16)
				{
					temp_tooth = &_dental_manager->cDental.teeth[i];

					if (ctrledges_ && !ctrledges_->empty())
					{
						peak_node = ctrledges_->at(1).vertA;
						gum_node = ctrledges_->at(1).vertB;
					}
					else
					{
						Axis local_axis = temp_tooth->localAxis;
						if (got_tooth_marks)
						{
							peak_node = temp_tooth->feature_marks_[0];
							gum_node = temp_tooth->feature_marks_[7];
						}
						else
						{
							peak_node = temp_tooth->getDirectionalPeak(-local_axis.axisXVector, local_axis.axisYVector, local_axis.axisZVector, false);
							gum_node = temp_tooth->getDirectionalPeak(-local_axis.axisXVector, Point3m(0, 0, 0), local_axis.axisZVector);
						}
					}

					vector<pair<Point3m, vector<CMeshO*>>> node_info;
					vector<CMeshO*> mesh_list;
					mesh_list.push_back(temp_tooth->p_mesh_);
					node_info.push_back(make_pair(peak_node, mesh_list));
					right_obj_ = new TowableCtrlSystem(1, node_info);
					right_direct_ = -temp_tooth->localAxis.axisYVector;
					right_view_direct_ = temp_tooth->localAxis.axisZVector;
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
		{
			if (_dental_manager->cDental.bToothExist[i])
			{
				Point3m peak_node, gum_node;
				int fdi = _dental_manager->cDental.teeth[i].strFDI.toInt();
				if (fdi == 36)
				{
					temp_tooth = &_dental_manager->cDental.teeth[i];

					if (ctrledges_ && !ctrledges_->empty())
					{
						peak_node = ctrledges_->at(0).vertA;
						gum_node = ctrledges_->at(0).vertB;
					}
					else
					{
						Axis local_axis = temp_tooth->localAxis;
						if (got_tooth_marks)
						{
							peak_node = temp_tooth->feature_marks_[5];
							gum_node = temp_tooth->feature_marks_[9];
						}
						else
						{
							peak_node = temp_tooth->getDirectionalPeak(Point3m(0, 0, 0), local_axis.axisYVector, local_axis.axisZVector, false);
							gum_node = temp_tooth->getDirectionalPeak(Point3m(0, 0, 0), Point3m(0, 0, 0), local_axis.axisZVector);
						}
					}

					vector<pair<Point3m, vector<CMeshO*>>> node_info;
					vector<CMeshO*> mesh_list;
					mesh_list.push_back(temp_tooth->p_mesh_);
					node_info.push_back(make_pair(peak_node, mesh_list));
					left_obj_ = new TowableCtrlSystem(0, node_info);
					left_direct_ = -temp_tooth->localAxis.axisYVector;
					left_view_direct_ = temp_tooth->localAxis.axisZVector;
				}
				else if (fdi == 46)
				{
					temp_tooth = &_dental_manager->cDental.teeth[i];

					if (ctrledges_ && !ctrledges_->empty())
					{
						peak_node = ctrledges_->at(1).vertA;
						gum_node = ctrledges_->at(1).vertB;
					}
					else
					{
						Axis local_axis = temp_tooth->localAxis;
						if (got_tooth_marks)
						{
							peak_node = temp_tooth->feature_marks_[5];
							gum_node = temp_tooth->feature_marks_[9];
						}
						else
						{
							peak_node = temp_tooth->getDirectionalPeak(Point3m(0, 0, 0), local_axis.axisYVector, local_axis.axisZVector, false);
							gum_node = temp_tooth->getDirectionalPeak(Point3m(0, 0, 0), Point3m(0, 0, 0), local_axis.axisZVector);
						}
					}

					vector<pair<Point3m, vector<CMeshO*>>> node_info;
					vector<CMeshO*> mesh_list;
					mesh_list.push_back(temp_tooth->p_mesh_);
					node_info.push_back(make_pair(peak_node, mesh_list));
					right_obj_ = new TowableCtrlSystem(1, node_info);
					right_direct_ = -temp_tooth->localAxis.axisYVector;
					right_view_direct_ = temp_tooth->localAxis.axisZVector;
				}
			}
		}
	}

	this->setPrompt(QString("Molar REL"));
	update();
}

MolarRelationshipItem::~MolarRelationshipItem()
{
	if (left_obj_ != nullptr)
	{
		delete left_obj_;
		left_obj_ = nullptr;
	}

	if (right_obj_ != nullptr)
	{
		delete right_obj_;
		right_obj_ = nullptr;
	}
	p_cur_obj_ = nullptr;
}

void MolarRelationshipItem::draw(QPainter *_p)
{
	Point3m cam_view, cam_pos;
	UtilityTools::getInstance()->getCurrentCameraState(nullptr, this->screen_width_, this->screen_height_, cam_view, cam_pos);

	if (right_obj_ != nullptr && left_obj_ != nullptr)
	{
		Point3m left_vec = left_obj_->ctrl_nodes_[0]->p_ - cam_pos;
		Point3m right_vec = right_obj_->ctrl_nodes_[0]->p_ - cam_pos;
		float left_distance = left_vec * cam_view;
		float right_distance = right_vec * cam_view;
		if (left_distance <= right_distance)
		{
			left_obj_->draw();
			drawEdge(left_edge_, Point3m(0.48f, 0.48f, 0.48f), Point3m(0.48f, 0.48f, 0.48f));
		}
		else
		{
			right_obj_->draw();
			drawEdge(right_edge_, Point3m(0.48f, 0.48f, 0.48f), Point3m(0.48f, 0.48f, 0.48f));
		}
	}
	else
	{
		if (left_obj_ != nullptr)
		{
			left_obj_->draw();
			drawEdge(left_edge_, Point3m(0.48f, 0.48f, 0.48f), Point3m(0.48f, 0.48f, 0.48f));
		}
		if (right_obj_ != nullptr)
		{
			right_obj_->draw();
			drawEdge(right_edge_, Point3m(0.48f, 0.48f, 0.48f), Point3m(0.48f, 0.48f, 0.48f));
		}
	}
}

void MolarRelationshipItem::update()
{
	if (left_node_ != nullptr && left_obj_ != nullptr)
	{
		*left_node_ = left_obj_->ctrl_nodes_[0]->p_;
		left_edge_ = FEdge(*left_node_, *left_node_ + left_direct_ * 5.0f);
	}
	if (right_node_ != nullptr && right_obj_ != nullptr)
	{
		*right_node_ = right_obj_->ctrl_nodes_[0]->p_;
		right_edge_ = FEdge(*right_node_, *right_node_ + right_direct_ * 5.0f);
	}
}

bool MolarRelationshipItem::mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn )
{
	if (left_obj_ != nullptr && left_obj_->mousePress(_mouse_x, _mouse_y))
	{
		p_cur_obj_ = left_obj_;
		return true;
	}
	else if (right_obj_ != nullptr && right_obj_->mousePress(_mouse_x, _mouse_y))
	{
		p_cur_obj_ = right_obj_;
		return true;
	}
	p_cur_obj_ = nullptr;
	return false;
}

void MolarRelationshipItem::updateCtrlParts()
{
	if (ctrledges_ != nullptr && left_obj_ != nullptr && right_obj_ != nullptr)
	{
		ctrledges_->clear();
		Point3m a, b;

		a = left_obj_->ctrl_nodes_[0]->p_;
		b = a;
		ctrledges_->push_back(FEdge(a, b));

		a = right_obj_->ctrl_nodes_[0]->p_;
		b = a;
		ctrledges_->push_back(FEdge(a, b));
	}
}

void MolarRelationshipItem::startEdit()
{
}

MidlineRelationshipItem::MidlineRelationshipItem(QString* _context, DentalManager* _dental_manager, Axis* _plane, vector<FEdge>* _ctrledges)
    : EditItem(EditItemIndex(MIDLINE_RELATIONSHIP), _context)
{
	first_initial_ = true;
	ctrledges_ = _ctrledges;
	plane_ = _plane;
	Point3m plane_pos, proj_plane_pos, plane_normal, extand_vec, proj_ha;
	vector<Point3m> vert_list;
	if (_dental_manager->cDental.bUpperDental)
	{
		int find_time = 0;
		plane_pos = Point3m(0, 0, 0);
		for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
		{
			if (_dental_manager->cDental.bToothExist[i])
			{
				int fdi = _dental_manager->cDental.teeth[i].strFDI.toInt();
				if (fdi == 11 || fdi == 21)
				{
					Point3m vert = _dental_manager->cDental.teeth[i].localAxis.centerPoint;
					vert += _dental_manager->cDental.teeth[i].localAxis.axisZVector;
					plane_pos += vert;
					find_time += 1;
				}
			}
		}
		if (find_time > 0)
		{
			if (find_time == 2)
			{
				plane_pos *= 0.5f;
			}

			proj_ha = _dental_manager->cDental.basePlane.center + _dental_manager->cDental.basePlane.axisZV * 10.0f;
			proj_plane_pos = UtilityTools::getInstance()->getProjPointOnPlane(plane_pos, proj_ha, -_dental_manager->cDental.basePlane.axisZV);
			occlusal_plane_normal = _dental_manager->cDental.basePlane.axisZV;

			if (ctrledges_ && !ctrledges_->empty())
			{
				vert_list.push_back(ctrledges_->at(0).vertA);
				vert_list.push_back(ctrledges_->at(0).vertB);
			}
			else
			{
				vert_list.push_back(plane_pos);
				vert_list.push_back(proj_plane_pos);
			}

			obj_ = new TowableCtrlSystem(0, vert_list, plane_pos, plane_normal);
			obj_->initialTowableCtrlEdges();
		}
	}
	else
	{
		int find_time = 0;
		plane_pos = Point3m(0, 0, 0);
		for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
		{
			if (_dental_manager->cDental.bToothExist[i])
			{
				int fdi = _dental_manager->cDental.teeth[i].strFDI.toInt();
				if (fdi == 31 || fdi == 41)
				{
					Point3m vert = _dental_manager->cDental.teeth[i].localAxis.centerPoint;
					vert += _dental_manager->cDental.teeth[i].localAxis.axisZVector;
					plane_pos += vert;
					find_time += 1;
				}
			}
		}
		if (find_time > 0)
		{
			if (find_time == 2)
			{
				plane_pos *= 0.5f;
			}

			proj_ha = _dental_manager->cDental.basePlane.center + _dental_manager->cDental.basePlane.axisZV * 10.0f;
			proj_plane_pos = UtilityTools::getInstance()->getProjPointOnPlane(plane_pos, proj_ha, -_dental_manager->cDental.basePlane.axisZV);
			occlusal_plane_normal = _dental_manager->cDental.basePlane.axisZV;

			if (ctrledges_ && !ctrledges_->empty())
			{
				vert_list.push_back(ctrledges_->at(0).vertA);
				vert_list.push_back(ctrledges_->at(0).vertB);
			}
			else
			{
				vert_list.push_back(plane_pos);
				vert_list.push_back(proj_plane_pos);
			}

			obj_ = new TowableCtrlSystem(1, vert_list, plane_pos, plane_normal);
			obj_->initialTowableCtrlEdges();
		}
	}
	first_initial_direct_ = -_dental_manager->cDental.basePlane.axisYV;
	this->setPrompt(QString("Centerline REL"));
	update();
}

MidlineRelationshipItem::~MidlineRelationshipItem()
{
	if (obj_ != nullptr)
	{
		delete obj_;
		obj_ = nullptr;
	}
	p_cur_obj_ = nullptr;
}

void MidlineRelationshipItem::draw(QPainter *_p)
{
	if (obj_ != nullptr)
	{
		obj_->draw();
		drawCrevicePolygonPlane(plane_verts_);
	}
}

void MidlineRelationshipItem::update()
{
	if (obj_ != nullptr)
	{
		updatePlane(obj_, plane_, plane_verts_);
	}
}

bool MidlineRelationshipItem::mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn )
{
	if (obj_ != nullptr && obj_->mousePress(_mouse_x, _mouse_y))
	{
		p_cur_obj_ = obj_;
		first_initial_ = false;
		return true;
	}
	p_cur_obj_ = nullptr;
	return false;
}

void MidlineRelationshipItem::updatePlane(TowableCtrlSystem* _obj, Axis* _axis, vector<Point3m>& _plane_verts)
{
	Point3m cam_view, cam_pos;
	UtilityTools::getInstance()->getCurrentCameraState(nullptr, this->screen_width_, this->screen_height_, cam_view, cam_pos);

	Point3m edge_vec = _obj->ctrl_nodes_[0]->p_ - _obj->ctrl_nodes_[1]->p_;
	Point3m occlusal_nomral;

	Point3m v0, v1;
	if (edge_vec * occlusal_plane_normal > 0)
	{
		v0 = _obj->ctrl_nodes_[0]->p_;
		v1 = _obj->ctrl_nodes_[1]->p_;
	}
	else
	{
		v0 = _obj->ctrl_nodes_[1]->p_;
		v1 = _obj->ctrl_nodes_[0]->p_;
	}

	Point3m axis_z;
	Point3m axis_y = (v0 - v1).Normalize();
	if (first_initial_)
	{
		axis_z = first_initial_direct_;
	}
	else
	{
		axis_z = (cam_view).Normalize();
	}
	Point3m axis_x = (axis_y ^ axis_z).Normalize();
	Point3m center = (v0 + v1) * 0.5f;
	*_axis = Axis(center, axis_x, axis_y, axis_z);

	_plane_verts.clear();
	Point3m v2 = v0 + axis_z * 5.0f;
	Point3m v3 = v1 + axis_z * 5.0f;
	_plane_verts.push_back(v0 - axis_z * 5.0f);
	_plane_verts.push_back(v1 - axis_z * 5.0f);
	_plane_verts.push_back(v1 + axis_z * 5.0f);
	_plane_verts.push_back(v0 + axis_z * 5.0f);
}

void MidlineRelationshipItem::updateCtrlParts()
{
	if (ctrledges_ != nullptr && obj_ != nullptr)
	{
		ctrledges_->clear();
		Point3m a, b;

		a = obj_->ctrl_nodes_[0]->p_;
		b = obj_->ctrl_nodes_[1]->p_;
		ctrledges_->push_back(FEdge(a, b));
	}
}

void MidlineRelationshipItem::startEdit()
{
}

ArchWidthItem::ArchWidthItem(Dental* _dental, float* _front_value, float* _middle_value, float* _rear_value, float* _basalbone_value, QString* _context, vector<FEdge>* _ctrledges)
    :EditItem(EditItemIndex(ARCH_WIDTH), _context)
{
	dental_ = _dental;
	ctrledges_ = _ctrledges;
	front_value_ = _front_value;
	middle_value_ = _middle_value;
	rear_value_ = _rear_value;
	basalbone_value_ = _basalbone_value;
	bool got_tooth_marks = dental_->gotFeatureMarks();

	int fdi, tooth_sign;
	Point3m vertA, vertB;
	CMeshO* meshA = nullptr, * meshB = nullptr;
	bool find_nodeA, find_nodeB;

	find_nodeA = false;  find_nodeB = false;
	meshA = nullptr; meshB = nullptr;
	for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (dental_->bToothExist[i])
		{
			fdi = dental_->teeth[i].strFDI.toInt();
			tooth_sign = fdi % 10;
			if (tooth_sign == 3)
			{
				Point3m vert;
				if (got_tooth_marks)
				{
					vert = dental_->teeth[i].feature_marks_[2];
				}
				else
				{
					Axis local_axis = dental_->teeth[i].localAxis;
					vert = dental_->teeth[i].getDirectionalPeak(Point3m(0, 0, 0), local_axis.axisYVector, local_axis.axisZVector);
				}
				if (!find_nodeA)
				{
					vertA = vert;
					meshA = dental_->teeth[i].p_mesh_;
					find_nodeA = true;
				}
				else if (!find_nodeB)
				{
					vertB = vert;
					meshB = dental_->teeth[i].p_mesh_;
					find_nodeB = true;
				}
			}
		}
	}
	if (ctrledges_ && !ctrledges_->empty())
	{
		vertA = ctrledges_->at(0).vertA;
		vertB = ctrledges_->at(0).vertB;
	}
	if (find_nodeA && find_nodeB && meshA && meshB)
	{
		this->front_obj_ = new TowableCtrlSystem(0, vertA, meshA, vertB, meshB);
	}

	find_nodeA = false;  find_nodeB = false;
	meshA = nullptr; meshB = nullptr;
	for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (dental_->bToothExist[i])
		{
			fdi = dental_->teeth[i].strFDI.toInt();
			tooth_sign = fdi % 10;
			if (tooth_sign == 4)
			{
				Point3m vert;
				if (got_tooth_marks)
				{
					vert = (dental_->teeth[i].feature_marks_[2] + dental_->teeth[i].feature_marks_[3]) * 0.5f;
				}
				else
				{
					Axis local_axis = dental_->teeth[i].localAxis;
					vert = dental_->teeth[i].getDirectionalPeak(Point3m(0, 0, 0), local_axis.axisYVector, Point3m(0, 0, 0));
				}
				if (!find_nodeA)
				{
					vertA = vert;
					meshA = dental_->teeth[i].p_mesh_;
					find_nodeA = true;
				}
				else if (!find_nodeB)
				{
					vertB = vert;
					meshB = dental_->teeth[i].p_mesh_;
					find_nodeB = true;
				}
			}
		}
	}
	if (ctrledges_ && !ctrledges_->empty())
	{
		vertA = ctrledges_->at(1).vertA;
		vertB = ctrledges_->at(1).vertB;
	}
	if (find_nodeA && find_nodeB && meshA && meshB)
	{
		this->middle_obj_ = new TowableCtrlSystem(1, vertA, meshA, vertB, meshB);
	}

	find_nodeA = false;  find_nodeB = false;
	meshA = nullptr; meshB = nullptr;
	for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (dental_->bToothExist[i])
		{
			fdi = dental_->teeth[i].strFDI.toInt();
			tooth_sign = fdi % 10;
			if (tooth_sign == 6)
			{
				Point3m vert;
				if (got_tooth_marks)
				{
					vert = dental_->teeth[i].feature_marks_[6];
				}
				else
				{
					Axis local_axis = dental_->teeth[i].localAxis;
					vert = dental_->teeth[i].getDirectionalPeak(Point3m(0, 0, 0), local_axis.axisYVector, Point3m(0, 0, 0), false);
				}
				if (!find_nodeA)
				{
					vertA = vert;
					meshA = dental_->teeth[i].p_mesh_;
					find_nodeA = true;
				}
				else if (!find_nodeB)
				{
					vertB = vert;
					meshB = dental_->teeth[i].p_mesh_;
					find_nodeB = true;
				}
			}
		}
	}
	if (ctrledges_ && !ctrledges_->empty())
	{
		vertA = ctrledges_->at(2).vertA;
		vertB = ctrledges_->at(2).vertB;
	}
	if (find_nodeA && find_nodeB && meshA && meshB)
	{
		this->rear_obj_ = new TowableCtrlSystem(1, vertA, meshA, vertB, meshB);
	}

	if (ctrledges_ && !ctrledges_->empty())
	{
		Point3m plane_pos = (ctrledges_->at(3).vertA + ctrledges_->at(3).vertB) / 2.0f;
		Point3m plane_normal = dental_->basePlane.axisZV;
		vector<Point3m> vert_list;
		vert_list.push_back(ctrledges_->at(3).vertA);
		vert_list.push_back(ctrledges_->at(3).vertB);
		this->basalbone_obj_ = new TowableCtrlSystem(3, vert_list, plane_pos, plane_normal);
	}
	else
	{

		find_nodeA = false;  find_nodeB = false;
		meshA = nullptr; meshB = nullptr;
		for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
		{
			if (dental_->bToothExist[i])
			{
				fdi = dental_->teeth[i].strFDI.toInt();
				tooth_sign = fdi % 10;
				if (tooth_sign == 4)
				{
					Point3m vert;
					if (got_tooth_marks)
					{
						vert = dental_->teeth[i].feature_marks_[2];
					}
					else
					{
						Axis local_axis = dental_->teeth[i].localAxis;
						if (fdi / 10 == 1 || fdi / 10 == 3)
						{
							vert = dental_->teeth[i].getDirectionalPeak(local_axis.axisXVector, Point3m(0, 0, 0), Point3m(0, 0, 0));
						}
						else
						{
							vert = dental_->teeth[i].getDirectionalPeak(-local_axis.axisXVector, Point3m(0, 0, 0), Point3m(0, 0, 0));
						}
					}
					if (!find_nodeA)
					{
						vertA = vert;
						find_nodeA = true;
					}
					else if (!find_nodeB)
					{
						vertB = vert;
						find_nodeB = true;
					}
				}
			}
		}
		if (find_nodeA && find_nodeB)
		{
			Point3m plane_pos = (vertA + vertB) / 2.0f;
			Point3m plane_normal = dental_->basePlane.axisZV;
			vector<Point3m> vert_list;
			vert_list.push_back(vertA);
			vert_list.push_back(vertB);
			this->basalbone_obj_ = new TowableCtrlSystem(3, vert_list, plane_pos, plane_normal, true);
		}
	}

	this->setPrompt(QString("Arch Width"));
	update();
}

ArchWidthItem::~ArchWidthItem()
{
	if (front_obj_ != nullptr)
	{
		delete front_obj_;
		front_obj_ = nullptr;
	}
	if (middle_obj_ != nullptr)
	{
		delete middle_obj_;
		middle_obj_ = nullptr;
	}
	if (rear_obj_ != nullptr)
	{
		delete rear_obj_;
		rear_obj_ = nullptr;
	}
	if (basalbone_obj_ != nullptr)
	{
		delete basalbone_obj_;
		basalbone_obj_ = nullptr;
	}
	p_cur_obj_ = nullptr;
}

void ArchWidthItem::draw(QPainter *_p)
{
	if (front_obj_ != nullptr)
	{
		front_obj_->draw();
	}
	if (middle_obj_ != nullptr)
	{
		middle_obj_->draw();
	}
	if (rear_obj_ != nullptr)
	{
		rear_obj_->draw();
	}
}

void ArchWidthItem::update()
{
	int len;
	float len2;
	if (front_obj_ != nullptr)
	{
		len = (front_obj_->length_ * 100) / (int)1;
		len2 = len / 100.0f;
		*front_value_ = len2;
	}
	else
	{
		*front_value_ = -1;
	}
	if (middle_obj_ != nullptr)
	{
		len = (middle_obj_->length_ * 100) / (int)1;
		len2 = len / 100.0f;
		*middle_value_ = len2;
	}
	else
	{
		*middle_value_ = -1;
	}
	if (rear_obj_ != nullptr)
	{
		len = (rear_obj_->length_ * 100) / (int)1;
		len2 = len / 100.0f;
		*rear_value_ = len2;
	}
	else
	{
		*rear_value_ = -1;
	}
}

bool ArchWidthItem::mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn )
{
	if (front_obj_ != nullptr && front_obj_->mousePress(_mouse_x, _mouse_y))
	{
		p_cur_obj_ = front_obj_;
		return true;
	}
	else if (middle_obj_ != nullptr && middle_obj_->mousePress(_mouse_x, _mouse_y))
	{
		p_cur_obj_ = middle_obj_;
		return true;
	}
	else if (rear_obj_ != nullptr && rear_obj_->mousePress(_mouse_x, _mouse_y))
	{
		p_cur_obj_ = rear_obj_;
		return true;
	}
	else if (basalbone_obj_ != nullptr && basalbone_obj_->mousePress(_mouse_x, _mouse_y))
	{
		p_cur_obj_ = basalbone_obj_;
		return true;
	}
	p_cur_obj_ = nullptr;
	return false;
}

void ArchWidthItem::updateCtrlParts()
{
	if (ctrledges_ != nullptr)
	{
		ctrledges_->clear();
		Point3m a, b;

		if (front_obj_)
		{
			a = front_obj_->ctrl_nodes_[0]->p_;
			b = front_obj_->ctrl_nodes_[1]->p_;
		}
		else
		{
			a = Point3m(0, 0, 0);
			b = Point3m(0, 0, 0);
		}
		ctrledges_->push_back(FEdge(a, b));

		if (middle_obj_)
		{
			a = middle_obj_->ctrl_nodes_[0]->p_;
			b = middle_obj_->ctrl_nodes_[1]->p_;
		}
		else
		{
			a = Point3m(0, 0, 0);
			b = Point3m(0, 0, 0);
		}
		ctrledges_->push_back(FEdge(a, b));

		if (rear_obj_)
		{
			a = rear_obj_->ctrl_nodes_[0]->p_;
			b = rear_obj_->ctrl_nodes_[1]->p_;
		}
		else
		{
			a = Point3m(0, 0, 0);
			b = Point3m(0, 0, 0);
		}
		ctrledges_->push_back(FEdge(a, b));
	}
}

void ArchWidthItem::startEdit()
{
}

ArchLengthItem::ArchLengthItem(Dental* _dental, float* _front_value, float* _middle_value, float* _rear_value, float* _whole_value, QString* _context, vector<FEdge>* _ctrledges)
    :EditItem(EditItemIndex(ARCH_LENGTH), _context)
{
	dental_ = _dental;
	ctrledges_ = _ctrledges;
	front_value_ = _front_value;
	middle_value_ = _middle_value;
	rear_value_ = _rear_value;
	whole_value_ = _whole_value;
	bool got_tooth_marks = dental_->gotFeatureMarks();

	int fdi, tooth_sign;
	Point3m vertA, vertB;
	CMeshO* meshA = nullptr, * meshB = nullptr;
	bool find_nodeA, find_nodeB;
	Point3m plane_pos = _dental->basePlane.center;
	Point3m plane_normal = _dental->basePlane.axisZV;

	find_nodeA = false;  find_nodeB = false;
	meshA = nullptr; meshB = nullptr;
	for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (dental_->bToothExist[i])
		{
			fdi = dental_->teeth[i].strFDI.toInt();
			tooth_sign = fdi % 10;
			if (tooth_sign == 1)
			{
				Point3m vert;
				if (got_tooth_marks)
				{
					vert = dental_->teeth[i].feature_marks_[1];
				}
				else
				{
					Axis local_axis = dental_->teeth[i].localAxis;
					if (fdi / 10 == 1 || fdi / 10 == 3)
					{
						vert = dental_->teeth[i].getDirectionalPeak(-local_axis.axisXVector, local_axis.axisYVector, local_axis.axisZVector);
					}
					else
					{
						vert = dental_->teeth[i].getDirectionalPeak(local_axis.axisXVector, local_axis.axisYVector, local_axis.axisZVector);
					}
				}
				if (!find_nodeA)
				{
					vertA = vert;
					meshA = dental_->teeth[i].p_mesh_;
					find_nodeA = true;
				}
				else if (!find_nodeB)
				{
					vertB = vert;
					meshB = dental_->teeth[i].p_mesh_;
					find_nodeB = true;
				}
			}
		}
	}
	if (find_nodeA && find_nodeB && meshA && meshB)
	{
		Point3m vert = (vertA + vertB) * 0.5f;
		this->anterior_tangent_vert = UtilityTools::getInstance()->getProjPointOnPlane(vert, plane_pos, plane_normal);
	}

	if (ctrledges_ && !ctrledges_->empty())
	{
		vector<Point3m> vert_list;
		vert_list.push_back(ctrledges_->at(0).vertA);
		vert_list.push_back(ctrledges_->at(0).vertB);
		this->front_obj_ = new TowableCtrlSystem(0, vert_list, plane_pos, plane_normal);
		this->addToObjectList(front_obj_);
	}
	else
	{
		find_nodeA = false;  find_nodeB = false;
		meshA = nullptr; meshB = nullptr;
		for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
		{
			if (dental_->bToothExist[i])
			{
				fdi = dental_->teeth[i].strFDI.toInt();
				tooth_sign = fdi % 10;
				if (tooth_sign == 3)
				{
					Point3m vert;
					if (got_tooth_marks)
					{
						vert = dental_->teeth[i].feature_marks_[0];
					}
					else
					{
						Axis local_axis = dental_->teeth[i].localAxis;
						vert = dental_->teeth[i].getDirectionalPeak(local_axis.axisXVector, local_axis.axisYVector, local_axis.axisZVector);
					}
					if (!find_nodeA)
					{
						vertA = UtilityTools::getInstance()->getProjPointOnPlane(vert, plane_pos, plane_normal);
						meshA = dental_->teeth[i].p_mesh_;
						find_nodeA = true;
					}
					else if (!find_nodeB)
					{
						vertB = UtilityTools::getInstance()->getProjPointOnPlane(vert, plane_pos, plane_normal);
						meshB = dental_->teeth[i].p_mesh_;
						find_nodeB = true;
					}
				}
			}
		}
		if (find_nodeA && find_nodeB && meshA && meshB)
		{
			vector<Point3m> vert_list;
			vert_list.push_back(vertA);
			vert_list.push_back(vertB);
			this->front_obj_ = new TowableCtrlSystem(0, vert_list, plane_pos, plane_normal);
			this->addToObjectList(front_obj_);
		}
	}

	if (ctrledges_ && !ctrledges_->empty())
	{
		vector<Point3m> vert_list;
		vert_list.push_back(ctrledges_->at(1).vertA);
		vert_list.push_back(ctrledges_->at(1).vertB);
		this->middle_obj_ = new TowableCtrlSystem(1, vert_list, plane_pos, plane_normal);
		this->middle_obj_->setVisible(false);
		this->addToObjectList(middle_obj_);
	}
	else
	{
		find_nodeA = false;  find_nodeB = false;
		meshA = nullptr; meshB = nullptr;
		for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
		{
			if (dental_->bToothExist[i])
			{
				fdi = dental_->teeth[i].strFDI.toInt();
				tooth_sign = fdi % 10;
				if (tooth_sign == 4)
				{
					Point3m vert;
					if (got_tooth_marks)
					{
						vert = (dental_->teeth[i].feature_marks_[2] + dental_->teeth[i].feature_marks_[3]) * 0.5f;
					}
					else
					{
						Axis local_axis = dental_->teeth[i].localAxis;
						vert = dental_->teeth[i].getDirectionalPeak(Point3m(0, 0, 0), local_axis.axisYVector, Point3m(0, 0, 0));
					}
					if (!find_nodeA)
					{
						vertA = UtilityTools::getInstance()->getProjPointOnPlane(vert, plane_pos, plane_normal);
						meshA = dental_->teeth[i].p_mesh_;
						find_nodeA = true;
					}
					else if (!find_nodeB)
					{
						vertB = UtilityTools::getInstance()->getProjPointOnPlane(vert, plane_pos, plane_normal);
						meshB = dental_->teeth[i].p_mesh_;
						find_nodeB = true;
					}
				}
			}
		}
		if (find_nodeA && find_nodeB && meshA && meshB)
		{
			vector<Point3m> vert_list;
			vert_list.push_back(vertA);
			vert_list.push_back(vertB);
			this->middle_obj_ = new TowableCtrlSystem(1, vert_list, plane_pos, plane_normal);
			this->middle_obj_->setVisible(false);
			this->addToObjectList(middle_obj_);
		}
	}

	if (ctrledges_ && !ctrledges_->empty())
	{
		vector<Point3m> vert_list;
		vert_list.push_back(ctrledges_->at(2).vertA);
		vert_list.push_back(ctrledges_->at(2).vertB);
		this->rear_obj_ = new TowableCtrlSystem(2, vert_list, plane_pos, plane_normal);
		this->rear_obj_->setVisible(false);
		this->addToObjectList(rear_obj_);
	}
	else
	{
		find_nodeA = false;  find_nodeB = false;
		meshA = nullptr; meshB = nullptr;
		for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
		{
			if (dental_->bToothExist[i])
			{
				fdi = dental_->teeth[i].strFDI.toInt();
				tooth_sign = fdi % 10;
				if (tooth_sign == 6)
				{
					Point3m vert;
					if (got_tooth_marks)
					{
						if (fdi == 16 || fdi == 36)
						{
							vert = dental_->teeth[i].feature_marks_[10];
						}
						else
						{
							vert = dental_->teeth[i].feature_marks_[2];
						}
					}
					else
					{
						Axis local_axis = dental_->teeth[i].localAxis;
						if (fdi == 16 || fdi == 36)
						{
							vert = dental_->teeth[i].getDirectionalPeak(local_axis.axisXVector, Point3m(0, 0, 0), Point3m(0, 0, 0));
						}
						else
						{
							vert = dental_->teeth[i].getDirectionalPeak(-local_axis.axisXVector, Point3m(0, 0, 0), Point3m(0, 0, 0));
						}
					}
					if (!find_nodeA)
					{
						vertA = UtilityTools::getInstance()->getProjPointOnPlane(vert, plane_pos, plane_normal);
						meshA = dental_->teeth[i].p_mesh_;
						find_nodeA = true;
					}
					else if (!find_nodeB)
					{
						vertB = UtilityTools::getInstance()->getProjPointOnPlane(vert, plane_pos, plane_normal);
						meshB = dental_->teeth[i].p_mesh_;
						find_nodeB = true;
					}
				}
			}
		}
		if (find_nodeA && find_nodeB && meshA && meshB)
		{
			vector<Point3m> vert_list;
			vert_list.push_back(vertA);
			vert_list.push_back(vertB);
			this->rear_obj_ = new TowableCtrlSystem(2, vert_list, plane_pos, plane_normal);
			this->rear_obj_->setVisible(false);
			this->addToObjectList(rear_obj_);
		}
	}

	setPrompt(QString("Arch Length"));
	update();
}

ArchLengthItem::~ArchLengthItem()
{
	if (front_obj_ != nullptr)
	{
		delete front_obj_;
		front_obj_ = nullptr;
	}
	if (middle_obj_ != nullptr)
	{
		delete middle_obj_;
		middle_obj_ = nullptr;
	}
	if (rear_obj_ != nullptr)
	{
		delete rear_obj_;
		rear_obj_ = nullptr;
	}
	this->obj_list_.clear();
	p_cur_obj_ = nullptr;
}

void ArchLengthItem::draw(QPainter *_p)
{
	if (front_obj_ != nullptr)
	{
		front_obj_->draw();
	}
	if (middle_obj_ != nullptr)
	{
		middle_obj_->draw();
	}
	if (rear_obj_ != nullptr)
	{
		rear_obj_->draw();
		drawEdge(total_edge_);
	}
}

bool ArchLengthItem::getIntersectPoint3m(Point3m _pa, Point3m _pb, Point3m _pc, Point3m _pd, Point3m& _result)
{
	Point3m vec_ab = (_pb - _pa).Normalize();
	Point3m vec_ac = _pc - _pa;
	Point3m vec_dc = _pd - _pa;
	if ((vec_ab ^ vec_ac) * (vec_ab ^ vec_dc) > 0)
	{
		return false;
	}

	Point3m vec_cd = _pd - _pc;
	Point3m normal = (vec_cd ^ dental_feature_->base_plane_.axisZV).Normalize();
	_result = UtilityTools::getInstance()->getProjPointOnPlane(_pa, vec_ab, _pc, -normal);
	return true;
}

void ArchLengthItem::update()
{
	int len;
	Point3m vec1, vec2;
	float length;
	if (rear_obj_)
	{
		Point3m vert_a, vert_b, vert_c;
		vec1 = this->anterior_tangent_vert - rear_obj_->ctrl_nodes_[0]->p_;
		vec2 = (rear_obj_->ctrl_nodes_[1]->p_ - rear_obj_->ctrl_nodes_[0]->p_).Normalize();
		length = vec1 * vec2;
		vert_c = rear_obj_->ctrl_nodes_[0]->p_ + vec2 * length;
		total_edge_ = FEdge(this->anterior_tangent_vert, vert_c);
		length = total_edge_.calculateLength();
		len = (length * 100) / (int)1;
		length = len / 100.0f;
		*whole_value_ = length;

		bool intersect_a = false;
		if (front_obj_)
		{
			intersect_a = getIntersectPoint3m(anterior_tangent_vert, vert_c, front_obj_->ctrl_nodes_[0]->p_, front_obj_->ctrl_nodes_[1]->p_, vert_a);
		}

		bool intersect_b = false;
		if (middle_obj_)
		{
			intersect_b = getIntersectPoint3m(anterior_tangent_vert, vert_c, middle_obj_->ctrl_nodes_[0]->p_, middle_obj_->ctrl_nodes_[1]->p_, vert_b);
		}

		if (intersect_a && intersect_b)
		{
			front_edge_ = FEdge(this->anterior_tangent_vert, vert_a);
			length = front_edge_.calculateLength();
			len = (length * 100) / (int)1;
			length = len / 100.0f;
			*front_value_ = length;

			middle_edge_ = FEdge(vert_a, vert_b);
			length = middle_edge_.calculateLength();
			len = (length * 100) / (int)1;
			length = len / 100.0f;
			*middle_value_ = length;

			rear_edge_ = FEdge(vert_b, vert_c);
			length = rear_edge_.calculateLength();
			len = (length * 100) / (int)1;
			length = len / 100.0f;
			*rear_value_ = length;
			return;
		}
		else if (intersect_a && !intersect_b)
		{
			front_edge_ = FEdge(this->anterior_tangent_vert, vert_a);
			length = front_edge_.calculateLength();
			len = (length * 100) / (int)1;
			length = len / 100.0f;
			*front_value_ = length;
			*middle_value_ = -1;
			*rear_value_ = -1;
			return;
		}
		else if (!intersect_a && intersect_b)
		{
			rear_edge_ = FEdge(vert_b, vert_c);
			length = rear_edge_.calculateLength();
			len = (length * 100) / (int)1;
			length = len / 100.0f;
			*rear_value_ = length;
			*front_value_ = -1;
			*middle_value_ = -1;
			return;
		}
	}

	*front_value_ = -1;
	*middle_value_ = -1;
	*rear_value_ = -1;
	*whole_value_ = -1;
	return;
}

bool ArchLengthItem::mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn )
{
	if (front_obj_ != nullptr && front_obj_->mousePress(_mouse_x, _mouse_y))
	{
		p_cur_obj_ = front_obj_;
		return true;
	}
	else if (middle_obj_ != nullptr && middle_obj_->mousePress(_mouse_x, _mouse_y))
	{
		p_cur_obj_ = middle_obj_;
		return true;
	}
	else if (rear_obj_ != nullptr && rear_obj_->mousePress(_mouse_x, _mouse_y))
	{
		p_cur_obj_ = rear_obj_;
		return true;
	}
	p_cur_obj_ = nullptr;
	return false;
}

void ArchLengthItem::updateCtrlParts()
{
	if (ctrledges_ != nullptr)
	{
		ctrledges_->clear();
		Point3m a, b;

		if (front_obj_)
		{
			a = front_obj_->ctrl_nodes_[0]->p_;
			b = front_obj_->ctrl_nodes_[1]->p_;
		}
		else
		{
			a = Point3m(0, 0, 0);
			b = Point3m(0, 0, 0);
		}
		ctrledges_->push_back(FEdge(a, b));

		if (middle_obj_)
		{
			a = middle_obj_->ctrl_nodes_[0]->p_;
			b = middle_obj_->ctrl_nodes_[1]->p_;
		}
		else
		{
			a = Point3m(0, 0, 0);
			b = Point3m(0, 0, 0);
		}
		ctrledges_->push_back(FEdge(a, b));

		if (rear_obj_)
		{
			a = rear_obj_->ctrl_nodes_[0]->p_;
			b = rear_obj_->ctrl_nodes_[1]->p_;
		}
		else
		{
			a = Point3m(0, 0, 0);
			b = Point3m(0, 0, 0);
		}
		ctrledges_->push_back(FEdge(a, b));
	}
}

void ArchLengthItem::startEdit()
{
}

GnathotectumHeightItem::GnathotectumHeightItem(Dental* _dental, float* _p_value, QString* _context)
    :EditItem(EditItemIndex(GNATHOTECTUM_HEIGHT), _context)
{
	p_height_value_ = _p_value;
	dental_ = _dental;
	bool got_tooth_marks = dental_->gotFeatureMarks();

	int fdi, tooth_sign;
	Point3m vertA, vertB;
	CMeshO* meshA = nullptr, * meshB = nullptr;
	bool find_nodeA, find_nodeB;

	find_nodeA = false;  find_nodeB = false;
	meshA = nullptr; meshB = nullptr;
	for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
	{
		if (dental_->bToothExist[i])
		{
			fdi = dental_->teeth[i].strFDI.toInt();
			tooth_sign = fdi % 10;
			if (tooth_sign == 6)
			{
				Point3m vert;
				if (got_tooth_marks)
				{
					vert = dental_->teeth[i].feature_marks_[6];
				}
				else
				{
					Axis local_axis = dental_->teeth[i].localAxis;
					vert = dental_->teeth[i].getDirectionalPeak(Point3m(0, 0, 0), local_axis.axisYVector, Point3m(0, 0, 0));
				}
				if (!find_nodeA)
				{
					vertA = vert;
					meshA = dental_->teeth[i].p_mesh_;
					find_nodeA = true;
				}
				else if (!find_nodeB)
				{
					vertB = vert;
					meshB = dental_->teeth[i].p_mesh_;
					find_nodeB = true;
				}
			}
		}
	}
	if (find_nodeA && find_nodeB && meshA && meshB && gum_mesh_)
	{
		this->tooth_ope_obj_ = new TowableCtrlSystem(0, vertA, meshA, vertB, meshB);

		Point3m mid_point = (vertA + vertB) * 0.5f;
		std::pair<bool, Point3m> intersection_test = UtilityTools::getInstance()->intersectMesh(mid_point, dental_->basePlane.axisZV, gum_mesh_);
		if (intersection_test.first)
		{
			vector<std::pair<Point3m, vector<CMeshO*>>> node_list;
			vector<CMeshO*> mesh_list;
			mesh_list.push_back(gum_mesh_);
			node_list.push_back(make_pair(intersection_test.second, mesh_list));
			this->gum_ope_obj_ = new TowableCtrlSystem(1, node_list);
		}
	}
	setPrompt(QString("Palatal height"));
	update();
}

GnathotectumHeightItem::~GnathotectumHeightItem()
{
	if (tooth_ope_obj_ != nullptr)
	{
		delete tooth_ope_obj_;
		tooth_ope_obj_ = nullptr;
	}
	if (gum_ope_obj_ != nullptr)
	{
		delete gum_ope_obj_;
		gum_ope_obj_ = nullptr;
	}
}

void GnathotectumHeightItem::draw(QPainter *_p)
{
	if (tooth_ope_obj_ != nullptr)
	{
		tooth_ope_obj_->draw();
	}
	if (gum_ope_obj_ != nullptr)
	{
		gum_ope_obj_->draw();
		drawEdge(height_edge_);
	}
}

void GnathotectumHeightItem::update()
{
	if (tooth_ope_obj_ == nullptr || gum_mesh_ == nullptr)
	{
		return;
	}

	this->setContext(QString("/"));
	if (p_height_value_)
	{
		*p_height_value_ = -1;
	}

	if (ctrledges_)
	{
		ctrledges_->clear();
	}
	ctrledges_->push_back(FEdge(tooth_ope_obj_->ctrl_nodes_[0]->p_, tooth_ope_obj_->ctrl_nodes_[1]->p_));
	Point3m mid_point = (tooth_ope_obj_->ctrl_nodes_[0]->p_ + tooth_ope_obj_->ctrl_nodes_[1]->p_) * 0.5f;
	if (tooth_ope_obj_->pickedHandle())
	{
		std::pair<bool, Point3m> intersection_test;
		if (dental_)
		{
			intersection_test = UtilityTools::getInstance()->intersectMesh(mid_point, dental_->basePlane.axisZV, gum_mesh_);
		}
		if (dental_feature_)
		{
			intersection_test = UtilityTools::getInstance()->intersectMesh(mid_point, dental_feature_->base_plane_.axisZV, gum_mesh_);
		}
		if (intersection_test.first)
		{
			if (gum_ope_obj_ != nullptr)
			{
				gum_ope_obj_->ctrl_nodes_[0]->p_ = intersection_test.second;
			}
			else
			{
				vector<std::pair<Point3m, vector<CMeshO*>>> node_list;
				vector<CMeshO*> mesh_list;
				mesh_list.push_back(gum_mesh_);
				node_list.push_back(make_pair(intersection_test.second, mesh_list));
				this->gum_ope_obj_ = new TowableCtrlSystem(1, node_list);
			}
			ctrledges_->push_back(FEdge(intersection_test.second, intersection_test.second));
		}
		else
		{
			if (gum_ope_obj_ != nullptr)
			{
				delete gum_ope_obj_;
				gum_ope_obj_ = nullptr;
			}
			this->setContext(QString("/"));
			if (p_height_value_)
			{
				*p_height_value_ = -1;
			}
		}
	}

	if (gum_ope_obj_ != nullptr && gum_ope_obj_->pickedHandle())
	{
		Point3m normal = (tooth_ope_obj_->ctrl_nodes_[0]->p_ - tooth_ope_obj_->ctrl_nodes_[1]->p_).Normalize();
		Point3m proj_vert = UtilityTools::getInstance()->getProjPointOnPlane(gum_ope_obj_->ctrl_nodes_[0]->p_, mid_point, -normal);
		Point3m new_direct = (proj_vert - mid_point).Normalize();
		std::pair<bool, Point3m> new_intersection = UtilityTools::getInstance()->intersectMesh(mid_point, new_direct, gum_mesh_);
		if (new_intersection.first)
		{
			gum_ope_obj_->ctrl_nodes_[0]->p_ = new_intersection.second;
			ctrledges_->push_back(FEdge(new_intersection.second, new_intersection.second));
		}
		else
		{
			std::pair<bool, Point3m> intersection_test = UtilityTools::getInstance()->intersectMesh(mid_point, dental_feature_->base_plane_.axisZV, gum_mesh_);
			if (intersection_test.first)
			{
				gum_ope_obj_->ctrl_nodes_[0]->p_ = intersection_test.second;
				ctrledges_->push_back(FEdge(intersection_test.second, intersection_test.second));
			}
			else
			{
				if (gum_ope_obj_ != nullptr)
				{
					delete gum_ope_obj_;
					gum_ope_obj_ = nullptr;
				}
				this->setContext(QString("/"));
				if (p_height_value_)
				{
					*p_height_value_ = -1;
				}
			}
		}
	}

	if (gum_ope_obj_ != nullptr)
	{
		height_edge_ = FEdge(gum_ope_obj_->ctrl_nodes_[0]->p_, mid_point);
		Point3m vec = gum_ope_obj_->ctrl_nodes_[0]->p_ - mid_point;
		float height = sqrtf(vec * vec);
		int temp = (height * 100) / (int)1;
		height = temp / 100.0f;
		this->setContext(QString::number(height).append("mm"));
		if (p_height_value_)
		{
			*p_height_value_ = height;
		}
	}
}

bool GnathotectumHeightItem::mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn )
{
	if (tooth_ope_obj_ != nullptr && tooth_ope_obj_->mousePress(_mouse_x, _mouse_y))
	{
		p_cur_obj_ = tooth_ope_obj_;
		return true;
	}
	if (gum_ope_obj_ != nullptr && gum_ope_obj_->mousePress(_mouse_x, _mouse_y))
	{
		p_cur_obj_ = gum_ope_obj_;
		return true;
	}
	p_cur_obj_ = nullptr;
	return false;
}

void GnathotectumHeightItem::startEdit()
{
}

BasalBoneArchItem::~BasalBoneArchItem()
{
	if (basalbone_len_obj_ != nullptr)
	{
		delete basalbone_len_obj_;
		basalbone_len_obj_ = nullptr;
	}
	if (basalbone_wid_obj_ != nullptr)
	{
		delete basalbone_wid_obj_;
		basalbone_wid_obj_ = nullptr;
	}
	if (gum_vert_obj_ != nullptr)
	{
		delete gum_vert_obj_;
		gum_vert_obj_ = nullptr;
	}
}

void BasalBoneArchItem::draw(QPainter* _p)
{
	if (basalbone_wid_obj_ != nullptr && basalbone_wid_obj_->visible_)
	{
		basalbone_wid_obj_->draw();
		this->drawEdge(basalbone_wid_edge_);
	}

	if (basalbone_len_obj_ != nullptr && gum_vert_obj_ != nullptr && basalbone_len_obj_->visible_)
	{
		basalbone_len_obj_->draw();
		gum_vert_obj_->draw();

		drawEdge(gum_len_edge_, Point3m(1, 1, 0), Point3m(0, 0, 0));
		drawEdge(basalbone_len_edge_, Point3m(1, 1, 0), Point3m(0, 0, 0));

		if (dental_feature_)
		{
			dental_feature_->spee_plane_.drawCutFace();
		}
	}
}

void BasalBoneArchItem::update()
{
	int len;
	Point3m vec1, vec2, vert;
	float length;

	if (basalbone_wid_obj_ != nullptr)
	{
		Point3m proj_a, proj_b, proj_vec;
		if (dental_feature_)
		{
			proj_a = UtilityTools::getInstance()->getProjPointOnPlane(basalbone_wid_obj_->ctrl_nodes_[0]->p_, dental_feature_->base_plane_.center, -dental_feature_->base_plane_.axisZV);
			proj_b = UtilityTools::getInstance()->getProjPointOnPlane(basalbone_wid_obj_->ctrl_nodes_[1]->p_, dental_feature_->base_plane_.center, -dental_feature_->base_plane_.axisZV);
		}
		proj_vec = proj_a - proj_b;
		length = sqrtf(proj_vec * proj_vec);
		len = (length * 100) / (int)1;
		length = len / 100.0f;

		basalbone_wid_edge_ = FEdge(proj_a, proj_b);
		*basalbone_wid_value_ = length;
	}
	else
	{
		*basalbone_wid_value_ = -1;
	}

	if (gum_vert_obj_ != nullptr)
	{
		gum_vert_ = gum_vert_obj_->ctrl_nodes_[0]->p_;
		Point3m normal;
		if (dental_feature_)
		{
			normal = dental_feature_->base_plane_.axisZV;
		}

		proj_gum_vert_ = UtilityTools::getInstance()->getProjPointOnPlane(gum_vert_, anterior_tangent_vert, -normal);
		gum_len_edge_ = FEdge(gum_vert_, proj_gum_vert_);
	}
	if (basalbone_len_obj_ != nullptr)
	{
		vec1 = this->proj_gum_vert_ - basalbone_len_obj_->ctrl_nodes_[0]->p_;
		vec2 = (basalbone_len_obj_->ctrl_nodes_[1]->p_ - basalbone_len_obj_->ctrl_nodes_[0]->p_).Normalize();
		length = vec1 * vec2;
		vert = basalbone_len_obj_->ctrl_nodes_[0]->p_ + vec2 * length;

		basalbone_len_edge_ = FEdge(vert, this->proj_gum_vert_);
		length = basalbone_len_edge_.calculateLength();
		len = (length * 100) / (int)1;
		length = len / 100.0f;
		*basalbone_len_value_ = length;
	}
	else
	{
		*basalbone_len_value_ = -1;
	}
}

void BasalBoneArchItem::updateCtrlParts()
{
	if (ctrledges_ != nullptr)
	{
		ctrledges_->clear();
		Point3m a, b;

		if (basalbone_wid_obj_)
		{
			a = basalbone_wid_obj_->ctrl_nodes_[0]->p_;
			b = basalbone_wid_obj_->ctrl_nodes_[1]->p_;
		}
		else
		{
			a = Point3m(0, 0, 0);
			b = Point3m(0, 0, 0);
		}
		ctrledges_->push_back(FEdge(a, b));

		if (basalbone_len_obj_)
		{
			a = basalbone_len_obj_->ctrl_nodes_[0]->p_;
			b = basalbone_len_obj_->ctrl_nodes_[1]->p_;
		}
		else
		{
			a = Point3m(0, 0, 0);
			b = Point3m(0, 0, 0);
		}
		ctrledges_->push_back(FEdge(a, b));

		if (gum_vert_obj_)
		{
			a = gum_vert_obj_->ctrl_nodes_[0]->p_;
		}
		else
		{
			a = Point3m(0, 0, 0);
		}
		ctrledges_->push_back(FEdge(a, a));
	}
}

bool BasalBoneArchItem::mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn )
{
	if (basalbone_wid_obj_ != nullptr && basalbone_wid_obj_->mousePress(_mouse_x, _mouse_y))
	{
		p_cur_obj_ = basalbone_wid_obj_;
		return true;
	}
	if (basalbone_len_obj_ != nullptr && basalbone_len_obj_->mousePress(_mouse_x, _mouse_y))
	{
		p_cur_obj_ = basalbone_len_obj_;
		return true;
	}
	if (gum_vert_obj_ != nullptr && gum_vert_obj_->mousePress(_mouse_x, _mouse_y))
	{
		p_cur_obj_ = gum_vert_obj_;
		return true;
	}
	p_cur_obj_ = nullptr;
	return false;
}

void BasalBoneArchItem::startEdit()
{
}

DentalAnalysisData::DentalAnalysisData()
{
	connect(PSIGNALMANAGER, &SignalManager::updateSceneSizeSignal, this, &DentalAnalysisData::updateLabelsSlot);
}

DentalAnalysisData::DentalAnalysisData(const DentalAnalysisData& _data):DentalAnalysisData()
{
	*this = _data;
}

DentalAnalysisData::DentalAnalysisData(DentalManager* _upper_dental, DentalManager* _lower_dental)
{
	initialAnalysisData(_upper_dental, _lower_dental);
	connect(PSIGNALMANAGER, &SignalManager::updateSceneSizeSignal, this, &DentalAnalysisData::updateLabelsSlot);
}

DentalAnalysisData::DentalAnalysisData(DentalFeatures* _upper_dental_features, DentalFeatures* _lower_dental_features)
{
	QString(tr("Normal overjet"));
	QString(tr("I-degree overjet"));
	QString(tr("II-degree overjet"));
	QString(tr("III-degree overjet"));
	QString(tr("Crossbite"));
	QString(tr("Open bite"));
	QString(tr("Normal "));
	QString(tr("overbite"));
	QString(tr("I-degree open bite"));
	QString(tr("II-degree open bite"));
	QString(tr("III-degree open bite"));
	QString(tr("I-degree "));
	QString(tr("II-degree "));
	QString(tr("III-degree "));
	QString(tr("overbite"));
	QString(tr("Normal overbite"));
	QString(tr("I-degree overbite"));
	QString(tr("II-degree overbite"));
	QString(tr("III-degree overbite"));

	initialAnalysisData(_upper_dental_features, _lower_dental_features);
	connect(PSIGNALMANAGER, &SignalManager::updateSceneSizeSignal, this, &DentalAnalysisData::updateLabelsSlot);
}

DentalAnalysisData::~DentalAnalysisData()
{
	for (int i = 0; i < upper_item_list_.size(); ++i)
	{
		if (upper_item_list_[i] != nullptr)
		{
			delete upper_item_list_[i];
			upper_item_list_[i] = nullptr;
		}
	}
	vector<EditItem*>().swap(upper_item_list_);

	for (int i = 0; i < lower_item_list_.size(); ++i)
	{
		if (lower_item_list_[i] != nullptr)
		{
			delete lower_item_list_[i];
			lower_item_list_[i] = nullptr;
		}
	}
	vector<EditItem*>().swap(lower_item_list_);

	cur_ope_item_ = nullptr;
	cur_upper_item_ = nullptr;
	cur_lower_item_ = nullptr;
}

DentalAnalysisData& DentalAnalysisData::operator=(const DentalAnalysisData& _data)
{
	this->tooth_width_edges_upper_ = _data.tooth_width_edges_upper_;
	this->tooth_width_edges_lower_ = _data.tooth_width_edges_lower_;
	this->cur_length_arch_ctrlnodes_upper_ = _data.cur_length_arch_ctrlnodes_upper_;
	this->cur_length_arch_ctrlnodes_lower_ = _data.cur_length_arch_ctrlnodes_lower_;
	this->crowding_arch_5_5_upper_ = _data.crowding_arch_5_5_upper_;
	this->crowding_arch_7_7_upper_ = _data.crowding_arch_7_7_upper_;
	this->crowding_arch_5_5_lower_ = _data.crowding_arch_5_5_lower_;
	this->crowding_arch_7_7_lower_ = _data.crowding_arch_7_7_lower_;
	this->spee_ctrlnodes_ = _data.spee_ctrlnodes_;
	this->molar_ctrledges_upper_ = _data.molar_ctrledges_upper_;
	this->molar_ctrledges_lower_ = _data.molar_ctrledges_lower_;
	this->midline_ctrledges_upper_ = _data.midline_ctrledges_upper_;
	this->midline_ctrledges_lower_ = _data.midline_ctrledges_lower_;
	this->arch_width_ctrledges_upper_ = _data.arch_width_ctrledges_upper_;
	this->arch_width_ctrledges_lower_ = _data.arch_width_ctrledges_lower_;
	this->arch_len_ctrledges_upper_ = _data.arch_len_ctrledges_upper_;
	this->arch_len_ctrledges_lower_ = _data.arch_len_ctrledges_lower_;
	this->basal_bone_arch_ctrledges_upper_ = _data.basal_bone_arch_ctrledges_upper_;
	this->basal_bone_arch_ctrledges_lower_ = _data.basal_bone_arch_ctrledges_lower_;
	this->gna_height_ctrledges_upper_ = _data.gna_height_ctrledges_upper_;
	this->left_overlay_measure_edge_ = _data.left_overlay_measure_edge_;
	this->right_overlay_measure_edge_ = _data.right_overlay_measure_edge_;
	this->overbite_value_ = _data.overbite_value_;
	this->overbite_rank_ = _data.overbite_rank_;
	this->overjet_value_ = _data.overjet_value_;
	this->overjet_rank_ = _data.overjet_rank_;
	this->confirm_missing_fdi_ = _data.confirm_missing_fdi_;
	this->confirm_tooth_width_ = _data.confirm_tooth_width_;
	this->confirm_anterior_parameter_ = _data.confirm_anterior_parameter_;
	this->confirm_arch_length_ = _data.confirm_arch_length_;
	this->confirm_arch_width_ = _data.confirm_arch_width_;
	this->confirm_bolton_ = _data.confirm_bolton_;
	this->confirm_crowding_ = _data.confirm_crowding_;
	this->confirm_gnathotectum_height_ = _data.confirm_gnathotectum_height_;
	this->confirm_basal_bone_arch_ = _data.confirm_basal_bone_arch_;
	this->confirm_overlay_ = _data.confirm_overlay_;
	this->confirm_molar_ = _data.confirm_molar_;
	this->confirm_spee_ = _data.confirm_spee_;
	this->browsed_item_prompt_ = _data.browsed_item_prompt_;
	this->remark_ = _data.remark_;
	this->patient_name_ = _data.patient_name_;
	this->doctor_name_ = _data.doctor_name_;
	return *this;
}

void DentalAnalysisData::initialAnalysisData(DentalManager* _upper_dental, DentalManager* _lower_dental)
{
	if (_upper_dental != nullptr && _lower_dental != nullptr)
	{
		upper_dental_manager_ = _upper_dental;
		lower_dental_manager_ = _lower_dental;
		vector<EditItemIndex>().swap(avaliable_item_index_list_);

		for (auto& item : upper_item_list_)
		{
			SAFE_DELETE(item);
		}
		upper_item_list_.clear();
		for (auto& item : lower_item_list_)
		{
			SAFE_DELETE(item);
		}
		lower_item_list_.clear();

        switchEditItem(EditItemIndex(NONE));
        switchEditItem(EditItemIndex(MISSING_TOOTH));
        switchEditItem(EditItemIndex(TEETH_WIDTH));
        switchEditItem(EditItemIndex(SPEE_CURVE_DEPTH));
        switchEditItem(EditItemIndex(BOLTON_COMPLETELY));
        switchEditItem(EditItemIndex(CUR_LENGTH_OF_DENTAL_ARCH));
        switchEditItem(EditItemIndex(MOLAR_RELATIONSHIP));
        switchEditItem(EditItemIndex(MIDLINE_RELATIONSHIP));
        switchEditItem(EditItemIndex(ARCH_WIDTH));
        switchEditItem(EditItemIndex(ARCH_LENGTH));
        switchEditItem(EditItemIndex(GNATHOTECTUM_HEIGHT));
        switchEditItem(EditItemIndex(OVERLAY_ANALYSIS));
        switchEditItem(EditItemIndex(OCCLUSION_COLORING));
        switchEditItem(EditItemIndex(NONE));

		updateSpeedDeepth();
		updateSeverityOfCrowding();
		updateBoltonAnterior();
		updateBoltonCompletely();
		updateMolarRelationship();
		udpateMidlineRelationship();
		updateTeethWidthList();
		setOverjetRank(overjet_rank_);
		setOverbiteRank(overbite_rank_);

		emit PSIGNALMANAGER->updateAvaliableItemActionsSignal(avaliable_item_index_list_);
	}
}

void DentalAnalysisData::switchEditItem(EditItemIndex _item, bool _b_upper)
{
	if (cur_mode_ == _item)
	{
		return;
	}

	cur_upper_item_ = nullptr;
	cur_lower_item_ = nullptr;

	cur_mode_ = _item;
	switch (_item)
	{
	case MISSING_TOOTH:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->initialUpdate();
		}
		else
		{
			this->cur_upper_item_ = new MissingToothItem(&upper_dental_manager_->cDental, this->missing_tooth_upper_, nullptr);
			upper_item_list_.push_back(this->cur_upper_item_);
		}

		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->initialUpdate();
		}
		else
		{
			this->cur_lower_item_ = new MissingToothItem(&lower_dental_manager_->cDental, this->missing_tooth_lower_, nullptr);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		break;

	case DOCTOR_REMARK:
        this->cur_upper_item_ = new EditItem(EditItemIndex(DOCTOR_REMARK) , &this->remark_);
		break;

	case TEETH_WIDTH:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->initialUpdate();
		}
		else
		{
			this->cur_upper_item_ = new TeethWidthItem(&upper_dental_manager_->cDental, &this->space_requared_upper_);
			this->cur_upper_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			upper_item_list_.push_back(this->cur_upper_item_);
		}

		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->initialUpdate();
		}
		else
		{
			this->cur_lower_item_ = new TeethWidthItem(&lower_dental_manager_->cDental, &this->space_requared_lower_);
			this->cur_lower_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		break;

	case	CUR_LENGTH_OF_DENTAL_ARCH:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->initialUpdate();
		}
		else
		{
			this->cur_upper_item_ = new CurrentArchLengthItem(&upper_dental_manager_->cDental, &this->cur_length_arch_upper_, &cur_length_arch_ctrlnodes_upper_);
			upper_item_list_.push_back(this->cur_upper_item_);
		}

		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->initialUpdate();
		}
		else
		{
			this->cur_lower_item_ = new CurrentArchLengthItem(&lower_dental_manager_->cDental, &this->cur_length_arch_lower_, &cur_length_arch_ctrlnodes_lower_);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		break;

	case BOLTON_COMPLETELY:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->initialUpdate();
		}
		else
		{
            this->cur_upper_item_ = new BoltonItem(&upper_dental_manager_->cDental, &this->sum_anterior_width_upper_, &this->sum_whole_width_upper_, &this->space_requared_upper_, EditItemIndex(BOLTON_COMPLETELY), 6);
			this->cur_upper_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			upper_item_list_.push_back(this->cur_upper_item_);
		}

		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->initialUpdate();
		}
		else
		{
            this->cur_lower_item_ = new BoltonItem(&lower_dental_manager_->cDental, &this->sum_anterior_width_lower_, &this->sum_whole_width_lower_, &this->space_requared_lower_, EditItemIndex(BOLTON_COMPLETELY), 6);
			this->cur_lower_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		break;

	case SPEE_CURVE_DEPTH:
		this->cur_upper_item_ = nullptr;
		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->initialUpdate();
		}
		else
		{
			this->cur_lower_item_ = new SpeeCurveDepthItem(&lower_dental_manager_->cDental, &this->left_spee_depth_, &this->right_spee_deepth_, nullptr, &spee_ctrlnodes_);
			this->cur_lower_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		_b_upper = false;
		break;

	case MOLAR_RELATIONSHIP:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			this->cur_upper_item_->initialUpdate();
		}
		else
		{
			this->cur_upper_item_ = new MolarRelationshipItem(nullptr, &left_molar_node_upper_, &right_molar_node_upper_, upper_dental_manager_, &molar_ctrledges_upper_);
			this->cur_upper_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			upper_item_list_.push_back(this->cur_upper_item_);
		}

		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			this->cur_lower_item_->initialUpdate();
		}
		else
		{
			this->cur_lower_item_ = new MolarRelationshipItem(nullptr, &left_molar_node_lower_, &right_molar_node_lower_, lower_dental_manager_, &molar_ctrledges_lower_);
			this->cur_lower_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		break;

	case	MIDLINE_RELATIONSHIP:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			this->cur_upper_item_->initialUpdate();
		}
		else
		{
			this->cur_upper_item_ = new MidlineRelationshipItem(nullptr, upper_dental_manager_, &upper_plane_, &midline_ctrledges_upper_);
			this->cur_upper_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			upper_item_list_.push_back(this->cur_upper_item_);
		}

		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			this->cur_lower_item_->initialUpdate();
		}
		else
		{
			this->cur_lower_item_ = new MidlineRelationshipItem(nullptr, lower_dental_manager_, &lower_plane_, &midline_ctrledges_lower_);
			this->cur_lower_item_->setScreenWidthAndHeight(screen_width_, screen_height_);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		_b_upper = true;
		break;

	case ARCH_WIDTH:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->initialUpdate();
		}
		else
		{
			this->cur_upper_item_ = new ArchWidthItem(&upper_dental_manager_->cDental, &arch_width_front_upper_, &arch_width_middle_upper_, &arch_width_rear_upper_, &arch_width_basalbone_upper_,
				nullptr, &arch_width_ctrledges_upper_);
			upper_item_list_.push_back(this->cur_upper_item_);
		}

		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->initialUpdate();
		}
		else
		{
			this->cur_lower_item_ = new ArchWidthItem(&lower_dental_manager_->cDental, &arch_width_front_lower_, &arch_width_middle_lower_, &arch_width_rear_lower_, &arch_width_basalbone_lower_,
				nullptr, &arch_width_ctrledges_lower_);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		break;

	case ARCH_LENGTH:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->initialUpdate();
		}
		else
		{
			this->cur_upper_item_ = new ArchLengthItem(&upper_dental_manager_->cDental, &arch_len_front_upper_, &arch_len_middle_upper_, &arch_len_rear_upper_, &arch_len_whole_upper_,
				nullptr, &arch_len_ctrledges_upper_);
			upper_item_list_.push_back(this->cur_upper_item_);
		}

		if (searchItemInList(_item, lower_item_list_, this->cur_lower_item_))
		{
			this->cur_lower_item_->initialUpdate();
		}
		else
		{
			this->cur_lower_item_ = new ArchLengthItem(&lower_dental_manager_->cDental, &arch_len_front_lower_, &arch_len_middle_lower_, &arch_len_rear_lower_, &arch_len_whole_lower_,
				nullptr, &arch_len_ctrledges_lower_);
			lower_item_list_.push_back(this->cur_lower_item_);
		}
		break;

	case GNATHOTECTUM_HEIGHT:
		if (searchItemInList(_item, upper_item_list_, this->cur_upper_item_))
		{
			this->cur_upper_item_->initialUpdate();
		}
		else
		{
			this->cur_upper_item_ = new  GnathotectumHeightItem(&upper_dental_manager_->cDental, &gnathotectum_height_value_upper_, &gnathotectum_height_upper_);
			upper_item_list_.push_back(this->cur_upper_item_);
		}

		this->cur_lower_item_ = nullptr;
		break;

	default:
		break;
	}

	if (cur_upper_item_)
	{
		cur_upper_item_->updateCtrlParts();
	}
	if (cur_lower_item_)
	{
		cur_lower_item_->updateCtrlParts();
	}

	if (cur_upper_item_ != nullptr)
	{
		this->cur_ope_item_ = cur_upper_item_;
		cur_ope_item_->setSpecialSign(true);
	}
	else if(cur_lower_item_ != nullptr)
	{
		this->cur_ope_item_ = cur_lower_item_;
		cur_ope_item_->setSpecialSign(false);
	}

	bool is_avaliable_item = false;
	switch (_item)
	{
	case MISSING_TOOTH:
		if (upper_dental_manager_ != nullptr || lower_dental_manager_ != nullptr)
		{
			is_avaliable_item = true;
		}
		break;

	case TEETH_WIDTH:
		if (upper_dental_manager_ != nullptr || lower_dental_manager_ != nullptr)
		{
			is_avaliable_item = true;
		}
		break;

	case CUR_LENGTH_OF_DENTAL_ARCH:
		if (upper_dental_manager_ != nullptr || lower_dental_manager_ != nullptr)
		{
			is_avaliable_item = true;
		}
		break;

	case BOLTON_COMPLETELY:
		if (upper_dental_manager_ != nullptr && lower_dental_manager_ != nullptr)
		{
			is_avaliable_item = true;
		}
		break;

	case SPEE_CURVE_DEPTH:
		if (lower_dental_manager_ != nullptr)
		{
			is_avaliable_item = true;
		}
		break;

	case MOLAR_RELATIONSHIP:
		if (upper_dental_manager_ != nullptr && lower_dental_manager_ != nullptr)
		{
			is_avaliable_item = true;
		}
		break;

	case	MIDLINE_RELATIONSHIP:
		if (upper_dental_manager_ != nullptr && lower_dental_manager_ != nullptr)
		{
			is_avaliable_item = true;
		}
		break;

	case ARCH_WIDTH:
		if (upper_dental_manager_ != nullptr || lower_dental_manager_ != nullptr)
		{
			is_avaliable_item = true;
		}
		break;

	case ARCH_LENGTH:
		if (upper_dental_manager_ != nullptr || lower_dental_manager_ != nullptr)
		{
			is_avaliable_item = true;
		}
		break;

	case GNATHOTECTUM_HEIGHT:
		if (upper_dental_manager_ != nullptr)
		{
			is_avaliable_item = true;
		}
		break;

	case OVERLAY_ANALYSIS:
		if (upper_dental_manager_ != nullptr && lower_dental_manager_ != nullptr)
		{
			is_avaliable_item = true;
		}
		break;

	case OCCLUSION_COLORING:
		is_avaliable_item = true;
		break;

	default:
		break;
	}
	if (is_avaliable_item)
	{
		if (find(this->avaliable_item_index_list_.begin(), this->avaliable_item_index_list_.end(), _item) == this->avaliable_item_index_list_.end())
		{
			avaliable_item_index_list_.push_back(_item);
		}
	}

	update();
}

void DentalAnalysisData::updateLabels(EditItemIndex _item_index)
{
	QFont temp_font("Yu Gothic Ui Semibold", Interactive2DLabel::font_size_, QFont::Bold, false);
	temp_font.setLetterSpacing(QFont::AbsoluteSpacing, 1);
	QFontMetrics fm(temp_font);
	QRect rec = fm.boundingRect(QString(tr("Posterior part : ")));

	int append_width = rec.width() + 10;
	int row_height = rec.height() * 1.1f;
	int begin_x = 20, begin_y = screen_height_ - row_height * 2;
	int begin_x_append = begin_x + append_width + 5;
	int begin_x_append2 = begin_x + append_width + 150;

	vector<Interactive2DLabel>().swap(labels_);
	vector<Interactive2DFrame>().swap(frames_);
	switch (_item_index)
	{
	case TEETH_WIDTH:
	{
		if (lower_dental_manager_ || lower_dental_features_)
		{
			if (!tooth_width_list_left_lower.empty() && !tooth_width_list_right_lower.empty())
			{
				vector<Interactive2DLabel> labels;
				int left_lower_list[8] = { 31,32,33,34,35,36,37,38 };
				int right_lower_list[8] = { 41,42,43,44,45,46,47,48 };
				for (int i = 7; i >= 0; --i)
				{
					labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString::number(right_lower_list[i]).append(" : ").append(this->tooth_width_list_right_lower[i]), Point2i(begin_x, begin_y), false));
					labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString::number(left_lower_list[i]).append(" : ").append(this->tooth_width_list_left_lower[i]), Point2i(begin_x_append2, begin_y), false));
					begin_y -= row_height;
				}
				labels.push_back(Interactive2DLabel(Interactive2DLabel::TOP_VIEW, QString(tr("Mandible : ")), Point2i(begin_x, begin_y), false));

				frames_.push_back(Interactive2DFrame(Interactive2DLabel::TOP_VIEW, labels));
			}
			begin_y -= row_height * 2;
		}

		if (upper_dental_manager_ || upper_dental_features_)
		{
			if (!tooth_width_list_left_upper.empty() && !tooth_width_list_right_upper.empty())
			{
				vector<Interactive2DLabel> labels;
				int left_upper_list[8] = { 21,22,23,24,25,26,27,28 };
				int right_upper_list[8] = { 11,12,13,14,15,16,17,18 };
				for (int i = 7; i >= 0; --i)
				{
					labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString::number(right_upper_list[i]).append(" : ").append(this->tooth_width_list_right_upper[i]), Point2i(begin_x, begin_y), false));
					labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString::number(left_upper_list[i]).append(" : ").append(this->tooth_width_list_left_upper[i]), Point2i(begin_x_append2, begin_y), false));
					begin_y -= row_height;
				}
				labels.push_back(Interactive2DLabel(Interactive2DLabel::BOTTOM_VIEW, QString(tr("Maxillary : ")), Point2i(begin_x, begin_y), false));

				frames_.push_back(Interactive2DFrame(Interactive2DLabel::BOTTOM_VIEW, labels));
			}
		}
		break;
	}

	case	CUR_LENGTH_OF_DENTAL_ARCH:
	{
		{
			vector<Interactive2DLabel> labels;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString::fromLocal8Bit("Ⅲ ").append(QString(tr("Congestion     "))).append(QString(tr("Crowding "))).append("> 8mm"), Point2i(begin_x, begin_y), false));
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString::fromLocal8Bit("Ⅱ ").append(QString(tr("Congestion     "))).append(QString("4mm < ")).append(QString(tr("Crowding "))).append(QString::fromLocal8Bit("≤")).append(" 8mm"), Point2i(begin_x, begin_y), false));
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString::fromLocal8Bit("Ⅰ ").append(QString(tr("Congestion     "))).append(QString(tr("Crowding "))).append(QString::fromLocal8Bit("≤")).append(" 4mm"), Point2i(begin_x, begin_y), false));
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Reference : ")), Point2i(begin_x, begin_y), false));

			frames_.push_back(Interactive2DFrame(Interactive2DLabel::DEFAULT_VIEW, labels, false, true));
			begin_y -= row_height * 2;
		}

		if (lower_dental_manager_ || lower_dental_features_)
		{
			vector<Interactive2DLabel> labels;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Crowding : ")).append(this->crowding_severity_lower_), Point2i(begin_x, begin_y), false));
			labels.back().setTextColor(crowding_severity_lower_color_);
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Available space : ")).append(this->cur_length_arch_lower_).append("mm"), Point2i(begin_x, begin_y), false));
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Required space : ")).append(this->space_requared_lower_).append("mm"), Point2i(begin_x, begin_y), false));
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::TOP_VIEW, QString(tr("Mandible : ")), Point2i(begin_x, begin_y), false));

			frames_.push_back(Interactive2DFrame(Interactive2DLabel::TOP_VIEW, labels));
			begin_y -= row_height * 2;
		}

		if (upper_dental_manager_ || upper_dental_features_)
		{
			vector<Interactive2DLabel> labels;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Crowding : ")).append(this->crowding_severity_upper_), Point2i(begin_x, begin_y), false));
			labels.back().setTextColor(crowding_severity_upper_color_);
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Available space : ")).append(this->cur_length_arch_upper_).append("mm"), Point2i(begin_x, begin_y), false));
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Required space : ")).append(this->space_requared_upper_).append("mm"), Point2i(begin_x, begin_y), false));
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::BOTTOM_VIEW, QString(tr("Maxillary : ")), Point2i(begin_x, begin_y), false));

			frames_.push_back(Interactive2DFrame(Interactive2DLabel::BOTTOM_VIEW, labels));
			begin_y -= row_height * 2;
		}

		{
			vector<Interactive2DLabel> labels;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::CROWDING_SWITCH_TO_7_7, QString("7-7"), Point2i(begin_x, begin_y), true));
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::CROWDING_SWITCH_TO_5_5, QString("5-5"), Point2i(begin_x, begin_y), true));
			frames_.push_back(Interactive2DFrame(corwding_compute_mode_motion_, labels));
			frames_.back().setIsCheckBox(true);
			frames_.back().setMotionPointer(&corwding_compute_mode_motion_);
			frames_.back().updateCheckboxState(corwding_compute_mode_motion_);
		}
		break;
	}

	case BOLTON_COMPLETELY:
	{
		{
			vector<Interactive2DLabel> labels;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Normal range of Overall Ratio")).append("     ").append(QString("91.5 % ")).append(QString::fromLocal8Bit("± ")).append(QString("1.51 %")), Point2i(begin_x, begin_y), false));
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Normal range of Anterior Ratio")).append("     ").append(QString("78.8 % ")).append(QString::fromLocal8Bit("± ")).append(QString("1.72 %")), Point2i(begin_x, begin_y), false));
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Reference : ")), Point2i(begin_x, begin_y), false));

			frames_.push_back(Interactive2DFrame(Interactive2DLabel::DEFAULT_VIEW, labels, false, true));
		}
		begin_y -= row_height * 2;
		{
			vector<Interactive2DLabel> labels;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, this->bolton_completely_discrepancy_, Point2i(begin_x, begin_y), false));
			labels.back().setTextColor(bolton_completely_color_);
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Overall Bolton Ratio : ")).append(this->bolton_completely_), Point2i(begin_x, begin_y), false));
			labels.back().setTextColor(bolton_completely_color_);
			begin_y -= row_height;
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, this->bolton_anterior_discrepancy_, Point2i(begin_x, begin_y), false));
			labels.back().setTextColor(bolton_anterior_color_);
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Anterior Bolton Ratio : ")).append(this->bolton_anterior_), Point2i(begin_x, begin_y), false));
			labels.back().setTextColor(bolton_anterior_color_);

			frames_.push_back(Interactive2DFrame(Interactive2DLabel::DEFAULT_VIEW, labels, false, false));
		}
		begin_y -= row_height * 2;

		if (lower_dental_manager_ || lower_dental_features_)
		{
			vector<Interactive2DLabel> labels;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Total width of All Teeth : ")).append(this->sum_whole_width_lower_).append("mm"), Point2i(begin_x, begin_y), false));
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Total width of Anterior Teeth : ")).append(this->sum_anterior_width_lower_).append("mm"), Point2i(begin_x, begin_y), false));
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::TOP_VIEW, QString(tr("Mandible : ")), Point2i(begin_x, begin_y), false));

			frames_.push_back(Interactive2DFrame(Interactive2DLabel::TOP_VIEW, labels));
			begin_y -= row_height * 2;
		}

		if (upper_dental_manager_ || upper_dental_features_)
		{
			vector<Interactive2DLabel> labels;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Total width of All Teeth : ")).append(this->sum_whole_width_upper_).append("mm"), Point2i(begin_x, begin_y), false));
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Total width of Anterior Teeth : ")).append(this->sum_anterior_width_upper_).append("mm"), Point2i(begin_x, begin_y), false));
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::BOTTOM_VIEW, QString(tr("Maxillary : ")), Point2i(begin_x, begin_y), false));

			frames_.push_back(Interactive2DFrame(Interactive2DLabel::BOTTOM_VIEW, labels));
		}
		break;
	}

	case SPEE_CURVE_DEPTH:
	{
		{
			vector<Interactive2DLabel> labels;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Space Needed : ")).append(this->needed_crevice_).append("mm"), Point2i(begin_x, begin_y), false));
			labels.back().setTextColor(QColor(255, 255, 0));

			frames_.push_back(Interactive2DFrame(Interactive2DLabel::DEFAULT_VIEW, labels, false, false));
		}
		begin_y -= row_height * 2;
		{
			vector<Interactive2DLabel> labels;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::LEFT_VIEW, QString(tr("Depth of Spee's curve")).append(QString("      ")).append(QString::number(this->left_spee_depth_)).append("mm"), Point2i(begin_x, begin_y), false));
			begin_y -= row_height * 2;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::LEFT_VIEW, QString(tr("Left : ")), Point2i(begin_x, begin_y), false));

			frames_.push_back(Interactive2DFrame(Interactive2DLabel::LEFT_VIEW, labels));
		}
		begin_y -= row_height * 2;
		{
			vector<Interactive2DLabel> labels;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::RIGHT_VIEW, QString(tr("Depth of Spee's curve")).append(QString("      ")).append(QString::number(this->right_spee_deepth_)).append("mm"), Point2i(begin_x, begin_y), false));
			begin_y -= row_height * 2;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::RIGHT_VIEW, QString(tr("Right : ")), Point2i(begin_x, begin_y), false));

			frames_.push_back(Interactive2DFrame(Interactive2DLabel::RIGHT_VIEW, labels));
		}
		break;
	}

	case MOLAR_RELATIONSHIP:
	{
		{
			QString result;
			QTextStream stream(&result);
			stream.setFieldWidth(10 - QString(tr("Bilateral : ")).length());
			stream.setFieldAlignment(QTextStream::AlignRight);
			stream << bilateral_molar_relationship_;

			vector<Interactive2DLabel> labels;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::FRONT_VIEW, QString(tr("Bilateral : ")), Point2i(begin_x, begin_y), false));

			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, result, Point2i(begin_x_append, begin_y), false));
			labels.back().setTextColor(bilateral_molar_relationship_color_);
			frames_.push_back(Interactive2DFrame(Interactive2DLabel::FRONT_VIEW, labels));
		}
		begin_y -= row_height * 2;
		{
			QString result;
			QTextStream stream(&result);
			stream.setFieldWidth(10 - QString(tr("Left : ")).length());
			stream.setFieldAlignment(QTextStream::AlignRight);
			stream << left_molar_relationship_;

			vector<Interactive2DLabel> labels;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::LEFT_VIEW, QString(tr("Left : ")), Point2i(begin_x, begin_y), false));

			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, result, Point2i(begin_x_append, begin_y), false));
			labels.back().setTextColor(left_molar_relationship_color_);
			frames_.push_back(Interactive2DFrame(Interactive2DLabel::LEFT_VIEW, labels));
		}
		begin_y -= row_height * 2;
		{
			QString result;
			QTextStream stream(&result);
			stream.setFieldWidth(10 - QString(tr("Right : ")).length());
			stream.setFieldAlignment(QTextStream::AlignRight);
			stream << right_molar_relationship_;

			vector<Interactive2DLabel> labels;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::RIGHT_VIEW, QString(tr("Right : ")), Point2i(begin_x, begin_y), false));

			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, result, Point2i(begin_x_append, begin_y), false));
			labels.back().setTextColor(right_molar_relationship_color_);
			frames_.push_back(Interactive2DFrame(Interactive2DLabel::RIGHT_VIEW, labels));
		}
		break;
	}

	case	MIDLINE_RELATIONSHIP:
	{
		vector<Interactive2DLabel> labels;
		labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, midline_relationship_, Point2i(begin_x, begin_y), false));
		labels.back().setTextColor(QColor(255, 255, 0));
		begin_y -= row_height;
		labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Center line Deviation between maxillary and mandible")), Point2i(begin_x, begin_y), false));
		begin_y -= row_height * 2;
		labels.push_back(Interactive2DLabel(Interactive2DLabel::FRONT_VIEW, QString(tr("Centerline REL :")), Point2i(begin_x, begin_y), false));
		frames_.push_back(Interactive2DFrame(Interactive2DLabel::FRONT_VIEW, labels));
		break;
	}

	case ARCH_WIDTH:
	{
		bool enable = true;
		if (lower_dental_manager_ || lower_dental_features_)
		{
			vector<Interactive2DLabel> labels;
			enable = arch_width_rear_lower_ >= 0;
			if (enable)
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Posterior part : ")).append(QString::number(this->arch_width_rear_lower_)).append("mm"), Point2i(begin_x, begin_y), false));
			}
			else
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Posterior part : ")).append("/"), Point2i(begin_x, begin_y), false));
			}
			begin_y -= row_height;

			enable = arch_width_middle_lower_ >= 0;
			if (enable)
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Middle part : ")).append(QString::number(this->arch_width_middle_lower_)).append("mm"), Point2i(begin_x, begin_y), false));
			}
			else
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Middle part : ")).append("/"), Point2i(begin_x, begin_y), false));
			}
			begin_y -= row_height;

			enable = arch_width_front_lower_ >= 0;
			if (enable)
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Anterior part : ")).append(QString::number(this->arch_width_front_lower_)).append("mm"), Point2i(begin_x, begin_y), false));
			}
			else
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Anterior part : ")).append("/"), Point2i(begin_x, begin_y), false));
			}
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::TOP_VIEW, QString(tr("Mandible : ")), Point2i(begin_x, begin_y), false));

			frames_.push_back(Interactive2DFrame(Interactive2DLabel::TOP_VIEW, labels));
			begin_y -= row_height * 2;
		}
		if (upper_dental_manager_ || upper_dental_features_)
		{
			vector<Interactive2DLabel> labels;
			enable = arch_width_rear_upper_ >= 0;
			if (enable)
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Posterior part : ")).append(QString::number(this->arch_width_rear_upper_)).append("mm"), Point2i(begin_x, begin_y), false));
			}
			else
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Posterior part : ")).append("/"), Point2i(begin_x, begin_y), false));
			}
			begin_y -= row_height;

			enable = arch_width_middle_upper_ >= 0;
			if (enable)
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Middle part : ")).append(QString::number(this->arch_width_middle_upper_)).append("mm"), Point2i(begin_x, begin_y), false));
			}
			else
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Middle part : ")).append("/"), Point2i(begin_x, begin_y), false));
			}
			begin_y -= row_height;

			enable = arch_width_front_upper_ >= 0;
			if (enable)
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Anterior part : ")).append(QString::number(this->arch_width_front_upper_)).append("mm"), Point2i(begin_x, begin_y), false));
			}
			else
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Anterior part : ")).append("/"), Point2i(begin_x, begin_y), false));
			}
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::BOTTOM_VIEW, QString(tr("Maxillary : ")), Point2i(begin_x, begin_y), false));

			frames_.push_back(Interactive2DFrame(Interactive2DLabel::BOTTOM_VIEW, labels));
		}
		break;
	}

	case ARCH_LENGTH:
	{
		bool enable = true;
		if (lower_dental_manager_ || lower_dental_features_)
		{
			vector<Interactive2DLabel> labels;
			enable = arch_len_whole_lower_ >= 0;
			if (enable)
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::REAR_SHOW_ONLY_LOWER, QString(tr("Total Length : ")).append(QString::number(this->arch_len_whole_lower_)).append("mm"), Point2i(begin_x, begin_y), false));
			}
			else
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::REAR_SHOW_ONLY_LOWER, QString(tr("Total Length : ")).append("/"), Point2i(begin_x, begin_y), false));
			}
			begin_y -= row_height;

			enable = arch_len_rear_lower_ >= 0;
			if (enable)
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::REAR_SHOW_ONLY_LOWER, QString(tr("Posterior part : ")).append(QString::number(this->arch_len_rear_lower_)).append("mm"), Point2i(begin_x, begin_y), false));
			}
			else
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::REAR_SHOW_ONLY_LOWER, QString(tr("Posterior part : ")).append("/"), Point2i(begin_x, begin_y), false));
			}
			begin_y -= row_height;

			enable = arch_len_middle_lower_ >= 0;
			if (enable)
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::MIDDLE_SHOW_ONLY_LOWER, QString(tr("Middle part : ")).append(QString::number(this->arch_len_middle_lower_)).append("mm"), Point2i(begin_x, begin_y), false));
			}
			else
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::MIDDLE_SHOW_ONLY_LOWER, QString(tr("Middle part : ")).append("/"), Point2i(begin_x, begin_y), false));
			}
			begin_y -= row_height;

			enable = arch_len_front_lower_ >= 0;
			if (enable)
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::FRONT_SHOW_ONLY_LOWER, QString(tr("Anterior part : ")).append(QString::number(this->arch_len_front_lower_)).append("mm"), Point2i(begin_x, begin_y), false));
			}
			else
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::FRONT_SHOW_ONLY_LOWER, QString(tr("Anterior part : ")).append("/"), Point2i(begin_x, begin_y), false));
			}
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::TOP_VIEW, QString(tr("Mandible : ")), Point2i(begin_x, begin_y), false));

			frames_.push_back(Interactive2DFrame(Interactive2DLabel::TOP_VIEW, labels));
			frames_.back().setSpecialSign(false);
			frames_.back().setIsCheckBox(false);
			frames_.back().setMotionPointer(&lower_arch_len_motion_);
			frames_.back().updateCheckboxState(lower_arch_len_motion_);
			begin_y -= row_height * 2;
		}

		if (upper_dental_manager_ || upper_dental_features_)
		{
			vector<Interactive2DLabel> labels;
			enable = arch_len_whole_upper_ >= 0;
			if (enable)
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::REAR_SHOW_ONLY_UPPER, QString(tr("Total Length : ")).append(QString::number(this->arch_len_whole_upper_)).append("mm"), Point2i(begin_x, begin_y), false));
			}
			else
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::REAR_SHOW_ONLY_UPPER, QString(tr("Total Length : ")).append("/"), Point2i(begin_x, begin_y), false));
			}
			begin_y -= row_height;

			enable = arch_len_rear_upper_ >= 0;
			if (enable)
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::REAR_SHOW_ONLY_UPPER, QString(tr("Posterior part : ")).append(QString::number(this->arch_len_rear_upper_)).append("mm"), Point2i(begin_x, begin_y), false));
			}
			else
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::REAR_SHOW_ONLY_UPPER, QString(tr("Posterior part : ")).append("/"), Point2i(begin_x, begin_y), false));
			}
			begin_y -= row_height;

			enable = arch_len_middle_upper_ >= 0;
			if (enable)
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::MIDDLE_SHOW_ONLY_UPPER, QString(tr("Middle part : ")).append(QString::number(this->arch_len_middle_upper_)).append("mm"), Point2i(begin_x, begin_y), false));
			}
			else
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::MIDDLE_SHOW_ONLY_UPPER, QString(tr("Middle part : ")).append("/"), Point2i(begin_x, begin_y), false));
			}
			begin_y -= row_height;

			enable = arch_len_front_upper_ >= 0;
			if (enable)
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::FRONT_SHOW_ONLY_UPPER, QString(tr("Anterior part : ")).append(QString::number(this->arch_len_front_upper_)).append("mm"), Point2i(begin_x, begin_y), false));
			}
			else
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::FRONT_SHOW_ONLY_UPPER, QString(tr("Anterior part : ")).append("/"), Point2i(begin_x, begin_y), false));
			}
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::BOTTOM_VIEW, QString(tr("Maxillary : ")), Point2i(begin_x, begin_y), false));

			frames_.push_back(Interactive2DFrame(Interactive2DLabel::BOTTOM_VIEW, labels));
			frames_.back().setSpecialSign(true);
			frames_.back().setIsCheckBox(false);
			frames_.back().setMotionPointer(&upper_arch_len_motion_);
			frames_.back().updateCheckboxState(upper_arch_len_motion_);
		}
		break;
	}

	case GNATHOTECTUM_HEIGHT:
	{
		vector<Interactive2DLabel> labels;
		labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Palatal cover height : ")).append(this->gnathotectum_height_upper_), Point2i(begin_x, begin_y), false));
		begin_y -= row_height;
		labels.push_back(Interactive2DLabel(Interactive2DLabel::BOTTOM_VIEW, QString(tr("Maxillary : ")), Point2i(begin_x, begin_y), false));
		frames_.push_back(Interactive2DFrame(Interactive2DLabel::BOTTOM_VIEW, labels));
		break;
	}

	case BASAL_BONE_ARCH:
	{
		bool enable = true;
		if (lower_dental_manager_ || lower_dental_features_)
		{
			vector<Interactive2DLabel> labels;
			if ((lower_dental_manager_ ) || lower_dental_features_)
			{
				enable = arch_len_basalbone_lower_ >= 0;
				if (enable)
				{
					labels.push_back(Interactive2DLabel(Interactive2DLabel::BASALBONE_LEN_SHOW_ONLY_LOWER, QString(tr("Basal bone length : ")).append(QString::number(this->arch_len_basalbone_lower_)).append("mm"), Point2i(begin_x, begin_y), true));
				}
				else
				{
					labels.push_back(Interactive2DLabel(Interactive2DLabel::BASALBONE_LEN_SHOW_ONLY_LOWER, QString(tr("Basal bone length : ")).append("/"), Point2i(begin_x, begin_y), true));
				}
				labels.back().setEnable(enable);
				begin_y -= row_height;
			}
			enable = arch_width_basalbone_lower_ >= 0;
			if (enable)
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::BASALBONE_WID_SHOW_ONLY_LOWER, QString(tr("Basal bone width : ")).append(QString::number(this->arch_width_basalbone_lower_)).append("mm"), Point2i(begin_x, begin_y), true));
			}
			else
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::BASALBONE_WID_SHOW_ONLY_LOWER, QString(tr("Basal bone width : ")).append("/"), Point2i(begin_x, begin_y), true));
			}
			labels.back().setEnable(enable);
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::TOP_VIEW, QString(tr("Mandible : ")), Point2i(begin_x, begin_y), false));

			frames_.push_back(Interactive2DFrame(Interactive2DLabel::TOP_VIEW, labels));
			frames_.back().setSpecialSign(false);
			frames_.back().setIsCheckBox(true);
			frames_.back().setMotionPointer(&lower_basal_bone_motion_);
			frames_.back().updateCheckboxState(lower_basal_bone_motion_);
			begin_y -= row_height * 2;
		}

		if (upper_dental_manager_ || upper_dental_features_)
		{
			vector<Interactive2DLabel> labels;
			if ((upper_dental_manager_ ) || upper_dental_features_)
			{
				enable = arch_len_basalbone_upper_ >= 0;
				if (enable)
				{
					labels.push_back(Interactive2DLabel(Interactive2DLabel::BASALBONE_LEN_SHOW_ONLY_UPPER, QString(tr("Basal bone length : ")).append(QString::number(this->arch_len_basalbone_upper_)).append("mm"), Point2i(begin_x, begin_y), true));
				}
				else
				{
					labels.push_back(Interactive2DLabel(Interactive2DLabel::BASALBONE_LEN_SHOW_ONLY_UPPER, QString(tr("Basal bone length : ")).append("/"), Point2i(begin_x, begin_y), true));
				}
				labels.back().setEnable(enable);
				begin_y -= row_height;
			}
			enable = arch_width_basalbone_upper_ >= 0;
			if (enable)
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::BASALBONE_WID_SHOW_ONLY_UPPER, QString(tr("Basal bone width : ")).append(QString::number(this->arch_width_basalbone_upper_)).append("mm"), Point2i(begin_x, begin_y), true));
			}
			else
			{
				labels.push_back(Interactive2DLabel(Interactive2DLabel::BASALBONE_WID_SHOW_ONLY_UPPER, QString(tr("Basal bone width : ")).append("/"), Point2i(begin_x, begin_y), true));
			}
			labels.back().setEnable(enable);
			begin_y -= row_height;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::BOTTOM_VIEW, QString(tr("Maxillary : ")), Point2i(begin_x, begin_y), false));

			frames_.push_back(Interactive2DFrame(Interactive2DLabel::BOTTOM_VIEW, labels));
			frames_.back().setSpecialSign(true);
			frames_.back().setIsCheckBox(true);
			frames_.back().setMotionPointer(&upper_basal_bone_motion_);
			frames_.back().updateCheckboxState(upper_basal_bone_motion_);
			begin_y -= row_height * 2;
		}

		{
			vector<Interactive2DLabel> labels;
			labels.push_back(Interactive2DLabel(Interactive2DLabel::DEFAULT_VIEW, QString(tr("Please adjust basebone analyse point manually")), Point2i(begin_x, begin_y), false));
			labels.back().setTextColor(QColor(255, 255, 0));
			frames_.push_back(Interactive2DFrame(Interactive2DLabel::DEFAULT_VIEW, labels, false));
		}

		break;
	}

	default:
		break;
	}
}

void DentalAnalysisData::alignFramesSize(vector<Interactive2DFrame> &_frames)
{

	vector<int> list;
	float max_width = 0;
	int index = 0;
	for (auto& frame : _frames)
	{
		if (Interactive2DLabel::TOP_VIEW <= frame.motion_id_ && frame.motion_id_ <= Interactive2DLabel::RIGHT_VIEW)
		{
			list.push_back(index);
			if (frame.rect_.width() > max_width)
			{
				max_width = frame.rect_.width();
			}
		}
		++index;
	}

	if (list.size() > 1)
	{
		for (auto& i : list)
		{
			_frames[i].setWidthCompensation(max_width - _frames[i].rect_.width());
		}
	}
}

void DentalAnalysisData::setContext(QString _context)
{
	if (this->cur_ope_item_ != nullptr)
	{
		cur_ope_item_->setContext(_context);
	}
}

void DentalAnalysisData::draw(QPainter *_p)
{
	if (cur_upper_item_ != nullptr)
	{
		if (upper_dental_manager_)
		{
			cur_upper_item_->setVisible(upper_dental_manager_->bVisible_);
		}
		if (upper_dental_features_)
		{
			cur_upper_item_->setVisible(upper_dental_features_->bVisible_);
		}
		if (cur_upper_item_->visible())
		{
			cur_upper_item_->draw(_p);
		}
	}

	if (cur_lower_item_ != nullptr)
	{
		if (lower_dental_manager_)
		{
			cur_lower_item_->setVisible(lower_dental_manager_->bVisible_);
		}
		if (lower_dental_features_)
		{
			cur_lower_item_->setVisible(lower_dental_features_->bVisible_);
		}
		if (cur_lower_item_->visible())
		{
			cur_lower_item_->draw(_p);
		}
	}

	if (!labels_.empty())
	{
		for (int i = 0; i < labels_.size(); ++i)
		{
			labels_[i].draw(_p);
		}
	}

	if (!frames_.empty())
	{
		alignFramesSize(frames_);

		int index = 0;
		for (int i = 0; i < frames_.size(); ++i)
		{
			frames_[i].draw(_p);
			index++;
			qDebug() << "DRAW FRAME width : " << index << " : " << frames_[i].rect_.width() << endl;
		}
	}

}

void DentalAnalysisData::update()
{
	if (this->cur_ope_item_ != nullptr)
	{
		if (upper_dental_manager_ || lower_dental_manager_)
		{
			cur_ope_item_->update();
		}
		if (upper_dental_features_ || lower_dental_features_)
		{
			cur_ope_item_->updateFeatures();
		}

		if (cur_ope_item_->itemIndex() == TEETH_WIDTH || cur_ope_item_->itemIndex() == CUR_LENGTH_OF_DENTAL_ARCH || cur_ope_item_->itemIndex() == BOLTON_COMPLETELY)
		{
			updateSeverityOfCrowding();
			updateBoltonCompletely();
			updateTeethWidthList();
		}
		else if (cur_ope_item_->itemIndex() == MIDLINE_RELATIONSHIP)
		{
			udpateMidlineRelationship();
		}
		else if (cur_ope_item_->itemIndex() == MOLAR_RELATIONSHIP)
		{
			updateMolarRelationship();
		}
		else if (cur_ope_item_->itemIndex() == SPEE_CURVE_DEPTH)
		{
			updateSpeedDeepth();
		}

		cur_ope_item_->updateCtrlParts();
		updateLabels(cur_ope_item_->itemIndex());
	}
}

void DentalAnalysisData::updateLabelsSlot(int _width, int _height)
{
	this->screen_width_ = _width;
	this->screen_height_ = _height;

	QFont font("Microsoft YaHei", 15, QFont::Normal, false);
	font.setLetterSpacing(QFont::AbsoluteSpacing, 1);
	QString sign = QString::fromStdWString(L"●");
	QFontMetrics fm(font);
	QRect rect = fm.boundingRect(sign);
	float ratio = rect.height() * 20 / (float)(_height);
	qDebug() <<" Font Height Ratio : " << ratio << endl;

	int reduce_size = (ratio - 0.5f) * 10 / (int)1;
	reduce_size = reduce_size >= 5 ? 6 : reduce_size;
	reduce_size = reduce_size <= 0 ? 0 : reduce_size;
	Interactive2DLabel::setFontSize(15 - reduce_size);

	if (cur_upper_item_ != nullptr)
	{
		updateLabels(cur_upper_item_->itemIndex());
	}
	else if (cur_lower_item_ != nullptr)
	{
		updateLabels(cur_lower_item_->itemIndex());
	}
}

bool DentalAnalysisData::mousePress(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn )
{
	mouse_pressed_ = true;
	if (cur_upper_item_ != nullptr)
	{
		if (cur_upper_item_->visible() && cur_upper_item_->mousePress(_mouse_x, _mouse_y, _mouse_btn))
		{
			cur_ope_item_ = cur_upper_item_;
			cur_ope_item_->setSpecialSign(true);
			return true;
		}
	}

	if (cur_lower_item_ != nullptr)
	{
		if (cur_lower_item_->visible() && cur_lower_item_->mousePress(_mouse_x, _mouse_y, _mouse_btn))
		{
			cur_ope_item_ = cur_lower_item_;
			cur_ope_item_->setSpecialSign(false);
			return true;
		}
	}

	if (!frames_.empty())
	{
		float logical_x, logical_y;
		emit PSIGNALMANAGER->getQtDeviceToLogicalValueSignal(_mouse_x, logical_x);
		emit PSIGNALMANAGER->getQtDeviceToLogicalValueSignal(_mouse_y, logical_y);
		for (int i = 0; i < frames_.size(); ++i)
		{
			Interactive2DLabel::AccompanyMotionId cur_motion_id = Interactive2DLabel::AccompanyMotionId::DEFAULT_VIEW;
			if (frames_[i].mousePress(logical_x, screen_height_ - logical_y, cur_motion_id))
			{
				switch (cur_motion_id)
				{
				case Interactive2DLabel::DEFAULT_VIEW:
				case Interactive2DLabel::TOP_VIEW:
				case Interactive2DLabel::BOTTOM_VIEW:
				case Interactive2DLabel::FRONT_VIEW:
				case Interactive2DLabel::BACK_VIEW:
				case Interactive2DLabel::LEFT_VIEW:
				case Interactive2DLabel::RIGHT_VIEW:
				{
					emit PSIGNALMANAGER->setViewDirByPhotosMouseSelectedSignal(cur_motion_id);
                    if (cur_mode_ == EditItemIndex(SPEE_CURVE_DEPTH))
					{
						PFusionAlignData->setUpperDentalObjectsVisible(false);
						PFusionAlignData->setLowerDentalObjectsVisible(true);
					}
				}
				break;

				case Interactive2DLabel::FRONT_SHOW_ONLY_UPPER:
				case Interactive2DLabel::MIDDLE_SHOW_ONLY_UPPER:
				case Interactive2DLabel::REAR_SHOW_ONLY_UPPER:
				case Interactive2DLabel::BASALBONE_SHOW_ONLY_UPPER:
				case Interactive2DLabel::FRONT_SHOW_ONLY_LOWER:
				case Interactive2DLabel::MIDDLE_SHOW_ONLY_LOWER:
				case Interactive2DLabel::REAR_SHOW_ONLY_LOWER:
				case Interactive2DLabel::BASALBONE_SHOW_ONLY_LOWER:
				case Interactive2DLabel::BASALBONE_WID_SHOW_ONLY_UPPER:
				case Interactive2DLabel::BASALBONE_LEN_SHOW_ONLY_UPPER:
				case Interactive2DLabel::BASALBONE_WID_SHOW_ONLY_LOWER:
				case Interactive2DLabel::BASALBONE_LEN_SHOW_ONLY_LOWER:
				{
					if (frames_[i].is_check_box_)
					{
						frames_[i].updateCheckboxState(cur_motion_id);
					}

					if (frames_[i].special_sign_ && cur_upper_item_)
					{
						cur_ope_item_ = cur_upper_item_;
					}
					else if (!frames_[i].special_sign_ && cur_lower_item_)
					{
						cur_ope_item_ = cur_lower_item_;
					}
					if (cur_ope_item_ != nullptr)
					{
						cur_ope_item_->setCurrentItemVisibleMode(cur_motion_id);
					}

					if ((Interactive2DLabel::FRONT_SHOW_ONLY_UPPER <= cur_motion_id && cur_motion_id <= Interactive2DLabel::BASALBONE_SHOW_ONLY_UPPER) ||
						(Interactive2DLabel::BASALBONE_WID_SHOW_ONLY_UPPER <= cur_motion_id && cur_motion_id <= Interactive2DLabel::BASALBONE_LEN_SHOW_ONLY_UPPER))
					{
						emit PSIGNALMANAGER->setViewDirByPhotosMouseSelectedSignal(Interactive2DLabel::BOTTOM_VIEW);
					}
					else
					{
						emit PSIGNALMANAGER->setViewDirByPhotosMouseSelectedSignal(Interactive2DLabel::TOP_VIEW);
					}
				}
				break;

				case Interactive2DLabel::CROWDING_SWITCH_TO_5_5:
				{
					if (frames_[i].is_check_box_)
					{
						frames_[i].updateCheckboxState(cur_motion_id);
					}
					emit PSIGNALMANAGER->setSixToSixModeCheckedStateSignal(true, true);
				}
				break;

				case Interactive2DLabel::CROWDING_SWITCH_TO_7_7:
				{
					if (frames_[i].is_check_box_)
					{
						frames_[i].updateCheckboxState(cur_motion_id);
					}
					emit PSIGNALMANAGER->setSixToSixModeCheckedStateSignal(false, true);
				}
				break;
				}
			}
		}
	}

	cur_ope_item_ = nullptr;
	return false;
}

bool DentalAnalysisData::mouseMove(int _mouse_x, int _mouse_y, Qt::MouseButton _mouse_btn )
{
	bool value = false;
	if (cur_ope_item_ != nullptr)
	{
		value = cur_ope_item_->mouseMove(_mouse_x, _mouse_y, _mouse_btn);
		if (value)
		{
			update();
		}
	}

	if (!frames_.empty())
	{
		float logical_x, logical_y;
		emit PSIGNALMANAGER->getQtDeviceToLogicalValueSignal(_mouse_x, logical_x);
		emit PSIGNALMANAGER->getQtDeviceToLogicalValueSignal(_mouse_y, logical_y);
		for (int i = 0; i < frames_.size(); ++i)
		{
			frames_[i].mouseMove(logical_x, screen_height_ - logical_y);
		}
	}
	return value;
}

bool DentalAnalysisData::mouseRelease(int _mouse_x, int _mouse_y)
{
	mouse_pressed_ = false;
	if (cur_ope_item_ != nullptr)
	{
		bool value = cur_ope_item_->mouseRelease(_mouse_x, _mouse_y);
		cur_ope_item_ = nullptr;
		emit PSIGNALMANAGER->updateDentalAnalysisDataReportSignal();
		return value;
	}

	if (!frames_.empty())
	{
		float logical_x, logical_y;
		emit PSIGNALMANAGER->getQtDeviceToLogicalValueSignal(_mouse_x, logical_x);
		emit PSIGNALMANAGER->getQtDeviceToLogicalValueSignal(_mouse_y, logical_y);
		for (int i = 0; i < frames_.size(); ++i)
		{
			frames_[i].mouseRelease(logical_x, screen_height_ - logical_y);
		}
	}
	return false;
}

void DentalAnalysisData::updateSeverityOfCrowding()
{
	float minus(0.0f), fSpaceRequared(0.0f), fCurArchLength(0.0f);
	if (!space_requared_upper_.isEmpty() && !cur_length_arch_upper_.isEmpty())
	{
		fSpaceRequared = space_requared_upper_.toFloat();
		fCurArchLength = cur_length_arch_upper_.toFloat();
		minus = fSpaceRequared - fCurArchLength;
		int temp = (minus * 100) / (int)1;
		minus = temp / 100.0f;

		if (minus > 2.0f)
		{
			crowding_severity_upper_color_ = QColor(255, 0, 0);
		}
		else
		{
			crowding_severity_upper_color_ = QColor(255, 255, 0);
		}

		crowding_severity_upper_ = QString::number(minus).append(QString("mm"));
		if (minus < 0)
		{
			crowding_severity_upper_.append(QString(tr("(Clearance)")));
		}
		else if (minus == 0)
		{
			crowding_severity_upper_.append(QString(tr("(Normal)")));
		}
		else if (0 < minus && minus <= 4)
		{
			crowding_severity_upper_.append(QString(tr("(I-degree)")));
		}
		else if(4 < minus && minus <= 8)
		{
			crowding_severity_upper_.append(QString(tr("(II-degree)")));
		}
		else
		{
			crowding_severity_upper_.append(QString(tr("(III-degree)")));
		}
	}
	else
	{
		crowding_severity_upper_ = QString("");
	}

	if (!space_requared_lower_.isEmpty() && !cur_length_arch_lower_.isEmpty())
	{
		fSpaceRequared = space_requared_lower_.toFloat();
		fCurArchLength = cur_length_arch_lower_.toFloat();
		minus = fSpaceRequared - fCurArchLength;
		int temp = (minus * 100) / (int)1;
		minus = temp / 100.0f;

		if (minus > 2.0f)
		{
			crowding_severity_lower_color_ = QColor(255, 0, 0);
		}
		else
		{
			crowding_severity_lower_color_ = QColor(255, 255, 0);
		}

		crowding_severity_lower_ = QString::number(minus).append(QString("mm"));
		if (minus < 0)
		{
			crowding_severity_lower_.append(QString(tr("(Clearance)")));
		}
		else if (minus == 0)
		{
			crowding_severity_lower_.append(QString(tr("(Normal)")));
		}
		else if (0 < minus && minus <= 4)
		{
			crowding_severity_lower_.append(QString(tr("(I-degree)")));
		}
		else if (4 < minus && minus <= 8)
		{
			crowding_severity_lower_.append(QString(tr("(II-degree)")));
		}
		else
		{
			crowding_severity_lower_.append(QString(tr("(III-degree)")));
		}
	}
	else
	{
		crowding_severity_lower_ = QString("");
	}
}

void DentalAnalysisData::updateBoltonAnterior()
{
	if (!sum_anterior_width_upper_.isEmpty() && !sum_anterior_width_lower_.isEmpty())
	{
		float bolton_upper = sum_anterior_width_upper_.toFloat();
		float bolton_lower = sum_anterior_width_lower_.toFloat();

		float boltonAnterior = 0.0f;
		if(fabs(bolton_lower) > 0.0001f && bolton_upper != 0)
		{
			boltonAnterior = bolton_lower / bolton_upper;
			int t1 = (boltonAnterior * 10000) / (int)1;
			boltonAnterior = t1 / 100.0f;
		}

		if (boltonAnterior > 80.52 || boltonAnterior < 77.08)
		{
			bolton_anterior_color_ = QColor(255, 0, 0);
		}
		else
		{
			bolton_anterior_color_ = QColor(255, 255, 0);
		}

		this->bolton_anterior_ = QString::number(boltonAnterior).append("%");

		float discrepancy = 0;
		if (boltonAnterior > 78.8f)
		{
			discrepancy = bolton_lower - bolton_upper * 0.788f;
		}
		else
		{
			discrepancy = bolton_upper - bolton_lower / 0.788f;
		}
		int t2 = (discrepancy * 100) / (int)1;
		discrepancy = t2 / 100.0f;
		bolton_anterior_larger_value_ = QString::number(discrepancy);

		if (boltonAnterior > 78.8f)
		{
			bolton_anterior_larger_describe_ = QString(tr("Mandibular Excess"));
			bolton_anterior_discrepancy_ = QString(tr("Mandibular Excess : ")).append(QString::number(discrepancy)).append("mm");
		}
		else
		{
			bolton_anterior_larger_describe_ = QString(tr("Maxillary Excess"));
			bolton_anterior_discrepancy_ = QString(tr("Maxillary Excess : ")).append(QString::number(discrepancy)).append("mm");
		}
	}
	else
	{
		this->bolton_anterior_ = QString("");
		bolton_anterior_discrepancy_ = QString("");
	}
}

void DentalAnalysisData::updateBoltonCompletely()
{
	updateBoltonAnterior();
	if (!sum_whole_width_upper_.isEmpty() && !sum_whole_width_lower_.isEmpty())
	{
		float bolton_upper = sum_whole_width_upper_.toFloat();
		float bolton_lower = sum_whole_width_lower_.toFloat();

		float t = 0;
		if (bolton_upper != 0)
		{
			t = bolton_lower / bolton_upper;
			int t1 = (t * 10000) / (int)1;
			t = t1 / 100.0f;
		}

		if (t > 93.01 || t < 89.99)
		{
			bolton_completely_color_ = QColor(255, 0, 0);
		}
		else
		{
			bolton_completely_color_ = QColor(255, 255, 0);
		}

		this->bolton_completely_ = QString::number(t).append("%");

		float discrepancy = 0;
		if (t > 91.5f)
		{
			discrepancy = bolton_lower - bolton_upper * 0.915f;
		}
		else
		{
			discrepancy = bolton_upper - bolton_lower / 0.915f;
		}
		int t2 = (discrepancy * 100) / (int)1;
		discrepancy = t2 / 100.0f;
		bolton_completely_larger_value_ = QString::number(discrepancy);

		if (t > 91.5f)
		{
			bolton_completely_larger_describe_ = QString(tr("Mandibular Excess"));
			bolton_completely_discrepancy_ = QString(tr("Mandibular Excess : ")).append(QString::number(discrepancy)).append("mm");
		}
		else
		{
			bolton_completely_larger_describe_ = QString(tr("Maxillary Excess"));
			bolton_completely_discrepancy_ = QString(tr("Maxillary Excess : ")).append(QString::number(discrepancy)).append("mm");
		}
	}
	else
	{
		this->bolton_completely_ = QString("");
		this->bolton_completely_discrepancy_ = QString("");
	}
}

void DentalAnalysisData::udpateMidlineRelationship()
{
	Point3m axis_x_upper = upper_plane_.axisXVector;
	Point3m axis_x_lower = lower_plane_.axisXVector;
	if (axis_x_lower * axis_x_upper < 0)
	{
		axis_x_lower *= -1.0f;
	}
	Point3m axis_x = ((axis_x_upper + axis_x_lower) / 2.0f).Normalize();
	Point3m vec = upper_plane_.centerPoint - lower_plane_.centerPoint;
	float value = vec * axis_x;
	if (abs(value) < 1e-5)
	{
		value = 0;
	}
	int temp_value = (value * 100) / (int)1;
	value = temp_value / 100.0f;
	if (value > 0)
	{
		midline_relationship_ = QString(tr("To the left     "));
	}
	else
	{
		midline_relationship_ = QString(tr("To the right     "));
	}
	float mid_distance = abs(value);
	midline_relationship_.append(QString::number(mid_distance)).append(QString("mm"));
}

void DentalAnalysisData::updateMolarRelationship()
{
	Point3m plane_pos, plane_normal, eye_direct;
	Point3m plane_pos_left, plane_pos_right, plane_normal_left, plane_normal_right;
	if (this->lower_dental_manager_ != nullptr)
	{
		plane_pos = lower_dental_manager_->cDental.basePlane.center;
		plane_normal = lower_dental_manager_->cDental.basePlane.axisZV;
		eye_direct = lower_dental_manager_->cDental.basePlane.axisYV;

		plane_pos_left = (left_molar_node_upper_ + left_molar_node_lower_) / 2.0f;
		plane_pos_right = (right_molar_node_upper_ + right_molar_node_lower_) / 2.0f;
		plane_normal_left = -lower_dental_manager_->cDental.basePlane.axisXV;
		plane_normal_right = lower_dental_manager_->cDental.basePlane.axisXV;
	}
	else if (this->lower_dental_features_ != nullptr)
	{
		plane_normal = -lower_dental_features_->base_plane_.axisZV;
		eye_direct = lower_dental_features_->base_plane_.axisYV;

		plane_pos_left = left_molar_node_lower_;
		plane_pos_right = right_molar_node_lower_;

		Point3m left_view_direct, right_view_direct;
		bool get_prefer_view = lower_dental_features_->getPreferViewDirect(left_view_direct, right_view_direct);
		if (get_prefer_view)
		{
			plane_normal_left = left_view_direct;
			plane_normal_right = right_view_direct;
		}
		else
		{
			plane_normal_left = -lower_dental_features_->base_plane_.axisXV;
			plane_normal_right = lower_dental_features_->base_plane_.axisXV;
		}
	}

	Point3m proj_left_upper = UtilityTools::getInstance()->getProjPointOnPlane(left_molar_node_upper_, plane_pos_left, -plane_normal);
	Point3m proj_right_upper = UtilityTools::getInstance()->getProjPointOnPlane(right_molar_node_upper_, plane_pos_right, -plane_normal);
	Point3m proj_left_lower = UtilityTools::getInstance()->getProjPointOnPlane(left_molar_node_lower_, plane_pos_left, -plane_normal);
	Point3m proj_right_lower = UtilityTools::getInstance()->getProjPointOnPlane(right_molar_node_lower_, plane_pos_right, -plane_normal);
	proj_left_upper = UtilityTools::getInstance()->getProjPointOnPlane(proj_left_upper, plane_pos_left, -plane_normal_left);
	proj_right_upper = UtilityTools::getInstance()->getProjPointOnPlane(proj_right_upper, plane_pos_right, -plane_normal_right);
	proj_left_lower = UtilityTools::getInstance()->getProjPointOnPlane(proj_left_lower, plane_pos_left, -plane_normal_left);
	proj_right_lower = UtilityTools::getInstance()->getProjPointOnPlane(proj_right_lower, plane_pos_right, -plane_normal_right);

	float length;
	Point3m temp_vec;
	int left_class, right_class;
	float left_length, right_length;
	temp_vec = proj_left_upper - proj_left_lower;
	length = sqrtf(temp_vec * temp_vec);
	left_length = length;
	if (length < 1.0f)
	{
		this->left_molar_relationship_ = QString(tr("Class I"));
		this->left_molar_relationship_color_ = QColor(255, 255, 0);
		left_class = 1;
	}
	else
	{
		this->left_molar_relationship_color_ = QColor(255, 0, 0);
		if (temp_vec * eye_direct > 0)
		{
			this->left_molar_relationship_ = QString(tr("Class II"));
			left_class = 2;
		}
		else
		{
			this->left_molar_relationship_ = QString(tr("Class III"));
			left_class = 3;
		}
	}

	temp_vec = proj_right_upper - proj_right_lower;
	length = sqrtf(temp_vec * temp_vec);
	right_length = length;
	if (length < 1.0f)
	{
		this->right_molar_relationship_ = QString(tr("Class I"));
		this->right_molar_relationship_color_ = QColor(255, 255, 0);
		right_class = 1;
	}
	else
	{
		this->right_molar_relationship_color_ = QColor(255, 0, 0);
		if (temp_vec * eye_direct > 0)
		{
			this->right_molar_relationship_ = QString(tr("Class II"));
			right_class = 2;
		}
		else
		{
			this->right_molar_relationship_ = QString(tr("Class III"));
			right_class = 3;
		}
	}

	this->bilateral_molar_relationship_color_ = QColor(255, 0, 0);
	if (left_class == right_class)
	{
		if (left_class == 1)
		{
			this->bilateral_molar_relationship_ = QString(tr("Class I"));
			this->bilateral_molar_relationship_color_ = QColor(255, 255, 0);
		}
		if (left_class == 2)
		{
			this->bilateral_molar_relationship_ = QString(tr("Class II"));
		}
		if (left_class == 3)
		{
			this->bilateral_molar_relationship_ = QString(tr("Class III"));
		}
	}
	else
	{
		if ((left_class == 1 && right_class == 2) || (left_class == 2 && right_class == 1))
		{
			this->bilateral_molar_relationship_ = QString(tr("Class II subdivision"));
		}
		else if ((left_class == 1 && right_class == 3) || (left_class == 3 && right_class == 1))
		{
			this->bilateral_molar_relationship_ = QString(tr("Class III subdivision"));
		}
		else if ((left_class == 3 && right_class == 2) || (left_class == 2 && right_class == 3))
		{
			if (abs(left_length) > abs(right_length))
			{
				if (left_class == 2)
				{
					this->bilateral_molar_relationship_ = QString(tr("Class II subdivision"));
				}
				else if (left_class == 3)
				{
					this->bilateral_molar_relationship_ = QString(tr("Class III subdivision"));
				}
			}
			else
			{
				if (right_class == 2)
				{
					this->bilateral_molar_relationship_ = QString(tr("Class II subdivision"));
				}
				else if (right_class == 3)
				{
					this->bilateral_molar_relationship_ = QString(tr("Class III subdivision"));
				}
			}
		}
	}
}

void DentalAnalysisData::updateSpeedDeepth()
{
	float curve_curvature = (left_spee_depth_+ right_spee_deepth_) * 0.5f;
	float needed_crevice = curve_curvature + 0.5f;
	int temp = (needed_crevice * 100) / (int)1;
	needed_crevice = temp / 100.0f;
	this->needed_crevice_ = QString::number(needed_crevice);
}

void DentalAnalysisData::updateTeethWidthList()
{
	vector<QString>().swap(tooth_width_list_right_upper);
	int right_upper_list[8] = { 11,12,13,14,15,16,17,18 };
	for (int i = 0; i < 8; ++i)
	{
		int cur_fdi = right_upper_list[i];

		bool found = false;
		float width;
		if (upper_dental_manager_)
		{
			for (int j = 0; j < STANDARD_TOOTH_SUM; ++j)
			{
				if (upper_dental_manager_->cDental.bToothExist[j])
				{
					if (upper_dental_manager_->cDental.teeth[j].strFDI.toInt() == cur_fdi)
					{
						found = true;
						width = upper_dental_manager_->cDental.teeth[j].fToothWidth;
						tooth_width_list_right_upper.push_back(QString::number(width).append("mm"));
						break;
					}
				}
			}
		}

		if (upper_dental_features_)
		{
			for (int j = 0; j < upper_dental_features_->teeth_list_.size(); ++j)
			{
				if (upper_dental_features_->teeth_list_[j].fdi_ == cur_fdi)
				{
					found = true;
					width = upper_dental_features_->teeth_list_[j].f_tooth_width_;
					tooth_width_list_right_upper.push_back(QString::number(width).append("mm"));
					break;
				}
			}
		}

		if (!found)
		{
			tooth_width_list_right_upper.push_back(QString("/"));
		}
	}

	vector<QString>().swap(tooth_width_list_left_upper);
	int left_upper_list[8] = { 21,22,23,24,25,26,27,28 };
	for (int i = 0; i < 8; ++i)
	{
		int cur_fdi = left_upper_list[i];

		bool found = false;
		float width;
		if (upper_dental_manager_)
		{
			for (int j = 0; j < STANDARD_TOOTH_SUM; ++j)
			{
				if (upper_dental_manager_->cDental.bToothExist[j])
				{
					if (upper_dental_manager_->cDental.teeth[j].strFDI.toInt() == cur_fdi)
					{
						found = true;
						width = upper_dental_manager_->cDental.teeth[j].fToothWidth;
						tooth_width_list_left_upper.push_back(QString::number(width).append("mm"));
						break;
					}
				}
			}
		}

		if (upper_dental_features_)
		{
			for (int j = 0; j < upper_dental_features_->teeth_list_.size(); ++j)
			{
				if (upper_dental_features_->teeth_list_[j].fdi_ == cur_fdi)
				{
					found = true;
					width = upper_dental_features_->teeth_list_[j].f_tooth_width_;
					tooth_width_list_left_upper.push_back(QString::number(width).append("mm"));
					break;
				}
			}
		}

		if (!found)
		{
			tooth_width_list_left_upper.push_back(QString("/"));
		}
	}

	vector<QString>().swap(tooth_width_list_right_lower);
	int right_lower_list[8] = { 41,42,43,44,45,46,47,48 };
	for (int i = 0; i < 8; ++i)
	{
		int cur_fdi = right_lower_list[i];

		bool found = false;
		float width;
		if (lower_dental_manager_)
		{
			for (int j = 0; j < STANDARD_TOOTH_SUM; ++j)
			{
				if (lower_dental_manager_->cDental.bToothExist[j])
				{
					if (lower_dental_manager_->cDental.teeth[j].strFDI.toInt() == cur_fdi)
					{
						found = true;
						width = lower_dental_manager_->cDental.teeth[j].fToothWidth;
						tooth_width_list_right_lower.push_back(QString::number(width).append("mm"));
						break;
					}
				}
			}
		}

		if (lower_dental_features_)
		{
			for (int j = 0; j < lower_dental_features_->teeth_list_.size(); ++j)
			{
				if (lower_dental_features_->teeth_list_[j].fdi_ == cur_fdi)
				{
					found = true;
					width = lower_dental_features_->teeth_list_[j].f_tooth_width_;
					tooth_width_list_right_lower.push_back(QString::number(width).append("mm"));
					break;
				}
			}
		}

		if (!found)
		{
			tooth_width_list_right_lower.push_back(QString("/"));
		}
	}

	vector<QString>().swap(tooth_width_list_left_lower);
	int left_lower_list[8] = { 31,32,33,34,35,36,37,38 };
	for (int i = 0; i < 8; ++i)
	{
		int cur_fdi = left_lower_list[i];

		bool found = false;
		float width;
		if (lower_dental_manager_)
		{
			for (int j = 0; j < STANDARD_TOOTH_SUM; ++j)
			{
				if (lower_dental_manager_->cDental.bToothExist[j])
				{
					if (lower_dental_manager_->cDental.teeth[j].strFDI.toInt() == cur_fdi)
					{
						found = true;
						width = lower_dental_manager_->cDental.teeth[j].fToothWidth;
						tooth_width_list_left_lower.push_back(QString::number(width).append("mm"));
						break;
					}
				}
			}
		}

		if (lower_dental_features_)
		{
			for (int j = 0; j < lower_dental_features_->teeth_list_.size(); ++j)
			{
				if (lower_dental_features_->teeth_list_[j].fdi_ == cur_fdi)
				{
					found = true;
					width = lower_dental_features_->teeth_list_[j].f_tooth_width_;
					tooth_width_list_left_lower.push_back(QString::number(width).append("mm"));
					break;
				}
			}
		}

		if (!found)
		{
			tooth_width_list_left_lower.push_back(QString("/"));
		}
	}

	updateTeethWidthEdgesList();
}

void DentalAnalysisData::updateTeethWidthEdgesList()
{
	vector<FEdge>().swap(tooth_width_edges_upper_);
	if (upper_dental_features_ != nullptr)
	{
		for (int i = 0; i < upper_dental_features_->teeth_list_.size(); ++i)
		{
			int fdi = upper_dental_features_->teeth_list_[i].fdi_;

			Point3m a = upper_dental_features_->teeth_list_[i].width_edge_.vertA;
			Point3m b = upper_dental_features_->teeth_list_[i].width_edge_.vertB;
			if (a == Point3m(0, 0, 0) && a == b)
			{
				continue;
			}
			tooth_width_edges_upper_.push_back(FEdge(a, b));
		}
	}

	vector<FEdge>().swap(tooth_width_edges_lower_);
	if (lower_dental_features_ != nullptr)
	{
		for (int i = 0; i < lower_dental_features_->teeth_list_.size(); ++i)
		{
			int fdi = lower_dental_features_->teeth_list_[i].fdi_;

			Point3m a = lower_dental_features_->teeth_list_[i].width_edge_.vertA;
			Point3m b = lower_dental_features_->teeth_list_[i].width_edge_.vertB;
			if (a == Point3m(0, 0, 0) && a == b)
			{
				continue;
			}
			tooth_width_edges_lower_.push_back(FEdge(a, b));
		}
	}
}

void DentalAnalysisData::clearItems()
{
	if (cur_upper_item_ != nullptr)
	{
		delete cur_upper_item_;
		cur_upper_item_ = nullptr;
	}
	if (cur_lower_item_ != nullptr)
	{
		delete cur_lower_item_;
		cur_lower_item_ = nullptr;
	}
	cur_ope_item_ = nullptr;
}

bool DentalAnalysisData::searchItemInList(EditItemIndex _item_index, vector<EditItem*>& _item_list, EditItem*& _result)
{
	for (int i = 0; i < _item_list.size(); ++i)
	{
		if (_item_list[i]->itemIndex() == _item_index)
		{
			_result = _item_list[i];
			return true;
		}
	}
	_result = nullptr;
	return false;
}

bool DentalAnalysisData::deleteItemInList(EditItemIndex _item_index, vector<EditItem*>& _item_list)
{
	auto ite = _item_list.begin();
	while (ite != _item_list.end())
	{
		if ((*ite)->itemIndex() == _item_index)
		{
			_item_list.erase(ite);
		}
		else
		{
			++ite;
		}
	}
	return true;
}

void DentalAnalysisData::unFocusAllFrames()
{
	for (int i = 0; i < this->frames_.size(); ++i)
	{
		frames_[i].unFocus();
	}
	update();
}

bool DentalAnalysisData::picked()
{
	if (cur_ope_item_ != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void DentalAnalysisData::setScreenWidthAndHeight(float _width, float _height)
{
	screen_width_ = _width;
	screen_height_ = _height;
}

void DentalAnalysisData::setOverjetValue(float _value)
{
	overjet_value_ = _value;
}

void DentalAnalysisData::setOverbiteValue(float _value)
{
	overbite_value_ = _value;
}

void DentalAnalysisData::setOverjetRank(QString _rank)
{
	overjet_rank_ = _rank;
	overjet_rank_translated_ = tr(_rank.toStdString().c_str());
}

void DentalAnalysisData::setOverbiteRank(QString _rank)
{
	overbite_rank_ = _rank;
	overbite_rank_translated_ = tr(_rank.toStdString().c_str());
}

void DentalAnalysisData::setRemark(QString _remark)
{
	remark_ = _remark;
}

void DentalAnalysisData::setPatientName(QString _name)
{
	patient_name_ = _name;
}

void DentalAnalysisData::setDoctorName(QString _name)
{
	doctor_name_ = _name;
}

void DentalAnalysisData::copyToClipboard()
{
	QString context;
	generateCopyContext(context);

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setText(context);

	emit PSIGNALMANAGER->setProgressBarSignal(100, QString(tr("Copy successful")));
}

void DentalAnalysisData::generateCopyContext(QString& _context)
{
	QString context, sub_context;

	sub_context = QString(tr("Remark :"));
	sub_context.append("{");
	sub_context.append(remark_);
	sub_context.append("}");
	sub_context.append("\n\n");
	context.append(sub_context);

	if (confirm_missing_fdi_)
	{
		sub_context = QString(tr("Missing tooth :"));
		sub_context.append("{");
		if (PFusionAlignData->upper_dental_features_)
		{
			for (auto& fdi : missing_tooth_upper_)
			{
				sub_context.append(QString::number(fdi)).append(",");
			}
		}
		if (PFusionAlignData->lower_dental_features_)
		{
			for (auto& fdi : missing_tooth_lower_)
			{
				sub_context.append(QString::number(fdi)).append(",");
			}
		}
		sub_context.append("}");
		sub_context.append("\n\n");
		context.append(sub_context);
	}

	if (confirm_crowding_)
	{

		sub_context = QString(tr("Crowding :"));
		sub_context.append("\n");
		if (PFusionAlignData->upper_dental_features_)
		{
			sub_context.append(tr("Maxillary arch "));
			sub_context.append("{").append(crowding_severity_upper_).append("},");
			sub_context.append(tr("Required space")).append("{").append(space_requared_upper_).append("}mm,");
			sub_context.append(tr("Available space")).append("{").append(cur_length_arch_upper_).append("}mm");
			sub_context.append("\n");
		}
		if (PFusionAlignData->lower_dental_features_)
		{
			sub_context.append(tr("Mandible arch "));
			sub_context.append("{").append(crowding_severity_lower_).append("},");
			sub_context.append(tr("Required space")).append("{").append(space_requared_lower_).append("}mm,");
			sub_context.append(tr("Available space")).append("{").append(cur_length_arch_lower_).append("}mm");
			sub_context.append("\n");
		}
		context.append(sub_context);
		context.append("\n");
	}

	if (confirm_bolton_)
	{

		sub_context = QString(tr("Bolton :"));
		sub_context.append(tr("Anterior Bolton Ratio")).append("{").append(bolton_anterior_).append("},");
		sub_context.append(tr("Discrepancy of anterior teeth")).append("{").append(bolton_anterior_discrepancy_).append("},");
		sub_context.append(tr("Overall Bolton Ratio")).append("{").append(bolton_completely_).append("}");
		sub_context.append(tr("Discrepancy of overall teeth")).append("{").append(bolton_completely_discrepancy_).append("}");
		sub_context.append("\n\n");
		context.append(sub_context);
	}

	if (confirm_spee_)
	{

		sub_context = QString(tr("Spee curve :"));
		sub_context.append(tr("Right")).append("{").append(QString::number(this->right_spee_deepth_)).append("}").append("mm,");
		sub_context.append(tr("Left")).append("{").append(QString::number(this->left_spee_depth_)).append("}").append("mm,");
		sub_context.append(tr("Space Needed")).append("{").append(this->needed_crevice_).append("}").append("mm");
		sub_context.append("\n\n");
		context.append(sub_context);
	}

	if (confirm_molar_)
	{
		sub_context = QString(tr("Molar REL :"));
		sub_context.append(tr("Right")).append("{").append(this->right_molar_relationship_).append("},");
		sub_context.append(tr("Left")).append("{").append(this->left_molar_relationship_).append("}");
		sub_context.append("\n\n");
		context.append(sub_context);
	}

	if (confirm_anterior_parameter_)
	{

		sub_context = QString(tr("Centerline REL :"));
		sub_context.append(" ").append(tr("Center line Deviation between maxillary and mandible")).append(" ");
		sub_context.append("{").append(midline_relationship_).append("}");
		sub_context.append("\n\n");
		context.append(sub_context);
	}

	if (confirm_arch_width_)
	{
		if (PFusionAlignData->upper_dental_features_)
		{

			sub_context = QString(tr("The width of maxillary arch"));
			sub_context.append(" ").append(tr("Anterior part : ")).append("{").append(QString::number(arch_width_front_upper_)).append("}mm,");
			sub_context.append(" ").append(tr("Middle part : ")).append("{").append(QString::number(arch_width_middle_upper_)).append("}mm,");
			sub_context.append(" ").append(tr("Posterior part : ")).append("{").append(QString::number(arch_width_rear_upper_)).append("}mm,");
			sub_context.append("\n");
			context.append(sub_context);
		}
		if (PFusionAlignData->lower_dental_features_)
		{

			sub_context = QString(tr("The width of mandible arch"));
			sub_context.append(" ").append(tr("Anterior part : ")).append("{").append(QString::number(arch_width_front_lower_)).append("}mm,");
			sub_context.append(" ").append(tr("Middle part : ")).append("{").append(QString::number(arch_width_middle_lower_)).append("}mm,");
			sub_context.append(" ").append(tr("Posterior part : ")).append("{").append(QString::number(arch_width_rear_lower_)).append("}mm,");
			sub_context.append("\n");
			context.append(sub_context);
		}
		context.append("\n");
	}

	if (confirm_arch_length_)
	{
		if (PFusionAlignData->upper_dental_features_)
		{

			sub_context = QString(tr("The length of maxillary arch"));
			sub_context.append(" ").append(tr("Anterior part : ")).append("{").append(QString::number(arch_len_front_upper_)).append("}mm,");
			sub_context.append(" ").append(tr("Middle part : ")).append("{").append(QString::number(arch_len_middle_upper_)).append("}mm,");
			sub_context.append(" ").append(tr("Posterior part : ")).append("{").append(QString::number(arch_len_rear_upper_)).append("}mm,");
			sub_context.append(" ").append(tr("Total Length : ")).append("{").append(QString::number(arch_len_whole_upper_)).append("}mm,");
			sub_context.append("\n");
			context.append(sub_context);
		}
		if (PFusionAlignData->lower_dental_features_)
		{

			sub_context = QString(tr("The length of mandible arch"));
			sub_context.append(" ").append(tr("Anterior part : ")).append("{").append(QString::number(arch_len_front_lower_)).append("}mm,");
			sub_context.append(" ").append(tr("Middle part : ")).append("{").append(QString::number(arch_len_middle_lower_)).append("}mm,");
			sub_context.append(" ").append(tr("Posterior part : ")).append("{").append(QString::number(arch_len_rear_lower_)).append("}mm,");
			sub_context.append(" ").append(tr("Total Length : ")).append("{").append(QString::number(arch_len_whole_lower_)).append("}mm,");
			sub_context.append("\n");
			context.append(sub_context);
		}
		context.append("\n");
	}

	if (confirm_basal_bone_arch_)
	{
		sub_context = QString(tr("Basal bone width"));
		if (PFusionAlignData->upper_dental_features_)
		{
			sub_context.append(" ").append(tr("Maxilla : ")).append("{").append(QString::number(arch_width_basalbone_upper_)).append("}mm ");
		}
		if (PFusionAlignData->lower_dental_features_)
		{
			sub_context.append(" ").append(tr("Mandible : ")).append("{").append(QString::number(arch_width_basalbone_lower_)).append("}mm ");
		}
		sub_context.append("\n");
		context.append(sub_context);

		sub_context = QString(tr("Basal bone length"));
		if (PFusionAlignData->upper_dental_features_)
		{
			sub_context.append(" ").append(tr("Maxilla : ")).append("{").append(QString::number(arch_len_basalbone_upper_)).append("}mm ");
		}
		if (PFusionAlignData->lower_dental_features_)
		{
			sub_context.append(" ").append(tr("Mandible : ")).append("{").append(QString::number(arch_len_basalbone_lower_)).append("}mm");
		}
		sub_context.append("\n");
		context.append(sub_context);
		context.append("\n");
	}

	if (confirm_gnathotectum_height_)
	{

		sub_context = QString(tr("Palatal cover height :"));
		sub_context.append(" ").append("{").append(gnathotectum_height_upper_).append("}");
		sub_context.append("\n\n");
		context.append(sub_context);
	}

	if (confirm_overlay_)
	{

		sub_context = QString(tr("Overbite :"));
		if (overbite_value_ < 0)
		{
			sub_context.append("{/}");
		}
		else
		{
			sub_context.append("{").append(this->overbite_rank_translated_).append("},");
			sub_context.append("{").append(QString::number(this->overbite_value_)).append("}").append("mm");
		}
		sub_context.append("\n");
		context.append(sub_context);

		sub_context = QString(tr("Overjet :"));
		if (overjet_value_ < 0)
		{
			sub_context.append("{/}");
		}
		else
		{
			sub_context.append("{").append(this->overjet_rank_translated_).append("},");
			sub_context.append("{").append(QString::number(this->overjet_value_)).append("}").append("mm");
		}
		sub_context.append("\n");
		context.append(sub_context);
		context.append("\n");
	}

	_context = context;
}

void DentalAnalysisData::setConfirmState(QString _item_name, bool _state)
{
	if (_item_name == ITEM_MISSING_TOOTH)
	{
		confirm_missing_fdi_ = _state;
		return;
	}
	if (_item_name == ITEM_TOOTH_WIDTH)
	{
		confirm_tooth_width_ = _state;
		return;
	}
	if (_item_name == ITEM_CROWDING)
	{
		confirm_crowding_ = _state;
		return;
	}
	if (_item_name == ITEM_BOLTON_RATIO)
	{
		confirm_bolton_ = _state;
		return;
	}
	if (_item_name == ITEM_SPEE)
	{
		confirm_spee_ = _state;
		return;
	}
	if (_item_name == ITEM_MOLAR_REL_RATIO)
	{
		confirm_molar_ = _state;
		return;
	}
	if (_item_name == ITEM_CENTERLINE_REL)
	{
		confirm_anterior_parameter_ = _state;
		return;
	}
	if (_item_name == ITEM_ARCH_WIDTH )
	{
		confirm_arch_width_ = _state;
		return;
	}
	if (_item_name == ITEM_ARCH_LENGTH)
	{
		confirm_arch_length_ = _state;
		return;
	}
	if (_item_name == ITEM_BASAL_BONE)
	{
		confirm_basal_bone_arch_ = _state;
		return;
	}
	if (_item_name == ITEM_PALATAL_HEIGHT)
	{
		confirm_gnathotectum_height_ = _state;
		return;
	}
	if (_item_name == ITEM_OJ_OB)
	{
		confirm_overlay_ = _state;
		return;
	}
}

void DentalAnalysisData::redoCrowdingAnalysis()
{
	if (find(this->avaliable_item_index_list_.begin(), this->avaliable_item_index_list_.end(), CUR_LENGTH_OF_DENTAL_ARCH) != this->avaliable_item_index_list_.end())
	{
		EditItemIndex index = cur_mode_;

		cur_length_arch_ctrlnodes_upper_.clear();
		cur_length_arch_ctrlnodes_lower_.clear();
        deleteItemInList(EditItemIndex(CUR_LENGTH_OF_DENTAL_ARCH), upper_item_list_);
        deleteItemInList(EditItemIndex(CUR_LENGTH_OF_DENTAL_ARCH), lower_item_list_);
        switchEditItemFeatureMode(EditItemIndex(CUR_LENGTH_OF_DENTAL_ARCH));
		updateBoltonAnterior();
		updateBoltonCompletely();

		switchEditItemFeatureMode(index);
		update();
		emit PSIGNALMANAGER->updateDentalAnalysisDataReportSignal();
	}
}

void DentalAnalysisData::setCrowdingAnalysisMode(bool _six_to_six_mode)
{
	crowding_analysis_6to6_mode_ = _six_to_six_mode;
}

vector<std::pair<QString, bool>> DentalAnalysisData::getEachItemConfirmState()
{
	vector<std::pair<QString, bool>> state_list;
	state_list.push_back(make_pair(ITEM_MISSING_TOOTH, confirm_missing_fdi_));
	state_list.push_back(make_pair(ITEM_TOOTH_WIDTH, confirm_tooth_width_));
	state_list.push_back(make_pair(ITEM_CROWDING, confirm_crowding_));
	state_list.push_back(make_pair(ITEM_BOLTON_RATIO, confirm_bolton_));
	state_list.push_back(make_pair(ITEM_SPEE, confirm_spee_));
	state_list.push_back(make_pair(ITEM_MOLAR_REL_RATIO, confirm_molar_));
	state_list.push_back(make_pair(ITEM_CENTERLINE_REL, confirm_anterior_parameter_));
	state_list.push_back(make_pair(ITEM_ARCH_WIDTH, confirm_arch_width_));
	state_list.push_back(make_pair(ITEM_ARCH_LENGTH, confirm_arch_length_));
	state_list.push_back(make_pair(ITEM_BASAL_BONE, confirm_basal_bone_arch_));
	state_list.push_back(make_pair(ITEM_PALATAL_HEIGHT, confirm_gnathotectum_height_));
	state_list.push_back(make_pair(ITEM_OJ_OB, confirm_overlay_));
	return state_list;
}

void DentalAnalysisData::addBrowsedPrompt(QString _prompt)
{
	if (find(browsed_item_prompt_.begin(), browsed_item_prompt_.end(), _prompt) == browsed_item_prompt_.end())
	{
		browsed_item_prompt_.push_back(_prompt);
	}
	emit PSIGNALMANAGER->updateBrowserItemSignal();
}

void DentalAnalysisData::autoPickonTopInteractiveFrame()
{
	if (frames_.empty())
	{
		return;
	}

	for (auto ite = frames_.rbegin(); ite != frames_.rend(); ++ite)
	{
		if (Interactive2DLabel::TOP_VIEW <= ite->motion_id_ && ite->motion_id_ <= Interactive2DLabel::RIGHT_VIEW)
		{
			ite->focus_ = true;
			emit PSIGNALMANAGER->setViewDirByPhotosMouseSelectedSignal(ite->motion_id_);
            if (cur_mode_ == EditItemIndex(SPEE_CURVE_DEPTH))
			{
				PFusionAlignData->setUpperDentalObjectsVisible(false);
				PFusionAlignData->setLowerDentalObjectsVisible(true);
			}
			break;
		}
	}
}

void DentalAnalysisData::startEdit()
{
	if (this->cur_ope_item_)
	{
		cur_ope_item_->startEdit();
		addBrowsedPrompt(cur_ope_item_->prompt());
	}
	if (cur_lower_item_)
	{
		cur_lower_item_->setCurrentItemVisibleMode(cur_lower_item_->first_enable_item_index_);
	}
	if (cur_upper_item_)
	{
		cur_upper_item_->setCurrentItemVisibleMode(cur_upper_item_->first_enable_item_index_);
	}

	update();
	autoPickonTopInteractiveFrame();
}
