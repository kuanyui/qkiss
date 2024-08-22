#ifndef ENCODING_H
#define ENCODING_H
#include <QString>

/**
 * Encoding Name List
 * - https://www.iana.org/assignments/character-sets/character-sets.xhtml
 */
class Encoding {
public:
    Encoding() {};
    static std::string toUtf8(const std::string &input, const char* converterName);
    static QString toUtf8(const QString &input, const char* converterName);

};
#endif // ENCODING_H
