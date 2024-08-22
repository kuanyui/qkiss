#ifndef ENUMS_H
#define ENUMS_H
#include <qnamespace.h>

namespace QtKISS {
    enum class Case {
        Sensitive = Qt::CaseSensitive,
        Insensitive = Qt::CaseInsensitive
    };
    enum class Encoding {
        ASCII,
        // Japanese
        SJIS,
        EUC_JP,
        ISO_2022_JP
    };
}

#endif // ENUMS_H
