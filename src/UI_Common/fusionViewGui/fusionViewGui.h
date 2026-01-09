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

#ifndef FUSIONVIEWGUI_H
#define FUSIONVIEWGUI_H

#include <QDialog>

#include "uicommon_global.h"

namespace Ui
{
class fusionViewGui;
}

class QSignalMapper;
namespace ui_common
{
enum eViewDirection
{
    E_DEFAULT_VIEW,
    E_TOP_VIEW,
    E_BOTTOM_VIEW,
    E_FRONT_VIEW,
    E_BACK_VIEW,
    E_LEFT_VIEW,
    E_RIGHT_VIEW,
    E_UP_DENTAL_VIEW,
    E_DOWN_DENTAL_VIEW,
};
enum eSelectDentalType
{
    E_SELECT_UP_DENTAL,
    E_SELECT_DOWN_DENTAL
};

class UI_COMMONSHARED_EXPORT fusionViewGui : public QDialog
{
    Q_OBJECT

public:
    explicit fusionViewGui(QWidget* pgla, QWidget* parent = nullptr);
    ~fusionViewGui();
    QPushButton* getUpDentalBtn() const;
    QPushButton* getDownDentalBtn() const;
    void setDentalBtnState(eViewDirection viewDirection);
    void setDentalBtnState(bool isUp, bool isCheck);

signals:
    void setViewDirSignal(int viewDir);
    void setSelectDentalSignal(int dentalType);

    // public slots:
    //     void doClicked(int dir);
public slots:
    void onSelectDentalBtnClickedSlot(int dentalType);
    void setViewDirectConsoleVisible(bool _visible);
    void setSelectDentalConsoleVisible(bool _visible);
    void setDentalVisibleConsoleVisible(bool _visible);
    void setLMRViewDirectConsoleVisible(bool);
    void setUpperDentalSelectedSlot();
    void setLowerDentalSelectedSlot();
    void onlyShowUpperDentalSlot(bool _sure);
    void onlyShowLowerDentalSlot(bool _sure);

protected:
    void initBtn();
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* event);

private:
    Ui::fusionViewGui* ui;
    QSignalMapper* p_signal_mapper_ = nullptr;
    QWidget* p_gla_ = nullptr;

    bool b_up_dental_check_state_ = false;
    bool b_down_dental_check_state_ = false;
    ;

    int width_ = 0;
    int height_ = 0;
};

}    // namespace ui_common

#endif    // FUSIONVIEWGUI_H
