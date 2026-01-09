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

#ifndef GifViewLabel_H
#define GifViewLabel_H

#include <QBoxLayout>
#include <QButtonGroup>
#include <QLabel>
#include <QObject>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QWidget>

#include "uicommon_global.h"

class UI_COMMONSHARED_EXPORT GifViewLabel : public QLabel
{
    Q_OBJECT
public:
    GifViewLabel(QWidget* parent = nullptr);
    ~GifViewLabel();

    void addImage(const QList<std::pair<QString, QString>>& imagepathpairlst);
    std::map<int, QMovie*> mov;
    QList<std::pair<QString, QString>> m_imagepathpairlst2;

protected:
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* event);

private:
    int m_offset = 0, m_curIndex = 0, m_preIndex = 0;
    bool m_blefttoright = false;
    QMovie* movie2 = nullptr;
    QMovie* movie1 = nullptr;
    QButtonGroup* m_btnGroup = nullptr;
    QParallelAnimationGroup* m_btnParalGroup = nullptr;
    QPropertyAnimation *m_imageAnimation = nullptr, *m_btnExpAnimation = nullptr,
                       *m_btnShrikAnimation = nullptr;
    QWidget* m_horizontalLayoutWidget = nullptr;
    QHBoxLayout* m_switchBtnLayout = nullptr;
    QTimer* m_imageTimer = nullptr;
    QList<std::pair<QString, QString>> m_imagepathpairlst;

private:
    void initControl();
public slots:
    void setGifSlot(bool is_marking_status);
private slots:
    void onImagevalueChanged(const QVariant& variant);
    void onBtnExpvalueChanged(const QVariant& variant);
    void onBtnShrikvalueChanged(const QVariant& variant);
    void onbuttonClicked(int index);
    void onImageShowTimeOut();
    void onStopMovieSlot(int frameNumber);
};

#endif    // GifViewLabel_H
