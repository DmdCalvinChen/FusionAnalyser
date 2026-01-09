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

#ifndef FUSIONMESSAGEBOX_H
#define FUSIONMESSAGEBOX_H

#include <QWidget>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <map>
#include <common_base/SignalManager.h>

class FusionMessageBox : public QDialog
{
    Q_OBJECT

public:
	FusionMessageBox(QWidget* parent = 0, const QString& title = tr("Tip"), const QString& text = "",
		QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::Ok);

	FusionMessageBox(std::map<QMessageBox::StandardButton, QString>& bts, QMessageBox::StandardButton defaultButton, QWidget* parent = 0, const QString& title = tr("Tip"), const QString& text = "");
	FusionMessageBox(const QString& text, QWidget* parent);
	~FusionMessageBox();
	QAbstractButton* clickedButton() const;
	QMessageBox::StandardButton standardButton(QAbstractButton* button) const;
	// Set default button
	void setDefaultButton(QPushButton* button);
	void setDefaultButton(QMessageBox::StandardButton button);
	// Set window title
	void setTitle(const QString& title);
	// Set message text
	void setText(const QString& text);
	// Set window icon
	void setIcon(const QString& icon);
	// Add widget - replace QLabel containing message text
	void addWidget(QWidget* pWidget);
	// Get close button
	QPushButton* getCloseBtn() const { return p_close_btn_; }

	QAbstractButton* getBtn(QMessageBox::StandardButton button);

protected:
	bool isCaption(int x, int y);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void createCloseBtn(QWidget* widget, int  width);
	virtual void showEvent(QShowEvent* event);

protected:
	// Multi-language translation
	void changeEvent(QEvent* event);

private slots:
	void onButtonClicked(QAbstractButton* button);

private:
	void translateUI();
	int execReturnCode(QAbstractButton* button);

private:
	QLabel* p_icon_label_=nullptr;
	QLabel* p_label_=nullptr;
	QGridLayout* p_grid_layout_=nullptr;
	QVBoxLayout* p_lay_out_ = nullptr;
	QDialogButtonBox* p_btn_box_=nullptr;
	QAbstractButton* p_clicked_btn_=nullptr;
	QAbstractButton* p_default_btn_=nullptr;

	QPushButton* p_close_btn_ = nullptr;
	QPoint start_pos_;
	QPoint end_pos_;
	bool b_left_btn_pressed_ = false;
	bool b_is_catpion_ = false;
	QWidget* p_parent_ = nullptr;

};

#endif // FUSIONMESSAGEBOX_H
