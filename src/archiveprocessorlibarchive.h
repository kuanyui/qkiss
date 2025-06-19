#ifndef ARCHIVEPROCESSORLIBARCHIVE_H
#define ARCHIVEPROCESSORLIBARCHIVE_H

#ifdef HAVE_LIBARCHIVE

#include <QObject>
#include "abstractarchiveprocessor.h"

class ArchiveProcessorLibarchive : public AbstractArchiveProcessor
{
    Q_OBJECT
public:
    explicit ArchiveProcessorLibarchive(QObject *parent = nullptr);
    virtual ~ArchiveProcessorLibarchive();
    virtual Info info() const override;
    virtual bool checkAvailability() override;
    virtual QFuture<std::shared_ptr<ExtractedRawFilesMap> > extractFileAsPromise(const QString &lzhOrCnfFilePath) override;

private:
    /// Open a new thread to extracts archive file in memory, and returns QFuture in main thread.
    QFuture<std::shared_ptr<ExtractedRawFilesMap> > extractArchiveInMemory(const QByteArray &archiveData);
    static std::shared_ptr<ExtractedRawFilesMap> callLibarchiveToExtractArchiveInMemory(const QByteArray &archiveData);

};
#endif // HAVE_LIBARCHIVE

#endif // ARCHIVEPROCESSORLIBARCHIVE_H
