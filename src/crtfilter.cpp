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
// (GLOW_RADIUS and SVIDEO_RADIUS are in crtfilter.h, because GKissView needs them too.)
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
/// render() only: each row of pixels is a scanline, which fades up and down like the beam of CRT-Royale (a generalized
/// Gaussian). Its width is a sigma in rows of KiSS pixels, from SCANLINE_SIGMA_DARK (black) to SCANLINE_SIGMA_BRIGHT
/// (full brightness): sigma = DARK + (BRIGHT - DARK) * brightness ^ SCANLINE_SIGMA_POWER.
/// A wider scanline leaves a thinner dark gap. (CRT-Royale uses 0.02 and 0.3, which make the gaps of dark and middle
/// colors almost black.)
constexpr float SCANLINE_SIGMA_DARK = 0.12f;
constexpr float SCANLINE_SIGMA_BRIGHT = 0.38f;
constexpr float SCANLINE_SIGMA_POWER = 1 / 3.0f;
/// render() only: the shape of a scanline. 2 is a Gaussian. Larger values make its middle flatter and its edges
/// sharper: shape = DARK + (BRIGHT - DARK) * brightness ^ SCANLINE_SHAPE_POWER.
constexpr float SCANLINE_SHAPE_DARK = 2;
constexpr float SCANLINE_SHAPE_BRIGHT = 4;
constexpr float SCANLINE_SHAPE_POWER = 0.25f;
/// render() only: how much light of the other two colors a phosphor stripe lets through: 0 (none) ~ 1 (all, so no
/// stripes). The stripes don't make the picture darker, because the light they take away is added back.
constexpr float STRIPE_OTHER_LIGHT = 0.6f;
/// render() only: light above full brightness can't be shown. It spreads to the pixels around it, this far (a sigma in
/// KiSS pixels), like the bloom of CRT-Royale. 0 means that it is cut off instead.
constexpr float BLOOM_SIGMA = 0.3f;

// ==========================================================================
/// CrtFilter::REACH is how far one pixel can change other pixels, and GKissView renders that far (and 1 pixel more)
/// around the area that it shows. REACH follows the glow, so the bloom must not reach further. The bloom spreads the
/// light of the spots, which reach 1 KiSS pixel (the beam blend) + up to 3 (the spot), by about 3 * BLOOM_SIGMA KiSS
/// pixels, + 1 for rounding.
static_assert(1 + 3 + 3 * BLOOM_SIGMA + 1 <= 1 + CrtFilter::GLOW_RADIUS + 1, "The bloom must not reach further than the glow");
/// The weights of the left, center and right pixel for the beam.
constexpr std::array<float, 3> BEAM_KERNEL = { BEAM_SPREAD, 1 - 2 * BEAM_SPREAD, BEAM_SPREAD };
/// Before linear light becomes an sRGB value again, it is rounded to one of this many levels.
/// With 4096 levels, every sRGB value comes back unchanged, so flat areas keep their exact color.
constexpr int LINEAR_LEVELS = 4096;

/// Red, green and blue, one float per pixel, row by row. Linear light, except in svideoSignal().
using Planes = std::array<std::vector<float>, 3>;

/// sRGB value (0.0 ~ 1.0) -> linear light (0.0 ~ 1.0).
/// The filter blends linear light, because light adds up on a real screen. If it blended the sRGB values directly,
/// a black and white checkerboard would become a gray that is too dark.
float srgbToLinear(float c)
{
    return (c <= 0.04045f) ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
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

/// The sRGB values of `image` (0.0 ~ 1.0), as the video signal brings them to the CRT. With CrtFilter::SVIDEO_RADIUS,
/// the signal comes through an S-Video cable, so the colors bleed to the left and right.
/// The cable carries the brightness (luma) and the color differences (red - luma, blue - luma), and only the color
/// differences lose detail. This happens before the CRT turns the signal into light, so it works on the sRGB values,
/// not on linear light.
Planes videoSignal(const QImage &image)
{
    const int width = image.width();
    const int height = image.height();
    const qsizetype pixelCount = qsizetype(width) * height;
    Planes signal;
    for (int color = 0; color < 3; color++) {
        signal[color].resize(pixelCount);
        for (int y = 0; y < height; y++) {
            const QRgb *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
            for (int x = 0; x < width; x++) {
                signal[color][qsizetype(y) * width + x] = ((line[x] >> shiftOf(color)) & 0xff) / 255.0f;
            }
        }
    }
    if constexpr (CrtFilter::SVIDEO_RADIUS > 0) {
        static const std::vector<float> kernel = gaussianKernel(CrtFilter::SVIDEO_RADIUS);
        auto &[red, green, blue] = signal;
        std::vector<float> luma(pixelCount);
        for (qsizetype i = 0; i < pixelCount; i++) {
            luma[i] = 0.299f * red[i] + 0.587f * green[i] + 0.114f * blue[i];
        }
        std::vector<float> difference(pixelCount);
        std::vector<float> blurred(pixelCount);
        for (std::vector<float> *channel : { &red, &blue }) {
            for (qsizetype i = 0; i < pixelCount; i++) {
                difference[i] = (*channel)[i] - luma[i];
            }
            blurRows(difference, blurred, width, height, kernel);
            for (qsizetype i = 0; i < pixelCount; i++) {
                (*channel)[i] = luma[i] + blurred[i];
            }
        }
        for (qsizetype i = 0; i < pixelCount; i++) {
            green[i] = (luma[i] - 0.299f * red[i] - 0.114f * blue[i]) / 0.587f;
        }
        // A color that bleeds into another color can come out more saturated than the screen can show. It is cut off.
        for (std::vector<float> &channel : signal) {
            for (float &value : channel) {
                value = std::clamp(value, 0.0f, 1.0f);
            }
        }
    }
    return signal;
}

/// Steps of brightness in the table of scanlineShare(): 0 (black) ~ SCANLINE_LEVELS (full brightness).
constexpr int SCANLINE_LEVELS = 256;

/// render() only: the share of a scanline's light that lies between its middle and `distance` rows of KiSS pixels
/// below it: 0 ~ 0.5. For a negative `distance` (above the middle), the share is negative. `level` is the brightness of
/// the scanline: 0 ~ SCANLINE_LEVELS.
/// Like the beam of CRT-Royale, a scanline fades like a generalized Gaussian, exp(-(|distance| / alpha) ^ shape), and
/// all of its light adds up to its brightness. So a thin scanline is brighter in its middle, and the scanlines don't
/// make the picture darker. See SCANLINE_SIGMA_DARK and SCANLINE_SHAPE_DARK.
float scanlineShare(int level, float distance)
{
    constexpr float T_MAX = 6;   // Beyond |distance| = 6 * alpha, no light is left.
    constexpr int T_STEPS = 600;
    struct Table
    {
        std::array<float, SCANLINE_LEVELS + 1> alpha {};   // In rows of KiSS pixels
        /// For each level: the share between the middle and t * alpha rows below it, for t = 0 ~ T_MAX.
        std::vector<std::array<float, T_STEPS + 1>> share = std::vector<std::array<float, T_STEPS + 1>>(SCANLINE_LEVELS + 1);
    };
    static const Table table = [] {
        Table t;
        for (int level = 0; level <= SCANLINE_LEVELS; level++) {
            const float brightness = float(level) / SCANLINE_LEVELS;
            const float sigma = SCANLINE_SIGMA_DARK
                + (SCANLINE_SIGMA_BRIGHT - SCANLINE_SIGMA_DARK) * std::pow(brightness, SCANLINE_SIGMA_POWER);
            const double shape = SCANLINE_SHAPE_DARK
                + (SCANLINE_SHAPE_BRIGHT - SCANLINE_SHAPE_DARK) * std::pow(brightness, SCANLINE_SHAPE_POWER);
            t.alpha[level] = std::sqrt(2.0f) * sigma;
            // Add up exp(-u ^ shape) from u = 0 to T_MAX, in small trapezoids.
            constexpr int SUBSTEPS = 8;
            const double du = double(T_MAX) / (T_STEPS * SUBSTEPS);
            std::array<double, T_STEPS + 1> sum {};
            double previous = 1;   // At u = 0
            for (int step = 1; step <= T_STEPS; step++) {
                sum[step] = sum[step - 1];
                for (int sub = 1; sub <= SUBSTEPS; sub++) {
                    const double value = std::exp(-std::pow(((step - 1) * SUBSTEPS + sub) * du, shape));
                    sum[step] += (previous + value) / 2 * du;
                    previous = value;
                }
            }
            for (int step = 0; step <= T_STEPS; step++) {
                t.share[level][step] = float(0.5 * sum[step] / sum[T_STEPS]);   // Half of the light is below the middle
            }
        }
        return t;
    }();
    const float t = std::abs(distance) / table.alpha[level] * (T_STEPS / T_MAX);   // In steps of the table
    if (t >= T_STEPS) { return std::copysign(0.5f, distance); }
    const int step = int(t);
    const auto &share = table.share[level];
    return std::copysign(share[step] + (t - step) * (share[step + 1] - share[step]), distance);
}

/// Fills `beam` with the light of `image` after the beam blend, and `halo` with the glow that the beam adds to darker
/// pixels. The glow only adds light: it brightens a pixel next to brighter pixels, and it never darkens a pixel.
/// So `halo` is 0 in flat areas.
void computeBeamAndHalo(const QImage &image, Planes &beam, Planes &halo)
{
    const int width = image.width();
    const int height = image.height();
    const qsizetype pixelCount = qsizetype(width) * height;
    static const std::vector<float> glowKernel = gaussianKernel(CrtFilter::GLOW_RADIUS);
    Planes light = videoSignal(image);
    std::vector<float> blurredRows(pixelCount);
    std::vector<float> glow(pixelCount);
    for (int color = 0; color < 3; color++) {
        for (float &value : light[color]) {
            value = srgbToLinear(value);
        }
        beam[color].resize(pixelCount);
        halo[color].resize(pixelCount);
        blurRows(light[color], beam[color], width, height, BEAM_KERNEL);
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

    // 2. Each output row adds the light of the scanlines of the KiSS rows above, its own row and the row below: the part
    //    of each scanline's light that falls between the top and the bottom of the output row. Then come the phosphor
    //    stripes.
    // Each stripe is a whole number of device pixels, and the 3 colors are equally wide.
    // Otherwise one color would cover more device pixels than the others, and the whole picture would get its tint.
    const int stripeWidth = int(scale / 3);   // 0: no stripes
    const int firstColumn = qRound(topLeft.x() * scale);   // Counted from x = 0 of the scene, so the stripes stay in place
    // The stripes take away light of the other colors. Add it back, so that the picture keeps its brightness.
    const float stripeCompensation = (stripeWidth > 0) ? 3 / (1 + 2 * STRIPE_OTHER_LIGHT) : 1;
    // Where the middle of each scanline is, inside its row of KiSS pixels (0 ~ 1). At a whole-number zoom, it is the
    // middle of a row of device pixels, so that each scanline has a brightest row of device pixels. (At 200%, the middle
    // of the KiSS row would lie between its 2 rows of device pixels and light both of them equally: no gaps at all.)
    // The rows of device pixels start at a KiSS row (topLeft is a whole KiSS pixel), so the scanlines stay in place.
    const float wholeScale = std::round(scale);
    const float scanlineMiddle =
        (std::abs(scale - wholeScale) < 0.001f) ? (std::floor((wholeScale - 1) / 2) + 0.5f) / wholeScale : 0.5f;
    const float outRowHeight = 1 / scale;   // In rows of KiSS pixels
    const qsizetype outPixelCount = qsizetype(outWidth) * outHeight;
    Planes screen;   // The light on the screen, before the bloom and the glow
    for (std::vector<float> &channel : screen) {
        channel.resize(outPixelCount);
    }
    // For the scanline above, its own scanline and the one below: their light that falls on the output row, as a share of
    // their brightness, divided by the height of the output row (so that it is the light per row of KiSS pixels).
    std::array<std::array<float, SCANLINE_LEVELS + 1>, 3> scanlineWeight;
    for (int y = 0; y < outHeight; y++) {
        const float v = (y + 0.5f) / scale;   // In KiSS pixels. Row j covers v = j ~ j + 1.
        const int row = std::min(int(v), height - 1);
        for (int n = 0; n < 3; n++) {
            const float distance = v - (row + n - 1 + scanlineMiddle);
            for (int level = 0; level <= SCANLINE_LEVELS; level++) {
                scanlineWeight[n][level] = (scanlineShare(level, distance + outRowHeight / 2)
                                            - scanlineShare(level, distance - outRowHeight / 2)) / outRowHeight;
            }
        }
        for (int x = 0; x < outWidth; x++) {
            int stripeColor = -1;   // Red, green or blue (0, 1, 2), or -1 without stripes
            if (stripeWidth > 0) {
                const int period = 3 * stripeWidth;
                stripeColor = ((firstColumn + x) % period + period) % period / stripeWidth;
            }
            for (int color = 0; color < 3; color++) {
                float light = 0;
                for (int n = 0; n < 3; n++) {
                    const int scanline = row + n - 1;
                    if (scanline < 0 || scanline >= height) { continue; }
                    const float rowValue = rowLight[color][qsizetype(scanline) * outWidth + x];
                    // Interpolated between the 2 nearest levels
                    const float level = std::clamp(rowValue, 0.0f, 1.0f) * SCANLINE_LEVELS;
                    const int level0 = std::min(int(level), SCANLINE_LEVELS - 1);
                    const auto &weight = scanlineWeight[n];
                    light += rowValue * (weight[level0] + (level - level0) * (weight[level0 + 1] - weight[level0]));
                }
                if (stripeColor >= 0 && stripeColor != color) {
                    light *= STRIPE_OTHER_LIGHT;
                }
                screen[color][qsizetype(y) * outWidth + x] = light * stripeCompensation;
            }
        }
    }

    // 3. Light above full brightness can't be shown. Like the bloom of CRT-Royale, it spreads to the pixels around it
    //    instead of being cut off. So bright areas keep their brightness, and the light fills the dark gaps between their
    //    scanlines and stripes.
    const int bloomRadius = int(std::ceil(3 * BLOOM_SIGMA * scale));   // In device pixels. gaussianKernel(): sigma = radius / 3
    if (bloomRadius > 0) {
        const std::vector<float> bloomKernel = gaussianKernel(bloomRadius);
        std::vector<float> excess(outPixelCount);
        std::vector<float> blurredRows(outPixelCount);
        std::vector<float> bloom(outPixelCount);
        for (int color = 0; color < 3; color++) {
            for (qsizetype i = 0; i < outPixelCount; i++) {
                excess[i] = std::max(0.0f, screen[color][i] - 1);
            }
            blurRows(excess, blurredRows, outWidth, outHeight, bloomKernel);
            blurColumns(blurredRows, bloom, outWidth, outHeight, bloomKernel);
            for (qsizetype i = 0; i < outPixelCount; i++) {
                screen[color][i] += bloom[i] - excess[i];
            }
        }
    }

    // 4. The glow comes last, so the scanlines and the stripes don't cover it.
    for (int y = 0; y < outHeight; y++) {
        const float v = (y + 0.5f) / scale;
        // The glow is interpolated between the 4 nearest KiSS pixels.
        const float glowV = std::clamp(v - 0.5f, 0.0f, float(height - 1));
        const int glowRow0 = std::min(int(glowV), height - 1);
        const int glowRow1 = std::min(glowRow0 + 1, height - 1);
        const float glowFractionV = glowV - glowRow0;
        QRgb *out = reinterpret_cast<QRgb *>(result.scanLine(y));
        for (int x = 0; x < outWidth; x++) {
            const float u = (x + 0.5f) / scale;
            const float glowU = std::clamp(u - 0.5f, 0.0f, float(width - 1));
            const int glowColumn0 = std::min(int(glowU), width - 1);
            const int glowColumn1 = std::min(glowColumn0 + 1, width - 1);
            const float glowFractionU = glowU - glowColumn0;
            QRgb pixel = 0xff000000;
            for (int color = 0; color < 3; color++) {
                float light = screen[color][qsizetype(y) * outWidth + x];
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
