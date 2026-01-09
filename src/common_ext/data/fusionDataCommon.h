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

#ifndef FUSIONDATACOMMON_H
#define FUSIONDATACOMMON_H

#include <QString>

struct  sAlignProj
{
	enum eAlignProjType
	{
		EALIGNPROJTYPE_TEMPORARY = 0,
		EALIGNPROJTYPE_SUBMIT
	};

	eAlignProjType type_ = EALIGNPROJTYPE_TEMPORARY;
	QString       file_name_;
	QString       scheme_name_ = "";
	QString       time_;
	int           design_id_ = -1;

	bool operator == (const sAlignProj& proj)
	{
		return proj.file_name_ == this->file_name_ || proj.scheme_name_ == this->scheme_name_;
	}
};

class FusionData
{
public:
	virtual ~FusionData() {};
};



#endif
