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

#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include <QDialog>
#include <QObject>
#include <QWidget>
#include <QDialogButtonBox>
#include "enums.h"
#include "abstractarchiveprocessor.h"
#include "zcombobox.h"


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
    void syncCopiedModelToUiComponents();
    void syncUiComponentsToCopiedModel();
    void refreshDirtyState();

private:
    /// Read QSettings file to model
    void readSettingsToModel();
    /// Write model to QSettings file
    void writeModelToSettings();
    QWidget *createUiTab();
    QWidget *createBackendTab();
    QDialogButtonBox* m_dialogButtons;
    ZComboBox *m_cboxLzhExtractorBackend;
    std::vector<std::unique_ptr<AbstractArchiveProcessor>> m_allAvailableLzhExtractorBackends;
    PreferenceDialogModel::Root m_appliedModel;
    PreferenceDialogModel::Root m_copiedModel;
    bool m_modelDirty;
};

#endif // PREFERENCEDIALOG_H
