#ifndef ARCHIVEPROCESSORLIBARCHIVE_H
#define ARCHIVEPROCESSORLIBARCHIVE_H

#include <QObject>
#include "abstractarchiveprocessor.h"

class ArchiveProcessorLibarchive : public AbstractArchiveProcessor
{
    Q_OBJECT
public:
    explicit ArchiveProcessorLibarchive(QObject *parent = nullptr);
    virtual ~ArchiveProcessorLibarchive();


    // AbstractArchiveProcessor interface
public:
    virtual QFuture<std::shared_ptr<ExtractedRawFilesMap> > extractFileAsPromise(const QString &lzhOrCnfFilePath) override;

private:
    /// Open a new thread to extracts archive file in memory, and returns QFuture in main thread.
    QFuture<std::shared_ptr<ExtractedRawFilesMap> > extractArchiveInMemory(const QByteArray &archiveData);
    static std::shared_ptr<ExtractedRawFilesMap> callLibarchiveToExtractArchiveInMemory(const QByteArray &archiveData);
};

#endif // ARCHIVEPROCESSORLIBARCHIVE_H
