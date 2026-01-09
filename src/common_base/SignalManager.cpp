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

#include "SignalManager.h"
#include <QSharedMemory>

SignalManager::Garbo  SignalManager::garbo;
SignalManager* SignalManager::p_instance_ = nullptr;

SignalManager * getSignalManager()
{
    if (p_signal_mgr == nullptr)
    {
        p_signal_mgr = SignalManager::getInstance();
    }
    return  p_signal_mgr;
}
SignalManager *p_signal_mgr = SignalManager::getInstance();
SignalManager::SignalManager(QObject *parent)
    : QObject(parent)
{
}

SignalManager::~SignalManager()
{
}
