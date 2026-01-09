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

#include "pdfreadergui.h"

#include <QDesktopWidget>
#include <QScreen>

#include <common/config.h>
#include <common_base/SignalManager.h>
#include <common_ext/data/dentalanalysisdata.h>
#include <common_ext/data/fusionaligndata.h>

#include "analysisgroup/PdfReaderGui/PdfReader/pdfreader.h"
#include "analysisgroup/analysisreport/analysisreport.h"
#include "ui_pdfreadergui.h"

#define CLOSEBTN_WIDTH 20
#define CLOSEBTN_HEIGHT 20
PdfReaderGui::PdfReaderGui(QWidget* parent)
    : FusionFramelessBaseDlg(parent), ui(new Ui::PdfReaderGui)
{
    ui->setupUi(this);
    int max_char = 8;
    if (PFusionAppData->getAppLanguage() == E_CHINESE)
    {
        max_char = 8;
    }
    else if (PFusionAppData->getAppLanguage() == E_ENGLISH)
    {
        max_char = 16;
    }

#if PLATFORM_WINDOWS
    HDC desk = GetDC(NULL);
    float hDPI = GetDeviceCaps(desk, LOGPIXELSX);
    float vDPI = GetDeviceCaps(desk, LOGPIXELSY);
    int dpi = (hDPI + vDPI) / 2;
#else
    float dpi = 150;
#endif

    QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect deskRect = desktopWidget->availableGeometry();
    QRect screenRect = desktopWidget->screenGeometry();
    ui->patient_name_lineEdit->setMaxLength(max_char);
    ui->doctor_name_lineEdit->setMaxLength(max_char);

    pdf_reader_ = new PdfReader(this);
    ui->verticalLayout_2->addWidget(pdf_reader_);

    pdf_data_ = new AnalysisReport(this);
    pdf_data_->setEnabeldLocalVersion(true);
    pdf_data_->setVisible(false);

    ui->date_value_lab->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd"));

    ui->export_toolButton->setIcon(QIcon(":res/default/images/pdf_preview/export.png"));
    ui->export_toolButton->setLayoutDirection(Qt::RightToLeft);

    ui->copy_text_toolButton->setIcon(QIcon(":/res/default/images/pdf_preview/copy.png"));
    ui->copy_text_toolButton->setLayoutDirection(Qt::RightToLeft);

    ui->cancel_toolButton->setIcon(QIcon(":res/default/images/pdf_preview/quit.png"));
    ui->cancel_toolButton->setLayoutDirection(Qt::RightToLeft);
    ui->cancel_toolButton->setAttribute(Qt::WA_TranslucentBackground);

    connect(pdf_data_, &AnalysisReport::updateDrawelements, pdf_reader_,
            &PdfReader::updateDrawElements);
    connect(ui->export_toolButton, &QToolButton::clicked, pdf_data_,
            &AnalysisReport::exportToPdfSlot);
    connect(this, &PdfReaderGui::selectBoxChangedSignal, pdf_data_,
            &AnalysisReport::selectBoxChangedSlot);

    connect(ui->patient_name_lineEdit, &QLineEdit::textChanged, this,
            [&]()
            {
                if (PFusionAlignData->analysisData() && pdf_data_)
                {
                    auto& info_patient = PFusionAlignData->getAnalyserData();
                    info_patient.patient_data_.patient_name_ = ui->patient_name_lineEdit->text();
                    PFusionAlignData->analysisData()->setPatientName(
                            ui->patient_name_lineEdit->text());
                    pdf_data_->updateGuiDataSlot();
                }
            });

    connect(ui->doctor_name_lineEdit, &QLineEdit::textChanged, this,
            [&]()
            {
                if (PFusionAlignData->analysisData() && pdf_data_)
                {
                    auto& info_patient = PFusionAlignData->getAnalyserData();
                    info_patient.patient_data_.doctors_ = ui->doctor_name_lineEdit->text();
                    PFusionAlignData->analysisData()->setDoctorName(
                            ui->doctor_name_lineEdit->text());
                    pdf_data_->updateGuiDataSlot();
                }
            });

    connect(ui->note_textEdit, &QTextEdit::textChanged, this,
            [&]()
            {
                QString textContent = ui->note_textEdit->toPlainText();
                int length = textContent.count();
                int maxLength = 300;    // 最大字符数
                if (length > maxLength)
                {
                    int position = ui->note_textEdit->textCursor().position();
                    QTextCursor textCursor = ui->note_textEdit->textCursor();
                    textContent.remove(position - (length - maxLength), length - maxLength);
                    ui->note_textEdit->setText(textContent);
                    textCursor.setPosition(position - (length - maxLength));
                    ui->note_textEdit->setTextCursor(textCursor);
                }

                if (PFusionAlignData->analysisData() && pdf_data_)
                {
                    PFusionAlignData->analysisData()->setRemark(ui->note_textEdit->toPlainText());
                    pdf_data_->updateGuiDataSlot();
                }
            });

    connect(ui->copy_text_toolButton, &QToolButton::clicked, this,
            [&]()
            {
                auto it = PFusionAlignData->analysisData();
                if (it)
                {
                    it->copyToClipboard();
                }
            });

    connect(ui->cancel_toolButton, &QToolButton::clicked, this, [&]() { closeEvent(nullptr); });

    connect(PSIGNALMANAGER, &SignalManager::updateAvaliableItemActionsSignal, this,
            &PdfReaderGui::updateAnalyzerAvaliableItemActionsSlot);
    connect(PSIGNALMANAGER, &SignalManager::updateDentalAnalysisDataReportSignal, this,
            [&]() {});

    connect(PSIGNALMANAGER, &SignalManager::updateBrowserItemSignal, this,
            &PdfReaderGui::updateBrowserItemSlot);

    createSelectBoxs();

    createCloseBtn(this, this->width());
    qDebug() << dpi;
    qDebug() << deskRect.height();
    if (dpi > 170 || deskRect.height() < 1000)
    {
        ui->note_textEdit->setFixedHeight(160);
        this->setFixedHeight(691);
        p_close_btn_->setFixedSize(15, 15);
    }
    else if ((deskRect.height() >= 1100 && dpi < 180 && dpi >= 150) ||
             (deskRect.height() < 1100 && deskRect.height() >= 1000 && dpi >= 140 && dpi < 180))
    {
        this->setFixedHeight(900);
        this->setFixedWidth(1600);
        ui->horizontalLayout->setContentsMargins(220, 0, 0, 0);
    }
    else if (deskRect.height() >= 1100 && dpi < 150 && dpi > 125)
    {
        this->setFixedHeight(820);
        this->setFixedWidth(1400);
        ui->horizontalLayout->setContentsMargins(85, 0, 0, 0);
    }
    else
    {
        this->setFixedHeight(820);
        this->setFixedWidth(1400);
        ui->horizontalLayout->setContentsMargins(120, 0, 0, 0);
    }

    ui->textEdit->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    ui->note_textEdit->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    ui->patient_name_lineEdit->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    ui->doctor_name_lineEdit->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
}

PdfReaderGui::~PdfReaderGui()
{
    delete ui;
}

void PdfReaderGui::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    p_close_btn_->setGeometry(this->width() - p_close_btn_->width() - 8, 8, CLOSEBTN_WIDTH,
                              CLOSEBTN_HEIGHT);

    return;
}
void PdfReaderGui::closeEvent(QCloseEvent* event)
{
    this->hide();
    emit closeGuiSignal();
}

void PdfReaderGui::createSelectBoxs()
{
    auto addCheckBoxFunc = [&](QString text)
    {
        QCheckBox* box = new QCheckBox(this);
        box->setText(text);
        box->setProperty("id", text);
        return box;
    };

    all_select_box_ = addCheckBoxFunc(tr("Select All") /*QString::fromLocal8Bit("全选")*/);

    select_item_boxs_.push_back(addCheckBoxFunc(ITEM_MISSING_TOOTH));
    select_item_boxs_.push_back(addCheckBoxFunc(ITEM_TOOTH_WIDTH));
    select_item_boxs_.push_back(addCheckBoxFunc(ITEM_CROWDING));

    select_item_boxs_.push_back(addCheckBoxFunc(ITEM_BOLTON_RATIO));
    select_item_boxs_.push_back(addCheckBoxFunc(ITEM_SPEE));
    select_item_boxs_.push_back(addCheckBoxFunc(ITEM_MOLAR_REL_RATIO));

    select_item_boxs_.push_back(addCheckBoxFunc(ITEM_CENTERLINE_REL));
    select_item_boxs_.push_back(addCheckBoxFunc(ITEM_ARCH_WIDTH));
    select_item_boxs_.push_back(addCheckBoxFunc(ITEM_ARCH_LENGTH));

    select_item_boxs_.push_back(addCheckBoxFunc(ITEM_PALATAL_HEIGHT));
    select_item_boxs_.push_back(addCheckBoxFunc(ITEM_BASAL_BONE));
    select_item_boxs_.push_back(addCheckBoxFunc(ITEM_OJ_OB));

    ui->all_select_verticalLayout_5->addWidget(all_select_box_);

    for (int i = 0; i < select_item_boxs_.size(); i += 3)
    {
        int row = i / 3;
        for (int column = 0; column < 3; ++column)
        {
            int index = i + column;
            if (index < select_item_boxs_.size())
            {
                auto box = select_item_boxs_[index];

                if (box)
                {
                    QLabel* suffix_lab = new QLabel(this);
                    suffix_lab->setText("*");
                    suffix_lab->setStyleSheet("color:rgb(255,0,0);");
                    select_item_boxs_suffix_.insert(
                            std::make_pair(box->property("id").toString(), suffix_lab));

                    QHBoxLayout* pLayout = new QHBoxLayout();    // 水平布局
                    pLayout->setSpacing(2);
                    pLayout->addWidget(box);
                    pLayout->addWidget(suffix_lab);
                    ui->checkbox_gridLayout->addLayout(pLayout, row, column);
                }
            }
        }
    }

    connect(all_select_box_, &QCheckBox::stateChanged, this,
            [&]
            {
                emit selectBoxChangedSignal(all_select_box_->property("id").toString(),
                                            all_select_box_->isChecked());

                auto state = all_select_box_->checkState();
                for (auto& it : select_item_boxs_)
                {
                    if (it->isEnabled())
                    {
                        it->setCheckState(state);
                    }
                }
            });

    for (auto& box : select_item_boxs_)
    {
        connect(box, &QCheckBox::stateChanged, this,
                [&]
                {
                    emit selectBoxChangedSignal(box->property("id").toString(), box->isChecked());

                    // 首先屏蔽所有选中状态按钮的信号
                    all_select_box_->blockSignals(true);

                    bool all_selected_status = true;
                    for (auto& each_check_box : select_item_boxs_)
                    {
                        if (Qt::CheckState::Checked != each_check_box->checkState())
                        {
                            all_selected_status = false;
                            break;
                        }
                    }

                    // 非全部选中、或全手动选中，则修改全选状态逻辑
                    all_select_box_->setCheckState(all_selected_status ? Qt::Checked
                                                                       : Qt::Unchecked);

                    // 恢复所有选中状态按钮的信号
                    all_select_box_->blockSignals(false);
                });
    }
}

void PdfReaderGui::initCheckBoxsAllSelect()
{
    bool all_select = true;

    if (all_select_box_)
    {
        all_select_box_->setChecked(all_select);
    }
}

void PdfReaderGui::initCheckBoxsStateFromProject()
{
    // 读取工程数据中的bool状态
    auto data = PFusionAlignData->analysisData();

    if (data)
    {
        for (auto& item : data->getEachItemConfirmState())
        {
            for (auto& each_check_box : select_item_boxs_)
            {
                if (each_check_box->property("id").toString() == item.first)
                {
                    each_check_box->setCheckState(item.second ? Qt::Checked : Qt::Unchecked);
                    break;
                }
            }
        }
    }

    ui->note_textEdit->setText(PFusionAlignData->analysisData()->getMark());
    QString name_patient = PFusionAlignData->analysisData()->getPatientName();
    name_patient = name_patient.isEmpty() ? PFusionAlignData->getShortProjectName() : name_patient;
    ui->patient_name_lineEdit->setText(name_patient);
    ui->doctor_name_lineEdit->setText(PFusionAlignData->analysisData()->getDoctorName());

    updateBrowserItemSlot();
}

void PdfReaderGui::setItemBrowsedState(QString item, bool is_confirm)
{
    if (select_item_boxs_suffix_.find(item) != select_item_boxs_suffix_.end())
    {
        select_item_boxs_suffix_[item]->setVisible(!is_confirm);
    }
}

void PdfReaderGui::updateBrowserItemSlot()
{
    // 初始化确认*项的label显示全部
    for (auto& it : select_item_boxs_suffix_)
    {
        it.second->setVisible(true);
    }

    // 默认选项不显示确认
    setItemBrowsedState(ITEM_MISSING_TOOTH, true);
    setItemBrowsedState(ITEM_TOOTH_WIDTH, true);

    auto data = PFusionAlignData->analysisData();
    if (data)
    {
        for (auto& it : data->browsed_item_prompt_)
        {
            QString text = QObject::tr(it.toStdString().c_str());
            setItemBrowsedState(text, true);
        }
    }
}

void PdfReaderGui::updateAnalyzerAvaliableItemActionsSlot(
        std::vector<EditItemIndex> _avaliable_item_indexes)
{
    emit PSIGNALMANAGER->updateDentalAnalysisDataReportSignal();

    for (auto& each_check_box : select_item_boxs_)
    {
        each_check_box->setEnabled(false);
    }

    for (auto& item_index : _avaliable_item_indexes)
    {
        switch (item_index)
        {
            case MISSING_TOOTH:
                for (auto& each_check_box : select_item_boxs_)
                {
                    if (each_check_box->property("id").toString() == ITEM_MISSING_TOOTH)
                    {
                        each_check_box->setEnabled(true);
                        break;
                    }
                }
                break;

            case TEETH_WIDTH:
                for (auto& each_check_box : select_item_boxs_)
                {
                    if (each_check_box->property("id").toString() == ITEM_TOOTH_WIDTH)
                    {
                        each_check_box->setEnabled(true);
                        break;
                    }
                }
                break;

            case CUR_LENGTH_OF_DENTAL_ARCH:
                for (auto& each_check_box : select_item_boxs_)
                {
                    if (each_check_box->property("id").toString() == ITEM_CROWDING)
                    {
                        each_check_box->setEnabled(true);
                        break;
                    }
                }
                break;

            case BOLTON_COMPLETELY:
                for (auto& each_check_box : select_item_boxs_)
                {
                    if (each_check_box->property("id").toString() == ITEM_BOLTON_RATIO)
                    {
                        each_check_box->setEnabled(true);
                        break;
                    }
                }
                break;

            case SPEE_CURVE_DEPTH:
                for (auto& each_check_box : select_item_boxs_)
                {
                    if (each_check_box->property("id").toString() == ITEM_SPEE)
                    {
                        each_check_box->setEnabled(true);
                        break;
                    }
                }
                break;

            case MOLAR_RELATIONSHIP:
                for (auto& each_check_box : select_item_boxs_)
                {
                    if (each_check_box->property("id").toString() == ITEM_MOLAR_REL_RATIO)
                    {
                        each_check_box->setEnabled(true);
                        break;
                    }
                }
                break;

            case MIDLINE_RELATIONSHIP:
                for (auto& each_check_box : select_item_boxs_)
                {
                    if (each_check_box->property("id").toString() == ITEM_CENTERLINE_REL)
                    {
                        each_check_box->setEnabled(true);
                        break;
                    }
                }
                break;

            case ARCH_WIDTH:
                for (auto& each_check_box : select_item_boxs_)
                {
                    if (each_check_box->property("id").toString() == ITEM_ARCH_WIDTH)
                    {
                        each_check_box->setEnabled(true);
                        break;
                    }
                }
                break;

            case ARCH_LENGTH:
                for (auto& each_check_box : select_item_boxs_)
                {
                    if (each_check_box->property("id").toString() == ITEM_ARCH_LENGTH)
                    {
                        each_check_box->setEnabled(true);
                        break;
                    }
                }
                break;

            case GNATHOTECTUM_HEIGHT:
                for (auto& each_check_box : select_item_boxs_)
                {
                    if (each_check_box->property("id").toString() == ITEM_PALATAL_HEIGHT)
                    {
                        each_check_box->setEnabled(true);
                        break;
                    }
                }
                break;

            case BASAL_BONE_ARCH:
                for (auto& each_check_box : select_item_boxs_)
                {
                    if (each_check_box->property("id").toString() == ITEM_BASAL_BONE)
                    {
                        each_check_box->setEnabled(true);
                        break;
                    }
                }
                break;

            case OVERLAY_ANALYSIS:
                for (auto& each_check_box : select_item_boxs_)
                {
                    if (each_check_box->property("id").toString() == ITEM_OJ_OB)
                    {
                        each_check_box->setEnabled(true);
                        break;
                    }
                }
                break;

            default:
                break;
        }
    }

    bool have_disabled = false;
    for (auto& it : select_item_boxs_)
    {
        have_disabled |= !it->isEnabled();

        if (!it->isEnabled())
        {
            it->setChecked(false);
        }

        pdf_data_->selectBoxEnabledSlot(it->property("id").toString(), it->isEnabled());
    }

    if (have_disabled)
    {
        all_select_box_->blockSignals(true);
        all_select_box_->setChecked(false);
        all_select_box_->blockSignals(false);
    }

    all_select_box_->setEnabled(!have_disabled);

    emit PSIGNALMANAGER->updateDentalAnalysisDataReportSignal();
    return;
}
