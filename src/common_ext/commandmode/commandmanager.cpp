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

#include "basecommandmanagerinterface.h"
#include "commandmanager.h"

CommandManager::CommandManager()
{
	p_cur_undo_stack_ = &stackUndo;
}

CommandManager::~CommandManager()
{
    clearAllCommands();
}

CommandManager::CommandManager(const CommandManager& rhs)
{
	this->clearAllCommands();
	this->stackUndo = rhs.stackUndo;
	this->stackRedo = rhs.stackRedo;
}

CommandManager& CommandManager::operator=(const CommandManager& rhs)
{
	this->clearAllCommands();
	this->stackUndo = rhs.stackUndo;
	this->stackRedo = rhs.stackRedo;
	return *this;
}

bool CommandManager::addCommand(std::pair<BaseCommand*, BaseCommand*> pCommand)
{
    if (pCommand.first)
    {
		pushUndoCommand(pCommand);
		deleteRedoCommands();
		return true;
    }
    return false;
}

void CommandManager::clearAllCommands()
{
    deleteUndoCommands();
    deleteRedoCommands();
}

CommandCombine CommandManager::unDo()
{
	BaseCommand* pOldCommand = nullptr;
	if (p_cur_undo_stack_ && !p_cur_undo_stack_->empty())
	{
		pOldCommand = p_cur_undo_stack_->top().first;
	}

	std::pair<BaseCommand*, BaseCommand*> pCommandVec = popUndoCommand();
	if (pCommandVec.first || pCommandVec.second)
	{
		pushRedoCommand(pCommandVec);
	}

	BaseCommand* pCopyCommand = nullptr;
    if(pCommandVec.second)
    {
        if (pCommandVec.second->getCommandType() > COPY_COMMAND_BEGIN &&
            pCommandVec.second->getCommandType() < COPY_COMMAND_END)
        {
            pCopyCommand = pCommandVec.second;
        }
    }

	BaseCommand * pNewCommand = nullptr;
	if (p_cur_undo_stack_ && !p_cur_undo_stack_->empty())
	{
		pNewCommand = p_cur_undo_stack_->top().first;
	}
	return CommandCombine(pNewCommand, pOldCommand, pCopyCommand);
}

CommandCombine CommandManager::reDo()
{
	std::pair<BaseCommand*, BaseCommand*> pCommand = popRedoCommand();
    if (pCommand.first)
    {
		pushUndoCommand(pCommand);
    }
	return CommandCombine(pCommand.first, pCommand.second);
}

bool CommandManager::canUndo() const
{
	if (p_cur_undo_stack_)
	{
		return !p_cur_undo_stack_->empty();
	}
	else
	{
		return false;
	}
}

bool CommandManager::canRedo() const
{
    return !stackRedo.empty();
}

void CommandManager::pushUndoCommand(std::pair<BaseCommand*, BaseCommand*> pCommand)
{
    if (pCommand.first)
    {
		if (p_cur_undo_stack_)
		{
			if (p_cur_undo_stack_->size() == COMMAND_MAX_SIZE)
			{
				p_cur_undo_stack_ = p_cur_undo_stack_ == &stackUndo ? &stack_undo_buf_ : &stackUndo;
				if (p_cur_undo_stack_->size() == COMMAND_MAX_SIZE)
				{
					while (!p_cur_undo_stack_->empty())
					{
						p_cur_undo_stack_->pop();
					}
				}
			}
			p_cur_undo_stack_->push(pCommand);
		}

    }
}

std::pair<BaseCommand*, BaseCommand*> CommandManager::popUndoCommand()
{
	std::pair<BaseCommand*, BaseCommand*> pCommand;
	if (p_cur_undo_stack_)
	{
		if (p_cur_undo_stack_->empty())
		{
			if (!stackUndo.empty() || !stack_undo_buf_.empty())
			{
				p_cur_undo_stack_ = p_cur_undo_stack_ == &stackUndo ? &stack_undo_buf_ : &stackUndo;
			}
		}
	}
    if (p_cur_undo_stack_ && !p_cur_undo_stack_->empty())
    {
		pCommand = p_cur_undo_stack_->top();
		p_cur_undo_stack_->pop();
    }
	if (p_cur_undo_stack_)
	{
		if (p_cur_undo_stack_->empty())
		{
			if (!stackUndo.empty() || !stack_undo_buf_.empty())
			{
				p_cur_undo_stack_ = p_cur_undo_stack_ == &stackUndo ? &stack_undo_buf_ : &stackUndo;
			}
		}
	}
    return pCommand;
}

void CommandManager::pushRedoCommand(std::pair<BaseCommand*, BaseCommand*> pCommand)
{
	stackRedo.push(pCommand);
}

std::pair<BaseCommand*, BaseCommand*> CommandManager::popRedoCommand()
{
	std::pair<BaseCommand*, BaseCommand*> pCommand;
    if (!stackRedo.empty())
    {
        pCommand = stackRedo.top();
        stackRedo.pop();
    }
    return pCommand;
}

void CommandManager::deleteUndoCommands()
{
    while (!stackUndo.empty())
    {
        delete stackUndo.top().first;
        stackUndo.pop();
    }
	p_cur_undo_stack_ = &stackUndo;

	std::stack<std::pair<BaseCommand*, BaseCommand*>>().swap(stack_undo_buf_);
}

void CommandManager::deleteRedoCommands()
{
    while (!stackRedo.empty())
    {
		if (stackRedo.top().first != nullptr)
		{
			delete stackRedo.top().first;
			stackRedo.top().first = nullptr;
		}

		if (stackRedo.top().second != nullptr)
		{
			delete stackRedo.top().second;
			stackRedo.top().second = nullptr;
		}

        stackRedo.pop();
    }

	std::stack<std::pair<BaseCommand*, BaseCommand*>>().swap(stack_redo_buf_);

}
