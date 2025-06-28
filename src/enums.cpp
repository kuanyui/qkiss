// fruit.cpp
#include "enums.h"
#include <QMetaType>

// Register enum classes, so that they can be used in QVariant<T>
static bool registered = []() {
    qRegisterMetaType<QKiSS::Case>("QKiSS::Case");
    qRegisterMetaType<QKiSS::Encoding>("QKiSS::Encoding");
    qRegisterMetaType<QKiSS::LzhDecompressorBackend>("QKiSS::LzhDecompressorBackend");
    return true;
}();
