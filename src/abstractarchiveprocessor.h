#ifndef ABSTRACTARCHIVEPROCESSOR_H
#define ABSTRACTARCHIVEPROCESSOR_H

#include <QObject>
#include <QPromise>
#include <QFuture>
#include <QMap>
#include <QElapsedTimer>
#include <QDebug>
using ExtractedRawFilesMap = QMap<QString, QByteArray>;

/// QPair<Filename, Size>. Size is in bytes.
using FileSummary = QPair<QString, int>;

/// `QHash<K, V>` and `QMap<K, V>` do NEVER remember the ordering of insertion. Use QVector<QPair<K, V>> to replace it.
using FilesSummaryList = QVector<QPair<QString, int>>;

class AbstractArchiveProcessor : public QObject
{
    Q_OBJECT
public:
    AbstractArchiveProcessor(QObject *parent);
    virtual ~AbstractArchiveProcessor();
    virtual bool checkAvailability() = 0;
    /// This function merely extract a LZH file, doesn't do any other checks.
    /// TODO: Should be able to accept a CNF file. Should rename this function.
    virtual QFuture<std::shared_ptr<ExtractedRawFilesMap>> extractFileAsPromise(const QString &lzhOrCnfFilePath) = 0;
protected:
    static QString generateTempDirPath();
private:
    static int instanceCount;
};


#endif // ABSTRACTARCHIVEPROCESSOR_H
