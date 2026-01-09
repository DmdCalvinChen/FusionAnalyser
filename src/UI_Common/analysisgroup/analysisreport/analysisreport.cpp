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

#include "analysisreport.h"

#include <QDesktopWidget>
#include <QFileDialog>
#include <QPainter>
#include <QPixmap>
#include <QPrinter>

#include <common_ext/data/dentalanalysisdata.h>
#include <common_ext/data/fusionaligndata.h>
#include <common_ext/fileManager/filemanager.h>

#include "./ui_analysisreport.h"
#include "treatmentpreviewguigroup/treatmentoverviewgui/imagearea.h"
#include "treatmentpreviewguigroup/treatmentoverviewgui/linearea.h"
#include "treatmentpreviewguigroup/treatmentoverviewgui/tablearea.h"
#include "treatmentpreviewguigroup/treatmentoverviewgui/textarea.h"

AnalysisReport::AnalysisReport(QWidget* parent) : QWidget(parent), ui(new Ui::AnalysisReport)
{
    ui->setupUi(this);

    QFont font;
    font.setFamily("Microsoft YaHei");
    font.setPixelSize(22);
    font.setBold(true);

    setFontMainTitle(font);

    font.setPixelSize(19);
    font.setBold(true);
    setFontSubTitle(font);

    font.setPixelSize(19);
    font.setBold(false);
    setFontTableSubTitle(font);

    setColorBackground(QColor(255, 255, 255));
    setColorMainTitle(QColor(255, 255, 255, 255));
    setColorSubTitle(QColor(0, 0, 0, 255));

    connect(PSIGNALMANAGER, &SignalManager::updateDentalAnalysisDataReportSignal, this,
            &AnalysisReport::updateGuiDataSlot);
    connect(PSIGNALMANAGER, &SignalManager::exportReportInfoPdf, this,
            &AnalysisReport::exportToPdfSlot);
}

AnalysisReport::~AnalysisReport()
{
    clearDrawElements();
    delete ui;
}

void AnalysisReport::paintEvent(QPaintEvent* event)
{
    drawBackground();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (auto it : draw_elements_)
    {
        it->draw(painter);
    }
}

void AnalysisReport::resizeEvent(QResizeEvent* event)
{
    update();
    updateGuiDataSlot();
    updateHeightItems();
}

void AnalysisReport::drawBackground()
{
    QPainter painter(this);
    painter.endNativePainting();
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    // draw background rectangle
    painter.setBrush(QBrush(color_background_, Qt::SolidPattern));
    painter.drawRect(-1, -1, this->width() + 2, this->height() + 2);

    painter.restore();
}

void AnalysisReport::createDrawElements(int width_draw_area, int height_draw_area)
{
    width_draw_area_ = width_draw_area;
    clearDrawElements();

    int start_x = start_x_main_title_;
    int start_y = start_y_title_;

    placeTopButtons(start_x, start_y, width_draw_area, height_draw_area);

    fillPatientInfo(start_x, start_y, width_draw_area, height_draw_area);
    fillToothFdiExistedInfo(start_x, start_y, width_draw_area, height_draw_area);
    fillReportInfo(start_x, start_y, width_draw_area, height_draw_area);
    fillReportRemainedInfo(start_x, start_y, width_draw_area, height_draw_area);

    fillImageInfo(start_x, start_y, width_draw_area, height_draw_area);

    connectSelectBoxsSignal();
    height_text_area_ = start_y;

    updateHeightItems();
}

void AnalysisReport::createDrawElementsForPdf(int width_draw_area, int height_draw_area)
{
    width_draw_area_ = width_draw_area;
    export_pdf_status_ = true;

    clearDrawElements();

    int start_x = start_x_main_title_ * 3;
    int start_y = start_y_title_;

    /*第一项:患者基本信息*/
    fillPatientInfo(start_x, start_y, width_draw_area, height_draw_area);
    /*第二项：牙位信息*/
    fillToothFdiExistedInfo(start_x, start_y, width_draw_area, height_draw_area);
    /*第三项：拥挤度、boltofiln...*/
    fillReportInfo(start_x, start_y, width_draw_area, height_draw_area);

    fillReportRemainedInfo(start_x, start_y, width_draw_area, height_draw_area);
    fillImageInfo(start_x, start_y, width_draw_area, height_draw_area);
    export_pdf_status_ = false;
}

void AnalysisReport::placeTopButtons(int start_x, int& start_y, int width_draw_area,
                                     int height_draw_area)
{
    auto it = PFusionAlignData->analysisData();

    if (!it)
    {
        return;
    }

    {
        if (!all_select_box_)
        {
            all_select_box_ = new QCheckBox(this);
            all_select_box_->setText(tr("Select All"));
            all_select_box_->setFont(font_sub_title_);
            connect(all_select_box_, &QCheckBox::stateChanged, this,
                    [&]()
                    {
                        auto state = all_select_box_->checkState();
                        for (auto& it : select_item_boxs_)
                        {
                            if (it.second->isEnabled())
                            {
                                it.second->setCheckState(state);
                            }
                        }
                    });
        }
        start_x = 0;
        start_x += blank_x_check_box_;
        all_select_box_->setGeometry(start_x, start_y, 60, 25);
        all_select_box_->show();

        if (!copy_btn_)
        {
            copy_btn_ = new QPushButton(this);
            copy_btn_->setObjectName("copyBtn");
            copy_btn_->setToolTip(tr("Copy to clipboard"));
            connect(copy_btn_, &QPushButton::pressed, this,
                    [&]()
                    {
                        auto it = PFusionAlignData->analysisData();
                        if (!it)
                        {
                            return;
                        }
                        it->copyToClipboard();
                    });
        }

        start_x = width_draw_area * 0.8;
        copy_btn_->setGeometry(start_x, start_y, 25, 25);
        copy_btn_->show();

        if (!export_btn_)
        {
            export_btn_ = new QPushButton(this);
            export_btn_->setObjectName("exportBtn");
            export_btn_->setToolTip(tr("Export PDF"));
            connect(export_btn_, &QPushButton::pressed, this, &AnalysisReport::exportToPdfSlot);
        }

        start_x += 40;
        export_btn_->setGeometry(start_x, start_y, 25, 25);
        export_btn_->show();
    }
}

void AnalysisReport::fillPatientInfo(int start_x, int& start_y, int width_draw_area,
                                     int height_draw_area)
{
    auto it = PFusionAlignData->analysisData();

    if (!it)
    {
        return;
    }

    std::vector<TextArea*> main_title_vec;
    std::vector<TextArea*> sub_title_vec;

    int record_start_x = start_x;
    start_y += height_H3_title_;
    {
        int x = start_x - start_x_main_title_ * 2;
        LineArea* rect = new LineArea(this, x, start_y, width_draw_area - x * 2, 50);
        rect->setColor(QColor(83, 99, 113));
        rect->setWireMode(false);
        draw_elements_.push_back(rect);

        int width_image = 255;
        start_x = width_draw_area * 0.5 - width_image * 0.7;

        ImageArea* image = new ImageArea(this, ":/res/default/images/titleBar/FusionAnalyser.png",
                                         start_x, start_y, 0, 0);
        draw_elements_.push_back(image);

        start_y += height_H3_title_;
        main_title_vec.push_back(
                new TextArea(tr("Analysis Report"), start_x + width_image * 0.87, start_y + 10));
    }

    {
        auto info_patient = PFusionAlignData->getAnalyserData();

        auto getLengthString = [&](QString text)
        {
            QFontMetrics fm(font_sub_title_);
            return fm.width(text);
        };

        if (is_local_version)
        {
            start_x = record_start_x;
            // start_x += blank_x_sub_title_;
            start_y += height_H1_title_ + height_H3_title_;

            int blank = width_draw_area / 2.3;

            int space = 8;

            QString string = tr("Patient Name");
            int start_first_x = start_x;
            int start_first_sub_x = start_first_x + getLengthString(string) + space;
            sub_title_vec.push_back(new TextArea(string, start_first_x, start_y));
            sub_title_vec.push_back(new TextArea(info_patient.patient_data_.patient_name_,
                                                 start_first_sub_x, start_y));

            string = tr("Date of visit");
            int start_second_x = start_first_sub_x + blank;
            int start_second_sub_x = start_second_x + getLengthString(string) + space;
            sub_title_vec.push_back(new TextArea(string, start_second_x, start_y));
            sub_title_vec.push_back(
                    new TextArea(QDateTime::currentDateTime().toString("yyyy-MM-dd"),
                                 start_second_sub_x, start_y));

            string = tr("Orthodontist");
            int start_third_x = start_x;
            int start_third_sub_x = start_third_x + getLengthString(string) + space;
            start_y += height_H1_title_;
            sub_title_vec.push_back(new TextArea(string, start_third_x, start_y));
            sub_title_vec.push_back(
                    new TextArea(info_patient.patient_data_.doctors_, start_third_sub_x, start_y));

            start_y += height_H1_title_ * 0.2;
        }
        else
        {
            start_x -= blank_x_sub_title_ * 7;
            start_y += height_H1_title_ + height_H3_title_;

            int start_first_x = start_x;
            int start_first_sub_x = start_first_x + blank_x_ * 1.4;

            int start_second_x = start_first_sub_x + blank_x_ * 1.2;
            int start_second_sub_x = start_second_x + blank_x_ * 1.0;

            int start_third_x = start_second_sub_x + blank_x_;
            int start_third_sub_x = start_third_x + blank_x_ * 1.5;

            sub_title_vec.push_back(
                    new TextArea(tr("Patient Name"), start_first_x, start_y));
            sub_title_vec.push_back(new TextArea(info_patient.patient_data_.patient_name_,
                                                 start_first_sub_x, start_y));

            sub_title_vec.push_back(
                    new TextArea(tr("Patient ID"), start_second_x, start_y));
            sub_title_vec.push_back(new TextArea(info_patient.patient_data_.patient_id_,
                                                 start_second_sub_x, start_y));

            start_y += height_H2_title_;

            sub_title_vec.push_back(
                    new TextArea(tr("Physician"), start_first_x, start_y));
            sub_title_vec.push_back(
                    new TextArea(info_patient.patient_data_.doctors_, start_first_sub_x, start_y));

            sub_title_vec.push_back(
                    new TextArea(tr("Date"), start_second_x, start_y));
            sub_title_vec.push_back(
                    new TextArea(QDateTime::currentDateTime().toString("yyyy-MM-dd"),
                                 start_second_sub_x, start_y));
        }

    }

    start_y += height_H1_title_ + height_H3_title_;

    for (auto& it : main_title_vec)
    {
        it->setColorText(color_main_title_);
        it->setFont(font_main_title_);
    }

    for (auto& it : sub_title_vec)
    {
        it->setColorText(color_sub_title_);
        it->setFont(font_sub_title_);
    }

    draw_elements_.insert(draw_elements_.end(), main_title_vec.begin(), main_title_vec.end());
    draw_elements_.insert(draw_elements_.end(), sub_title_vec.begin(), sub_title_vec.end());
}

void AnalysisReport::fillToothFdiExistedInfo(int start_x, int& start_y, int width_draw_area,
                                             int height_draw_area)
{
    auto it = PFusionAlignData->analysisData();

    if (!it)
    {
        return;
    }

    {
        start_y -= height_H1_title_;

        if (PFusionAlignData->getAnalyserData().getIsBmu())
        {
            if (note_text_edit_ == nullptr)
            {
                note_text_edit_ = new QTextEdit(this);
                note_text_edit_->setPlaceholderText(tr("Add Notes"));
                note_text_edit_->setStyleSheet(
                        "QTextEdit{ color: black; background-color: rgb(255,255,255); border:1px "
                        "solid rgb(169,169,169); }");
                connect(note_text_edit_, &QTextEdit::textChanged, this,
                        [&]()
                        {
                            auto it = PFusionAlignData->analysisData();
                            if (!it)
                            {
                                return;
                            }
                            it->setRemark(note_text_edit_->toPlainText());
                        });
            }

            note_text_edit_->setText(it->remark_);
            note_text_edit_->setGeometry(start_x, start_y, width_draw_area * 0.9, 60);
            note_text_edit_->show();
        }
    }

    int height_note_remain = 100;
    if (export_pdf_status_)
    {
        int height_text = font_sub_title_.pixelSize() > 0 ? font_sub_title_.pixelSize() + 2 : 16;
        QString content_note; /* = note_text_edit_->toPlainText();*/

        if (PFusionAlignData->analysisData())
        {
            content_note = PFusionAlignData->analysisData()->getMark();
        }

        auto getLengthString = [&](QString text)
        {
            QFontMetrics fm(font_sub_title_);
            return fm.width(text);
        };

        int length_note = content_note.size();

        auto segment_string = content_note.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);

        int length_segment = width_draw_area - 58 - 6 * 2;

        auto getSubString = [&](QString text)
        {
            int length_note = text.size();
            std::vector<QString> string_segments;
            QString segment;
            bool is_last_including = false;
            for (int index_start = 0; index_start < length_note; ++index_start)
            {
                segment.push_back(text.at(index_start));
                if (getLengthString(segment) >= length_segment)
                {
                    QString str_new;
                    if (getLengthString(segment) > length_segment)
                    {
                        str_new = segment.back();
                        segment.chop(1);
                    }
                    string_segments.push_back(segment);
                    segment = str_new;

                    if (index_start >= length_note)
                    {
                        is_last_including = true;
                    }
                }
            }
            if (!is_last_including && !segment.isEmpty())
            {
                string_segments.push_back(segment);
            }
            return string_segments;
        };

        std::vector<QString> string_segments;
        for (auto& s : segment_string)
        {
            auto string_vec = getSubString(s);
            for (auto& ss : string_vec)
            {
                string_segments.push_back(ss);
            }
        }

        start_y += height_text;
        makeMainTitle(start_x, start_y, tr("Notes"));

        start_y += height_text;
        int record_y = start_y;

        for (auto& string : string_segments)
        {
            start_y += height_text;
            makeMainTitle(start_x + 6, start_y, string, &font_table_sub_);
        }

        start_y = std::max(record_y + height_note_remain, start_y);

        LineArea* rect =
                new LineArea(this, start_x, record_y, width_draw_area - 58, start_y - record_y + 4);
        rect->setColor(QColor(126, 138, 149));
        draw_elements_.push_back(rect);

        start_y += height_text * 1.5;
    }
    else
    {
        start_y += height_note_remain;
    }

    QString unit = QString("mm");

    {
        std::pair<QString, QString> desrc = std::make_pair(getItemId(), ITEM_MISSING_TOOTH);
        std::vector<QString> fdi_lost;
        for (auto& fdi : it->missing_tooth_upper_)
        {
            fdi_lost.push_back(QString::number(fdi));
        }

        for (auto& fdi : it->missing_tooth_lower_)
        {
            fdi_lost.push_back(QString::number(fdi));
        }
        std::vector<FormColumnData> columns_data;
        columns_data.push_back(FormColumnData(tr("Tooth Missing"), fdi_lost));

        make_item_func(start_x, start_y, desrc.first, desrc.second, columns_data);
    }

    {
        auto getFdiGroupFunc = [](int start_fdi)
        {
            std::vector<QString> fdi_vec;
            for (int i = 0; i < 8; ++i)
            {
                fdi_vec.push_back(QString::number(start_fdi + i));
            }
            return fdi_vec;
        };

        {
            std::pair<QString, QString> desrc = std::make_pair(getItemId(), ITEM_TOOTH_WIDTH);
            std::vector<FormColumnData> columns_data;

            columns_data.push_back(FormColumnData(tr("FDI"), {getFdiGroupFunc(11)}));
            columns_data.push_back(FormColumnData(tr("Width"), {it->tooth_width_list_right_upper}));

            columns_data.push_back(FormColumnData(tr("FDI"), {getFdiGroupFunc(21)}));
            columns_data.push_back(FormColumnData(tr("Width"), {it->tooth_width_list_left_upper}));

            std::vector<FormColumnData> columns_second_data;

            columns_second_data.push_back(FormColumnData(tr("FDI"), {getFdiGroupFunc(41)}));
            columns_second_data.push_back(
                    FormColumnData(tr("Width"), {it->tooth_width_list_right_lower}));

            columns_second_data.push_back(FormColumnData(tr("FDI"), {getFdiGroupFunc(31)}));
            columns_second_data.push_back(
                    FormColumnData(tr("Width"), {it->tooth_width_list_left_lower}));

            if (PFusionAlignData->upper_dental_features_ &&
                PFusionAlignData->lower_dental_features_)
            {
                make_item_func(start_x, start_y, desrc.first, desrc.second, columns_data,
                               &columns_second_data);
            }
            else if (PFusionAlignData->upper_dental_features_)
            {
                make_item_func(start_x, start_y, desrc.first, desrc.second, columns_data);
            }
            else if (PFusionAlignData->lower_dental_features_)
            {
                make_item_func(start_x, start_y, desrc.first, desrc.second, columns_second_data);
            }
        }
    }
}

void AnalysisReport::fillReportInfo(int start_x, int& start_y, int width_draw_area,
                                    int height_draw_area)
{
    auto it = PFusionAlignData->analysisData();

    if (!it)
    {
        return;
    }

    QString unit = QString("mm");
    {
        std::pair<QString, QString> desrc = std::make_pair(getItemId(), ITEM_CROWDING);
        std::vector<FormColumnData> columns_data;

        if (PFusionAlignData->upper_dental_features_ && PFusionAlignData->lower_dental_features_)
        {
            columns_data.push_back(FormColumnData((""), {tr("Maxilla"), tr("Mandibular")}));
            columns_data.push_back(FormColumnData(
                    tr("Deserved arch length"),
                    {it->space_requared_upper_ + unit, it->space_requared_lower_ + unit}));
            columns_data.push_back(FormColumnData(
                    tr("Existing arch length"),
                    {it->cur_length_arch_upper_ + unit, it->cur_length_arch_lower_ + unit}));
            columns_data.push_back(FormColumnData(
                    tr("Crowding"), {it->crowding_severity_upper_, it->crowding_severity_lower_}));
        }
        else if (PFusionAlignData->upper_dental_features_)
        {
            columns_data.push_back(FormColumnData((""), {tr("Maxilla")}));
            columns_data.push_back(
                    FormColumnData(tr("Deserved arch length"), {it->space_requared_upper_ + unit}));
            columns_data.push_back(FormColumnData(tr("Existing arch length"),
                                                  {it->cur_length_arch_upper_ + unit}));
            columns_data.push_back(FormColumnData(tr("Crowding"), {it->crowding_severity_upper_}));
        }
        else if (PFusionAlignData->lower_dental_features_)
        {
            columns_data.push_back(FormColumnData((""), {tr("Mandibular")}));
            columns_data.push_back(
                    FormColumnData(tr("Deserved arch length"), {it->space_requared_lower_ + unit}));
            columns_data.push_back(FormColumnData(tr("Existing arch length"),
                                                  {it->cur_length_arch_lower_ + unit}));
            columns_data.push_back(FormColumnData(tr("Crowding"), {it->crowding_severity_lower_}));
        }

        make_item_func(start_x, start_y, desrc.first, desrc.second, columns_data);
    }

    {
        std::pair<QString, QString> desrc = std::make_pair(getItemId(), ITEM_BOLTON_RATIO);
        std::vector<FormColumnData> columns_data;
        columns_data.push_back(FormColumnData(
                tr("Bolton 3-3"), {it->bolton_anterior_}));
        columns_data.push_back(FormColumnData(it->bolton_anterior_larger_describe_,
                                              {it->bolton_anterior_larger_value_ + unit}));
        columns_data.push_back(
                FormColumnData(tr("Bolton 6-6"),
                               {it->bolton_completely_}));
        columns_data.push_back(FormColumnData(it->bolton_completely_larger_describe_,
                                              {it->bolton_completely_larger_value_ + unit}));

        make_item_func(start_x, start_y, desrc.first, desrc.second, columns_data);
    }

    {
        std::pair<QString, QString> desrc = std::make_pair(getItemId(), ITEM_SPEE);
        std::vector<FormColumnData> columns_data;
        columns_data.push_back(
                FormColumnData(tr("Right"), {QString::number(it->right_spee_deepth_) + unit}));
        columns_data.push_back(
                FormColumnData(tr("Left"), {QString::number(it->left_spee_depth_) + unit}));
        columns_data.push_back(FormColumnData(tr("Space Needed"), {it->needed_crevice_ + unit}));
        make_item_func(start_x, start_y, desrc.first, desrc.second, columns_data);
    }
}

void AnalysisReport::fillReportRemainedInfo(int start_x, int& start_y, int width_draw_area,
                                            int height_draw_area)
{
    auto it = PFusionAlignData->analysisData();

    if (!it)
    {
        return;
    }

    QString unit = QString("mm");

    {
        std::pair<QString, QString> desrc = std::make_pair(getItemId(), ITEM_MOLAR_REL_RATIO);
        std::vector<FormColumnData> columns_data;
        columns_data.push_back(FormColumnData(tr("Right"),
                                              {it->right_molar_relationship_}));
        columns_data.push_back(FormColumnData(tr("Left"),
                                              {it->left_molar_relationship_}));
        columns_data.push_back(FormColumnData(tr("Both"),
                                              {it->bilateral_molar_relationship_}));
        make_item_func(start_x, start_y, desrc.first, desrc.second, columns_data);
    }

    {
        std::pair<QString, QString> desrc = std::make_pair(getItemId(), ITEM_CENTERLINE_REL);
        std::vector<FormColumnData> columns_data;
        columns_data.push_back(FormColumnData(
                tr("Centerline"), {tr("Center line Deviation between maxillary and mandible") +
                                   QString(" ") + it->midline_relationship_}));
        make_item_func(start_x, start_y, desrc.first, desrc.second, columns_data);
    }

    {
        std::pair<QString, QString> desrc = std::make_pair(getItemId(), ITEM_ARCH_WIDTH);
        std::vector<FormColumnData> columns_data;

        if (PFusionAlignData->upper_dental_features_ && PFusionAlignData->lower_dental_features_)
        {
            columns_data.push_back(FormColumnData((""), {tr("Maxilla"), tr("Mandibular")}));
            columns_data.push_back(FormColumnData(tr("Anterior  part"),
                                                  {getGoodData(it->arch_width_front_upper_),
                                                   getGoodData(it->arch_width_front_lower_)}));
            columns_data.push_back(
                    FormColumnData(tr("Middle part"), {getGoodData(it->arch_width_middle_upper_),
                                                       getGoodData(it->arch_width_middle_lower_)}));
            columns_data.push_back(FormColumnData(tr("Posterior part"),
                                                  {getGoodData(it->arch_width_rear_upper_),
                                                   getGoodData(it->arch_width_rear_lower_)}));
        }
        else if (PFusionAlignData->upper_dental_features_)
        {
            columns_data.push_back(FormColumnData((""), {tr("Maxilla")}));
            columns_data.push_back(FormColumnData(tr("Anterior  part"),
                                                  {getGoodData(it->arch_width_front_upper_)}));
            columns_data.push_back(
                    FormColumnData(tr("Middle part"), {getGoodData(it->arch_width_middle_upper_)}));
            columns_data.push_back(FormColumnData(tr("Posterior part"),
                                                  {getGoodData(it->arch_width_rear_upper_)}));
        }
        else if (PFusionAlignData->lower_dental_features_)
        {
            columns_data.push_back(FormColumnData((""), {tr("Mandibular")}));
            columns_data.push_back(FormColumnData(tr("Anterior  part"),
                                                  {getGoodData(it->arch_width_front_lower_)}));
            columns_data.push_back(
                    FormColumnData(tr("Middle part"), {getGoodData(it->arch_width_middle_lower_)}));
            columns_data.push_back(FormColumnData(tr("Posterior part"),
                                                  {getGoodData(it->arch_width_rear_lower_)}));
        }

        make_item_func(start_x, start_y, desrc.first, desrc.second, columns_data);
    }

    {
        std::pair<QString, QString> desrc = std::make_pair(getItemId(), ITEM_ARCH_LENGTH);
        std::vector<FormColumnData> columns_data;

        if (PFusionAlignData->upper_dental_features_ && PFusionAlignData->lower_dental_features_)
        {
            columns_data.push_back(
                    FormColumnData("", {tr("Maxilla"), tr("Mandibular")}));
            columns_data.push_back(
                    FormColumnData(tr("Anterior  part"), {getGoodData(it->arch_len_front_upper_),
                                                          getGoodData(it->arch_len_front_lower_)}));
            columns_data.push_back(
                    FormColumnData(tr("Middle part"), {getGoodData(it->arch_len_middle_upper_),
                                                       getGoodData(it->arch_len_middle_lower_)}));
            columns_data.push_back(
                    FormColumnData(tr("Posterior part"), {getGoodData(it->arch_len_rear_upper_),
                                                          getGoodData(it->arch_len_rear_lower_)}));
            columns_data.push_back(
                    FormColumnData(tr("Total Length"), {getGoodData(it->arch_len_whole_upper_),
                                                        getGoodData(it->arch_len_whole_lower_)}));
        }
        else if (PFusionAlignData->upper_dental_features_)
        {
            columns_data.push_back(FormColumnData("", {tr("Maxilla")}));
            columns_data.push_back(
                    FormColumnData(tr("Anterior  part"), {getGoodData(it->arch_len_front_upper_)}));
            columns_data.push_back(
                    FormColumnData(tr("Middle part"), {getGoodData(it->arch_len_middle_upper_)}));
            columns_data.push_back(
                    FormColumnData(tr("Posterior part"), {getGoodData(it->arch_len_rear_upper_)}));
            columns_data.push_back(
                    FormColumnData(tr("Total Length"), {getGoodData(it->arch_len_whole_upper_)}));
        }
        else if (PFusionAlignData->lower_dental_features_)
        {
            columns_data.push_back(FormColumnData("", {tr("Mandibular")}));
            columns_data.push_back(
                    FormColumnData(tr("Anterior  part"), {getGoodData(it->arch_len_front_lower_)}));
            columns_data.push_back(
                    FormColumnData(tr("Middle part"), {getGoodData(it->arch_len_middle_lower_)}));
            columns_data.push_back(
                    FormColumnData(tr("Posterior part"), {getGoodData(it->arch_len_rear_lower_)}));
            columns_data.push_back(
                    FormColumnData(tr("Total Length"), {getGoodData(it->arch_len_whole_lower_)}));
        }
        make_item_func(start_x, start_y, desrc.first, desrc.second, columns_data);
    }

    {
        std::pair<QString, QString> desrc = std::make_pair(getItemId(), ITEM_BASAL_BONE);
        std::vector<FormColumnData> columns_data;

        if (PFusionAlignData->upper_dental_features_ && PFusionAlignData->lower_dental_features_)
        {
            columns_data.push_back(
                    FormColumnData("", {tr("Maxilla"), tr("Mandibular")}));
            columns_data.push_back(FormColumnData(tr("Basal bone width"),
                                                  {getGoodData(it->arch_width_basalbone_upper_),
                                                   getGoodData(it->arch_width_basalbone_lower_)}));
            columns_data.push_back(FormColumnData(tr("Basal bone length"),
                                                  {getGoodData(it->arch_len_basalbone_upper_),
                                                   getGoodData(it->arch_len_basalbone_lower_)}));
        }
        else if (PFusionAlignData->upper_dental_features_)
        {
            columns_data.push_back(FormColumnData("", {tr("Maxilla")}));
            columns_data.push_back(FormColumnData(tr("Basal bone width"),
                                                  {getGoodData(it->arch_width_basalbone_upper_)}));
            columns_data.push_back(FormColumnData(tr("Basal bone length"),
                                                  {getGoodData(it->arch_len_basalbone_upper_)}));
        }
        else if (PFusionAlignData->lower_dental_features_)
        {
            columns_data.push_back(FormColumnData("", {tr("Mandibular")}));
            columns_data.push_back(FormColumnData(tr("Basal bone width"),
                                                  {getGoodData(it->arch_width_basalbone_lower_)}));
            columns_data.push_back(FormColumnData(tr("Basal bone length"),
                                                  {getGoodData(it->arch_len_basalbone_lower_)}));
        }
        make_item_func(start_x, start_y, desrc.first, desrc.second, columns_data);
    }

    {
        std::pair<QString, QString> desrc = std::make_pair(getItemId(), ITEM_PALATAL_HEIGHT);
        std::vector<FormColumnData> columns_data;
        columns_data.push_back(FormColumnData(tr("Height of palatal vault"),
                                              {getGoodData(it->gnathotectum_height_value_upper_)}));
        make_item_func(start_x, start_y, desrc.first, desrc.second, columns_data);
    }

    if (is_local_version)
    {
        std::pair<QString, QString> desrc = std::make_pair(getItemId(), ITEM_OJ_OB);
        std::vector<FormColumnData> columns_data;
        columns_data.push_back(FormColumnData(
                tr("overbite"), {getGoodData(it->overbite_value_) +
                                 (getGoodData(it->overbite_value_) == "/"
                                          ? ""
                                          : ("(" + it->overbite_rank_translated_ + ")"))}));

        columns_data.push_back(FormColumnData(
                tr("overjet"), {getGoodData(it->overjet_value_) +
                                (getGoodData(it->overjet_value_) == "/"
                                         ? ""
                                         : ("(" + it->overjet_rank_translated_ + ")"))}));

        make_item_func(start_x, start_y, desrc.first, desrc.second, columns_data);
    }
}

void AnalysisReport::fillImageInfo(int start_x, int& start_y, int width_draw_area,
                                   int height_draw_area)
{
    int height_iamge = 100;
    auto make_image = [&](QString& image_path)
    {
        if (image_path.isEmpty())
        {
            return;
        }

        start_x += blank_x_sub_title_;
        start_y += height_H3_title_ / 2;

        ImageArea* image = new ImageArea(this, image_path, start_x, start_y, width_draw_area * 0.95,
                                         height_iamge);
        draw_elements_.push_back(image);

        start_y += height_H1_title_ + height_iamge;
    };
}

void AnalysisReport::loadItemsStatusAndInitSelectBoxStatus()
{
    auto data = PFusionAlignData->analysisData();

    if (data)
    {
        initSelectBoxsCheckedStatus(data->getEachItemConfirmState());
    }
}

void AnalysisReport::connectSelectBoxsSignal()
{
    if (!connect_once && !select_item_boxs_.empty())
    {
        for (auto& it : select_item_boxs_)
        {
            connect(it.second, &QCheckBox::stateChanged, this, [&]() { updateElementsPreview(); });

            connect(it.second, &QCheckBox::stateChanged, this,
                    [&]()
                    {
                        auto data = PFusionAlignData->analysisData();

                        if (data)
                        {
                            data->setConfirmState(it.first, it.second->checkState() == Qt::Checked);
                        }

                        if (all_select_box_)
                        {
                            all_select_box_->blockSignals(true);

                            bool all_selected_status = true;
                            for (auto& each_check_box : select_item_boxs_)
                            {
                                if (Qt::CheckState::Checked != each_check_box.second->checkState())
                                {
                                    all_selected_status = false;
                                    break;
                                }
                            }

                            all_select_box_->setCheckState(all_selected_status ? Qt::Checked
                                                                               : Qt::Unchecked);

                            all_select_box_->blockSignals(false);
                        }
                    });
        }

        connect_once = true;
    }
}

void AnalysisReport::initSelectBoxsCheckedStatus(
        std::vector<std::pair<QString, bool>> items_description)
{
    for (auto& item : items_description)
    {
        if (select_item_boxs_.find(item.first) != select_item_boxs_.end())
        {
            if (select_item_boxs_[item.first]->isEnabled())
            {
                select_item_boxs_[item.first]->setCheckState(item.second ? Qt::Checked
                                                                         : Qt::Unchecked);
            }
        }
    }
    update();
}

void AnalysisReport::makeMainTitle(int start_x, int& start_y, const QString& _main_title,
                                   QFont* font)
{
    start_x += 0;
    start_y += 0;
    TextArea* main_title = new TextArea(_main_title, start_x, start_y);
    main_title->setColorText(color_sub_title_);
    if (font == nullptr)
    {
        main_title->setFont(font_sub_title_);
    }
    else
    {
        main_title->setFont(*font);
    }

    draw_elements_.push_back(main_title);
}

void AnalysisReport::makeMainTitleSelectBox(int start_x, int& start_y, const QString& prefix,
                                            const QString& name_table)
{
    if (select_item_boxs_.find(name_table) != select_item_boxs_.end())
    {
    }
    else
    {
        select_item_boxs_[name_table] = new QCheckBox(this);
        select_item_boxs_[name_table]->setText(prefix + name_table);
        select_item_boxs_[name_table]->setFont(font_sub_title_);
    }
    start_x = 0;
    start_x += blank_x_check_box_;
    select_item_boxs_[name_table]->setGeometry(start_x, start_y - blank_y_ * 3, width_check_box,
                                               height_check_box);
    select_item_boxs_[name_table]->show();
}

void AnalysisReport::makeTableIncludingMainTitle(int start_x, int& start_y, int width_draw_area,
                                                 const QString& _table_name,
                                                 const std::vector<FormColumnData>& _columns_data)
{
    if (_columns_data.empty())
    {
        return;
    }

    int font_size = font_table_sub_.pixelSize();
    int height_table = _columns_data.front().getNumRow() * (font_size + 6);

    // start_x += blank_x_sub_title_;
    start_y += height_H3_title_ / 2;

    TableArea* table = new TableArea(this, start_x, start_y,
                                     width_draw_area - start_x_main_title_ * 4, height_table);
    table->setData(_columns_data);
    table->setColorText(color_sub_title_);
    table->setColorTitleRectangle(QColor(135, 145, 155));
    table->setFont(font_table_sub_);
    table->setPropertyName(_table_name);

    draw_elements_.push_back(table);

    start_y += height_H1_title_ + height_table;
}

template <typename T>
QString AnalysisReport::getGoodData(T data, QString unit)
{
    if (data < 0)
    {
        return "/";
    }
    else
    {
        return QString::number(data) + unit;
    }
}

template <>
QString AnalysisReport::getGoodData(QString data, QString unit)
{
    QString value = (QString)(data);
    if (value.isEmpty() || data.toFloat() < 0)
    {
        return "/";
    }
    else
    {
        return value + unit;
    }
}

void AnalysisReport::make_item_func(
        int start_x, int& start_y, QString& order_prefix, QString& name_table,
        std::vector<FormColumnData>& columns_data,
        std::vector<FormColumnData>* columns_second_data /* = nullptr */)
{
    if (!export_pdf_status_)
    {
        makeMainTitleSelectBox(start_x, start_y, order_prefix, name_table);
        makeTableIncludingMainTitle(start_x, start_y, width_draw_area_, name_table, columns_data);
        if (columns_second_data)
        {
            start_y -= height_H2_title_;
            makeTableIncludingMainTitle(start_x, start_y, width_draw_area_, "",
                                        *columns_second_data);
        }
    }
    else
    {
        if (select_item_boxs_.find(name_table) != select_item_boxs_.end())
        {
            if (select_item_boxs_[name_table]->isChecked() &&
                select_item_boxs_[name_table]->isEnabled())
            {
                makeMainTitle(start_x, start_y, order_prefix + name_table);
                makeTableIncludingMainTitle(start_x, start_y, width_draw_area_, name_table,
                                            columns_data);
                if (columns_second_data)
                {
                    start_y -= height_H2_title_;
                    makeTableIncludingMainTitle(start_x, start_y, width_draw_area_, "",
                                                *columns_second_data);
                }
            }
        }
    }
}

void AnalysisReport::clearDrawElements()
{
    for (auto& it : draw_elements_)
    {
        if (it)
        {
            delete it;
            it = nullptr;
        }
    }
    std::vector<PrimitiveBase*>().swap(draw_elements_);

    setClearItemId();
}

void AnalysisReport::updateHeightItems()
{
    int width_win_parent = this->width();
    if (this->parentWidget())
    {
        width_win_parent = this->parentWidget()->width();
    }
    setFixedHeight(height_text_area_);
    update();
}

void AnalysisReport::setColorMainTitle(const QColor& color)
{
    this->color_main_title_ = color;
}

void AnalysisReport::setColorSubTitle(const QColor& color)
{
    this->color_sub_title_ = color;
}

void AnalysisReport::setFontMainTitle(const QFont& font)
{
    this->font_main_title_ = font;
}

void AnalysisReport::setFontSubTitle(const QFont& font)
{
    this->font_sub_title_ = font;
}

void AnalysisReport::setFontTableSubTitle(const QFont& font)
{
    this->font_table_sub_ = font;
}

void AnalysisReport::setColorBackground(const QColor& color)
{
    color_background_ = color;
}

void AnalysisReport::updateGuiDataSlot()
{
    createDrawElements(this->width(), this->height());
    loadItemsStatusAndInitSelectBoxStatus();
    updateElementsPreview();
}

void AnalysisReport::exportToPdfSlot()
{
    QString pdf_export_file_path = PFusionAlignData->getProjectFilePath();
    QString name_patient = PFusionAlignData->analysisData()->getPatientName();

    QString file_name = QFileDialog::getSaveFileName(
            this, tr("select the PDF file save path"),
            pdf_export_file_path + "/" + name_patient + "-" + tr("report"), "*.pdf");

    if (FileManager::isFileUsed(file_name))
    {
        UiUtilityTools::getInstance()->showInfoMessageBox(
                tr("Info"), tr("file is open, please close it firstly."));
        return;
    }

    if (file_name.isEmpty())
    {
        return;
    }
    else
    {
        pdf_export_file_path = file_name;
    }

    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPrinter::A4);
    printer.setOutputFileName(pdf_export_file_path);

    QPainter painter(&printer);
    painter.endNativePainting();
    painter.save();
    painter.setRenderHint(QPainter::HighQualityAntialiasing);

    createDrawElementsForPdf(printer.width(), printer.height());

    int id_page = 1;
    int start_add = 0;
    for (auto element : draw_elements_)
    {
        if (element->getYCoordinateScreenShowPos() + element->getHeight() >
            printer.height() * id_page)
        {
            printer.newPage();
            ++id_page;

            int sub_y = element->getYCoordinateScreenShowPosOrigion() -
                        printer.height() * (id_page - 1);
            if (sub_y <= 0)
            {
                start_add = std::abs(sub_y) + 20;
            }
            else
            {
                start_add = 0;
            }
        }

        element->transformY(-printer.height() * (id_page - 1) + start_add);

        element->draw(painter);
    }
    painter.restore();
    painter.beginNativePainting();

    createDrawElements(this->width(), this->height());

    emit PSIGNALMANAGER->setProgressBarSignal(100, QString(tr("Export PDF successful")));
}

void AnalysisReport::updateElementsPreview()
{
    createDrawElementsForPdf(800, 800);
    emit updateDrawelements(&draw_elements_);
    createDrawElements(this->width(), this->height());
}

void AnalysisReport::selectBoxChangedSlot(QString text, bool check_state)
{
    if (all_select_box_)
    {
        if (text == all_select_box_->text())
        {
            emit all_select_box_->setChecked(check_state);
        }
        else
        {
            if (select_item_boxs_.find(text) != select_item_boxs_.end())
            {
                select_item_boxs_[text]->setChecked(check_state);
            }
        }
    }
}

void AnalysisReport::selectBoxEnabledSlot(QString text, bool enabled)
{
    if (all_select_box_ == nullptr)
    {
        if (select_item_boxs_.find(text) != select_item_boxs_.end())
        {
            select_item_boxs_[text]->setEnabled(enabled);
        }
    }
    else
    {
        if (text == all_select_box_->text())
        {
            all_select_box_->setEnabled(enabled);
        }
        else
        {
            if (select_item_boxs_.find(text) != select_item_boxs_.end())
            {
                select_item_boxs_[text]->setEnabled(enabled);
            }
        }
    }
}
