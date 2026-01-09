#ifndef QTABOUTDIALOG_H
#define QTABOUTDIALOG_H

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

#include <QDialog>
#include <QPixmap>
#include <QScopedPointer>
#include <QSize>
#include <QString>

#include "../uicommon_global.h"

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QTextEdit;
class QTabWidget;

// Forward declaration
class QtAboutDialogConfig;

class UI_COMMONSHARED_EXPORT QtAboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QtAboutDialog(QWidget* parent = nullptr);
    explicit QtAboutDialog(const QtAboutDialogConfig& config, QWidget* parent = nullptr);
    ~QtAboutDialog();

    // Static convenience methods
    static void show(QWidget* parent = nullptr);
    static void show(const QtAboutDialogConfig& config, QWidget* parent = nullptr);

    // Quick configuration methods
    static void showWithAppInfo(const QString& appName, const QString& version,
                                QWidget* parent = nullptr);
    static void showWithAppInfo(const QString& appName, const QString& version,
                                const QString& copyright, QWidget* parent = nullptr);

    // Static helper method
    static QtAboutDialogConfig createDefaultConfig();

    // Configuration methods
    void setConfig(const QtAboutDialogConfig& config);
    QtAboutDialogConfig config() const;

private slots:
    void openSourceCodeLink();
    void openQtWebsite();
    void openCompanyWebsite();
    void sendSupportEmail();

private:
    void setupUI();
    void loadLicenseTexts();
    void updateDisplay();
    void initializeDefaultConfig();

    void generateBuildInfo();
    QString generateDetailedBuildInfo();
    QString getCompilerInfo();
    QString getBuildConfiguration();
    QString getSystemInfo();
    QString getRuntimeInfo();

    class Private;
    QScopedPointer<Private> d;
};

// Configuration class
class UI_COMMONSHARED_EXPORT QtAboutDialogConfig
{
public:
    QtAboutDialogConfig();
    QtAboutDialogConfig(const QtAboutDialogConfig& other);
    QtAboutDialogConfig& operator=(const QtAboutDialogConfig& other);
    ~QtAboutDialogConfig();

    // Chainable configuration methods
    QtAboutDialogConfig& setAppName(const QString& name);
    QtAboutDialogConfig& setVersion(const QString& version);
    QtAboutDialogConfig& setLogo(const QString& logoPath);
    QtAboutDialogConfig& setLogo(const QPixmap& logo);
    QtAboutDialogConfig& setCopyright(const QString& copyright);
    QtAboutDialogConfig& setDescription(const QString& description);
    QtAboutDialogConfig& setWebsite(const QString& url);
    QtAboutDialogConfig& setSupportEmail(const QString& email);
    QtAboutDialogConfig& setSourceCodeUrl(const QString& url);

    // Build information
    QtAboutDialogConfig& setBuildDate(const QString& date);
    QtAboutDialogConfig& setBuildTime(const QString& time);
    QtAboutDialogConfig& setCompilerInfo(const QString& compiler);

    // Display options
    QtAboutDialogConfig& setShowDetailedBuildInfo(bool show);
    QtAboutDialogConfig& setShowRuntimeInfo(bool show);
    QtAboutDialogConfig& setShowSystemInfo(bool show);

    // Window size settings
    QtAboutDialogConfig& setMinimumSize(const QSize& size);
    QtAboutDialogConfig& setMaximumSize(const QSize& size);
    QtAboutDialogConfig& setDefaultSize(const QSize& size);

    bool isValid() const;

    // Getters
    QString appName() const;
    QString version() const;
    QPixmap logo() const;
    QString copyright() const;
    QString description() const;
    QString website() const;
    QString supportEmail() const;
    QString sourceCodeUrl() const;
    QString buildDate() const;
    QString buildTime() const;
    QString compilerInfo() const;

    bool showDetailedBuildInfo() const;
    bool showRuntimeInfo() const;
    bool showSystemInfo() const;

    QSize minimumSize() const;
    QSize maximumSize() const;
    QSize defaultSize() const;

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif    // QTABOUTDIALOG_H
