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

#include "fusionMessageBox.h"
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QEvent>
#include <QApplication>
#include <QMouseEvent>
#define CLOSEBTN_WIDTH 20
#define CLOSEBTN_HEIGHT 20

QString btnStyleSheet("QPushButton{background-color:#36404c; color:white; }\
                                  .QPushButton:pressed{background-color:#36404c;}\
									.QPushButton:hover{background-color:#5e6976;}");

FusionMessageBox::FusionMessageBox(QWidget* parent, const QString& title, const QString& text,
	QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
	: QDialog(parent)
{
	p_parent_ = parent;
	this->setObjectName("fusionMsgBox");
	setWindowTitle(title);
	setMinimumSize(300, 130);
	setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);

	p_btn_box_ = new QDialogButtonBox(this);
	p_btn_box_->setStandardButtons(QDialogButtonBox::StandardButtons(int(buttons)));
	p_btn_box_->setStyleSheet(btnStyleSheet);
	setDefaultButton(defaultButton);

	QPushButton* pYesButton = p_btn_box_->button(QDialogButtonBox::Yes);
	if (pYesButton != NULL)
	{
		pYesButton->setObjectName("blueButton");
		pYesButton->setStyle(QApplication::style());
	}

	p_icon_label_ = new QLabel(this);
	p_label_ = new QLabel(this);

	QPixmap pixmap(":/Images/information");
	p_icon_label_->setPixmap(pixmap);
	p_icon_label_->setFixedSize(35, 35);
	p_icon_label_->setScaledContents(true);

	p_label_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	p_label_->setObjectName("whiteLabel");
	p_label_->setOpenExternalLinks(true);
	p_label_->setText(text);
	p_label_->adjustSize();

	p_grid_layout_ = new QGridLayout();
	p_grid_layout_->addWidget(p_icon_label_, 0, 0, 2, 1, Qt::AlignTop);
	p_grid_layout_->addWidget(p_label_, 0, 1, 2, 1);
	p_grid_layout_->addWidget(p_btn_box_, p_grid_layout_->rowCount(), 0, 1, p_grid_layout_->columnCount());
	p_grid_layout_->setSizeConstraint(QLayout::SetNoConstraint);
	p_grid_layout_->setHorizontalSpacing(10);
	p_grid_layout_->setVerticalSpacing(10);
	p_grid_layout_->setContentsMargins(10, 10, 10, 10);
	p_lay_out_ = new QVBoxLayout();
	p_lay_out_->addLayout(p_grid_layout_);
	this->setLayout(p_lay_out_);
	createCloseBtn(this, this->width());

	translateUI();

	connect(p_btn_box_, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButtonClicked(QAbstractButton*)));
}

FusionMessageBox::FusionMessageBox(const QString& text, QWidget* parent)
	: QDialog(parent)
{
	p_parent_ = parent;
	this->setObjectName("fusionMsgBox");
	setMinimumSize(300, 130);
	setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);

	p_icon_label_ = new QLabel(this);
	p_label_ = new QLabel(this);

	QPixmap pixmap(":/Images/information");
	p_icon_label_->setPixmap(pixmap);
	p_icon_label_->setFixedSize(35, 35);
	p_icon_label_->setScaledContents(true);

	p_label_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	p_label_->setObjectName("whiteLabel");
	p_label_->setOpenExternalLinks(true);
	p_label_->setText(text);
	p_label_->adjustSize();

	p_grid_layout_ = new QGridLayout();
	p_grid_layout_->addWidget(p_icon_label_, 0, 0, 2, 1, Qt::AlignTop);
	p_grid_layout_->addWidget(p_label_, 0, 1, 2, 1);
	p_grid_layout_->addWidget(p_btn_box_, p_grid_layout_->rowCount(), 0, 1, p_grid_layout_->columnCount());
	p_grid_layout_->setSizeConstraint(QLayout::SetNoConstraint);
	p_grid_layout_->setHorizontalSpacing(10);
	p_grid_layout_->setVerticalSpacing(10);
	p_grid_layout_->setContentsMargins(10, 10, 10, 10);
	p_lay_out_ = new QVBoxLayout();
	p_lay_out_->addLayout(p_grid_layout_);
	this->setLayout(p_lay_out_);
}
FusionMessageBox::FusionMessageBox(std::map<QMessageBox::StandardButton, QString>&bts, QMessageBox::StandardButton defaultButton,QWidget* parent /*= 0*/, const QString& title /*= tr("Tip")*/, const QString& text /*= ""*/)
	:QDialog(parent)
{
	p_parent_ = parent;
	this->setObjectName("fusionMsgBox");
	setWindowTitle(title);
	setMinimumSize(300, 130);
	setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);

	p_btn_box_ = new QDialogButtonBox(this);
	QMessageBox::StandardButtons buttons;
	for (auto &btn : bts)
	{
		buttons |= btn.first;
	}
	p_btn_box_->setStandardButtons(QDialogButtonBox::StandardButtons(int(buttons)));

	for (auto &btn : bts)
	{
		QPushButton *pBtn = p_btn_box_->button(QDialogButtonBox::StandardButton(int(btn.first)));
		pBtn->setText(btn.second);
		pBtn->setStyleSheet(btnStyleSheet);
	}
	setDefaultButton(defaultButton);

	QPushButton* pYesButton = p_btn_box_->button(QDialogButtonBox::Yes);
	if (pYesButton != NULL)
	{
		pYesButton->setObjectName("blueButton");
		pYesButton->setStyle(QApplication::style());
	}

	p_icon_label_ = new QLabel(this);
	p_label_ = new QLabel(this);

	QPixmap pixmap(":/Images/information");
	p_icon_label_->setPixmap(pixmap);
	p_icon_label_->setFixedSize(35, 35);
	p_icon_label_->setScaledContents(true);

	p_label_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	p_label_->setObjectName("whiteLabel");
	p_label_->setOpenExternalLinks(true);
	p_label_->setText(text);
	p_label_->adjustSize();

	p_grid_layout_ = new QGridLayout();
	p_grid_layout_->addWidget(p_icon_label_, 0, 0, 2, 1, Qt::AlignTop);
	p_grid_layout_->addWidget(p_label_, 0, 1, 2, 1);
	p_grid_layout_->addWidget(p_btn_box_, p_grid_layout_->rowCount(), 0, 1, p_grid_layout_->columnCount());
	p_grid_layout_->setSizeConstraint(QLayout::SetNoConstraint);
	p_grid_layout_->setHorizontalSpacing(10);
	p_grid_layout_->setVerticalSpacing(10);
	p_grid_layout_->setContentsMargins(10, 10, 10, 10);
	p_lay_out_ = new QVBoxLayout();
	p_lay_out_->addLayout(p_grid_layout_);
	this->setLayout(p_lay_out_);
	createCloseBtn(this, this->width());

	connect(p_btn_box_, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButtonClicked(QAbstractButton*)));
}

FusionMessageBox::~FusionMessageBox()
{

}

bool FusionMessageBox::isCaption(int x, int y)
{
	QRect rect = this->rect();
	QRect titlebarRect = QRect(rect.x(), rect.y(), rect.width(), 30);
	if (titlebarRect.contains(x, y))
	{
		return true;
	}
	return false;
}

void FusionMessageBox::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		b_left_btn_pressed_ = true;
		start_pos_ = event->globalPos();

		b_is_catpion_ = isCaption(event->x(), event->y());
	}
}

void FusionMessageBox::mouseMoveEvent(QMouseEvent* event)
{
	if (b_left_btn_pressed_)
	{
		if (b_is_catpion_)
		{
			int  cursor = event->globalY();
			this->move(this->geometry().topLeft() +
				event->globalPos() - start_pos_);
			start_pos_ = event->globalPos();
		}
	}

}

void FusionMessageBox::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		b_left_btn_pressed_ = true;
		start_pos_ = event->globalPos();

		b_is_catpion_ = isCaption(event->x(), event->y());
	}
}

void FusionMessageBox::createCloseBtn(QWidget* widget, int width)
{
	p_close_btn_ = new QPushButton(this);
	p_close_btn_->setAutoFillBackground(true);
	p_close_btn_->setObjectName("closeBtn");
	p_close_btn_->setAttribute(Qt::WA_TranslucentBackground);
	p_close_btn_->setFlat(true);
	p_close_btn_->setFixedSize(CLOSEBTN_WIDTH, CLOSEBTN_HEIGHT);
	connect(p_close_btn_, &QPushButton::clicked, [&] { this->close(); });
}

void FusionMessageBox::showEvent(QShowEvent* event)
{
	QDialog::showEvent(event);
	if(p_parent_)
		this->move(p_parent_->mapToGlobal(QPoint(0, 0)) + QPoint((p_parent_->width() - this->width()) / 2, (p_parent_->height() - this->height()) / 2));
	if (p_close_btn_)
	{
		p_close_btn_->setGeometry(this->width() - p_close_btn_->width() - 8, 8, CLOSEBTN_WIDTH, CLOSEBTN_HEIGHT);
	}

	return;
}

void FusionMessageBox::changeEvent(QEvent* event)
{
	switch (event->type())
	{
	case QEvent::LanguageChange:
		translateUI();
		break;
	default:
		QWidget::changeEvent(event);
	}
}

void FusionMessageBox::translateUI()
{
	QPushButton* pYesButton = p_btn_box_->button(QDialogButtonBox::Yes);
	if (pYesButton != NULL)
		pYesButton->setText(tr("Yes"));

	QPushButton* pNoButton = p_btn_box_->button(QDialogButtonBox::No);
	if (pNoButton != NULL)
		pNoButton->setText(tr("No"));

	QPushButton* pOkButton = p_btn_box_->button(QDialogButtonBox::Ok);
	if (pOkButton != NULL)
		pOkButton->setText(tr("Ok"));

	QPushButton* pCancelButton = p_btn_box_->button(QDialogButtonBox::Cancel);
	if (pCancelButton != NULL)
		pCancelButton->setText(tr("Cancel"));
}

QMessageBox::StandardButton FusionMessageBox::standardButton(QAbstractButton* button) const
{
	return (QMessageBox::StandardButton)p_btn_box_->standardButton(button);
}

QAbstractButton* FusionMessageBox::clickedButton() const
{
	return p_clicked_btn_;
}

int FusionMessageBox::execReturnCode(QAbstractButton* button)
{
	int nResult = p_btn_box_->standardButton(button);
	return nResult;
}

void FusionMessageBox::onButtonClicked(QAbstractButton* button)
{
	p_clicked_btn_ = button;
	done(execReturnCode(button));
}

void FusionMessageBox::setDefaultButton(QPushButton* button)
{
	if (!p_btn_box_->buttons().contains(button))
		return;
	p_default_btn_ = button;
	button->setDefault(true);
	button->setFocus();
}

void FusionMessageBox::setDefaultButton(QMessageBox::StandardButton button)
{
	setDefaultButton(p_btn_box_->button(QDialogButtonBox::StandardButton(button)));
}

void FusionMessageBox::setTitle(const QString& title)
{
	setWindowTitle(title);
}

void FusionMessageBox::setText(const QString& text)
{
	p_label_->setText(text);
}

void FusionMessageBox::setIcon(const QString& icon)
{
	p_icon_label_->setPixmap(QPixmap(icon));
}

void FusionMessageBox::addWidget(QWidget* pWidget)
{
	p_label_->hide();
	p_grid_layout_->addWidget(pWidget, 0, 1, 2, 1);
}

QAbstractButton* FusionMessageBox::getBtn(QMessageBox::StandardButton button)
{
	QPushButton* pBtn = p_btn_box_->button(QDialogButtonBox::StandardButton( button));;

	return pBtn;
}

