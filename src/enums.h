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

#ifndef ENUMS_H
#define ENUMS_H
#include <qnamespace.h>
#include <QCoreApplication>

class QKiSS {
    Q_DECLARE_TR_FUNCTIONS(QKiSS)   // Use tr() without Q_OBJECT

public:
    enum class Case {
        Sensitive = Qt::CaseSensitive,
        Insensitive = Qt::CaseInsensitive
    };
    /** https://www.iana.org/assignments/character-sets/character-sets.xhtml */
    enum class Encoding {
        ASCII,
        UTF8,
        // Japanese
        SJIS,
        EUC_JP,
    };
    enum class FileLoadMode {
        LoadAsIndependent,
        LoadAsPatch,
    };
    enum class LzhDecompressorBackend {
        /// read CNF file in plain folder directly.
        PlainFolder,
        /// LHA
        ExternalLhaProcess,
        /// Unarchive
        ExternalUnarProcess,
        /// Libarchive
        Libarchive,
    };
    static const QStringList encodingNames() {
        return {
            tr("ASCII"),
            tr("UTF-8"),
            tr("Shift JIS"),
            tr("EUC-JP")
        };
    };
    static const QList<QPair<Case, QString>> caseSensitivityNames() {
        return {
            { QKiSS::Case::Insensitive, tr("Case Insensitive") },
            { QKiSS::Case::Sensitive, tr("Case Sensitive") },
        };
    }
};

// For storing enum class in QVariant
Q_DECLARE_METATYPE(QKiSS::Case);
Q_DECLARE_METATYPE(QKiSS::Encoding);
Q_DECLARE_METATYPE(QKiSS::LzhDecompressorBackend);

#endif // ENUMS_H
