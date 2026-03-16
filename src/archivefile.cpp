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

#include "archivefile.h"
#include "encoding.h"
#include "configfile.h"
#include "fkissruntime.h"
#include "casepreservedstring.h"
#include "logging.h"
#include <QStringList>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QSet>

using namespace Qt::Literals::StringLiterals;

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

ArchiveFile::ArchiveFile(QObject *parent):
    QObject(parent),
    m_gs{ConfigFile::instance()},
    m_parsedCnfFiles(new QMap<CPString, ParsedCnf*>),
    m_parsedKcfFiles(new QHash<CPString, ParsedKcf*>),
    m_parsedCelFiles(new QHash<CPString, ParsedCel*>),
    m_docFiles(new QMap<CPString, QString>),
    m_otherFiles(new QMap<CPString, QByteArray>())
{

}

ArchiveFile::~ArchiveFile()
{
    unloadKissFiles();
}

KissFilesParsingResult ArchiveFile::parseKissFiles(std::shared_ptr<QMap<QString, QByteArray> > extractedRawFiles, QString lzhFilePath)
{
    KissFilesParsingResult res = KissFilesParsingResult();
    ConfigFile &cfg = ConfigFile::instance();
    QElapsedTimer elapsedTimer;
    elapsedTimer.start();
    for (const auto &[fileName, fileRawContent] : extractedRawFiles->asKeyValueRange()) {
        CPString cpFileName { fileName };
        QString fileNameUpper = fileName.toUpper();
        QKiSS::Encoding encoding = cfg.encoding();
        if (fileNameUpper.endsWith(".CNF", Qt::CaseInsensitive)) {
            // TODO: Check parse result, if invalid, delete it.
            res.parsedCnfFiles.insert(cpFileName, new ParsedCnf {cpFileName, fileRawContent, encoding});
        } else if (fileNameUpper.endsWith(".KCF")) {
            res.parsedKcfFiles.insert(cpFileName, new ParsedKcf {cpFileName, fileRawContent});
        } else if (fileNameUpper.endsWith(".CEL")) {
            res.parsedCelFiles.insert(cpFileName, new ParsedCel {cpFileName, fileRawContent});
        } else if (fileNameUpper.endsWith(".DOC") || fileNameUpper.endsWith(".TXT") || fileNameUpper == "README") {
            res.docFiles.insert(cpFileName, Encoding::toUtf8(fileRawContent, encoding));
        } else {
            res.otherFiles.insert(cpFileName, fileRawContent);
        }
    }
    qkDebug.noquote() << QStringLiteral("[LOG][Benchmark][ArchiveFile] parsing all CNF, KCF, CEL files in \"%1\" spent %2 ms").arg(lzhFilePath).arg(elapsedTimer.elapsed());
    return res;
}

#define argnum(num) arg(QString::number(num))

bool ArchiveFile::loadKissFiles(KissFilesParsingResult &parseRes, QString lzhFilePath)
{
    qkInfo.noquote() << tr("Found valid files: %1 CNF, %2 CEL, %3 KCF.")
                          .argnum(parseRes.parsedCnfFiles.count())
                          .argnum(parseRes.parsedCelFiles.count())
                          .argnum(parseRes.parsedKcfFiles.count());
    if (!parseRes.parsedCnfFiles.isEmpty() && !parseRes.parsedCelFiles.isEmpty()) {
        unloadKissFiles();
        m_archiveFilePath = lzhFilePath;
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
        return false;
    }

}

bool ArchiveFile::loadKissFilesAsPatch(KissFilesParsingResult patchFiles, QString lzhFilePath)
{
    qkInfo.noquote() << tr("Found valid files: %1 CNF, %2 CEL, %3 KCF.")
                          .argnum(patchFiles.parsedCnfFiles.count())
                          .argnum(patchFiles.parsedCelFiles.count())
                          .argnum(patchFiles.parsedKcfFiles.count());
    if (patchFiles.parsedCnfFiles.isEmpty() && patchFiles.parsedCelFiles.isEmpty() && patchFiles.parsedKcfFiles.isEmpty()) {
        qkWarning << "[WARNING][ArchiveFile] Try to load this file as patch, but it contains no any CNF/CEL/KCF file, abort.";
        return false;
    }
    m_archiveFilePath = lzhFilePath;
    for (auto itPatch = patchFiles.parsedCnfFiles.begin(); itPatch != patchFiles.parsedCnfFiles.end(); itPatch++) {
        if (m_parsedCnfFiles->contains(itPatch.key())) {
            qkDebug << "[loadKissFilesAsPatch] detele ParsedCnf and replace with the patched version." << itPatch.key();
            delete m_parsedCnfFiles->value(itPatch.key());
        }
        m_parsedCnfFiles->insert(itPatch.key(), itPatch.value());
    }
    for (auto itPatch = patchFiles.parsedKcfFiles.begin(); itPatch != patchFiles.parsedKcfFiles.end(); itPatch++) {
        if (m_parsedKcfFiles->contains(itPatch.key())) {
            qkDebug << "[loadKissFilesAsPatch] detele ParsedKcf and replace with the patched version." << itPatch.key();
            delete m_parsedKcfFiles->value(itPatch.key());
        }
        m_parsedKcfFiles->insert(itPatch.key(), itPatch.value());
    }
    for (auto itPatch = patchFiles.parsedCelFiles.begin(); itPatch != patchFiles.parsedCelFiles.end(); itPatch++) {
        if (m_parsedCelFiles->contains(itPatch.key())) {
            qkDebug << "[loadKissFilesAsPatch] detele ParsedCel and replace with the patched version." << itPatch.key();
            delete m_parsedCelFiles->value(itPatch.key());
        }
        m_parsedCelFiles->insert(itPatch.key(), itPatch.value());
    }

    m_docFiles->insert(patchFiles.docFiles);
    m_otherFiles->insert(patchFiles.otherFiles);
    m_curCnf = m_parsedCnfFiles->first();
    m_curCnfSetNo = 0;
    m_curPaletteNo = 0;
    return true;
}

void ArchiveFile::unloadKissFiles()
{
    foreach (ParsedCnf *x, *m_parsedCnfFiles) { delete x; }
    m_parsedCnfFiles->clear();
    foreach (ParsedKcf *x, *m_parsedKcfFiles) { delete x; }
    m_parsedKcfFiles->clear();
    foreach (ParsedCel *x, *m_parsedCelFiles) { delete x; }
    m_parsedCelFiles->clear();
    m_docFiles->clear();
    m_otherFiles->clear();
    m_curCnf=nullptr;
    m_curCnfOrderedParsedKcfList.clear();
    m_curCnfSetNo = 0;
    m_curPaletteNo = 0;
}



CPStringList ArchiveFile::getCnfNameList()
{
    return m_parsedCnfFiles->keys();
}

CPStringList ArchiveFile::getOtherFileNameList()
{
    return m_otherFiles->keys();
}

CPStringList ArchiveFile::getDocFileNameList()
{
    return m_docFiles->keys();
}


std::optional<ParsedCnf *> ArchiveFile::getParsedCnf(CPString cnfFileName) const
{
    if (m_parsedCnfFiles->contains(cnfFileName)) {
        return m_parsedCnfFiles->value(cnfFileName);
    }
    return std::nullopt;
}

std::optional<ParsedKcf *> ArchiveFile::getParsedKcf(CPString kcfFileName) const
{
    if (m_parsedKcfFiles->contains(kcfFileName)) {
        return std::optional<ParsedKcf*>(m_parsedKcfFiles->value(kcfFileName));
    }
    return std::nullopt;
}

std::optional<ParsedCel *> ArchiveFile::getParsedCel(CPString celFileName) const
{
    if (m_parsedCelFiles->contains(celFileName)) {
        return std::optional<ParsedCel*>(m_parsedCelFiles->value(celFileName));
    }
    return std::nullopt;
}

std::optional<ParsedKcf *> ArchiveFile::getParsedKcfByOrderedIndexInCurrentCnf(uint32_t kcfIndexInCurrentCnf)
{
    if (m_curCnfOrderedParsedKcfList.length() == 0) {
        qkCritical.noquote() << tr("Need to get ParsedKcf for current CNF, but this CNF has no KCF file declared. Abort.");
        return std::nullopt;
    }
    if (kcfIndexInCurrentCnf >= m_curCnfOrderedParsedKcfList.length()) {
        qkDebug << QStringLiteral("Need to get ParsedKcf of current CNF at index %1, but out of index, so fallback to 0.");
        return m_curCnfOrderedParsedKcfList[0];
    }
    return m_curCnfOrderedParsedKcfList[kcfIndexInCurrentCnf];
}

std::optional<QString> ArchiveFile::getDocFile(CPString docFileName) const
{
    if (m_docFiles->contains(docFileName)) {
        return std::optional<QString>(m_docFiles->value(docFileName));
    }
    return std::nullopt;
}

std::optional<QByteArray> ArchiveFile::getOtherFile(CPString fileName) const
{
    if (m_otherFiles->contains(fileName)) {
        return std::optional<QByteArray>(m_otherFiles->value(fileName));
    }
    return std::nullopt;
}

QString ArchiveFile::getCnfFileAsString(CPString cnfFileName) const
{
    auto tmp = getParsedCnf(cnfFileName);
    if (tmp.has_value()) {
        return tmp.value()->fileContentText();
    }
    return QString();
}

QString ArchiveFile::getDocFileAsString(CPString docFileName) const
{
    auto tmp = getDocFile(docFileName);
    if (tmp.has_value()) {
        return tmp.value();
    }
    return QString();
}

QMap<CPString, ParsedCnf *> *ArchiveFile::parsedCnfFiles() const
{
    return m_parsedCnfFiles;
}

bool ArchiveFile::setCurCnfByName(CPString cnfFileName)
{
    auto _cnf = getParsedCnf(cnfFileName);
    if (!_cnf.has_value()) {
        qkCritical.noquote() << tr(R"(Set current CNF failed; no CNF named "%1" in this archive file.)").arg(cnfFileName.original());
        return false;
    }
    m_curCnf = _cnf.value();
    m_curCnfOrderedParsedKcfList.clear();
    qkDebug.noquote() << tr(R"(The CNF '%1' requires %2 KCF files.)").arg(cnfFileName.original()).arg(m_curCnf->kcfFileNames().length());
    for (const QString &kcfFileName : m_curCnf->kcfFileNames()) {
        auto _kcf = getParsedKcf(kcfFileName);
        if (!_kcf.has_value()) {
            qkCritical.noquote() << tr("CNF file '%1' requires KCF file '%2', but this KCF file is not found in this archived file.")
                           .arg(m_curCnf->fileName().original())
                           .arg(kcfFileName);
            continue;
        }
        m_curCnfOrderedParsedKcfList.append(_kcf.value());
    }
    m_curCnfSetNo = 0;
    emit curCnfChangedAndLoaded(cnfFileName);
    return true;
}


uint8_t ArchiveFile::curPaletteNo() const
{
    return m_curPaletteNo;
}

void ArchiveFile::setPaletteNo(uint8_t newPaletteNo)
{
    m_curPaletteNo = newPaletteNo;
}

void ArchiveFile::setPaletteNoFromFkiss(uint8_t newPaletteNo)
{
    if (m_curPaletteNo != newPaletteNo) {
        m_curPaletteNo = newPaletteNo;
        qkDebug << "emit ArchiveFile::curPaletteNoChanged(%1)"_L1.arg(QString::number(newPaletteNo));
        emit fkissChangedPaletteNo(newPaletteNo);
    }
}

void ArchiveFile::setPaletteNoFromMainWindow(uint8_t newPaletteNo)
{
    m_curCnf->fkiss()->runtime()->triggerCol(newPaletteNo);
}

size_t ArchiveFile::paletteAmountOfFirstKcfInCurCnf() const
{
    if (m_curCnf == nullptr) { return 0; }
    if (m_curCnfOrderedParsedKcfList.length() == 0) { return 0; }
    return m_curCnfOrderedParsedKcfList[0]->paletteAmounts();
}

ArchiveFile::LoadingState ArchiveFile::loadingState() const
{
    return m_loadingState;
}

void ArchiveFile::setLoadingState(LoadingState newLoadingState)
{
    m_loadingState = newLoadingState;
}

void ArchiveFile::setCnfSetNo(uint8_t newCurCnfSetNo)
{
    m_curCnfSetNo = newCurCnfSetNo;
}

void ArchiveFile::setCnfSetNoFromFkiss(uint8_t newCurCnfSetNo)
{
    if (m_curCnfSetNo != newCurCnfSetNo) {
        m_curCnfSetNo = newCurCnfSetNo;
        qkDebug << "emit ArchiveFile::curCnfSetNoChanged(%1)"_L1.arg(QString::number(newCurCnfSetNo));
        emit fkissChangedCnfSetNo(newCurCnfSetNo);
    }
}

void ArchiveFile::setCnfSetNoFromMainWindow(uint8_t newCurCnfSetNo)
{
    m_curCnf->fkiss()->runtime()->triggerSet(newCurCnfSetNo);
}

uint8_t ArchiveFile::curCnfSetsAmount() const
{
    if (m_curCnf == nullptr) { return 0; }
    return m_curCnf->sets().length();
}

uint8_t ArchiveFile::curCnfSetNo() const
{
    return m_curCnfSetNo;
}



ParsedCnf *ArchiveFile::curCnf() const
{
    return m_curCnf;
}


