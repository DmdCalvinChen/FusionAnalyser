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

#include "EnamelReductAna.h"

#include "meshmodel.h"
#include "utility_tools.h"
using namespace  fusionAlign;
using namespace vcg;

EnamelReductAna::EnamelReductAna(QObject *parent)
    : QObject(parent)
{
    v_enamal_data_L_.clear();
    v_enamal_data_U_.clear();
}

EnamelReductAna::~EnamelReductAna()
{
}

void EnamelReductAna::updateEnamelReductData(MeshDocument *md)
{
    v_enamal_data_L_.clear();
    v_enamal_data_U_.clear();
    for (auto mesh : md->meshList)
    {
        if (mesh->upperOrLowerToothModelMark == UpperToothModel)
        {
            computeMeshModelReductionData(mesh, v_enamal_data_U_);
        }
        else if (mesh->upperOrLowerToothModelMark == LowerToothModel)
        {
            computeMeshModelReductionData(mesh, v_enamal_data_L_);
        }
    }
}

void EnamelReductAna::computeMeshModelReductionData(MeshModel* pMesh, std::vector<SEnamelReductionData>& enamalData)
{
    assert(pMesh);
    pMesh->cDental.computeEachToothCreviceEdges(); //是否需要计算？

    int tag = 0;
    float length = 0.0f;
    Point3f pos, linkPos, dir, backgoudColor = BACKGROUNDCOLOR_1;
    CustomPlane &basePlane = pMesh->cDental.basePlane;
    bool bVisible = true;
    for (auto edge : pMesh->cDental.toothCreviceEdges)
    {
        pos = (edge.vertA + edge.vertB) / 2.0f;
        linkPos = UtilToolInstance->getProjPointOnPlane(pos, basePlane.axisZV, basePlane.center, -basePlane.axisZV);
        dir = (linkPos - basePlane.center).normalized() * 0.5 + basePlane.axisZV*0.5;

        length = edge.fLength;

        if (length < 0.05f)
        {
            bVisible = false;
        }
        else if (length >= 0.05f && length <= 0.15f)
        {
            backgoudColor = BACKGROUNDCOLOR_2;
        }
        else if (length > 0.15f && length < 0.5f)
        {
            backgoudColor = BACKGROUNDCOLOR_3;
        }
        else if (length > 0.5f)
        {
            backgoudColor = BACKGROUNDCOLOR_4;
        }
        //data = fabs(data);

        SEnamelReductionData data(length, pos, dir, QString::number(tag), backgoudColor, bVisible);
        enamalData.push_back(data);
    }
}
