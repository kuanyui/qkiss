/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2024 - 2026 ono ono (kuanyui) <azazabc123@gmail.com>
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
#ifndef MYCSTR_H
#define MYCSTR_H

#include <cstring>

template <size_t N>
inline bool cstrStartsWith(const char* str, const char (&prefix)[N]) noexcept {
    constexpr size_t prefixLen = N - 1;   // compiler referred `N` contains `\0`
    // str cannot be nullptr
    return str && std::memcmp(str, prefix, prefixLen) == 0;
}

template <size_t N>
inline bool cstrEndsWith(const char* str, const char (&suffix)[N]) noexcept {
    constexpr size_t suffixLen = N - 1;   // compiler referred `N` contains `\0`
    if (!str) { return false; }  // str cannot but nullptr
    size_t strLen = std::strlen(str);   // std::strlen() does NOT count `\0`
    if (strLen < suffixLen) { return false; }
    const char *cmpBegin = str + (strLen -  suffixLen);
    return std::memcmp(cmpBegin, suffix, suffixLen) == 0;
}

#endif // MYCSTR_H
