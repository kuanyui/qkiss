#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include <QDialog>
#include <QObject>
#include <QWidget>
#include <QComboBox>


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
};

#endif // PREFERENCEDIALOG_H
