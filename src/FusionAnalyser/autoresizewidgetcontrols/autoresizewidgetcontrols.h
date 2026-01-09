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

#ifndef AUTORESIZEWIDGETCONTROLS_H
#define AUTORESIZEWIDGETCONTROLS_H

#include <QWidget>
#include <QMap>
#include <QList>

/*
*      ************** *                                                        ***************
*      *                  *                                                        *                 *
*      *   W1 W2     ********RESIZE(W, H)--->RESIZE(w,h)******   w1 w2      *
*      *                  *                                                        *                 *
*      ***************                                                         ***************
*
*    AUTO RESIZE WIDGET CONTROLS TO SUIT THE DIFFERENT RESOLUTION
*       the code designed by liuchuang on 29/11/2019.
*/

	struct AutoResizeOriginalData
	{
		QRect data_rect;
		QFont data_font;
	};

	template <typename T>
	void autoResizeWidgetControls(QList<T*>& _listWidget, QMap<QWidget*, AutoResizeOriginalData>& resizeMap)
	{
		QWidget* item = nullptr;
		AutoResizeOriginalData resizeData;
		QRect tmp;

		for (auto it = _listWidget.begin(); it != _listWidget.end(); it++)
		{
			item = *it;
			tmp = item->geometry();
			tmp.setX(item->x());
			tmp.setY(item->y());
			tmp.setWidth(abs(tmp.width()));
			tmp.setHeight(abs(tmp.height()));
			resizeData.data_rect = tmp;
			resizeData.data_font = item->font();
			resizeMap[item] = resizeData;
		}
	}

	class  AutoResizeWidgetControls
	{
	public:
		typedef enum
		{
			INCLUDE_LABEL = 0x1,
			INCLUDE_BUTTON = 0x2,
			INCLUDE_LINE_EDITOR = 0x4,
			INCLUDE_TEXT_EDITRO = 0x8,
			INCLUDE_COMBOBOX = 0x10,
			INCLUDE_RADIO_BUTTON = 0x20,
			INCLUDE_CHECK_BOX = 0x40,
			INCLUDE_SPIN_BOX = 0x80,
			INCLUDE_DOUBLE_SPIN_BOX = 0x100,
			INCLUDE_LINE = 0x200,
			INCLUDE_ALL_CONTROL = 0xfff

		}AUTORESIZEFLAG;

		AutoResizeWidgetControls(QWidget* obj, float base_width, float base_height);
		~AutoResizeWidgetControls(void);
		void doAutoResize();
		void addOtherItem(QWidget* other);
		void addIgnoreItem(QWidget* ignore);
		void setAutoResizeFlag(int flag);
		void pushAllResizeItem();
		float getHorResizeRatio();
		float getVerResizeRatio();
		float getFontResizeRatio();

	private:
		bool autoResize;
		QMap<QWidget*, AutoResizeOriginalData> resizeMap;
		QMap<QWidget*, AutoResizeOriginalData> fontMap;
		QList<QWidget*> otherItem;
		QList<QWidget*> ignoreItem;
		int autoResizeFlag;
		float horRatio;
		float verRatio;
		float fontRatio;
		float baseWidth;
		float baseHeight;
		QWidget* autoResizeObj;
		void calculateResizeRatio();
		void fontAutoResize(QWidget* obj, int fontSIze);
		void dealLayoutItems();
		void ignoreAllChiledren(QObject* obj);

	};

#endif//AUTORESIZEWIDGETCONTROLS_H

