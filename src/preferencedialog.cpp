#include "preferencedialog.h"
#include "abstractarchiveprocessor.h"
#include "archiveprocessorfactory.h"
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>

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
    std::unique_ptr<std::vector<std::unique_ptr<AbstractArchiveProcessor>>> allBackends = ArchiveProcessorFactory::getAllAvailableArchiveProcessors();
    for (auto &be : *allBackends) {
        auto beInfo = be->info();
        m_cboxLzhExtractorBackend->addItem(beInfo.name, (int)beInfo.id);
        m_allAvailableLzhExtractorBackends.push_back(be);
    }
    QList<QPair<QString, QString>> fields = {
        {"application-x-executable", tr("Name")},   // Gnome
        {"gtk-info", tr("Summary")},    // Humanity
        {"extract-archive", tr("LZH Compatibility")},    // Humanity
        {"gnome-settings-default-applications", tr("Installation")},    // Humanity
        {"gimp-controller-linux-input", tr("OS Support")},  // GimpDefault
        {"drive-harddisk", tr("Disk Operation")},    // Humanity
        {"web-browser", tr("Website")}   // Humanity
    };
    QGridLayout*grid = new QGridLayout;
    for (uint r = 0; r < m_allAvailableLzhExtractorBackends.size(); r++) {
        QLabel *labelIcon = new QLabel();
        labelIcon->setPixmap(QIcon::fromTheme(fields[r].first).pixmap(16, 16));
        grid->addWidget(labelIcon, r, 0);

        QLabel *labelFieldName = new QLabel(fields[r].second);
        grid->addWidget(labelFieldName, r, 1);

        QLabel *labelfieldValue = new QLabel();
        grid->addWidget(labelfieldValue, r, 2);
    }
    vbox->addLayout(grid);
    vbox->addStretch();
    return rootWidget;
}
