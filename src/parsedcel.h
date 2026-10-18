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

#ifndef PARSEDCEL_H
#define PARSEDCEL_H

#include <QString>
#include <QList>
#include <QVector>
#include <QColor>
#include <QRect>
#include <QDebug>
#include <QCoreApplication>
#include "parsedkcf.h"
#include "casepreservedstring.h"

/**
 * Represent a parsed .CEL file
 *
 * https://www2s.biglobe.ne.jp/~yav/kiss/kissgsj.html
 * https://otakuworld.com/kiss/kisshelp/DIRECTKISSKISS_General_Specification_CNF.htm
 */
class ParsedCel
{
    Q_DECLARE_TR_FUNCTIONS(ParsedCel)
public:
    enum class Type {
        PaletteBased,
        FullColor
    };
    ParsedCel(CPString fileName, const QByteArray &rawData);
    ~ParsedCel();
    /** False when the file is truncated or has an invalid header. Do not use an invalid CEL. */
    bool isValid() const;
    std::unique_ptr<QVector<QRgb> > getColorizedPaletteBasedCelPixels(ParsedKcf &kcf, uint16_t paletteIdxInKcf) const;  // REFACTOR: const reference
    QVector<QRgb> *getColorBasedPixels() const;
    QRect getGeometry() const;
    /** 4 / 8 / 32 */
    uint8_t bitsPerPixel() const;
    Type celType() const;
    uint16_t x() const;
    uint16_t y() const;
    uint16_t width() const;
    uint16_t height() const;
    QPoint pos() const;
    QSize size() const;
    CPString fileName() const;

private:
    void parseModernCel(const QByteArray &data);
    void parseLegacyCel(const QByteArray &data);
    /** Logs an error and returns false when `data` is shorter than `requiredBytes`. */
    bool isLongEnough(const QByteArray &data, qsizetype requiredBytes) const;
    /** Palette Based, NOT colorized yet. (4 bits) */
    QVector<uint8_t> *parse4BitsPixelsAsCache(QByteArray *pixelRawData);  // REFACTOR: Use reference instead?
    /** 32 bits full-color (rgba) */
    QVector<QRgb> *parse32BitsPixelsAsCache(QByteArray *pixelRawData);   // REFACTOR: Use reference instead?

    CPString m_fileName;
    /**
     * +4 offset, 1 byte
     * - Cel file mark (the type of cel):
     * - 20h = Palette-based cel, colors come from a palette.
     * - 21h = Color-based cel, each pixel in the cel is a 32-bit color with alpha (transparency) control.
     */
    Type m_celType = Type::PaletteBased;
    /**
     * - 4: palette-based image (16 colors per pixel in a palette)
     * - 8: palette-based image (256 colors per pixel in a palette)
     * - 32: color-based image (16,777,216 colors per pixel, no palette)
     */
    uint8_t m_bitsPerPixel = 0;
    uint16_t m_x = 0;
    uint16_t m_y = 0;
    uint16_t m_width = 0;
    uint16_t m_height = 0;
    /*
    ◎１ピクセル当たり 4 ビット（16色）の場合のピクセル・データの構成
    With 4 bits per pixel, each byte in a cel row contains 2 pixels. In the situation where the number of pixels is odd, the remaining 4 bits are set to 0.

    ｘ方向１ライン分

    |<-  バイト ->|  |<-  バイト ->|  |<-  バイト ->|
    MSB         LSB  MSB         LSB  MSB         LSB

    | pix0 | pix1 |  | pix2 | pix3 |  | pix4 | pix5 |  ......... | pixN |

    ｘ方向幅が奇数のときは最後に１ピクセル分、０が追加されます。

    これが全体でｙ方向幅分、繰り返されます。

    -----------------------------------
    ◎１ピクセル当たり 8 ビット（256色）の場合のピクセル・データの構成
    With 8 bits per pixel, each byte in a cel row contains 1 pixel.

    ｘ方向１ライン分

    |<-  バイト ->|  |<-  バイト ->|  |<-  バイト ->|
    MSB         LSB  MSB         LSB  MSB         LSB

    |     pix0    |  |     pix1    |  |     pix2    | ...  |     pixN    |

    これが全体でｙ方向幅分、繰り返されます。

    ------------------------------------
    With 32 bits per pixel, each pixel in a cel row uses 4 bytes.
    - Byte 0: Blue
    - Byte 1: Green
    - Byte 2: Red
    - Byte 3: Alpha (0 means completely transparent while an "alpha" of 255 means completely opaque.
)
    ------------------------------------
    先頭 4 バイトが 'KiSS' で無かった場合は、

    +0	W(L,H)	ｘ方向の大きさ
    +2	W(L,H)	ｙ方向の大きさ

    +4～	ピクセル・データ

    で ４ビット／ピクセルとして扱います。
    また ｘ、ｙ方向の表示位置オフセットは 0 とします。
    要するに、旧フォーマットです。
     */

    QVector<uint8_t> *m_cachedPaletteBasedPixels = nullptr;
    QVector<QRgb> *m_cachedColorBasedPixels = nullptr;
    /** Becomes true only after the header and all the pixels are parsed. */
    bool m_isValid = false;
};

QDebug operator<<(QDebug debug, const ParsedCel &cel);


#endif // PARSEDCEL_H
