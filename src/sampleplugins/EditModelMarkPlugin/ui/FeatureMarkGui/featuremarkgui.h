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

#ifndef FEATUREMARKGUI_H
#define FEATUREMARKGUI_H

#include <QWidget>
#include <UI_Common/gifViewLabel/GifViewLabel.h>
QT_BEGIN_NAMESPACE
namespace Ui { class FeatureMarkGui; }
QT_END_NAMESPACE

namespace ui_common
{
    class ToothFdiGuideGui;
}
// Feature marking point interface

class FeatureMarkGui : public QWidget
{
    Q_OBJECT

public:
    FeatureMarkGui(QWidget *parent = nullptr);
    ~FeatureMarkGui();

    // Tooth position navigation map
    ui_common::ToothFdiGuideGui* tooth_fdi_preview_ui_ = nullptr;
    GifViewLabel* gif_view_label_ui = nullptr;
signals:
    void doneSignal();
public slots:
    void setTextSlot(bool is_marking_status);
private:
    bool is_upper = true;
    Ui::FeatureMarkGui *ui;
};
#endif // FEATUREMARKGUI_H
