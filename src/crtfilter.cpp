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
/// The weights of the left, center and right pixel. The beam of a CRT is a little wider than one pixel, so each pixel
/// spreads into its neighbors on the same line. These weights blend a checkerboard of two colors into one even color.
constexpr std::array<float, 3> BEAM_KERNEL = { 0.25f, 0.5f, 0.25f };
/// How strongly a bright area glows into its darker neighbors: 0 means no glow.
constexpr float GLOW_AMOUNT = 0.15f;
/// How much of the other two colors a phosphor stripe lets through: 0 (none) ~ 255 (all, so no stripes).
constexpr int STRIPE_OTHER_COLORS = 190;
/// Before linear light becomes an sRGB value again, it is rounded to one of this many levels.
/// With 4096 levels, every sRGB value comes back unchanged, so flat areas keep their exact color.
constexpr int LINEAR_LEVELS = 4096;

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

/// Level of linear light (0 ~ LINEAR_LEVELS - 1) -> sRGB value (0 ~ 255).
const std::array<uint8_t, LINEAR_LEVELS> &linearToSrgb()
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
}

void CrtFilter::apply(QImage &image)
{
    Q_ASSERT(image.format() == QImage::Format_RGB32);
    if (image.isNull()) { return; }
    const int width = image.width();
    const int height = image.height();
    const qsizetype pixelCount = qsizetype(width) * height;
    const auto &toLinear = srgbToLinear();
    const auto &toSrgb = linearToSrgb();
    static const std::vector<float> glowKernel = gaussianKernel(GLOW_RADIUS);
    std::vector<float> channel(pixelCount);
    std::vector<float> beam(pixelCount);
    std::vector<float> blurredRows(pixelCount);
    std::vector<float> glow(pixelCount);
    for (const int shift : { 16, 8, 0 }) {   // Where red, green and blue are in a QRgb
        for (int y = 0; y < height; y++) {
            const QRgb *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
            for (int x = 0; x < width; x++) {
                channel[qsizetype(y) * width + x] = toLinear[(line[x] >> shift) & 0xff];
            }
        }
        blurRows(channel, beam, width, height, BEAM_KERNEL);
        blurRows(beam, blurredRows, width, height, glowKernel);
        blurColumns(blurredRows, glow, width, height, glowKernel);
        for (int y = 0; y < height; y++) {
            QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
            for (int x = 0; x < width; x++) {
                const qsizetype i = qsizetype(y) * width + x;
                // The glow only adds light. It brightens a pixel next to brighter pixels, and it never darkens a pixel.
                // So the glow doesn't change flat areas.
                const float light = beam[i] + GLOW_AMOUNT * std::max(0.0f, glow[i] - beam[i]);
                const int level = std::clamp(int(light * (LINEAR_LEVELS - 1) + 0.5f), 0, LINEAR_LEVELS - 1);
                line[x] = (line[x] & ~(0xffu << shift)) | (QRgb(toSrgb[level]) << shift);
            }
        }
    }
}

void CrtFilter::drawPhosphorStripes(QPainter *painter, const QRectF &rect, qreal devicePixelsPerKissPixel)
{
    // Each stripe is a whole number of device pixels, and the 3 colors are equally wide.
    // Otherwise one color would cover more device pixels than the others, and the whole picture would get its tint.
    const int stripeWidth = int(devicePixelsPerKissPixel / 3);
    if (stripeWidth < 1) { return; }
    static const QImage stripes = [] {
        constexpr int o = STRIPE_OTHER_COLORS;
        QImage image(3, 1, QImage::Format_RGB32);
        image.setPixel(0, 0, qRgb(255, o, o));
        image.setPixel(1, 0, qRgb(o, 255, o));
        image.setPixel(2, 0, qRgb(o, o, 255));
        return image;
    }();
    QBrush brush(stripes);
    // `stripes` has 1 pixel per stripe. Make that pixel `stripeWidth` device pixels wide, in KiSS pixels (scene units).
    brush.setTransform(QTransform::fromScale(stripeWidth / devicePixelsPerKissPixel, 1));
    painter->save();
    painter->setCompositionMode(QPainter::CompositionMode_Multiply);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, false);   // Sharp stripes
    painter->fillRect(rect, brush);
    painter->restore();
}
