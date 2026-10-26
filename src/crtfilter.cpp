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
#include <numbers>
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
/// render() for Glow and Soft: each pixel is a spot of light, which spreads to the left and right like a Gaussian.
/// These are its sigma for black and for full brightness, in KiSS pixels. A bright spot reaches into darker neighbors.
constexpr float SPOT_SIGMA_DARK = 0.2f;
constexpr float SPOT_SIGMA_BRIGHT = 0.5f;
/// render() only: each row of pixels is a scanline, which fades up and down like the beam of CRT-Royale (a generalized
/// Gaussian). Its width is a sigma in rows of KiSS pixels, from SCANLINE_SIGMA_DARK (black) to SCANLINE_SIGMA_BRIGHT
/// (full brightness): sigma = DARK + (BRIGHT - DARK) * brightness ^ SCANLINE_SIGMA_POWER.
/// A wider scanline leaves a thinner dark gap. (Sharp and Soft use the widths of CRT-Royale instead,
/// ROYALE_SCANLINE_SIGMA_DARK and ROYALE_SCANLINE_SIGMA_BRIGHT, which make the gaps of dark and middle colors almost
/// black.)
constexpr float SCANLINE_SIGMA_DARK = 0.12f;
constexpr float SCANLINE_SIGMA_BRIGHT = 0.38f;
constexpr float SCANLINE_SIGMA_POWER = 1 / 3.0f;
/// render() only: the shape of a scanline. 2 is a Gaussian. Larger values make its middle flatter and its edges
/// sharper: shape = DARK + (BRIGHT - DARK) * brightness ^ SCANLINE_SHAPE_POWER.
constexpr float SCANLINE_SHAPE_DARK = 2;
constexpr float SCANLINE_SHAPE_BRIGHT = 4;
constexpr float SCANLINE_SHAPE_POWER = 0.25f;
/// render() for Glow: how much light of the other two colors a phosphor stripe lets through: 0 (none) ~ 1 (all, so no
/// stripes). The stripes don't make the picture darker, because the light they take away is added back.
constexpr float STRIPE_OTHER_LIGHT = 0.6f;
/// render() for Glow: light above full brightness can't be shown. It spreads to the pixels around it, this far (a sigma in
/// KiSS pixels), like the bloom of CRT-Royale. 0 means that it is cut off instead.
constexpr float BLOOM_SIGMA = 0.3f;

// ==========================================================================
// The presets Sharp and Soft copy the look of CRT-Royale with its default settings, as closely as they can.
// CRT-Royale is a CRT shader for RetroArch, Copyright (C) 2014 TroggleMonkey <trogglemonkey@gmx.com>, under the GNU
// General Public License, version 2 or (at your option) any later version:
// https://github.com/libretro/slang-shaders/tree/master/crt/shaders/crt-royale (commit 901441a).
// renderRoyale() and the functions it uses are ported from it. The names in [brackets] are CRT-Royale's names.
// ==========================================================================
/// The gamma of the CRT [crt_gamma] and the gamma of the screen that shows the picture now [lcd_gamma].
/// The gamma of the CRT is higher, so dark and middle colors come out darker than in the KiSS set.
constexpr float ROYALE_CRT_GAMMA = 2.5f;
constexpr float ROYALE_LCD_GAMMA = 2.2f;
/// The width of a scanline for black and for full brightness [beam_min_sigma, beam_max_sigma]. See SCANLINE_SIGMA_DARK.
/// (The other values of the scanline shape are the same as ours.)
constexpr float ROYALE_SCANLINE_SIGMA_DARK = 0.02f;
constexpr float ROYALE_SCANLINE_SIGMA_BRIGHT = 0.3f;
/// The width of one red, green and blue triad of the slot mask, in device pixels [mask_triad_size_desired].
constexpr int ROYALE_TRIAD_SIZE = 3;
/// How much light the mask lets through on average [mask_slot_avg_color]. The light is multiplied by the inverse of
/// this value, so that the mask doesn't make the picture darker.
constexpr float ROYALE_MASK_AVERAGE = 46 / 255.0f;
/// The light that a pixel can't show (above full brightness) spreads over about one triad. To decide how much light
/// spreads, CRT-Royale counts only this share of the light [bloom_underestimate_levels]. A lower value spreads less.
constexpr float ROYALE_BLOOM_UNDERESTIMATE = 0.8f;
/// To decide how much light a pixel spreads, CRT-Royale also needs the light around it: a blurred copy of the picture
/// [BLOOM_APPROX]. This is the sigma of that blur, in KiSS pixels.
/// CRT-Royale makes the copy 320 pixels wide, whatever the size of the picture, so the size of the blur depends on the
/// picture. This value assumes a KiSS set 640 pixels wide.
constexpr float ROYALE_AREA_SIGMA = 0.8f;
/// How much of the light scatters in the glass of the screen [diffusion_weight]. It reaches as far as
/// CrtFilter::DIFFUSION_RADIUS: a Gaussian with sigma DIFFUSION_RADIUS / 3 = 3.7 KiSS pixels, which is CRT-Royale's
/// size [HALATION_BLUR] for a KiSS set 640 pixels wide.
constexpr float ROYALE_DIFFUSION_WEIGHT = 0.075f;

// ==========================================================================
/// CrtFilter::REACH is how far one pixel can change other pixels, and GKissView renders that far (and 1 pixel more)
/// around the area that it shows. REACH covers the glow, so the bloom must not reach further. The bloom spreads the
/// light of the spots, which reach 1 KiSS pixel (the beam blend) + up to 3 (the spot), by about 3 * BLOOM_SIGMA KiSS
/// pixels, + 1 for rounding.
static_assert(1 + 3 + 3 * BLOOM_SIGMA + 1 <= 1 + CrtFilter::GLOW_RADIUS + 1, "The bloom must not reach further than the glow");
/// The radius of the blur of ROYALE_AREA_SIGMA, in KiSS pixels (3 sigma, rounded up).
constexpr int ROYALE_AREA_RADIUS = 3;
static_assert(ROYALE_AREA_RADIUS >= 3 * ROYALE_AREA_SIGMA);
/// The radius of the bloom of renderRoyale(), in device pixels. CRT-Royale blurs 17 pixels [tex2Dblur17fast()].
constexpr int ROYALE_BLOOM_RADIUS = 8;
/// In renderRoyale(), the bloom spreads light by ROYALE_BLOOM_RADIUS device pixels, which are at most 4 KiSS pixels at
/// CrtFilter::MIN_RENDER_SCALE. That light came from up to 3 KiSS pixels away (the beam blend and the spots), and how
/// much of it spreads depends on the light ROYALE_AREA_RADIUS + 1 KiSS pixels away (+ 1 because it is interpolated).
/// The S-Video cable comes before all of it.
static_assert(CrtFilter::SVIDEO_RADIUS + ROYALE_BLOOM_RADIUS / CrtFilter::MIN_RENDER_SCALE + std::max(3, ROYALE_AREA_RADIUS + 1)
                  <= CrtFilter::REACH,
              "The bloom must not reach further than CrtFilter::REACH");
/// The weights of the left, center and right pixel for the beam.
constexpr std::array<float, 3> BEAM_KERNEL = { BEAM_SPREAD, 1 - 2 * BEAM_SPREAD, BEAM_SPREAD };
/// Before linear light becomes an sRGB value again, it is rounded to one of this many levels.
/// With 4096 levels, every sRGB value comes back unchanged, so flat areas keep their exact color.
constexpr int LINEAR_LEVELS = 4096;

/// Red, green and blue, one float per pixel, row by row. Linear light, except in videoSignal().
using Planes = std::array<std::vector<float>, 3>;

/// sRGB value (0.0 ~ 1.0) -> linear light (0.0 ~ 1.0).
/// The filter blends linear light, because light adds up on a real screen. If it blended the sRGB values directly,
/// a black and white checkerboard would become a gray that is too dark.
float srgbToLinear(float c)
{
    return (c <= 0.04045f) ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
}

/// The table of linearToSrgb(): the sRGB value (0 ~ 255) of each of the LINEAR_LEVELS steps of linear light.
const std::array<uint8_t, LINEAR_LEVELS> &srgbTable()
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
    return table;
}

/// Linear light (any value; it is clamped to 0.0 ~ 1.0) -> sRGB value (0 ~ 255).
uint8_t linearToSrgb(float light)
{
    return srgbTable()[std::clamp(int(light * (LINEAR_LEVELS - 1) + 0.5f), 0, LINEAR_LEVELS - 1)];
}

/// The table of gaussianCdf() has GAUSSIAN_CDF_STEPS + 1 values, for t = -GAUSSIAN_CDF_MAX ~ GAUSSIAN_CDF_MAX.
constexpr int GAUSSIAN_CDF_STEPS = 1024;
constexpr float GAUSSIAN_CDF_MAX = 4;

/// The table of gaussianCdf().
const std::array<float, GAUSSIAN_CDF_STEPS + 1> &gaussianCdfTable()
{
    static const std::array<float, GAUSSIAN_CDF_STEPS + 1> table = [] {
        std::array<float, GAUSSIAN_CDF_STEPS + 1> c {};
        for (int i = 0; i <= GAUSSIAN_CDF_STEPS; i++) {
            const float t = -GAUSSIAN_CDF_MAX + 2 * GAUSSIAN_CDF_MAX * i / GAUSSIAN_CDF_STEPS;
            c[i] = 0.5f * (1 + std::erf(t / std::sqrt(2.0f)));
        }
        return c;
    }();
    return table;
}

/// How much of a Gaussian with sigma 1 lies to the left of `t`: 0 ~ 1.
float gaussianCdf(float t)
{
    const std::array<float, GAUSSIAN_CDF_STEPS + 1> &table = gaussianCdfTable();
    const float position = (std::clamp(t, -GAUSSIAN_CDF_MAX, GAUSSIAN_CDF_MAX) + GAUSSIAN_CDF_MAX)
                         * (GAUSSIAN_CDF_STEPS / (2 * GAUSSIAN_CDF_MAX));
    const int i = std::min(int(position), GAUSSIAN_CDF_STEPS - 1);
    return table[i] + (position - i) * (table[i + 1] - table[i]);
}

/// A Gaussian kernel with 2 * radius + 1 weights, which add up to 1.
std::vector<float> gaussianKernel(int radius, float sigma)
{
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

/// A Gaussian kernel with 2 * radius + 1 weights, which add up to 1.
/// Sigma is radius / 3, so the weight at each end is about 1% of the weight in the middle.
std::vector<float> gaussianKernel(int radius)
{
    return gaussianKernel(radius, radius / 3.0f);
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

/// The weights of the blur of the S-Video cable in videoSignal(). Only for CrtFilter::SVIDEO_RADIUS > 0.
const std::vector<float> &sVideoKernel()
{
    static const std::vector<float> kernel = gaussianKernel(CrtFilter::SVIDEO_RADIUS);
    return kernel;
}

/// The weights of the blur of the glow in computeBeamAndHalo().
const std::vector<float> &glowKernel()
{
    static const std::vector<float> kernel = gaussianKernel(CrtFilter::GLOW_RADIUS);
    return kernel;
}

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
        const std::vector<float> &kernel = sVideoKernel();
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
/// The table of scanlineShare() goes from t = 0 to T_MAX in T_STEPS steps, where t = |distance| / alpha.
constexpr float T_MAX = 6;   // Beyond |distance| = 6 * alpha, no light is left.
constexpr int T_STEPS = 600;

/// The table of scanlineShare().
struct ScanlineTable
{
    std::array<float, SCANLINE_LEVELS + 1> alpha {};   // In rows of KiSS pixels
    /// For each level: the share between the middle and t * alpha rows below it, for t = 0 ~ T_MAX.
    std::vector<std::array<float, T_STEPS + 1>> share = std::vector<std::array<float, T_STEPS + 1>>(SCANLINE_LEVELS + 1);
};

/// The table of scanlineShare() for `preset` (see there).
const ScanlineTable &scanlineTable(QKiSS::CrtSimulationPreset preset)
{
    // The table for scanlines with the sigma `sigmaDark` for black and `sigmaBright` for full brightness
    const auto makeTable = [](float sigmaDark, float sigmaBright) {
        ScanlineTable t;
        for (int level = 0; level <= SCANLINE_LEVELS; level++) {
            const float brightness = float(level) / SCANLINE_LEVELS;
            const float sigma = sigmaDark + (sigmaBright - sigmaDark) * std::pow(brightness, SCANLINE_SIGMA_POWER);
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
    };
    static const ScanlineTable ourTable = makeTable(SCANLINE_SIGMA_DARK, SCANLINE_SIGMA_BRIGHT);
    static const ScanlineTable royaleTable = makeTable(ROYALE_SCANLINE_SIGMA_DARK, ROYALE_SCANLINE_SIGMA_BRIGHT);
    return (preset == QKiSS::CrtSimulationPreset::Glow) ? ourTable : royaleTable;
}

/// render() only: the share of a scanline's light that lies between its middle and `distance` rows of KiSS pixels
/// below it: 0 ~ 0.5. For a negative `distance` (above the middle), the share is negative. `level` is the brightness of
/// the scanline: 0 ~ SCANLINE_LEVELS.
/// Like the beam of CRT-Royale, a scanline fades like a generalized Gaussian, exp(-(|distance| / alpha) ^ shape), and
/// all of its light adds up to its brightness. So a thin scanline is brighter in its middle, and the scanlines don't
/// make the picture darker. See SCANLINE_SIGMA_DARK and SCANLINE_SHAPE_DARK. For Sharp and Soft (`preset`), the
/// scanlines are as wide as in CRT-Royale (ROYALE_SCANLINE_SIGMA_DARK and ROYALE_SCANLINE_SIGMA_BRIGHT).
float scanlineShare(int level, float distance, QKiSS::CrtSimulationPreset preset)
{
    const ScanlineTable &table = scanlineTable(preset);
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
        blurRows(beam[color], blurredRows, width, height, glowKernel());
        blurColumns(blurredRows, glow, width, height, glowKernel());
        for (qsizetype i = 0; i < pixelCount; i++) {
            halo[color][i] = std::max(0.0f, glow[i] - beam[color][i]);
        }
    }
}

/// render() only: the scanlines. `rows` has one row of `columns` values for each of the `height` rows of KiSS pixels.
/// The result has `outHeight` rows of `columns` values. Each output row adds the light of the scanlines of the KiSS
/// rows above, its own row and the row below: the part of each scanline's light that falls between the top and the
/// bottom of the output row. The width of the scanlines depends on `preset` (see scanlineShare()).
Planes addScanlines(const Planes &rows, int columns, int height, int outHeight, float scale,
                    QKiSS::CrtSimulationPreset preset)
{
    const float scanlineMiddle = CrtFilter::scanlineMiddle(scale);
    const float outRowHeight = 1 / scale;   // In rows of KiSS pixels
    Planes result;
    for (std::vector<float> &channel : result) {
        channel.resize(qsizetype(columns) * outHeight);
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
                scanlineWeight[n][level] = (scanlineShare(level, distance + outRowHeight / 2, preset)
                                            - scanlineShare(level, distance - outRowHeight / 2, preset)) / outRowHeight;
            }
        }
        for (int x = 0; x < columns; x++) {
            for (int color = 0; color < 3; color++) {
                float light = 0;
                for (int n = 0; n < 3; n++) {
                    const int scanline = row + n - 1;
                    if (scanline < 0 || scanline >= height) { continue; }
                    const float rowValue = rows[color][qsizetype(scanline) * columns + x];
                    // Interpolated between the 2 nearest levels
                    const float level = std::clamp(rowValue, 0.0f, 1.0f) * SCANLINE_LEVELS;
                    const int level0 = std::min(int(level), SCANLINE_LEVELS - 1);
                    const auto &weight = scanlineWeight[n];
                    light += rowValue * (weight[level0] + (level - level0) * (weight[level0 + 1] - weight[level0]));
                }
                result[color][qsizetype(y) * columns + x] = light;
            }
        }
    }
    return result;
}

/// The value of `plane` (one value per KiSS pixel, `width` x `height`) at the point (u, v) in KiSS pixels,
/// interpolated between the 4 nearest pixels. The middle of the pixel in column i and row j is at (i + 0.5, j + 0.5).
float interpolate(const std::vector<float> &plane, int width, int height, float u, float v)
{
    const float x = std::clamp(u - 0.5f, 0.0f, float(width - 1));
    const float y = std::clamp(v - 0.5f, 0.0f, float(height - 1));
    const int column0 = std::min(int(x), width - 1);
    const int column1 = std::min(column0 + 1, width - 1);
    const int row0 = std::min(int(y), height - 1);
    const int row1 = std::min(row0 + 1, height - 1);
    const float fractionX = x - column0;
    const float fractionY = y - row0;
    const float top = plane[qsizetype(row0) * width + column0] * (1 - fractionX)
                    + plane[qsizetype(row0) * width + column1] * fractionX;
    const float bottom = plane[qsizetype(row1) * width + column0] * (1 - fractionX)
                       + plane[qsizetype(row1) * width + column1] * fractionX;
    return top * (1 - fractionY) + bottom * fractionY;
}

/// renderRoyale() only: resizes one row or column of the mask to `size` values, like CRT-Royale
/// [downsample_vertical_sinc_tiled(), downsample_horizontal_sinc_tiled()]: with a Lanczos filter of 3 lobes.
/// The mask repeats, so the row or column wraps around. The result is cut off to 0 ~ 1 and rounded to 8 bits, because
/// CRT-Royale keeps it in an image with 8 bits per color.
std::vector<float> resizeMaskLine(const std::vector<float> &line, int size)
{
    constexpr float LOBES = 3;   // [mask_sinc_lobes]
    constexpr float PI = std::numbers::pi_v<float>;
    const int lineSize = int(line.size());
    const float magnification = float(size) / lineSize;
    // How many old values make each new value: enough for the 3 lobes, rounded up to a multiple of 4.
    const int samples = int(std::ceil(2 * LOBES / magnification / 4)) * 4;
    std::vector<float> result(size);
    for (int i = 0; i < size; i++) {
        // The middle of the new value, in old values. The middle of old value k is at k + 0.5.
        const float position = (i + 0.5f) / magnification;
        // The middle of the old value just before `position`. (0.4995 instead of 0.5 [under_half] avoids a rounding
        // problem when `position` is exactly the middle of an old value.)
        const float previous = std::floor(position - 0.4995f) + 0.5f;
        float sum = 0;
        float weightSum = 0;
        for (int k = 0; k < samples; k++) {
            const float middle = previous - (samples / 2 - 1) + k;
            const float distance = magnification * std::abs(position - middle);   // In new values
            const float weight = (distance == 0) ? 1
                : std::min(std::sin(PI * distance) * std::sin(PI * distance / LOBES)
                               / (PI * distance * PI * distance / LOBES), 1.0f);
            sum += weight * line[(int(std::floor(middle)) % lineSize + lineSize) % lineSize];
            weightSum += weight;
        }
        result[i] = std::round(std::clamp(sum / weightSum, 0.0f, 1.0f) * 255) / 255;
    }
    return result;
}

/// renderRoyale() only: the slot mask of CRT-Royale (`size` rows of `size` values). It is a square tile of 8 triads of
/// red, green and blue, which repeats across the screen. Each value is the share of the light of its color that the
/// mask lets through at that device pixel: 0 ~ 1.
struct MaskTile
{
    int size = 0;
    Planes light;
};

/// renderRoyale() only: the mask of CRT-Royale, resized like CRT-Royale does, so that each triad is ROYALE_TRIAD_SIZE
/// device pixels wide. The picture of the mask is in resources/crt-royale/.
const MaskTile &royaleMask()
{
    static const MaskTile mask = [] {
        MaskTile m;
        m.size = 8 * ROYALE_TRIAD_SIZE;   // [mask_triads_per_tile]
        for (std::vector<float> &channel : m.light) {
            channel.assign(qsizetype(m.size) * m.size, ROYALE_MASK_AVERAGE);   // An even mask, if the picture is missing
        }
        const QImage image = QImage(":/crt-royale/slot-mask.png").convertToFormat(QImage::Format_RGB32);
        Q_ASSERT(!image.isNull());
        if (image.isNull()) { return m; }
        const int width = image.width();
        const int height = image.height();
        for (int color = 0; color < 3; color++) {
            // The picture holds linear light (not sRGB values), so its values are used as they are.
            // Like CRT-Royale, resize each column first, then each row.
            std::vector<float> column(height);
            std::vector<float> shorter(qsizetype(width) * m.size);   // `m.size` rows of `width` values
            for (int x = 0; x < width; x++) {
                for (int y = 0; y < height; y++) {
                    column[y] = ((image.pixel(x, y) >> shiftOf(color)) & 0xff) / 255.0f;
                }
                const std::vector<float> resized = resizeMaskLine(column, m.size);
                for (int y = 0; y < m.size; y++) {
                    shorter[qsizetype(y) * width + x] = resized[y];
                }
            }
            for (int y = 0; y < m.size; y++) {
                const std::vector<float> row(shorter.begin() + qsizetype(y) * width, shorter.begin() + qsizetype(y + 1) * width);
                const std::vector<float> resized = resizeMaskLine(row, m.size);
                std::copy(resized.begin(), resized.end(), m.light[color].begin() + qsizetype(y) * m.size);
            }
        }
        return m;
    }();
    return mask;
}

/// render() for Sharp and Soft (`preset`): the picture as CRT-Royale shows it with its default settings, as closely as
/// it can. The differences from CRT-Royale:
/// - Soft blends neighboring pixels with our beam blend and spots, instead of the filter of CRT-Royale (see 2. below).
/// - The S-Video cable of videoSignal() comes first, if it is on (CrtFilter::SVIDEO_RADIUS).
/// - The scanlines add up exactly the light that falls on each row of device pixels. CRT-Royale takes 3 samples per
///   row instead, which is enough at its usual zoom (4.5 times and more), but not at 200%.
/// - At a whole-number zoom, the middle of each scanline is the middle of a row of device pixels (see
///   addScanlines()). With CRT-Royale, there would be no scanlines at 200%.
/// - CRT-Royale makes its blurred copies of the picture on a smaller copy, 320 x 240 pixels big. Here they are at KiSS
///   resolution, and their sizes assume a KiSS set 640 pixels wide (see ROYALE_AREA_SIGMA).
/// - Every step keeps its result in floats. CRT-Royale keeps 8 bits per color between its steps, so it halves the
///   light to avoid cutting it off [levels_autodim_temp], and it doubles it again later. Floats don't need that.
/// - Not copied: interlacing (a KiSS set is a still picture), the curved screen, and the dark border of the screen.
QImage renderRoyale(const QImage &image, QPoint topLeft, float scale, QKiSS::CrtSimulationPreset preset)
{
    const int width = image.width();
    const int height = image.height();
    const int outWidth = qRound(width * scale);
    const int outHeight = qRound(height * scale);
    QImage result(outWidth, outHeight, QImage::Format_RGB32);
    if (image.isNull() || result.isNull()) { return image; }
    const qsizetype pixelCount = qsizetype(width) * height;
    const qsizetype outPixelCount = qsizetype(outWidth) * outHeight;

    // 1. The video signal becomes light, with the gamma of a CRT.
    Planes light = videoSignal(image);
    for (std::vector<float> &channel : light) {
        for (float &value : channel) {
            value = std::pow(value, ROYALE_CRT_GAMMA);
        }
    }
    // Two blurred copies of the light, at KiSS resolution: the light around each pixel (for the bloom in 3.) and the
    // light that scatters in the glass (in 4.).
    const CrtFilter::LookData &data = CrtFilter::lookData();
    Planes area;
    Planes diffusion;
    std::vector<float> blurredRows(pixelCount);
    for (int color = 0; color < 3; color++) {
        area[color].resize(pixelCount);
        diffusion[color].resize(pixelCount);
        blurRows(light[color], blurredRows, width, height, data.areaKernel);
        blurColumns(blurredRows, area[color], width, height, data.areaKernel);
        blurRows(light[color], blurredRows, width, height, data.diffusionKernel);
        blurColumns(blurredRows, diffusion[color], width, height, data.diffusionKernel);
    }

    // 2. The scanlines, in each column of KiSS pixels. Then the light spreads to the left and right, to each column of
    //    device pixels.
    const Planes scanlines = addScanlines(light, width, height, outHeight, scale, preset);   // `outHeight` rows of `width` values
    Planes screen;   // The light on the screen
    for (std::vector<float> &channel : screen) {
        channel.resize(outPixelCount);
    }
    if (preset == QKiSS::CrtSimulationPreset::Sharp) {
        // Like CRT-Royale [beam_horiz_filter]: between the middles of two KiSS pixels, the light fades from one to the
        // other in an S curve. Each pixel keeps its own color in its middle, so a checkerboard stays a checkerboard.
        for (int x = 0; x < outWidth; x++) {
            const float u = (x + 0.5f) / scale - 0.5f;   // In KiSS pixels, from the middle of the first one
            const float left = std::floor(u);
            const float t = u - left;
            const float rightWeight = t * t * t * (t * (t * 6 - 15) + 10);
            const int leftColumn = std::clamp(int(left), 0, width - 1);
            const int rightColumn = std::clamp(int(left) + 1, 0, width - 1);
            for (int color = 0; color < 3; color++) {
                for (int y = 0; y < outHeight; y++) {
                    const float *in = scanlines[color].data() + qsizetype(y) * width;
                    screen[color][qsizetype(y) * outWidth + x] = (1 - rightWeight) * in[leftColumn] + rightWeight * in[rightColumn];
                }
            }
        }
    } else {
        // Soft: our beam blend and spots (BEAM_SPREAD, SPOT_SIGMA_DARK and SPOT_SIGMA_BRIGHT), as in render() for Glow,
        // but on the scanlines. They blend a checkerboard into one color. The width of a spot comes from the brightness
        // of its pixel (after the beam blend) in the KiSS row of the output row.
        Planes blendedScanlines;
        Planes brightness;
        for (int color = 0; color < 3; color++) {
            blendedScanlines[color].resize(qsizetype(width) * outHeight);
            brightness[color].resize(pixelCount);
            blurRows(scanlines[color], blendedScanlines[color], width, outHeight, BEAM_KERNEL);
            blurRows(light[color], brightness[color], width, height, BEAM_KERNEL);
        }
        for (int color = 0; color < 3; color++) {
            for (int y = 0; y < outHeight; y++) {
                const int row = std::min(int((y + 0.5f) / scale), height - 1);
                const float *in = blendedScanlines[color].data() + qsizetype(y) * width;
                const float *spotBrightness = brightness[color].data() + qsizetype(row) * width;
                float *out = screen[color].data() + qsizetype(y) * outWidth;
                for (int x = 0; x < outWidth; x++) {
                    const float u = (x + 0.5f) / scale;   // In KiSS pixels. Pixel i covers u = i ~ i + 1.
                    const int nearest = std::min(int(u), width - 1);
                    float spots = 0;
                    for (int i = std::max(nearest - 2, 0); i <= std::min(nearest + 2, width - 1); i++) {
                        const float sigma = SPOT_SIGMA_DARK + (SPOT_SIGMA_BRIGHT - SPOT_SIGMA_DARK) * spotBrightness[i];
                        spots += in[i] * (gaussianCdf((u - i) / sigma) - gaussianCdf((u - i - 1) / sigma));
                    }
                    out[x] = spots;
                }
            }
        }
    }

    // 3. The slot mask, and the bloom of CRT-Royale, which gives back the light that the mask takes away. The light is
    //    multiplied by 1 / ROYALE_MASK_AVERAGE. A pixel can't show more than full brightness, so the share of its light
    //    that it can't keep goes to a blur [the brightpass], which spreads it over about one triad.
    const MaskTile &mask = royaleMask();
    // Counted from the top left of the scene, so that the mask stays in place when the view scrolls.
    const int firstColumn = qRound(topLeft.x() * scale);
    const int firstRow = qRound(topLeft.y() * scale);
    const float amplify = 1 / ROYALE_MASK_AVERAGE;
    const float centerWeight = data.bloomCenterWeight;
    Planes brightpass;
    for (int color = 0; color < 3; color++) {
        brightpass[color].resize(outPixelCount);
    }
    for (int y = 0; y < outHeight; y++) {
        const float v = (y + 0.5f) / scale;
        const int maskRow = ((firstRow + y) % mask.size + mask.size) % mask.size;
        for (int x = 0; x < outWidth; x++) {
            const float u = (x + 0.5f) / scale;
            const int maskColumn = ((firstColumn + x) % mask.size + mask.size) % mask.size;
            const qsizetype i = qsizetype(y) * outWidth + x;
            for (int color = 0; color < 3; color++) {
                const float masked = screen[color][i] * mask.light[color][qsizetype(maskRow) * mask.size + maskColumn];
                const float intensity = masked * amplify;
                // The share of the light that goes to the blur: just enough that the pixel doesn't get brighter than 1,
                // assuming that its neighbors give all their light to the blur too.
                float share = 0;
                if (intensity > 0) {
                    const float fromNeighbors =
                        std::max(0.0f, interpolate(area[color], width, height, u, v) - centerWeight * intensity);
                    share = std::clamp(((1 - ROYALE_BLOOM_UNDERESTIMATE * fromNeighbors) / (ROYALE_BLOOM_UNDERESTIMATE * intensity) - 1)
                                           / (centerWeight - 1),
                                       0.0f, 1.0f);
                }
                screen[color][i] = masked;
                brightpass[color][i] = masked * share;
            }
        }
    }
    std::vector<float> blurredOutRows(outPixelCount);
    std::vector<float> bloom(outPixelCount);
    for (int color = 0; color < 3; color++) {
        blurRows(brightpass[color], blurredOutRows, outWidth, outHeight, data.bloomKernel);
        blurColumns(blurredOutRows, bloom, outWidth, outHeight, data.bloomKernel);
        for (qsizetype i = 0; i < outPixelCount; i++) {
            screen[color][i] = (screen[color][i] - brightpass[color][i] + bloom[i]) * amplify;
        }
    }

    // 4. Some light scatters in the glass. Then the light gets the gamma of the screen that shows it.
    for (int y = 0; y < outHeight; y++) {
        const float v = (y + 0.5f) / scale;
        QRgb *out = reinterpret_cast<QRgb *>(result.scanLine(y));
        for (int x = 0; x < outWidth; x++) {
            const float u = (x + 0.5f) / scale;
            QRgb pixel = 0xff000000;
            for (int color = 0; color < 3; color++) {
                const float onScreen = screen[color][qsizetype(y) * outWidth + x];
                const float scattered = interpolate(diffusion[color], width, height, u, v);
                const float mixed = onScreen + ROYALE_DIFFUSION_WEIGHT * (scattered - onScreen);
                const float value = std::pow(std::clamp(mixed, 0.0f, 1.0f), 1 / ROYALE_LCD_GAMMA);
                pixel |= QRgb(std::lround(value * 255)) << shiftOf(color);
            }
            out[x] = pixel;
        }
    }
    return result;
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

QImage CrtFilter::render(const QImage &image, QPoint topLeft, qreal devicePixelsPerKissPixel,
                         QKiSS::CrtSimulationPreset preset)
{
    Q_ASSERT(image.format() == QImage::Format_RGB32);
    switch (preset) {
    case QKiSS::CrtSimulationPreset::None:
        return image;
    case QKiSS::CrtSimulationPreset::Sharp:
    case QKiSS::CrtSimulationPreset::Soft:
        return renderRoyale(image, topLeft, devicePixelsPerKissPixel, preset);
    case QKiSS::CrtSimulationPreset::Glow:
        break;   // The rest of this function
    }
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

    // 2. The scanlines (see addScanlines()). Then come the phosphor stripes.
    const qsizetype outPixelCount = qsizetype(outWidth) * outHeight;
    Planes screen = addScanlines(rowLight, outWidth, height, outHeight, scale, preset);   // The light on the screen, before the bloom and the glow
    const int stripeWidth = CrtFilter::stripeWidth(scale);   // 0: no stripes
    const int firstColumn = qRound(topLeft.x() * scale);   // Counted from x = 0 of the scene, so the stripes stay in place
    // The stripes take away light of the other colors. Add it back, so that the picture keeps its brightness.
    const float stripeCompensation = CrtFilter::stripeCompensation(stripeWidth);
    for (int y = 0; y < outHeight; y++) {
        for (int x = 0; x < outWidth; x++) {
            int stripeColor = -1;   // Red, green or blue (0, 1, 2), or -1 without stripes
            if (stripeWidth > 0) {
                const int period = 3 * stripeWidth;
                stripeColor = ((firstColumn + x) % period + period) % period / stripeWidth;
            }
            for (int color = 0; color < 3; color++) {
                float &light = screen[color][qsizetype(y) * outWidth + x];
                if (stripeColor >= 0 && stripeColor != color) {
                    light *= STRIPE_OTHER_LIGHT;
                }
                light *= stripeCompensation;
            }
        }
    }

    // 3. Light above full brightness can't be shown. Like the bloom of CRT-Royale, it spreads to the pixels around it
    //    instead of being cut off. So bright areas keep their brightness, and the light fills the dark gaps between their
    //    scanlines and stripes.
    const std::vector<float> bloomKernel = CrtFilter::glowBloomKernel(scale);
    if (!bloomKernel.empty()) {
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

const CrtFilter::LookData &CrtFilter::lookData()
{
    static const LookData data = [] {
        LookData d;
        if constexpr (SVIDEO_RADIUS > 0) {
            d.sVideoKernel = sVideoKernel();
        }
        d.beamKernel.assign(BEAM_KERNEL.begin(), BEAM_KERNEL.end());
        d.glowKernel = glowKernel();
        d.glowAmount = GLOW_AMOUNT;
        d.srgbTable.assign(srgbTable().begin(), srgbTable().end());
        d.spotSigmaDark = SPOT_SIGMA_DARK;
        d.spotSigmaBright = SPOT_SIGMA_BRIGHT;
        d.gaussianCdf.assign(gaussianCdfTable().begin(), gaussianCdfTable().end());
        d.gaussianCdfMax = GAUSSIAN_CDF_MAX;
        d.scanlineLevels = SCANLINE_LEVELS;
        d.scanlineSteps = T_STEPS;
        d.scanlineTMax = T_MAX;
        const auto copyTable = [](const auto &table) {
            LookData::ScanlineTable t;
            t.alpha.assign(table.alpha.begin(), table.alpha.end());
            for (const auto &share : table.share) {
                t.share.insert(t.share.end(), share.begin(), share.end());
            }
            return t;
        };
        d.glowScanlines = copyTable(scanlineTable(QKiSS::CrtSimulationPreset::Glow));
        d.royaleScanlines = copyTable(scanlineTable(QKiSS::CrtSimulationPreset::Sharp));
        d.stripeOtherLight = STRIPE_OTHER_LIGHT;
        d.crtGamma = ROYALE_CRT_GAMMA;
        d.areaKernel = gaussianKernel(ROYALE_AREA_RADIUS, ROYALE_AREA_SIGMA);
        d.diffusionKernel = gaussianKernel(DIFFUSION_RADIUS);
        const MaskTile &mask = royaleMask();
        d.maskSize = mask.size;
        for (qsizetype i = 0; i < qsizetype(mask.size) * mask.size; i++) {
            for (const std::vector<float> &channel : mask.light) {
                d.mask.push_back(channel[i]);
            }
        }
        d.maskAverage = ROYALE_MASK_AVERAGE;
        d.bloomUnderestimate = ROYALE_BLOOM_UNDERESTIMATE;
        // The sigma of the blur of the bloom, in device pixels: just enough to blur a triad into one even color (within
        // 1/256) [get_min_sigma_to_blur_triad()]. CRT-Royale found this formula by fitting a curve.
        const float bloomSigma =
            -0.05168f + 0.6113f * ROYALE_TRIAD_SIZE - 1.122f * ROYALE_TRIAD_SIZE * std::sqrt(0.000416f + 1 / 256.0f);
        // The weight of the middle pixel of the blur in one direction [get_center_weight()], also from a fitted curve.
        // CRT-Royale uses it like this with its default settings. (Without runtime settings, it uses the square of this
        // weight instead: the weight in both directions. The curve also has a second part, which matters only for much
        // smaller blurs.)
        d.bloomCenterWeight = 0.399334576340352f / bloomSigma;
        d.bloomKernel = gaussianKernel(ROYALE_BLOOM_RADIUS, bloomSigma);
        d.diffusionWeight = ROYALE_DIFFUSION_WEIGHT;
        d.lcdGamma = ROYALE_LCD_GAMMA;
        return d;
    }();
    return data;
}

float CrtFilter::scanlineMiddle(float scale)
{
    // At a whole-number zoom, it is the middle of a row of device pixels, so that each scanline has a brightest row of
    // device pixels. (At 200%, the middle of the KiSS row would lie between its 2 rows of device pixels and light both
    // of them equally: no gaps at all.) GKissView passes an image that starts at a whole KiSS pixel, so the rows of
    // device pixels start at a KiSS row, and the scanlines stay in place.
    const float wholeScale = std::round(scale);
    return (std::abs(scale - wholeScale) < 0.001f) ? (std::floor((wholeScale - 1) / 2) + 0.5f) / wholeScale : 0.5f;
}

int CrtFilter::stripeWidth(float scale)
{
    // Each stripe is a whole number of device pixels, and the 3 colors are equally wide.
    // Otherwise one color would cover more device pixels than the others, and the whole picture would get its tint.
    return int(scale / 3);
}

float CrtFilter::stripeCompensation(int stripeWidth)
{
    return (stripeWidth > 0) ? 3 / (1 + 2 * STRIPE_OTHER_LIGHT) : 1;
}

std::vector<float> CrtFilter::glowBloomKernel(float scale)
{
    const int radius = int(std::ceil(3 * BLOOM_SIGMA * scale));   // In device pixels. gaussianKernel(): sigma = radius / 3
    return (radius > 0) ? gaussianKernel(radius) : std::vector<float>();
}
