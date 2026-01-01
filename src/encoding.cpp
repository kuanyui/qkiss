/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2024 - 2025 ono ono (kuanyui) <azazabc123@gmail.com>
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

#include "encoding.h"
#include <QDebug>


namespace {

#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
QStringList stdVectorStringToQStringList(const std::vector<std::string> &vec)
{
    QStringList result;
    result.reserve(static_cast<int>(vec.size()));
    for (const std::string &s : vec) {
        result.append(QString::fromUtf8(s));
    }
    return result;
}
#endif

/**
 * QDebug::operator<<(const std::basic_string<Char, Args...> &s) is since Qt6.5
 * QDebug::operator<<(QDebug debug, const std::vector<T, Alloc> &vec) is since Qt6.9
 */
class DebugTypeAdapter {
public:
    explicit DebugTypeAdapter(const std::vector<std::string> &v) : vec(v) { }
    const std::vector<std::string> &vec;
};

QDebug operator<<(QDebug dbg, const DebugTypeAdapter &dataAdapter)
{
    QDebugStateSaver saver(dbg);

#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
    dbg << stdVectorStringToQStringList(dataAdapter.vec);
#else
    dbg << dataAdapter.vec;
#endif
    return dbg;
}


}  // Anonymous namespace end here



#if defined(HAVE_ICU)

#define qd qDebug() << "[ICU] "

namespace icu {
#include <unicode/ucnv.h>
}

QString Encoding::toUtf8(const QByteArray &input, QKiSS::Encoding encoding)
{
    switch (encoding) {
    case QKiSS::Encoding::ASCII: return QString::fromLatin1(input);
    case QKiSS::Encoding::UTF8: return QString::fromUtf8(input);
    case QKiSS::Encoding::SJIS: return QString::fromStdString(innerToUtf8(input.toStdString(), {"Shift_JIS"}));
    case QKiSS::Encoding::EUC_JP: return QString::fromStdString(innerToUtf8(input.toStdString(), {"EUC-JP"}));
    // case QKiSS::Encoding::ISO_2022_JP: return QString::fromStdString(innerToUtf8(input.toStdString(), {"ISO-2022-JP"}));
    }
    return QString::fromLatin1(input);
}

// Forked from https://note.com/rakko_v30/n/ncbe33ecaea13
std::string Encoding::innerToUtf8(const std::string& input, const std::vector<std::string> &fromEncodings) {
    icu::UErrorCode error_code = icu::U_ZERO_ERROR;
    if (fromEncodings.size() == 0) {
        return "";
    }
    icu::UConverter* icuConverter = nullptr;
    bool icuOpened = false;
    for (const std::string &fromEnc : fromEncodings) {
        icuConverter = icu::ucnv_open(fromEnc.c_str(), &error_code);
        if (icu::U_SUCCESS(error_code)) {
            icuOpened = true;
            break;
        }
    }
    if (!icuOpened) {
        qd << "Tried to open converter in this list, but failed:" << DebugTypeAdapter(fromEncodings);
        return "";
    }
    int32_t utf8_len = icu::ucnv_toAlgorithmic(icu::UCNV_UTF8, icuConverter, nullptr, 0, input.c_str(), input.length(), &error_code);
    if (error_code != icu::U_BUFFER_OVERFLOW_ERROR) {
        qd << "Failed to estimate buffer size for UTF-8 conversion" << "\n";
        icu::ucnv_close(icuConverter);
        return "";
    }

    error_code = icu::U_ZERO_ERROR;
    std::string utf8_str(utf8_len, '\0');
    icu::ucnv_toAlgorithmic(icu::UCNV_UTF8, icuConverter, &utf8_str[0], utf8_len, input.c_str(), input.length(), &error_code);

    icu::ucnv_close(icuConverter);

    if (icu::U_FAILURE(error_code)) {
        qd << "Failed to convert Shift-JIS to UTF-8" << "\n";
        return "";
    }

    return utf8_str;
}


#elif defined(HAVE_ICONV)

#define qd qDebug() << "[iconv]"

extern "C" {
#include <iconv.h>
}

QString Encoding::toUtf8(const QByteArray &input, QKiSS::Encoding encoding)
{
    switch (encoding) {
    case QKiSS::Encoding::ASCII: return QString::fromLatin1(input);
    case QKiSS::Encoding::UTF8: return QString::fromUtf8(input);
    case QKiSS::Encoding::SJIS: return QString::fromStdString(innerToUtf8(input.toStdString(), {"SJIS", "SHIFT-JIS"}));
    case QKiSS::Encoding::EUC_JP: return QString::fromStdString(innerToUtf8(input.toStdString(), {"EUC-JP"}));
    // case QKiSS::Encoding::ISO_2022_JP: return QString::fromStdString(innerToUtf8(input.toStdString(), {"ISO-2022-JP"}));
    }
    return QString::fromLatin1(input);
}

std::string Encoding::innerToUtf8(const std::string& input, const std::vector<std::string> &fromEncodings) {
    iconv_t cd;
    bool opened = false;
    for (const std::string &fromEnc : fromEncodings) {
        cd = iconv_open("UTF-8", fromEnc.c_str());
        if (cd == (iconv_t)-1) {   // if open failed.
            continue;
        } else {
            opened = true;
            break;
        }
    }
    if (!opened) {
        qd << "iconv_open() failed. Already tried:" << DebugTypeAdapter(fromEncodings);
        return "";
    }
    // input buffer
    size_t inBytesLeft = input.size();
    char *inBuf = const_cast<char*>(input.data());   // don't use C-style cast here to avoid UB...

    // output buffer
    size_t outBufSize = input.size() * 4;   // std::string::size() returns size_t in bytes,
    size_t outBytesLeft = outBufSize;
    std::string outStr (outBytesLeft, '\0');   // init std::string filled with `\0`
    char *outBuf = outStr.data();

    // convert
    size_t result = iconv(cd, &inBuf, &inBytesLeft, &outBuf, &outBytesLeft);
    if (result == (size_t)-1) {
        qd << "conversion failed.";
        iconv_close(cd);
        return "";
    }
    outStr.resize(outBufSize - outBytesLeft);
    iconv_close(cd);
    return outStr;
}



#endif
