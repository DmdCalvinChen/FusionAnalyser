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

#include "skip_list.h"

template<typename T> SkipList<T>::SkipList()
{
    this->level_ = 1;
    this->length_ = 0;
    this->header_ = createNode(SKIPLIST_MAXLEVEL, 0, nullptr);
    for (int i = 0; i < SKIPLIST_MAXLEVEL; ++i)
    {
        header_->level_[i].forward_ = nullptr;
        header_->level_[i].span_ = 0;
    }
    header_->backward_ = nullptr;
    this->tail_ = nullptr;
}

template<typename T> SkiplistNode<T>* SkipList<T>::createNode(int _level, size_t _score, T* _obj)
{
    SkiplistNode<T>* zn = (SkiplistNode<T>*)malloc(sizeof(*zn) + _level * sizeof(SkiplistNode<T>::SkiplistLevel));
    zn->score_ = _score;
    zn->obj_ = _obj;
    return zn;
}

template<typename T> int SkipList<T>::randomLevel()
{
    int level = 1;
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
    while (qrand() % 100 < 25 && level < SKIPLIST_MAXLEVEL)
    {
        level += 1;
    }
    return level;
}

template<typename T> T* SkipList<T>::insert(size_t _score, T* _obj)
{
    unsigned int rank[SKIPLIST_MAXLEVEL];
    SkiplistNode<T>* update[SKIPLIST_MAXLEVEL];

    SkiplistNode<T>* x = this->header_;
    for (int i = this->level_ - 1; i >= 0; --i)
    {
        rank[i] = i == (this->level_ - 1) ? 0 : rank[i + 1];
        while (x->level_[i].forward_ && x->level_[i].forward_->score_ <= _score)
        {
            x = x->level_[i].forward_;//

            if (x->score_ == _score)
            {
                return x->obj_;
            }
        }
        update[i] = x;
    }

    int level = randomLevel();
    if (level > this->level_)
    {
        for (int i = this->level_; i < level; ++i)
        {
            rank[i] = 0;
            update[i] = this->header_;
        }
        this->level_ = level;
    }

    x = createNode(level, _score, _obj);
    for (int i = 0; i < level; ++i)
    {
        x->level_[i].forward_ = update[i]->level_[i].forward_;
        update[i]->level_[i].forward_ = x;
    }

    x->backward_ = (update[0] == this->header_) ? nullptr : update[0];//ǰָ
    if (x->level_[0].forward_)
    {
        x->level_[0].forward_->backward_ = x;
    }
    else
    {
        this->tail_ = x;
    }

    ++this->length_;
    return nullptr;
}

template<typename T> void SkipList<T>::deleteList(size_t _score)
{
    SkiplistNode<T>* update[SKIPLIST_MAXLEVEL];
    SkiplistNode<T>* x = this->header_;
    for (int i = this->level_ - 1; i >= 0; i--)
    {
        while (x->level_[i].forward_ && x->level_[i].forward_->score_ < _score)
        {
            x = x->level_[i].forward_;
        }
        update[i] = x;
    }

    x = x->level_[0].forward_;
    if (x && _score == x->score_)
    {
        this->deleteNode(x, update);
        free(x);
        return;
    }
    return;
}

template<typename T> void SkipList<T>::deleteNode(SkiplistNode<T>* _x, SkiplistNode<T>** _update)
{
    for (int i = 0; i < this->level_; ++i)
    {
        if (_update[i]->level_[i].forward_ == _x)
        {
            _update[i]->level_[i].forward_ = _x->level_[i].forward_;
        }
    }
    if (_x->level_[0].forward_)
    {
        _x->level_[0].forward_->backward_ = _x->backward_;
    }
    else
    {
        this->tail_ = _x->backward_;
    }

    while (this->level_ > 1 && this->header_->level_[this->level_ - 1].forward_ == nullptr)
    {
        --this->level_;
    }
    --this->length_;
}

template<typename T> T* SkipList<T>::findNode(size_t _score)
{
    if (tail_ && tail_->score_ < _score)
    {
        return nullptr;
    }

    SkiplistNode<T>* x = this->header_;
    for (int i = this->level_ - 1; i >= 0; i--)
    {
        while (x->level_[i].forward_ && x->level_[i].forward_->score_ <= _score)
        {
            x = x->level_[i].forward_;

            if (x->score_ == _score)
            {
                return x->obj_;
            }
        }
    }
    return nullptr;
}
