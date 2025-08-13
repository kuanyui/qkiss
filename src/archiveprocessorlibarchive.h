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

#ifndef ARCHIVEPROCESSORLIBARCHIVE_H
#define ARCHIVEPROCESSORLIBARCHIVE_H

#ifdef HAVE_LIBARCHIVE

#include <QObject>
#include "abstractarchiveprocessor.h"

class ArchiveProcessorLibarchive : public AbstractArchiveProcessor
{
    Q_OBJECT
public:
    explicit ArchiveProcessorLibarchive(QObject *parent = nullptr);
    virtual ~ArchiveProcessorLibarchive();
    virtual Info info() const override;
    virtual bool checkAvailability() override;
    virtual QFuture<std::shared_ptr<ExtractedRawFilesMap> > extractFileAsPromise(const QString &lzhOrCnfFilePath) override;

private:
    /// Open a new thread to extracts archive file in memory, and returns QFuture in main thread.
    QFuture<std::shared_ptr<ExtractedRawFilesMap> > extractArchiveInMemory(const QByteArray &archiveData);
    static std::shared_ptr<ExtractedRawFilesMap> callLibarchiveToExtractArchiveInMemory(const QByteArray &archiveData);

};
#endif // HAVE_LIBARCHIVE

#endif // ARCHIVEPROCESSORLIBARCHIVE_H
