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

#ifndef FKISSVARIANT_H
#define FKISSVARIANT_H
#include <QString>
#include <QDebug>
#include <QHash>

/** Used by FKiSS script */
class FkissVariant
{
public:
    enum class Type {
        Int = 1,      // 2147483647
        String = 2,   // "Hello!"
        Cel = 4,      // "RIBBON.CEL"
        Object = 8,   // #123
        // Group,    // !groupName   // Fkiss4 only, unused currently
    };
private:
    FkissVariant(Type type, int value);
    FkissVariant(Type type, const QString& value);
public:
    static const char* typeToString(Type type);
    static FkissVariant fromString(const QString &value);
    static FkissVariant fromCelName(const QString &value);
    static FkissVariant fromInt(const QString &value);
    static FkissVariant fromInt(const int &value);
    static FkissVariant fromObjectId(const QString &value);
    static FkissVariant fromObjectId(int value);
    bool isInt() const;
    bool isString() const;
    bool isCelName() const;
    /// Workaround for convenience.
    bool isStringOrCelName() const;
    bool isObjectId() const;
    int getInt() const;
    QString getString() const;
    QString getStringOrCelNameForCompare() const;
    QString getCelName() const;
    int getObjectId() const;
    Type type() const;
    operator QString() const;
    /// Type safe compare
    bool strictEq(const FkissVariant &other) const;
    /// Make FkissVariant::operator== treat Cel and String as same type, for convenience.
    /// Because currently, for String and Cel, when parsing FKiSS script, I doesn't do extra type checking for each event/action and cast to corresponding type,
    /// but when interpreting in FKiSS runtime, a type checking is done.
    /// TODO: do type checking everywhere for consistency?
    bool operator==(const FkissVariant &other) const;
    bool operator!=(const FkissVariant &other) const;
private:
    Type m_type;
    int m_intValue;
    QString m_stringValue;
    /// Convert & cache according to Qt::CaseSensitivity, for qHash and operator==()
    QString m_stringValueForCompare;
    // Qt::CaseSensitivity m_caseSensitivity;
};


QDebug operator<<(QDebug debug, const FkissVariant& arg);
size_t qHash(const FkissVariant& key, size_t seed = 0);


#endif // FKISSVARIANT_H

