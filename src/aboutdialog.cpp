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
    , m_labelIcon(new QLabel(this))
    , m_labelAppName(new QLabel(this))
    , m_labelSummary(new QLabel(this))
    , m_browserLicense(new QTextBrowser(this))
    , m_browserTranslators(new QTextBrowser(this))
{
    setupUi();
}

AboutDialog::~AboutDialog()
{
}

void AboutDialog::setupUi()
{
    setWindowTitle(tr("About QKiSS"));
    resize(500, 400);

    QVBoxLayout *layoutMain = new QVBoxLayout(this);
    QVBoxLayout *layoutHeader = new QVBoxLayout();
    layoutHeader->setAlignment(Qt::AlignmentFlag::AlignCenter);

    QIcon appIcon = QIcon::fromTheme("help-about");
    m_labelIcon->setPixmap(appIcon.pixmap(48, 48));
    m_labelIcon->setAlignment(Qt::AlignmentFlag::AlignCenter);
    m_labelIcon->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    layoutHeader->addWidget(m_labelIcon);


    m_labelAppName->setText("App Name Is Here");
    m_labelAppName->setAlignment(Qt::AlignmentFlag::AlignCenter);
    m_labelAppName->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    layoutHeader->addWidget(m_labelAppName);

    QLabel *labelAppVersion = new QLabel{this};
    labelAppVersion->setText("1.0.0");
    labelAppVersion->setAlignment(Qt::AlignmentFlag::AlignCenter);
    labelAppVersion->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    layoutHeader->addWidget(labelAppVersion);

    QLabel *labelSummary = new QLabel{this};
    labelSummary->setText(tr("QKiSS is a modern implementation of KiSS (Kisekae, 着せ替え) in Qt 6. Kisekae is short for kisekae ningyou, a Japanese term meaning "));
    labelSummary->setAlignment(Qt::AlignmentFlag::AlignCenter);
    labelSummary->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    layoutHeader->addWidget(labelSummary);

    layoutMain->addLayout(layoutHeader);
    layoutMain->addWidget(m_tabWidget);

    // Information tab
    QWidget *infoTab = new QWidget();
    QVBoxLayout *infoLayout = new QVBoxLayout(infoTab);
    m_labelSummary->setWordWrap(true);
    m_labelSummary->setText("Copyright (C) 2024 - 2025");
    infoLayout->addWidget(m_labelSummary);
    infoLayout->addStretch();
    m_tabWidget->addTab(infoTab, tr("Information"));

    // Licenses tab
    m_browserLicense->setOpenExternalLinks(true);
    m_browserLicense->setReadOnly(true);
    m_browserLicense->setHtml("GPLv3");
    m_tabWidget->addTab(m_browserLicense, tr("Licenses"));

    // Translators tab
    m_browserTranslators->setOpenExternalLinks(true);
    m_browserTranslators->setReadOnly(true);
    m_browserTranslators->setHtml("English: Author");
    m_tabWidget->addTab(m_browserTranslators, tr("Translators"));

    // Button box
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::accept);
    layoutMain->addWidget(buttonBox);
}


