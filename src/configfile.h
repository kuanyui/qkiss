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
        static constexpr auto Global_CaseSensitivity        = "Global/CaseSensitivity";
        static constexpr auto Global_LzhDecompressorBackend = "Global/LzhDecompressorBackend ";
        static constexpr auto Global_Encoding               = "View/Encoding";
        static constexpr auto View_Zoom                     = "View/Zoom";
        static constexpr auto File_RecentFiles              = "File/RecentFiles";
        static constexpr auto File_RecentDirs               = "File/RecentDirs";
        static constexpr auto File_HomeDirectory            = "File/HomeDirectory";
    };
    static const int RECENTS_MAX_COUNT = 16;
    inline static QStringList ZOOM_LIST = { "4", "3.5", "3", "2.5", "2", "1.5", "1", "0.5" };
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

    QKiSS::Encoding encoding() const;
    void setEncoding(QKiSS::Encoding newEncoding);

    QKiSS::LzhDecompressorBackend lzhDecompressorBackend() const;
    void setLzhDecompressorBackend(QKiSS::LzhDecompressorBackend newLzhDecompressorBackend);

    QKiSS::Case caseSensitivity() const;
    void setCaseSensitivity(QKiSS::Case newCaseSensitivity);

    QStringList recentFiles() const;
    QStringList recentDirs() const;
    void appendRecentFile(QString filePath);

    QStringList zoomList() const;
    QString zoom() const;
    void setZoom(const QString &newZoom);

public slots:
    void zoomReset();
    QString zoomIn();
    QString zoomOut();

    void clearRecentFiles();
    void clearRecentDirs();

    void loadAll();
    void saveAll();


signals:

    void encodingChanged();
    void lzhDecompressorBackendChanged();
    void caseSensitivityChanged();
    void recentFilesChanged();
    void recentDirsChanged();

    void zoomScaleChanged();

private:
    /** Avoid initialized from outside. */
    ConfigFile(QObject *parent = nullptr);
    Q_DISABLE_COPY(ConfigFile)

    template<typename EnumClassT> EnumClassT getEnumClassFromSettings(const QString& cfgKey, EnumClassT defaultValue);

    QSettings m_set;
    QKiSS::Case m_caseSensitivity = QKiSS::Case::Insensitive;
    QKiSS::Encoding m_encoding = QKiSS::Encoding::SJIS;
    QKiSS::LzhDecompressorBackend m_lzhDecompressorBackend = QKiSS::LzhDecompressorBackend::Libarchive;
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
