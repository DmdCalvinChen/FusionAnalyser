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

#ifndef SKIPLIST_H
#define SKIPLIST_H

#define SKIPLIST_MAXLEVEL 32
#define SKIPLIST_P 25

#include <QTime>
#include "common_ext_global.h"

template<typename T>
class COMMON_EXT_EXPORT SkiplistNode
{
public:
	SkiplistNode() {}
public:
	size_t score_;
	T* obj_ = nullptr;
	SkiplistNode* backward_ = nullptr;
	struct SkiplistLevel
	{
		SkiplistLevel() {}
		SkiplistNode<T>* forward_ = nullptr;
		unsigned int span_;
	} level_[];
};

template<typename T>
class COMMON_EXT_EXPORT SkipList
{
public:
	SkipList();
	SkiplistNode<T>* createNode(int _level, size_t _score, T* _obj);
	int randomLevel();
	T* insert(size_t _score, T* _obj);
	void deleteList(size_t _score);
	void deleteNode(SkiplistNode<T>* _x, SkiplistNode<T>** _update);
	T* findNode(size_t _score);
private:
	SkiplistNode<T>* header_ = nullptr, * tail_ = nullptr;
	unsigned long length_ = 0;//ڵĿ
	int level_ = 0;//Ŀǰ
};

#endif // ! SKIPLIST_H
