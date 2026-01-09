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

#ifndef BASEATTRIBUTEDEFAULTGUI_H
#define BASEATTRIBUTEDEFAULTGUI_H
#include <QFont>
#include <QPalette>
#include <QApplication>
#include <QScreen>

class BaseAttributeDefaultGui
{
public:
	BaseAttributeDefaultGui()
	{
		spilterLineStyle = ".QFrame{ border:2px solid #0b84e6;}";

		colorMainTitle.setColor(QPalette::WindowText, Qt::white);

		colorErrorInfoTitle.setColor(QPalette::WindowText, Qt::red);

		QString familyFont =
			QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221");

		auto dpi = qApp->primaryScreen()->logicalDotsPerInch() * 1.0f / 96;

		if (dpi > 1.0f)
		{
			scaleResolutionDpi /= dpi;
		}
		else
		{
			scaleResolutionDpi *= dpi;
		}

		if ( dpi > 0.74f && dpi < 0.873)
		{
			scaleResolutionDpi = 0.6f;
		}

		float fontSize = 24.0f;
		fontSize *= scaleResolutionDpi;

		QFont _fontMainTitle;
		_fontMainTitle.setFamily(familyFont);
		_fontMainTitle.setPixelSize(fontSize);

		fontSize = 20.0f;
		fontSize *= scaleResolutionDpi;
		QFont _fontMiddleTitle;
		_fontMiddleTitle.setFamily(familyFont);
		_fontMiddleTitle.setPixelSize(fontSize);

		fontSize = 18.0f;
		fontSize *= scaleResolutionDpi;
		QFont _fontSubTitle;
		_fontSubTitle.setFamily(familyFont);
		_fontSubTitle.setPixelSize(fontSize);

		fontSize = 16.0f;
		fontSize *= scaleResolutionDpi;
		QFont _fontTableContentTitle;
		_fontTableContentTitle.setFamily(familyFont);
		_fontTableContentTitle.setPixelSize(fontSize);

		fontMainTitle = _fontMainTitle;
		fontMiddleTitle = _fontMiddleTitle;
		fontSubTitle = _fontSubTitle;
		fontTableContentTitle = _fontTableContentTitle;
	}

	virtual ~BaseAttributeDefaultGui()
	{

	}

private:
	float scaleResolutionDpi = 1.0f;
	QString titleAdditional;
	QString spilterLineStyle;
	QPalette colorMainTitle;
	QPalette colorErrorInfoTitle;
	QFont fontMainTitle;
	QFont fontMiddleTitle;
	QFont fontSubTitle;
	QFont fontTableContentTitle;
	int layoutSpace = 8;
	int width_window = 300;
	int height_widow = 600;

public:
	void setWidthWindow(int width)
	{
		this->width_window = width;
	}

	void setHeightWindow(int height)
	{
		this->height_widow = height;
	}

	int getWidthWindow()const
	{
		return width_window;
	}

	int getHeightWindow()const
	{
		return height_widow;
	}

	void setTitleAdditional(const QString& _titleAdditional)
	{
		this->titleAdditional = _titleAdditional;
	}

	void setSpilterLineStyle(const QString& _style)
	{
		spilterLineStyle = _style;
	}

	void setLayoutSpace(int _layoutSpace)
	{
		this->layoutSpace = _layoutSpace;
	}

	int getLayoutSpace()const
	{
		return layoutSpace;
	}

	void setColorMainTitle(const QPalette& pen)
	{
		colorMainTitle = pen;
	}

	void setColorErrorText(const QPalette& pen)
	{
		colorErrorInfoTitle = pen;
	}

	QString getTitleAdditional()const
	{
		return this->titleAdditional;
	}

	QPalette getColorMainTitle()const
	{
		return colorMainTitle;
	}

	QPalette getColorErrorText()const
	{
		return colorErrorInfoTitle;
	}

	void setFontTitle(const QFont& _fontMainTitle, const QFont& _fontMiddleTitle, const QFont& _fontSubTitle)
	{
		fontMainTitle = _fontMainTitle;
		fontMiddleTitle = _fontMiddleTitle;
		fontSubTitle = _fontSubTitle;
	}

	void setFontTableContentTitle(const QFont& _fontTableContentTitle)
	{
		fontTableContentTitle = _fontTableContentTitle;
	}

	QString getSpilterLineStyle()const
	{
		return spilterLineStyle;
	}

	QFont getFontMainTitle()const
	{
		return fontMainTitle;
	}

	QFont getFontMiddleTitle()const
	{
		return fontMiddleTitle;
	}

	QFont getFontSubTitle()const
	{
		return fontSubTitle;
	}

	QFont getFontTableContentTitle()const
	{
		return fontTableContentTitle;
	}

	float getCurrentScreenScaleResolutionDpi()const
	{
		return this->scaleResolutionDpi;
	}

};
#endif
