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

#ifndef LOGGING_H
#define LOGGING_H
#include <QLoggingCategory>
#include <QDebug>
#include "mycstr.h"

/** `qlc` means "Qt Logging Category" */
Q_DECLARE_LOGGING_CATEGORY(qlcGeneral)
Q_DECLARE_LOGGING_CATEGORY(qlcGeneralSuccess)

#define qkDebug     qCDebug(qlcGeneral)
#define qkSuccess   qCInfo(qlcGeneralSuccess)
#define qkInfo      qCInfo(qlcGeneral)
#define qkWarning   qCWarning(qlcGeneral)
#define qkCritical  qCCritical(qlcGeneral)
#define qkFatal     qCFatal(qlcGeneral)

/** Add missing `Success` into `QtMsgType` */
enum class ExtendedLogType {
    Debug,
    /// Added, Syslog has no this category.
    Success,
    Info,
    Warning,
    /// Error
    Critical,
    Fatal,
};

ExtendedLogType formalizeLogType (QtMsgType type, const char *category);

#endif // LOGGING_H
