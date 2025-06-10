#include "configfile.h"
#include <QSettings>


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

QString ConfigFile::uiLanguage() const
{
    return m_set.value(CPath::Global_UiLanguage, "en_US").toString();;
}

void ConfigFile::setUiLanguage(const QString &langId)
{
    m_set.setValue(CPath::Global_UiLanguage, langId);
    m_set.sync();
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

QStringList ConfigFile::zoomList() const
{
    return ZOOM_LIST;
}

QString ConfigFile::zoom() const
{
    // NOTE: Scale will be stored into ComboBox and QGraphicsView, so don't need to cache with a m_zoomScale here.
    QString val = m_set.value(CPath::View_Zoom).toString();
    if (val.isEmpty()) {
        val = "1";
    };
    return val;
}

void ConfigFile::setZoom(const QString &newZoom)
{
    if (zoom() == newZoom)
        return;
    m_set.setValue(CPath::View_Zoom, newZoom);
    m_set.sync();
    emit zoomScaleChanged();
}

void ConfigFile::zoomReset()
{
    setZoom("1");
}

QString ConfigFile::zoomIn()
{
    QString ov = zoom();
    int i = ZOOM_LIST.indexOf(ov) - 1;
    i = qMax(i, 0);
    QString nv = ZOOM_LIST[i];
    setZoom(nv);
    return nv;
}

QString ConfigFile::zoomOut()
{
    QString ov = zoom();
    int i = ZOOM_LIST.indexOf(ov) + 1;
    i = qMin(i, ZOOM_LIST.count() - 1);
    QString nv = ZOOM_LIST[i];
    setZoom(nv);
    return nv;
}

void ConfigFile::loadAll()
{
    // LZH backend
#ifdef HAVE_LIBARCHIVE
    m_lzhDecompressorBackend = QKiSS::LzhDecompressorBackend::Libarchive;
#endif
#if defined(Q_OS_UNIX) || !defined(HAVE_LIBARCHIVE)
    m_lzhDecompressorBackend = QKiSS::LzhDecompressorBackend::ExternalLhaProcess;
#endif

    m_encoding = getEnumClassFromSettings<QKiSS::Encoding>(CPath::Global_Encoding, QKiSS::Encoding::SJIS);
    m_caseSensitivity = getEnumClassFromSettings<QKiSS::Case>(CPath::Global_CaseSensitivity, QKiSS::Case::Insensitive);
    m_lzhDecompressorBackend = getEnumClassFromSettings<QKiSS::LzhDecompressorBackend>(CPath::Global_LzhDecompressorBackend, m_lzhDecompressorBackend);
}

void ConfigFile::saveAll()
{
    m_set.setValue(CPath::Global_Encoding, static_cast<int>(m_encoding));
    m_set.setValue(CPath::Global_CaseSensitivity, static_cast<int>(m_caseSensitivity));
    m_set.setValue(CPath::Global_LzhDecompressorBackend, static_cast<int>(m_lzhDecompressorBackend));
    m_set.sync();
}
