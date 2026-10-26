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
#include <cstdint>
#include <vector>
#include "enums.h"

/**
 * Makes an image look like a CRT monitor shows it, in one of the presets of QKiSS::CrtSimulationPreset.
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
    /// How far light that scatters in the glass of the screen reaches, in KiSS pixels. Only render() uses it, for the
    /// presets that copy CRT-Royale (Sharp and Soft). There it takes the place of the glow.
    static constexpr int DIFFUSION_RADIUS = 11;
    /// How far colors bleed to the left and right, in KiSS pixels, like on a TV with an S-Video cable. 0 turns this off.
    /// The cable carries the brightness and the color on separate wires, and the color wire carries much less detail.
    /// So a small red dot becomes a short red streak, while the edges between bright and dark stay sharp.
    static constexpr int SVIDEO_RADIUS = 2;
    /// How far one pixel can change other pixels, in KiSS pixels: SVIDEO_RADIUS for the S-Video cable, then 1 for the
    /// beam + GLOW_RADIUS for the glow, or DIFFUSION_RADIUS if that is larger, and 1 more because render() interpolates
    /// the glow (or the scattered light) between pixels. In apply(), a pixel changes only when a pixel of another color
    /// is within this distance.
    static constexpr int REACH = SVIDEO_RADIUS + qMax(1 + GLOW_RADIUS, DIFFUSION_RADIUS) + 1;
    /// Use render() from this many device pixels per KiSS pixel, and apply() below it. A scanline needs at least 2 rows
    /// of device pixels: a bright one and a darker one.
    static constexpr qreal MIN_RENDER_SCALE = 2;

    /// For a small zoom: filters `image` in place, so that neighboring pixels blend and bright areas glow. This is the
    /// same for every preset. `image` must be QImage::Format_RGB32, with one KiSS pixel per image pixel.
    static void apply(QImage &image);
    /// For a large zoom: returns `image` as a CRT shows it, at device resolution, in the preset `preset`. `image` must
    /// be QImage::Format_RGB32, with one KiSS pixel per image pixel. The result is `devicePixelsPerKissPixel` times as
    /// wide and as tall, with one device pixel per image pixel.
    /// Glow, our own look:
    /// - Each pixel is a spot of light that spreads to the left and right. A bright spot reaches into darker neighbors.
    /// - Each row of pixels is a scanline, brightest in its middle. A bright scanline is thicker than a dark one, and a
    ///   thin scanline is brighter in its middle, so the scanlines don't make the picture darker.
    /// - From 3 device pixels per KiSS pixel, there are red, green and blue phosphor stripes. Each stripe is a whole
    ///   number of device pixels wide, and the 3 colors together are at most 1 KiSS pixel wide.
    /// - Light above full brightness can't be shown, so it spreads to the pixels around it.
    /// - The glow comes last, so the scanlines and the stripes don't cover it.
    /// Sharp and Soft copy the look of CRT-Royale instead (see renderRoyale() in crtfilter.cpp).
    /// With QKiSS::CrtSimulationPreset::None, it returns `image` as it is (GKissView doesn't call it then).
    /// `topLeft` is the scene position of `image`, in KiSS pixels. It keeps the stripes (or the mask of CRT-Royale) in
    /// place when the view scrolls.
    static QImage render(const QImage &image, QPoint topLeft, qreal devicePixelsPerKissPixel,
                         QKiSS::CrtSimulationPreset preset);

    /// The kernels, tables and other numbers of apply() and render(), computed once. CrtFilterGL needs them to do the
    /// same work on the graphics card. The values that change the look are at the top of crtfilter.cpp.
    /// A kernel holds the weights of a blur, from left to right (or from top to bottom): 2 * radius + 1 weights.
    struct LookData
    {
        /// apply() and every preset: the blur of the S-Video cable, at KiSS resolution. Empty when SVIDEO_RADIUS is 0.
        std::vector<float> sVideoKernel;

        // apply() and Glow
        /// The beam blend, at KiSS resolution. Soft uses it too.
        std::vector<float> beamKernel;
        /// The glow around bright areas: its blur at KiSS resolution, and how strong it is.
        std::vector<float> glowKernel;
        float glowAmount = 0;
        /// Linear light -> sRGB value (0 ~ 255), like linearToSrgb() in crtfilter.cpp: the light is cut off to 0 ~ 1
        /// and rounded to one of srgbTable.size() even steps, which are the indexes of this table.
        std::vector<uint8_t> srgbTable;

        // Glow and Soft: the spots
        /// The sigma of a spot for black and for full brightness, in KiSS pixels.
        float spotSigmaDark = 0;
        float spotSigmaBright = 0;
        /// How much of a Gaussian with sigma 1 lies to the left of t, for t = -gaussianCdfMax ~ gaussianCdfMax in
        /// gaussianCdf.size() - 1 even steps. Between two steps, the value is interpolated.
        std::vector<float> gaussianCdf;
        float gaussianCdfMax = 0;

        // Every preset: the scanlines
        /// A table of the scanlines, for the brightness levels 0 (black) ~ scanlineLevels (full brightness). For each
        /// level: alpha, the width of the scanline in rows of KiSS pixels, and the share of its light between its
        /// middle and t * alpha rows below it, for t = 0 ~ scanlineTMax in scanlineSteps steps.
        struct ScanlineTable
        {
            std::vector<float> alpha;   // scanlineLevels + 1 values
            std::vector<float> share;   // scanlineLevels + 1 rows of scanlineSteps + 1 values
        };
        int scanlineLevels = 0;
        int scanlineSteps = 0;
        float scanlineTMax = 0;
        ScanlineTable glowScanlines;   // For Glow
        ScanlineTable royaleScanlines;   // For Sharp and Soft, as wide as in CRT-Royale

        // Glow: the phosphor stripes (see also stripeWidth() and stripeCompensation())
        /// How much light of the other two colors a stripe lets through.
        float stripeOtherLight = 0;

        // Sharp and Soft, which copy CRT-Royale
        /// The gamma of the CRT.
        float crtGamma = 0;
        /// The two blurred copies of the light, at KiSS resolution: the light around each pixel, and the light that
        /// scatters in the glass of the screen.
        std::vector<float> areaKernel;
        std::vector<float> diffusionKernel;
        /// The slot mask: a square tile of maskSize x maskSize device pixels, which repeats across the screen. For each
        /// pixel, row by row: the share of its red, green and blue light that the mask lets through (0 ~ 1).
        int maskSize = 0;
        std::vector<float> mask;
        /// How much light the mask lets through on average.
        float maskAverage = 0;
        /// The bloom, which gives back the light that the mask takes away: the share of the light that it counts, the
        /// weight of the middle pixel of its blur, and its blur, at device resolution.
        float bloomUnderestimate = 0;
        float bloomCenterWeight = 0;
        std::vector<float> bloomKernel;
        /// How much of the light scatters in the glass of the screen.
        float diffusionWeight = 0;
        /// The gamma of the screen that shows the picture now.
        float lcdGamma = 0;
    };
    /// See LookData.
    static const LookData &lookData();
    /// Where the middle of each scanline is, inside its row of KiSS pixels (0 ~ 1), at the zoom `scale` (device pixels
    /// per KiSS pixel). render() uses it for every preset. See crtfilter.cpp.
    static float scanlineMiddle(float scale);
    /// render() for Glow: how wide each phosphor stripe is at the zoom `scale`, in device pixels. 0 means no stripes.
    static int stripeWidth(float scale);
    /// render() for Glow: the stripes take away light of the other colors. Multiplying the light by this number adds it
    /// back, so that the picture keeps its brightness. `stripeWidth` comes from stripeWidth(). Without stripes, this
    /// number is 1.
    static float stripeCompensation(int stripeWidth);
    /// render() for Glow: the blur of the bloom at the zoom `scale`, at device resolution. Empty without the bloom.
    static std::vector<float> glowBloomKernel(float scale);
};

#endif // CRTFILTER_H
