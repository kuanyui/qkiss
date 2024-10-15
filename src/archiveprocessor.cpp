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

bool ArchiveProcessor::extractLzh(const QString &lzhFilePath)
{
    if (!QFileInfo::exists(lzhFilePath)) {
        qCritical() << QStringLiteral("[ERROR] archive file path '%1' doesn't exist, abort.").arg(lzhFilePath);
        return false;
    }
    m_archiveFilePath = lzhFilePath;
    m_tmpDir = new QTemporaryDir(QDir(QDir::tempPath()).filePath("qtkiss-extracted-XXXXXX"));
    if (!m_tmpDir->isValid()) {
        emit extractFail(m_archiveFilePath, "[ERROR] Cannot create temporary folder to extracted files, aborted.");
        return false;
    }
    m_process->start("lha", { "x",   // extract
                              // "-q",   // quiet
                              "-i",   // flatten folders in target
                              "-f",   // forces overwrite without asking.
                              "-w",   // specify extract path
                              m_tmpDir->path(),   // (extract to here)
                              "--archive-kanji-code=sjis",
                              m_archiveFilePath
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
    qsizetype prefixLen {m_tmpDir->path().length() + 1};
    while (m_process->canReadLine()) {
        QString raw = QString(m_process->readLine());
        if (raw.endsWith(QStringLiteral("\t- Melted  \n"))) {
            QString absPath {raw.split("\t")[0]};
            QString relPath {absPath.mid(prefixLen)};
            qDebug() << relPath;
        }
        //qDebug() << "===> " << raw;
    }
}

void ArchiveProcessor::onQProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    if (exitStatus == QProcess::CrashExit) {
        emit extractFail(m_archiveFilePath, "Extract failed.");
        return;
    }
    QDirIterator it {
        m_tmpDir->path(),
                QDir::NoDotAndDotDot | QDir::AllEntries,
                QDirIterator::Subdirectories
    };
    auto filesMap = std::make_shared<QMap<QString, QByteArray>>();
    while (it.hasNext()) {
        QString filePath = it.next();
        QFile f { filePath };
        QFileInfo fi { f };
        QString fileNameKey = fi.fileName();
        // qDebug() << "read file: " << fileNameKey << "  -- from " << f.fileName();
        f.open(QIODevice::ReadOnly);
        QByteArray rawData = f.readAll();
        filesMap->insert(fileNameKey, rawData);
        f.close();
    }
    delete m_tmpDir;
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
