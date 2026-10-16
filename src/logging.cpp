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

#include "logging.h"

/**
 * The second argument should be namespace in convension. (e.g. `myapp.core`, `myapp.ui`...etc)
 *
 * This value is accessible from `QMessageLogContext.category` via `qInstallMessageHandler`
 */
Q_LOGGING_CATEGORY(qlcGeneral, "qkiss")
Q_LOGGING_CATEGORY(qlcGeneralSuccess, "qkiss.success")

ExtendedLogType formalizeLogType(QtMsgType type, const char *category) {
    switch (type) {
    case QtMsgType::QtDebugMsg: return ExtendedLogType::Debug;
    case QtMsgType::QtInfoMsg:
        if (cstrEndsWith(category, ".success")) {
            return ExtendedLogType::Success;
        }
        return ExtendedLogType::Info;
    case QtMsgType::QtWarningMsg: return ExtendedLogType::Warning;
    case QtMsgType::QtCriticalMsg: return ExtendedLogType::Critical;
    case QtMsgType::QtFatalMsg: return ExtendedLogType::Fatal;
    default: return ExtendedLogType::Debug;
    }
}
