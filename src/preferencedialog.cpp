/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2024 - 2025 ono ono (kuanyui) <azazabc123@gmail.com>
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

#include "preferencedialog.h"
#include "abstractarchiveprocessor.h"
#include "archiveprocessorfactory.h"
#include "configfile.h"
#include "zgroupbox.h"
#include "logging.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QApplication>

using namespace Qt::StringLiterals;

PreferenceDialog::PreferenceDialog(QWidget *parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);    // Auto delete self when close / ok
    setModal(true);
    setWindowTitle(tr("Preferences"));

    m_dialogButtons = new QDialogButtonBox(
                QDialogButtonBox::Apply |
                QDialogButtonBox::Ok    |
                QDialogButtonBox::Cancel,
                Qt::Horizontal, this);

    // Ok
    connect(m_dialogButtons, &QDialogButtonBox::accepted, this, [=, this](){
        writeModelToSettings();
        close();
    });
    // Apply
    connect(m_dialogButtons->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, [=, this]() {
        writeModelToSettings();
    });
    // Cancel
    connect(m_dialogButtons, &QDialogButtonBox::rejected, this, [=, this] {
        close();
    });
    auto *tabs = new QTabWidget(this);
    // tabs->addTab(createUiTab(), tr("User Interface"));
    tabs->addTab(createBackendTab(), tr("LZH Extractor Backend"));

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(tabs);
    layout->addWidget(m_dialogButtons);
    readSettingsToModel();
}

PreferenceDialog::~PreferenceDialog()
{
    qkDebug << "PreferenceDialog dtor()";
}

void PreferenceDialog::readSettingsToModel()
{
    auto &cfgFile = ConfigFile::instance();
    m_appliedModel.lzhBackend.lzhBackend = cfgFile.lzhDecompressorBackend();
    m_appliedModel.files.caseSensitivity = cfgFile.caseSensitivity();
    m_appliedModel.files.encoding = cfgFile.encoding();
    m_appliedModel.ui.language = cfgFile.uiLanguage();
    m_copiedModel = m_appliedModel;
    syncCopiedModelToUiComponents();
}

void PreferenceDialog::writeModelToSettings()
{
    auto &cfgFile = ConfigFile::instance();
    cfgFile.setLzhDecompressorBackend(m_copiedModel.lzhBackend.lzhBackend);
    cfgFile.setCaseSensitivity(m_copiedModel.files.caseSensitivity);
    cfgFile.setEncoding(m_copiedModel.files.encoding);
    cfgFile.setUiLanguage(m_copiedModel.ui.language);
    m_appliedModel = m_copiedModel;
    syncCopiedModelToUiComponents();
}

void PreferenceDialog::syncCopiedModelToUiComponents()
{
    m_cboxLzhExtractorBackend->setCurrentIndexByUserData(QVariant::fromValue(m_copiedModel.lzhBackend.lzhBackend));
    refreshDirtyState();
}

void PreferenceDialog::syncUiComponentsToCopiedModel()
{
    m_copiedModel.lzhBackend.lzhBackend = m_cboxLzhExtractorBackend->currentData().value<QKiSS::LzhDecompressorBackend>();
    refreshDirtyState();
}

void PreferenceDialog::refreshDirtyState()
{
    // Update dirty state & dialog buttons
    m_modelDirty = m_appliedModel != m_copiedModel;
    m_dialogButtons->button(QDialogButtonBox::Apply)->setDisabled(!m_modelDirty);
    m_dialogButtons->button(QDialogButtonBox::Ok)->setDisabled(!m_modelDirty);
}

QWidget *PreferenceDialog::createUiTab()
{
    auto *w = new QWidget(this);
    auto *l = new QVBoxLayout(w);

    l->addStretch();
    return w;
}

namespace {
struct IconTextField {
    QString iconName;
    QString fieldName;
    QLabel *labelFieldValue;
    QString AbstractArchiveProcessor::Info::* structFieldPtr;
};
}

QWidget *PreferenceDialog::createBackendTab()
{
    auto *rootWidget = new QWidget(this);
    auto *rootVbox = new QVBoxLayout(rootWidget);
    auto *layoutForm = new QFormLayout();   // Don't pass QWidget to it. If passed, the passed QWidget will use this layout, but each QWidget can only hold one layout in the same time.
    // Backend ComboBox
    m_cboxLzhExtractorBackend = new ZComboBox(rootWidget);
    m_cboxLzhExtractorBackend->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    layoutForm->addRow(new QLabel(tr("Backend")), m_cboxLzhExtractorBackend);
    // Backend availability checking
    QHBoxLayout *layoutLzhStatus = new QHBoxLayout();
    QLabel *labelLzhStatusIcon = new QLabel();
    QLabel *labelLzhStatusText= new QLabel();
    layoutForm->addRow(new QLabel(tr("Status")), layoutLzhStatus);
    layoutLzhStatus->addWidget(labelLzhStatusIcon);
    layoutLzhStatus->addWidget(labelLzhStatusText, 1);

    //
    rootVbox->addLayout(layoutForm);

    // Information for selected backend
    std::unique_ptr<std::vector<std::unique_ptr<AbstractArchiveProcessor>>> allBackends = ArchiveProcessorFactory::getAllAvailableArchiveProcessors();
    for (auto &be : *allBackends) {
        auto beInfo = be->info();
        m_cboxLzhExtractorBackend->addItem(beInfo.name, (int)beInfo.id);
        m_allAvailableLzhExtractorBackends.push_back(std::move(be));
    }

    QList<IconTextField> fields = {
       {"backend-name"                       , tr("Name"),               new QLabel(), &AbstractArchiveProcessor::Info::name }             , // Gnome
       {"backend-summary"                    , tr("Summary"),            new QLabel(), &AbstractArchiveProcessor::Info::summary }          , // Humanity
       {"backend-lzh-compatibility"          , tr("LZH Compatibility"),  new QLabel(), &AbstractArchiveProcessor::Info::lzhCompatibility } , // Humanity
       {"backend-installation"               , tr("Installation"),       new QLabel(), &AbstractArchiveProcessor::Info::installation }     , // Humanity
       {"backend-os-support"                 , tr("OS Support"),         new QLabel(), &AbstractArchiveProcessor::Info::osSupport }        , // GimpDefault
       {"backend-disk"                       , tr("Disk Operation"),     new QLabel(), &AbstractArchiveProcessor::Info::diskOperation }    , // Humanity
       {"backend-website"                    , tr("Website"),            new QLabel(), &AbstractArchiveProcessor::Info::website }            // Humanity
    };
    auto *groupBox = new ZGroupBox(tr("Backend Information"), rootWidget);
    rootVbox->addWidget(groupBox);
    QGridLayout *grid = new QGridLayout;
    groupBox->setContentLayout(grid);
    for (uint r = 0; r < fields.size(); r++) {
        IconTextField &field = fields[r];
        QFont defaultFont = QApplication::font();
        QFontMetrics metrics(defaultFont);
        int h = metrics.height();

        QLabel *labelIcon = new QLabel();
        labelIcon->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
        labelIcon->setPixmap(QIcon::fromTheme(field.iconName).pixmap(h, h));
        grid->addWidget(labelIcon, r, 0, Qt::AlignTop);

        QLabel *labelFieldName = new QLabel(field.fieldName);
        labelFieldName->setStyleSheet("font-weight: bold; color: gray;"_L1);
        grid->addWidget(labelFieldName, r, 1, Qt::AlignTop);

        field.labelFieldValue->setWordWrap(true);
        field.labelFieldValue->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
        grid->addWidget(fields[r].labelFieldValue, r, 2, Qt::AlignTop);
    }

    QFont defaultFont = QApplication::font();
    QFontMetrics metrics(defaultFont);
    int iconSize = metrics.height();

    auto cboxIndexChangedCb = [=, this](int index) {
        const auto &selectedBackend = m_allAvailableLzhExtractorBackends[index];
        // Display info in `QLabel`s
        const auto &selectedBackendInfo = selectedBackend->info();
        for (auto &field : fields) {
            field.labelFieldValue->setText(selectedBackendInfo.*field.structFieldPtr);
        }
        if (selectedBackend->checkAvailability()) {
            labelLzhStatusIcon->setPixmap(QIcon::fromTheme("dialog-ok").pixmap(iconSize, iconSize));
            labelLzhStatusText->setText(tr("Installed"));
        } else {
            labelLzhStatusIcon->setPixmap(QIcon::fromTheme("dialog-cancel").pixmap(iconSize, iconSize));
            labelLzhStatusText->setText(tr("Not Found / Not Installed"));
        }
        syncUiComponentsToCopiedModel();
        // cbox -> copiedModel
        // m_copiedModel.lzhBackend.lzhBackend = selectedBackendInfo.id;
        // syncCopiedModelToUiComponents();
    };
    connect(m_cboxLzhExtractorBackend, &QComboBox::currentIndexChanged, this, cboxIndexChangedCb);
    cboxIndexChangedCb(m_cboxLzhExtractorBackend->currentIndex());

    rootVbox->addStretch();
    return rootWidget;
}
