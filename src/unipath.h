/**************************************************************************
* OOM Sentinel - A simple GUI tool to avoid out-of-memory (OOM) on Linux desktop
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
#ifndef UNIPATH_H
#define UNIPATH_H

#include <QString>
#include <QFileInfo>

/** Avoid ambiguiety of directory / file path.
 *
 * That is to say, always add slash suffix to prevent ambigious meaning of string.
 *
 * NOTE: The following test results are tested under Linux.
 *
 * @code{.cpp}
 * QFileInfo a ("/usr/share");
 * QFileInfo b ("/usr/share/");
 *
 * qDebug() << "a" << a.isDir() << a.absolutePath();      // a  true  "/usr"
 * qDebug() << "b" << b.isDir() << b.absolutePath();      // b  true  "/usr/share"
 *
 * qDebug() << "a" << a.isDir() << a.absoluteFilePath();  // a  true  "/usr/share"
 * qDebug() << "b" << b.isDir() << b.absoluteFilePath();  // b  true  "/usr/share/"
 * @endcode
 *
 * Therefore, NEVER relys on `QFileInfo` to store original path. To preserve path, use QString instead.
 *
 * ATTENTION: This class is designed for an existed file / folder. A non-existed path will treated as empty path.
 */
class UniPath
{
public:
    UniPath(const QString &path);
    QString absolutePath() const;
    QString absolutePathNative() const;
    bool exists() const;
    bool isDir() const;
    bool isFile() const;
    /**
     * Ensure a dir path always ends with a slash `/` (`/` even if on Windows), while fild path should never ends with a slash.
     * If the path is not existed, returns an empty string.
     */
    static QString formalizePath(const QString& dirOrFilePath);

private:
    bool init(const QString &path);
    /// If it's a folder, always ends with slash.
    QString m_absPath;
    QFileInfo m_fInfo;
};

#endif // UNIPATH_H
