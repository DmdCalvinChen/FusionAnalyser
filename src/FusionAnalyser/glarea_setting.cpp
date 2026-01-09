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

#include "../common/filterparameter.h"
#include "glarea_setting.h"

void GLAreaSetting::initGlobalParameterSet( RichParameterSet * defaultGlobalParamSet)
{
    defaultGlobalParamSet->addParam(new RichColor(backgroundBotColorParam(),QColor(250,250,250),"Fusion Bottom BackGround Color","Fusion GLarea's BackGround Color(bottom corner)"));
    defaultGlobalParamSet->addParam(new RichColor(backgroundTopColorParam(),QColor(200,200,200),"Fusion Top BackGround Color","Fusion GLarea's BackGround Color(top corner)"));
    defaultGlobalParamSet->addParam(new RichColor(logAreaColorParam(),   QColor(128,128,128),"Fusion GLarea's Log Area Color","Fusion GLarea's BackGround Color(bottom corner)"));
    defaultGlobalParamSet->addParam(new RichColor(textColorParam(),vcg::Color4b::White,"Text Color","Color of the text used in all the Graphics Window (it should be well different from the background color...)"));

    defaultGlobalParamSet->addParam(new RichColor(baseLightAmbientColorParam()	,QColor(30,30,30),"Fusion Base Light Ambient Color","Fusion GLarea's BackGround Color(bottom corner)"));
    defaultGlobalParamSet->addParam(new RichColor(baseLightDiffuseColorParam()	,QColor(255,255,255),"Fusion Base Light Diffuse Color","Fusion GLarea's BackGround Color(top corner)"));
    defaultGlobalParamSet->addParam(new RichColor(baseLightSpecularColorParam() ,QColor(255,255,255),"Fusion Base Light Specular Color","Fusion GLarea's BackGround Color(bottom corner)"));

    defaultGlobalParamSet->addParam(new RichColor(fancyBLightDiffuseColorParam()	,QColor(255,204,204),"Fusion Base Light Diffuse Color","Fusion GLarea's BackGround Color(top corner)"));
    defaultGlobalParamSet->addParam(new RichColor(fancyFLightDiffuseColorParam()	,QColor(204,204,255),"Fusion Base Light Diffuse Color","Fusion GLarea's BackGround Color(top corner)"));

    QStringList textureMinFilterModes =  (QStringList() << "Nearest" << "MipMap");
    QStringList textureMagFilterModes =  (QStringList() << "Nearest" << "Linear");
    defaultGlobalParamSet->addParam(new RichEnum(textureMinFilterParam()	, 1,textureMinFilterModes,"Fusion Texture Minification Filtering","Fusion GLarea's BackGround Color(top corner)"));
    defaultGlobalParamSet->addParam(new RichEnum(textureMagFilterParam()	, 1,textureMagFilterModes,"Fusion Texture Magnification Filtering","Fusion GLarea's BackGround Color(top corner)"));

    defaultGlobalParamSet->addParam(new RichBool(pointDistanceAttenuationParam()	, true,"Perspective Varying Point Size","If true the size of the points is drawn with a size proprtional to the distance from the observer."));
    defaultGlobalParamSet->addParam(new RichBool(pointSmoothParam()	, false,"Antialiased Point","If true the points are drawn with small circles instead of fast squared dots."));
    defaultGlobalParamSet->addParam(new RichFloat(pointSizeParam()	, 2.0, "Point Size","The base size of points when drawn"));
}

void GLAreaSetting::updateGlobalParameterSet( RichParameterSet& rps )
{
    logAreaColor = rps.getColor4b(logAreaColorParam());
    backgroundBotColor =  rps.getColor4b(backgroundBotColorParam());
    backgroundTopColor =  rps.getColor4b(backgroundTopColorParam());

    baseLightAmbientColor =  rps.getColor4b(baseLightAmbientColorParam()	);
    baseLightDiffuseColor =  rps.getColor4b(baseLightDiffuseColorParam()	);
    baseLightSpecularColor =  rps.getColor4b(baseLightSpecularColorParam() );

    fancyBLightDiffuseColor =  rps.getColor4b(fancyBLightDiffuseColorParam());
    fancyFLightDiffuseColor =  rps.getColor4b(fancyFLightDiffuseColorParam());

    textureMinFilter = rps.getEnum(this->textureMinFilterParam());
    textureMagFilter = rps.getEnum(this->textureMagFilterParam());

    pointDistanceAttenuation = rps.getBool(this->pointDistanceAttenuationParam());
    pointSmooth = rps.getBool(this->pointSmoothParam());
    pointSize = rps.getFloat(this->pointSizeParam());
    currentGlobalParamSet=&rps;
}
