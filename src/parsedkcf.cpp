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
    qDebug() << "[LOG] Parse Moden KCF file";
    if (data[4] != 0x10) {
        qCritical() << "[ERROR] Not a palette file mark!"; return;
    }
    m_bitsPerColor = data[5];
    if (m_bitsPerColor != 12 && m_bitsPerColor != 24) {
        qCritical() << "[ERROR] Invalid bitsPerColor (12 or 24): " << m_bitsPerColor; return;
    }
    m_colorAmountsInOnePalette = (static_cast<uint16_t>(data[9]) << 8) | data[8];
    if (m_colorAmountsInOnePalette != 16 && m_colorAmountsInOnePalette != 256) {
        qCritical() << "[ERROR] Invalid colorAmountsInOnePalette (16 or 256): " << m_colorAmountsInOnePalette; return;
    }
    m_paletteAmounts = (static_cast<uint16_t>(data[11]) << 8) | data[10];
    if (m_bitsPerColor == 12) {
        for (uint32_t pltIdx = 0; pltIdx < m_paletteAmounts; pltIdx++) {
            QVector<QColor> palette;
            palette.reserve(m_colorAmountsInOnePalette);
            for (uint32_t colorIdx = 0; colorIdx < m_colorAmountsInOnePalette; colorIdx++) {
                uint32_t i = 32 + (pltIdx * m_colorAmountsInOnePalette * 2) + colorIdx * 2;  // 2 bytes per color
                if (i >= data.size()) {
                    qCritical() << "[ERROR] Out of range palatte colors definition, byte offset:" << i; return;
                }
                uint8_t r = data[i] >> 4;
                uint8_t b = data[i] & 0b1111;
                uint8_t g = data[i+1];
                QColor color = QColor::fromRgb(r<<4, g<<4, b<<4);
                // qDebug() << "[12 bits]" << color;
                palette.append(color);
            }
            m_palettesList.append(palette);
        }
    } else if (m_bitsPerColor == 24) {
        for (uint32_t pltIdx = 0; pltIdx < m_paletteAmounts; pltIdx++) {
            QVector<QColor> palette;
            palette.reserve(m_colorAmountsInOnePalette);
            for (uint32_t colorIdx = 0; colorIdx < m_colorAmountsInOnePalette; colorIdx++) {
                uint32_t i = 32 + (pltIdx * m_colorAmountsInOnePalette * 3) + colorIdx * 3;  // 3 bytes per color
                if (i >= data.size()) {
                    qCritical() << "[ERROR] Out of range palatte colors definition, byte offset:" << i; return;
                }
                uint8_t r = data[i];
                uint8_t b = data[i+1];
                uint8_t g = data[i+2];
                QColor color = QColor::fromRgb(r, g, b);
                // qDebug() << "[24 bits]" << color;
                palette.append(color);
            }
            m_palettesList.append(palette);
        }
    }
}

void ParsedKcf::parseLegacyKcf(QByteArray data)
{
    qDebug() << "[LOG] Parse Legacy KCF file";
    qDebug() << "[WARNING] May wrong implementations for legacy KCF because the spec is ambigious.";
    m_bitsPerColor = 12;
    m_colorAmountsInOnePalette = 16;
    m_paletteAmounts = 10;
    for (uint32_t pltIdx = 0; pltIdx < m_paletteAmounts; pltIdx++) {
        QVector<QColor> palette;
        palette.reserve(m_colorAmountsInOnePalette);
        for (uint32_t colorIdx = 0; colorIdx < m_colorAmountsInOnePalette; colorIdx++) {
            uint32_t i = (pltIdx * m_colorAmountsInOnePalette * 2) + colorIdx * 2;  // 2 bytes per color
            if (i >= data.size()) {
                qCritical() << "[ERROR] Out of range palatte colors definition, byte offset:" << i; return;
            }
            uint8_t r = data[i] >> 4;
            uint8_t b = data[i] & 0b1111;
            uint8_t g = data[i+1];
            QColor color = QColor::fromRgb(r<<4, g<<4, b<<4);
            palette.append(color);
        }
        m_palettesList.append(palette);
    }
}
