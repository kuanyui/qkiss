/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2024 - 2025 ono ono (kuanyui) <azazabc123@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifndef ABSTRACTARCHIVEPROCESSOR_H
#define ABSTRACTARCHIVEPROCESSOR_H

#include <QObject>
#include <QPromise>
#include <QFuture>
#include <QMap>
#include <QElapsedTimer>
#include <QDebug>
#include "enums.h"
using ExtractedRawFilesMap = QMap<QString, QByteArray>;

/// QPair<Filename, Size>. Size is in bytes.
using FileSummary = QPair<QString, int>;

/// `QHash<K, V>` and `QMap<K, V>` do NEVER remember the ordering of insertion. Use QVector<QPair<K, V>> to replace it.
using FilesSummaryList = QVector<QPair<QString, int>>;

class AbstractArchiveProcessor : public QObject
{
    Q_OBJECT

public:
    struct Info {
        QKiSS::LzhDecompressorBackend id;
        QString name;
        QString summary;
        QString lzhCompatibility;
        QString installation;
        QString osSupport;
        QString diskOperation;
        QString website;
    };
    AbstractArchiveProcessor(QObject *parent);
    virtual ~AbstractArchiveProcessor();
    /// Displayed in setting page.
    virtual Info info() const = 0;
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
