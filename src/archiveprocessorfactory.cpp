#include "archiveprocessorfactory.h"
#include "archiveprocessorexternallha.h"
#include "archiveprocessorlibarchive.h"



AbstractArchiveProcessor *ArchiveProcessorFactory::newArchiveProcessor(Backend backend)
{
    backend = Backend::ExternalLhaProcess;
    switch (backend) {
#ifdef HAVE_LIBARCHIVE
    case Backend::Libarchive: return new ArchiveProcessorLibarchive();
#endif
    case Backend::ExternalLhaProcess: return new ArchiveProcessorExternalLha();
    default: return new ArchiveProcessorLibarchive();
    }
}
