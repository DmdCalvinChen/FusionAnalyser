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

#ifndef BOLTONANA_H_
#define BOLTONANA_H_

#include <QObject>
#include <QString>
#include "common_ext_global.h"

class MeshModel;
class MeshDocument;
namespace common_ext
{
    class COMMON_EXT_EXPORT BoltonAna : public QObject
    {
        Q_OBJECT

    public:
        BoltonAna(QObject *parent = nullptr);
        BoltonAna(MeshModel *upTooth, MeshModel *lowerTooth, QObject *parent = nullptr);
        BoltonAna(MeshDocument *md, QObject *parent = nullptr);

        ~BoltonAna();

        void updateMeshModel(MeshModel *upTooth, MeshModel *lowerTooth);
        void updateMeshDoc(MeshDocument *md);

        QString getDiscrepancy_33_info() const  { return discrepancy_33_info_; }
        QString getDiscrepancy_66_info() const  { return discrepancy_66_info_; }
        QString getBolton_33() const            { return s_bolton_33_; }
        QString getBolton_66() const            { return s_bolton_66_; }
        QString getDiscrepancy_33_num() const   { return discrepancy_33_num_; }
        QString getDiscrepancy_66_num() const   { return discrepancy_66_num_; }

        std::map<QString, QString> getUpper_tooth_width_data() const { return upper_tooth_width_data_; }
        std::map<QString, QString> getLower_tooth_width_data() const { return lower_tooth_width_data_; }
    signals:
    public slots:
        void updateToothDataSlot();

    private:
        void computeTeethWidth();
        void computeBolton();
        void parseMeshDoc(MeshDocument *md);

    private:
        std::map<QString, QString> upper_tooth_width_data_;
        std::map<QString, QString> lower_tooth_width_data_;

        bool b_up_tooth_exist_ = false;
        bool b_lower_tooth_exist_ = false;

        MeshModel *p_up_tooth_model_ = nullptr;
        MeshModel *p_lower_tooth_model_ = nullptr;
        MeshDocument *p_mesh_doc_ = nullptr;

        float f_bolton_33_ = 0.0f;
        float f_bolton_66_ = 0.0f;
        float f_discrepancy_33_ = 0.0f;
        float f_discrepancy_66_ = 0.0f;

        QString s_bolton_33_;
        QString s_bolton_66_;
        QString discrepancy_33_info_;
        QString discrepancy_66_info_;
        QString discrepancy_33_num_;
        QString discrepancy_66_num_;

        bool b_model_changed_ = true;
    };

}

#endif
