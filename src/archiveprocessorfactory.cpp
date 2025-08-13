/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2024 - 2025 ono ono (kuanyui) <azazabc123@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

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

