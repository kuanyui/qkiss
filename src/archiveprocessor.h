#ifndef ARCHIVEPROCESSOR_H
#define ARCHIVEPROCESSOR_H

#include <QObject>
#include <QProcess>
#include <QTemporaryDir>
#include <QPromise>
#include <QFuture>

using RawFilesMap = QMap<QString, QByteArray>;

class ArchiveProcessor : public QObject
{
    Q_OBJECT
public:
    explicit ArchiveProcessor(QObject *parent = nullptr);
    static bool checkLhaCommandAvailability();
    bool extractLzh(const QString &lzhFilePath);
    static QFuture<std::shared_ptr<RawFilesMap>> extractFileAsPromise(const QString &lzhFilePath);
signals:
    void extractSuccess(QString compressedFilePath, std::shared_ptr<RawFilesMap> extractedRawFiles);
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
