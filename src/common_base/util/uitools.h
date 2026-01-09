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

#ifndef UITOOLS_H
#define UITOOLS_H

#include <QObject>
#include <QMessageBox>

#include "common_base/common_base_global.h"
class QMainWindow;
class QDialog;
class FusionMessageBox;
class COMMON_BASESHARED_EXPORT UiUtilityTools : public QObject
{
public:
    class Garbo
    {
    public:
        ~Garbo()
        {
            if (UiUtilityTools::p_instance_)
                delete UiUtilityTools::p_instance_;
        }
    };
    static Garbo garbo;
private:
    UiUtilityTools(QObject* parent = nullptr);
    ~UiUtilityTools();
    static UiUtilityTools* p_instance_;

public:
    static UiUtilityTools* getInstance()
    {
        if (p_instance_ == nullptr)
		{
			p_instance_ = new UiUtilityTools;
		}
        return p_instance_;
    }
    FusionMessageBox* showNoButtonMessage(const QString& info);
    void showInfoMessageBox(const QString &title, const QString &info);
    int showQuestionMessageBox(const QString &title, const QString &info, int style=0, QMessageBox::StandardButtons buttons = QMessageBox::StandardButtons(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::StandardButton defaultButton = QMessageBox::StandardButton(QMessageBox::Ok));
    QMessageBox::StandardButton showQuestion(QWidget* parent, const QString& title,
        const QString& text, QMessageBox::StandardButtons buttons= QMessageBox::StandardButtons(QMessageBox::Ok|QMessageBox::Cancel),
        QMessageBox::StandardButton defaultButton=QMessageBox::StandardButton(QMessageBox::Ok));
	int showSaveMessageBox(const QString& title, QString info, std::map<QMessageBox::StandardButton, QString>& bts, QMessageBox::StandardButton defaultButton);

    void setStyleFileSheet(QWidget *pWidet, QString styleSheetFile);
    void setStyleFileSheet(QDialog *pWidet, QString styleSheetFile);

    bool showQuestionMessageBox(const QString& title, const QString& info, const QString& option_first, const QString& option_second);
    bool showQuestionNonMessageBox(const QString& title, const QString& info, const QString& option_first, const QString& option_second, FusionMessageBox*&msg_box);
    QMainWindow *getMainWindow();

    QWidget* p_msg_widget_ = nullptr;

};

#endif // UITOOLS_H
