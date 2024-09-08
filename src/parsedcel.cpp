#include "parsedcel.h"
#include <QDebug>
using namespace Qt::Literals::StringLiterals;

ParsedCel::ParsedCel(QString fileName, QByteArray rawData):
    m_fileName(fileName)
{
    const static QByteArray identifier = QByteArray::fromRawData("\x4B\x69\x53\x53", 4);
    if (rawData.startsWith(identifier)) {
        parseModernCel(rawData);
    } else {
        parseLegacyCel(rawData);
    }
}

std::unique_ptr<QVector<QRgb>> ParsedCel::getColorizedPaletteBasedCelPixels(ParsedKcf &kcf, int paletteIdxInKcf) const
{
    ParsedKcfPalette *paletteInKcf = kcf.getPaletteInKcf(paletteIdxInKcf);
    std::unique_ptr<QVector<QRgb>> colorizedPixels = std::make_unique<QVector<QRgb>>();
    colorizedPixels->reserve(m_cachedPaletteBasedPixels->length());
    for (uint32_t i = 0; i < m_cachedPaletteBasedPixels->length(); i++) {
        uint8_t colorNo = m_cachedPaletteBasedPixels->at(i);
        // (palette-based) a color in a palette is indexed from 1. if a pixel of a CEL is 0, it's transparent.
        if (colorNo == 0) {  // means transparent
            colorizedPixels->append(qRgba(0, 0, 0, 0));
        } else {
            colorizedPixels->append(paletteInKcf->at(colorNo));
            // if (true) {
            //     qDebug() << "[VERBOSE] colorNo of pixel = " << colorNo << ", real color of pixel = " << paletteInKcf->at(colorNo);
            // }
        }
    }
    return colorizedPixels;
}

QVector<QRgb> *ParsedCel::getColorBasedPixels() const
{
    return m_cachedColorBasedPixels;
}

void ParsedCel::parseModernCel(QByteArray data)
{
    const unsigned char* udata = reinterpret_cast<const unsigned char*>(data.constData());
    switch (data[4]) {
    case 0x20: m_celType = Type::PaletteBased; break;
    case 0x21: m_celType = Type::ColorBased; break;
    default: qCritical() << "[ERROR] Not a valid CEL file mark!"; return;
    }
    m_bitsPerPixel = udata[5];
    if (m_bitsPerPixel != 4 && m_bitsPerPixel != 8 && m_bitsPerPixel != 32) {
        qCritical() << "[ERROR] Invalid m_bitsPerPixel (4 or 8 or 32): " << m_bitsPerPixel; return;
    }
    m_width = (static_cast<uint16_t>(udata[9]) << 8) | udata[8];
    m_height = (static_cast<uint16_t>(udata[11]) << 8) | udata[10];
    m_x = (static_cast<uint16_t>(udata[13]) << 8) | udata[12];
    m_y = (static_cast<uint16_t>(udata[15]) << 8) | udata[14];
    QByteArray pixelRawData = data.mid(32);  // OPTIMIZE: Don't use mid()
    switch (m_bitsPerPixel) {
    case 4: m_cachedPaletteBasedPixels = parse4BitsPixelsAsCache(&pixelRawData); break;
    case 8: m_cachedPaletteBasedPixels = new QVector<uint8_t>(pixelRawData.begin(), pixelRawData.end()); break;
    case 32: m_cachedColorBasedPixels = parse32BitsPixelsAsCache(&pixelRawData); break;
    }
    qDebug() << QStringLiteral("[LOG][ParsedCel] (%1, %2 bits per pixel)     %3    pos(%4, %5)     size(%6, %7)")
                .arg(m_celType == Type::PaletteBased ? "Palette Based"_L1 : "Full Color"_L1)
                .arg(m_bitsPerPixel)
                .arg(m_fileName)
                .arg(m_x)
                .arg(m_y)
                .arg(width())
                .arg(height())
                ;
}

void ParsedCel::parseLegacyCel(QByteArray data)
{
    const unsigned char* udata = reinterpret_cast<const unsigned char*>(data.constData());
    m_celType = Type::PaletteBased;
    m_bitsPerPixel = 4;
    m_x = 0;
    m_y = 0;
    m_width = (static_cast<uint16_t>(udata[1]) << 8) | udata[0];
    m_height = (static_cast<uint16_t>(udata[3]) << 8) | udata[2];
    QByteArray pixelRawData = data.mid(4);  // OPTIMIZE: Don't use mid()
    m_cachedPaletteBasedPixels = parse4BitsPixelsAsCache(&pixelRawData);
}

uint16_t ParsedCel::height() const
{
    return m_height;
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
    qsizetype pxCount = m_width * m_height;
    QVector<QRgb> *vec = new QVector<QRgb>();
    vec->reserve(pxCount);
    for (size_t y = 0; y < m_height; y++) {
        for (size_t x = 0; x < m_width; x++) {
            uint32_t byteOffset = ((y * m_width) + x) * 4;
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

QRect ParsedCel::getGeometry() const
{
    return QRect(m_x, m_y, m_width, m_height);
}

