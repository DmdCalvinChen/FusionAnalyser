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

#include "fusionTitleBar.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QStyle>

#include "common/config.h"
#include "common_ext/data/fusionaligndata.h"

#define ICONMINHEIGHT 28
#define TITLEBARHEIGHT 30

#ifdef Q_OS_WIN
#    include <qt_windows.h>
#    include <windowsx.h>
#    pragma comment(lib, "user32.lib")
#endif

using namespace ui_common;

FusionTitleBar::FusionTitleBar(QWidget* parent) : QWidget(parent)
{
    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    icon_label_ = new QLabel(this);

    menu_btn_ = new QPushButton(this);
    createMenu();
    menu_btn_->setMenu(main_menu_);
    menu_btn_->setObjectName("mainMenuBtn");
    title_label_ = new QLabel(this);
    title_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setObjectName("fusionTitleBar");

    patient_name_label_ = new QLabel(this);
    user_name_ = QStringLiteral("...");

    schedule_label_ = new QLabel(this);
    schedule_label_->setText("");

    QString btnStyle("border:none");
    minimize_btn_ = new QPushButton(this);
    minimize_btn_->setObjectName("minimizeButton");

    maximize_btn_ = new QPushButton(this);
    maximize_btn_->setObjectName("maximizeButton");

    close_btn_ = new QPushButton(this);
    close_btn_->setObjectName("closeButton");

    layout_ = new QHBoxLayout;
    layout_->addWidget(menu_btn_);
    layout_->addWidget(patient_name_label_);
    layout_->addStretch(4);
    title_label_->setGeometry(this->width() / 2 - title_label_->width() / 2,
                              this->height() / 2 - title_label_->height() / 2, 120, 90);

    layout_->addStretch(5);

    p_patient_name_schedule_frame_ = new QFrame(this);
    p_patient_name_schedule_frame_->setFixedWidth(3);
    layout_->addWidget(p_patient_name_schedule_frame_);
    layout_->addWidget(schedule_label_);
    layout_->addSpacing(10);
    layout_->addWidget(minimize_btn_);
    layout_->addWidget(maximize_btn_);
    layout_->addWidget(close_btn_);
    layout_->setContentsMargins(2, 0, 2, 0);
    setLayout(layout_);

    setProperty("titleBar", true);

    connect(minimize_btn_, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
    connect(maximize_btn_, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
    connect(close_btn_, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
    main_window_ = qobject_cast<QMainWindow*>(window());
    setSchemeVisible(false);

#ifdef Q_OS_MAC
    QString font_style("background-color:rgb(255,255,255);color:rgb(0,0,0);");
    font_style.append("font-family:" + FONT_FAMILY_CURRENT_PLATFORM + ";");
    this->setStyleSheet(font_style);
#endif
}

void FusionTitleBar::setWindowTitle(const QString& title)
{
    title_label_->setAlignment(Qt::AlignVertical_Mask);

    QImage Image;
    Image.load(":/res/default/images/titleBar/title.png");
    QIcon icon(title);
    title_label_->setPixmap(icon.pixmap(title_label_->size(), QIcon::Normal));
}

void FusionTitleBar::setWindowTitle(const QImage& image)
{
    title_label_->setAlignment(Qt::AlignVertical_Mask);
    QPixmap pixmap = QPixmap::fromImage(image);
    QPixmap fitpixmap = pixmap.scaled(title_label_->width(), title_label_->height(),
                                      Qt::KeepAspectRatio, Qt::SmoothTransformation);
    title_label_->setPixmap(fitpixmap);
    title_label_->setContentsMargins(0, 0, 0, 0);
}

void FusionTitleBar::setPatientName(const QString& patientName)
{
    QString string_to_short = patientName;
    QFontMetrics fontMetrics(patient_name_label_->font());
    // If the string length is greater than the label width under the current font
    if (fontMetrics.width(string_to_short) > patient_name_label_->width())
    {
        string_to_short = QFontMetrics(patient_name_label_->font())
                                  .elidedText(string_to_short, Qt::ElideRight, 300);
        patient_name_label_->setToolTip(patientName);
    }
    patient_name_label_->setText(string_to_short);
    if (PFusionAlignData->getAnalyserData().getIsBmu())
    {
        patient_name_label_->setStyleSheet("color: #717478; font: 16px Microsoft YaHei;");
    }
    else
    {
        patient_name_label_->setStyleSheet("color: #ffffff; font: 16px Microsoft YaHei;");
    }

    s_patient_name_ = patientName;
}

void FusionTitleBar::setScheduleName(const QString& scheduleName)
{
    schedule_label_->setText(scheduleName);
    schedule_label_->setStyleSheet("color: #717478; font: 16px Microsoft YaHei;");
    s_schedule_name_ = scheduleName;
}

void FusionTitleBar::setTitleBarIcon(const QString& iconPath)
{
    QPixmap map(iconPath);
    icon_label_->setPixmap(map);
}

void FusionTitleBar::windowMaximized()
{
    QRect desktop = QApplication::desktop()->availableGeometry(
            QApplication::desktop()->screenNumber(QCursor::pos()));
    main_window_->setWindowState(
            (main_window_->windowState() & ~(Qt::WindowMinimized | Qt::WindowFullScreen)) |
            Qt::WindowMaximized);
    main_window_->setGeometry(desktop);
}

bool FusionTitleBar::isCaption(int x, int y) const
{
    if (this->rect().contains(x, y))
    {
        if (!this->childAt(x, y))
        {
            return true;
        }
    }
    return false;
}

void FusionTitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        maximize_btn_->click();
    }
}

void FusionTitleBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        b_left_btn_pressed_ = true;
        start_pos_ = event->globalPos();
    }

    QWidget::mousePressEvent(event);
}

void FusionTitleBar::mouseMoveEvent(QMouseEvent* event)
{
    if (b_left_btn_pressed_)
    {
        QDesktopWidget* deskTop = QApplication::desktop();
        int curMonitor = deskTop->screenNumber(QCursor::pos());
        QRect desktop = QApplication::desktop()->availableGeometry(curMonitor);
        bool max = main_window_->isMaximized();
        int cursor = event->globalY();
        if (main_window_->isMaximized() && event->globalY() - desktop.y() >= 5)
        {
            float scale = (float)(event->globalX() - main_window_->mapToGlobal(QPoint(0, 0)).x()) /
                          this->width();
            main_window_->showNormal();
            QRect normalGeo = main_window_->normalGeometry();
            main_window_->setGeometry(event->globalX() - 1080 * scale, event->globalY(), 1280, 720);
        }
        else if (event->globalY() <= desktop.y() && !main_window_->isMaximized())
        {
            main_window_->showMaximized();
        }
        else if (!main_window_->isMaximized())
        {
            QPoint pos = event->globalPos() - start_pos_;
            QPoint targetPos = main_window_->geometry().topLeft() + pos;
            main_window_->setGeometry(QRect(targetPos.x(), targetPos.y(), main_window_->width(),
                                            main_window_->height()));
            start_pos_ = event->globalPos();
        }
    }
}

void FusionTitleBar::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        b_left_btn_pressed_ = false;
    }
}

bool FusionTitleBar::eventFilter(QObject* obj, QEvent* event)
{
    switch (event->type())
    {
        // Set title
        case QEvent::WindowTitleChange:
        {
            QWidget* pWidget = qobject_cast<QWidget*>(obj);
            if (pWidget)
            {
                return true;
            }
        }
        case QEvent::WindowStateChange:
        case QEvent::Resize:
            updateMaximize();
            return true;
    }
    return QWidget::eventFilter(obj, event);
}

void FusionTitleBar::updateMaximize()
{
    QWidget* pWindow = this->window();
    Qt::WindowStates states = pWindow->windowState();
    if (states & Qt::WindowMaximized)
    {
        this->showNormal();
        maximize_btn_->setIcon(QIcon(QPixmap(QString(":/%1/res/%1/images/titleBar/normal.svg")
                                                     .arg(PFusionAppData->getSkinName()))));
    }
    else
    {
        this->showMaximized();
        maximize_btn_->setIcon(QIcon(QPixmap(QString(":/%1/res/%1/images/titleBar/max.svg")
                                                     .arg(PFusionAppData->getSkinName()))));
    }
    return;
}

void FusionTitleBar::createMenu()
{
    main_menu_ = new QMenu(this);

#ifdef Q_OS_MAC
    QString font_style(
            "QMenu::item:selected {background-color:rgb(211,255,255);} QMenu{ color:rgb(0,0,0);");
    font_style.append("font-family:" + FONT_FAMILY_CURRENT_PLATFORM + ";");
    font_style.append("}");
    this->setStyleSheet(font_style);
#endif
}

void FusionTitleBar::onClicked()
{
    QPushButton* pButton = qobject_cast<QPushButton*>(sender());
    QWidget* pWindow = this->window();
    QDesktopWidget* deskTop = QApplication::desktop();
    int curMonitor = deskTop->screenNumber(QCursor::pos());
    QRect desktop = QApplication::desktop()->availableGeometry(curMonitor);
    if (pWindow->isTopLevel())
    {
        if (pButton == minimize_btn_)
        {
            pWindow->showMinimized();
        }
        else if (pButton == maximize_btn_)
        {
            if (pWindow->isMaximized())
            {
                main_window_->showNormal();
                QDesktopWidget* desktopWidget = QApplication::desktop();    // 获取可用桌面大小
                QRect deskRect = desktopWidget->availableGeometry();    // 获取设备屏幕大小
                QRect screenRect = desktopWidget->screenGeometry();
#ifdef Q_OS_WIN
                HDC desk = GetDC(NULL);
                float hDPI = GetDeviceCaps(desk, LOGPIXELSX);
                float vDPI = GetDeviceCaps(desk, LOGPIXELSY);
                int dpi = (hDPI + vDPI) / 2;
                if (dpi > 170 || deskRect.height() < 1000)
                {
                    main_window_->setGeometry(QRect(desktop.center() - QPoint(640, 340),
                                                    desktop.center() + QPoint(640, 360)));
                }
                else if ((deskRect.height() >= 1100 && dpi < 180 && dpi >= 150) ||
                         (deskRect.height() < 1100 && deskRect.height() >= 1000 && dpi >= 140 &&
                          dpi < 180))
                {
                    main_window_->setGeometry(QRect(desktop.center() - QPoint(900, 500),
                                                    desktop.center() + QPoint(900, 500)));
                }
                else
                {
                    main_window_->setGeometry(QRect(desktop.center() - QPoint(750, 450),
                                                    desktop.center() + QPoint(750, 450)));
                }
#else
                // On macOS, use screen height to determine window size
                if (deskRect.height() < 1000)
                {
                    main_window_->setGeometry(QRect(desktop.center() - QPoint(640, 340),
                                                    desktop.center() + QPoint(640, 360)));
                }
                else if (deskRect.height() >= 1100)
                {
                    main_window_->setGeometry(QRect(desktop.center() - QPoint(900, 500),
                                                    desktop.center() + QPoint(900, 500)));
                }
                else
                {
                    main_window_->setGeometry(QRect(desktop.center() - QPoint(750, 450),
                                                    desktop.center() + QPoint(750, 450)));
                }
#endif
            }
            else
            {
                pWindow->showMaximized();
            }
            PSIGNALMANAGER->updatePluginUISignal();
        }
        else if (pButton == close_btn_)
        {
            pWindow->close();
        }
    }
}

void ui_common::FusionTitleBar::addCategoryToolBar(QToolBar* pToolBar)
{
    layout_->insertWidget(1, (QWidget*)pToolBar);
}

void ui_common::FusionTitleBar::setSchemeVisible(bool visible /*= true*/)
{
    if (visible)
    {
        patient_name_label_->setText(s_patient_name_);
        schedule_label_->setText(s_schedule_name_);
    }
    else
    {
        patient_name_label_->setText("");
        schedule_label_->setText("");
    }
}

void ui_common::FusionTitleBar::resizeEvent(QResizeEvent* event)
{
    title_label_->setGeometry(this->width() / 2 - title_label_->width() / 2,
                              this->height() / 2 - title_label_->height() / 2,
                              title_label_->width(), title_label_->height());

    return QWidget::resizeEvent(event);
}
