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

#ifndef COMMANDCOMMON_H
#define COMMANDCOMMON_H

#include "common/ml_mesh_type.h"
#include "basecommand.h"

template<class DataType>
class  CommandCommon : public BaseCommand
{
private:
	DataType dataCommand;
public:
	CommandCommon(CommandTypeEnum _commandTypeEnum, DataType _dataCommand);
	~CommandCommon();
public:
	DataType getData()const;
	void setData(const DataType& _dataCommand);
};

template<class DataType>
 CommandCommon<DataType>::CommandCommon(CommandTypeEnum _commandTypeEnum, DataType _dataCommand)
{
	BaseCommand::setCommandType(_commandTypeEnum);
	this->dataCommand = _dataCommand;
}

template<class DataType>
 CommandCommon<DataType>::~CommandCommon()
{
}

template<class DataType>
DataType  CommandCommon<DataType>::getData()const
{
	return this->dataCommand;
}

template<class DataType>
void  CommandCommon<DataType>::setData(const DataType& _dataCommand)
{
	this->dataCommand = _dataCommand;
}

// Specialization for CMeshO*
template<>
class  COMMON_EXT_EXPORT CommandCommon<CMeshO*> : public BaseCommand
{
private:
	CMeshO dataCommand;
public:
	CommandCommon(CommandTypeEnum _commandTypeEnum, CMeshO* _dataCommand);
	~CommandCommon();
public:
	CMeshO* getData();
	void setData(CMeshO* _dataCommand);
};

#endif
