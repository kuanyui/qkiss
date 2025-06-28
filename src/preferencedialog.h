#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include <QDialog>
#include <QObject>
#include <QWidget>
#include <QComboBox>
#include <QDialogButtonBox>
#include "enums.h"
#include "abstractarchiveprocessor.h"


namespace PreferenceDialogModel {
struct LzhBackend {
    bool operator==(const LzhBackend&) const = default;
    QKiSS::LzhDecompressorBackend lzhBackend;
};
struct Files {
    bool operator==(const Files&) const = default;
    QKiSS::Encoding encoding;
    /// In CNF's CEL filename; filenames in LZH.
    QKiSS::Case caseSensitivity;
    // TODO: int recentFilesMaxAmount;
    // TODO: int recentDirsMaxAmount;
};
struct Ui {
    bool operator==(const Ui&) const = default;
    QString language;
};

struct Root {
    bool operator==(const Root&) const = default;
    LzhBackend lzhBackend;
    Files files;
    Ui ui;
};
}

class PreferenceDialog : public QDialog
{
    Q_OBJECT

public:
    PreferenceDialog(QWidget *parent = nullptr);
    ~PreferenceDialog();
private slots:
    void updateUiComponentByCopiedModel();

private:
    /// Read QSettings file to model
    void readSettingsToModel();
    /// Write model to QSettings file
    void writeModelToSettings();
    QWidget *createUiTab();
    QWidget *createBackendTab();
    QDialogButtonBox* m_dialogButtons;
    QComboBox *m_cboxLzhExtractorBackend;
    std::vector<std::unique_ptr<AbstractArchiveProcessor>> m_allAvailableLzhExtractorBackends;
    PreferenceDialogModel::Root m_appliedModel;
    PreferenceDialogModel::Root m_copiedModel;
    bool m_modelDirty;
};

#endif // PREFERENCEDIALOG_H
