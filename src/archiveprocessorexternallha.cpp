#include "archiveprocessorexternallha.h"
#include "configfile.h"

#include <QDirIterator>
#include <QTemporaryDir>
using namespace Qt::Literals::StringLiterals;

#define argn(num) arg(QString::number(num))

ArchiveProcessorExternalLha::ArchiveProcessorExternalLha(QObject *parent)
    : AbstractArchiveProcessor{parent},
      m_process{new QProcess(this)},
      m_extractedFiles{std::make_shared<QMap<QString, QByteArray>>()}
{
    connect(m_process, &QProcess::readyRead, this, &ArchiveProcessorExternalLha::onQProcessReadyRead);
    connect(m_process, &QProcess::finished, this, &ArchiveProcessorExternalLha::onQProcessFinished);
    m_tmpDir = new QTemporaryDir(QDir(QDir::tempPath()).filePath("qtkiss-extracted-XXXXXX"));
    // connect(m_process, &QProcess::errorOccurred, this, &ArchiveProcessor::onProcessErrorOccurred);
}

ArchiveProcessorExternalLha::~ArchiveProcessorExternalLha()
{
    delete m_tmpDir;
}

QFuture<std::shared_ptr<ExtractedRawFilesMap> > ArchiveProcessorExternalLha::extractFileAsPromise(const QString &lzhOrCnfFilePath)
{
    QPromise<std::shared_ptr<ExtractedRawFilesMap>> *promise = new QPromise<std::shared_ptr<ExtractedRawFilesMap>>();
    QFuture<std::shared_ptr<ExtractedRawFilesMap>> future = promise->future();
    QFileInfo fileInfo {lzhOrCnfFilePath};
    QString fileName = fileInfo.fileName();
    m_extractedFiles = std::make_shared<QMap<QString, QByteArray>>();
    promise->start();
    if (lzhOrCnfFilePath.endsWith(".CNF"_L1, Qt::CaseInsensitive)) {
        qInfo().noquote() << tr("Reading a CNF file \"%1\" directly. Trying to fetch all related CNF/KCF/CEL files in the same folder...").arg(lzhOrCnfFilePath);
        promise->addResult(getRelatedFilesInTheSameFolder(lzhOrCnfFilePath));
        promise->finish();
        delete promise;
        return future;
    }
    qInfo().noquote() << tr("Trying to extract LZH file \"%1\"...").arg(fileName);
    this->callExternalProgramOrLibraryToExtractLzh(lzhOrCnfFilePath);
    QObject::connect(this, &ArchiveProcessorExternalLha::extractFail, [promise, fileName](QString compressedFilePath, QString errMsg) {
        qCritical().noquote() << tr("Extracting LZH file \"%1\" failed. Abort.").arg(fileName);
        Q_UNUSED(compressedFilePath);
        promise->setException(std::make_exception_ptr(
                                  std::runtime_error("Process failed with exit code: %1"_L1.arg(errMsg).toStdString())
                                  ));
        promise->finish();
        delete promise;
    });
    QObject::connect(this, &ArchiveProcessorExternalLha::extractSuccess, [promise, fileName](QString compressedFilePath, std::shared_ptr<ExtractedRawFilesMap> extractedRawFiles) {
        Q_UNUSED(compressedFilePath);
        qInfo().noquote() << tr("LZH file \"%1\" is extracted successfully. Got %2 file(s) in it").arg(fileName).arg(extractedRawFiles->size());
        promise->addResult(extractedRawFiles);
        promise->finish();
        delete promise;
    });
    return future;
}

bool ArchiveProcessorExternalLha::checkLhaCommandAvailability()
{
    QProcess p {};
    p.start("lha");
    p.waitForFinished();
    return p.exitCode() == 0;
}


FilesSummaryList ArchiveProcessorExternalLha::getFileSizeList(const QString &lzhFilePath)
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
                    "--archive-kanji-code=%1"_L1.arg(getEncodingParam()),
                    lzhFilePath
                  });
    process.waitForFinished();
    FilesSummaryList fileSizeSummary;
    if (!ok || lines.isEmpty()) { return fileSizeSummary; }
    /*
     * Start to parse output of LHA command.
     * ATTENTION: the count of `-` at column NAME may be not same with the length of the longest relative filename.
     *
     * @example
PERMISSION  UID  GID      SIZE  RATIO     STAMP           NAME
---------- ----------- ------- ------ ------------ --------------------
[MS-DOS]                 37608  21.3% Oct  8  2009 FA1280B/fa120001.cel
[MS-DOS]                 59440  14.4% Oct  8  2009 FA1280B/fa120002.cel
[MS-DOS]                 59440  14.3% Oct  8  2009 FA1280B/fa120003.cel
[generic]                92688  12.6% May 30  2006 Haruhi_m1/Haruhi_m1.cnf
---------- ----------- ------- ------ ------------ --------------------
 Total       709 files 113297883  38.9% Oct  9  2009
     */
    int sizeTitleIdx = lines[0].indexOf(QStringLiteral("SIZE"));
    int nameTitleIdx = lines[0].indexOf(QStringLiteral("NAME"));
    int sizeColIdxFrom = 0, sizeColWidth = 0;
    int nameColIdxFrom = 0, nameColWidth = 0;
    const QStringList seperators = lines[1].split(" ");
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
    Q_UNUSED(nameColWidth);
    for (int i = 2; i < lines.length() - 2; i++) {
        const QString& line = lines[i];
        int byte = line.mid(sizeColIdxFrom, sizeColWidth).trimmed().toInt();
        QString relPath = line.mid(nameColIdxFrom).trimmed();
        QFileInfo fileInfo(relPath);
        QString fileName = fileInfo.fileName();
        qDebug() << "List file name: " << fileName << byte;
        fileSizeSummary.append(FileSummary(fileName, byte));
    }
    return fileSizeSummary;
}







void ArchiveProcessorExternalLha::onQProcessReadyRead()
{
    QByteArray tmp = m_process->readAll();
    // qDebug() << "QProcess readyRead";
    m_buffer.append(tmp);
}

void ArchiveProcessorExternalLha::onQProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    m_buffer.append(m_process->readAll());
    qDebug() << "QProcess finished, buffer size = " << m_buffer.size();
    if (exitStatus == QProcess::CrashExit) {
        emit extractFail(m_archiveFilePath, "Extract failed.");
        return;
    }
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
        if (!fileName.endsWith(".LZH"_L1, Qt::CaseInsensitive)) {
            m_extractedFiles->insert(fileName, data);
        } else {   // Nested LZH file, extract the inner LZH file into tmp folder and extract it recursively.
            QString tmpFilePath = m_tmpDir->filePath(fileName);
            QFile tmpFile { tmpFilePath };
            if (!tmpFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    emit extractFail(m_archiveFilePath, tr("Failed when extracting nested LZH file \"%1\" (failed to open tmp file \"%2\").").arg(fileName).arg(tmpFilePath));
                    return;
                }
            qint64 writtenBytes = tmpFile.write(data);
            if (writtenBytes != data.size()) {
                emit extractFail(m_archiveFilePath, tr("Partial write occurred. Expected: %1 bytes, actual: %2 bytes").argn(data.size()).argn(writtenBytes));
                tmpFile.close();
                return;
            }
            m_tmpNestedLzhFilePathList.append(tmpFilePath);
            tmpFile.close();
        }
    }
    qDebug() << "[LOG][Benchmark][ArchiveProcessor] Decompression spent " << m_elapsedTimer.elapsed() << " ms";
    m_buffer.clear();
    if (m_tmpNestedLzhFilePathList.length()) {
        QString nextLzhFilePath = m_tmpNestedLzhFilePathList.takeLast();
        callExternalProgramOrLibraryToExtractLzh(nextLzhFilePath);
    } else {
        emit extractSuccess(m_archiveFilePath, m_extractedFiles);
    }
    return;
}

void ArchiveProcessorExternalLha::onProcessErrorOccurred(QProcess::ProcessError error)
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

bool ArchiveProcessorExternalLha::callExternalProgramOrLibraryToExtractLzh(const QString &lzhFilePath)
{
    if (!QFileInfo::exists(lzhFilePath)) {
        qCritical() << tr("Archive file path '%1' doesn't exist, abort.").arg(lzhFilePath);
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

const char *ArchiveProcessorExternalLha::getEncodingParam()
{
    ConfigFile &cfg = ConfigFile::instance();
    switch (cfg.encoding()) {
        case QKiSS::Encoding::SJIS: return "sjis";
        case QKiSS::Encoding::EUC_JP: return "euc";
        case QKiSS::Encoding::UTF8: return "utf8";
        case QKiSS::Encoding::ASCII: return "utf8";
        default: return "sjis";
    }
}

std::shared_ptr<ExtractedRawFilesMap> ArchiveProcessorExternalLha::getRelatedFilesInTheSameFolder(const QString &cnfFilePath)
{
    auto fileContents = std::make_shared<QMap<QString, QByteArray>>();
    QFileInfo cnfFileInfo(cnfFilePath);
    QDir dir = cnfFileInfo.dir();

    for (const QFileInfo &info : dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable)) {
        QFile file(info.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly)) {
            fileContents->insert(info.fileName(), file.readAll());
            file.close();
        }
    }
    return fileContents;
}
