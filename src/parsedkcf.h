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

#ifndef PARSEDKCF_H
#define PARSEDKCF_H

#include <QString>
#include <QList>
#include <QVector>
#include <QColor>
#include <QCoreApplication>
#include "casepreservedstring.h"

using ParsedKcfPalette = QVector<QRgb>;

/**
 * Represent a parsed .KCF file
 *
 * https://www2s.biglobe.ne.jp/~yav/kiss/kissgsj.html
 * https://otakuworld.com/kiss/kisshelp/DIRECTKISSKISS_General_Specification_KCF.htm
 *
Modern KCF Header block
=======================

Offset	Size	Contents
+0		4		Identifier 'KiSS' ( 4Bh 69h 53h 53h )
+4		1		Palette file mark ( 10h )
+5		1		bits per color ( 12 or 24 )
+6		2		Reserved. Must be 0.
+8		2		Number of colors in one palette (16 or 256). (+8 = low byte, +9 = high byte)
+10		2		Number of palettes (1...65535). (+10 = low byte, +11 = high byte)
+12		2		Reserved. Must be 0.
+14		2		Reserved. Must be 0.
+16		16		Reserved. Must be all 0's.
+32		...		Palette data

Old-style KCF Format
====================
The original KCF format was a binary file containing up to 10 palettes. Each palette contained 16 colors and each color was made up of 12 bits. There was no header at all.
All modern KiSS Artforms use the modern KCF format but most KiSS viewers should be able to load the old format.
 */

class ParsedKcf
{
    Q_DECLARE_TR_FUNCTIONS(ParsedKcf)
public:
    enum class Format {
        /**
         *  - No header.
         *  - Max 10 palattes.
         *  - Each palette contains 16 colors
         *  - Each color is 12 bits
         */
        Legacy,
        Modern,
        InvalidKcf,
    };
    /** fileName is the relative file name in archive file. */
    ParsedKcf(CPString fileName, QByteArray rawData);

    ParsedKcfPalette *getPaletteInKcf(uint16_t paletteIdxInKcf = 0);  // REFACTOR: Use reference?
    uint8_t bitsPerColor() const;
    uint16_t paletteAmounts() const;

    uint16_t colorAmountsInOnePalette() const;

private:
    void parseModernKcf(QByteArray data);
    void parseLegacyKcf(QByteArray data);
    CPString m_fileName;
    /**
     * - 12 or 24 bits
     * - Offset +5, 1 byte
     */
    uint8_t m_bitsPerColor;
    /**
     * == [ATTENTION] ===============
     *
     * Otakuworld's KiSS spec said this:
     *
     * ```
     * - 16 or 256 colors
     * - Offset +8, 2 bytes (+8 low, +9 high)
     * ```
     *
     * But the KISS/GS spec in https://www2s.biglobe.ne.jp/~yav/kiss/kissgsj.html (元祖 fkiss) is conflicted with it:
     *
     * ```
     *  +8	W(L,H)	１パレット組内の色数 ( 1 ... 256 )
     * ```
     *
     * And the latter one seems more generic, so take this.
     */
    uint16_t m_colorAmountsInOnePalette;
    /**
     * - 1 ~ 65535
     * - Offset +10, 2 bytes (+10 low, +11 high)
     */
    uint16_t m_paletteAmounts;
    /**

    ◎１色当たり 12 ビット（4096色）の場合のパレット・データの構成

    １つのパレット・データは 2 バイト、ＲＧＢ各 4 ビットで 4096 色。

    |<-  バイト ->|  |<-  バイト ->|
    MSB         LSB  MSB         LSB

    | rrrr | bbbb |  | 0000 | gggg | ....
    １パレット組内の色数 × 全パレット組数 分並びます。


    ----------------------------------------------------
    ◎１色当たり 24 ビット（1677万色）の場合のパレット・データの構成


    １つのパレット・データは 3 バイト、ＲＧＢ各 8 ビットで 16777216 色。

    |<-  バイト ->|  |<-  バイト ->|  |<-  バイト ->|
    MSB         LSB  MSB         LSB  MSB         LSB

    |  rrrrrrrr   |  |  gggggggg   |  |  bbbbbbbb   | ...
    １パレット組内の色数 × 全パレット組数 分並びます。


    全パレット組数が 10 に満たない場合、その分は 0 番のパレット
    組のデータが入ります。

    ----------------------------------------------------
    先頭 4 バイトが 'KiSS' で無かった場合は、

    +0～	パレット・データ

    で 12 ビット／色、パレット組当たりの色数 16 、全パレット組数 10
    として扱います。要するに、旧フォーマットです。

     */
    QVector<ParsedKcfPalette> m_palettesList;
};

#endif // PARSEDKCF_H
