#ifndef ARCHIVEPROCESSOR_H
#define ARCHIVEPROCESSOR_H

#include <QObject>
#include <QProcess>
#include <QTemporaryDir>
#include <QPromise>
#include <QFuture>
#include <QMap>
#include <QElapsedTimer>

using RawFilesMap = QMap<QString, QByteArray>;

using FileSummary = QPair<QString, int>;
using FilesSummaryList = QVector<QPair<QString, int>>;

class ArchiveProcessor : public QObject
{
    Q_OBJECT
public:
    explicit ArchiveProcessor(QObject *parent = nullptr);
    static bool checkLhaCommandAvailability();
    static FilesSummaryList getFileSizeList(const QString &lzhFilePath);
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
    /// QHash<K, V> and QMap<K, V> do NEVER remember the ordering of insertion. Use QVector<QPair<K, V>> to replace it.
    FilesSummaryList m_filesSummaryList;
    QByteArray m_buffer;
    QElapsedTimer m_elapsedTimer;
};

#endif // ARCHIVEPROCESSOR_H
