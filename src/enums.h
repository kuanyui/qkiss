#ifndef ENUMS_H
#define ENUMS_H
#include <qnamespace.h>

namespace QKiSS {
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
        ISO_2022_JP
    };
}

#endif // ENUMS_H
