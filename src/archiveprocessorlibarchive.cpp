#include "archiveprocessorlibarchive.h"

namespace la {   // libarchive
#include <archive.h>
#include <archive_entry.h>
}

ArchiveProcessorLibarchive::ArchiveProcessorLibarchive(QObject *parent) :
    AbstractArchiveProcessor{parent}
{

}

ArchiveProcessorLibarchive::~ArchiveProcessorLibarchive()
{

}

QFuture<std::shared_ptr<ExtractedRawFilesMap> > ArchiveProcessorLibarchive::extractFileAsPromise(const QString &lzhOrCnfFilePath)
{

}

QHash<QString, QByteArray> ArchiveProcessorLibarchive::extractArchive(const QByteArray &archiveData)
{
    QHash<QString, QByteArray> filesDict;

    struct la::archive *arch = la::archive_read_new();
    /// a file in archive
    struct la::archive_entry *entry = nullptr;

    // la::archive_read_support_filter_all(arch);  // Filter is not needed for LHA format.
    la::archive_read_support_format_lha(arch);

    // List contents of Archive stored in Memory   // https://github.com/libarchive/libarchive/wiki/Examples#list-contents-of-archive-stored-in-memory
    if (la::archive_read_open_memory(arch, const_cast<char*>(archiveData.constData()), static_cast<size_t>(archiveData.size())) != ARCHIVE_OK) {
        qCritical() << tr("Error when opening this archive file. Aborted.");
        archive_read_free(arch);
        return filesDict;
    }

    while (la::archive_read_next_header(arch, &entry) == ARCHIVE_OK) {
        QString fileName = QString::fromUtf8(archive_entry_pathname(entry));
        QByteArray fileData;
        while (true) {
            const void *buf = nullptr;
            size_t size = 0;
            la::la_int64_t offset = 0;
            int readStatus = archive_read_data_block(arch, &buf, &size, &offset);
            if (readStatus == ARCHIVE_EOF) {
                break;
            }
            if (readStatus < ARCHIVE_OK) {
                qCritical() << tr("Error when reading this archive file. Aborted.");
                break;
            }
            fileData.append(static_cast<const char*>(buf), static_cast<int>(size));
        }
        filesDict.insert(fileName, fileData);
    }

    archive_read_close(arch);
    archive_read_free(arch);

    return filesDict;
}
