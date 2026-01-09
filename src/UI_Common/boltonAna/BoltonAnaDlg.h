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

#ifndef BOLTON_ANA_DLG_H_
#define BOLTON_ANA_DLG_H_

#include <QWidget>

#include "uicommon_global.h"

namespace Ui
{
class BoltonAnaDlg;
};

namespace common_ext
{
class BoltonAna;
}

namespace ui_common
{

class UI_COMMONSHARED_EXPORT BoltonAnaDlg : public QWidget
{
    Q_OBJECT

public:
    BoltonAnaDlg(QWidget* parent = Q_NULLPTR);
    BoltonAnaDlg(common_ext::BoltonAna* pBoltonAna, QWidget* parent = Q_NULLPTR);

    ~BoltonAnaDlg();
    void paintEvent(QPaintEvent* event);

signals:
    void closedSignal();
public slots:
    void updateUI(common_ext::BoltonAna* pBoltonAna);

private:
    void updateBoltonRatio();
    void updateToothWidth();
    void updateUpperToothWidth();
    void updateLowerToothWidth();
    void setupTableWidget();

protected:
    void closeEvent(QCloseEvent* event);

private:
    Ui::BoltonAnaDlg* ui;

    common_ext::BoltonAna* p_bolton_ana_ = nullptr;
    // QString     s_bolton_33_;
    // QString     s_bolton_66_;
};
}    // namespace ui_common

#endif
