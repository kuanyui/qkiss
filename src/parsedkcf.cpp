#include "parsedkcf.h"
#include <QDebug>

ParsedKcf::ParsedKcf(QString fileName, QByteArray rawData):
    m_fileName(fileName)
{
    const static QByteArray identifier = QByteArray::fromRawData("\x4B\x69\x53\x53", 4);
    if (rawData.startsWith(identifier)) {
        parseModernKcf(rawData);
    } else {
        parseLegacyKcf(rawData);
    }
}

void ParsedKcf::parseModernKcf(QByteArray data)
{
    const unsigned char* udata = reinterpret_cast<const unsigned char*>(data.constData());
    qDebug() << "[LOG] Parse Moden KCF file";
    if (data[4] != 0x10) {
        qCritical() << "[ERROR] Not a palette file mark!"; return;
    }
    m_bitsPerColor = udata[5];
    if (m_bitsPerColor != 12 && m_bitsPerColor != 24) {
        qCritical() << "[ERROR] Invalid bitsPerColor (12 or 24): " << m_bitsPerColor; return;
    }
    m_colorAmountsInOnePalette = (static_cast<uint16_t>(udata[9]) << 8) | udata[8];
    if (m_colorAmountsInOnePalette != 16 && m_colorAmountsInOnePalette != 256) {
        qCritical() << "[ERROR] Invalid colorAmountsInOnePalette (16 or 256): " << m_colorAmountsInOnePalette; return;
    }
    m_paletteAmounts = (static_cast<uint16_t>(udata[11]) << 8) | udata[10];
    if (m_bitsPerColor == 12) {
        for (uint32_t pltIdx = 0; pltIdx < m_paletteAmounts; pltIdx++) {
            QVector<QRgb> palette;
            palette.reserve(m_colorAmountsInOnePalette);
            for (uint32_t colorIdx = 0; colorIdx < m_colorAmountsInOnePalette; colorIdx++) {
                uint32_t i = 32 + (pltIdx * m_colorAmountsInOnePalette * 2) + colorIdx * 2;  // 2 bytes per color
                if (i >= data.size()) {
                    qCritical() << "[ERROR] Out of range palatte colors definition, byte offset:" << i; return;
                }
                uint8_t r = udata[i] >> 4;
                uint8_t b = udata[i] & 0b00001111;
                uint8_t g = udata[i+1];
                QRgb color = qRgb(r<<4, g<<4, b<<4);
                // qDebug() << "[12 bits]" << color;
                palette.append(color);
            }
            m_palettesList.append(palette);
        }
    } else if (m_bitsPerColor == 24) {
        for (uint32_t pltIdx = 0; pltIdx < m_paletteAmounts; pltIdx++) {
            QVector<QRgb> palette;
            palette.reserve(m_colorAmountsInOnePalette);
            for (uint32_t colorIdx = 0; colorIdx < m_colorAmountsInOnePalette; colorIdx++) {
                uint32_t i = 32 + (pltIdx * m_colorAmountsInOnePalette * 3) + colorIdx * 3;  // 3 bytes per color
                if (i >= data.size()) {
                    qCritical() << "[ERROR] Out of range palatte colors definition, byte offset:" << i; return;
                }
                uint8_t r = udata[i];
                uint8_t g = udata[i+1];
                uint8_t b = udata[i+2];
                QRgb color = qRgb(r, g, b);
                // qDebug() << "[24 bits]" << color;
                palette.append(color);
            }
            m_palettesList.append(palette);
        }
    }
    qDebug() << QStringLiteral("[LOG][ParsedKcf] (Modern KCF, %1 bits per color)     %2     %3 palettes    %4 colors per palette")
                .arg(m_bitsPerColor)
                .arg(m_fileName)
                .arg(m_paletteAmounts)
                .arg(m_colorAmountsInOnePalette);
}

void ParsedKcf::parseLegacyKcf(QByteArray data)
{
    const unsigned char* udata = reinterpret_cast<const unsigned char*>(data.constData());
    qDebug() << QString("[LOG][ParsedKcf] Parse Legacy KCF file: %1").arg(m_fileName);
    m_bitsPerColor = 12;
    m_colorAmountsInOnePalette = 16;
    m_paletteAmounts = 10;
    for (uint32_t pltIdx = 0; pltIdx < m_paletteAmounts; pltIdx++) {
        QVector<QRgb> palette;
        palette.reserve(m_colorAmountsInOnePalette);
        for (uint32_t colorIdx = 0; colorIdx < m_colorAmountsInOnePalette; colorIdx++) {
            uint32_t i = (pltIdx * m_colorAmountsInOnePalette * 2) + colorIdx * 2;  // 2 bytes per color
            if (i >= data.size()) {
                qCritical() << "[ERROR] Out of range palatte colors definition, byte offset:" << i; return;
            }
            uint8_t r = udata[i] >> 4;
            uint8_t b = udata[i] & 0b1111;
            uint8_t g = udata[i+1];
            QRgb color = qRgb(r<<4, g<<4, b<<4);
            palette.append(color);
        }
        m_palettesList.append(palette);
    }
}

uint8_t ParsedKcf::bitsPerColor() const
{
    return m_bitsPerColor;
}

ParsedKcfPalette *ParsedKcf::getPaletteInKcf(uint16_t paletteIdxInKcf)
{
    if (m_palettesList.length() == 0) {
        qCritical() << "[ERROR] This KCF file contains no valid palette, but you try to get palette from it. Abort.";
        return nullptr;
    }
    if (paletteIdxInKcf >= m_paletteAmounts) {
        qCritical() << "[ERROR] paletteIdx out of actual paletteAmount, use index 0 as fallback";
        paletteIdxInKcf = 0;
    }
    return &m_palettesList[paletteIdxInKcf];
}
