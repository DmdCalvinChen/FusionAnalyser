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

#include "anomalybutton.h"

#include <QBitmap>

AnomalyButton::AnomalyButton(const QString& _label, const QSize& _size,
                             const QString& _general_image_file_path,
                             const QString& _working_image_file_path,
                             QWidget* parent /* = nullptr */)
    : QPushButton(parent)
{
    setLabel(_label);
    setSize(_size);

    this->general_image_file_path_ = _general_image_file_path;
    this->working_image_file_path_ = _working_image_file_path;

    setStatus(General);
}

void AnomalyButton::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::MouseButton::LeftButton)
    {
        is_left_mouse_pressed_ = true;
    }
    else
    {
        is_left_mouse_pressed_ = false;
    }

    emit pressedSignal();
}

void AnomalyButton::setLabel(const QString& label)
{
    this->setObjectName(label);
}

void AnomalyButton::setImage(const QString& image_file_path)
{
    if (!image_file_path.isEmpty())
    {
        QPixmap pixmap(image_file_path);
        pixmap = pixmap.scaled(size_control_, Qt::KeepAspectRatio,
                               Qt::TransformationMode::SmoothTransformation);

        this->clearMask();
        this->setMask(pixmap.mask());
        this->setFixedSize(pixmap.size());
        this->setStyleSheet("QPushButton{border-image:url(" + image_file_path +
                            ");} \
							 QToolTip{background-color:rgba(255,255,255, 255);}");
    }
    else
    {
        this->setStyleSheet("QPushButton{background-color:rgba(255,255,255,40);}");
    }
}

void AnomalyButton::setStatus(Status _status)
{
    this->status_ = _status;

    updateGuiWhenStatusChanging();
}

void AnomalyButton::setSize(const QSize& size)
{
    this->size_control_ = size;
}

void AnomalyButton::updateGuiWhenStatusChanging()
{
    switch (status_)
    {
        case General:
            setImage(general_image_file_path_);
            break;
        case Lost:
            setImage(lost_image_file_path_);
            break;
        case Working:
            setImage(working_image_file_path_);
            break;
        default:
            break;
    }
}
