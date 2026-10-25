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
    /// When insensitive, you should always cache the upper-cased string.
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
    /// The preset of View > CRT Simulation. See CrtFilter. The numbers are saved in the settings file, so don't change them.
    enum class CrtSimulationPreset {
        /// Off
        None = 0,
        /// Copies CRT-Royale: every pixel stays sharp, and the colors have more contrast.
        Sharp = 1,
        /// Like Sharp, but neighboring pixels blend, so a checkerboard becomes one color.
        Soft = 2,
        /// Our own look: neighboring pixels blend, the colors stay the same, and bright areas glow.
        Glow = 3,
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
    static const QList<QPair<CrtSimulationPreset, QString>> crtSimulationPresetNames() {
        return {
            { QKiSS::CrtSimulationPreset::None, tr("None") },
            { QKiSS::CrtSimulationPreset::Sharp, tr("Sharp") },
            { QKiSS::CrtSimulationPreset::Soft, tr("Soft") },
            { QKiSS::CrtSimulationPreset::Glow, tr("Glow") },
        };
    }
};

// For storing enum class in QVariant
Q_DECLARE_METATYPE(QKiSS::Case);
Q_DECLARE_METATYPE(QKiSS::Encoding);
Q_DECLARE_METATYPE(QKiSS::LzhDecompressorBackend);

#endif // ENUMS_H
