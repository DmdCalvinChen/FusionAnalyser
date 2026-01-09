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

#include "BoltonAna.h"
#include "common/meshmodel.h"
#include "data/fusionaligndata.h"

using namespace common_ext;
using namespace std;

BoltonAna::BoltonAna(QObject *parent)
    : QObject(parent)
{
}

BoltonAna::BoltonAna(MeshModel *upTooth, MeshModel *lowerTooth, QObject *parent /*= nullptr*/)
    :QObject(parent)
{
    assert(upTooth && lowerTooth);
    p_lower_tooth_model_ = lowerTooth;
    p_up_tooth_model_ = upTooth;
    b_model_changed_ = true;
    updateToothDataSlot();
}

BoltonAna::BoltonAna(MeshDocument *md, QObject *parent /*= nullptr*/)
{
    assert(md);
    parseMeshDoc(md);
    p_mesh_doc_ = md;
    assert(p_lower_tooth_model_ && p_up_tooth_model_);
    b_model_changed_ = true;
    updateToothDataSlot();
}

BoltonAna::~BoltonAna()
{
}

void BoltonAna::updateMeshModel(MeshModel *upTooth, MeshModel *lowerTooth)
{
    assert(lowerTooth && upTooth);
    if (p_lower_tooth_model_ != lowerTooth)
    {
        p_lower_tooth_model_ = lowerTooth;
        b_model_changed_ = true;
    }
    if (p_up_tooth_model_ != upTooth)
    {
        p_up_tooth_model_ = upTooth;
        b_model_changed_ = true;
    }
}

void BoltonAna::computeTeethWidth()
{
    if (PFusionAlignData->upperManager())
    {
        for (auto tooth : PFusionAlignData->upperManager()->cDental.teeth)
        {
            if (tooth.strFDI != "")
            {
                upper_tooth_width_data_.insert(make_pair(tooth.strFDI, QString::number(tooth.localBoundbox.fLength, 'f', 2)+"mm"));
            }
        }
        QString fdi;
        for (int i = 1; i <= 8; ++i)
        {
            fdi = "1" + QString::number(i);
            if (upper_tooth_width_data_.find(fdi) == upper_tooth_width_data_.end())
            {
                upper_tooth_width_data_[fdi] = "/";
            }
            fdi = "2" + QString::number(i);
            if (upper_tooth_width_data_.find(fdi) == upper_tooth_width_data_.end())
            {
                upper_tooth_width_data_[fdi] = "/";
            }
        }

        b_up_tooth_exist_ = true;
    }

    if (PFusionAlignData->lowerManager())
    {
        for (auto tooth : PFusionAlignData->lowerManager()->cDental.teeth)
        {
            if (tooth.strFDI != "")
            {
                lower_tooth_width_data_.insert(make_pair(tooth.strFDI, QString::number(tooth.localBoundbox.fLength, 'f', 2)+"mm"));
            }
        }
        QString fdi;
        for (int i = 1; i <= 8; ++i)
        {
            fdi = "3" + QString::number(i);
            if (lower_tooth_width_data_.find(fdi) == lower_tooth_width_data_.end())
            {
                lower_tooth_width_data_[fdi] = "/";
            }
            fdi = "4" + QString::number(i);
            if (lower_tooth_width_data_.find(fdi) == lower_tooth_width_data_.end())
            {
                lower_tooth_width_data_[fdi] = "/";
            }
        }
        b_lower_tooth_exist_ = true;
    }
}

void BoltonAna::computeBolton()
{
    if (b_up_tooth_exist_ && b_lower_tooth_exist_)
    {
        float upTotalToothWidth = .0, lowerTotalToothWidth = .0f;
        float upTotal33ToothWidth = .0, lowerTotal33ToothWidth = .0f;

        auto computeTotalTeethLength = [](DentalManager*model, float&width ,int num)
        {
            for (int i = 0; i < STANDARD_TOOTH_SUM; ++i)
            {
                if (model->cDental.bToothExist[i])
                {
                    int toothNum = model->cDental.teeth[i].strFDI.toInt() % 10;
                    if (toothNum >= 1 && toothNum <= num)
                    {
                        width += model->cDental.teeth[i].localBoundbox.fLength;
                    }
                }
            }
        };

        computeTotalTeethLength(PFusionAlignData->upperManager(), upTotalToothWidth, 6);
        computeTotalTeethLength(PFusionAlignData->lowerManager(), lowerTotalToothWidth, 6);
        computeTotalTeethLength(PFusionAlignData->upperManager(), upTotal33ToothWidth, 3);
        computeTotalTeethLength(PFusionAlignData->lowerManager(), lowerTotal33ToothWidth, 3);

        if (fabs(upTotal33ToothWidth) > 0.001f)
        {
            float standard_bolton = 0.788f;
            f_bolton_33_ = lowerTotal33ToothWidth / upTotal33ToothWidth;
            s_bolton_33_ = QString::number(f_bolton_33_ * 100, 'f', 1) +"%";
            qDebug() << s_bolton_33_;
            if (f_bolton_33_ > standard_bolton)
            {
                f_discrepancy_33_ = lowerTotal33ToothWidth - upTotal33ToothWidth * standard_bolton;
                discrepancy_33_info_ = tr("mandibular excess");
            }
            else
            {
                f_discrepancy_33_ = upTotal33ToothWidth - lowerTotal33ToothWidth / standard_bolton;
                discrepancy_33_info_ = tr("maxillary excess");
            }
            f_discrepancy_33_ = fabs(f_discrepancy_33_);
            discrepancy_33_num_ = QString::number(f_discrepancy_33_, 'f', 2) + "mm";
        }

        if (fabs(upTotalToothWidth) > 0.001f)
        {
            float standard_bolton = 0.915f;
            f_bolton_66_ = lowerTotalToothWidth / upTotalToothWidth;
            s_bolton_66_ = QString::number(f_bolton_66_ * 100, 'f', 1) +"%";
            qDebug() << s_bolton_66_;
            if (f_bolton_66_ > standard_bolton)
            {
                f_discrepancy_66_ = lowerTotalToothWidth - upTotalToothWidth * standard_bolton;
                discrepancy_66_info_ = tr("mandibular excess");
            }
            else
            {
                f_discrepancy_66_ = upTotalToothWidth - lowerTotalToothWidth / standard_bolton;
                discrepancy_66_info_ = tr("maxillary excess");
            }
            f_discrepancy_66_ = fabs(f_discrepancy_66_);
            discrepancy_66_num_ = QString::number(f_discrepancy_66_, 'f', 2) + "mm";
        }
    }
}

void BoltonAna::updateToothDataSlot()
{
    if (b_model_changed_)
    {
        computeTeethWidth();
        computeBolton();
        b_model_changed_ = false;
    }
}

void BoltonAna::updateMeshDoc(MeshDocument *md)
{
    if (p_mesh_doc_ != md)
    {
        p_mesh_doc_ = md;
        parseMeshDoc(md);
        b_model_changed_ = true;
    }
    else
        b_model_changed_ = false;
}

void BoltonAna::parseMeshDoc(MeshDocument *md)
{
    for (auto meshmodel : md->meshList)
    {
        if (meshmodel->upperOrLowerToothModelMark == UpperToothModel)
        {
            p_up_tooth_model_ = meshmodel;
        }
        else if (meshmodel->upperOrLowerToothModelMark == LowerToothModel)
        {
            p_lower_tooth_model_ = meshmodel;
        }
    }
    updateToothDataSlot();
}

