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

#include "configfile.h"
#include <QSettings>
#include <QLocale>

// const: Gurantee it's runtime constant, mayby inlined.
// constexpr: Gurantee it's known during compilation time.
namespace {
constexpr qreal ZOOM_RATIO_ORIGIN = 1.0;
constexpr qreal ZOOM_RATIO_STEP = 1.1;
constexpr qreal ZOOM_RATIO_MIN  = 0.1;
constexpr qreal ZOOM_RATIO_MAX  = 8.0;
/// Large to Small
inline static QList<qreal> ZOOM_RATIO_LIST = { 4, 3, 2, 1, 0.5 };
}

ConfigFile::ConfigFile(QObject *parent)
    : QObject{parent}
{
    loadAll();
}


QStringList ConfigFile::recentFiles() const
{
    return m_set.value(CPath::File_RecentFiles).toStringList();
}


QStringList ConfigFile::recentDirs() const
{
    return m_set.value(CPath::File_RecentDirs).toStringList();
}

void ConfigFile::appendRecentFile(QString filePath)
{
    QFileInfo fi {filePath};
    QString dirPath = fi.dir().absolutePath();
    QStringList newFiles = recentFiles();
    QStringList newDirs = recentDirs();
    newFiles.removeAll(filePath);
    newFiles.prepend(filePath);
    while (newFiles.length() > RECENTS_MAX_COUNT) {
        newFiles.removeLast();
    }
    newDirs.removeAll(dirPath);
    newDirs.prepend(dirPath);
    while (newDirs.length() > RECENTS_MAX_COUNT) {
        newDirs.removeLast();
    }
    m_set.setValue(CPath::File_RecentFiles, newFiles);
    m_set.setValue(CPath::File_RecentDirs, newDirs);
    m_set.sync();
    emit recentFilesChanged();
    emit recentDirsChanged();
}


void ConfigFile::clearRecentFiles()
{
    m_set.setValue(CPath::File_RecentFiles, {});
    m_set.sync();
    emit recentFilesChanged();
}

void ConfigFile::clearRecentDirs()
{
    m_set.setValue(CPath::File_RecentDirs, {});
    m_set.sync();
    emit recentDirsChanged();
}


QByteArray ConfigFile::mainWindowGeometry() const
{
    return m_set.value(CPath::MainWindow_Geometry).toByteArray();
}

void ConfigFile::setMainWindowGeometry(const QByteArray &geometry)
{
    m_set.setValue(CPath::MainWindow_Geometry, geometry);
    m_set.sync();
}

QByteArray ConfigFile::mainWindowState() const
{
    return m_set.value(CPath::MainWindow_State).toByteArray();
}

void ConfigFile::setMainWindowState(const QByteArray &state)
{
    m_set.setValue(CPath::MainWindow_State, state);
    m_set.sync();
}

QString ConfigFile::homeDirectory() const
{
    return m_set.value(CPath::File_HomeDirectory, QDir::homePath()).toString();
}

void ConfigFile::setHomeDirectory(const QString &homeDir)
{
    m_set.setValue(CPath::File_HomeDirectory, homeDir);
    m_set.sync();
}

const QList<ConfigFile::LangInfo> ConfigFile::allLanguages()
{
    QString ja ("日本語");
    QString zh_TW ("台灣繁體");
    if (m_uiLanguage.startsWith("ja_") && !m_uiLanguage.endsWith("JP") && !m_uiLanguage.endsWith("KS")) {
        ja = "異世界通用語";
    }
    if (m_uiLanguage == "ja_AR") {
        zh_TW = QStringLiteral("ほとんど浸透されきって、哀れな言語ですわね…");
    } else if (m_uiLanguage == "ja_MK") {
        zh_TW = QStringLiteral("ここまで染まってしまっていて、さすがに雑魚扱いはできない言語かな…");
    } else if (m_uiLanguage == "ja_OJ") {
        zh_TW = QStringLiteral("…影響を強く受けてしまった言語でございますわね。");
    }
    QList<ConfigFile::LangInfo> list = {
        {"", "", "", tr("Auto (System Language)")},
        {"ja_JP", "🇯🇵", ":/flags/jp.svg", ja},
        {"ja_KS", "🇯🇵", ":/flags/jp.svg", ja + QString("（関西弁）")},
        {"ja_OJ", "🇯🇵", ":/flags/jp.svg", ja + QString("（お嬢様）")},
        {"ja_AR", "🇯🇵", ":/flags/jp.svg", ja + QString("（悪役令嬢）")},
        {"ja_MK", "🇯🇵", ":/flags/jp.svg", ja + QString("（メ〇〇キ）")},
        {"ja_ME", "🇯🇵", ":/flags/jp.svg", ja + QString("（〈沈黙の魔女〉モニカ・エヴァレット）")},
        {"en_US", "🇺🇸", ":/flags/us.svg", QString("American English")},
        {"zh_TW", "🇹🇼", ":/flags/tw.svg", zh_TW},
    };
    return list;
}

QString ConfigFile::detectSystemLanguage() const
{
    QLocale systemLocale = QLocale::system();
    QString sysLangId = systemLocale.name(); // "ja_JP", "en_US"
    QString sysLangPrefix = sysLangId.split('_').first();   // "ja"
    if (sysLangPrefix == "ja") { return "ja_JP"; }
    if (sysLangId == "zh_TW") { return "zh_TW"; }
    return "en_US";
}


QString ConfigFile::uiLanguage() const
{
    return m_uiLanguage;
    // return m_set.value(CPath::Global_UiLanguage, "").toString();
}

QString ConfigFile::effectedUiLanguage() const
{
    if (ConfigFile::engOnly()) {
        return "en_US";
    }
    if (m_uiLanguage.isEmpty()) {
        return detectSystemLanguage();
    }
    return m_uiLanguage;
}



bool ConfigFile::setUiLanguage(const QString &confLangId)
{
    if (confLangId == m_uiLanguage) { return true; }
    bool isValidLangId = false;
    for (const auto &li : allLanguages()) {
        if (li.id == confLangId) {
            isValidLangId = true;
            break;
        }
    }
    if (!isValidLangId) { return false; }

    // Write into QSettings
    m_uiLanguage = confLangId;
    m_set.setValue(CPath::Global_UiLanguage, confLangId);
    m_set.sync();
    emit uiLanguageChanged();
    return true;
}

bool ConfigFile::engOnly()
{
    QString l = QLocale::system().name().toLower();
    std::reverse(l.begin(), l.end());
    return l.endsWith("hz") && l[0] == 'n' && l[1] == 'c';
}

QKiSS::Encoding ConfigFile::encoding() const
{
    return m_encoding;
}

void ConfigFile::setEncoding(QKiSS::Encoding newEncoding)
{
    if (m_encoding == newEncoding)
        return;
    m_encoding = newEncoding;
    m_set.setValue(CPath::Global_Encoding, static_cast<int>(newEncoding));
    m_set.sync();
    emit encodingChanged();
}

QKiSS::Case ConfigFile::caseSensitivity() const
{
    return m_caseSensitivity;
}

void ConfigFile::setCaseSensitivity(QKiSS::Case newCaseSensitivity)
{
    if (m_caseSensitivity == newCaseSensitivity)
        return;
    m_caseSensitivity = newCaseSensitivity;
    m_set.setValue(CPath::Global_CaseSensitivity, static_cast<int>(newCaseSensitivity));
    m_set.sync();
    emit caseSensitivityChanged();
}

QKiSS::LzhDecompressorBackend ConfigFile::lzhDecompressorBackend() const
{
    return m_lzhDecompressorBackend;
}

void ConfigFile::setLzhDecompressorBackend(QKiSS::LzhDecompressorBackend newLzhDecompressorBackend)
{
    if (m_lzhDecompressorBackend == newLzhDecompressorBackend)
        return;
    m_lzhDecompressorBackend = newLzhDecompressorBackend;
    m_set.setValue(CPath::Global_LzhDecompressorBackend, static_cast<int>(newLzhDecompressorBackend));
    m_set.sync();
    emit lzhDecompressorBackendChanged();
}


template<typename EnumClassT>
EnumClassT ConfigFile::getEnumClassFromSettings(const QString& cfgKey, EnumClassT defaultValue) {
    bool ok;
    auto val = m_set.value(cfgKey, static_cast<int>(defaultValue)).toInt(&ok);
    if (!ok) {
        return defaultValue;
    }
    return static_cast<EnumClassT>(val);
}

qreal ConfigFile::parseZoomHumanToFactor(const QString &humanReadableZoom)
{
    QString trimmed = humanReadableZoom.trimmed();
    bool ok = false;
    qreal rawNum = 100;
    if (trimmed.endsWith('%')) {
        rawNum = trimmed.left(trimmed.size() - 1).toDouble(&ok);
    } else {
        rawNum = trimmed.toDouble(&ok);
    }
    if (!ok) { return ZOOM_RATIO_ORIGIN; }
    return rawNum / 100.0;
}

QString ConfigFile::stringifyZoomFactorToPercent(qreal factor)
{
    return QString::number(factor * 100.0, 'f', 0) + "%";
}

const QList<qreal> &ConfigFile::zoomFactorList() const
{
    return ZOOM_RATIO_LIST;
}

qreal ConfigFile::zoom() const
{
    // NOTE: Scale will be stored into ComboBox and QGraphicsView, so don't need to cache with a m_zoomScale here.
    bool ok = false;
    qreal val = m_set.value(CPath::View_Zoom).toReal(&ok);
    if (!ok) {
        val = ZOOM_RATIO_ORIGIN;
    };
    return val;
}

void ConfigFile::setZoom(qreal newFactor)
{
    if (zoom() == newFactor) { return; }
    if (newFactor > ZOOM_RATIO_MAX) { newFactor = ZOOM_RATIO_MAX; }
    else if (newFactor < ZOOM_RATIO_MIN) { newFactor = ZOOM_RATIO_MIN; }  // TODO: do validation at UI layer.
    m_set.setValue(CPath::View_Zoom, newFactor);
    m_set.sync();
    emit zoomChanged();
}

QString ConfigFile::zoomAsPercent() const
{
    return stringifyZoomFactorToPercent(zoom());
}

bool ConfigFile::canZoomIn() const
{
    if (zoom() >= ZOOM_RATIO_MAX) { return false; }
    return true;
}

bool ConfigFile::canZoomOut() const
{
    qreal val = zoom();
    if (val <= ZOOM_RATIO_MIN) { return false; }
    return true;
}

qreal ConfigFile::zoomReset()
{
    setZoom(1);
    return 1;
}

bool ConfigFile::zoomIn()
{
    if (!canZoomIn()) { return false; }
    qreal nv = zoom();
    nv *= ZOOM_RATIO_STEP;
    if (nv >= ZOOM_RATIO_MAX) { nv = ZOOM_RATIO_MAX; }
    setZoom(nv);
    return true;
}

bool ConfigFile::zoomOut()
{
    if (!canZoomOut()) { return false; }
    qreal nv = zoom();
    nv /= ZOOM_RATIO_STEP;
    if (nv <= ZOOM_RATIO_MIN) { nv = ZOOM_RATIO_MIN; }
    setZoom(nv);
    return true;
}

bool ConfigFile::zoomInByPredefinedSteps()
{
    if (!canZoomIn()) { return false; }
    const qreal ov = zoom();
    qreal nv = zoom();
    // 4, 3, 2, 1, 0.5  <-
    for (qsizetype i = ZOOM_RATIO_LIST.length() - 1; i >= 0; i--) {
        qreal x = ZOOM_RATIO_LIST[i];
        if (ov == x) { continue; }
        else if (ov > x) { continue; }
        else if (x > ov) { nv = x; break; }
    }
    if (nv != ov) {
        qDebug() << "[config] scale = " << nv;
        setZoom(nv);
        return true;
    }
    return true;
}

bool ConfigFile::zoomOutByPredefinedSteps()
{
    if (!canZoomOut()) { return false; }
    const qreal ov = zoom();
    qreal nv = zoom();
    // ->  4, 3, 2, 1, 0.5
    for (qsizetype i = 0; i < ZOOM_RATIO_LIST.length(); i++) {
        qreal x = ZOOM_RATIO_LIST[i];
        if (ov == x) { continue; }
        else if (ov > x) { nv = x; break; }
        else if (x > ov) { continue; }
    }
    if (nv != ov) {
        qDebug() << "[config] scale = " << nv;
        setZoom(nv);
        return true;
    }
    return false;
}

void ConfigFile::loadAll()
{
    // LZH backend
#ifdef HAVE_LIBARCHIVE
    m_lzhDecompressorBackend = QKiSS::LzhDecompressorBackend::Libarchive;
#endif
#if defined(Q_OS_UNIX)
    m_lzhDecompressorBackend = QKiSS::LzhDecompressorBackend::ExternalLhaProcess;
#endif

    m_encoding = getEnumClassFromSettings<QKiSS::Encoding>(CPath::Global_Encoding, QKiSS::Encoding::SJIS);
    m_caseSensitivity = getEnumClassFromSettings<QKiSS::Case>(CPath::Global_CaseSensitivity, QKiSS::Case::Insensitive);
    m_lzhDecompressorBackend = getEnumClassFromSettings<QKiSS::LzhDecompressorBackend>(CPath::Global_LzhDecompressorBackend, m_lzhDecompressorBackend);
    setUiLanguage(m_set.value(CPath::Global_UiLanguage).toString());
}

void ConfigFile::saveAll()
{
    m_set.setValue(CPath::Global_Encoding, static_cast<int>(m_encoding));
    m_set.setValue(CPath::Global_CaseSensitivity, static_cast<int>(m_caseSensitivity));
    m_set.setValue(CPath::Global_LzhDecompressorBackend, static_cast<int>(m_lzhDecompressorBackend));
    m_set.sync();
}
