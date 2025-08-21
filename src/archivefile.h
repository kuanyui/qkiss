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

#ifndef ARCHIVEFILE_H
#define ARCHIVEFILE_H

#include "enums.h"
#include <QObject>
#include <QString>
#include <QHash>
#include <QVector>
#include "parsedcnf.h"
#include "parsedkcf.h"
#include "parsedcel.h"
#include "archiveprocessorexternallha.h"

class ConfigFile;

/// Attention: Remember to call deleteAllObjects() to release memory.
struct KissFilesParsingResult {
    void deleteAllObjects();
    QMap<QString,  ParsedCnf*> parsedCnfFiles;
    QHash<QString, ParsedKcf*> parsedKcfFiles;
    QHash<QString, ParsedCel*> parsedCelFiles;
    QMap<QString, QString> docFiles;
    QMap<QString, QByteArray> otherFiles;
};

class ArchiveFile: public QObject
{
    Q_OBJECT
public:
    enum class LoadingState {
        Idle,
        Decompressing,
        ParsingFiles,
        Rendering,
    };

    ArchiveFile(QObject *parent = nullptr);
    ~ArchiveFile();
    static KissFilesParsingResult parseKissFiles(std::shared_ptr<QMap<QString, QByteArray>> extractedRawFiles, QString lzhFilePath);
    bool loadKissFiles(KissFilesParsingResult &parseRes, QString lzhFilePath);
    bool loadKissFilesAsPatch(KissFilesParsingResult parseRes, QString lzhFilePath);
    void unloadKissFiles();
    QStringList getCnfNameList();
    QStringList getOtherFileNameList();
    QStringList getDocFileNameList();
    /** Auto handling case sensitivity. */
    std::optional<ParsedCnf *> getParsedCnf(QString cnfFileName) const;
    /** Auto handling case sensitivity. */
    std::optional<ParsedKcf *> getParsedKcf(QString kcfFileName) const;
    /** Auto handling case sensitivity. */
    std::optional<ParsedCel *> getParsedCel(QString celFileName) const;
    std::optional<ParsedKcf *>getParsedKcfByOrderedIndexInCurrentCnf(uint32_t kcfIndexInCurrentCnf);
    /** Auto handling case sensitivity. */
    std::optional<QString> getDocFile(QString docFileName) const;
    /** Auto handling case sensitivity. */
    std::optional<QByteArray> getOtherFile(QString fileName) const;
    /// Auto handling case sensitivity. Return empty string if failed
    QString getCnfFileAsString(QString cnfFileName) const;
    /// Auto handling case sensitivity. Return empty string if failed
    QString getDocFileAsString(QString docFileName) const;
    QMap<QString, ParsedCnf *> *parsedCnfFiles() const;
    /// This should be called by GKissView. This won't trigger rendering of GKissView.
    bool setCurCnfByName(QString cnfFileName);
    ParsedCnf *curCnf() const;
    uint8_t curCnfSetNo() const;
    void setCnfSetNo(uint8_t newCurCnfSetNo);
    void setCnfSetNoFromFkiss(uint8_t newCurCnfSetNo);
    /// This will try to trigger `set()` in fkiss runtime
    void setCnfSetNoFromMainWindow(uint8_t newCurCnfSetNo);
    uint8_t curCnfSetsAmount() const;
    uint8_t curPaletteNo() const;
    void setPaletteNo(uint8_t newPaletteNo);
    /// Though ArchiveFile itself doesn't store the "real palette no. state", here is a copy of it.
    /// (the real one should be stored by Set (GKissScene) because the `N` of `$N` in CNF is the paletteNo of that set).
    /// Set via this method will emit `curPaletteNoChanged` signal, as an event bus.
    void setPaletteNoFromFkiss(uint8_t newPaletteNo);
    /// This will try to trigger `col()` in fkiss runtime
    void setPaletteNoFromMainWindow(uint8_t newPaletteNo);
    /// Get the first KCF file, and return the palette amount of this KCF file.
    size_t paletteAmountOfFirstKcfInCurCnf() const;

    LoadingState loadingState() const;
    void setLoadingState(LoadingState newLoadingState);

private slots:

signals:
    void newLogLine(QString line);
    void curCnfChangedAndLoaded(QString newCnfFileName);
    /// fkiss script requests to change set, so MainWindow can know it's time to toggle toolbar button.
    void fkissChangedCnfSetNo(uint8_t newCurCnfSetNo);
    /// fkiss script requests to change palette, so MainWindow can know it's time to toggle toolbar button.
    void fkissChangedPaletteNo(uint8_t newCurPaletteNo);
    // void archivedFilesUncompressAndParseFinished(bool success, QString errmsg);
    // void curCnfSetNoChanged(uint8_t nv);
private:
    ConfigFile &m_gs;
    QMap<QString, ParsedCnf*> *m_parsedCnfFiles;   /// this class owns these two pointers
    QHash<QString, ParsedKcf*> *m_parsedKcfFiles;  /// this class owns these two pointers
    QHash<QString, ParsedCel*> *m_parsedCelFiles;  /// this class owns these two pointers
    QMap<QString, QString> *m_docFiles;   /// this class owns these two pointers
    /// - *.DOC, *.PCM, ... etc.
    /// - this class owns these two pointers
    QMap<QString, QByteArray> *m_otherFiles;
    QString m_archiveFilePath;
    // ==========================================
    /** currently renderred CNF */
    ParsedCnf *m_curCnf = nullptr;
    /**
     * - A CNF file would specify the used KCF files as an ordered list.
     * - In this CNF file, each CEL line would specify this CEL should be rendered with which KCF file.
     * - This value is the ordered KCF files list used by the current CNF.
    */
    QVector<ParsedKcf*> m_curCnfOrderedParsedKcfList;
    /**
     *  - Range is 0-9. MAX is 9. Multiple lines starts with `$` in CNF, this value is the index of these lines.
     *  - '$'	セット情報（複数個指定可能）
     *    書式： $<パレット組> [<x座標,y座標> ...] 最初の指定が 0 で 9 まで、最大10個のセット情報を指定できます。
     */
    uint8_t m_curCnfSetNo;
    /** <s>これは ` $<パレット組>` のあれ「パレット組」。KCF ごと N 個パレット組がある。</s>
      This real state should be stored in each set (GKissScene), instead of this class. */
    uint8_t m_curPaletteNo;
    LoadingState m_loadingState;
};

#endif // ARCHIVEFILE_H
