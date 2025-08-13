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
