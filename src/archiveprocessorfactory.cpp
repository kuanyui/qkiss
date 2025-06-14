#include "archiveprocessorfactory.h"
#include "archiveprocessorexternallha.h"
#include "archiveprocessorexternalunar.h"
#include "archiveprocessorlibarchive.h"



AbstractArchiveProcessor *ArchiveProcessorFactory::newArchiveProcessor(QKiSS::LzhDecompressorBackend backend, QString lzhOrCnfFilePath)
{
    return new ArchiveProcessorExternalUnar();
    if (lzhOrCnfFilePath.endsWith(".CNF")) {
        return new ArchiveProcessorExternalUnar();
    }
    switch (backend) {
    case QKiSS::LzhDecompressorBackend::ExternalUnarProcess: return new ArchiveProcessorExternalUnar();
    case QKiSS::LzhDecompressorBackend::ExternalLhaProcess: return new ArchiveProcessorExternalLha();
#ifdef HAVE_LIBARCHIVE
    case QKiSS::LzhDecompressorBackend::Libarchive: return new ArchiveProcessorLibarchive();
#endif
    default: return new ArchiveProcessorExternalLha();
    }
}
