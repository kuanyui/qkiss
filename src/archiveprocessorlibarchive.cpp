#include "archiveprocessorlibarchive.h"


ArchiveProcessorLibarchive::ArchiveProcessorLibarchive(QObject *parent) :
    AbstractArchiveProcessor{parent}
{

}

ArchiveProcessorLibarchive::~ArchiveProcessorLibarchive()
{

}

QFuture<std::shared_ptr<ExtractedRawFilesMap> > ArchiveProcessorLibarchive::extractFileAsPromise(const QString &lzhOrCnfFilePath)
{

}
