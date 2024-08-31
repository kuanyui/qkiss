#include "archivedfile.h"

#include <QStringList>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>


// #include <QStringConverter>  // Qt6. In Qt5 this is <QTextCodec>.  http://mochiuwiki.e2.valueserver.jp/index.php?title=Qt%E3%81%AE%E5%9F%BA%E7%A4%8E_-_%E6%96%87%E5%AD%97%E3%82%B3%E3%83%BC%E3%83%89

ArchivedFile::ArchivedFile(QObject *parent):
    QObject(parent),
    m_process(new QProcess(this)),
    m_parsedCnfFiles(new QMap<QString, ParsedCnf*>),
    m_parsedKcfFiles(new QHash<QString, ParsedKcf*>),
    m_parsedCelFiles(new QHash<QString, ParsedCel*>),
    m_otherFiles(new QMap<QString, QByteArray>())
{
    connect(m_process, &QProcess::readyRead, this, &ArchivedFile::onProcessReadyRead);
    connect(m_process, &QProcess::finished, this, &ArchivedFile::onProcessFinished);
}

bool ArchivedFile::loadLzh(const QString &filePath, QtKISS::Case caseSensitivity)
{
    m_archiveFilePath = filePath;
    m_caseSensitivity = caseSensitivity;
    m_parsedCnfFiles->clear();
    m_parsedKcfFiles->clear();
    m_parsedCelFiles->clear();
    m_otherFiles->clear();
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

QStringList ArchivedFile::getCnfNameList()
{
    return m_parsedCnfFiles->keys();
}

QStringList ArchivedFile::getOtherFileNameList()
{
    return m_otherFiles->keys();
}


std::optional<ParsedCnf *> ArchivedFile::getParsedCnf(QString cnfFileName) const
{
    if (m_caseSensitivity == QtKISS::Case::Insensitive) {
        cnfFileName = cnfFileName.toUpper();
    }
    if (m_parsedCnfFiles->contains(cnfFileName)) {
        return m_parsedCnfFiles->value(cnfFileName);
    }
    return std::nullopt;
}

std::optional<ParsedKcf *> ArchivedFile::getParsedKcf(QString kcfFileName) const
{
    if (m_caseSensitivity == QtKISS::Case::Insensitive) {
        kcfFileName = kcfFileName.toUpper();
    }
    if (m_parsedKcfFiles->contains(kcfFileName)) {
        return std::optional<ParsedKcf*>(m_parsedKcfFiles->value(kcfFileName));
    }
    return std::nullopt;
}

std::optional<ParsedCel *> ArchivedFile::getParsedCel(QString celFileName) const
{
    if (m_caseSensitivity == QtKISS::Case::Insensitive) {
        celFileName = celFileName.toUpper();
    }
    if (m_parsedCelFiles->contains(celFileName)) {
        return std::optional<ParsedCel*>(m_parsedCelFiles->value(celFileName));
    }
    return std::nullopt;
}

std::optional<QByteArray> ArchivedFile::getOtherFile(QString fileName) const
{
    if (m_caseSensitivity == QtKISS::Case::Insensitive) {
        fileName = fileName.toUpper();
    }
    if (m_otherFiles->contains(fileName)) {
        return std::optional<QByteArray>(m_otherFiles->value(fileName));
    }
    return std::nullopt;
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
        QString fileNameKey = fi.fileName();
        if (m_caseSensitivity == QtKISS::Case::Insensitive) {
            fileNameKey = fileNameKey.toUpper();
        }
        qDebug() << "read file: " << fileNameKey << "  -- from " << f.fileName();
        f.open(QIODevice::ReadOnly);
        QByteArray rawData = f.readAll();
        if (fileNameKey.toUpper().endsWith(".CNF")) {
            // qDebug() << Encoding::toUtf8(rawData, "Shift_JIS");
            // qDebug() << QString::fromStdString( Encoding::toUtf8(rawData.toStdString(), "Shift_JIS") );
            m_parsedCnfFiles->insert(fileNameKey, new ParsedCnf {fileNameKey, rawData, QtKISS::Encoding::SJIS});
        } else if (fileNameKey.toUpper().endsWith(".KCF")) {
            m_parsedKcfFiles->insert(fileNameKey, new ParsedKcf {fileNameKey, rawData});
        } else if (fileNameKey.toUpper().endsWith(".CEL")) {
            m_parsedCelFiles->insert(fileNameKey, new ParsedCel {fileNameKey, rawData});
        } else {
            m_otherFiles->insert(fileNameKey, rawData);
        }
        f.close();
    }
    qDebug() << "<=== finished!";
    delete m_tmpDir;
}
