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

#ifndef EXTRACTORSANDBOX_H
#define EXTRACTORSANDBOX_H

#include <QString>

class QProcess;

/**
 * Sandbox for the external LZH extractors (lha, unar), which parse untrusted archives.
 *
 * On Linux, the program that `process` starts runs under Landlock and seccomp. It can:
 * - Read the system folders (`/usr`, `/etc`...) and `archiveFilePath`, but nothing else: nothing in the home folder.
 * - Write the files only beneath `writableDirPath`, when it is given.
 * - Run only programs from the system folders, and `program` itself.
 * - Not create sockets or namespaces, use io_uring, read other processes' memory, or send them signals.
 * Each limit applies only when the kernel supports it. On other platforms, this does nothing.
 *g
 * Call this function before each `process->start(program, ...)`.
 */
void sandboxExtractorProcess(QProcess *process, const QString &program, const QString &archiveFilePath, const QString &writableDirPath = QString());

#endif // EXTRACTORSANDBOX_H
