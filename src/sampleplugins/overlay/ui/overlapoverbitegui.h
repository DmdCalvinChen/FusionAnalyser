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

#ifndef OVERLAPOVERBITE_H
#define OVERLAPOVERBITE_H

#include <qdialog.h>
//#include <QDockWidget>
#include <common_ext/guiAttributeCustom/baseattributedefaultgui.h>
#include "overlayoutlinepreview.h"

namespace Ui {
class OverlapOverbiteGui;
}

class OverlapOverbiteGui : public QDialog, public BaseAttributeDefaultGui
{
    Q_OBJECT

public:
    explicit OverlapOverbiteGui(QWidget *parent = 0);
    ~OverlapOverbiteGui();

signals:
	void doneSignal();
	void setStepOverlaySendSignal(float step);
	void updateCutOutlineSignal();

public slots:
	void wellDoneSlot();
	void initControlDefaultValueSlot();
	void updateStepOverlaySliderLabelSlot(int step);
	void switchToRightSide();
	void switchToLeftSide();

protected:
    void resizeEvent(QResizeEvent *_event);
    void showEvent(QShowEvent *event);

public:
	// overlay preview gui
	OverlayOutlinePreview* overlayOutlinePreview = nullptr;

private:
    Ui::OverlapOverbiteGui *ui;
};

#endif // OVERLAPOVERBITE_H
