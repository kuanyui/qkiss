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

#ifndef ARCHIVEPROCESSORCNFINFILESYSTEM_H
#define ARCHIVEPROCESSORCNFINFILESYSTEM_H

#include "abstractarchiveprocessor.h"
#include <QObject>

class ArchiveProcessorCnfInFileSystem : public AbstractArchiveProcessor
{
    Q_OBJECT
public:
    explicit ArchiveProcessorCnfInFileSystem(QObject *parent = nullptr);
    virtual Info info() const override;
    virtual bool checkAvailability() override;
    virtual QFuture<std::shared_ptr<ExtractedRawFilesMap> > extractFileAsPromise(const QString &lzhOrCnfFilePath) override;

private:
    static std::shared_ptr<ExtractedRawFilesMap> getRelatedFilesInTheSameFolder(const QString &cnfFilePath);
    std::shared_ptr<QMap<QString, QByteArray>> m_extractedFiles;
};

#endif // ARCHIVEPROCESSORCNFINFILESYSTEM_H
