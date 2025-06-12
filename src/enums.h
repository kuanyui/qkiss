#ifndef ENUMS_H
#define ENUMS_H
#include <qnamespace.h>
#include <QCoreApplication>

class QKiSS {
    Q_DECLARE_TR_FUNCTIONS(QKiSS)   // Use tr() without Q_OBJECT

public:
    enum class Case {
        Sensitive = Qt::CaseSensitive,
        Insensitive = Qt::CaseInsensitive
    };
    /** https://www.iana.org/assignments/character-sets/character-sets.xhtml */
    enum class Encoding {
        ASCII,
        UTF8,
        // Japanese
        SJIS,
        EUC_JP,
    };
    enum class FileLoadMode {
        LoadAsIndependent,
        LoadAsPatch,
    };
    enum class LzhDecompressorBackend {
        Libarchive,
        ExternalLhaProcess,
    };
    static const QStringList encodingNames() {
        return {
            tr("ASCII"),
            tr("UTF-8"),
            tr("Shift JIS (Default)"),
            tr("EUC-JP")
        };
    };
};

#endif // ENUMS_H
