#include "archivedfile.h"

#include <QStringList>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>


// #include <QStringConverter>  // Qt6. In Qt5 this is <QTextCodec>.  http://mochiuwiki.e2.valueserver.jp/index.php?title=Qt%E3%81%AE%E5%9F%BA%E7%A4%8E_-_%E6%96%87%E5%AD%97%E3%82%B3%E3%83%BC%E3%83%89

ArchivedFile::ArchivedFile(QObject *parent):
    QObject(parent),
    m_process(new QProcess(this)),
    m_parsedCnfFiles(new QMap<QString, ParsedCnf*>),
    m_parsedKcfFiles(new QHash<QString, ParsedKcf*>),
    m_parsedCelFiles(new QHash<QString, ParsedCel*>),
    m_otherFiles(new QMap<QString, QByteArray>())
{
    connect(m_process, &QProcess::readyRead, this, &ArchivedFile::onQProcessReadyRead);
    connect(m_process, &QProcess::finished, this, &ArchivedFile::onQProcessFinished);
}

ArchivedFile::~ArchivedFile()
{
    unloadLzh();
}

bool ArchivedFile::loadLzh(const QString &filePath, QtKISS::Case caseSensitivity)
{
    if (!QFileInfo::exists(filePath)) {
        qCritical() << QStringLiteral("[ERROR][ArchivedFile] archive file path '%1' doesn't exist, abort.").arg(filePath);
        return false;
    }
    m_archiveFilePath = filePath;
    m_caseSensitivity = caseSensitivity;
    m_parsedCnfFiles->clear();
    m_parsedKcfFiles->clear();
    m_parsedCelFiles->clear();
    m_otherFiles->clear();
    m_tmpDir = new QTemporaryDir(QDir(QDir::tempPath()).filePath("qtkiss-extracted-XXXXXX"));
    if (!m_tmpDir->isValid()) {
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

void ArchivedFile::unloadLzh()
{
    for (ParsedCnf *x : *m_parsedCnfFiles) { delete x; }
    m_parsedCnfFiles->clear();
    for (ParsedKcf *x : *m_parsedKcfFiles) { delete x; }
    m_parsedKcfFiles->clear();
    for (ParsedCel *x : *m_parsedCelFiles) { delete x; }
    m_parsedCelFiles->clear();
    m_otherFiles->clear();
    m_curCnf=nullptr;
    m_curCnfOrderedParsedKcfList.clear();
    m_curCnfSetNo = 0;
    m_curPaletteNo = 0;
}

bool ArchivedFile::checkLhaCommandAvailability()
{
    QProcess p {};
    p.start("lha");
    p.waitForFinished();
    return p.exitCode() == 0;
}

QStringList ArchivedFile::getCnfNameList()
{
    return m_parsedCnfFiles->keys();
}

QStringList ArchivedFile::getOtherFileNameList()
{
    return m_otherFiles->keys();
}


std::optional<ParsedCnf *> ArchivedFile::getParsedCnf(QString cnfFileName) const
{
    if (m_caseSensitivity == QtKISS::Case::Insensitive) {
        cnfFileName = cnfFileName.toUpper();
    }
    if (m_parsedCnfFiles->contains(cnfFileName)) {
        return m_parsedCnfFiles->value(cnfFileName);
    }
    return std::nullopt;
}

std::optional<ParsedKcf *> ArchivedFile::getParsedKcf(QString kcfFileName) const
{
    if (m_caseSensitivity == QtKISS::Case::Insensitive) {
        kcfFileName = kcfFileName.toUpper();
    }
    if (m_parsedKcfFiles->contains(kcfFileName)) {
        return std::optional<ParsedKcf*>(m_parsedKcfFiles->value(kcfFileName));
    }
    return std::nullopt;
}

std::optional<ParsedCel *> ArchivedFile::getParsedCel(QString celFileName) const
{
    if (m_caseSensitivity == QtKISS::Case::Insensitive) {
        celFileName = celFileName.toUpper();
    }
    if (m_parsedCelFiles->contains(celFileName)) {
        return std::optional<ParsedCel*>(m_parsedCelFiles->value(celFileName));
    }
    return std::nullopt;
}

std::optional<ParsedKcf *> ArchivedFile::getParsedKcfByOrderedIndexInCurrentCnf(uint32_t kcfIndexInCurrentCnf)
{
    if (m_curCnfOrderedParsedKcfList.length() == 0) {
        qDebug() << QStringLiteral("[ERROR][ArchivedFile] Need to get ParsedKcf for current CNF, but this CNF has no KCF file declared. Abort.");
        return std::nullopt;
    }
    if (kcfIndexInCurrentCnf >= m_curCnfOrderedParsedKcfList.length()) {
        qDebug() << QStringLiteral("[WARN][ArchivedFile] Need to get ParsedKcf of current CNF at index %1, but out of index, so fallback to 0.");
        return m_curCnfOrderedParsedKcfList[0];
    }
    return m_curCnfOrderedParsedKcfList[kcfIndexInCurrentCnf];
}

std::optional<QByteArray> ArchivedFile::getOtherFile(QString fileName) const
{
    if (m_caseSensitivity == QtKISS::Case::Insensitive) {
        fileName = fileName.toUpper();
    }
    if (m_otherFiles->contains(fileName)) {
        return std::optional<QByteArray>(m_otherFiles->value(fileName));
    }
    return std::nullopt;
}

bool ArchivedFile::setCurCnf(QString cnfFileName)
{
    auto _cnf = getParsedCnf(cnfFileName);
    if (!_cnf.has_value()) {
        qCritical() << "[ERROR] Set current CNF failed; no CNF named " << cnfFileName << " in this archivedFile.";
        return false;
    }
    m_curCnf = _cnf.value();
    m_curCnfOrderedParsedKcfList.clear();
    qDebug() << QStringLiteral("[LOG][ArchivedFile] The CNF '%1' requires %2 KCF files.").arg(cnfFileName).arg(m_curCnf->kcfFileNames().length());
    for (const QString &kcfFileName : m_curCnf->kcfFileNames()) {
        auto _kcf = getParsedKcf(kcfFileName);
        if (!_kcf.has_value()) {
            qCritical() << QStringLiteral("[ERROR] CNF file '%1' requires KCF file '%2', but this KCF file is not found in this archived file.")
                           .arg(m_curCnf->fileName())
                           .arg(kcfFileName);
            continue;
        }
        m_curCnfOrderedParsedKcfList.append(_kcf.value());
    }
    m_curCnfSetNo = 0;
    return true;
}

void ArchivedFile::onQProcessReadyRead()
{
    qsizetype prefixLen {m_tmpDir->path().length() + 1};
    while ( m_process->canReadLine() ) {
        QString raw = QString(m_process->readLine());
        if (raw.endsWith(QStringLiteral("\t- Melted  \n"))) {
            QString absPath {raw.split("\t")[0]};
            QString relPath {absPath.mid(prefixLen)};
            qDebug() << relPath;
        }
        //qDebug() << "===> " << raw;
    }
}


void ArchivedFile::onQProcessFinished()
{
    QDirIterator it {
        m_tmpDir->path(),
        QDir::NoDotAndDotDot | QDir::AllEntries,
        QDirIterator::Subdirectories
    };
    while (it.hasNext()) {
        QString filePath = it.next();
        QFile f { filePath };
        QFileInfo fi { f };
        QString fileNameKey = fi.fileName();
        if (m_caseSensitivity == QtKISS::Case::Insensitive) {
            fileNameKey = fileNameKey.toUpper();
        }
        // qDebug() << "read file: " << fileNameKey << "  -- from " << f.fileName();
        f.open(QIODevice::ReadOnly);
        QByteArray rawData = f.readAll();
        if (fileNameKey.endsWith(".CNF")) {
            // qDebug() << Encoding::toUtf8(rawData, "Shift_JIS");
            // qDebug() << QString::fromStdString( Encoding::toUtf8(rawData.toStdString(), "Shift_JIS") );
            m_parsedCnfFiles->insert(fileNameKey, new ParsedCnf {fileNameKey, rawData, QtKISS::Encoding::SJIS});
        } else if (fileNameKey.endsWith(".KCF")) {
            m_parsedKcfFiles->insert(fileNameKey, new ParsedKcf {fileNameKey, rawData});
        } else if (fileNameKey.endsWith(".CEL")) {
            m_parsedCelFiles->insert(fileNameKey, new ParsedCel {fileNameKey, rawData});
        } else {
            m_otherFiles->insert(fileNameKey, rawData);
        }
        f.close();
    }
    qDebug() << "<=== all archived files parsing finished!";
    m_curCnf = m_parsedCnfFiles->first();
    m_curCnfSetNo = 0;
    m_curPaletteNo = 0;
    delete m_tmpDir;
    emit archivedFilesUncompressAndParseFinished(true);
}

uint8_t ArchivedFile::curPaletteNo() const
{
    return m_curPaletteNo;
}

void ArchivedFile::setCurPaletteNo(uint8_t newCurPaletteNo)
{
    m_curPaletteNo = newCurPaletteNo;
}

size_t ArchivedFile::paletteAmountOfFirstKcfInCurCnf() const
{
    if (m_curCnf == nullptr) { return 0; }
    if (m_curCnfOrderedParsedKcfList.length() == 0) { return 0; }
    return m_curCnfOrderedParsedKcfList[0]->paletteAmounts();
}

void ArchivedFile::setCurCnfSetNo(uint8_t newCurCnfSetNo)
{
    m_curCnfSetNo = newCurCnfSetNo;
}

uint8_t ArchivedFile::curCnfSetNo() const
{
    return m_curCnfSetNo;
}

ParsedCnf *ArchivedFile::curCnf() const
{
    return m_curCnf;
}
