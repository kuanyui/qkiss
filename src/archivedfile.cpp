#include "archivedfile.h"

#include <QStringList>
#include <QDebug>

ArchivedFile::ArchivedFile(QObject *parent):
    QObject(parent),
    m_process(new QProcess(this))
{

}

void ArchivedFile::loadLzh(const QString &filePath)
{
    connect(m_process, &QProcess::readyRead, this, &ArchivedFile::onProcessReadyRead);
    connect(m_process, &QProcess::finished, this, &ArchivedFile::onProcessFinished);
    m_process->start("lha", { "x", filePath });
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
    while ( m_process->canReadLine() ) {
        QString raw = QString(m_process->readLine());
        qDebug() << "===> " << raw;
    }
}

void ArchivedFile::onProcessFinished()
{
    qDebug() << "<=== finished!";
}
