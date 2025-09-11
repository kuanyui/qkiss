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

#include <QDebug>
#include "casepreservedstring.h"
#include "configfile.h"

CasePreservedString::CasePreservedString(QString original)
    : m_original(std::move(original)) {}

CasePreservedString &CasePreservedString::operator=(const QString &newString)
{
    m_original = newString;
    m_cachedNormalized.clear(); // set string to null
    return *this;
}

const QString &CasePreservedString::normalized() const {
    if (ConfigFile::instance().caseSensitivity() == QKiSS::Case::Sensitive) {
        return m_original;
    }
    if (!m_cachedNormalized.isNull()) {
        m_cachedNormalized = m_original.toCaseFolded();
    }
    return m_cachedNormalized;
}

const QString &CasePreservedString::original() const {
    return m_original;
}

QDebug operator<<(QDebug dbg, const CasePreservedString &s)
{
    dbg.nospace() << s.original();
    return dbg.space();
}

size_t qHash(const CasePreservedString &s, size_t seed)
{
    return qHash(s.normalized(), seed);
}
