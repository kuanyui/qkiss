#include "archiveprocessorfactory.h"
#include "archiveprocessorexternallha.h"
#include "archiveprocessorexternalunar.h"
#include "archiveprocessorlibarchive.h"
#include "archiveprocessorcnfinfilesystem.h"


AbstractArchiveProcessor *ArchiveProcessorFactory::newArchiveProcessor(QKiSS::LzhDecompressorBackend backend, QString lzhOrCnfFilePath)
{
    if (lzhOrCnfFilePath.endsWith(".CNF", Qt::CaseInsensitive)) {
        return new ArchiveProcessorCnfInFileSystem();
    }
    return new ArchiveProcessorExternalUnar();
    switch (backend) {
    case QKiSS::LzhDecompressorBackend::ExternalUnarProcess: return new ArchiveProcessorExternalUnar();
    case QKiSS::LzhDecompressorBackend::ExternalLhaProcess: return new ArchiveProcessorExternalLha();
#ifdef HAVE_LIBARCHIVE
    case QKiSS::LzhDecompressorBackend::Libarchive: return new ArchiveProcessorLibarchive();
#endif
    default: return new ArchiveProcessorExternalLha();
    }
}

std::unique_ptr<std::vector<std::unique_ptr<AbstractArchiveProcessor> > > ArchiveProcessorFactory::getAllAvailableArchiveProcessors()
{
    std::unique_ptr<std::vector<std::unique_ptr<AbstractArchiveProcessor> > > vec = std::make_unique<std::vector<std::unique_ptr<AbstractArchiveProcessor> > >();
    vec->push_back(std::make_unique<ArchiveProcessorExternalLha>());
    vec->push_back(std::make_unique<ArchiveProcessorExternalUnar>());
#ifdef HAVE_LIBARCHIVE
    vec->push_back(std::make_unique<ArchiveProcessorLibarchive>());
#endif
    return vec;
}

