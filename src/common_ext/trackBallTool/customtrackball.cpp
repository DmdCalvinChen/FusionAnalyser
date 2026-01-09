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

#include "customtrackball.h"
#include "trackBallTool/customadjustinghandle.h"
#include "meshExt/AbsMesh.h"

CustomHandlePackage::CustomHandlePackage(int _mode_index)
{
	mode_index_ = _mode_index;
}

CustomHandlePackage::~CustomHandlePackage()
{
	for (int i = 0; i < this->handles_.size(); ++i)
	{
		delete handles_[i];
		handles_[i] = nullptr;
	}
	std::vector<CustomAdjustingHandle*>().swap(handles_);
}

void CustomHandlePackage::draw()
{
	for (int i = 0; i < this->handles_.size(); ++i)
	{
		handles_[i]->drawHandle();
	}
}

bool CustomHandlePackage::pick(int _mouseX, int _mouseY)
{
	for (int i = 0; i < this->handles_.size(); ++i)
	{
		if (handles_[i]->pickHandel(_mouseX, _mouseY))
		{
			picked_handle_ = handles_[i];
			return true;
		}
	}
	picked_handle_ = nullptr;
	return false;
}

bool CustomHandlePackage::pickFineTuneMode(int _mouseX, int _mouseY)
{
	bool picked = pick(_mouseX, _mouseY);
	if (picked)
	{
		picked_handle_->setFineTune(true);
	}
	else
	{
		for (int i = 0; i < this->handles_.size(); ++i)
		{
			handles_[i]->setFineTune(false);
		}
	}
	return picked;
}

void CustomHandlePackage::clearFineTuneMode()
{
	if (picked_handle_ != nullptr)
	{
		picked_handle_->setFineTune(false);
	}
	return;
}

bool CustomHandlePackage::action(int _mouseX, int _mouseY)
{
	if (picked_handle_ == nullptr)
	{
		return false;
	}
	picked_handle_->adjustAction(_mouseX, _mouseY);
	return true;
}

bool CustomHandlePackage::actionFineTune(int _direct)
{
	if (picked_handle_ == nullptr)
	{
		return false;
	}
	if (picked_handle_->fineTuneEnable())
	{
		picked_handle_->adjustFineTuneAction(_direct);
		return true;
	}
	else
	{
		return false;
	}
}

void CustomHandlePackage::addHandle(CustomAdjustingHandle* _handle)
{
	this->handles_.push_back(_handle);
}

int CustomHandlePackage::getModeIndex()
{
	return mode_index_;
}

void CustomHandlePackage::setObject(AbsMesh *_obj)
{
	for (int i = 0; i < handles_.size(); ++i)
	{
		handles_[i]->setObject(_obj);
	}
}

void CustomHandlePackage::update()
{
	for (int i = 0; i < handles_.size(); ++i)
	{
		handles_[i]->updateCtrlSys();
	}
}

bool CustomHandlePackage::fineTuneEnable()
{
	if (picked_handle_ == nullptr)
	{
		return false;
	}
	return picked_handle_->fineTuneEnable();
}

void CustomHandlePackage::afterMouseRelease()
{
	if (picked_handle_ == nullptr)
	{
		return;
	}
	picked_handle_->afterMouseRelease();
	return;
}

void CustomHandlePackage::setAdjustAxisSelf(bool _state)
{
	for (int i = 0; i < this->handles_.size(); ++i)
	{
		handles_[i]->setAdjustAxisSelf(_state);
	}
}

Point3f CustomHandlePackage::currentMoveDirect()
{
	if (picked_handle_ == nullptr)
	{
		return Point3m(0,0,0);
	}
	return picked_handle_->currentMoveDirect();
}

bool CustomHandlePackage::operatePositiveHandle()
{
	if (picked_handle_ == nullptr)
	{
		return false;
	}
	return picked_handle_->operatePositiveHandle();
}

CustomTrackball::CustomTrackball()
{
}

CustomTrackball::~CustomTrackball()
{
}

void CustomTrackball::draw()
{
	if (cur_package_ == nullptr)
	{
		return;
	}
	cur_package_->draw();
}

bool CustomTrackball::pick(int _mouseX, int _mouseY)
{
	if (cur_package_ == nullptr)
	{
		return false;
	}
	return cur_package_->pick(_mouseX, _mouseY);
}

bool CustomTrackball::pickFineTuneMode(int _mouseX, int _mouseY)
{
	if (cur_package_ == nullptr)
	{
		return false;
	}
	return cur_package_->pickFineTuneMode(_mouseX, _mouseY);
}

void CustomTrackball::clearFineTuneMode()
{
	if (cur_package_ == nullptr)
	{
		return;
	}
	cur_package_->clearFineTuneMode();
	return;
}

bool CustomTrackball::action(int _mouseX, int _mouseY)
{
	if (cur_package_ == nullptr)
	{
		return false;
	}
	cur_package_->action(_mouseX, _mouseY);
	this->update();
	return true;
}

vcg::Point3f CustomTrackball::currentMoveDirection()
{
	if (cur_package_ == nullptr)
	{
		return Point3m(0, 0, 0);
	}
	return cur_package_->currentMoveDirect();
}

bool CustomTrackball::operatePositiveHandle()
{
	if (cur_package_ == nullptr)
	{
		return false;
	}
	return cur_package_->operatePositiveHandle();
}

bool CustomTrackball::actionFineTune(int _direct)
{
	if (cur_package_ == nullptr)
	{
		return false;
	}
	cur_package_->actionFineTune(_direct);
	this->update();
	return true;
}

void CustomTrackball::addHandlePackage(CustomHandlePackage* _package)
{
	this->package_list_.push_back(_package);
	setCurrentMode(_package->getModeIndex());
}

bool CustomTrackball::setCurrentMode(int _mode_index)
{
	for (int i = 0; i < package_list_.size(); ++i)
	{
		if (_mode_index == package_list_[i]->getModeIndex())
		{
			cur_package_ = package_list_[i];
			return true;
		}
	}
	return false;
}

void CustomTrackball::setObject(AbsMesh *_obj)
{
	for (int i = 0; i < package_list_.size(); ++i)
	{
		package_list_[i]->setObject(_obj);
	}
}

void CustomTrackball::update()
{
	for (int i = 0; i < package_list_.size(); ++i)
	{
		package_list_[i]->update();
	}
}

bool CustomTrackball::fineTuneEnable()
{
	if (cur_package_ == nullptr)
	{
		return false;
	}
	return cur_package_->fineTuneEnable();
}

void CustomTrackball::afterMouseRelease()
{
	if (cur_package_ == nullptr)
	{
		return;
	}
	cur_package_->afterMouseRelease();
	return;
}

void CustomTrackball::setAdjustAxisSelf(bool _state)
{
	if (cur_package_ == nullptr)
	{
		return;
	}
	return cur_package_->setAdjustAxisSelf(_state);
}
