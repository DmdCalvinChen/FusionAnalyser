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

#pragma once

#include <QObject>
#include <vector>

#include "vcg/space/point3.h"
class MeshDocument;
class MeshModel;

namespace fusionAlign
{
    // 后续规范命名
    const vcg::Point3f BACKGROUNDCOLOR_1(125.f / 255.0f, 205.f / 255.f, 245.f / 255.f);
    const vcg::Point3f BACKGROUNDCOLOR_2(100.f / 255.0f, 205.f / 255.f, 155.f / 255.f);
    const vcg::Point3f BACKGROUNDCOLOR_3(1.0, 1.0f, 155.f / 255.f);
    const vcg::Point3f BACKGROUNDCOLOR_4(1.f, 105.f / 255.f, 95.f / 255.f);

    struct SEnamelReductionData
    {
    public:
        float enamal_crevice_  = 0;
        vcg::Point3f pos_;
        vcg::Point3f dir_;
        QString      s_tag_;
        vcg::Point3f background_color_;
        bool         b_visible_ = false;

        SEnamelReductionData()
        {
            pos_.SetZero();
            dir_.SetZero();
            background_color_.SetZero();
        }

        SEnamelReductionData(float data, vcg::Point3f pos, vcg::Point3f dir, QString tag, vcg::Point3f color, bool visible)
        {
            enamal_crevice_ = data;
            pos_ = pos;
            dir_ = dir;
            s_tag_ = tag;
            background_color_ = color;
            b_visible_ = visible;
        }
    };
    class EnamelReductAna : public QObject
    {
        Q_OBJECT

    public:
        EnamelReductAna(QObject *parent = nullptr);
        EnamelReductAna(MeshDocument *md, QObject *parent = nullptr);

        void updateEnamelReductData(MeshDocument *md);
        ~EnamelReductAna();

    private:
        void computeMeshModelReductionData(MeshModel* pMesh, std::vector<SEnamelReductionData>& enamalData);

    private:
        std::vector<SEnamelReductionData> v_enamal_data_L_;
        std::vector<SEnamelReductionData> v_enamal_data_U_;
    };

}
