#include "archiveprocessorfactory.h"
#include "archiveprocessorexternallha.h"
#include "archiveprocessorexternalunar.h"
#include "archiveprocessorlibarchive.h"
#include "archiveprocessorcnfinfilesystem.h"


std::unique_ptr<AbstractArchiveProcessor> ArchiveProcessorFactory::getArchiveProcessor(QKiSS::LzhDecompressorBackend backend, QString lzhOrCnfFilePath)
{
    if (lzhOrCnfFilePath.endsWith(".CNF", Qt::CaseInsensitive)) {
        return std::make_unique<ArchiveProcessorCnfInFileSystem>();
    }
    switch (backend) {
    case QKiSS::LzhDecompressorBackend::ExternalUnarProcess: return std::make_unique<ArchiveProcessorExternalUnar>();
    case QKiSS::LzhDecompressorBackend::ExternalLhaProcess: return std::make_unique<ArchiveProcessorExternalLha>();
#ifdef HAVE_LIBARCHIVE
    case QKiSS::LzhDecompressorBackend::Libarchive: return std::make_unique<ArchiveProcessorLibarchive>();
#endif
    default: return std::make_unique<ArchiveProcessorExternalLha>();
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

