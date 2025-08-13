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
