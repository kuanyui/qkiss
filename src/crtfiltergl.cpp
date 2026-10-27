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

#include "crtfiltergl.h"
#include "crtfilter.h"
#include "logging.h"
#include <QFile>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPixelTransferOptions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QScopeGuard>
#include <algorithm>
#include <array>
#include <initializer_list>
#include <utility>
#include <vector>

using namespace Qt::Literals::StringLiterals;

namespace {
/// Parts of the names (GL_RENDERER) of OpenGL renderers that run on the CPU, without 3D acceleration.
constexpr std::array SOFTWARE_RENDERERS = {
    "llvmpipe", "softpipe", "swrast", "software rasterizer",   // Mesa
    "GDI Generic",   // Windows without a graphics driver
    "Microsoft Basic Render Driver",   // Windows (and WSL) without a graphics driver
    "SwiftShader",   // Google
};

/// The framebuffers between the steps hold light as 16-bit floats: light can be brighter than 1, and dark light keeps
/// its detail. (GL_RGBA32F is more exact, but it needs twice the memory.)
constexpr GLenum LIGHT_FORMAT = GL_RGBA16F;

/// The largest radius of the bloom of Glow, in device pixels. The radius depends on the zoom (see
/// CrtFilter::glowBloomKernel()): 8 at 800%, the largest zoom, or 29 on a screen with 4 device pixels per pixel. With a
/// larger radius, render() returns a null image.
constexpr int MAX_GLOW_BLOOM_RADIUS = 32;

/// The largest difference from CrtFilter that CrtFilterGL::test() accepts, in levels (0 ~ 255) of a color. The graphics
/// card rounds its numbers in other ways, and adds them up in another order, so its results can differ a little: tests
/// showed at most 1 level. A shader that a driver has compiled wrongly makes much larger differences.
constexpr int MAX_TEST_DIFFERENCE = 4;

/// Creates a shader program from fullscreen.vert and the fragment shader `name`, with common.glsl and `defines` in
/// front of it. (The shaders are in resources/crt-filter-gl/.) Returns null, and writes the reason to the log, when it
/// fails.
std::unique_ptr<QOpenGLShaderProgram> createProgram(const QString &name, const QByteArray &defines = {})
{
    const auto read = [](const QString &path) {
        QFile file(path);
        return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
    };
    // "#line 1" makes the error messages count the lines from the start of `name`.
    const QByteArray fragmentShader =
        read(u":/crt-filter-gl/common.glsl"_s) + defines + "#line 1\n" + read(":/crt-filter-gl/"_L1 + name);
    auto program = std::make_unique<QOpenGLShaderProgram>();
    if (program->addShaderFromSourceFile(QOpenGLShader::Vertex, u":/crt-filter-gl/fullscreen.vert"_s)
        && program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader) && program->link()) {
        return program;
    }
    // The name of the shader, and its defines, for example "blur.frag (RADIUS 3)"
    QString shader = name;
    if (!defines.isEmpty()) {
        shader += " ("_L1 + QString::fromLatin1(defines.trimmed()).remove(u"#define "_s).replace(u'\n', u", "_s) + u')';
    }
    qkWarning.noquote() << CrtFilterGL::tr("[CRT Simulation] The OpenGL shader %1 failed, so the CRT Simulation runs "
                                           "on the CPU:").arg(shader) + u'\n' + program->log();
    return nullptr;
}

/// The line "#define RADIUS ..." for a shader that blurs with `kernel`.
QByteArray radiusOf(const std::vector<float> &kernel)
{
    return "#define RADIUS " + QByteArray::number(int(kernel.size()) / 2) + "\n";
}

/// Creates a texture that holds `values`, which don't change. `format`, `pixelFormat` and `pixelType` tell what each
/// pixel holds, for example one float (QOpenGLTexture::R32F, Red, Float32).
std::unique_ptr<QOpenGLTexture> createTable(int width, int height, QOpenGLTexture::TextureFormat format,
                                            QOpenGLTexture::PixelFormat pixelFormat,
                                            QOpenGLTexture::PixelType pixelType, const void *values)
{
    auto texture = std::make_unique<QOpenGLTexture>(QOpenGLTexture::Target2D);
    texture->setSize(width, height);
    texture->setFormat(format);
    texture->allocateStorage(pixelFormat, pixelType);
    texture->setMinMagFilters(QOpenGLTexture::Nearest, QOpenGLTexture::Nearest);   // See `image` in run()
    texture->setData(pixelFormat, pixelType, values);
    return texture;
}

/// Makes `buffer` at least `size` big, with `textures` textures in the format `format`. A framebuffer only grows, like
/// the texture of `image` in run(). Returns false when OpenGL can't create it.
bool grow(std::unique_ptr<QOpenGLFramebufferObject> &buffer, QSize size, int textures = 1, GLenum format = LIGHT_FORMAT)
{
    if (buffer && buffer->width() >= size.width() && buffer->height() >= size.height()) { return true; }
    if (buffer) { size = size.expandedTo(buffer->size()); }
    buffer.reset();   // First, so that the old and the new framebuffer don't need memory at the same time
    buffer = std::make_unique<QOpenGLFramebufferObject>(size, QOpenGLFramebufferObject::NoAttachment, GL_TEXTURE_2D,
                                                        format);
    if (!buffer->isValid()) { return false; }
    if (textures == 1) { return true; }
    buffer->bind();   // addColorAttachment() adds the texture to the framebuffer that is bound.
    std::vector<GLenum> outputs = { GL_COLOR_ATTACHMENT0 };
    for (int i = 1; i < textures; i++) {
        buffer->addColorAttachment(size, format);
        outputs.push_back(GL_COLOR_ATTACHMENT0 + i);
    }
    // A shader draws its output `location = i` into texture i.
    QOpenGLExtraFunctions *gl = QOpenGLContext::currentContext()->extraFunctions();
    gl->glDrawBuffers(textures, outputs.data());
    return gl->glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

/// Sets the uniform `name` (an ivec2) of `program`, which must be bound. QOpenGLShaderProgram::setUniformValue() would
/// send a QSize or a QPoint as floats.
void setIvec2(QOpenGLShaderProgram &program, const char *name, int x, int y)
{
    QOpenGLContext::currentContext()->extraFunctions()->glUniform2i(program.uniformLocation(name), x, y);
}

/// Draws with `program` into the part of `buffer` that is `size` big. `inputs` are the textures that the shader reads,
/// with the names of their uniforms.
void draw(QOpenGLShaderProgram &program, QOpenGLFramebufferObject &buffer, QSize size,
          std::initializer_list<std::pair<const char *, GLuint>> inputs)
{
    QOpenGLExtraFunctions *gl = QOpenGLContext::currentContext()->extraFunctions();
    program.bind();
    int unit = 0;
    for (const auto &[name, texture] : inputs) {
        gl->glActiveTexture(GL_TEXTURE0 + unit);
        gl->glBindTexture(GL_TEXTURE_2D, texture);
        program.setUniformValue(name, unit);
        unit++;
    }
    buffer.bind();
    gl->glViewport(0, 0, size.width(), size.height());   // Only the part of the framebuffer that the result needs
    gl->glDrawArrays(GL_TRIANGLES, 0, 3);   // One triangle that covers the whole viewport. See fullscreen.vert.
}

/// The directions of blur()
constexpr QPoint ALONG_ROWS(1, 0);
constexpr QPoint ALONG_COLUMNS(0, 1);

/// Blurs `source` (`size` big) into `target` with `program` (blur.frag), along each row or along each column
/// (`direction`).
void blur(QOpenGLShaderProgram &program, GLuint source, QOpenGLFramebufferObject &target, QSize size, QPoint direction)
{
    program.bind();
    setIvec2(program, "size", size.width(), size.height());
    setIvec2(program, "direction", direction.x(), direction.y());
    draw(program, target, size, { { "source", source } });
}

/// The picture of CrtFilterGL::test(), 16 x 12 pixels. Each color of each pixel is one of LEVELS, from black to full
/// brightness, chosen at random (but the same each time). So it has dark, bright, saturated and mixed colors, and hard
/// edges, like a KiSS set.
QImage testPicture()
{
    static constexpr std::array LEVELS = { 0, 1, 10, 60, 128, 200, 254, 255 };
    quint32 random = 2463534242;
    const auto randomLevel = [&random] {
        // xorshift, a simple random generator
        random ^= random << 13;
        random ^= random >> 17;
        random ^= random << 5;
        return LEVELS[random % LEVELS.size()];
    };
    QImage picture(16, 12, QImage::Format_RGB32);
    for (int y = 0; y < picture.height(); y++) {
        for (int x = 0; x < picture.width(); x++) {
            const int red = randomLevel();
            const int green = randomLevel();
            const int blue = randomLevel();
            picture.setPixel(x, y, qRgb(red, green, blue));
        }
    }
    return picture;
}

/// The largest difference of a color (red, green or blue) between two pictures, in levels (0 ~ 255). Pictures of
/// different sizes differ by 255.
int largestDifference(const QImage &picture, const QImage &other)
{
    if (picture.size() != other.size()) { return 255; }
    int largest = 0;
    for (int y = 0; y < picture.height(); y++) {
        for (int x = 0; x < picture.width(); x++) {
            const QRgb a = picture.pixel(x, y);
            const QRgb b = other.pixel(x, y);
            largest = std::max({ largest, qAbs(qRed(a) - qRed(b)), qAbs(qGreen(a) - qGreen(b)),
                                 qAbs(qBlue(a) - qBlue(b)) });
        }
    }
    return largest;
}

/// `nanoseconds` in milliseconds, for the benchmark lines of the log
QString milliseconds(qint64 nanoseconds)
{
    return QString::number(nanoseconds / 1e6, 'f', 2);
}
}

struct CrtFilterGL::Objects
{
    /// A shader program for each step. Their fragment shaders are in resources/crt-filter-gl/.
    struct Programs
    {
        // video.frag: with the gamma of a CRT for Sharp and Soft, from sRGB values for Glow and apply()
        std::unique_ptr<QOpenGLShaderProgram> royaleVideo;
        std::unique_ptr<QOpenGLShaderProgram> glowVideo;
        // blur.frag, with the radius of each blur
        std::unique_ptr<QOpenGLShaderProgram> beamBlur;
        std::unique_ptr<QOpenGLShaderProgram> glowBlur;
        std::unique_ptr<QOpenGLShaderProgram> areaBlur;
        std::unique_ptr<QOpenGLShaderProgram> diffusionBlur;
        std::unique_ptr<QOpenGLShaderProgram> royaleBloomBlur;
        std::unique_ptr<QOpenGLShaderProgram> glowBloomBlur;   // Its radius depends on the zoom.
        std::unique_ptr<QOpenGLShaderProgram> halo;   // halo.frag
        std::unique_ptr<QOpenGLShaderProgram> spots;   // spots.frag
        std::unique_ptr<QOpenGLShaderProgram> scanlines;   // scanlines.frag
        std::unique_ptr<QOpenGLShaderProgram> sharpScreen;   // screen.frag, for Sharp and for Soft
        std::unique_ptr<QOpenGLShaderProgram> softScreen;
        std::unique_ptr<QOpenGLShaderProgram> stripes;   // stripes.frag
        std::unique_ptr<QOpenGLShaderProgram> royaleResult;   // royaleresult.frag
        std::unique_ptr<QOpenGLShaderProgram> glowResult;   // glowresult.frag
        std::unique_ptr<QOpenGLShaderProgram> applyResult;   // apply.frag
    } programs;
    // The tables of CrtFilter::LookData, which don't change
    struct ScanlineTable
    {
        std::unique_ptr<QOpenGLTexture> share;
        std::unique_ptr<QOpenGLTexture> alpha;
    };
    ScanlineTable glowScanlines;
    ScanlineTable royaleScanlines;
    std::unique_ptr<QOpenGLTexture> gaussianCdf;
    std::unique_ptr<QOpenGLTexture> srgbTable;
    std::unique_ptr<QOpenGLTexture> mask;
    /// `image` of run()
    std::unique_ptr<QOpenGLTexture> image;
    /// The framebuffers that the steps draw into, one after the other. The next steps read them. Each preset uses only
    /// some of them.
    struct Buffers
    {
        // At KiSS resolution
        std::unique_ptr<QOpenGLFramebufferObject> light;
        std::unique_ptr<QOpenGLFramebufferObject> blurredRows;   // Blurred along each row, before along each column
        std::unique_ptr<QOpenGLFramebufferObject> beam;   // The light after the beam blend: Soft, Glow and apply()
        std::unique_ptr<QOpenGLFramebufferObject> halo;   // Glow and apply()
        std::unique_ptr<QOpenGLFramebufferObject> area;   // Sharp and Soft
        std::unique_ptr<QOpenGLFramebufferObject> diffusion;   // Sharp and Soft
        // Glow: the light of the spots. A row for each row of KiSS pixels, a column for each column of device pixels.
        std::unique_ptr<QOpenGLFramebufferObject> spots;
        // The scanlines. A row for each row of device pixels, and a column for each column of KiSS pixels (Sharp and
        // Soft) or of device pixels (Glow).
        std::unique_ptr<QOpenGLFramebufferObject> scanlines;
        std::unique_ptr<QOpenGLFramebufferObject> blendedScanlines;   // Soft: the scanlines after the beam blend
        // At device resolution
        std::unique_ptr<QOpenGLFramebufferObject> screen;   // Two textures: kept, and brightpass or excess (Glow)
        std::unique_ptr<QOpenGLFramebufferObject> bloomRows;
        std::unique_ptr<QOpenGLFramebufferObject> result;   // 8 bits per color, like Format_RGB32. For apply(): at
                                                            // KiSS resolution.
    } buffers;

    /// The steps of CrtFilter::render() for Sharp and Soft (`preset`): `image` -> buffers.result. Returns false when
    /// OpenGL can't create a framebuffer.
    bool drawRoyale(QSize kissSize, QPoint topLeft, float scale, QKiSS::CrtSimulationPreset preset);
    /// The same for Glow, with the blur of the bloom `bloomKernel` (CrtFilter::glowBloomKernel()).
    bool drawGlow(QSize kissSize, QPoint topLeft, float scale, const std::vector<float> &bloomKernel);
    /// The same for CrtFilter::apply().
    bool drawApply(QSize kissSize);
    /// computeBeamAndHalo() in CrtFilter, for Glow and apply(): `image` -> buffers.beam and buffers.halo.
    bool drawBeamAndHalo(QSize kissSize);
    /// addScanlines() in CrtFilter: `light` (`lightSize` big) -> buffers.scanlines (`size` big), with the scanlines of
    /// `table`. buffers.scanlines must be large enough.
    void drawScanlines(GLuint light, QSize lightSize, QSize size, float scale, const ScanlineTable &table);
};

bool CrtFilterGL::Objects::drawRoyale(QSize kissSize, QPoint topLeft, float scale, QKiSS::CrtSimulationPreset preset)
{
    const bool soft = preset == QKiSS::CrtSimulationPreset::Soft;
    const int width = kissSize.width();
    const int height = kissSize.height();
    const QSize scanlinesSize(width, qRound(height * scale));
    const QSize outSize(qRound(width * scale), qRound(height * scale));
    if (!grow(buffers.light, kissSize) || !grow(buffers.blurredRows, kissSize) || !grow(buffers.area, kissSize)
        || !grow(buffers.diffusion, kissSize) || !grow(buffers.scanlines, scanlinesSize)
        || (soft && (!grow(buffers.beam, kissSize) || !grow(buffers.blendedScanlines, scanlinesSize)))
        || !grow(buffers.screen, outSize, 2) || !grow(buffers.bloomRows, outSize)
        || !grow(buffers.result, outSize, 1, GL_RGBA8)) {
        return false;
    }
    // The video signal becomes light, with the gamma of a CRT.
    programs.royaleVideo->bind();
    setIvec2(*programs.royaleVideo, "size", width, height);
    draw(*programs.royaleVideo, *buffers.light, kissSize, { { "image", image->textureId() } });
    // Two blurred copies of the light: the light around each pixel, and the light that scatters in the glass.
    blur(*programs.areaBlur, buffers.light->texture(), *buffers.blurredRows, kissSize, ALONG_ROWS);
    blur(*programs.areaBlur, buffers.blurredRows->texture(), *buffers.area, kissSize, ALONG_COLUMNS);
    blur(*programs.diffusionBlur, buffers.light->texture(), *buffers.blurredRows, kissSize, ALONG_ROWS);
    blur(*programs.diffusionBlur, buffers.blurredRows->texture(), *buffers.diffusion, kissSize, ALONG_COLUMNS);
    // The scanlines, in each column of KiSS pixels
    drawScanlines(buffers.light->texture(), kissSize, scanlinesSize, scale, royaleScanlines);
    if (soft) {
        // Soft blends the scanlines with our beam blend. The light after the beam blend tells how wide each spot is.
        blur(*programs.beamBlur, buffers.scanlines->texture(), *buffers.blendedScanlines, scanlinesSize, ALONG_ROWS);
        blur(*programs.beamBlur, buffers.light->texture(), *buffers.beam, kissSize, ALONG_ROWS);
    }
    // The light at device resolution, the mask, and the light that goes to the bloom
    QOpenGLShaderProgram &screen = soft ? *programs.softScreen : *programs.sharpScreen;
    screen.bind();
    setIvec2(screen, "kissSize", width, height);
    screen.setUniformValue("scale", scale);
    // Counted from the top left of the scene, like in CrtFilter, so that the mask stays in place when the view scrolls.
    // In GLSL, % is undefined for negative numbers, so the offset must not be negative.
    const int maskSize = CrtFilter::lookData().maskSize;
    const auto maskOffset = [maskSize](int scenePosition) { return (scenePosition % maskSize + maskSize) % maskSize; };
    setIvec2(screen, "maskOffset", maskOffset(qRound(topLeft.x() * scale)), maskOffset(qRound(topLeft.y() * scale)));
    if (soft) {
        draw(screen, *buffers.screen, outSize,
             { { "scanlines", buffers.blendedScanlines->texture() }, { "area", buffers.area->texture() },
               { "mask", mask->textureId() }, { "beam", buffers.beam->texture() },
               { "cdfTable", gaussianCdf->textureId() } });
    } else {
        draw(screen, *buffers.screen, outSize,
             { { "scanlines", buffers.scanlines->texture() }, { "area", buffers.area->texture() },
               { "mask", mask->textureId() } });
    }
    // The bloom along each row. Then the last step: the bloom along each column, the light that scatters in the glass,
    // and the gamma of the screen.
    const QList<GLuint> screenTextures = buffers.screen->textures();   // kept, brightpass
    blur(*programs.royaleBloomBlur, screenTextures.at(1), *buffers.bloomRows, outSize, ALONG_ROWS);
    programs.royaleResult->bind();
    setIvec2(*programs.royaleResult, "outSize", outSize.width(), outSize.height());
    setIvec2(*programs.royaleResult, "kissSize", width, height);
    programs.royaleResult->setUniformValue("scale", scale);
    draw(*programs.royaleResult, *buffers.result, outSize,
         { { "bloomRows", buffers.bloomRows->texture() }, { "kept", screenTextures.at(0) },
           { "diffusion", buffers.diffusion->texture() } });
    return true;
}

bool CrtFilterGL::Objects::drawGlow(QSize kissSize, QPoint topLeft, float scale, const std::vector<float> &bloomKernel)
{
    const int width = kissSize.width();
    const int height = kissSize.height();
    const QSize spotsSize(qRound(width * scale), height);
    const QSize outSize(qRound(width * scale), qRound(height * scale));
    if (!drawBeamAndHalo(kissSize) || !grow(buffers.spots, spotsSize) || !grow(buffers.scanlines, outSize)
        || !grow(buffers.screen, outSize, 2) || !grow(buffers.bloomRows, outSize)
        || !grow(buffers.result, outSize, 1, GL_RGBA8)) {
        return false;
    }
    // 1. Each row of KiSS pixels, at each column of device pixels: the light of the spots of the pixels nearby
    programs.spots->bind();
    setIvec2(*programs.spots, "kissSize", width, height);
    programs.spots->setUniformValue("scale", scale);
    draw(*programs.spots, *buffers.spots, spotsSize,
         { { "beam", buffers.beam->texture() }, { "cdfTable", gaussianCdf->textureId() } });
    // 2. The scanlines. Then come the phosphor stripes, and the light that goes to the bloom.
    drawScanlines(buffers.spots->texture(), spotsSize, outSize, scale, glowScanlines);
    const int stripeWidth = CrtFilter::stripeWidth(scale);
    // Counted from x = 0 of the scene, like in CrtFilter, so that the stripes stay in place when the view scrolls. In
    // GLSL, % is undefined for negative numbers, so the offset must not be negative.
    const int period = qMax(3 * stripeWidth, 1);
    programs.stripes->bind();
    programs.stripes->setUniformValue("stripeWidth", stripeWidth);
    programs.stripes->setUniformValue("stripeOffset", (qRound(topLeft.x() * scale) % period + period) % period);
    programs.stripes->setUniformValue("compensation", CrtFilter::stripeCompensation(stripeWidth));
    draw(*programs.stripes, *buffers.screen, outSize, { { "scanlines", buffers.scanlines->texture() } });
    // 3. The bloom along each row. Then the last step: the bloom along each column, the glow, and the sRGB values.
    //    (Without the bloom, the blur keeps the light above 1 as it is, and the sRGB values cut it off.)
    const std::vector<float> kernel = bloomKernel.empty() ? std::vector<float> { 1 } : bloomKernel;
    const int bloomRadius = int(kernel.size()) / 2;
    const QList<GLuint> screenTextures = buffers.screen->textures();   // kept, excess
    programs.glowBloomBlur->bind();
    programs.glowBloomBlur->setUniformValue("radius", bloomRadius);
    programs.glowBloomBlur->setUniformValueArray("kernel", kernel.data(), int(kernel.size()), 1);
    blur(*programs.glowBloomBlur, screenTextures.at(1), *buffers.bloomRows, outSize, ALONG_ROWS);
    programs.glowResult->bind();
    programs.glowResult->setUniformValue("bloomRadius", bloomRadius);
    programs.glowResult->setUniformValueArray("bloomKernel", kernel.data(), int(kernel.size()), 1);
    setIvec2(*programs.glowResult, "outSize", outSize.width(), outSize.height());
    setIvec2(*programs.glowResult, "kissSize", width, height);
    programs.glowResult->setUniformValue("scale", scale);
    draw(*programs.glowResult, *buffers.result, outSize,
         { { "bloomRows", buffers.bloomRows->texture() }, { "kept", screenTextures.at(0) },
           { "halo", buffers.halo->texture() }, { "srgbTable", srgbTable->textureId() } });
    return true;
}

bool CrtFilterGL::Objects::drawApply(QSize kissSize)
{
    if (!drawBeamAndHalo(kissSize) || !grow(buffers.result, kissSize, 1, GL_RGBA8)) { return false; }
    draw(*programs.applyResult, *buffers.result, kissSize,
         { { "beam", buffers.beam->texture() }, { "halo", buffers.halo->texture() },
           { "srgbTable", srgbTable->textureId() } });
    return true;
}

bool CrtFilterGL::Objects::drawBeamAndHalo(QSize kissSize)
{
    if (!grow(buffers.light, kissSize) || !grow(buffers.beam, kissSize) || !grow(buffers.blurredRows, kissSize)
        || !grow(buffers.halo, kissSize)) {
        return false;
    }
    // The video signal becomes light, like sRGB values.
    programs.glowVideo->bind();
    setIvec2(*programs.glowVideo, "size", kissSize.width(), kissSize.height());
    draw(*programs.glowVideo, *buffers.light, kissSize, { { "image", image->textureId() } });
    // The beam blend. Then the glow along each row, and the glow along each column with the halo.
    blur(*programs.beamBlur, buffers.light->texture(), *buffers.beam, kissSize, ALONG_ROWS);
    blur(*programs.glowBlur, buffers.beam->texture(), *buffers.blurredRows, kissSize, ALONG_ROWS);
    programs.halo->bind();
    setIvec2(*programs.halo, "size", kissSize.width(), kissSize.height());
    draw(*programs.halo, *buffers.halo, kissSize,
         { { "blurredRows", buffers.blurredRows->texture() }, { "beam", buffers.beam->texture() } });
    return true;
}

void CrtFilterGL::Objects::drawScanlines(GLuint light, QSize lightSize, QSize size, float scale,
                                         const ScanlineTable &table)
{
    programs.scanlines->bind();
    setIvec2(*programs.scanlines, "lightSize", lightSize.width(), lightSize.height());
    programs.scanlines->setUniformValue("scale", scale);
    programs.scanlines->setUniformValue("scanlineMiddle", CrtFilter::scanlineMiddle(scale));
    draw(*programs.scanlines, *buffers.scanlines, size,
         { { "light", light }, { "shareTable", table.share->textureId() }, { "alphaTable", table.alpha->textureId() } });
}

CrtFilterGL::CrtFilterGL() = default;

CrtFilterGL::~CrtFilterGL()
{
    destroyContext();
}

QImage CrtFilterGL::render(const QImage &image, QPoint topLeft, qreal devicePixelsPerKissPixel,
                           QKiSS::CrtSimulationPreset preset)
{
    const float scale = devicePixelsPerKissPixel;
    const QSize outSize(qRound(image.width() * scale), qRound(image.height() * scale));
    switch (preset) {
    case QKiSS::CrtSimulationPreset::None:
        return image;   // Like CrtFilter::render()
    case QKiSS::CrtSimulationPreset::Sharp:
    case QKiSS::CrtSimulationPreset::Soft:
        return run(image, outSize, (preset == QKiSS::CrtSimulationPreset::Sharp) ? "Sharp"_L1 : "Soft"_L1,
                   [&](Objects &objects) { return objects.drawRoyale(image.size(), topLeft, scale, preset); });
    case QKiSS::CrtSimulationPreset::Glow: {
        const std::vector<float> bloomKernel = CrtFilter::glowBloomKernel(scale);
        if (int(bloomKernel.size()) > 2 * MAX_GLOW_BLOOM_RADIUS + 1) { return QImage(); }   // Too wide for the shaders
        return run(image, outSize, "Glow"_L1,
                   [&](Objects &objects) { return objects.drawGlow(image.size(), topLeft, scale, bloomKernel); });
    }
    }
    return QImage();
}

bool CrtFilterGL::apply(QImage &image)
{
    QImage result = run(image, image.size(), "small zoom"_L1,
                        [&image](Objects &objects) { return objects.drawApply(image.size()); });
    if (result.isNull()) { return false; }
    image = std::move(result);
    return true;
}

void CrtFilterGL::freeBuffers()
{
    if (!m_usable || !makeCurrent()) { return; }
    // The shader programs and the tables are small, so they stay.
    m_objects->image.reset();
    m_objects->buffers = {};
    m_context->doneCurrent();
}

QImage CrtFilterGL::run(const QImage &image, QSize resultSize, QLatin1StringView name,
                        const std::function<bool(Objects &)> &drawSteps)
{
    Q_ASSERT(image.format() == QImage::Format_RGB32);
    if (!m_checked) {
        m_checked = true;
        QElapsedTimer timer;
        timer.start();
        m_usable = init();
        const qint64 prepared = timer.nsecsElapsed();
        // test() draws with render() and apply(), so it runs after m_usable is set.
        m_usable = m_usable && test();
        if (m_usable) {
            qkDebug.noquote() << QStringLiteral("[Benchmark] CRT Simulation with OpenGL: preparing %1 ms, test %2 ms")
                                 .arg(milliseconds(prepared), milliseconds(timer.nsecsElapsed() - prepared));
        } else {
            // Free what init() has created, for example the threads of an OpenGL that runs on the CPU.
            destroyContext();
        }
    }
    if (!m_usable || image.isNull()) { return QImage(); }
    // Too large for the graphics card. (The result is at least as large as `image`.)
    if (qMax(resultSize.width(), resultSize.height()) > m_maxImageSize) { return QImage(); }
    QImage result(resultSize, QImage::Format_RGB32);
    if (result.isNull()) { return QImage(); }
    // When OpenGL fails here, it will most likely fail again. So CrtFilter does the work from now on.
    if (!makeCurrent()) { return QImage(); }
    const auto doneCurrent = qScopeGuard([this] {
        if (m_context) { m_context->doneCurrent(); }   // stopUsingOpenGL() deletes it.
    });
    QOpenGLExtraFunctions *gl = m_context->extraFunctions();
    Objects &objects = *m_objects;
    QElapsedTimer timer;
    timer.start();

    // 1. Upload `image`. The texture only grows (like the framebuffers), so that a size that changes by a pixel, for
    //    example while scrolling, doesn't create a new texture every time.
    const int width = image.width();
    const int height = image.height();
    std::unique_ptr<QOpenGLTexture> &imageTexture = objects.image;
    if (!imageTexture || imageTexture->width() < width || imageTexture->height() < height) {
        QSize size = image.size();
        if (imageTexture) { size = size.expandedTo(QSize(imageTexture->width(), imageTexture->height())); }
        imageTexture = std::make_unique<QOpenGLTexture>(QOpenGLTexture::Target2D);
        imageTexture->setSize(size.width(), size.height());
        imageTexture->setFormat(QOpenGLTexture::RGBA8_UNorm);
        imageTexture->allocateStorage(QOpenGLTexture::BGRA, QOpenGLTexture::UInt32_RGBA8_Rev);
        // A new texture expects smaller copies of itself (mipmaps), which this one doesn't have. Without them, OpenGL
        // reads only black from it.
        imageTexture->setMinMagFilters(QOpenGLTexture::Nearest, QOpenGLTexture::Nearest);
    }
    // Format_RGB32 stores each pixel as the number 0xffRRGGBB. BGRA with UInt32_RGBA8_Rev reads such numbers, whatever
    // the byte order of the CPU.
    QOpenGLPixelTransferOptions rows;
    rows.setRowLength(image.bytesPerLine() / 4);   // In pixels, from the start of a row to the start of the next row
    imageTexture->setData(0, 0, 0, width, height, 1, QOpenGLTexture::BGRA, QOpenGLTexture::UInt32_RGBA8_Rev,
                          image.constBits(), &rows);
    const qint64 uploaded = timer.nsecsElapsed();

    // 2. Draw the steps of CrtFilter. Each step draws into its own framebuffer.
    if (!drawSteps(objects)) {
        stopUsingOpenGL(tr("OpenGL can't create a framebuffer (%1)").arg(name));
        return QImage();
    }
    // Wait until the graphics card has finished, so that the log can show the time of each step. This costs no time,
    // because glReadPixels() below would wait for it anyway.
    gl->glFinish();
    const qint64 drawn = timer.nsecsElapsed();

    // 3. Read the result back. OpenGL counts rows from the bottom, and QImage from the top. The shaders draw row 0 of
    //    the result at row 0 of OpenGL. glReadPixels() returns that row first, as the top row of the QImage. So the
    //    result is the right way up.
    objects.buffers.result->bind();
    gl->glReadPixels(0, 0, resultSize.width(), resultSize.height(), QOpenGLTexture::BGRA,
                     QOpenGLTexture::UInt32_RGBA8_Rev, result.bits());
    const qint64 readBack = timer.nsecsElapsed();
    // An error in any step above (for example, no memory for the texture, or a lost context) makes the result wrong.
    // OpenGL keeps an error until it is read, so one check here is enough.
    if (const GLenum error = gl->glGetError(); error != GL_NO_ERROR) {
        stopUsingOpenGL(tr("OpenGL error 0x%1 (%2)").arg(error, 4, 16, QChar(u'0')).arg(name));
        return QImage();
    }

    // At most one line per second, so that an animation doesn't fill the log. None for the test picture.
    if (!m_testing && (!m_benchmarkTimer.isValid() || m_benchmarkTimer.hasExpired(1000))) {
        m_benchmarkTimer.start();
        qkDebug.noquote() << QStringLiteral("[Benchmark] CRT Simulation (%1) with OpenGL, %2 x %3 -> %4 x %5 pixels: "
                                            "upload %6 ms, draw %7 ms, read back %8 ms")
                             .arg(name).arg(width).arg(height).arg(resultSize.width()).arg(resultSize.height())
                             .arg(milliseconds(uploaded), milliseconds(drawn - uploaded),
                                  milliseconds(readBack - drawn));
    }
    return result;
}

bool CrtFilterGL::init()
{
    const QString renderer = qEnvironmentVariable("QKISS_RENDERER");
    if (renderer == "software"_L1) {
        qkInfo.noquote() << tr("[CRT Simulation] QKISS_RENDERER is software, so the CRT Simulation runs on the CPU.");
        return false;
    }
    const bool forced = renderer == "opengl"_L1;
    if (!forced && !renderer.isEmpty() && renderer != "auto"_L1) {
        qkWarning.noquote() << tr("[CRT Simulation] QKISS_RENDERER is \"%1\", but it can only be auto, "
                                  "opengl or software. Using auto.").arg(renderer);
    }

    // OpenGL 3.3 has everything that the shaders need. Graphics cards since about 2010 have it. The core profile leaves
    // out the old functions.
    // With ResetNotification, OpenGL fails instead of drawing garbage after the graphics card has been reset (for
    // example by Windows, when the graphics card hangs). Then CrtFilter does the work. Some drivers can't create such a
    // context, so the second try is without it. Returns false when the context isn't OpenGL 3.3 or newer.
    const auto createContext = [this](bool resetNotification) {
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGL);
        format.setVersion(3, 3);
        format.setProfile(QSurfaceFormat::CoreProfile);
        format.setOption(QSurfaceFormat::ResetNotification, resetNotification);
        m_context = std::make_unique<QOpenGLContext>();
        m_context->setFormat(format);
        return m_context->create() && !m_context->isOpenGLES() && m_context->format().version() >= qMakePair(3, 3);
    };
    if (!createContext(true)) { createContext(false); }
    if (!m_context->isValid()) {
        qkInfo.noquote() << tr("[CRT Simulation] OpenGL is not available, so the CRT Simulation runs on the CPU.");
        return false;
    }
    // The system can give an older version than asked for.
    const QSurfaceFormat actualFormat = m_context->format();
    if (m_context->isOpenGLES() || actualFormat.version() < qMakePair(3, 3)) {
        qkInfo.noquote() << tr("[CRT Simulation] It needs OpenGL 3.3, but this system has %1 %2.%3, so the "
                               "CRT Simulation runs on the CPU.")
                            .arg(m_context->isOpenGLES() ? "OpenGL ES"_L1 : "OpenGL"_L1)
                            .arg(actualFormat.majorVersion()).arg(actualFormat.minorVersion());
        return false;
    }
    m_surface = std::make_unique<QOffscreenSurface>();
    m_surface->setFormat(actualFormat);
    m_surface->create();
    if (!m_surface->isValid() || !m_context->makeCurrent(m_surface.get())) {
        qkInfo.noquote() << tr("[CRT Simulation] OpenGL can't be used, so the CRT Simulation runs on the CPU.");
        return false;
    }
    const auto doneCurrent = qScopeGuard([this] { m_context->doneCurrent(); });
    QOpenGLExtraFunctions *gl = m_context->extraFunctions();
    const auto glString = [gl](GLenum name) {
        return QString::fromUtf8(reinterpret_cast<const char *>(gl->glGetString(name)));
    };
    const QString rendererName = glString(GL_RENDERER);
    qkInfo.noquote() << tr("[CRT Simulation] OpenGL renderer: %1. Version: %2.")
                        .arg(rendererName, glString(GL_VERSION));
    const bool runsOnCpu = std::ranges::any_of(SOFTWARE_RENDERERS, [&rendererName](const char *softwareRenderer) {
        return rendererName.contains(QLatin1StringView(softwareRenderer), Qt::CaseInsensitive);
    });
    if (runsOnCpu && !forced) {
        qkInfo.noquote() << tr("[CRT Simulation] This OpenGL renderer has no 3D acceleration, so the CRT Simulation "
                               "runs on the CPU without it. QKISS_RENDERER=opengl uses it anyway.");
        return false;
    }

    // An image that OpenGL draws into is a texture, and OpenGL draws into it through the viewport. Both have a limit.
    GLint maxTextureSize = 0;
    GLint maxViewportSize[2] = {};
    gl->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    gl->glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxViewportSize);
    m_maxImageSize = std::min({ maxTextureSize, maxViewportSize[0], maxViewportSize[1] });
    gl->glDisable(GL_DITHER);   // It could change the last bit of the result.
    // Here first: when something fails, they are deleted while the context is still current.
    auto objects = std::make_unique<Objects>();
    Objects::Programs &programs = objects->programs;
    const CrtFilter::LookData &data = CrtFilter::lookData();
    // createProgram() writes the reason to the log when a program fails. The others are still created, so that the log
    // shows every program that fails.
    bool created = true;
    const auto create = [&created](const QString &name, const QByteArray &defines = {}) {
        std::unique_ptr<QOpenGLShaderProgram> program = createProgram(name, defines);
        created = created && program;
        return program;
    };
    const QByteArray maxGlowBloomRadius = "#define RADIUS " + QByteArray::number(MAX_GLOW_BLOOM_RADIUS) + "\n";
    programs.royaleVideo = create(u"video.frag"_s, radiusOf(data.sVideoKernel));
    programs.glowVideo = create(u"video.frag"_s, radiusOf(data.sVideoKernel) + "#define SRGB\n");
    programs.beamBlur = create(u"blur.frag"_s, radiusOf(data.beamKernel));
    programs.glowBlur = create(u"blur.frag"_s, radiusOf(data.glowKernel));
    programs.areaBlur = create(u"blur.frag"_s, radiusOf(data.areaKernel));
    programs.diffusionBlur = create(u"blur.frag"_s, radiusOf(data.diffusionKernel));
    programs.royaleBloomBlur = create(u"blur.frag"_s, radiusOf(data.bloomKernel));
    programs.glowBloomBlur = create(u"blur.frag"_s, maxGlowBloomRadius + "#define VARIABLE_RADIUS\n");
    programs.halo = create(u"halo.frag"_s, radiusOf(data.glowKernel));
    programs.spots = create(u"spots.frag"_s);
    programs.scanlines = create(u"scanlines.frag"_s);
    programs.sharpScreen = create(u"screen.frag"_s);
    programs.softScreen = create(u"screen.frag"_s, "#define SOFT\n");
    programs.stripes = create(u"stripes.frag"_s);
    programs.royaleResult = create(u"royaleresult.frag"_s, radiusOf(data.bloomKernel));
    programs.glowResult = create(u"glowresult.frag"_s, maxGlowBloomRadius);
    programs.applyResult = create(u"apply.frag"_s);
    if (!created) { return false; }
    // The uniforms that don't change
    const auto setKernel = [](QOpenGLShaderProgram &program, const char *name, const std::vector<float> &kernel) {
        program.bind();
        program.setUniformValueArray(name, kernel.data(), int(kernel.size()), 1);
    };
    setKernel(*programs.royaleVideo, "kernel", data.sVideoKernel);
    programs.royaleVideo->setUniformValue("crtGamma", data.crtGamma);
    setKernel(*programs.glowVideo, "kernel", data.sVideoKernel);
    setKernel(*programs.beamBlur, "kernel", data.beamKernel);
    setKernel(*programs.glowBlur, "kernel", data.glowKernel);
    setKernel(*programs.areaBlur, "kernel", data.areaKernel);
    setKernel(*programs.diffusionBlur, "kernel", data.diffusionKernel);
    setKernel(*programs.royaleBloomBlur, "kernel", data.bloomKernel);
    setKernel(*programs.halo, "kernel", data.glowKernel);
    for (QOpenGLShaderProgram *program : { programs.spots.get(), programs.softScreen.get() }) {   // The spots
        program->bind();
        program->setUniformValue("cdfMax", data.gaussianCdfMax);
        program->setUniformValue("spotSigmaDark", data.spotSigmaDark);
        program->setUniformValue("spotSigmaBright", data.spotSigmaBright);
    }
    programs.scanlines->bind();
    programs.scanlines->setUniformValue("levels", data.scanlineLevels);
    programs.scanlines->setUniformValue("steps", data.scanlineSteps);
    programs.scanlines->setUniformValue("tMax", data.scanlineTMax);
    const float amplify = 1 / data.maskAverage;
    for (QOpenGLShaderProgram *program : { programs.sharpScreen.get(), programs.softScreen.get() }) {
        program->bind();
        program->setUniformValue("maskSize", data.maskSize);
        program->setUniformValue("amplify", amplify);
        program->setUniformValue("bloomUnderestimate", data.bloomUnderestimate);
        program->setUniformValue("centerWeight", data.bloomCenterWeight);
    }
    programs.stripes->bind();
    programs.stripes->setUniformValue("otherLight", data.stripeOtherLight);
    setKernel(*programs.royaleResult, "bloomKernel", data.bloomKernel);
    programs.royaleResult->setUniformValue("amplify", amplify);
    programs.royaleResult->setUniformValue("diffusionWeight", data.diffusionWeight);
    programs.royaleResult->setUniformValue("lcdGamma", data.lcdGamma);
    for (QOpenGLShaderProgram *program : { programs.glowResult.get(), programs.applyResult.get() }) {
        program->bind();
        program->setUniformValue("glowAmount", data.glowAmount);
    }
    // The tables
    const auto createScanlineTable = [&data](const CrtFilter::LookData::ScanlineTable &table) {
        Objects::ScanlineTable textures;
        textures.share = createTable(data.scanlineSteps + 1, data.scanlineLevels + 1, QOpenGLTexture::R32F,
                                     QOpenGLTexture::Red, QOpenGLTexture::Float32, table.share.data());
        textures.alpha = createTable(data.scanlineLevels + 1, 1, QOpenGLTexture::R32F, QOpenGLTexture::Red,
                                     QOpenGLTexture::Float32, table.alpha.data());
        return textures;
    };
    objects->glowScanlines = createScanlineTable(data.glowScanlines);
    objects->royaleScanlines = createScanlineTable(data.royaleScanlines);
    objects->gaussianCdf = createTable(int(data.gaussianCdf.size()), 1, QOpenGLTexture::R32F, QOpenGLTexture::Red,
                                       QOpenGLTexture::Float32, data.gaussianCdf.data());
    // 8 bits each: OpenGL reads value k as k / 255, which the framebuffer of the result stores as k again.
    objects->srgbTable = createTable(int(data.srgbTable.size()), 1, QOpenGLTexture::R8_UNorm, QOpenGLTexture::Red,
                                     QOpenGLTexture::UInt8, data.srgbTable.data());
    objects->mask = createTable(data.maskSize, data.maskSize, QOpenGLTexture::RGB32F, QOpenGLTexture::RGB,
                                QOpenGLTexture::Float32, data.mask.data());
    // OpenGL can't draw without a vertex array, even an empty one. This context is only for CrtFilterGL, so the vertex
    // array stays bound, and it is deleted with the context.
    GLuint vertexArray = 0;
    gl->glGenVertexArrays(1, &vertexArray);
    gl->glBindVertexArray(vertexArray);
    if (const GLenum error = gl->glGetError(); error != GL_NO_ERROR) {
        qkWarning.noquote() << tr("[CRT Simulation] OpenGL error 0x%1 while preparing, so the CRT Simulation "
                                  "runs on the CPU.").arg(error, 4, 16, QChar(u'0'));
        return false;
    }
    m_objects = std::move(objects);
    return true;
}

bool CrtFilterGL::test()
{
    m_testing = true;
    const auto testDone = qScopeGuard([this] { m_testing = false; });
    const QImage picture = testPicture();
    // Zoom 3 uses every shader program. A negative top left (like at the top left of the scene) moves the mask and the
    // stripes.
    const QPoint topLeft(-5, -3);
    constexpr qreal zoom = 3;
    int largest = 0;   // Of all results
    // Compares the result of `name` with the result of CrtFilter. Returns false, and writes the reason to the log, when
    // it differs too much.
    const auto check = [&largest](QLatin1StringView name, const QImage &result, const QImage &expected) {
        if (result.isNull()) { return false; }   // When OpenGL fails, run() writes the reason to the log.
        const int difference = largestDifference(result, expected);
        if (difference > MAX_TEST_DIFFERENCE) {
            qkWarning.noquote() << tr("[CRT Simulation] OpenGL draws the test picture of %1 wrongly (a "
                                      "color differs from the CPU by %2 of 255 levels), so the CRT "
                                      "Simulation runs on the CPU.").arg(name).arg(difference);
            return false;
        }
        largest = std::max(largest, difference);
        return true;
    };
    for (const auto &[preset, name] : { std::pair(QKiSS::CrtSimulationPreset::Sharp, "Sharp"_L1),
                                        std::pair(QKiSS::CrtSimulationPreset::Soft, "Soft"_L1),
                                        std::pair(QKiSS::CrtSimulationPreset::Glow, "Glow"_L1) }) {
        if (!check(name, render(picture, topLeft, zoom, preset), CrtFilter::render(picture, topLeft, zoom, preset))) {
            return false;
        }
    }
    QImage applied = picture;
    QImage expected = picture;
    CrtFilter::apply(expected);
    if (!check("small zoom"_L1, apply(applied) ? applied : QImage(), expected)) { return false; }
    qkInfo.noquote() << tr("[CRT Simulation] OpenGL draws the test picture correctly (the colors differ "
                           "from the CPU by at most %1 of 255 levels), so the CRT Simulation runs on the "
                           "graphics card.").arg(largest);
    return true;
}

bool CrtFilterGL::makeCurrent()
{
    if (m_context->makeCurrent(m_surface.get())) { return true; }
    // isValid() becomes false when the context is lost, for example after the graphics card has been reset. See init().
    stopUsingOpenGL(m_context->isValid()
                        ? tr("OpenGL stopped working")
                        : tr("OpenGL lost its context (for example, after a reset of the graphics card)"));
    return false;
}

void CrtFilterGL::stopUsingOpenGL(const QString &problem)
{
    qkWarning.noquote() << tr("[CRT Simulation] %1, so the CRT Simulation runs on the CPU from now on.")
                           .arg(problem);
    destroyContext();
}

void CrtFilterGL::destroyContext()
{
    m_usable = false;
    if (m_objects) {
        // The objects can be deleted only while their context is current. When it can't be made current (for example,
        // after the graphics card has been reset), Qt warns that it can't delete the textures, and deleting the context
        // frees them.
        // A context that is current already isn't made current again. Qt marks a context as lost (isValid() false)
        // only in makeCurrent(), or on Wayland between events when the compositor restarts. doneCurrent() does nothing
        // for a lost context, so QOpenGLContext::currentContext() would still return it after it is deleted. A context
        // that is current here has passed makeCurrent() in the same call, without events in between, so it isn't
        // marked as lost.
        if (QOpenGLContext::currentContext() != m_context.get()) { m_context->makeCurrent(m_surface.get()); }
        m_objects.reset();
    }
    m_context.reset();
    m_surface.reset();
}
