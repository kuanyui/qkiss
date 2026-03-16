#include "logging.h"

/**
 * The second argument should be namespace in convension. (e.g. `myapp.core`, `myapp.ui`...etc)
 *
 * This value is accessible from `QMessageLogContext.category` via `qInstallMessageHandler`
 */
Q_LOGGING_CATEGORY(qlcGeneral, "qkiss")