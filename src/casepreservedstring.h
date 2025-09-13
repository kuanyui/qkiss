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

#ifndef CASEPRESERVEDSTRING_H
#define CASEPRESERVEDSTRING_H
#include <QString>

class CasePreservedString {
public:
    explicit CasePreservedString() = default;
    CasePreservedString(QString original);
    CasePreservedString &operator=(const QString &newString);
    /// For internal usage. For example, compare among:
    ///   - CEL file name in LZH archive / file system
    ///   - CEL file name referenced in CNF file.
    ///   - CEL file name in FKiSS script.
    ///
    /// The value may be different when Case Sensitivity setting is set to "Insensitive"
    const QString& normalized() const;
    /// For displaying on UI only.
    const QString& original() const;
    bool operator==(const CasePreservedString &other) const;
    bool operator<(const CasePreservedString &other) const;
private:
    QString m_original;
    mutable QString m_cachedNormalized;
};

QDebug operator<<(QDebug dbg, const CasePreservedString &s);
size_t qHash(const CasePreservedString &s, size_t seed = 0);


using CPString = CasePreservedString;
using CPStringList = QList<CPString>;

#endif // CASEPRESERVEDSTRING_H
