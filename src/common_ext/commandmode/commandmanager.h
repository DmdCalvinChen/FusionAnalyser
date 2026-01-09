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

#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <stack>
#include "basecommandmanagerinterface.h"
#include "basecommand.h"

#define COMMAND_MAX_SIZE 10

class COMMON_EXT_EXPORT CommandManager : public BaseCommandManagerInterface
{
public:
    CommandManager();
	~CommandManager();
    CommandManager(const CommandManager& rhs);
	CommandManager& operator=(const CommandManager& rhs);

public:
    bool addCommand(std::pair<BaseCommand*, BaseCommand*> pCommand);
	void clearAllCommands();
	// return the new top command and reference the old command
	CommandCombine unDo();
	CommandCombine reDo();
    bool canUndo() const;
    bool canRedo() const;
	BaseCommand* getTopUndoCommand()const
	{
		BaseCommand* pCommand = nullptr;
		if (stackUndo.empty())
		{
			return pCommand;
		}
		if (stackUndo.top().second->getCommandType() == NULL_COMMAND)
		{
			return stackUndo.top().first;
		}
		else
		{
			return stackUndo.top().second;
		}

	}
	BaseCommand* getTopRedoCommand()const
	{
		BaseCommand* pCommand = nullptr;
		if (stackRedo.empty())
		{
			return pCommand;
		}
		return stackRedo.top().first;
	}

private:
    void pushUndoCommand(std::pair<BaseCommand*, BaseCommand*> pCommand);
	std::pair<BaseCommand*, BaseCommand*> popUndoCommand();
    void pushRedoCommand(std::pair<BaseCommand*, BaseCommand*> pCommand);
	std::pair<BaseCommand*, BaseCommand*> popRedoCommand();
    void deleteUndoCommands();
    void deleteRedoCommands();

private:
	BaseCommand *pCommandOld = nullptr;
    std::stack<std::pair<BaseCommand*, BaseCommand*>> stackUndo;
    std::stack<std::pair<BaseCommand*, BaseCommand*>> stackRedo;

	std::stack<std::pair<BaseCommand*, BaseCommand*>> stack_undo_buf_;
	std::stack<std::pair<BaseCommand*, BaseCommand*>> stack_redo_buf_;

	int cur_undo_index_ = 0;
	int cur_redo_index_ = 0;

	std::stack<std::pair<BaseCommand*, BaseCommand*>>* p_cur_undo_stack_ = nullptr;
	std::stack<std::pair<BaseCommand*, BaseCommand*>>* p_cur_redo_stack  = nullptr;
};

#endif
