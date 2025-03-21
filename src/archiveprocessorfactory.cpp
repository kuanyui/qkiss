#include "archiveprocessorfactory.h"
#include "archiveprocessorexternallha.h"
#include "archiveprocessorlibarchive.h"



AbstractArchiveProcessor *ArchiveProcessorFactory::newArchiveProcessor(Backend backend)
{
    switch (backend) {
    case Backend::Libarchive: return new ArchiveProcessorLibarchive();
    case Backend::ExternalLhaProcess: return new ArchiveProcessorExternalLha();
    }
}
