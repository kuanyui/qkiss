#ifndef ARCHIVEDFILE_H
#define ARCHIVEDFILE_H

#include "enums.h"
#include <QObject>
#include <QString>
#include <QHash>
#include <QVector>
#include "parsedcnf.h"
#include "parsedkcf.h"
#include "parsedcel.h"
#include "archiveprocessor.h"

struct KissFilesParsingResult {
    QMap<QString,  std::unique_ptr<ParsedCnf>> parsedCnfFiles;
    QHash<QString, std::unique_ptr<ParsedKcf>> parsedKcfFiles;
    QHash<QString, std::unique_ptr<ParsedCel>> parsedCelFiles;
    QMap<QString, QString> docFiles;
    QMap<QString, QByteArray> otherFiles;
};

class ArchivedFile: public QObject
{
    Q_OBJECT
public:
    enum class LoadingState {
        Idle,
        Decompressing,
        ParsingFiles,
        Rendering,
    };

    ArchivedFile(QObject *parent = nullptr);
    ~ArchivedFile();
    static std::unique_ptr<KissFilesParsingResult> parseKissFiles(std::shared_ptr<QMap<QString, QByteArray>> extractedRawFiles, QString lzhFilePath);
    bool loadKissFiles(KissFilesParsingResult &parseRes, QString lzhFilePath);
    bool loadKissFilesAsPatch(KissFilesParsingResult parseRes, QString lzhFilePath);
    void unloadKissFiles();
    QStringList getCnfNameList();
    QStringList getOtherFileNameList();
    /** Auto handling case sensitivity. */
    std::optional<ParsedCnf *> getParsedCnf(QString cnfFileName) const;
    /** Auto handling case sensitivity. */
    std::optional<ParsedKcf *> getParsedKcf(QString kcfFileName) const;
    /** Auto handling case sensitivity. */
    std::optional<ParsedCel *> getParsedCel(QString celFileName) const;
    std::optional<ParsedKcf *>getParsedKcfByOrderedIndexInCurrentCnf(uint32_t kcfIndexInCurrentCnf);
    /** Auto handling case sensitivity. */
    std::optional<QByteArray> getOtherFile(QString celFileName) const;
    QMap<QString, ParsedCnf *> *parsedCnfFiles() const;
    /// This should be called by GKissView. This won't trigger rendering of GKissView.
    bool setCurCnfByName(QString cnfFileName);
    ParsedCnf *curCnf() const;
    uint8_t curCnfSetNo() const;
    void setCurCnfSetNo(uint8_t newCurCnfSetNo);
    uint8_t curPaletteNo() const;
    void setCurPaletteNo(uint8_t newCurPaletteNo);
    /// Get the first KCF file, and return the palette amount of this KCF file.
    size_t paletteAmountOfFirstKcfInCurCnf() const;

    LoadingState loadingState() const;
    void setLoadingState(LoadingState newLoadingState);

private slots:

signals:
    void newLogLine(QString line);
    void curCnfChanged(QString newCnfFileName);
    // void archivedFilesUncompressAndParseFinished(bool success, QString errmsg);
    // void curCnfSetNoChanged(uint8_t nv);
private:
    QtKISS::Case m_caseSensitivity;
    QMap<QString, ParsedCnf*> *m_parsedCnfFiles;
    QHash<QString, ParsedKcf*> *m_parsedKcfFiles;
    QHash<QString, ParsedCel*> *m_parsedCelFiles;
    QMap<QString, QString> *m_docFiles;
    /// *.DOC, *.PCM, ... etc.
    QMap<QString, QByteArray> *m_otherFiles;
    QString m_archiveFilePath;
    // ==========================================
    /** currently renderred CNF */
    ParsedCnf *m_curCnf;
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
    /** これは ` $<パレット組>` のあれ「パレット組」。KCF ごと N 個パレット組がある。*/
    uint8_t m_curPaletteNo;
    ArchiveProcessor *m_archiveExtractor;
    LoadingState m_loadingState;
};

#endif // ARCHIVEDFILE_H
