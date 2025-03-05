#include <QIcon>
#include "fileiconprovider.h"
using namespace Qt::Literals::StringLiterals;

FileIconProvider::FileIconProvider()
    : QFileIconProvider{}
{
}

QIcon FileIconProvider::icon(IconType type) const
{
    switch (type) {
    case QAbstractFileIconProvider::IconType::Computer: return QIcon::fromTheme("computer");
    case QAbstractFileIconProvider::IconType::Desktop: return QIcon::fromTheme("text-x-preview");
    case QAbstractFileIconProvider::IconType::Trashcan: return QIcon::fromTheme("user-trash");
    case QAbstractFileIconProvider::IconType::Network: return QIcon::fromTheme("folder-remote");
    case QAbstractFileIconProvider::IconType::Drive: return QIcon::fromTheme("media-optical");
    case QAbstractFileIconProvider::IconType::Folder: return QIcon::fromTheme("folder");
    case QAbstractFileIconProvider::IconType::File: return QIcon::fromTheme("text-x-preview");
    default: return QIcon::fromTheme("text-x-preview");
    }
}

QIcon FileIconProvider::icon(const QFileInfo &info) const
{
    if (info.suffix().toUpper() == "LZH") {
        return QIcon::fromTheme("package-zipped");
    }
    if (info.isDir()) {
        return icon(IconType::Folder);
    }
    return icon(IconType::File);
}
