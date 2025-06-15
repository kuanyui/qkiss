#include "archiveprocessorcnfinfilesystem.h"

#include <QDirIterator>
#include <QTemporaryDir>
using namespace Qt::Literals::StringLiterals;

ArchiveProcessorCnfInFileSystem::ArchiveProcessorCnfInFileSystem(QObject *parent)
    : AbstractArchiveProcessor{parent}
{

}

AbstractArchiveProcessor::Info ArchiveProcessorCnfInFileSystem::info() const
{
    return Info {};
}

bool ArchiveProcessorCnfInFileSystem::checkAvailability()
{
    return true;
}

QFuture<std::shared_ptr<ExtractedRawFilesMap> > ArchiveProcessorCnfInFileSystem::extractFileAsPromise(const QString &lzhOrCnfFilePath)
{
    QPromise<std::shared_ptr<ExtractedRawFilesMap>> *promise = new QPromise<std::shared_ptr<ExtractedRawFilesMap>>();
    QFuture<std::shared_ptr<ExtractedRawFilesMap>> future = promise->future();
    QFileInfo fileInfo {lzhOrCnfFilePath};
    QString fileName = fileInfo.fileName();
    m_extractedFiles = std::make_shared<QMap<QString, QByteArray>>();
    promise->start();
    if (!lzhOrCnfFilePath.endsWith(".CNF"_L1, Qt::CaseInsensitive)) {
        promise->setException(std::make_exception_ptr(
                                  std::runtime_error("\"%1\" is not a plain CNF file. Failed."_L1.arg(lzhOrCnfFilePath).toStdString())
                                  ));
        delete promise;
        return future;
    }
    qInfo().noquote() << tr("Reading a CNF file \"%1\" directly. Trying to fetch all related CNF/KCF/CEL files in the same folder...").arg(lzhOrCnfFilePath);
    promise->addResult(getRelatedFilesInTheSameFolder(lzhOrCnfFilePath));
    promise->finish();
    delete promise;
    return future;
}


std::shared_ptr<ExtractedRawFilesMap> ArchiveProcessorCnfInFileSystem::getRelatedFilesInTheSameFolder(const QString &cnfFilePath)
{
    auto fileContents = std::make_shared<QMap<QString, QByteArray>>();
    QFileInfo cnfFileInfo(cnfFilePath);
    QDir dir = cnfFileInfo.dir();
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);
    for (const QFileInfo &info : std::as_const(fileInfoList)) {
        QFile file(info.absoluteFilePath());
        qDebug() << "[CnfInFileSystem] " << info.absoluteFilePath();
        if (file.open(QIODevice::ReadOnly)) {
            fileContents->insert(info.fileName(), file.readAll());
            file.close();
        }
    }
    return fileContents;
}
