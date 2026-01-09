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

#include "autoresizewidgetcontrols.h"

#include <QLabel>
#include <QMapIterator>
#include <QObject>
#include <QDebug>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QRadioButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLayout>

AutoResizeWidgetControls::AutoResizeWidgetControls(QWidget* obj, float base_width, float base_height) :autoResizeObj(obj)
{
	autoResize = false;
	baseWidth = base_width;
	baseHeight = base_height;
	horRatio = 1.0;
	verRatio = 1.0;
	fontRatio = 1.0;
	dealLayoutItems();
}

AutoResizeWidgetControls::~AutoResizeWidgetControls(void)
{
}

void AutoResizeWidgetControls::pushAllResizeItem()
{
	QWidget* item = nullptr;
	AutoResizeOriginalData resizeData;
	QRect tmp;
	//add other items
	for (auto it = otherItem.begin(); it != otherItem.end(); it++)
	{
		//qDebug()<<*it;
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

	if (autoResizeFlag & INCLUDE_LABEL)
	{
		QList<QLabel*> _editorList = autoResizeObj->findChildren<QLabel*>();
		autoResizeWidgetControls(_editorList, resizeMap);
	}

	if (autoResizeFlag & INCLUDE_BUTTON)
	{
		QList<QAbstractButton*> _editorList = autoResizeObj->findChildren<QAbstractButton*>();
		autoResizeWidgetControls(_editorList, resizeMap);
	}

	if (autoResizeFlag & INCLUDE_LINE_EDITOR)
	{
		QList<QLineEdit*> _editorList = autoResizeObj->findChildren<QLineEdit*>();
		autoResizeWidgetControls(_editorList, resizeMap);
	}

	if (autoResizeFlag & INCLUDE_TEXT_EDITRO)
	{
		QList<QTextEdit*> _editorList = autoResizeObj->findChildren<QTextEdit*>();
		autoResizeWidgetControls(_editorList, resizeMap);
	}

	if (autoResizeFlag & INCLUDE_COMBOBOX)
	{
		QList<QComboBox*> _editorList = autoResizeObj->findChildren<QComboBox*>();
		autoResizeWidgetControls(_editorList, resizeMap);
	}

	if (autoResizeFlag & INCLUDE_RADIO_BUTTON)
	{
		QList<QRadioButton*> _editorList = autoResizeObj->findChildren<QRadioButton*>();
		autoResizeWidgetControls(_editorList, resizeMap);
	}

	if (autoResizeFlag & INCLUDE_CHECK_BOX)
	{
		QList<QCheckBox*> _editorList = autoResizeObj->findChildren<QCheckBox*>();
		autoResizeWidgetControls(_editorList, resizeMap);
	}

	if (autoResizeFlag & INCLUDE_SPIN_BOX)
	{
		QList<QSpinBox*> _editorList = autoResizeObj->findChildren<QSpinBox*>();
		autoResizeWidgetControls(_editorList, resizeMap);
	}

	if (autoResizeFlag & INCLUDE_DOUBLE_SPIN_BOX)
	{
		QList<QDoubleSpinBox*> _editorList = autoResizeObj->findChildren<QDoubleSpinBox*>();
		autoResizeWidgetControls(_editorList, resizeMap);
	}

	if (autoResizeFlag & INCLUDE_LINE)
	{
		QList<QFrame*> _editorList = autoResizeObj->findChildren<QFrame*>();
		autoResizeWidgetControls(_editorList, resizeMap);
	}

	for (auto it = ignoreItem.begin(); it != ignoreItem.end(); it++)
	{
		if (resizeMap.contains(*it))
			resizeMap.remove(*it);
	}
	autoResize = true;
}

float AutoResizeWidgetControls::getHorResizeRatio()
{
	return horRatio;
}

float AutoResizeWidgetControls::getVerResizeRatio()
{
	return verRatio;
}

float AutoResizeWidgetControls::getFontResizeRatio()
{
	return fontRatio;
}

void AutoResizeWidgetControls::calculateResizeRatio()
{
	horRatio = autoResizeObj->width() / baseWidth;
	verRatio = autoResizeObj->height() / baseHeight;
	fontRatio = horRatio < verRatio ? horRatio : verRatio;
}

void AutoResizeWidgetControls::fontAutoResize(QWidget* obj, int fontSize)
{
	qDebug() << "set" << obj << fontSize;
	if (fontSize <= 0)
		return;
	bool hasTextStyle = false;
	fontSize *= fontRatio;
	QString fontTextReg = "font:\\s+[0-9]+pt";
	QString fontFormat = "font: %1pt";
	QString fontSizeReg = "[0-9]+";
	QRegExp reg = QRegExp(fontTextReg);
	QRegExp size = QRegExp(fontSizeReg);
	QString styleText = obj->styleSheet();
	QString fontText = "";
	QString fontSizeText = "";
	QFont changedFont;
	if (reg.indexIn(obj->styleSheet()) != -1)
	{
		fontText = reg.capturedTexts().at(0);
		if (size.indexIn(fontText) != -1)
		{
			hasTextStyle = true;
		}
	}
	//set stylesheet to change font size
	if (hasTextStyle)
	{
		styleText.replace(reg, fontFormat.arg(fontSize));
		obj->setStyleSheet(styleText);
	}
	//set font pointsize to change font size
	else
	{
		changedFont = obj->font();
		changedFont.setPointSize(fontSize);
		obj->setFont(changedFont);
	}
}

void AutoResizeWidgetControls::dealLayoutItems()
{
	QString desName = "layoutWidget";
	QList<QLayout*> layoutList = autoResizeObj->findChildren<QLayout*>();
	for (auto it = layoutList.begin(); it != layoutList.end(); it++)
	{
		QString objName = (*it)->parent()->objectName();
		qDebug() << "layout parent->" << objName;
		if (objName.contains(desName))
		{
			qDebug() << "find layoutWidget->" << *it;
			QWidget* layoutWidget = qobject_cast<QWidget*>((*it)->parent());
			otherItem.push_back(layoutWidget);
			ignoreAllChiledren(layoutWidget);
		}
	}
}

void AutoResizeWidgetControls::ignoreAllChiledren(QObject* obj)
{
	QList<QObject*> children = obj->children();
	for (auto it = children.begin(); it != children.end(); it++)
	{
		QWidget* item = qobject_cast<QWidget*>(*it);
		ignoreItem.push_back(item);
		AutoResizeOriginalData resizeData;
		if (!item)
			continue;
		resizeData.data_font = item->font();
		fontMap[item] = resizeData;
	}
}

void AutoResizeWidgetControls::addOtherItem(QWidget* other)
{
	otherItem.push_back(other);
}

void AutoResizeWidgetControls::addIgnoreItem(QWidget* ignore)
{
	ignoreItem.push_back(ignore);
}

void AutoResizeWidgetControls::setAutoResizeFlag(int flag)
{
	autoResizeFlag = flag;
}

void AutoResizeWidgetControls::doAutoResize()
{
	calculateResizeRatio();
	if (autoResize)
	{
		QMapIterator<QWidget*, AutoResizeOriginalData> _itarator(resizeMap);
		QFont changedFont;
		while (_itarator.hasNext())
		{
			_itarator.next();
			QWidget* _item = _itarator.key();

			QRect tmp = _itarator.value().data_rect;
			tmp.setWidth(tmp.width() * horRatio);
			tmp.setHeight(tmp.height() * verRatio);
			QRect after = QRect(tmp.x() * horRatio, tmp.y() * verRatio, tmp.width(), tmp.height());
			//changed font
			changedFont = _itarator.value().data_font;
			fontAutoResize(_item, changedFont.pointSize());
			_item->setGeometry(after);
		}
		QMapIterator<QWidget*, AutoResizeOriginalData> _fontIt(fontMap);
		while (_fontIt.hasNext())
		{
			_fontIt.next();
			QWidget* _item = _fontIt.key();
			changedFont = _fontIt.value().data_font;
			fontAutoResize(_item, changedFont.pointSize());
		}
	}
}

