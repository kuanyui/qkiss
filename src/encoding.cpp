#include "encoding.h"
#include <QDebug>
namespace icu {
#include <unicode/ucnv.h>
}

// Forked from https://note.com/rakko_v30/n/ncbe33ecaea13
std::string Encoding::ShiftJisToUtf8(const std::string& input) {
    icu::UErrorCode error_code = icu::U_ZERO_ERROR;
    icu::UConverter* sjis_converter = ucnv_open("Shift-JIS", &error_code);
    if (U_FAILURE(error_code)) {
        qDebug() << "Failed to create Shift-JIS converter" << "\n";
        return "";
    }

    int32_t utf8_len = ucnv_toAlgorithmic(icu::UCNV_UTF8, sjis_converter, nullptr, 0, input.c_str(), input.length(), &error_code);
    if (error_code != icu::U_BUFFER_OVERFLOW_ERROR) {
        qDebug() << "Failed to estimate buffer size for UTF-8 conversion" << "\n";
        ucnv_close(sjis_converter);
        return "";
    }

    error_code = icu::U_ZERO_ERROR;
    std::string utf8_str(utf8_len, '\0');
    ucnv_toAlgorithmic(icu::UCNV_UTF8, sjis_converter, &utf8_str[0], utf8_len, input.c_str(), input.length(), &error_code);

    ucnv_close(sjis_converter);

    if (U_FAILURE(error_code)) {
        qDebug() << "Failed to convert Shift-JIS to UTF-8" << "\n";
        return "";
    }

    return utf8_str;
}


QString Encoding::ShiftJisToUtf8(const QString& input) {
    return QString::fromStdString(ShiftJisToUtf8(input.toStdString()));
}
