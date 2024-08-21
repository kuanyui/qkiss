#ifndef ARCHIVEDFILE_H
#define ARCHIVEDFILE_H

#include <QObject>
#include <QString>
#include <QProcess>
#include <QTemporaryDir>

class ArchivedFile: public QObject
{
    Q_OBJECT
public:
    ArchivedFile(QObject *parent = nullptr);
    bool loadLzh(const QString &filePath);
    static bool checkLhaCommandAvailability();
private slots:
    void onProcessReadyRead();
    void onProcessFinished();
signals:
    void newLogLine(QString line);
private:
    QProcess *m_process;
    QTemporaryDir *m_tmpDir;
};

#endif // ARCHIVEDFILE_H
