#ifndef ARCHIVEPROCESSOREXTERNALUNAR_H
#define ARCHIVEPROCESSOREXTERNALUNAR_H

#include "abstractarchiveprocessor.h"
#include <QObject>
#include <QProcess>
#include <QTemporaryDir>

class ArchiveProcessorExternalUnar : public AbstractArchiveProcessor
{
    Q_OBJECT
public:
    explicit ArchiveProcessorExternalUnar(QObject *parent = nullptr);
    virtual ~ArchiveProcessorExternalUnar();
    virtual QString name() const override;
    virtual QString description() const override;
    virtual bool checkAvailability() override;
    virtual QFuture<std::shared_ptr<ExtractedRawFilesMap> > extractFileAsPromise(const QString &lzhOrCnfFilePath) override;

private:
    static const char *getEncodingParam();
    bool startUnarProcessToExtractLzh(const QString &lzhFilePath);
    QStringList getAllFileNamesInTmpDirRecursively();
    std::shared_ptr<QMap<QString, QByteArray>> getAllFileRawContentsInTmpDirRecursively();
    /** Try to get nested (unextracted-yet) LZH file in temp dir. If not found, return empty string. */
    QString getNextUnextractedLzhPath();

signals:
    void extractSuccess(QString compressedFilePath);
    void extractFail(QString compressedFilePath, QString errMsg);

private slots:
    void onQProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onQProcessErrorOccurred(QProcess::ProcessError error);
    void onQProcessStdout();
private:
    static std::shared_ptr<ExtractedRawFilesMap> getRelatedFilesInTheSameFolder(const QString &cnfFilePath);
    QProcess *m_process;
    QString m_archiveFilePath;
    FilesSummaryList m_filesSummaryList;
    QByteArray m_buffer;
    QElapsedTimer m_elapsedTimer;
    std::unique_ptr<QTemporaryDir> m_tmpDir;
    /// nested LZH (LZH file in in main LZH file)
    QString m_tmpNestedLzhFilePath;
};

#endif // ARCHIVEPROCESSOREXTERNALUNAR_H
