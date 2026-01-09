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

#ifndef FUSIONFRAMELESSBASEDLG_H
#define FUSIONFRAMELESSBASEDLG_H

#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "uicommon_global.h"
class UI_COMMONSHARED_EXPORT FusionFramelessBaseDlg : public QWidget
{
    Q_OBJECT
public:
    FusionFramelessBaseDlg(QWidget* parent = nullptr);
    virtual ~FusionFramelessBaseDlg();

protected:
    bool isCaption(int x, int y);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void createCloseBtn(QWidget* widget, int width);

    QPushButton* p_close_btn_ = nullptr;
    void createTilte(QLabel* pTitleLabel, int width);

private:
    QPoint start_pos_;
    QPoint end_pos_;
    bool b_left_btn_pressed_ = false;
    bool b_is_catpion_ = false;
};

#endif    // FUSIONFRAMELESSBASEDLG_H
