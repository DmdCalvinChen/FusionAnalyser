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

#ifndef  COMMON_CUSTOMTRACKBALL_H
#define  COMMON_CUSTOMTRACKBALL_H
#include <vector>

#include "util/assist_geometry.h"
#include "common_ext_global.h"

class AbsMesh;
class CustomAdjustingHandle;
class  COMMON_EXT_EXPORT CustomHandlePackage
{
public:
	CustomHandlePackage(int mode_index_);
	~CustomHandlePackage();
	void draw();
	bool pick(int _mouseX, int _mouseY);
	bool pickFineTuneMode(int _mouseX, int _mouseY);
	void clearFineTuneMode();
	bool action(int _mouseX, int _mouseY);
	bool actionFineTune(int _direct);
	void addHandle(CustomAdjustingHandle* _handle);
	int getModeIndex();
	void setObject(AbsMesh *_obj);
	void update();
	bool fineTuneEnable();
	void afterMouseRelease();
	void setAdjustAxisSelf(bool _state);
	Point3f currentMoveDirect();
	bool operatePositiveHandle();

private:
	int mode_index_;
	std::vector<CustomAdjustingHandle*> handles_;
	CustomAdjustingHandle *picked_handle_ = nullptr;
};

class COMMON_EXT_EXPORT CustomTrackball
{
public:
	CustomTrackball();
	~CustomTrackball();
	void draw();
	bool pick(int _mouseX, int _mouseY);
	bool pickFineTuneMode(int _mouseX, int _mouseY);
	void clearFineTuneMode();
	bool action(int _mouseX, int _mouseY);
	bool actionFineTune(int _direct);
	void addHandlePackage(CustomHandlePackage* _package);
	bool setCurrentMode(int _mode_index);
	bool fineTuneEnable();
	void setObject(AbsMesh *_obj);
	void update();
	void afterMouseRelease();
	void setAdjustAxisSelf(bool _state);
	Point3f currentMoveDirection();
	bool operatePositiveHandle();

private:
	std::vector<CustomHandlePackage*> package_list_;
	CustomHandlePackage *cur_package_ = nullptr;
};

#endif // ! COMMON_CUSTOMTRACKBALL_H
