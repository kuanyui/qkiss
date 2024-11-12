#include "archivedfile.h"
#include "encoding.h"

#include <QStringList>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QElapsedTimer>

void KissFilesParsingResult::deleteAllObjects()
{
    for (auto it = parsedCnfFiles.begin(); it != parsedCnfFiles.end(); it++) {
        delete it.value();
    }
    parsedCnfFiles.clear();
    for (auto it = parsedKcfFiles.begin(); it != parsedKcfFiles.end(); it++) {
        delete it.value();
    }
    parsedKcfFiles.clear();
    for (auto it = parsedCelFiles.begin(); it != parsedCelFiles.end(); it++) {
        delete it.value();
    }
    parsedCelFiles.clear();
}
// #include <QStringConverter>  // Qt6. In Qt5 this is <QTextCodec>.  http://mochiuwiki.e2.valueserver.jp/index.php?title=Qt%E3%81%AE%E5%9F%BA%E7%A4%8E_-_%E6%96%87%E5%AD%97%E3%82%B3%E3%83%BC%E3%83%89

ArchivedFile::ArchivedFile(QObject *parent):
    QObject(parent),
    m_parsedCnfFiles(new QMap<QString, ParsedCnf*>),
    m_parsedKcfFiles(new QHash<QString, ParsedKcf*>),
    m_parsedCelFiles(new QHash<QString, ParsedCel*>),
    m_docFiles(new QMap<QString, QString>),
    m_otherFiles(new QMap<QString, QByteArray>()),
    m_archiveExtractor(new ArchiveProcessor(this))
{

}

ArchivedFile::~ArchivedFile()
{
    unloadKissFiles();
}

KissFilesParsingResult ArchivedFile::parseKissFiles(std::shared_ptr<QMap<QString, QByteArray> > extractedRawFiles, QString lzhFilePath)
{
    KissFilesParsingResult res = KissFilesParsingResult();
    QtKISS::Case caseSensitivity = QtKISS::Case::Insensitive;  // TODO: Global Variable.
    QElapsedTimer elapsedTimer;
    elapsedTimer.start();
    for (const auto &[fileName, fileRawContent] : extractedRawFiles->asKeyValueRange()) {
        QString fileNameKey = fileName;
        QString fileNameUpper = fileName.toUpper();
        if (caseSensitivity == QtKISS::Case::Insensitive) {
            fileNameKey = fileNameKey.toUpper();
        }
        QtKISS::Encoding encoding = QtKISS::Encoding::SJIS;
        if (fileNameUpper.endsWith(".CNF")) {
            // TODO: Check parse result, if invalid, delete it.
            res.parsedCnfFiles.insert(fileNameKey, new ParsedCnf {fileNameKey, fileRawContent, encoding});
        } else if (fileNameUpper.endsWith(".KCF")) {
            res.parsedKcfFiles.insert(fileNameKey, new ParsedKcf {fileNameKey, fileRawContent});
        } else if (fileNameUpper.endsWith(".CEL")) {
            res.parsedCelFiles.insert(fileNameKey, new ParsedCel {fileNameKey, fileRawContent});
        } else if (fileNameUpper.endsWith(".DOC") || fileNameUpper.endsWith(".TXT") || fileNameUpper == "README") {
            res.docFiles.insert(fileNameKey, Encoding::toUtf8(fileRawContent, encoding));
        } else {
            res.otherFiles.insert(fileNameKey, fileRawContent);
        }
    }
    qDebug().noquote() << QStringLiteral("[LOG][Benchmark][ArchiveFile] parsing all CNF, KCF, CEL files in \"%1\" spent %2 ms").arg(lzhFilePath).arg(elapsedTimer.elapsed());
    return res;
}

bool ArchivedFile::loadKissFiles(KissFilesParsingResult &parseRes, QString lzhFilePath)
{
    QtKISS::Case caseSensitivity = QtKISS::Case::Insensitive;  // TODO: Global Variable.
    setLoadingState(LoadingState::ParsingFiles);  // UNUSED
    if (!parseRes.parsedCnfFiles.isEmpty() && !parseRes.parsedCelFiles.isEmpty()) {
        setLoadingState(LoadingState::Rendering);  // UNUSED
        unloadKissFiles();
        m_archiveFilePath = lzhFilePath;
        m_caseSensitivity = caseSensitivity;
        *m_parsedCnfFiles = parseRes.parsedCnfFiles;
        *m_parsedKcfFiles = parseRes.parsedKcfFiles;
        *m_parsedCelFiles = parseRes.parsedCelFiles;
        *m_docFiles = parseRes.docFiles;
        *m_otherFiles = parseRes.otherFiles;
        m_curCnf = m_parsedCnfFiles->first();
        m_curCnfSetNo = 0;
        m_curPaletteNo = 0;
        return true;
    } else {
        setLoadingState(LoadingState::Idle);  // UNUSED
        return false;
    }

}

bool ArchivedFile::loadKissFilesAsPatch(KissFilesParsingResult parseRes, QString lzhFilePath)
{

}

void ArchivedFile::unloadKissFiles()
{
    for (ParsedCnf *x : *m_parsedCnfFiles) { delete x; }
    m_parsedCnfFiles->clear();
    for (ParsedKcf *x : *m_parsedKcfFiles) { delete x; }
    m_parsedKcfFiles->clear();
    for (ParsedCel *x : *m_parsedCelFiles) { delete x; }
    m_parsedCelFiles->clear();
    m_docFiles->clear();
    m_otherFiles->clear();
    m_curCnf=nullptr;
    m_curCnfOrderedParsedKcfList.clear();
    m_curCnfSetNo = 0;
    m_curPaletteNo = 0;
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

QMap<QString, ParsedCnf *> *ArchivedFile::parsedCnfFiles() const
{
    return m_parsedCnfFiles;
}

bool ArchivedFile::setCurCnfByName(QString cnfFileName)
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
    emit curCnfChanged(cnfFileName);
    return true;
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

ArchivedFile::LoadingState ArchivedFile::loadingState() const
{
    return m_loadingState;
}

void ArchivedFile::setLoadingState(LoadingState newLoadingState)
{
    m_loadingState = newLoadingState;
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


