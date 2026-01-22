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

#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <QObject>
#include <QWidget>
#include <QSettings>
#include "archivefile.h"


class ConfigFile : public QObject
{
    struct CPath {
        static constexpr auto MainWindow_Geometry           = "MainWindow/Geometry";
        static constexpr auto MainWindow_State              = "MainWindow/State";
        static constexpr auto Global_UiLanguage             = "Global/UiLanguage";
        static constexpr auto Global_Encoding               = "Global/Encoding";
        static constexpr auto Global_CaseSensitivity        = "Global/CaseSensitivity";
        static constexpr auto Global_LzhDecompressorBackend = "Global/LzhDecompressorBackend ";
        static constexpr auto View_Zoom                     = "View/Zoom";
        static constexpr auto File_RecentFiles              = "File/RecentFiles";
        static constexpr auto File_RecentDirs               = "File/RecentDirs";
        static constexpr auto File_HomeDirectory            = "File/HomeDirectory";
    };
    static const int RECENTS_MAX_COUNT = 16;
    inline static QList<qreal> ZOOM_FACTOR_LIST = { 4, 3, 2, 1, 0.5 };
    struct LangInfo {
        const char* id;
        const char* flagEmoji;
        /// SVG file path in qrc. Use SVG file instead of emoji because Windows 11 built-in font removed flags from emoji list.
        const char* flagSvgFile;
        const QString label;
    };

    Q_OBJECT
public:
    // static constexpr std::array<float, 8> ZOOM_ALL { 4.0f, 3.5f, 3.0f, 2.5f, 2.0f, 1.5f, 1.0f, 0.5f };
    static ConfigFile& instance() {
        static ConfigFile instance;
        return instance;
    }

    QByteArray mainWindowGeometry() const;
    void setMainWindowGeometry(const QByteArray &geometry);
    QByteArray mainWindowState() const;
    void setMainWindowState(const QByteArray &state);

    QString homeDirectory() const;
    void setHomeDirectory(const QString &homeDir);
    /// QPair("ja_JP", "Japanese")
    const QList<LangInfo> allLanguages();
    QString detectSystemLanguage() const;

    QString uiLanguage() const;
    /// When language is set to empty string `""` (auto) in config file, this method will still return the "actually effected" language ID.
    QString effectedUiLanguage() const;
    bool setUiLanguage(const QString &langId);
    static bool engOnly();
    QKiSS::Encoding encoding() const;
    void setEncoding(QKiSS::Encoding newEncoding);

    QKiSS::LzhDecompressorBackend lzhDecompressorBackend() const;
    void setLzhDecompressorBackend(QKiSS::LzhDecompressorBackend newLzhDecompressorBackend);
    /// When insensitive, you should always cache the upper-cased string.
    QKiSS::Case caseSensitivity() const;
    void setCaseSensitivity(QKiSS::Case newCaseSensitivity);

    QStringList recentFiles() const;
    QStringList recentDirs() const;
    void appendRecentFile(QString filePath);
    static qreal parseZoomHumanToFactor(const QString &humanReadableZoom);
    static QString stringifyZoomFactorToPercent(qreal factor);
    const QList<qreal> &zoomFactorList() const;
    qreal zoom() const;
    void setZoom(qreal newFactor);
    QString zoomAsPercent() const;
    bool canZoomIn() const;
    bool canZoomOut() const;

public slots:
    qreal zoomReset();
    bool zoomIn();
    bool zoomOut();

    void clearRecentFiles();
    void clearRecentDirs();

    /// This is called automatically when initializing.
    void loadAll();
    void saveAll();


signals:

    void encodingChanged();
    void lzhDecompressorBackendChanged();
    void caseSensitivityChanged();
    void recentFilesChanged();
    void recentDirsChanged();
    void uiLanguageChanged();

    void zoomChanged();

private:
    /** Avoid initialized from outside. */
    ConfigFile(QObject *parent = nullptr);
    Q_DISABLE_COPY(ConfigFile)

    template<typename EnumClassT> EnumClassT getEnumClassFromSettings(const QString& cfgKey, EnumClassT defaultValue);

    QSettings m_set;
    QKiSS::Case m_caseSensitivity = QKiSS::Case::Insensitive;
    QKiSS::Encoding m_encoding = QKiSS::Encoding::SJIS;
    QKiSS::LzhDecompressorBackend m_lzhDecompressorBackend = QKiSS::LzhDecompressorBackend::Libarchive;
    QString m_uiLanguage = "";
    // /** 0 ~ 9 */
    // uint8_t m_cnfSetNo;
    // /** usually it's 9. */
    // uint8_t m_cnfSetNoMax;
    // /** paletteNo in a KCF file */
    // uint8_t m_cnfPletteNo;
    Q_PROPERTY(QKiSS::Encoding encoding READ encoding WRITE setEncoding NOTIFY encodingChanged FINAL)
    Q_PROPERTY(QKiSS::LzhDecompressorBackend lzhDecompressorBackend READ lzhDecompressorBackend WRITE setLzhDecompressorBackend NOTIFY lzhDecompressorBackendChanged FINAL)
    Q_PROPERTY(QKiSS::Case caseSensitivity READ caseSensitivity WRITE setCaseSensitivity NOTIFY caseSensitivityChanged FINAL)
};

#endif // CONFIGFILE_H
