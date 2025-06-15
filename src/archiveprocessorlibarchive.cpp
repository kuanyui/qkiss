#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

#ifdef HAVE_LIBARCHIVE

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

AbstractArchiveProcessor::Info ArchiveProcessorLibarchive::info() const
{
    return Info {
        .name = tr("LibArchive (built-in)"),
        .summary = tr("Multi-format archive and compression library."),
        .lzhCompatibility = tr("Poor (some LZH files may be unable to be extracted, and non UTF-8 file names in LZH are totally unsupported.)"),
        .installation = tr("Built-in in QKiSS. No extra external program is required."),
        .osSupport = tr("Windows, macOS, Linux"),
        .diskOperation = tr("All operations can be done in RAM instead of extracting files to disk, relatively friendly to the SSD’s lifespan."),
        .website = tr("<a href='https://github.com/libarchive/libarchive'>Source Code</a></li>")
    };
}

bool ArchiveProcessorLibarchive::checkAvailability()
{
    return true;
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
        qCritical().noquote() << tr("Failed to open this file, aborted.");
        promise->setException(std::make_exception_ptr( std::runtime_error("Failed to open file \"%1\", aborted."_L1.arg(lzhOrCnfFilePath).toStdString()) ));
        promise->finish();    // NOTE: `QPromise::finished()` is still need to be called after `QPromise::setException()`. Documentation doesn't clearly say this.
        delete promise;
        return future;
    }
    QByteArray fileRawContent = file.readAll();
    file.close();
    // ATTENTION: `QFuture::then()` will be executed in the thread where the `QPromise::finished()` was called,
    // so specify the execution context (`this`) for this lambda here.
    extractArchiveInMemory(fileRawContent).then(this, [promise] (QFuture<std::shared_ptr<ExtractedRawFilesMap>> res) {
        promise->addResult(res.result());
        promise->finish();
        delete promise;
    });
    return future;
}

QFuture<std::shared_ptr<ExtractedRawFilesMap> > ArchiveProcessorLibarchive::extractArchiveInMemory(const QByteArray &archiveData)
{
    // NOTE: This method should NEVER return a filesDict contains any LZH. All nested LZH file must be extracted before return / promise finished.
    std::shared_ptr<ExtractedRawFilesMap> finalExtractedFilesDict = std::make_shared<ExtractedRawFilesMap>();
    // QPromise: producer; can add result or set exception. (it's a move-only class https://doc.qt.io/qt-6/qpromise.html )
    // QFuture: consumer; has `.then()`
    QPromise<std::shared_ptr<ExtractedRawFilesMap>> *promise = new QPromise<std::shared_ptr<ExtractedRawFilesMap>>();
    QFuture<std::shared_ptr<ExtractedRawFilesMap>> future = promise->future();
    promise->start();
    QtConcurrent::run(callLibarchiveToExtractArchiveInMemory, archiveData).then([this, promise, finalExtractedFilesDict] (QFuture<std::shared_ptr<ExtractedRawFilesMap>> resFut) {
        auto libarchiveFilesDict = resFut.result();
        for (auto laIt = libarchiveFilesDict->begin(); laIt != libarchiveFilesDict->end(); laIt++) {
            if (laIt.key().endsWith(".LZH"_L1, Qt::CaseInsensitive)) {
                // Found another LZH in this LZH, call this method again and wait for finished.
                auto nestedLzhFileContent = laIt.value();
                auto nestedFuture = extractArchiveInMemory(nestedLzhFileContent);
                nestedFuture.waitForFinished();
                auto nestedExtractedFilesDict = nestedFuture.result();
                finalExtractedFilesDict->insert(*nestedExtractedFilesDict);
            } else {
                finalExtractedFilesDict->insert(laIt.key(), laIt.value());
            }
        }
        promise->addResult(finalExtractedFilesDict);
        promise->finish();
        delete promise;
    });
    return future;
}

std::shared_ptr<ExtractedRawFilesMap> ArchiveProcessorLibarchive::callLibarchiveToExtractArchiveInMemory(const QByteArray &archiveData)
{
    std::shared_ptr<ExtractedRawFilesMap> filesDict = std::make_shared<ExtractedRawFilesMap>();
    struct la::archive *arch = la::archive_read_new();
    /// an entry means a file in this archive
    struct la::archive_entry *entry = nullptr;

    la::archive_read_support_format_lha(arch);
    // la::archive_read_support_filter_all(arch);  // Filter is not needed for LHA format.

    // List contents of Archive stored in Memory   // https://github.com/libarchive/libarchive/wiki/Examples#list-contents-of-archive-stored-in-memory
    if (la::archive_read_open_memory(arch, const_cast<char*>(archiveData.constData()), static_cast<size_t>(archiveData.size())) != ARCHIVE_OK) {
        const char *errMsg = la::archive_error_string(arch);
        qCritical().noquote() << tr("[Error][LibArchive] Error when opening this archive file (%1). Aborted.").arg(errMsg);
        archive_read_free(arch);
        return filesDict;
    }

    while (true) {
        auto r = la::archive_read_next_header(arch, &entry);
        if (r == ARCHIVE_FATAL) {
            qCritical().noquote() << tr("[Error][LibArchive] Fatal error (%1), no more operations are possible., abort.").arg(la::archive_error_string(arch));
            break;
        } else if (r == ARCHIVE_FAILED) {
            qCritical().noquote() << tr("[Warning][LibArchive] Current operation cannot complete (%1), skip.").arg(la::archive_error_string(arch));
            break;
        } else if (r == ARCHIVE_WARN) {
            qWarning().noquote() << tr("[Warning][Libarchive] Partial success (%1).").arg(la::archive_error_string(arch));
            break;
        } else if (r == ARCHIVE_RETRY) {
            qWarning().noquote() << tr("[Warning][Libarchive] Retry might succeed (%1)").arg(la::archive_error_string(arch));
            continue;
        }

        QString filePath = QString::fromUtf8(la::archive_entry_pathname_utf8(entry));   // FIXME: Donno how to retrieve the raw char* containing non-ASCII code......
        qDebug().noquote() << "[DEBUG][LibArchive] archive_read_next_header() == %1"_L1.arg(filePath);
        QByteArray fileData;
        while (true) {
            const void *buf = nullptr;
            size_t size = 0;
            la::la_int64_t offset = 0;
            int readStatus = la::archive_read_data_block(arch, &buf, &size, &offset);
            if (readStatus == ARCHIVE_EOF) {   // EOF of a file in this archive
                // qInfo().noquote() << tr("[Info][LibArchive] EOF of archive.");
                break;
            }
            if (readStatus < ARCHIVE_OK) {   // Operation was successful
                const char *errMsg = la::archive_error_string(arch);
                qCritical().noquote() << tr("[Error][LibArchive] when reading this archive file via LibArchive (Error message: %1). Aborted.").arg(errMsg);
                break;
            }
            fileData.append(static_cast<const char*>(buf), static_cast<int>(size));
        }
        QString fileName = filePath.split('/').last();
        filesDict->insert(fileName, fileData);
    }

    la::archive_read_free(arch);

    return filesDict;
}

#endif


