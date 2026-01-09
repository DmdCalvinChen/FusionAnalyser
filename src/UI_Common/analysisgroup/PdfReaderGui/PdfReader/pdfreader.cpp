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

#include "pdfreader.h"

#include <common/config.h>

#include "ui_pdfreader.h"

PdfReader::PdfReader(QWidget* parent) : QWidget(parent), ui(new Ui::PdfReader)
{
    ui->setupUi(this);
    color_background_ = QColor(255, 255, 255);
}

PdfReader::~PdfReader()
{
    delete ui;
}

void PdfReader::updateDrawElements(std::vector<PrimitiveBase*>* elements)
{
    clearPointerVector(draw_elements_);

    if (elements)
    {
        for (auto& it : *elements)
        {
            draw_elements_.push_back(it->clone());
        }
    }

    updateHeightItems();
}

void PdfReader::drawBackground()
{
    QPainter painter(this);
    painter.endNativePainting();
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    // draw background rectangle
    painter.setBrush(QBrush(color_background_, Qt::SolidPattern));
    painter.drawRect(-1, -1, this->width() + 2, this->height() + 2);

    painter.restore();
}

void PdfReader::paintEvent(QPaintEvent* event)
{
    drawBackground();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (auto it : draw_elements_)
    {
        it->draw(painter);
    }
}

void PdfReader::resizeEvent(QResizeEvent* event)
{
}

void PdfReader::updateHeightItems()
{
    int height = this->height();
    if (!draw_elements_.empty())
    {
        height = draw_elements_.back()->getYCoordinateScreenShowPos();
    }

    setFixedSize(800, std::max(height + 200, 800));
    update();
}
