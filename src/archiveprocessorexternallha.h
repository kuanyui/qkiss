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

#ifndef ARCHIVEPROCESSOREXTERNALLHA_H
#define ARCHIVEPROCESSOREXTERNALLHA_H

#include <QObject>
#include <QProcess>
#include <QTemporaryDir>
#include "abstractarchiveprocessor.h"

class ArchiveProcessorExternalLha : public AbstractArchiveProcessor
{
    Q_OBJECT
public:
    explicit ArchiveProcessorExternalLha(QObject *parent = nullptr);
    virtual ~ArchiveProcessorExternalLha();
    virtual Info info() const override;
    virtual bool checkAvailability() override;
    /// This function merely extract a LZH file, doesn't do any other checks.
    /// TODO: Should be able to accept a CNF file. Should rename this function.
    virtual QFuture<std::shared_ptr<ExtractedRawFilesMap>> extractFileAsPromise(const QString &lzhOrCnfFilePath) override;
private:
    static FilesSummaryList getFileSizeList(const QString &lzhFilePath);
    static const char *getEncodingParam();
    bool callExternalProgramOrLibraryToExtractLzh(const QString &lzhFilePath);
signals:
    void extractSuccess(QString compressedFilePath, std::shared_ptr<ExtractedRawFilesMap> extractedRawFiles);
    void extractFail(QString compressedFilePath, QString errMsg);

private slots:
    void onQProcessReadyRead();
    void onQProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessErrorOccurred(QProcess::ProcessError error);
private:
    static std::shared_ptr<ExtractedRawFilesMap> getRelatedFilesInTheSameFolder(const QString &cnfFilePath);   /// TODO: Move to abstract class
    QProcess *m_process;
    QString m_archiveFilePath;
    FilesSummaryList m_filesSummaryList;
    QByteArray m_buffer;
    QElapsedTimer m_elapsedTimer;
    std::shared_ptr<QMap<QString, QByteArray>> m_extractedFiles;
    QTemporaryDir *m_tmpDir;
    /// If nested LZH file is existed, they will be written into system temporary directory (ex: /tmp/)
    QStringList m_tmpNestedLzhFilePathList;
};

#endif // ARCHIVEPROCESSOREXTERNALLHA_H
