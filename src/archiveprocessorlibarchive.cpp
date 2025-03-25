#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

#include "archiveprocessorlibarchive.h"

namespace la {   // libarchive
#include <archive.h>
#include <archive_entry.h>
}

using namespace Qt::Literals::StringLiterals;

ArchiveProcessorLibarchive::ArchiveProcessorLibarchive(QObject *parent) :
    AbstractArchiveProcessor{parent}
{

}

ArchiveProcessorLibarchive::~ArchiveProcessorLibarchive()
{

}



QFuture<std::shared_ptr<ExtractedRawFilesMap> > ArchiveProcessorLibarchive::extractFileAsPromise(const QString &lzhOrCnfFilePath)
{
    // QPromise: producer; can add result or set exception. (it's a move-only class https://doc.qt.io/qt-6/qpromise.html )
    // QFuture: consumer; has `.then()`
    QPromise<std::shared_ptr<ExtractedRawFilesMap>> *promise = new QPromise<std::shared_ptr<ExtractedRawFilesMap>>();
    QFuture<std::shared_ptr<ExtractedRawFilesMap>> future = promise->future();
    // QFutureWatcher<std::shared_ptr<ExtractedRawFilesMap>> *watcher = new QFutureWatcher<std::shared_ptr<ExtractedRawFilesMap>>(this);
    promise->start();
    QFile file(lzhOrCnfFilePath);
    if (!file.open(QIODevice::OpenModeFlag::ReadOnly)) {
        qCritical() << tr("Failed to open this file, aborted.");
        promise->setException(std::make_exception_ptr( std::runtime_error("Failed to open file \"%1\", aborted."_L1.arg(lzhOrCnfFilePath).toStdString()) ));
        promise->finish();    // NOTE: `QPromise::finished()` is still need to be called after `QPromise::setException()`. Documentation doesn't clearly say this.
        delete promise;
        return future;
    }
    QByteArray fileRawContent = file.readAll();
    file.close();
    QtConcurrent::run(extractArchive, fileRawContent).then([promise] (QFuture<std::shared_ptr<ExtractedRawFilesMap>> res) {
        // ATTENTION: `QFuture::then()` will be executed in the thread where the `QPromise::finished()` was called,
        // so force call lambda back in the main thread via QMetaObject::invokeMethod().
        QMetaObject::invokeMethod(
            QCoreApplication::instance(),
            [promise, res]() {
                promise->addResult(res.result());
                promise->finish();
                delete promise;
            }
        );
    });
    return future;
}

std::shared_ptr<ExtractedRawFilesMap> ArchiveProcessorLibarchive::extractArchive(const QByteArray &archiveData)
{
    std::shared_ptr<ExtractedRawFilesMap> filesDict = std::make_shared<ExtractedRawFilesMap>();

    struct la::archive *arch = la::archive_read_new();
    /// an entry means a file in this archive
    struct la::archive_entry *entry = nullptr;

    la::archive_read_support_format_lha(arch);
    // la::archive_read_support_filter_all(arch);  // Filter is not needed for LHA format.

    // List contents of Archive stored in Memory   // https://github.com/libarchive/libarchive/wiki/Examples#list-contents-of-archive-stored-in-memory
    if (la::archive_read_open_memory(arch, const_cast<char*>(archiveData.constData()), static_cast<size_t>(archiveData.size())) != ARCHIVE_OK) {
        qCritical() << tr("Error when opening this archive file. Aborted.");
        archive_read_free(arch);
        return filesDict;
    }

    while (la::archive_read_next_header(arch, &entry) == ARCHIVE_OK) {
        QString filePath = QString::fromUtf8(archive_entry_pathname(entry));
        QByteArray fileData;
        while (true) {
            const void *buf = nullptr;
            size_t size = 0;
            la::la_int64_t offset = 0;
            int readStatus = la::archive_read_data_block(arch, &buf, &size, &offset);
            if (readStatus == ARCHIVE_EOF) {
                break;
            }
            if (readStatus < ARCHIVE_OK) {
                qCritical() << tr("Error when reading this archive file. Aborted.");
                break;
            }
            fileData.append(static_cast<const char*>(buf), static_cast<int>(size));
        }
        QString fileName = filePath.split('/').last();
        filesDict->insert(fileName, fileData);
    }

    archive_read_free(arch);

    return filesDict;
}
