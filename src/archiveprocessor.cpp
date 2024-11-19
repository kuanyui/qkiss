#include "archiveprocessor.h"

#include <QDirIterator>

ArchiveProcessor::ArchiveProcessor(QObject *parent)
    : QObject{parent},
      m_process{new QProcess(this)}
{
    connect(m_process, &QProcess::readyRead, this, &ArchiveProcessor::onQProcessReadyRead);
    connect(m_process, &QProcess::finished, this, &ArchiveProcessor::onQProcessFinished);
    // connect(m_process, &QProcess::errorOccurred, this, &ArchiveProcessor::onProcessErrorOccurred);
}

bool ArchiveProcessor::checkLhaCommandAvailability()
{
    QProcess p {};
    p.start("lha");
    p.waitForFinished();
    return p.exitCode() == 0;
}


FilesSummaryList ArchiveProcessor::getFileSizeList(const QString &lzhFilePath)
{
    QStringList lines;
    bool ok;
    QProcess process {};
    connect(&process, &QProcess::readyRead, [&lines, &process](){
        while (process.canReadLine()) {
            lines.append(QString(process.readLine()));
            qDebug() << lines.last();
        }
    });
    connect(&process, &QProcess::finished, [&ok](int exitCode, QProcess::ExitStatus exitStatus){
        Q_UNUSED(exitCode);
        ok = exitStatus == QProcess::NormalExit;
    });
    process.start("lha",
                  { "l",  // list files
                    "--archive-kanji-code=sjis",
                    lzhFilePath
                  });
    process.waitForFinished();
    FilesSummaryList fileSizeSummary;
    if (!ok || lines.isEmpty()) { return fileSizeSummary; }
    // Start to parse
    /* @example
PERMISSION  UID  GID      SIZE  RATIO     STAMP           NAME
---------- ----------- ------- ------ ------------ --------------------
[MS-DOS]                 37608  21.3% Oct  8  2009 FA1280B/fa120001.cel
[MS-DOS]                 59440  14.4% Oct  8  2009 FA1280B/fa120002.cel
[MS-DOS]                 59440  14.3% Oct  8  2009 FA1280B/fa120003.cel
---------- ----------- ------- ------ ------------ --------------------
 Total       709 files 113297883  38.9% Oct  9  2009
     */
    int sizeTitleIdx = lines[0].indexOf(QStringLiteral("SIZE"));
    int nameTitleIdx = lines[0].indexOf(QStringLiteral("NAME"));
    int sizeColIdxFrom, sizeColWidth;
    int nameColIdxFrom, nameColWidth;
    QStringList seperators = lines[1].split(" ");
    int curColFrom = 0;
    for (const QString &sep : seperators) {
        int curColWidth = sep.length();
        int curColTo = curColFrom + curColWidth;
        if (sizeTitleIdx >= curColFrom && sizeTitleIdx <= curColTo) {
            sizeColIdxFrom = curColFrom;
            sizeColWidth = curColWidth;
        } else if (nameTitleIdx >= curColFrom && nameTitleIdx <= curColTo) {
            nameColIdxFrom = curColFrom;
            nameColWidth = curColWidth;
        }
        curColFrom += curColWidth + 1;  // +1 is the space between each ---
    }
    for (int i = 2; i < lines.length() - 2; i++) {
        const QString& line = lines[i];
        int byte = line.mid(sizeColIdxFrom, sizeColWidth).trimmed().toInt();
        QString relPath = line.mid(nameColIdxFrom, nameColWidth).trimmed();
        QFileInfo fileInfo(relPath);
        QString fileName = fileInfo.fileName();
        qDebug() << "List file name: " << fileName << byte;
        fileSizeSummary.append(FileSummary(fileName, byte));
    }
    return fileSizeSummary;
}

bool ArchiveProcessor::extractLzh(const QString &lzhFilePath)
{
    if (!QFileInfo::exists(lzhFilePath)) {
        qCritical() << QStringLiteral("[ERROR] archive file path '%1' doesn't exist, abort.").arg(lzhFilePath);
        return false;
    }
    m_elapsedTimer.start();
    m_archiveFilePath = lzhFilePath;
    m_filesSummaryList = getFileSizeList(lzhFilePath);
    m_buffer.clear();
    m_process->start("lha",
                     { "-p",   // print the content of the archived files to stdout.
                       "-q",   // quiet (use along with `-p`; don't display filename header)
                       lzhFilePath
                     });
    return true;
}

QFuture<std::shared_ptr<RawFilesMap> > ArchiveProcessor::extractFileAsPromise(const QString &lzhFilePath)
{
    QPromise<std::shared_ptr<RawFilesMap>> *promise = new QPromise<std::shared_ptr<RawFilesMap>>();
    QFuture<std::shared_ptr<RawFilesMap>> future = promise->future();
    promise->start();
    ArchiveProcessor *process = new ArchiveProcessor();
    process->extractLzh(lzhFilePath);
    QObject::connect(process, &ArchiveProcessor::extractFail, [process, promise](QString compressedFilePath, QString errMsg) {
        Q_UNUSED(compressedFilePath);
        promise->setException(std::make_exception_ptr(
            std::runtime_error(QString("Process failed with exit code: %1").arg(errMsg).toStdString())
        ));
        delete promise;
        process->deleteLater();
    });
    QObject::connect(process, &ArchiveProcessor::extractSuccess, [process, promise](QString compressedFilePath, std::shared_ptr<RawFilesMap> extractedRawFiles) {
        Q_UNUSED(compressedFilePath);
        promise->addResult(extractedRawFiles);
        promise->finish();
        delete promise;
        process->deleteLater();
    });
    return future;
}

void ArchiveProcessor::onQProcessReadyRead()
{
    QByteArray tmp = m_process->readAll();
    // qDebug() << "QProcess readyRead";
    m_buffer.append(tmp);
}

void ArchiveProcessor::onQProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    m_buffer.append(m_process->readAll());
    qDebug() << "QProcess finished, buffer size = " << m_buffer.size();
    if (exitStatus == QProcess::CrashExit) {
        emit extractFail(m_archiveFilePath, "Extract failed.");
        return;
    }
    auto filesMap = std::make_shared<QMap<QString, QByteArray>>();
    int pos = 0;
    foreach (const auto fileSummary, m_filesSummaryList)  {
        QString fileName = fileSummary.first;
        uint byteSize = static_cast<qsizetype>(static_cast<uint>(fileSummary.second));
        QByteArray data = m_buffer.sliced(pos, byteSize);
        // qDebug() << "=====================================";
        // qDebug() << fileName << byteSize;
        // qDebug() << "=====================================";
        // qDebug() << data;
        pos += byteSize;
        filesMap->insert(fileName, data);
    }
    qDebug() << "[LOG][Benchmark][ArchiveProcessor] Decompression spent " << m_elapsedTimer.elapsed() << " ms";
    m_buffer.clear();
    emit extractSuccess(m_archiveFilePath, filesMap);
    return;
}

void ArchiveProcessor::onProcessErrorOccurred(QProcess::ProcessError error)
{
    switch (error) {
    case QProcess::FailedToStart: emit extractFail(m_archiveFilePath, "LAH process failed to start."); return;
    case QProcess::Crashed: emit extractFail(m_archiveFilePath, "LAH process crashed."); return;
    case QProcess::Timedout: emit extractFail(m_archiveFilePath, "LAH process timeout."); return;
    case QProcess::WriteError:
    case QProcess::ReadError:
    case QProcess::UnknownError:
        return;
    }
}
