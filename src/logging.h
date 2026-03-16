#ifndef LOGGING_H
#define LOGGING_H
#include <QLoggingCategory>
#include <QDebug>

/** `qlc` means "Qt Logging Category" */
Q_DECLARE_LOGGING_CATEGORY(qlcGeneral)

#define qkDebug     qCDebug(qlcGeneral)
#define qkInfo      qCInfo(qlcGeneral)
#define qkWarning   qCWarning(qlcGeneral)
#define qkCritical  qCCritical(qlcGeneral)
#define qkFatal     qCFatal(qlcGeneral)

#endif // LOGGING_H
