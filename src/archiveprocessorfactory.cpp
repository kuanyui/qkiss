#include "archiveprocessorfactory.h"
#include "archiveprocessorexternallha.h"
#include "archiveprocessorlibarchive.h"



AbstractArchiveProcessor *ArchiveProcessorFactory::newArchiveProcessor(QKiSS::LzhDecompressorBackend backend)
{
    switch (backend) {
#ifdef HAVE_LIBARCHIVE
    case QKiSS::LzhDecompressorBackend::Libarchive: return new ArchiveProcessorLibarchive();
#endif
    case QKiSS::LzhDecompressorBackend::ExternalLhaProcess: return new ArchiveProcessorExternalLha();
    default: return new ArchiveProcessorLibarchive();
    }
}
