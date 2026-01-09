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

#include "uitools.h"
#include <QMessageBox>
#include <QFile>
#include <QTextCodec>
#include <QTextStream>
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QDialog>
#include <QVBoxLayout>
#include "fusionMessageBox.h"
#include <QDebug>
UiUtilityTools::Garbo  UiUtilityTools::garbo;
UiUtilityTools* UiUtilityTools::p_instance_ = nullptr;

QString btnStyle("QPushButton{\
	color:white;\
	font: 9pt \"微软雅黑\";\
	background-color:#32AAC9;\
	border:solid;\
	border-radius:6px;\
	min-width:100px;\
	min-height:25px;\
    }\
    .QPushButton:hover\
    {\
        color:white;\
        background-color:#2BACCD;\
    }");

QString btnDarkStyle("QDialog{background-color: #18232D;color:white;}\
    .QPushButton{\
	color:white;\
	font: 9pt \"微软雅黑\";\
	background-color:#343E48;\
	border:solid;\
	border-radius:6px;\
	min-width:100px;\
	min-height:25px;\
    }\
    .QPushButton:hover\
    {\
        color:white;\
        background-color:#304751;\
    }");

FusionMessageBox* UiUtilityTools::showNoButtonMessage(const QString& info)
{
	QMainWindow* pMainWindow = nullptr;
	pMainWindow = getMainWindow();
	FusionMessageBox* msgBox = new FusionMessageBox(info, pMainWindow);
	msgBox->show();
	return msgBox;
}
void UiUtilityTools::showInfoMessageBox(const QString &title, const QString &info)
{
    QMainWindow *pMainWindow = nullptr;
    pMainWindow = getMainWindow();
	FusionMessageBox msgBox(pMainWindow, title, info);
    msgBox.setIcon(":/res/dark/info.png");
    msgBox.exec();
}
QMessageBox::StandardButton UiUtilityTools::showQuestion(QWidget* parent, const QString& title,
	const QString& text, QMessageBox::StandardButtons buttons,
	QMessageBox::StandardButton defaultButton)
{
	FusionMessageBox msgBox(parent, title, text, buttons, defaultButton);
	msgBox.setIcon(":/res/dark/question.png");
	if (msgBox.exec() == -1)
		return QMessageBox::Cancel;
	return msgBox.standardButton(msgBox.clickedButton());
}

int UiUtilityTools::showSaveMessageBox(const QString& title, QString info, std::map<QMessageBox::StandardButton, QString>& bts, QMessageBox::StandardButton defaultButton)
{
	QMainWindow* pMainWindow = nullptr;
	pMainWindow = getMainWindow();
	FusionMessageBox msgBox(bts, defaultButton, pMainWindow,  title, info );
	msgBox.setIcon(":/res/dark/question.png");
	if (msgBox.exec() == -1)
		return -1;
    QMessageBox::StandardButton btn = msgBox.standardButton(msgBox.clickedButton());
	if (btn == QMessageBox::Yes)
	{
		return 1;
	}
	else if (btn == QMessageBox::No)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int UiUtilityTools::showQuestionMessageBox(const QString &title, const QString &info, int style, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
	QMainWindow* pMainWindow = nullptr;
	pMainWindow = getMainWindow();
    QMessageBox::StandardButton ID =  showQuestion(pMainWindow, title, info, buttons, defaultButton);
   if (ID == QMessageBox::Ok || ID == QMessageBox::Yes)
   {
       return 1;
   }
   else if (ID == QMessageBox::Cancel || ID == QMessageBox::No)
   {
       return 0;
   }
   else
   {
       return -1;
   }
}

bool UiUtilityTools::showQuestionMessageBox(const QString& title, const QString& info, const QString& option_first, const QString& option_second)
{
	QMainWindow* pMainWindow = nullptr;
	pMainWindow = getMainWindow();
	std::map<QMessageBox::StandardButton, QString>bts;
	bts[QMessageBox::Yes] = QObject::tr("maxilla");
	bts[QMessageBox::No] = QObject::tr("mandible");
	FusionMessageBox msgBox(bts, QMessageBox::Yes, pMainWindow, title, info);
	msgBox.getCloseBtn()->setEnabled(false);
	msgBox.setIcon(":/res/dark/question.png");
    msgBox.exec();
	return msgBox.standardButton(msgBox.clickedButton()) == QMessageBox::Yes;
}

bool UiUtilityTools::showQuestionNonMessageBox(const QString& title, const QString& info, const QString& option_first, const QString& option_second, FusionMessageBox*&msg_box)
{
	QMainWindow* pMainWindow = nullptr;
	pMainWindow = getMainWindow();
	std::map<QMessageBox::StandardButton, QString>bts;
	bts[QMessageBox::Yes] = QObject::tr("maxilla");
	bts[QMessageBox::No] = QObject::tr("mandible");
	FusionMessageBox* msgBox = new FusionMessageBox(bts, QMessageBox::Yes, pMainWindow, title, info);
	msgBox->setModal(false);
	if (msgBox->getBtn(QMessageBox::Yes) != nullptr)
		connect(msgBox->getBtn(QMessageBox::Yes), &QAbstractButton::clicked, msgBox, &FusionMessageBox::accepted);
	if (msgBox->getBtn(QMessageBox::No) != nullptr)
		connect(msgBox->getBtn(QMessageBox::No), &QAbstractButton::clicked, msgBox, &FusionMessageBox::rejected);
	msgBox->getCloseBtn()->setEnabled(false);
	msgBox->setIcon(":/res/dark/question.png");
	msgBox->setAttribute(Qt::WA_DeleteOnClose);
	msgBox->show();
	msg_box = msgBox;
	return msgBox->standardButton(msgBox->clickedButton()) == QMessageBox::Yes;
}
void UiUtilityTools::setStyleFileSheet(QWidget *pWidget, QString styleSheetFile)
{
    QFile file(styleSheetFile);
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream((QIODevice*)&file);
        stream.setCodec(QTextCodec::codecForName("UTF-8"));
        QString qssStyle = stream.readAll();
        pWidget->setStyleSheet(qssStyle);
        file.close();
    }

}

void UiUtilityTools::setStyleFileSheet(QDialog *pWidet, QString styleSheetFile)
{
    QFile file(styleSheetFile);
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream((QIODevice*)&file);
        stream.setCodec(QTextCodec::codecForName("UTF-8"));
        QString qssStyle = stream.readAll();
        pWidet->setStyleSheet(qssStyle);
        file.close();
    }
}

QMainWindow * UiUtilityTools::getMainWindow()
{
    QMainWindow *pMainWindow = nullptr;
    foreach(QWidget *w, qApp->topLevelWidgets())
    {
        if (QMainWindow* mainWindow = qobject_cast<QMainWindow*>(w))
        {
            pMainWindow = mainWindow;
            break;
        }
    }
    return pMainWindow;
}

UiUtilityTools::UiUtilityTools(QObject* parent)
    : QObject(parent)
{
}

UiUtilityTools::~UiUtilityTools()
{
}
