#ifndef ARCHIVEPROCESSORFACTORY_H
#define ARCHIVEPROCESSORFACTORY_H
#include "abstractarchiveprocessor.h"

class ArchiveProcessorFactory
{
public:
    enum class Backend {
        Libarchive,
        ExternalLhaProcess,
    };
    static AbstractArchiveProcessor* newArchiveProcessor(Backend backend);
};

#endif // ARCHIVEPROCESSORFACTORY_H
