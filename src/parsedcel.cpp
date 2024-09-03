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

std::unique_ptr<QVector<QRgb>> ParsedCel::getColorizedPaletteBasedPixels(ParsedKcf &kcf, int paletteIdx) const
{
    QVector<QRgb> &palette = *kcf.getPaletteInKcf(paletteIdx);
    std::unique_ptr<QVector<QRgb>> colorizedPixels = std::make_unique<QVector<QRgb>>(m_cachedPaletteBasedPixels->length());
    for (uint16_t colorNo: *m_cachedPaletteBasedPixels) {
        // (palette-based) a color in a palette is indexed from 1. if a pixel of a CEL is 0, it's transparent.
        if (colorNo == 0) {  // means transparent
            colorizedPixels->append(qRgba(0, 0, 0, 0));
        } else {
            colorizedPixels->append(palette[colorNo - 1]);
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
    QByteArray pixelRawData = data.mid(32);
    switch (m_bitsPerPixel) {
    case 4: m_cachedPaletteBasedPixels = parse4BitsPixelsAsCache(&pixelRawData); break;
    case 8: m_cachedPaletteBasedPixels = new QVector<uint8_t>(pixelRawData.begin(), pixelRawData.end()); break;
    case 32: m_cachedColorBasedPixels = parse32BitsPixelsAsCache(&pixelRawData); break;
    }
    qDebug() << QStringLiteral("[LOG][ParsedCel] (%1, %2 bits)     %3    pos(%4, %5)     size(%6, %7)")
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
    m_bitsPerPixel = 4;
    m_width = (static_cast<uint16_t>(data[1]) << 8) | data[0];
    m_height = (static_cast<uint16_t>(data[3]) << 8) | data[2];
    QByteArray pixelRawData = data.mid(4);
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
    QVector<uint8_t> *vec = new QVector<uint8_t>;
    vec->reserve(pixelRawData->length() * 2);
    for (uint8_t x: *pixelRawData) {
        vec->append(x >> 4);
        vec->append(x & 0b00001111);
    }
    return vec;
}

QVector<QRgb> *ParsedCel::parse32BitsPixelsAsCache(QByteArray *pixelRawData)
{
    qsizetype pxCount = pixelRawData->length() / 4;
    QVector<QRgb> *vec = new QVector<QRgb>(pxCount);
    for (uint32_t i = 0; i < pxCount; i++) {
        uint32_t offset = i * 4;
        uint32_t b = (*pixelRawData)[offset+0];
        uint32_t g = (*pixelRawData)[offset+1];
        uint32_t r = (*pixelRawData)[offset+2];
        uint32_t a = (*pixelRawData)[offset+3];
        QRgb color =
                (a << 24) |
                (r << 16) |
                (g << 8) |
                (b << 0);
        vec->append(color);
    }
    return vec;
}

QRect ParsedCel::getGeometry() const
{
    return QRect(m_x, m_y, m_width, m_height);
}

