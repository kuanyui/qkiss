#ifndef ARCHIVEPROCESSOR_H
#define ARCHIVEPROCESSOR_H

#include <QObject>
#include <QProcess>
#include <QTemporaryDir>

class ArchiveProcessor : public QObject
{
    Q_OBJECT
public:
    explicit ArchiveProcessor(QObject *parent = nullptr);
    static bool checkLhaCommandAvailability();
    bool extractLzh(const QString &lzhFilePath);
signals:
    void extractSuccess(QString compressedFilePath, std::shared_ptr<QMap<QString, QByteArray>> extractedRawFiles);
    void extractFail(QString compressedFilePath, QString errMsg);

private slots:
    void onQProcessReadyRead();
    void onQProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessErrorOccurred(QProcess::ProcessError error);
private:
    QProcess *m_process;
    QString m_archiveFilePath;
    QTemporaryDir *m_tmpDir;
};

#endif // ARCHIVEPROCESSOR_H
