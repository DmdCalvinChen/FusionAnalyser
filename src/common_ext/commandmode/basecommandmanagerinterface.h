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

#ifndef BASECOMMANDMANAGERINTERFACE_H
#define BASECOMMANDMANAGERINTERFACE_H

#include<stack>

class BaseCommand;

class CommandCombine;

/*
** Base Command manager Interface for unDo and reDo command. Code designed by liuchuang
** on 20/2/2019.
*/

class  BaseCommandManagerInterface
{
public:
    virtual ~BaseCommandManagerInterface() {}
    virtual bool addCommand(std::pair<BaseCommand*, BaseCommand*> pCommand) = 0;
    virtual void clearAllCommands() = 0;
    virtual CommandCombine unDo() = 0;
    virtual CommandCombine reDo() = 0;
    virtual bool canUndo() const = 0;
    virtual bool canRedo() const = 0;
};

#endif //BASECOMMANDMANAGERINTERFACE_H
