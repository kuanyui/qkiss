#include "archivedfile.h"

#include <QStringList>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>

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
        qDebug() << "read file: " << fi.fileName() << "  -- from " << f.fileName();
        f.open(QIODevice::ReadOnly);
        QByteArray data = f.readAll();
        m_files->insert(fi.fileName(), data);
        f.close();
    }

    qDebug() << "<=== finished!";
    delete m_tmpDir;
}
