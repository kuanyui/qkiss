#include "preferencedialog.h"
#include "abstractarchiveprocessor.h"
#include "archiveprocessorfactory.h"
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

PreferenceDialog::PreferenceDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Settings"));

    auto *tabs = new QTabWidget(this);
    // tabs->addTab(createUiTab(), tr("User Interface"));
    tabs->addTab(createBackendTab(), tr("LZH Extractor Backend"));

    auto *buttons = new QDialogButtonBox(
                QDialogButtonBox::Apply |
                QDialogButtonBox::Ok    |
                QDialogButtonBox::Cancel,
                Qt::Horizontal, this);

    connect(buttons->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &PreferenceDialog::applySettings);

    connect(buttons, &QDialogButtonBox::accepted, this, &PreferenceDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &PreferenceDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(tabs);
    layout->addWidget(buttons);
}

void PreferenceDialog::applySettings()
{

}

QWidget *PreferenceDialog::createUiTab()
{
    auto *w = new QWidget(this);
    auto *l = new QVBoxLayout(w);

    l->addStretch();
    return w;
}

QWidget *PreferenceDialog::createBackendTab()
{
    auto *rootWidget = new QWidget(this);
    auto *vbox = new QVBoxLayout(rootWidget);
    m_cboxLzhExtractorBackend = new QComboBox(rootWidget);
    vbox->addWidget(m_cboxLzhExtractorBackend);
    auto allBackends = ArchiveProcessorFactory::getAllAvailableArchiveProcessors();
    for (auto &be : *allBackends) {
        auto beInfo = be->info();
        m_cboxLzhExtractorBackend->addItem(beInfo.name, (int)beInfo.id);
    }
    vbox->addStretch();
    return rootWidget;
}
