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

#ifndef FUSIONALIGNTOOLBAR_H
#define FUSIONALIGNTOOLBAR_H

#include <QToolBar>
#include <QMenu>
#include <QToolButton>
#include <QAction>

class FusionAlignToolBar : public QToolBar
{
    Q_OBJECT
public:
    FusionAlignToolBar(QWidget *parent = nullptr);
    void createActAndBtn();
    QToolBar *getRedoUndoToolBar() const { return p_redo_undo_bar_; }
    QToolBar *getProjToolBar() const { return p_proj_bar_; }
    QToolBar *getDecorateToolBar() const { return p_decorate_bar_; }
    QToolBar* getAnalysisToolBar() const { return p_analysis_bar; }
    QToolBar* getEditDecorateToolBar() const { return p_edit_decorate_bar_; }
    void initLayout();
	void setToolBarEnabled(bool enable);

    QToolButton* getSegmentBtn() const { return p_segment_btn_; }
    QMenu* getStageMenu() const { return p_stage_menu_; }
signals:
	void openHistoryProjsSignal();
	void saveSubmitProjSignal();
	void defySegmentSignal();

private:

    QToolBar *p_proj_bar_ = nullptr;
    QToolBar *p_redo_undo_bar_ = nullptr;
    QToolBar *p_decorate_bar_ = nullptr;
    QToolBar* p_analysis_bar = nullptr;
    QToolBar* p_edit_decorate_bar_ = nullptr;

	QToolButton *p_segment_btn_ = nullptr;

	QAction *p_defy_split_act_ = nullptr;

    QMenu* p_stage_menu_ = nullptr;
};

#endif // FUSIONALIGNTOOLBAR_H
