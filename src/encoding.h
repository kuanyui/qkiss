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
    static std::string ShiftJisToUtf8(const std::string &input);
    static QString ShiftJisToUtf8(QString input);

};
#endif // ENCODING_H
