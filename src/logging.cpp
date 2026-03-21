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
