#include "preferencedialog.h"
#include "abstractarchiveprocessor.h"
#include "archiveprocessorfactory.h"
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QApplication>

using namespace Qt::StringLiterals;

PreferenceDialog::PreferenceDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Preferences"));
    setModal(true);
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

namespace {
struct IconTextField {
    QString iconName;
    QString fieldName;
    QLabel *labelFieldValue;
};
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
        m_allAvailableLzhExtractorBackends.push_back(std::move(be));
    }
    QLabel *labelName = new QLabel();
    QLabel *labelSummary = new QLabel();
    QLabel *labelLzhCompa = new QLabel();
    QLabel *labelInstall = new QLabel();
    QLabel *labelOsSupport = new QLabel();
    QLabel *labelDiskOp = new QLabel();
    QLabel *labelWebsite = new QLabel();
    QList<IconTextField> fields = {
        {"application-x-executable",            tr("Name"),              labelName},       // Gnome
        {"gtk-info",                            tr("Summary"),           labelSummary},    // Humanity
        {"extract-archive",                     tr("LZH Compatibility"), labelLzhCompa},   // Humanity
        {"gnome-settings-default-applications", tr("Installation"),      labelInstall},    // Humanity
        {"gimp-controller-linux-input",         tr("OS Support"),        labelOsSupport},  // GimpDefault
        {"drive-harddisk",                      tr("Disk Operation"),    labelDiskOp},     // Humanity
        {"web-browser",                         tr("Website"),           labelWebsite}     // Humanity
    };
    QGridLayout *grid = new QGridLayout;
    for (uint r = 0; r < fields.size(); r++) {
        QFont defaultFont = QApplication::font();
        QFontMetrics metrics(defaultFont);
        int h = metrics.height();

        QLabel *labelIcon = new QLabel();
        labelIcon->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
        labelIcon->setPixmap(QIcon::fromTheme(fields[r].iconName).pixmap(h, h));
        grid->addWidget(labelIcon, r, 0);

        QLabel *labelFieldName = new QLabel(fields[r].fieldName);
        labelFieldName->setStyleSheet("font-weight: bold; color: gray;"_L1);
        grid->addWidget(labelFieldName, r, 1);

        fields[r].labelFieldValue->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
        grid->addWidget(fields[r].labelFieldValue, r, 2);
    }
    auto callback = [=, this](int index) {
        const auto &backend = m_allAvailableLzhExtractorBackends[index];
        const auto &info = backend->info();
        labelName->setText(info.name);
        labelSummary->setText(info.summary);
        labelLzhCompa->setText(info.lzhCompatibility);
        labelInstall->setText(info.installation);
        labelOsSupport->setText(info.osSupport);
        labelDiskOp->setText(info.diskOperation);
        labelWebsite->setText(info.website);
    };
    connect(m_cboxLzhExtractorBackend, &QComboBox::currentIndexChanged, this, callback);
    callback(m_cboxLzhExtractorBackend->currentIndex());

    vbox->addLayout(grid);
    vbox->addStretch();
    return rootWidget;
}
