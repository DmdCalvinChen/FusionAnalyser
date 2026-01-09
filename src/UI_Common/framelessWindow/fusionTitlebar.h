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

#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPixmap>
#include <QPoint>
#include <QPushButton>
#include <QString>
#include <QWidget>

#include "uicommon_global.h"
class QMainWindow;
class QToolBar;
namespace ui_common
{

class UI_COMMONSHARED_EXPORT FusionTitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit FusionTitleBar(QWidget* parent = nullptr);

    void setWindowTitle(const QString& title);
    void setWindowTitle(const QImage& image);
    void setPatientName(const QString& patientName);
    void setScheduleName(const QString& scheduleName);

    void addCategoryToolBar(QToolBar* pToolBar);

    void setTitleBarIcon(const QString& iconPath);
    void windowMaximized();
    bool isCaption(int x, int y) const;

    QMenu* getMainMenu() const
    {
        return main_menu_;
    }
    QMenu* getFileMenu() const
    {
        return file_menu_;
    }
    QMenu* getHelpMenu() const
    {
        return help_menu_;
    }
    QPushButton* getMainMenuBtn() const
    {
        return menu_btn_;
    }
    QPushButton* getMaximizeBtn() const
    {
        return maximize_btn_;
    }
    QPushButton* getMinimizeBtn() const
    {
        return minimize_btn_;
    }
    QPushButton* getCloseBtn() const
    {
        return close_btn_;
    }
    void setSchemeVisible(bool visible = true);

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

    virtual void resizeEvent(QResizeEvent* event);

    virtual bool eventFilter(QObject* watched, QEvent* event);

    void updateMaximize();

    void createMenu();

protected slots:
    void onClicked();

private:
    QLabel* icon_label_ = nullptr;
    QLabel* title_label_ = nullptr;
    QLabel* patient_name_label_ = nullptr;    // 患者姓名
    QLabel* patient_name_icon = nullptr;
    QLabel* schedule_label_ = nullptr;    // 治疗方案标签
    QFrame* p_patient_name_schedule_frame_ = nullptr;

    // buttons
    QPushButton* menu_btn_ = nullptr;
    QPushButton* minimize_btn_ = nullptr;
    QPushButton* maximize_btn_ = nullptr;
    QPushButton* close_btn_ = nullptr;

    // menu
    QMenu* main_menu_ = nullptr;
    QMenu* file_menu_ = nullptr;
    QMenu* help_menu_ = nullptr;

    // actions
    QAction* exit_act_ = nullptr;

    QPoint start_pos_;
    QPoint end_pos_;
    bool b_left_btn_pressed_ = false;
    QMainWindow* main_window_ = nullptr;
    bool b_maximized = false;

    QString user_name_ = "";
    QString s_patient_name_ = "";
    QString s_schedule_name_ = "";

    QHBoxLayout* layout_ = nullptr;

signals:

public slots:
};
}    // namespace ui_common
#endif    // TITLEBAR_H
