#ifndef ENCODING_H
#define ENCODING_H
#include <QString>
#include "enums.h"

/**
 * Encoding Name List
 * - https://www.iana.org/assignments/character-sets/character-sets.xhtml
 */
class Encoding {
public:
    Encoding() {};
    static QString toUtf8(const QByteArray &input, QKiSS::Encoding encoding);
private:
    /// encoding name may be not portable for iconv, so you can pass multiple possible.
    static std::string innerToUtf8(const std::string &input, const std::vector<std::string> &fromEncodingAlias);

};
#endif // ENCODING_H
