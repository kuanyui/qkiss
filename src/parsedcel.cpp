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

#include "parsedcel.h"
#include "logging.h"
#include <QDebug>
using namespace Qt::Literals::StringLiterals;

ParsedCel::ParsedCel(CPString fileName, const QByteArray &rawData):
    m_fileName(fileName)
{
    const static QByteArray identifier = QByteArray::fromRawData("\x4B\x69\x53\x53", 4);
    if (rawData.startsWith(identifier)) {
        parseModernCel(rawData);
    } else {
        parseLegacyCel(rawData);
    }
    qkDebug << "[DEBUG][ParsedCel] ctor: " << *this;
}

ParsedCel::~ParsedCel()
{

    if (m_cachedPaletteBasedPixels) {    // NOTE: I hear `delete nullptr` in C++ actually do nothing, so this if condition is actually unnecessary... orz
        delete m_cachedPaletteBasedPixels;
    }
    if (m_cachedColorBasedPixels) {
        delete m_cachedColorBasedPixels;
    }
}

bool ParsedCel::isValid() const
{
    return m_isValid;
}

std::unique_ptr<QVector<QRgb>> ParsedCel::getColorizedPaletteBasedCelPixels(ParsedKcf &kcf, uint16_t paletteIdxInKcf) const
{
    ParsedKcfPalette *paletteInKcf = kcf.getPaletteInKcf(paletteIdxInKcf);
    std::unique_ptr<QVector<QRgb>> colorizedPixels = std::make_unique<QVector<QRgb>>();
    // If the KCF has no valid palette, `paletteInKcf` would be nullptr, this means every pixel is transparent
    if (paletteInKcf == nullptr) {
        colorizedPixels->fill(qRgba(0, 0, 0, 0), m_cachedPaletteBasedPixels->length());
        return colorizedPixels;
    }
    colorizedPixels->reserve(m_cachedPaletteBasedPixels->length());
    for (uint32_t i = 0; i < m_cachedPaletteBasedPixels->length(); i++) {
        uint8_t colorNo = m_cachedPaletteBasedPixels->at(i);
        // colorNo == 0 means this pixel is transparent
        //   - colorNo 0 is never been used to draw CEL. For CEL, colorNo 0 always means transparent.
        //   - colorNo 0 is reserved in each paletteInKcf.  (paletteInKcf[0])
        //   - In a CNF, the paletteInKcf[0] in the first KCF file defined in CNF represents the "play field color" of each paletteInKcf.
        if (colorNo == 0) {  // means transparent
            colorizedPixels->append(qRgba(0, 0, 0, 0));
        } else {
            // if (colorNo >= kcf.colorAmountsInOnePalette()) {
            //     // NOTE: This is a dirty tolerance... Encountered a strange KiSS file which KCF declare it has 255 colors (?) and CEL requires colorNo=255...
            //     colorNo = kcf.colorAmountsInOnePalette() - 1;
            // }
            if (colorNo >= paletteInKcf->length()) {
                // For memory safe... Some malformed KiSS files containing out-of-range pixelPaletteColorNo
                colorNo = paletteInKcf->length() - 1;
            }
            colorizedPixels->append(paletteInKcf->at(colorNo));
            // if (true) {
            //     qkDebug << "[VERBOSE] colorNo of pixel = " << colorNo << ", real color of pixel = " << paletteInKcf->at(colorNo);
            // }
        }
    }
    return colorizedPixels;
}

QVector<QRgb> *ParsedCel::getColorBasedPixels() const
{
    return m_cachedColorBasedPixels;
}

void ParsedCel::parseModernCel(const QByteArray &data)
{
    if (!isLongEnough(data, 32)) { return; }   // The header is 32 bytes.
    const unsigned char* udata = reinterpret_cast<const unsigned char*>(data.constData());
    switch (data[4]) {
    case 0x20: m_celType = Type::PaletteBased; break;
    case 0x21: m_celType = Type::FullColor; break;
    default: qkCritical << "Not a valid CEL file mark!"; return;
    }
    m_bitsPerPixel = udata[5];
    if (m_bitsPerPixel != 4 && m_bitsPerPixel != 8 && m_bitsPerPixel != 32) {
        qkCritical.noquote() << tr("Invalid m_bitsPerPixel (4 or 8 or 32): %1").arg(m_bitsPerPixel); return;
    }
    if (m_celType == Type::FullColor && m_bitsPerPixel != 32) {
        qkWarning.noquote() << tr(R"("%1" claims it is a palette-based CEL file, but it said each pixel is %2 bits (expected: 32 bits), this is contradicted. Assume it is a palette-based CEL.)")
                                .arg(m_fileName.original())
                                .arg(QString::number(m_bitsPerPixel));
        m_celType = Type::PaletteBased;
    }
    if (m_celType == Type::PaletteBased && m_bitsPerPixel == 32) {
        qkCritical.noquote() << tr(R"("%1" is a palette-based CEL file, but it says each pixel is 32 bits (expected: 4 or 8 bits).)").arg(m_fileName.original()); return;
    }
    m_width = (static_cast<uint16_t>(udata[9]) << 8) | udata[8];
    m_height = (static_cast<uint16_t>(udata[11]) << 8) | udata[10];
    m_x = (static_cast<uint16_t>(udata[13]) << 8) | udata[12];
    m_y = (static_cast<uint16_t>(udata[15]) << 8) | udata[14];
    // Rounded up to whole bytes: with 4 bits per pixel and an odd width, each row ends with half a byte of padding.
    qsizetype bytesPerRow = (qsizetype(m_width) * m_bitsPerPixel + 7) / 8;
    if (!isLongEnough(data, 32 + bytesPerRow * m_height)) { return; }
    QByteArray pixelRawData = data.mid(32);  // OPTIMIZE: Don't use mid()
    switch (m_bitsPerPixel) {
    case 4: m_cachedPaletteBasedPixels = parse4BitsPixelsAsCache(&pixelRawData); break;
    case 8: m_cachedPaletteBasedPixels = new QVector<uint8_t>(pixelRawData.begin(), pixelRawData.end()); break;
    case 32: m_cachedColorBasedPixels = parse32BitsPixelsAsCache(&pixelRawData); break;
    }
    m_isValid = true;
}

void ParsedCel::parseLegacyCel(const QByteArray &data)
{
    if (!isLongEnough(data, 4)) { return; }   // The header is 4 bytes.
    const unsigned char* udata = reinterpret_cast<const unsigned char*>(data.constData());
    m_celType = Type::PaletteBased;
    m_bitsPerPixel = 4;
    m_x = 0;
    m_y = 0;
    m_width = (static_cast<uint16_t>(udata[1]) << 8) | udata[0];
    m_height = (static_cast<uint16_t>(udata[3]) << 8) | udata[2];
    qsizetype bytesPerRow = (qsizetype(m_width) * m_bitsPerPixel + 7) / 8;
    if (!isLongEnough(data, 4 + bytesPerRow * m_height)) { return; }
    QByteArray pixelRawData = data.mid(4);  // OPTIMIZE: Don't use mid()
    m_cachedPaletteBasedPixels = parse4BitsPixelsAsCache(&pixelRawData);
    m_isValid = true;
}

bool ParsedCel::isLongEnough(const QByteArray &data, qsizetype requiredBytes) const
{
    if (data.size() >= requiredBytes) { return true; }
    qkCritical.noquote() << tr(R"("%1" is truncated: it needs at least %2 bytes, but it has only %3 bytes.)")
                            .arg(m_fileName.original())
                            .arg(requiredBytes)
                            .arg(data.size());
    return false;
}

uint16_t ParsedCel::height() const
{
    return m_height;
}

QPoint ParsedCel::pos() const
{
    return QPoint(m_x, m_y);
}

QSize ParsedCel::size() const
{
    return QSize(m_width, m_height);
}

uint16_t ParsedCel::width() const
{
    return m_width;
}

uint16_t ParsedCel::y() const
{
    return m_y;
}

uint16_t ParsedCel::x() const
{
    return m_x;
}

ParsedCel::Type ParsedCel::celType() const
{
    return m_celType;
}

uint8_t ParsedCel::bitsPerPixel() const
{
    return m_bitsPerPixel;
}

QVector<uint8_t> *ParsedCel::parse4BitsPixelsAsCache(QByteArray *pixelRawData)
{
    bool widthIsOdd = (m_width % 2 == 1);
    QVector<uint8_t> *vec = new QVector<uint8_t>();
    vec->reserve(pixelRawData->length() * 2);
    if (widthIsOdd) {
        // ｘ方向幅が奇数のときは最後に１ピクセル分、０が追加されます。
        size_t maxHoriByteIdx = (m_width - 1) / 2;
        size_t byteIdx = 0;
        for (size_t y = 0; y < m_height; y++) {
            for (size_t bx = 0; bx <= maxHoriByteIdx; bx++) {   // This x is the index of byte, not pixel.
                uint8_t byte = static_cast<uint8_t>(pixelRawData->at(byteIdx++));
                vec->append(byte >> 4);
                if (bx == maxHoriByteIdx) { continue; }
                vec->append(byte & 0b00001111);
            }
        }
    } else {
       for (qsizetype i = 0; i < pixelRawData->length(); i++) {
            uint8_t x = static_cast<uint8_t>(pixelRawData->at(i));
            vec->append(x >> 4);
            vec->append(x & 0b00001111);
        }
    }
    return vec;
}

QVector<QRgb> *ParsedCel::parse32BitsPixelsAsCache(QByteArray *pixelRawData)
{
    qsizetype pxCount = qsizetype(m_width) * m_height;
    QVector<QRgb> *vec = new QVector<QRgb>();
    vec->reserve(pxCount);
    for (size_t y = 0; y < m_height; y++) {
        for (size_t x = 0; x < m_width; x++) {
            qsizetype byteOffset = ((y * m_width) + x) * 4;
            uint8_t b = static_cast<uint8_t>(pixelRawData->at(byteOffset + 0));
            uint8_t g = static_cast<uint8_t>(pixelRawData->at(byteOffset + 1));
            uint8_t r = static_cast<uint8_t>(pixelRawData->at(byteOffset + 2));
            uint8_t a = static_cast<uint8_t>(pixelRawData->at(byteOffset + 3));
            QRgb color = qRgba(r,g,b,a);
            vec->append(color);
        }
    }
    return vec;
}

CPString ParsedCel::fileName() const
{
    return m_fileName;
}

QRect ParsedCel::getGeometry() const
{
    return QRect(m_x, m_y, m_width, m_height);
}


QDebug operator<<(QDebug debug, const ParsedCel &cel) {
    return debug.noquote().nospace() << QStringLiteral("ParsedCel { type: %1 (%2 bits per pixel), \tfileName: \"%3\", \tpos: (%4, %5), \tsize(%6 x %7)")
                                        .arg(cel.celType() == ParsedCel::Type::PaletteBased ? QStringLiteral("PaletteBased") : QStringLiteral("FullColor"))
                                        .arg(cel.bitsPerPixel())
                                        .arg(cel.fileName().original())
                                        .arg(cel.x())
                                        .arg(cel.y())
                                        .arg(cel.width())
                                        .arg(cel.height())
                                        ;

}
