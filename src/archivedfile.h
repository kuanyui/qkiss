#ifndef ARCHIVEDFILE_H
#define ARCHIVEDFILE_H

#include <QObject>
#include <QString>
#include <QProcess>

class ArchivedFile: public QObject
{
    Q_OBJECT
public:
    ArchivedFile(QObject *parent = nullptr);
    void loadLzh(const QString &filePath);
    static bool checkLhaCommandAvailability();
private slots:
    void onProcessReadyRead();
    void onProcessFinished();
signals:
    void newLogLines();
private:
    QProcess *m_process;
};

#endif // ARCHIVEDFILE_H
