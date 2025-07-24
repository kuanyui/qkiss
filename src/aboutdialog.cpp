/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2025 ono ono (kuanyui) <azazabc123@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/
#include "aboutdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTextBrowser>
#include <QLabel>
#include <QIcon>
#include <QDialogButtonBox>
#include <QApplication>
#include <QCoreApplication>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , m_tabWidget(new QTabWidget(this))
    , m_iconLabel(new QLabel(this))
    , m_appNameLabel(new QLabel(this))
    , m_infoLabel(new QLabel(this))
    , m_licensesBrowser(new QTextBrowser(this))
    , m_translatorsBrowser(new QTextBrowser(this))
{
    setupUi();
}

AboutDialog::~AboutDialog()
{
}

void AboutDialog::setupUi()
{
    setWindowTitle(tr("About MyApp"));
    resize(500, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Header section (icon and app name)
    QHBoxLayout *headerLayout = new QHBoxLayout();

    QIcon appIcon = QIcon::fromTheme("qkiss");
    m_iconLabel->setPixmap(appIcon.pixmap(48, 48));
    headerLayout->addWidget(m_iconLabel);

    // App name and version
    m_appNameLabel->setText("<h2>QKiSS</h2><p>Version 1.0.0</p>");
    headerLayout->addWidget(m_appNameLabel);
    headerLayout->addStretch();

    mainLayout->addLayout(headerLayout);

    // Tab widget
    mainLayout->addWidget(m_tabWidget);

    // Information tab
    QWidget *infoTab = new QWidget();
    QVBoxLayout *infoLayout = new QVBoxLayout(infoTab);
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setText(tr("\n\n") +
                         tr("Copyright") + "© 2024 - 2025\n");
    infoLayout->addWidget(m_infoLabel);
    infoLayout->addStretch();
    m_tabWidget->addTab(infoTab, tr("Information"));

    // Licenses tab
    m_licensesBrowser->setOpenExternalLinks(true);
    m_licensesBrowser->setReadOnly(true);
    m_licensesBrowser->setHtml("GPLv3");
    m_tabWidget->addTab(m_licensesBrowser, tr("Licenses"));

    // Translators tab
    m_translatorsBrowser->setOpenExternalLinks(true);
    m_translatorsBrowser->setReadOnly(true);
    m_translatorsBrowser->setHtml(tr(R"(English: Ono ono)"));
    m_tabWidget->addTab(m_translatorsBrowser, tr("Translators"));

    // Button box
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::accept);
    mainLayout->addWidget(buttonBox);
}


