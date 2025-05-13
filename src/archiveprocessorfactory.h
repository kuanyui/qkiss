#ifndef ARCHIVEPROCESSORFACTORY_H
#define ARCHIVEPROCESSORFACTORY_H
#include "abstractarchiveprocessor.h"
#include "enums.h"

class ArchiveProcessorFactory
{
public:
    static AbstractArchiveProcessor* newArchiveProcessor(QKiSS::LzhDecompressorBackend backend);
};

#endif // ARCHIVEPROCESSORFACTORY_H
