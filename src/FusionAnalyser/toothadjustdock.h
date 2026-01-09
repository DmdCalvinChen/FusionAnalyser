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

#ifndef TOOTHADJUSTDOCK_H
#define TOOTHADJUSTDOCK_H

#include <QWidget>
#include <QDockWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>

namespace Ui {
class ToothAdjustDock;
}

class ToothAdjustDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit ToothAdjustDock(QWidget *parent = 0);
    ~ToothAdjustDock();

private:
    Ui::ToothAdjustDock *ui;
	float x_Value, y_Value, z_Value, a_Value, b_Value, g_Value;

signals:

	public slots :
		void updateToothHexValue();

public:
	QLabel *title_Label;
	QLabel *x_Translate_Label, *y_Translate_Label, *z_Translate_Label, *a_Rotate_Label, *b_Rotate_Label, *g_Rotate_Label;
	QTextEdit *x_Translate_Edit, *y_Translate_Edit, *z_Translate_Edit;
	QTextEdit *a_Rotate_Edit, *b_Rotate_Edit, *g_Rotate_Edit;
	QPushButton *translate_Mode_Btn, *rotate_Mode_Btn;
};

#endif // TOOTHADJUSTDOCK_H
