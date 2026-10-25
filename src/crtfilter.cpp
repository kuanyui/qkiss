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

#include "crtfilter.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <span>
#include <vector>

namespace {
// ==========================================================================
// The look of the filter. Change these values to tune it.
// (GLOW_RADIUS is in crtfilter.h, because GKissView needs it too.)
// ==========================================================================
/// How much each pixel spreads into its left and right neighbors: 0 (not at all) ~ 0.25.
/// The beam of a CRT is a little wider than one pixel. With 0.25, a checkerboard of two colors becomes one even color.
constexpr float BEAM_SPREAD = 0.25f;
/// How strongly a bright area glows into its darker neighbors: 0 means no glow.
constexpr float GLOW_AMOUNT = 0.3f;
/// render() only: each pixel is a spot of light, which spreads to the left and right like a Gaussian.
/// These are its sigma for black and for full brightness, in KiSS pixels. A bright spot reaches into darker neighbors.
constexpr float SPOT_SIGMA_DARK = 0.2f;
constexpr float SPOT_SIGMA_BRIGHT = 0.5f;
/// render() only: each row of pixels is a scanline, which fades up and down like a Gaussian. These are its sigma for
/// black and for full brightness, in rows of KiSS pixels. Larger values make the dark gaps between scanlines thinner.
constexpr float SCANLINE_SIGMA_DARK = 0.25f;
constexpr float SCANLINE_SIGMA_BRIGHT = 0.38f;
/// render() only: makes the scanlines brighter, to make up for the dark gaps between them: 1 means no change.
/// Light above full brightness is cut off, so the brightest areas become flat.
constexpr float SCANLINE_BRIGHTNESS = 1.25f;
/// render() only: how much light of the other two colors a phosphor stripe lets through: 0 (none) ~ 1 (all, so no
/// stripes).
constexpr float STRIPE_OTHER_LIGHT = 0.6f;

// ==========================================================================
/// The weights of the left, center and right pixel for the beam.
constexpr std::array<float, 3> BEAM_KERNEL = { BEAM_SPREAD, 1 - 2 * BEAM_SPREAD, BEAM_SPREAD };
/// Before linear light becomes an sRGB value again, it is rounded to one of this many levels.
/// With 4096 levels, every sRGB value comes back unchanged, so flat areas keep their exact color.
constexpr int LINEAR_LEVELS = 4096;

/// Red, green and blue in linear light, one float per pixel, row by row.
using Planes = std::array<std::vector<float>, 3>;

/// sRGB value (0 ~ 255) -> linear light (0.0 ~ 1.0).
/// The filter blends linear light, because light adds up on a real screen. If it blended the sRGB values directly,
/// a black and white checkerboard would become a gray that is too dark.
const std::array<float, 256> &srgbToLinear()
{
    static const std::array<float, 256> table = [] {
        std::array<float, 256> t {};
        for (int i = 0; i < 256; i++) {
            const float c = i / 255.0f;
            t[i] = (c <= 0.04045f) ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
        }
        return t;
    }();
    return table;
}

/// Linear light (any value; it is clamped to 0.0 ~ 1.0) -> sRGB value (0 ~ 255).
uint8_t linearToSrgb(float light)
{
    static const std::array<uint8_t, LINEAR_LEVELS> table = [] {
        std::array<uint8_t, LINEAR_LEVELS> t {};
        for (int i = 0; i < LINEAR_LEVELS; i++) {
            const float c = float(i) / (LINEAR_LEVELS - 1);
            const float s = (c <= 0.0031308f) ? c * 12.92f : 1.055f * std::pow(c, 1 / 2.4f) - 0.055f;
            t[i] = uint8_t(std::lround(s * 255));
        }
        return t;
    }();
    return table[std::clamp(int(light * (LINEAR_LEVELS - 1) + 0.5f), 0, LINEAR_LEVELS - 1)];
}

/// How much of a Gaussian with sigma 1 lies to the left of `t`: 0 ~ 1.
float gaussianCdf(float t)
{
    constexpr int STEPS = 1024;   // Over t = -4 ~ 4
    static const std::array<float, STEPS + 1> table = [] {
        std::array<float, STEPS + 1> c {};
        for (int i = 0; i <= STEPS; i++) {
            c[i] = 0.5f * (1 + std::erf((-4 + 8.0f * i / STEPS) / std::sqrt(2.0f)));
        }
        return c;
    }();
    const float position = (std::clamp(t, -4.0f, 4.0f) + 4) * (STEPS / 8.0f);
    const int i = std::min(int(position), STEPS - 1);
    return table[i] + (position - i) * (table[i + 1] - table[i]);
}

/// A Gaussian kernel with 2 * radius + 1 weights, which add up to 1.
/// Sigma is radius / 3, so the weight at each end is about 1% of the weight in the middle.
std::vector<float> gaussianKernel(int radius)
{
    const float sigma = radius / 3.0f;
    std::vector<float> kernel(2 * radius + 1);
    float sum = 0;
    for (int i = -radius; i <= radius; i++) {
        kernel[i + radius] = std::exp(-0.5f * i * i / (sigma * sigma));
        sum += kernel[i + radius];
    }
    for (float &weight : kernel) {
        weight /= sum;
    }
    return kernel;
}

/// Blurs each row of `src` into `dst`: dst[x] is the sum of kernel[k] * src[x + k - radius].
/// Outside the image, the pixel at the edge repeats.
void blurRows(const std::vector<float> &src, std::vector<float> &dst, int width, int height, std::span<const float> kernel)
{
    const int radius = int(kernel.size()) / 2;
    std::vector<float> paddedRow(width + 2 * radius);
    for (int y = 0; y < height; y++) {
        const float *row = src.data() + qsizetype(y) * width;
        std::fill_n(paddedRow.begin(), radius, row[0]);
        std::copy_n(row, width, paddedRow.begin() + radius);
        std::fill_n(paddedRow.begin() + radius + width, radius, row[width - 1]);
        float *out = dst.data() + qsizetype(y) * width;
        std::fill_n(out, width, 0.0f);
        for (size_t k = 0; k < kernel.size(); k++) {
            const float *in = paddedRow.data() + k;
            for (int x = 0; x < width; x++) {
                out[x] += kernel[k] * in[x];
            }
        }
    }
}

/// The same as blurRows(), but along each column.
void blurColumns(const std::vector<float> &src, std::vector<float> &dst, int width, int height, std::span<const float> kernel)
{
    const int radius = int(kernel.size()) / 2;
    for (int y = 0; y < height; y++) {
        float *out = dst.data() + qsizetype(y) * width;
        std::fill_n(out, width, 0.0f);
        for (int k = 0; k < int(kernel.size()); k++) {
            const int srcY = std::clamp(y + k - radius, 0, height - 1);
            const float *in = src.data() + qsizetype(srcY) * width;
            for (int x = 0; x < width; x++) {
                out[x] += kernel[k] * in[x];
            }
        }
    }
}

/// The shift of red, green and blue (0, 1, 2) in a QRgb.
constexpr int shiftOf(int color) { return 16 - 8 * color; }

/// Fills `beam` with the light of `image` after the beam blend, and `halo` with the glow that the beam adds to darker
/// pixels. The glow only adds light: it brightens a pixel next to brighter pixels, and it never darkens a pixel.
/// So `halo` is 0 in flat areas.
void computeBeamAndHalo(const QImage &image, Planes &beam, Planes &halo)
{
    const int width = image.width();
    const int height = image.height();
    const qsizetype pixelCount = qsizetype(width) * height;
    const auto &toLinear = srgbToLinear();
    static const std::vector<float> glowKernel = gaussianKernel(CrtFilter::GLOW_RADIUS);
    std::vector<float> channel(pixelCount);
    std::vector<float> blurredRows(pixelCount);
    std::vector<float> glow(pixelCount);
    for (int color = 0; color < 3; color++) {
        for (int y = 0; y < height; y++) {
            const QRgb *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
            for (int x = 0; x < width; x++) {
                channel[qsizetype(y) * width + x] = toLinear[(line[x] >> shiftOf(color)) & 0xff];
            }
        }
        beam[color].resize(pixelCount);
        halo[color].resize(pixelCount);
        blurRows(channel, beam[color], width, height, BEAM_KERNEL);
        blurRows(beam[color], blurredRows, width, height, glowKernel);
        blurColumns(blurredRows, glow, width, height, glowKernel);
        for (qsizetype i = 0; i < pixelCount; i++) {
            halo[color][i] = std::max(0.0f, glow[i] - beam[color][i]);
        }
    }
}
}

void CrtFilter::apply(QImage &image)
{
    Q_ASSERT(image.format() == QImage::Format_RGB32);
    if (image.isNull()) { return; }
    Planes beam;
    Planes halo;
    computeBeamAndHalo(image, beam, halo);
    for (int y = 0; y < image.height(); y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < image.width(); x++) {
            const qsizetype i = qsizetype(y) * image.width() + x;
            QRgb pixel = 0xff000000;
            for (int color = 0; color < 3; color++) {
                pixel |= QRgb(linearToSrgb(beam[color][i] + GLOW_AMOUNT * halo[color][i])) << shiftOf(color);
            }
            line[x] = pixel;
        }
    }
}

QImage CrtFilter::render(const QImage &image, QPoint topLeft, qreal devicePixelsPerKissPixel)
{
    Q_ASSERT(image.format() == QImage::Format_RGB32);
    const float scale = devicePixelsPerKissPixel;
    const int width = image.width();
    const int height = image.height();
    const int outWidth = qRound(width * scale);
    const int outHeight = qRound(height * scale);
    QImage result(outWidth, outHeight, QImage::Format_RGB32);
    if (image.isNull() || result.isNull()) { return image; }
    Planes beam;
    Planes halo;
    computeBeamAndHalo(image, beam, halo);

    // 1. Each row of KiSS pixels, at each output column: the light of the spots of the pixels nearby. A spot is the
    //    pixel spread out by a Gaussian, so it adds the part of the Gaussian that falls on the output column.
    Planes rowLight;   // `height` rows, `outWidth` columns
    for (int color = 0; color < 3; color++) {
        rowLight[color].resize(qsizetype(height) * outWidth);
        for (int y = 0; y < height; y++) {
            const float *in = beam[color].data() + qsizetype(y) * width;
            float *out = rowLight[color].data() + qsizetype(y) * outWidth;
            for (int x = 0; x < outWidth; x++) {
                const float u = (x + 0.5f) / scale;   // In KiSS pixels. Pixel i covers u = i ~ i + 1.
                const int nearest = std::min(int(u), width - 1);
                float light = 0;
                for (int i = std::max(nearest - 2, 0); i <= std::min(nearest + 2, width - 1); i++) {
                    const float sigma = SPOT_SIGMA_DARK + (SPOT_SIGMA_BRIGHT - SPOT_SIGMA_DARK) * in[i];
                    light += in[i] * (gaussianCdf((u - i) / sigma) - gaussianCdf((u - i - 1) / sigma));
                }
                out[x] = light;
            }
        }
    }

    // 2. Each output row adds the scanlines of the KiSS rows above, its own row and the row below. Then come the
    //    phosphor stripes, and the glow on top of them.
    // Each stripe is a whole number of device pixels, and the 3 colors are equally wide.
    // Otherwise one color would cover more device pixels than the others, and the whole picture would get its tint.
    const int stripeWidth = int(scale / 3);   // 0: no stripes
    const int firstColumn = qRound(topLeft.x() * scale);   // Counted from x = 0 of the scene, so the stripes stay in place
    constexpr int LIGHT_STEPS = 256;
    std::array<std::array<float, LIGHT_STEPS + 1>, 3> scanlineWeight;   // For the row above, own row, row below
    for (int y = 0; y < outHeight; y++) {
        const float v = (y + 0.5f) / scale;   // In KiSS pixels. Row j covers v = j ~ j + 1, and its scanline is at j + 0.5.
        const int row = std::min(int(v), height - 1);
        for (int n = 0; n < 3; n++) {
            const float distance = v - (row + n - 1 + 0.5f);
            for (int step = 0; step <= LIGHT_STEPS; step++) {
                const float light = float(step) / LIGHT_STEPS;
                const float sigma = SCANLINE_SIGMA_DARK + (SCANLINE_SIGMA_BRIGHT - SCANLINE_SIGMA_DARK) * light;
                scanlineWeight[n][step] = std::exp(-0.5f * distance * distance / (sigma * sigma));
            }
        }
        // The glow is interpolated between the 4 nearest KiSS pixels.
        const float glowV = std::clamp(v - 0.5f, 0.0f, float(height - 1));
        const int glowRow0 = std::min(int(glowV), height - 1);
        const int glowRow1 = std::min(glowRow0 + 1, height - 1);
        const float glowFractionV = glowV - glowRow0;
        QRgb *out = reinterpret_cast<QRgb *>(result.scanLine(y));
        for (int x = 0; x < outWidth; x++) {
            int stripeColor = -1;   // Red, green or blue (0, 1, 2), or -1 without stripes
            if (stripeWidth > 0) {
                const int period = 3 * stripeWidth;
                stripeColor = ((firstColumn + x) % period + period) % period / stripeWidth;
            }
            const float u = (x + 0.5f) / scale;
            const float glowU = std::clamp(u - 0.5f, 0.0f, float(width - 1));
            const int glowColumn0 = std::min(int(glowU), width - 1);
            const int glowColumn1 = std::min(glowColumn0 + 1, width - 1);
            const float glowFractionU = glowU - glowColumn0;
            QRgb pixel = 0xff000000;
            for (int color = 0; color < 3; color++) {
                float light = 0;
                for (int n = 0; n < 3; n++) {
                    const int scanline = std::clamp(row + n - 1, 0, height - 1);
                    const float rowValue = rowLight[color][qsizetype(scanline) * outWidth + x];
                    light += rowValue * scanlineWeight[n][std::clamp(int(rowValue * LIGHT_STEPS), 0, LIGHT_STEPS)];
                }
                light *= SCANLINE_BRIGHTNESS;
                if (stripeColor >= 0 && stripeColor != color) {
                    light *= STRIPE_OTHER_LIGHT;
                }
                const float *glow = halo[color].data();
                const float glowTop = glow[qsizetype(glowRow0) * width + glowColumn0] * (1 - glowFractionU)
                                    + glow[qsizetype(glowRow0) * width + glowColumn1] * glowFractionU;
                const float glowBottom = glow[qsizetype(glowRow1) * width + glowColumn0] * (1 - glowFractionU)
                                       + glow[qsizetype(glowRow1) * width + glowColumn1] * glowFractionU;
                light += GLOW_AMOUNT * (glowTop * (1 - glowFractionV) + glowBottom * glowFractionV);
                pixel |= QRgb(linearToSrgb(light)) << shiftOf(color);
            }
            out[x] = pixel;
        }
    }
    return result;
}
