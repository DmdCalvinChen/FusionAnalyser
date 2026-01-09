/****************************************************************************
* MeshLab                                                           o o     *
* A versatile mesh processing toolbox                             o     o   *
*                                                                _   O  _   *
* Copyright(C) 2005                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/

#ifndef GLAREA_SETTING_H
#define GLAREA_SETTING_H

class GLAreaSetting
{
public:

    vcg::Color4b baseLightAmbientColor;
    vcg::Color4b baseLightDiffuseColor;
    vcg::Color4b baseLightSpecularColor;
    inline static QString baseLightAmbientColorParam()		{return "Fusion::Appearance::baseLightAmbientColor";}
    inline static QString baseLightDiffuseColorParam()		{return "Fusion::Appearance::baseLightDiffuseColor";}
    inline static QString baseLightSpecularColorParam()		{return "Fusion::Appearance::baseLightSpecularColor";}

    vcg::Color4b fancyBLightDiffuseColor;
    inline static QString fancyBLightDiffuseColorParam()		{return "Fusion::Appearance::fancyBLightDiffuseColor";}

    vcg::Color4b fancyFLightDiffuseColor;
    inline static QString fancyFLightDiffuseColorParam()		{return "Fusion::Appearance::fancyFLightDiffuseColor";}

    vcg::Color4b backgroundBotColor;
    vcg::Color4b backgroundTopColor;
    vcg::Color4b logAreaColor;
    vcg::Color4b textColor;
    inline static QString backgroundBotColorParam()		{return "Fusion::Appearance::backgroundBotColor";}
    inline static QString backgroundTopColorParam()		{return "Fusion::Appearance::backgroundTopColor";}
    inline static QString logAreaColorParam()           {return "Fusion::Appearance::logAreaColor";}
    inline static QString textColorParam()           {return "Fusion::Appearance::textColor";}

    int textureMagFilter;
    int textureMinFilter;
    inline static QString textureMinFilterParam()           {return "Fusion::Appearance::textureMinFilter";}
    inline static QString textureMagFilterParam()           {return "Fusion::Appearance::textureMagFilter";}

    bool pointDistanceAttenuation;
    inline static QString pointDistanceAttenuationParam()           {return "Fusion::Appearance::pointDistanceAttenuation";}
    bool pointSmooth;
    inline static QString pointSmoothParam()           {return "Fusion::Appearance::pointSmooth";}
    float pointSize;
    inline static QString pointSizeParam()           {return "Fusion::Appearance::pointSize";}

    void updateGlobalParameterSet( RichParameterSet& rps );
    static void initGlobalParameterSet( RichParameterSet * defaultGlobalParamSet);

    RichParameterSet *currentGlobalParamSet;
};

#endif // GLAREA_SETTING_H
