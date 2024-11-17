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
    static std::string toUtf8(const std::string &input, const char* converterName);
    static QString toUtf8(const QByteArray &input, const char* converterName);
    static QString toUtf8(const QByteArray &input, QKiSS::Encoding encoding);

};
#endif // ENCODING_H
