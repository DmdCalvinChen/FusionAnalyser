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

#include "fusionanalyserhomegui.h"

#include <QDateTime>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QSettings>

#include <common_base/util/uitools.h>
#include <common_ext/fileManager/filemanager.h>

#include "common_base/SignalManager.h"
#include "common_base/logsingleton.h"
#include "common_ext/util/utility_tools.h"
#include "ui_fusionanalyserhomegui.h"
using namespace ui_common;

FusionAnalyserHomeGui::FusionAnalyserHomeGui(QWidget* parent)
    : QWidget(parent), ui(new Ui::FusionAnalyserHomeGui)
{
    ui->setupUi(this);
    ui->recently_projects_listWidget->setFrameShape(QListWidget::NoFrame);
    ui->recently_projects_listWidget->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    initToolButtonsGui();

    connect(ui->recently_projects_listWidget, &QListWidget::itemDoubleClicked, this,
            &FusionAnalyserHomeGui::selectWhichRecentlyProject);

    connect(ui->import_toolButton, &QToolButton::clicked, this,
            &FusionAnalyserHomeGui::importModelSlot);
    connect(ui->open_toolButton, &QToolButton::clicked, this,
            &FusionAnalyserHomeGui::openRecentlyMeasureProject);
    connect(ui->help_toolButton, &QToolButton::clicked,
            [&] { emit PSIGNALMANAGER->openHelpSignal(); });
    setShowState(true);
}

FusionAnalyserHomeGui::~FusionAnalyserHomeGui()
{
    delete ui;
}

void FusionAnalyserHomeGui::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.fillRect(this->rect(), QColor(36, 50, 68, 0));
}

void FusionAnalyserHomeGui::setShowState(bool enable_show)
{
    if (enable_show)
    {
        updateRecentProjects();
        this->show();
    }
    else
    {
        this->hide();
    }
}

void FusionAnalyserHomeGui::initToolButtonsGui()
{
    auto create_btn_style = [&](const QString& icon_path)
    {
        QString style;
        style = "QToolButton:pressed,QToolButton:hover{background-color:rgb(24,34,46);} \
            QToolButton { padding:45 20 20 20;  \
            background-color:rgb(36,49,66);border-radius:5px;font-size:18px;color:rgb(182,196,209);text-align:center;border-left-width:1px;font-family=MicrosoftYaHei;\
            qproperty-iconSize: 50px 50px;border: none;";

        style += "qproperty-icon: url(" + icon_path + "); }";

        return style;
    };

    QSize button_size(246, 174);
    ui->recent_project_label->setFixedSize(button_size.width(), 20);
    ui->recently_projects_listWidget->setFixedSize(button_size.width(), button_size.height() - 20);

    ui->import_toolButton->setFixedSize(button_size);
    ui->import_toolButton->setStyleSheet(
            create_btn_style(":/default/res/default/images/fusionanalyserhome/open.svg"));

    ui->open_toolButton->setFixedSize(button_size);
    ui->open_toolButton->setStyleSheet(create_btn_style(
            ":/default/res/default/images/fusionanalyserhome/open test record.svg"));

    ui->help_toolButton->setFixedSize(button_size);
    ui->help_toolButton->setStyleSheet(
            create_btn_style(":/default/res/default/images/fusionanalyserhome/help.svg"));
}

void FusionAnalyserHomeGui::addRecentlyProject(const QString& project, const QString& name,
                                               const QString& date)
{
    if (ui->recently_projects_listWidget->count() >= max_number_projects_record_)
    {
        ui->recently_projects_listWidget->takeItem(max_number_projects_record_ - 1);
    }

    QWidget* widget = new QWidget;

    QHBoxLayout* layout = new QHBoxLayout();
    layout->addSpacing(10);

    auto getShortStrLabelWidget = [&widget](const QString& _name)
    {
        QLabel* name_lab_widget = new QLabel(widget);

        QString string_to_short = _name;
        QFontMetrics fontMetrics(name_lab_widget->font());
        // If the string length is greater than the label width under the current font
        if (fontMetrics.width(string_to_short) > name_lab_widget->width())
        {
            string_to_short =
                    QFontMetrics(name_lab_widget->font())
                            .elidedText(string_to_short, Qt::ElideRight, name_lab_widget->width());
            name_lab_widget->setToolTip(_name);
        }
        name_lab_widget->setText(string_to_short);
        return name_lab_widget;
    };

    layout->addWidget(getShortStrLabelWidget(name));
    layout->addWidget(getShortStrLabelWidget(date));

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    widget->setLayout(mainLayout);

    QListWidgetItem* item = new QListWidgetItem;
    item->setSizeHint(QSize(ui->recently_projects_listWidget->sizeHint().width(), 50));
    item->setData(Qt::UserRole, project);
    ui->recently_projects_listWidget->addItem(item);

    ui->recently_projects_listWidget->setItemWidget(item, widget);
}

void FusionAnalyserHomeGui::updateRecentProjects()
{
    QSettings settings;
    QStringList projs = settings.value("recentProjList").toStringList();

    int numRecentProjs = qMin(projs.size(), max_number_projects_record_);

    if (numRecentProjs > 0)
    {
        ui->recently_projects_listWidget->clear();
        for (int i = 0; i < numRecentProjs; ++i)
        {
            if (QFileInfo(projs[i]).exists())
            {
                addRecentlyProject(
                        projs[i], QFileInfo(projs[i]).fileName(),
                        QFileInfo(projs[i]).lastModified().date().toString("yyyy-MM-dd"));
            }
        }
    }
}

void FusionAnalyserHomeGui::selectWhichRecentlyProject()
{
    auto it_selected = ui->recently_projects_listWidget->currentItem();
    QString project = it_selected->data(Qt::UserRole).toString();

    if (FileManager::isFileExist(project))
    {
        setShowState(false);
        emit openRecentlyProject(project);
    }
    else
    {
        UiUtilityTools::getInstance()->showInfoMessageBox(
                tr("Info"), tr("this  project file not exist in this hard."));
    }
}

bool FusionAnalyserHomeGui::importModelSlot()
{
    QFileDialog* pFileDlg = new QFileDialog(this, tr("import model"), "", "*.stl");
    pFileDlg->move(100, 100);
    QStringList files_name = pFileDlg->getOpenFileNames(this, tr("import model"), "", "*.stl");
    if (files_name.empty())
    {
        return false;
    }
    SPDLOG->info("home page import gui");

    QString path = files_name.first();
    QFileInfo file(path);
    path.chop(file.fileName().size() + 1);
    QStringList modelList;
    for (auto& modelPath : files_name)
    {
        QFileInfo info(modelPath);
        modelList.append(info.fileName());
    }

    auto projects_existed = getProjectsDir(path, modelList);
    if (!projects_existed.empty())
    {
        if (projects_existed.size() == 1)
        {
            int open = UiUtilityTools::getInstance()->showQuestionMessageBox(
                    tr("Info"),
                    tr("Current model has a measurement record.Would you like to load it?"));
            if (open > 0)
            {
                setShowState(false);
                emit openRecentlyProject(projects_existed.front());
                return true;
            }
            }
            else
            {
                int open = UiUtilityTools::getInstance()->showQuestionMessageBox(
                        tr("Info"), tr("there is some measure projects existed, choose one?"));
                if (open > 0)
                {
                    if (openRecentlyMeasureProject())
                {
                    return true;
                }
            }
        }
    }

    // If multiple models are selected, only the first two models are selected
    if (files_name.size() > 2)
    {
        UiUtilityTools::getInstance()->showInfoMessageBox(
                tr("Info"), tr("Please select no more than 2 models (maxillary + mandible)!"));
        return false;
    }
    else
    {
        setShowState(false);
        emit importModelSignal(files_name);
    }

    return true;
}

std::vector<QString> FusionAnalyserHomeGui::getProjectsDir(const QString& path,
                                                           const QStringList& file_list)
{
    std::vector<QString> files;

    QDir dir(path);
    dir.setFilter(QDir::Files);
    QStringList filter;
    filter << "*.mlp";
    filter << "*.fa";
    dir.setNameFilters(filter);

    QFileInfoList project_list = dir.entryInfoList();
    for (int i = 0; i < project_list.size(); i++)
    {
        QFileInfo fileInfo = project_list.at(i);
        QString fileName = fileInfo.absoluteFilePath();
        std::vector<QString> modelFiles = UtilityTools::getInstance()->getProjModelFiles(fileName);
        ;
        for (auto model : modelFiles)
        {
            if (file_list.contains(model))
            {
                if (std::find(files.begin(), files.end(), fileName) == files.end())
                    files.push_back(fileInfo.absoluteFilePath());
            }
        }
    }

    return files;
}

bool FusionAnalyserHomeGui::openRecentlyMeasureProject()
{
    QString project = QFileDialog::getOpenFileName(this, tr("Open Record"), "", "*.fa *.mlp");
    QFileInfo file(project);
    if (!file.exists())
    {
        return false;
    }

    setShowState(false);
    emit openRecentlyProject(project);

    return true;
}
