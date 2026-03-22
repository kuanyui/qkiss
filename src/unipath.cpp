/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2026 ono ono (kuanyui) <azazabc123@gmail.com>
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
#include "unipath.h"
#include <QFileInfo>
#include <QDir>

UniPath::UniPath(const QString &path)
{
    init(path);
}

QString UniPath::absolutePath() const
{
    return m_absPath;
}

QString UniPath::absolutePathNative() const
{
    return QDir::toNativeSeparators(m_absPath);
}

bool UniPath::exists() const
{
    return m_fInfo.exists();
}

bool UniPath::isDir() const
{
    return m_fInfo.isDir();
}

bool UniPath::isFile() const
{
    return m_fInfo.isFile();
}

bool UniPath::init(const QString &path)
{
    // ex: `C:\\Data/` -> `C:/Data`
    QString fixedPath = QDir::cleanPath(QDir::fromNativeSeparators(path));   // QDir::cleanPath() always remove trailing `/` (except for root dir)
    m_fInfo.setFile(fixedPath);
    if (!m_fInfo.exists()) { return false; }
    if (m_fInfo.isDir()) {
        // FileInfo::absolutePath() is wrong when `dirPath` in `FileInfo(dirPath)` is not ends with `/`.
        if (!fixedPath.endsWith("/")) {
            m_fInfo.setFile(fixedPath + "/");
            m_absPath = m_fInfo.absoluteFilePath();
        } else {
            m_absPath = m_fInfo.absoluteFilePath();
        }
    } else {
        if (!fixedPath.endsWith("/")) {
            m_fInfo.setFile(fixedPath.sliced(0, fixedPath.size() - 1));
            m_absPath = m_fInfo.absoluteFilePath();
        } else {
            m_absPath = m_fInfo.absoluteFilePath();
        }
    }
    return true;
}

QString UniPath::formalizePath(const QString &dirOrFilePath)
{
    QFileInfo rawInfo {QDir::cleanPath(QDir::fromNativeSeparators(dirOrFilePath))};
    QString absPath {};
    if (!rawInfo.exists()) { return ""; }
    if (rawInfo.isDir()) {
        // FileInfo::absolutePath() is wrong when `dirPath` in `FileInfo(dirPath)` is not ends with `/`.
        if (!dirOrFilePath.endsWith("/")) {
            QFileInfo fixedInfo { dirOrFilePath + "/" };
            absPath = fixedInfo.absoluteFilePath();
        } else {
            absPath = rawInfo.absoluteFilePath();
        }
    } else {
        if (!dirOrFilePath.endsWith("/")) {
            QFileInfo fixedInfo { dirOrFilePath.sliced(0, dirOrFilePath.size() - 1) };
            absPath = fixedInfo.absoluteFilePath();
        } else {
            absPath = rawInfo.absoluteFilePath();
        }
    }
    // return QDir::toNativeSeparators(absPath);  // For windows
    return absPath;  // For windows
}
