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

#ifndef ARCHIVEPROCESSOREXTERNALUNAR_H
#define ARCHIVEPROCESSOREXTERNALUNAR_H

#include "abstractarchiveprocessor.h"
#include <QObject>
#include <QProcess>
#include <QTemporaryDir>

class ArchiveProcessorExternalUnar : public AbstractArchiveProcessor
{
    Q_OBJECT
public:
    explicit ArchiveProcessorExternalUnar(QObject *parent = nullptr);
    virtual ~ArchiveProcessorExternalUnar();
    virtual Info info() const override;
    virtual bool checkAvailability() override;
    virtual QFuture<std::shared_ptr<ExtractedRawFilesMap> > extractFileAsPromise(const QString &lzhOrCnfFilePath) override;

private:
    static const char *getEncodingParam();
    /// The `unar` to run. On Windows, it is the `unar\unar.exe` that comes with the app, when it exists.
    /// Otherwise, it is `unar` in PATH.
    static QString getUnarProgram();
    bool startUnarProcessToExtractLzh(const QString &lzhFilePath);
    QStringList getAllFileNamesInTmpDirRecursively();
    std::shared_ptr<QMap<QString, QByteArray>> getAllFileRawContentsInTmpDirRecursively();
    /** Try to get nested (unextracted-yet) LZH file in temp dir. If not found, return empty string. */
    QString getNextUnextractedLzhPath();

signals:
    void extractSuccess(QString compressedFilePath);
    void extractFail(QString compressedFilePath, QString errMsg);

private slots:
    void onQProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onQProcessErrorOccurred(QProcess::ProcessError error);
    void onQProcessStdout();
private:
    static std::shared_ptr<ExtractedRawFilesMap> getRelatedFilesInTheSameFolder(const QString &cnfFilePath);
    QProcess *m_process;
    QString m_archiveFilePath;
    FilesSummaryList m_filesSummaryList;
    QByteArray m_buffer;
    QElapsedTimer m_elapsedTimer;
    std::unique_ptr<QTemporaryDir> m_tmpDir;
    /// nested LZH (LZH file in in main LZH file)
    QString m_tmpNestedLzhFilePath;
};

#endif // ARCHIVEPROCESSOREXTERNALUNAR_H
