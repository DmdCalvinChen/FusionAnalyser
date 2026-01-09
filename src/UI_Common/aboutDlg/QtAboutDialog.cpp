#include "QtAboutDialog.h"

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

#include <QApplication>
#include <QDate>
#include <QDesktopServices>
#include <QFile>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSysInfo>
#include <QTabWidget>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>

// QtAboutDialogConfig Private Data
class QtAboutDialogConfig::Private
{
public:
    QString appName;
    QString version;
    QPixmap logo;
    QString copyright;
    QString description;
    QString website;
    QString supportEmail;
    QString sourceCodeUrl;
    QString buildDate;
    QString buildTime;
    QString compilerInfo;

    bool showDetailedBuildInfo = false;
    bool showRuntimeInfo = false;
    bool showSystemInfo = false;

    QSize minimumSize = QSize(600, 500);
    QSize maximumSize = QSize(1000, 800);
    QSize defaultSize = QSize(700, 600);
};

// QtAboutDialog Private Data
class QtAboutDialog::Private
{
public:
    QtAboutDialogConfig config;

    QLabel* logoLabel = nullptr;
    QLabel* appNameLabel = nullptr;
    QLabel* versionLabel = nullptr;
    QLabel* descriptionLabel = nullptr;

    QTabWidget* tabWidget = nullptr;
    QTextEdit* lgplTextEdit = nullptr;
    QTextEdit* thirdPartyTextEdit = nullptr;
    QTextEdit* noWarrantyTextEdit = nullptr;
    QTextEdit* buildInfoTextEdit = nullptr;

    QPushButton* sourceCodeButton = nullptr;
    QPushButton* qtWebsiteButton = nullptr;
    QPushButton* companyWebsiteButton = nullptr;
    QPushButton* supportEmailButton = nullptr;
};

// QtAboutDialogConfig Implementation
QtAboutDialogConfig::QtAboutDialogConfig() : d(new Private)
{
}

QtAboutDialogConfig::QtAboutDialogConfig(const QtAboutDialogConfig& other)
    : d(new Private(*other.d))
{
}

QtAboutDialogConfig& QtAboutDialogConfig::operator=(const QtAboutDialogConfig& other)
{
    if (this != &other)
    {
        *d = *other.d;
    }
    return *this;
}

QtAboutDialogConfig::~QtAboutDialogConfig()
{
}

QtAboutDialogConfig& QtAboutDialogConfig::setAppName(const QString& name)
{
    d->appName = name;
    return *this;
}

QtAboutDialogConfig& QtAboutDialogConfig::setVersion(const QString& version)
{
    d->version = version;
    return *this;
}

QtAboutDialogConfig& QtAboutDialogConfig::setLogo(const QString& logoPath)
{
    d->logo = QPixmap(logoPath);
    return *this;
}

QtAboutDialogConfig& QtAboutDialogConfig::setLogo(const QPixmap& logo)
{
    d->logo = logo;
    return *this;
}

QtAboutDialogConfig& QtAboutDialogConfig::setCopyright(const QString& copyright)
{
    d->copyright = copyright;
    return *this;
}

QtAboutDialogConfig& QtAboutDialogConfig::setDescription(const QString& description)
{
    d->description = description;
    return *this;
}

QtAboutDialogConfig& QtAboutDialogConfig::setWebsite(const QString& url)
{
    d->website = url;
    return *this;
}

QtAboutDialogConfig& QtAboutDialogConfig::setSupportEmail(const QString& email)
{
    d->supportEmail = email;
    return *this;
}

QtAboutDialogConfig& QtAboutDialogConfig::setSourceCodeUrl(const QString& url)
{
    d->sourceCodeUrl = url;
    return *this;
}

QtAboutDialogConfig& QtAboutDialogConfig::setBuildDate(const QString& date)
{
    d->buildDate = date;
    return *this;
}

QtAboutDialogConfig& QtAboutDialogConfig::setBuildTime(const QString& time)
{
    d->buildTime = time;
    return *this;
}

QtAboutDialogConfig& QtAboutDialogConfig::setCompilerInfo(const QString& compiler)
{
    d->compilerInfo = compiler;
    return *this;
}

QtAboutDialogConfig& QtAboutDialogConfig::setShowDetailedBuildInfo(bool show)
{
    d->showDetailedBuildInfo = show;
    return *this;
}

QtAboutDialogConfig& QtAboutDialogConfig::setShowRuntimeInfo(bool show)
{
    d->showRuntimeInfo = show;
    return *this;
}

QtAboutDialogConfig& QtAboutDialogConfig::setShowSystemInfo(bool show)
{
    d->showSystemInfo = show;
    return *this;
}

QtAboutDialogConfig& QtAboutDialogConfig::setMinimumSize(const QSize& size)
{
    d->minimumSize = size;
    return *this;
}

QtAboutDialogConfig& QtAboutDialogConfig::setMaximumSize(const QSize& size)
{
    d->maximumSize = size;
    return *this;
}

QtAboutDialogConfig& QtAboutDialogConfig::setDefaultSize(const QSize& size)
{
    d->defaultSize = size;
    return *this;
}

QString QtAboutDialogConfig::appName() const
{
    return d->appName;
}
QString QtAboutDialogConfig::version() const
{
    return d->version;
}
QPixmap QtAboutDialogConfig::logo() const
{
    return d->logo;
}
QString QtAboutDialogConfig::copyright() const
{
    return d->copyright;
}
QString QtAboutDialogConfig::description() const
{
    return d->description;
}
QString QtAboutDialogConfig::website() const
{
    return d->website;
}
QString QtAboutDialogConfig::supportEmail() const
{
    return d->supportEmail;
}
QString QtAboutDialogConfig::sourceCodeUrl() const
{
    return d->sourceCodeUrl;
}
QString QtAboutDialogConfig::buildDate() const
{
    return d->buildDate;
}
QString QtAboutDialogConfig::buildTime() const
{
    return d->buildTime;
}
QString QtAboutDialogConfig::compilerInfo() const
{
    return d->compilerInfo;
}

bool QtAboutDialogConfig::showDetailedBuildInfo() const
{
    return d->showDetailedBuildInfo;
}
bool QtAboutDialogConfig::showRuntimeInfo() const
{
    return d->showRuntimeInfo;
}
bool QtAboutDialogConfig::showSystemInfo() const
{
    return d->showSystemInfo;
}

QSize QtAboutDialogConfig::minimumSize() const
{
    return d->minimumSize;
}
QSize QtAboutDialogConfig::maximumSize() const
{
    return d->maximumSize;
}
QSize QtAboutDialogConfig::defaultSize() const
{
    return d->defaultSize;
}

bool QtAboutDialogConfig::isValid() const
{
    return !d->appName.isEmpty() || !d->version.isEmpty();
}

// QtAboutDialog Implementation
QtAboutDialog::QtAboutDialog(QWidget* parent) : QDialog(parent), d(new Private)
{
    initializeDefaultConfig();
    setupUI();
    loadLicenseTexts();
    updateDisplay();

    setWindowTitle(tr("About"));
    resize(d->config.defaultSize());
}

QtAboutDialog::QtAboutDialog(const QtAboutDialogConfig& config, QWidget* parent)
    : QDialog(parent), d(new Private)
{
    d->config = config;
    if (!config.isValid())
    {
        initializeDefaultConfig();
    }

    setupUI();
    loadLicenseTexts();
    updateDisplay();

    setWindowTitle(tr("About"));
    resize(d->config.defaultSize());
}

QtAboutDialog::~QtAboutDialog()
{
}

void QtAboutDialog::show(QWidget* parent)
{
    QtAboutDialogConfig config = createDefaultConfig();
    QtAboutDialog dialog(config, parent);
    dialog.exec();
}

void QtAboutDialog::show(const QtAboutDialogConfig& config, QWidget* parent)
{
    if (config.isValid())
    {
        QtAboutDialog dialog(config, parent);
        dialog.exec();
    }
    else
    {
        QtAboutDialog dialog(createDefaultConfig(), parent);
        dialog.exec();
    }
}

QtAboutDialogConfig QtAboutDialog::createDefaultConfig()
{
    QtAboutDialogConfig config;

    // Application information
    QString appName = QApplication::applicationName();
    if (appName.isEmpty())
    {
        appName = "FusionAnalyser";
    }
    config.setAppName(appName);

    QString version = QApplication::applicationVersion();
    if (version.isEmpty())
    {
        version = "1.0.0";
    }
    config.setVersion(version);

    // Copyright and description
    config.setCopyright(
            tr("Copyright © %1 AiAlign").arg(QDate::currentDate().year()));
    config.setDescription(
            tr("Digital Dental Model Measurement Analysis Software\n"
               "An open-source dental analysis tool"));

    // Project website and source code
    config.setWebsite("https://www.ai-align.cn/fusionanalyser/");
    config.setSourceCodeUrl("https://github.com/aialign2025/FusionAnalyser");

    // Support email
    config.setSupportEmail("your-email@example.com");

    // Logo
    config.setLogo(":/dark/res/dark/images/titlebar/FusionAnalyser.png");

    // Display options
    config.setShowDetailedBuildInfo(true);
    config.setShowRuntimeInfo(true);
    config.setShowSystemInfo(true);

    return config;
}

void QtAboutDialog::showWithAppInfo(const QString& appName, const QString& version, QWidget* parent)
{
    QtAboutDialogConfig config = createDefaultConfig();
    config.setAppName(appName);
    config.setVersion(version);
    QtAboutDialog dialog(config, parent);
    dialog.exec();
}

void QtAboutDialog::showWithAppInfo(const QString& appName, const QString& version,
                                    const QString& copyright, QWidget* parent)
{
    QtAboutDialogConfig config = createDefaultConfig();
    config.setAppName(appName);
    config.setVersion(version);
    config.setCopyright(copyright);
    QtAboutDialog dialog(config, parent);
    dialog.exec();
}

void QtAboutDialog::setConfig(const QtAboutDialogConfig& config)
{
    d->config = config;
    updateDisplay();
}

QtAboutDialogConfig QtAboutDialog::config() const
{
    return d->config;
}

void QtAboutDialog::initializeDefaultConfig()
{
    d->config = createDefaultConfig();
}

void QtAboutDialog::setupUI()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Logo
    d->logoLabel = new QLabel();
    d->logoLabel->setAlignment(Qt::AlignCenter);
    d->logoLabel->setFixedHeight(80);
    mainLayout->addWidget(d->logoLabel);

    // App name
    d->appNameLabel = new QLabel();
    QFont nameFont;
    nameFont.setPointSize(16);
    nameFont.setBold(true);
    d->appNameLabel->setFont(nameFont);
    d->appNameLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(d->appNameLabel);

    // Version
    d->versionLabel = new QLabel();
    d->versionLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(d->versionLabel);

    // Description
    d->descriptionLabel = new QLabel();
    d->descriptionLabel->setAlignment(Qt::AlignCenter);
    d->descriptionLabel->setWordWrap(true);
    mainLayout->addWidget(d->descriptionLabel);

    // Separator
    auto separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator);

    // Tab widget
    d->tabWidget = new QTabWidget();

    // GPL License tab
    d->lgplTextEdit = new QTextEdit();
    d->lgplTextEdit->setReadOnly(true);
    d->tabWidget->addTab(d->lgplTextEdit, tr("GPL v3.0 License"));

    // Third-party components tab
    d->thirdPartyTextEdit = new QTextEdit();
    d->thirdPartyTextEdit->setReadOnly(true);
    d->tabWidget->addTab(d->thirdPartyTextEdit, tr("Third-Party Components"));

    // No Warranty tab
    d->noWarrantyTextEdit = new QTextEdit();
    d->noWarrantyTextEdit->setReadOnly(true);
    d->tabWidget->addTab(d->noWarrantyTextEdit, tr("No Warranty"));

    // Build Info tab
    d->buildInfoTextEdit = new QTextEdit();
    d->buildInfoTextEdit->setReadOnly(true);
    d->tabWidget->addTab(d->buildInfoTextEdit, tr("Build Information"));

    mainLayout->addWidget(d->tabWidget);

    // Button area
    auto buttonLayout = new QHBoxLayout();

    d->sourceCodeButton = new QPushButton(tr("Source Code"));
    d->sourceCodeButton->setToolTip(tr("View FusionAnalyser source code on GitHub"));
    connect(d->sourceCodeButton, &QPushButton::clicked, this, &QtAboutDialog::openSourceCodeLink);

    d->qtWebsiteButton = new QPushButton(tr("Qt Framework"));
    d->qtWebsiteButton->setToolTip(tr("Visit Qt official website"));
    connect(d->qtWebsiteButton, &QPushButton::clicked, this, &QtAboutDialog::openQtWebsite);

    d->companyWebsiteButton = new QPushButton(tr("Project Website"));
    d->companyWebsiteButton->setToolTip(tr("Visit FusionAnalyser project website"));
    connect(d->companyWebsiteButton, &QPushButton::clicked, this,
            &QtAboutDialog::openCompanyWebsite);

    d->supportEmailButton = new QPushButton(tr("Contact Us"));
    d->supportEmailButton->setToolTip(tr("Send email to technical support"));
    connect(d->supportEmailButton, &QPushButton::clicked, this, &QtAboutDialog::sendSupportEmail);

    auto closeButton = new QPushButton(tr("Close"));
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    buttonLayout->addWidget(d->sourceCodeButton);
    buttonLayout->addWidget(d->qtWebsiteButton);
    buttonLayout->addWidget(d->companyWebsiteButton);
    buttonLayout->addWidget(d->supportEmailButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    mainLayout->addLayout(buttonLayout);
}

void QtAboutDialog::loadLicenseTexts()
{
    // Load GPL v3.0 license from resources
    QFile gplFile(":/licenses/licenses/gpl_v3.txt");
    if (gplFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        d->lgplTextEdit->setPlainText(QString::fromUtf8(gplFile.readAll()));
        gplFile.close();
    }
    else
    {
        d->lgplTextEdit->setPlainText(tr("Unable to load GPL v3.0 license text."));
    }

    // Build comprehensive third-party licenses text
    QString thirdPartyText;

    // Qt Framework
    thirdPartyText += tr("=== Qt Framework (LGPL v3) ===\n\n");
    thirdPartyText += tr("Qt is a cross-platform application development framework.\n");
    thirdPartyText += tr("License: GNU Lesser General Public License v3\n");
    thirdPartyText += tr("Website: https://www.qt.io/\n");
    thirdPartyText += tr("Source: https://www.qt.io/download-open-source\n\n");

    // VCG Library
    thirdPartyText += tr("=== VCG Library (GPL v3.0) ===\n\n");
    thirdPartyText += tr("Visualization and Computer Graphics Library\n");
    thirdPartyText += tr("License: GNU General Public License v3.0\n");
    thirdPartyText += tr("Website: http://vcg.isti.cnr.it/\n");
    thirdPartyText += tr("Source: https://github.com/cnr-isti-vclab/vcglib\n\n");

    // Eigen
    thirdPartyText += tr("=== Eigen (MPL 2.0) ===\n\n");
    thirdPartyText += tr("C++ template library for linear algebra\n");
    thirdPartyText += tr("License: Mozilla Public License 2.0\n");
    thirdPartyText += tr("Website: http://eigen.tuxfamily.org/\n\n");

    // GLEW
    thirdPartyText += tr("=== GLEW 2.1.0 (BSD/Mesa/Khronos) ===\n\n");
    thirdPartyText += tr("OpenGL Extension Wrangler Library\n");
    thirdPartyText += tr("License: Modified BSD License / Mesa 3-D License / Khronos License\n");
    thirdPartyText += tr("Website: https://github.com/nigels-com/glew\n\n");

    // SpdLog
    thirdPartyText += tr("=== SpdLog (MIT) ===\n\n");
    thirdPartyText += tr("Fast C++ logging library\n");
    thirdPartyText += tr("License: MIT License\n");
    thirdPartyText += tr("Website: https://github.com/gabime/spdlog\n\n");

    // QuaZip
    thirdPartyText += tr("=== QuaZip (LGPL v2.1+) ===\n\n");
    thirdPartyText += tr("Qt wrapper for minizip\n");
    thirdPartyText += tr("License: GNU Lesser General Public License v2.1 or later\n");
    thirdPartyText += tr("Website: https://github.com/stachenov/quazip\n\n");

    // Google Breakpad
    thirdPartyText += tr("=== Google Breakpad (Apache 2.0) ===\n\n");
    thirdPartyText += tr("Crash reporting library\n");
    thirdPartyText += tr("License: Apache License 2.0\n");
    thirdPartyText += tr("Website: https://chromium.googlesource.com/breakpad/breakpad\n\n");

    // jhead
    thirdPartyText += tr("=== jhead 2.95 (Public Domain) ===\n\n");
    thirdPartyText += tr("JPEG header manipulation tool\n");
    thirdPartyText += tr("License: Public Domain\n");
    thirdPartyText += tr("Website: http://www.sentex.net/~mwandel/jhead/\n\n");

    thirdPartyText += tr(
            "\nFor detailed license texts, see the license files in each library's directory.\n");
    thirdPartyText += tr("See THIRD_PARTY_LICENSES.md for complete information.\n");

    d->thirdPartyTextEdit->setPlainText(thirdPartyText);

    // No warranty notice
    d->noWarrantyTextEdit->setPlainText(
            tr("NO WARRANTY\n\n"
               "This program is distributed in the hope that it will be useful, "
               "but WITHOUT ANY WARRANTY; without even the implied warranty of "
               "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n"
               "See the GNU General Public License for more details.\n\n"
               "You should have received a copy of the GNU General Public License "
               "along with this program. If not, see https://www.gnu.org/licenses/"));

    generateBuildInfo();
}

void QtAboutDialog::updateDisplay()
{
    if (!d->config.logo().isNull())
    {
        d->logoLabel->setPixmap(
                d->config.logo().scaled(200, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    d->appNameLabel->setText(d->config.appName());
    d->versionLabel->setText(tr("Version %1").arg(d->config.version()));

    if (!d->config.copyright().isEmpty())
    {
        d->descriptionLabel->setText(d->config.copyright());
    }
    else if (!d->config.description().isEmpty())
    {
        d->descriptionLabel->setText(d->config.description());
    }

    d->companyWebsiteButton->setVisible(!d->config.website().isEmpty());
    d->supportEmailButton->setVisible(!d->config.supportEmail().isEmpty());
}

void QtAboutDialog::openSourceCodeLink()
{
    if (d->config.sourceCodeUrl().isEmpty())
    {
        QMessageBox::information(this, tr("Source Code"), tr("Source code URL not configured."));
        return;
    }

    QString url = d->config.sourceCodeUrl();
    QDesktopServices::openUrl(QUrl(url));
}

void QtAboutDialog::openQtWebsite()
{
    QDesktopServices::openUrl(QUrl("https://www.qt.io/"));
}

void QtAboutDialog::openCompanyWebsite()
{
    if (d->config.website().isEmpty())
    {
        return;
    }

    QDesktopServices::openUrl(QUrl(d->config.website()));
}

void QtAboutDialog::sendSupportEmail()
{
    QString email = d->config.supportEmail();
    if (email.isEmpty())
    {
        QMessageBox::information(this, tr("Contact Us"), tr("Support email not configured."));
        return;
    }

    if (!email.contains("@") || !email.contains("."))
    {
        QMessageBox::warning(this, tr("Contact Us"), tr("Email address format is incorrect."));
        return;
    }

    QString mailUrl = QString("mailto:%1?subject=%2 %3 Support Request")
                              .arg(email, d->config.appName(), d->config.version());

    if (!QDesktopServices::openUrl(QUrl(mailUrl)))
    {
        QMessageBox::information(
                this, tr("Contact Us"),
                tr("Unable to automatically open email client, please send email manually to:\n%1")
                        .arg(email));
    }
}

void QtAboutDialog::generateBuildInfo()
{
    d->buildInfoTextEdit->setPlainText(generateDetailedBuildInfo());
}

QString QtAboutDialog::generateDetailedBuildInfo()
{
    QStringList info;

    info << tr("=== %1 ===").arg(tr("Application Information"));
    info << tr("Application Name: %1").arg(d->config.appName());
    info << tr("Version: %1").arg(d->config.version());
    info << "";

    info << tr("=== %1 ===").arg(tr("Qt Framework Information"));
    info << tr("Qt Runtime Version: %1").arg(qVersion());
    info << tr("Qt Compile Version: %1").arg(QT_VERSION_STR);
    info << "";

    info << tr("=== %1 ===").arg(tr("Build Information"));
    info << getCompilerInfo();
    info << tr("Build Date: %1 %2").arg(__DATE__).arg(__TIME__);
    info << getBuildConfiguration();

    if (d->config.showSystemInfo())
    {
        info << "";
        info << tr("=== %1 ===").arg(tr("System Information"));
        info << getSystemInfo();
    }

    if (d->config.showRuntimeInfo())
    {
        info << "";
        info << tr("=== %1 ===").arg(tr("Runtime Information"));
        info << getRuntimeInfo();
    }

    return info.join("\n");
}

QString QtAboutDialog::getCompilerInfo()
{
#if defined(_MSC_VER)
    return tr("Compiler: MSVC %1").arg(_MSC_VER);
#elif defined(__GNUC__)
    return tr("Compiler: GCC %1.%2.%3").arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__);
#elif defined(__clang__)
    return tr("Compiler: Clang %1.%2.%3")
            .arg(__clang_major__)
            .arg(__clang_minor__)
            .arg(__clang_patchlevel__);
#else
    return tr("Compiler: Unknown");
#endif
}

QString QtAboutDialog::getBuildConfiguration()
{
#ifdef QT_NO_DEBUG
    return tr("Build Type: Release");
#else
    return tr("Build Type: Debug");
#endif
}

QString QtAboutDialog::getSystemInfo()
{
    QStringList info;
    info << tr("OS: %1").arg(QSysInfo::prettyProductName());
    info << tr("Kernel: %1 %2").arg(QSysInfo::kernelType()).arg(QSysInfo::kernelVersion());
    info << tr("CPU Architecture: %1").arg(QSysInfo::currentCpuArchitecture());
    info << tr("Build Architecture: %1").arg(QSysInfo::buildCpuArchitecture());
    return info.join("\n");
}

QString QtAboutDialog::getRuntimeInfo()
{
    QStringList info;
    info << tr("Application PID: %1").arg(QApplication::applicationPid());
    info << tr("Application Path: %1").arg(QApplication::applicationDirPath());
    return info.join("\n");
}
