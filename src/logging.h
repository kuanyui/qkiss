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
