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

#ifndef BASECOMMAND_H
#define BASECOMMAND_H

/*
** Command Type Enum Command for unDo and reDo command.
** Code designed by liuchuang on 20/2/2019.
*/

#include "common_ext_global.h"

enum CommandTypeEnum
{
	NULL_COMMAND,              // null command for init one null command.
	// single Data stores. Such as int intDataCommand;
	INT_TYPE_COMMAND,          // int data type
	FLOAT_TYPE_COMMAND,        // float data type
	DOUBLE_TYPE_COMMAND,       // double data type
	BOOL_TYPE_COMMAND,         // bool data type
	MESH_TYPE_COMMAND,         // mesh of vcg data type
	POINT3M_TYPE_COMMAND,      // point3m  of vcg vertex data type
	RGBA_TYPE_COMMAND,         // rgba of vcg color data type

	// Data stores as a vector. Such as std::vector<int> intDataCommandVec;
	VECTOR_INT_TYPE_COMMAND,   // int data stores as a vector container type
	VECTOR_FLOAT_TYPE_COMMAND, // float data stores as a vector container type
	VECTOR_DOUBLE_TYPE_COMMAND,// double data stores as a vector container type
	VECTOR_BOOL_TYPE_COMMAND,  // bool data stores as a vector container type
	VECTOR_MESH_TYPE_COMMAND,  // mesh of vcg data stores as a vector container type
	VECTOR_POINT3M_TYPE_COMMAND, // point3m  of vcg vertex data stores as a vector container type
	VECTOR_RGBA_TYPE_COMMAND,    // rgba of vcg color data stores as a vector container type

	MACRO_TYPE_COMMAND,         // macro command including some command type, seem as group command.
								//...add others data type here...

	MATRIX_TYPE_COMMAND,        // model matrix changes
	GLARE_OPITION_CONTROL_COMMAND,// glare control set

    /////////// Segment part enum begin.
	SEGMENT_NEARFAR_POINTS_PICKED_COMMAND,  // the axis vector stores for tooth segmented when picking vertexs.
    SEGMENT_CUTTINGLINES_ADJUST_COMMAND, // used in the plugin of segment for adjsut cutting lines.
	SEGMENT_TOOTH_FDI_MARK_COMMAND,      // tooth fdi data store
	SEGMENT_AXIS_TOOTH_ADJUST_COMMAND,   // adjust tooth axis
	SEGMENT_LOCAL_TOOTH_REPAIR_COMMAND, // local tooth smooth command
	////////////Segment part enum end.

	// !!!!!!Note:all copy command must write here!!!!!!!!!!
	COPY_COMMAND_BEGIN,

	MATRIX_TYPE_COPY_COMMAND,        // model matrix changes
	GLARE_OPITION_CONTROL_COPY_COMMAND,// glare control set
	MESH_COPY_COMMAND,                 // mesh of vcg data type

	SEGMENT_NEARFAR_POINTS_PICKED_COPY_COMMAND,// copy data for init command when the data type changes.
	SEGMENT_CUTTINGLINES_ADJUST_COPY_COMMAND,// copy data for init command when the data type changes.
	SEGMENT_TOOTH_FDI_MARK_COPY_COMMAND,     //  COPY data of tooth fdi data store
	SEGMENT_AXIS_TOOTH_ADJUST_COPY_COMMAND,  // adjust tooth axis
	SEGMENT_LOCAL_TOOTH_REPAIR_COPY_COMMAND, // local tooth smooth command

											 // copy command write here. add code here...

    /////////// ToothAdjust part enum begin.
	TOOTHADJ_TOOTH_PICKED_COMMAND,
	TOOTHADJ_TEETH_PICKED_COMMAND,
	TOOTHADJ_ADJUST_DENTATION_COMMAND,
	TOOTHADJ_STAGE_PICKED_COMMAND,
	TOOTHADJ_ADD_CURESTAGE_COMMAND,
	TOOTHADJ_DEL_CURESTAGE_COMMAND,
	////////////ToothAdjust part enum end.

	/////////// Accessory part enum begin.
	ACCESSORY_STAGE_PICKED_COMMAND,
	ACCESSORY_ADD_ONE_ACCESSORY_COMMAND,
	ACCESSORY_ACCESORY_PICKED_COMMAND,
	ACCESSORY_DELETE_ONE_ACCESORY_COMMAND,
	ACCESSORY_INHERIT_SAVED_ACC_TAB_COMMAND,
	ACCESSORY_TRANSFORMED_COMMAND,
	/////////// Accessory part enum end.

	/////////// Treatment Drag Design Table part enum begin.
	TREATMENT_DRAGTAB_ADD_NODECP_COMMAND,
	TREATMENT_DRAGTAB_DEL_NODECP_COMMAND,
	TREATMENT_DRAGTAB_ADJ_NODECP_COMMAND,
	TREATMENT_DRAGTAB_CLEAR_COMMAND,
	TREATMENT_DRAGTAB_STATE_SWITCH_COMMAND,
	TREATMENT_DRAGTAB_INSERT_KEYSTAGE_COMMAND,
	/////////// Treatment Drag Design Table enum end.

	/////////// Basal Plane part enum begin.
	BASALPLANE_SWITCH_MODE_COMMAND,
	BASALPLANE_ADJUST_DENTAL_COMMAND,
	BASALPLANE_ADJUST_PLANE_COMMAND,
	/////////// Basal Plane enum end.

	/// 标记插件命令枚举
	MODEL_MARK_POINTS_PICKED_COMMAND,
	MODEL_MARK_POINT_MODIFY_COMMAND,
	MODEL_MARK_POINT_CLEAR_COMMAND,
	MODEL_MARK_POINT_ALL_CLEAR_COMMAND,

	COPY_COMMAND_END
};

/*
** Base Command for unDo and reDo command. Code designed by liuchuang
** on 20/2/2019.
*/
class COMMON_EXT_EXPORT BaseCommand
{
private:
	CommandTypeEnum commandTypeEnum = NULL_COMMAND;
public:
	BaseCommand(){}
	virtual ~BaseCommand() {}

	void setCommandType(CommandTypeEnum _commandTypeEnum)
	{
		this->commandTypeEnum = _commandTypeEnum;
	}

	CommandTypeEnum getCommandType()const
	{
		return commandTypeEnum;
	}

};

/* command combine for look up the history command.Code designed by licuhuang
**on 25/2/2019.
*/
class COMMON_EXT_EXPORT CommandCombine : public BaseCommand
{
public:
	CommandCombine(
		BaseCommand* _pNewCommand,
		BaseCommand* _pOldCommand,
		BaseCommand* _pCopyCommand = nullptr)
	{
		if (pNewCommand)
		{
			delete pNewCommand;
		}
		if (pOldCommand)
		{
			delete pOldCommand;
		}
		if (pCopyCommand)
		{
			delete pCopyCommand;
		}
		pNewCommand = nullptr;
		pOldCommand = nullptr;
		pCopyCommand = nullptr;
		pNewCommand = _pNewCommand;
		pOldCommand = _pOldCommand;
		pCopyCommand = _pCopyCommand;
	}

	~CommandCombine()
	{

	}
	void setNewCommand(BaseCommand* _pNewCommand)
	{
		pNewCommand = _pNewCommand;
	}

	void setOldCommand(BaseCommand* _pOldCommand)
	{
		pOldCommand = _pOldCommand;
	}

	void setCopyCommand(BaseCommand* _pCopyCommand)
	{
		pCopyCommand = _pCopyCommand;
	}

	BaseCommand* getNewCommand()const
	{
		return pNewCommand;
	}

	BaseCommand* getOldCommand()const
	{
		return pOldCommand;
	}
	BaseCommand* getCopyCommand()const
	{
		return pCopyCommand;
	}
private:
	BaseCommand* pNewCommand = nullptr;
	BaseCommand* pOldCommand = nullptr;
	BaseCommand* pCopyCommand = nullptr;
};

#endif
