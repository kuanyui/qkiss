#ifndef ARCHIVEPROCESSORCNFINFILESYSTEM_H
#define ARCHIVEPROCESSORCNFINFILESYSTEM_H

#include "abstractarchiveprocessor.h"
#include <QObject>

class ArchiveProcessorCnfInFileSystem : public AbstractArchiveProcessor
{
    Q_OBJECT
public:
    explicit ArchiveProcessorCnfInFileSystem(QObject *parent = nullptr);
    virtual QString name() const override;
    virtual QString description() const override;
    virtual bool checkAvailability() override;
    virtual QFuture<std::shared_ptr<ExtractedRawFilesMap> > extractFileAsPromise(const QString &lzhOrCnfFilePath) override;

private:
    static std::shared_ptr<ExtractedRawFilesMap> getRelatedFilesInTheSameFolder(const QString &cnfFilePath);
    std::shared_ptr<QMap<QString, QByteArray>> m_extractedFiles;
};

#endif // ARCHIVEPROCESSORCNFINFILESYSTEM_H
