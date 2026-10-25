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
    /// How far the glow reaches, in KiSS pixels. The other values that change the look are at the top of crtfilter.cpp.
    static constexpr int GLOW_RADIUS = 6;
    /// How far colors bleed to the left and right, in KiSS pixels, like on a TV with an S-Video cable. 0 turns this off.
    /// The cable carries the brightness and the color on separate wires, and the color wire carries much less detail.
    /// So a small red dot becomes a short red streak, while the edges between bright and dark stay sharp.
    static constexpr int SVIDEO_RADIUS = 2;
    /// How far one pixel can change other pixels, in KiSS pixels: SVIDEO_RADIUS for the S-Video cable, 1 for the beam,
    /// GLOW_RADIUS for the glow, and 1 more because render() interpolates the glow between pixels. In apply(), a pixel
    /// changes only when a pixel of another color is within this distance.
    static constexpr int REACH = SVIDEO_RADIUS + 1 + GLOW_RADIUS + 1;
    /// Use render() from this many device pixels per KiSS pixel, and apply() below it. A scanline needs at least 2 rows
    /// of device pixels: a bright one and a darker one.
    static constexpr qreal MIN_RENDER_SCALE = 2;

    /// For a small zoom: filters `image` in place, so that neighboring pixels blend and bright areas glow.
    /// `image` must be QImage::Format_RGB32, with one KiSS pixel per image pixel.
    static void apply(QImage &image);
    /// For a large zoom: returns `image` as a CRT shows it, at device resolution. `image` must be QImage::Format_RGB32,
    /// with one KiSS pixel per image pixel. The result is `devicePixelsPerKissPixel` times as wide and as tall, with one
    /// device pixel per image pixel.
    /// - Each pixel is a spot of light that spreads to the left and right. A bright spot reaches into darker neighbors.
    /// - Each row of pixels is a scanline, brightest in its middle. A bright scanline is thicker than a dark one, and a
    ///   thin scanline is brighter in its middle, so the scanlines don't make the picture darker.
    /// - From 3 device pixels per KiSS pixel, there are red, green and blue phosphor stripes. Each stripe is a whole
    ///   number of device pixels wide, and the 3 colors together are at most 1 KiSS pixel wide.
    /// - Light above full brightness can't be shown, so it spreads to the pixels around it.
    /// - The glow comes last, so the scanlines and the stripes don't cover it.
    /// `topLeft` is the scene position of `image`, in KiSS pixels. It keeps the stripes in place when the view scrolls.
    static QImage render(const QImage &image, QPoint topLeft, qreal devicePixelsPerKissPixel);
};

#endif // CRTFILTER_H
