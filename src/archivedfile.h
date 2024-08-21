#ifndef ARCHIVEDFILE_H
#define ARCHIVEDFILE_H

#include <QObject>
#include <QString>
#include <QProcess>
#include <QHash>
#include <QTemporaryDir>

class ArchivedFile: public QObject
{
    Q_OBJECT
public:
    ArchivedFile(QObject *parent = nullptr);
    bool loadLzh(const QString &filePath, Qt::CaseSensitivity caseSensitivity);
    static bool checkLhaCommandAvailability();
private slots:
    void onProcessReadyRead();
    void onProcessFinished();
signals:
    void newLogLine(QString line);
private:
    Qt::CaseSensitivity m_caseSensitivity;
    QProcess *m_process;
    QTemporaryDir *m_tmpDir;
    QHash<QString, QByteArray> *m_files;
    QString m_archiveFilePath;
};

#endif // ARCHIVEDFILE_H
