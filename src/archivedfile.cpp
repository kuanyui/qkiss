#include "archivedfile.h"

#include <QStringList>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
namespace icu {
#include <unicode/ucnv.h>
}

// #include <QStringConverter>  // Qt6. In Qt5 this is <QTextCodec>.  http://mochiuwiki.e2.valueserver.jp/index.php?title=Qt%E3%81%AE%E5%9F%BA%E7%A4%8E_-_%E6%96%87%E5%AD%97%E3%82%B3%E3%83%BC%E3%83%89

ArchivedFile::ArchivedFile(QObject *parent):
    QObject(parent),
    m_process(new QProcess(this)),
    m_files(new QHash<QString, QByteArray>())
{
    connect(m_process, &QProcess::readyRead, this, &ArchivedFile::onProcessReadyRead);
    connect(m_process, &QProcess::finished, this, &ArchivedFile::onProcessFinished);
}

bool ArchivedFile::loadLzh(const QString &filePath, Qt::CaseSensitivity caseSensitivity)
{
    m_archiveFilePath = filePath;
    m_caseSensitivity = caseSensitivity;
    m_files->clear();
    m_tmpDir = new QTemporaryDir(QDir(QDir::tempPath()).filePath("qtkiss-extracted-XXXXXX"));
    if (!m_tmpDir->isValid()) {
        return false;
    }
    m_process->start("lha", { "x",   // extract
                              // "-q",   // quiet
                              "-i",   // flatten folders in target
                              "-f",   // forces overwrite without asking.
                              "-w",   // specify extract path
                              m_tmpDir->path(),   // (extract to here)
                              "--archive-kanji-code=sjis",
                              m_archiveFilePath
                     });
    return true;
}

bool ArchivedFile::checkLhaCommandAvailability()
{
    QProcess p {};
    p.start("lha");
    p.waitForFinished();
    return p.exitCode() == 0;
}

void ArchivedFile::onProcessReadyRead()
{
    qsizetype prefixLen {m_tmpDir->path().length() + 1};
    while ( m_process->canReadLine() ) {
        QString raw = QString(m_process->readLine());
        if (raw.endsWith(QStringLiteral("\t- Melted  \n"))) {
            QString absPath {raw.split("\t")[0]};
            QString relPath {absPath.mid(prefixLen)};
            qDebug() << relPath;
        }
        //qDebug() << "===> " << raw;
    }
}


// Forked from https://note.com/rakko_v30/n/ncbe33ecaea13
std::string ShiftJisToUtf8(const std::string& input) {
    icu::UErrorCode error_code = icu::U_ZERO_ERROR;
    icu::UConverter* sjis_converter = ucnv_open("Shift-JIS", &error_code);
    if (U_FAILURE(error_code)) {
        qDebug() << "Failed to create Shift-JIS converter" << "\n";
        return "";
    }

    int32_t utf8_len = ucnv_toAlgorithmic(icu::UCNV_UTF8, sjis_converter, nullptr, 0, input.c_str(), input.length(), &error_code);
    if (error_code != icu::U_BUFFER_OVERFLOW_ERROR) {
        qDebug() << "Failed to estimate buffer size for UTF-8 conversion" << "\n";
        ucnv_close(sjis_converter);
        return "";
    }

    error_code = icu::U_ZERO_ERROR;
    std::string utf8_str(utf8_len, '\0');
    ucnv_toAlgorithmic(icu::UCNV_UTF8, sjis_converter, &utf8_str[0], utf8_len, input.c_str(), input.length(), &error_code);

    ucnv_close(sjis_converter);

    if (U_FAILURE(error_code)) {
        qDebug() << "Failed to convert Shift-JIS to UTF-8" << "\n";
        return "";
    }

    return utf8_str;
}
void ArchivedFile::onProcessFinished()
{
    QDirIterator it {
        m_tmpDir->path(),
        QDir::NoDotAndDotDot | QDir::AllEntries,
        QDirIterator::Subdirectories
    };
    while (it.hasNext()) {
        QString filePath = it.next();
        QFile f { filePath };
        QFileInfo fi { f };
        QString fileNameKey = fi.fileName();
        if (m_caseSensitivity == Qt::CaseInsensitive) {
            fileNameKey = fileNameKey.toUpper();
        }
        qDebug() << "read file: " << fileNameKey << "  -- from " << f.fileName();
        f.open(QIODevice::ReadOnly);
        QByteArray rawData = f.readAll();
        m_files->insert(fi.fileName(), rawData);
        if (fileNameKey.toUpper().endsWith(".CNF")) {
            qDebug() << QString::fromStdString( ShiftJisToUtf8(rawData.toStdString()) );
        }
        f.close();
    }
    qDebug() << "<=== finished!";
    delete m_tmpDir;
}
