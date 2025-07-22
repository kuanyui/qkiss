#ifndef ARCHIVEPROCESSORFACTORY_H
#define ARCHIVEPROCESSORFACTORY_H
#include "abstractarchiveprocessor.h"
#include "enums.h"

class ArchiveProcessorFactory
{
public:
    static std::unique_ptr<AbstractArchiveProcessor> getArchiveProcessor(QKiSS::LzhDecompressorBackend backend, QString lzhOrCnfFilePath);
    static std::unique_ptr<std::vector<std::unique_ptr<AbstractArchiveProcessor>>> getAllAvailableArchiveProcessors();

};

#endif // ARCHIVEPROCESSORFACTORY_H
