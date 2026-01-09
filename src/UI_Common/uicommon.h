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

#ifndef UICOMMON_H
#define UICOMMON_H

#include "uicommon_global.h"

class QTranslator;

namespace ui_common
{
class UI_COMMONSHARED_EXPORT UI_Common
{
public:
    UI_Common();
    ~UI_Common();

    static void loadTranslator(int type);

private:
    static QTranslator translator_;
    static bool translator_loaded_;
};

}    // namespace ui_common

#endif    // UICOMMON_H
