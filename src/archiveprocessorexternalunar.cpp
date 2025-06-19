#include "archiveprocessorexternalunar.h"
#include "configfile.h"

#include <QDirIterator>
#include <QTemporaryDir>
using namespace Qt::Literals::StringLiterals;

#define argn(num) arg(QString::number(num))

ArchiveProcessorExternalUnar::ArchiveProcessorExternalUnar(QObject *parent) :
    AbstractArchiveProcessor{parent},
    m_process{new QProcess(this)}
{
    // NOTE: Due to indentation bug of QtCreator so move this shit into here instead of ctor member initializer list...
    connect(m_process, &QProcess::finished, this, &ArchiveProcessorExternalUnar::onQProcessFinished);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &ArchiveProcessorExternalUnar::onQProcessStdout);
    // connect(m_process, &QProcess::errorOccurred, this, &ArchiveProcessorExternalUnar::onQProcessErrorOccurred);
}

ArchiveProcessorExternalUnar::~ArchiveProcessorExternalUnar()
{
}

AbstractArchiveProcessor::Info ArchiveProcessorExternalUnar::info() const
{
    return Info {
        .id = QKiSS::LzhDecompressorBackend::ExternalUnarProcess,
        .name = tr("Unar (Unarchiver)"),
        .summary = tr("An extractor specially focused on archives of old formats containing non UTF-8 encoding file names."),
        .lzhCompatibility = tr("Pretty Good"),
        .installation = tr("Nativly supports macOS. Available on most of Linux distros. Supports Windows"),
        .osSupport = tr("External program; manual installation is needed."),
        .diskOperation = tr("All extractions will do writing operations to disk; may shorten the lifespan of SSD."),
        .website = tr("<a href=\"https://theunarchiver.com/command-line\">Official Site</a>, <a href='https://github.com/MacPaw/unar'>Source Code</a>, ")
    };
}

bool ArchiveProcessorExternalUnar::checkAvailability()
{
    QProcess p {};
    p.start("unar", {"-h"});
    p.waitForFinished();
    return p.exitCode() == 0;
}

QFuture<std::shared_ptr<ExtractedRawFilesMap> > ArchiveProcessorExternalUnar::extractFileAsPromise(const QString &lzhOrCnfFilePath)
{
    m_tmpDir = std::make_unique<QTemporaryDir>(generateTempDirPath());
    QPromise<std::shared_ptr<ExtractedRawFilesMap>> *promise = new QPromise<std::shared_ptr<ExtractedRawFilesMap>>();
    QFuture<std::shared_ptr<ExtractedRawFilesMap>> future = promise->future();
    QFileInfo fileInfo {lzhOrCnfFilePath};
    QString fileName = fileInfo.fileName();

    promise->start();

    if (!checkAvailability()) {
        qCritical().noquote() << tr("External program Unar is not installed or not found.");
        promise->setException(std::make_exception_ptr(
                                  std::runtime_error("External program Unar is not installed or not found.")
                                  ));
        promise->finish();
        delete promise;
    }

    QMetaObject::Connection conn1, conn2;
    conn1 = QObject::connect(this, &ArchiveProcessorExternalUnar::extractFail, [=](QString errMsg) {
        qCritical().noquote() << tr("Extracting LZH file \"%1\" failed. Abort.").arg(fileName);
        promise->setException(std::make_exception_ptr(
                                  std::runtime_error("Process failed with exit code: %1"_L1.arg(errMsg).toStdString())
                                  ));
        QObject::disconnect(conn1);
        QObject::disconnect(conn2);
        promise->finish();
        delete promise;
    });
    conn2 = QObject::connect(this, &ArchiveProcessorExternalUnar::extractSuccess, [=, this]() {
        std::shared_ptr<QMap<QString, QByteArray>> extractedRawFiles = getAllFileRawContentsInTmpDirRecursively();
        qInfo().noquote() << tr("LZH file \"%1\" is extracted successfully. Got %2 file(s) in it").arg(fileName).arg(extractedRawFiles->size());
        promise->addResult(extractedRawFiles);
        QObject::disconnect(conn1);
        QObject::disconnect(conn2);
        promise->finish();
        delete promise;
    });
    qInfo().noquote() << tr("Trying to extract LZH file \"%1\"...").arg(fileName);
    startUnarProcessToExtractLzh(lzhOrCnfFilePath);
    return future;
}

const char *ArchiveProcessorExternalUnar::getEncodingParam()
{
    ConfigFile &cfg = ConfigFile::instance();
    switch (cfg.encoding()) {
        case QKiSS::Encoding::SJIS: return "Shift_JIS";
        case QKiSS::Encoding::EUC_JP: return "EUC-JP";
        case QKiSS::Encoding::UTF8: return "EUC-JP";
        case QKiSS::Encoding::ASCII: return "EUC-JP";
        default: return "Shift_JIS";
    }
}


bool ArchiveProcessorExternalUnar::startUnarProcessToExtractLzh(const QString &lzhFilePath)
{
    if (!QFileInfo::exists(lzhFilePath)) {
        qCritical() << tr("Archive file path '%1' doesn't exist, abort.").arg(lzhFilePath);
        return false;
    }
    m_elapsedTimer.start();
    m_archiveFilePath = lzhFilePath;
    m_buffer.clear();
    m_process->start("unar",
                     {
                        "-no-directory",
                        "-output-directory", m_tmpDir->path(),
                        "-encoding", getEncodingParam(),
                        lzhFilePath
                     });
    return true;
}

void ArchiveProcessorExternalUnar::onQProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    qInfo() << "onQProcessFinished" << exitStatus << exitCode;
    m_elapsedTimer.start();
    if (exitStatus == QProcess::CrashExit) {
        emit extractFail(m_archiveFilePath, "Extract failed.");
        return;
    }
    if (!m_tmpNestedLzhFilePath.isEmpty()) {
        bool ok = QFile(m_tmpNestedLzhFilePath).remove();
        m_tmpNestedLzhFilePath.clear();
        if (!ok) {
            emit extractFail(m_archiveFilePath, "Extract failed");
            return;
        }
    }
    m_tmpNestedLzhFilePath = getNextUnextractedLzhPath();   // FIXME: Remember to delete this file after extracted.
    if (!m_tmpNestedLzhFilePath.isEmpty()) {
        startUnarProcessToExtractLzh(m_tmpNestedLzhFilePath);
    } else {
        emit extractSuccess(m_archiveFilePath);
    }
    return;
}

void ArchiveProcessorExternalUnar::onQProcessErrorOccurred(QProcess::ProcessError error)
{
    switch (error) {
    case QProcess::FailedToStart: emit extractFail(m_archiveFilePath, "Unar process failed to start."); return;
    case QProcess::Crashed: emit extractFail(m_archiveFilePath, "Unar process crashed."); return;
    case QProcess::Timedout: emit extractFail(m_archiveFilePath, "Unar process timeout."); return;
    case QProcess::WriteError: emit extractFail(m_archiveFilePath, "Unar process write timeout."); return;
    case QProcess::ReadError: emit extractFail(m_archiveFilePath, "Unar process read timeout."); return;
    case QProcess::UnknownError: emit extractFail(m_archiveFilePath, "Unknown error."); return;
    }
}

void ArchiveProcessorExternalUnar::onQProcessStdout(){
    QByteArray ba = m_process -> readAllStandardOutput();
    qDebug() << "[DEBUG][Unar]" << QString::fromUtf8(ba);
}

QStringList ArchiveProcessorExternalUnar::getAllFileNamesInTmpDirRecursively() {
    QStringList fin;
    QDirIterator it(m_tmpDir->path(), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        fin.append(filePath);
    }
    return fin;
}

std::shared_ptr<QMap<QString, QByteArray> > ArchiveProcessorExternalUnar::getAllFileRawContentsInTmpDirRecursively()
{
    std::shared_ptr<QMap<QString, QByteArray>> extractedRawFiles = std::make_shared<QMap<QString, QByteArray>>();
    QDirIterator it (m_tmpDir->path(), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fileInfo {filePath};
        QString fileName = fileInfo.fileName();
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray content = file.readAll();
            extractedRawFiles->insert(fileName, content);
            file.close();
        } else {
            qWarning() << "Failed to open file:" << file.errorString();
        }
    }
    return extractedRawFiles;
}

QString ArchiveProcessorExternalUnar::getNextUnextractedLzhPath()
{
    QDirIterator it (m_tmpDir->path(), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        if (filePath.endsWith(".LZH", Qt::CaseInsensitive)) {
            return filePath;
        }
    }
    return "";
}

