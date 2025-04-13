#include "archiveprocessorfactory.h"
#include "archiveprocessorexternallha.h"
#include "archiveprocessorlibarchive.h"



AbstractArchiveProcessor *ArchiveProcessorFactory::newArchiveProcessor(Backend backend)
{
    backend = Backend::ExternalLhaProcess;
    switch (backend) {
    case Backend::Libarchive: return new ArchiveProcessorLibarchive();
    case Backend::ExternalLhaProcess: return new ArchiveProcessorExternalLha();
    default: return new ArchiveProcessorLibarchive();
    }
}
