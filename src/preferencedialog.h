#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include <QDialog>
#include <QObject>
#include <QWidget>
#include <QComboBox>
#include "enums.h"
#include "abstractarchiveprocessor.h"

class PreferenceDialog : public QDialog
{
    Q_OBJECT
public:
    PreferenceDialog(QWidget *parent = nullptr);

private slots:
    void applySettings();

private:
    QWidget *createUiTab();
    QWidget *createBackendTab();
    QComboBox *m_cboxLzhExtractorBackend;
    std::vector<std::unique_ptr<AbstractArchiveProcessor>> m_allAvailableLzhExtractorBackends;
};

#endif // PREFERENCEDIALOG_H
