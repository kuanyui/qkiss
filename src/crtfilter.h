/**************************************************************************
* QKiSS - A modern implementation of KiSS (Kisekae) viewer in Qt 6.
*
* Copyright (C) 2026 ono ono (kuanyui) <azazabc123@gmail.com>
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

#ifndef CRTFILTER_H
#define CRTFILTER_H

#include <QImage>
#include <QPainter>

/**
 * Makes an image look like a CRT monitor shows it: neighboring pixels blend, and bright areas glow.
 *
 * Much pixel art of the 1990s was drawn for CRT monitors. For example, a checkerboard of opaque pixels stood for
 * semi-transparency, because the monitor blended the checkerboard into one color.
 *
 * The screen is not curved, so every pixel stays where it is, and the mouse still points at the right pixel.
 */
class CrtFilter
{
public:
    /// How far the glow reaches, in KiSS pixels.
    static constexpr int GLOW_RADIUS = 6;
    /// How far apply() spreads one pixel into its neighbors, in KiSS pixels: 1 for the beam, plus the glow.
    /// A pixel changes only when a pixel of another color is within this distance.
    static constexpr int REACH = 1 + GLOW_RADIUS;
    /// Filters `image` in place. It must be QImage::Format_RGB32, with one KiSS pixel per image pixel.
    static void apply(QImage &image);
    /// Darkens `rect` with red, green and blue stripes, like the phosphor stripes of a CRT.
    /// - Each stripe is a whole number of device pixels wide. The stripes get wider when you zoom in.
    /// - The 3 colors together are at most 1 KiSS pixel wide. They are exactly 1 KiSS pixel wide when a KiSS pixel is
    ///   3, 6, 9... device pixels wide.
    /// - Does nothing when a KiSS pixel is narrower than 3 device pixels, because the stripes wouldn't fit.
    /// `painter` must use scene coordinates (1 unit = 1 KiSS pixel).
    static void drawPhosphorStripes(QPainter *painter, const QRectF &rect, qreal devicePixelsPerKissPixel);
};

#endif // CRTFILTER_H
