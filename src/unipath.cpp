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

const QFileInfo &UniPath::fileInfo() const
{
    return m_fInfo;
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

UniPath UniPath::parent() const
{
    return UniPath(m_fInfo.dir().absolutePath());
}

QString UniPath::lastDirName() const
{
    if (isFile()) {
        return m_fInfo.dir().dirName();
    } else {
        return m_fInfo.baseName();
    }
}

QString UniPath::formalizePath(const QString &dirOrFilePath)
{
    QString cleanPath = QDir::cleanPath(QDir::fromNativeSeparators(dirOrFilePath));   // Never has `/` suffix.
    QFileInfo fInfo { cleanPath };
    /// Input is abspath
    bool isAbs = fInfo.isAbsolute();
    /// Input is already has slash suffix
    bool isDir = dirOrFilePath.endsWith("/") || dirOrFilePath.endsWith("\\");
    /// QFileInfo a ("/usr/share");  qDebug() << "a" << a.isDir() << a.absoluteFilePath();  // ===> a  true  "/usr/share"
    /// QFileInfo b ("/usr/share/"); qDebug() << "b" << b.isDir() << b.absoluteFilePath();  // ===> b  true  "/usr/share/"
    if (!fInfo.exists()) { return dirOrFilePath; }
    if (isDir || fInfo.isDir()) {
        if (cleanPath.length() == 1) {  // root path `/`
            return isAbs ? fInfo.absoluteFilePath() : fInfo.filePath();
        } else {
            return (isAbs ? fInfo.absoluteFilePath() : fInfo.filePath())+ "/";
        }
    } else {
        return isAbs ? fInfo.absoluteFilePath() : fInfo.filePath();
    }
}

QString UniPath::formalizePath(const QString &path, bool isDir)
{
    QString cleanPath = QDir::cleanPath(QDir::fromNativeSeparators(path));   // Never has `/` suffix.
    if (isDir) {
        if (cleanPath.length() == 1) { return path; }  // root path `/`
        return path + "/";
    }
    return path;
}


bool UniPath::init(const QString &path)
{
    // - ex: `C:\\Data/` -> `C:/Data`
    // - QDir::cleanPath() always remove trailing `/` (except for root dir)
    // - QDir::fromNativeSeparators() only works on Windows.
    QString cleanPath = QDir::cleanPath(QDir::fromNativeSeparators(path));   // Never has `/` suffix.
    m_fInfo.setFile(cleanPath);

    /// QFileInfo a ("/usr/share");  qDebug() << "a" << a.isDir() << a.absoluteFilePath();  // ===> a  true  "/usr/share"
    /// QFileInfo b ("/usr/share/"); qDebug() << "b" << b.isDir() << b.absoluteFilePath();  // ===> b  true  "/usr/share/"
    if (!m_fInfo.exists()) { return false; }
    if (m_fInfo.isDir()) {
        if (cleanPath.length() == 1) {  // root path `/`
            m_absPath = m_fInfo.absoluteFilePath();
        } else {
            m_absPath = m_fInfo.absoluteFilePath() + "/";
            m_fInfo.setFile(cleanPath + "/");
        }
    } else {
        m_absPath = m_fInfo.absoluteFilePath();
    }
    return true;
}

