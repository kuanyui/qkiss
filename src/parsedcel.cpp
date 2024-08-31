#include "parsedcel.h"
#include <QDebug>

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

void ParsedCel::parseModernCel(QByteArray data)
{
    qDebug() << "[LOG] Parse Moden CEL file";
    switch (data[4]) {
    case 0x20: m_celType = Type::PaletteBased; break;
    case 0x21: m_celType = Type::ColorBased; break;
    default: qCritical() << "[ERROR] Not a valid CEL file mark!"; return;
    }
    m_bitsPerPixel = data[5];
    if (m_bitsPerPixel != 4 && m_bitsPerPixel != 8 && m_bitsPerPixel != 32) {
        qCritical() << "[ERROR] Invalid m_bitsPerPixel (4 or 8 or 32): " << m_bitsPerPixel; return;
    }
    m_width = (static_cast<uint16_t>(data[9]) << 8) | data[8];
    m_height = (static_cast<uint16_t>(data[11]) << 8) | data[10];
    m_x = (static_cast<uint16_t>(data[13]) << 8) | data[12];
    m_y = (static_cast<uint16_t>(data[15]) << 8) | data[14];
    m_pixelRawData = data.mid(32);
}

void ParsedCel::parseLegacyCel(QByteArray data)
{
    qDebug() << "[LOG] Parse Legacy CEL file";
    m_width = (static_cast<uint16_t>(data[1]) << 8) | data[0];
    m_height = (static_cast<uint16_t>(data[3]) << 8) | data[2];
    m_pixelRawData = data.mid(4);
}

QVector<uint8_t> ParsedCel::getAs4BitsPixels()
{
    QVector<uint8_t> vec;
    vec.reserve(m_width * m_height);
    for (uint8_t x: m_pixelRawData) {
        vec.append(x >> 4);
        vec.append(x & 0b00001111);
    }
    return vec;
}

QVector<QRgb> ParsedCel::getAs4BitsPixels(const ParsedKcf &kcf, int paletteIdx)
{
    const QVector<QRgb> &palette = kcf.getPalette(paletteIdx);
    QVector<QRgb> colorizedPixels;
    colorizedPixels.reserve(m_pixelRawData.length() * 2);
    for (uint16_t x: m_pixelRawData) {
        uint8_t colorNo0 = x >> 4;  // 0 means transparent, which is not included in KCF definition.
        uint8_t colorNo1 = x & 0b0001111;
        // (palette-based) a color in a palette is indexed from 1. if a pixel of a CEL is 0, it's transparent.
        if (colorNo0 == 0) {
            colorizedPixels.append(qRgba(0, 0, 0, 0));
        } else {
            colorizedPixels.append(palette[colorNo0 - 1]);
        }
        if (colorNo1 == 0) {
            colorizedPixels.append(qRgba(0, 0, 0, 0));
        } else {
            colorizedPixels.append(palette[colorNo1 - 1]);
        }
    }
    return colorizedPixels;
}

QVector<uint8_t> ParsedCel::getAs8BitsPixels()
{
    QVector<uint8_t> vec (m_pixelRawData.begin(), m_pixelRawData.end());
    return vec;
}

QVector<QRgb> ParsedCel::getAs8BitsPixels(const ParsedKcf &kcf, int paletteIdx)
{
    const QVector<QRgb> &palette = kcf.getPalette(paletteIdx);
    QVector<QRgb> colorizedPixels;
    colorizedPixels.reserve(m_pixelRawData.length());
    for (uint16_t colorNo: m_pixelRawData) {
        // (palette-based) a color in a palette is indexed from 1. if a pixel of a CEL is 0, it's transparent.
        if (colorNo == 0) {  // means transparent
            colorizedPixels.append(qRgba(0, 0, 0, 0));
        } else {
            colorizedPixels.append(palette[colorNo - 1]);
        }
    }
    return colorizedPixels;
}

QVector<QRgb> ParsedCel::getAs32BitsPixels()
{
    uint32_t pxCount = m_width * m_height;
    QVector<QRgb> vec;
    vec.reserve(pxCount);
    for (uint32_t i = 0; i < pxCount; i++) {
        uint32_t offset = i * 4;
        uint32_t b = m_pixelRawData[offset+0];
        uint32_t g = m_pixelRawData[offset+1];
        uint32_t r = m_pixelRawData[offset+2];
        uint32_t a = m_pixelRawData[offset+3];
        QRgb color =
                (a << 24) |
                (r << 16) |
                (g << 8) |
                (b << 0);
        vec.append(color);
    }
    return vec;
}

QRect ParsedCel::getGeometry() const
{
    return QRect(m_x, m_y, m_width, m_height);
}

