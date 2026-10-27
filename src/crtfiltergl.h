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

#ifndef CRTFILTERGL_H
#define CRTFILTERGL_H

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QImage>
#include <functional>
#include <memory>
#include "enums.h"

class QOffscreenSurface;
class QOpenGLContext;

/**
 * Does the work of CrtFilter::render() and CrtFilter::apply() on the graphics card, with OpenGL. This is much faster
 * than on the CPU.
 *
 * The first call checks whether OpenGL can be used, and writes the result to the log. By default, OpenGL is used only
 * when it has 3D acceleration and is at least version 3.3. (An OpenGL that runs on the CPU, such as llvmpipe of Mesa,
 * has no 3D acceleration.) It must also draw a small test picture almost like CrtFilter (see test()). Otherwise
 * render() returns a null image and apply() returns false, and CrtFilter does the work instead.
 *
 * The environment variable QKISS_RENDERER tells whether qkiss may use the graphics card. (Only the CRT Simulation uses
 * it.)
 * - Not set, or `auto`: OpenGL with 3D acceleration, otherwise CrtFilter.
 * - `opengl`: OpenGL, even when it runs on the CPU. For testing.
 * - `software`: always CrtFilter.
 *
 * qkiss uses OpenGL only here. So CrtFilterGL doesn't bring back another OpenGL context that was current before: after
 * each call, no context is current.
 */
class CrtFilterGL
{
    Q_DECLARE_TR_FUNCTIONS(CrtFilterGL)
public:
    CrtFilterGL();
    ~CrtFilterGL();
    /// Returns `image` as a CRT shows it, like CrtFilter::render() (the same arguments and the same result). Returns a
    /// null image when OpenGL can't do it.
    QImage render(const QImage &image, QPoint topLeft, qreal devicePixelsPerKissPixel,
                  QKiSS::CrtSimulationPreset preset);
    /// Filters `image` in place, like CrtFilter::apply(). Returns false, and leaves `image` unchanged, when OpenGL
    /// can't do it.
    bool apply(QImage &image);
    /// Frees the memory of the images on the graphics card (the framebuffers and the texture of the image), for
    /// example when the CRT Simulation is turned off. The next render() or apply() creates them again.
    void freeBuffers();

private:
    /// The shaders, textures and framebuffers, in crtfiltergl.cpp.
    struct Objects;

    /// The work of render() and apply(): uploads `image`, lets `drawSteps` draw the steps into the framebuffer of the
    /// result (`resultSize` big), and reads the result back. `drawSteps` returns false when OpenGL can't create a
    /// framebuffer. Returns a null image when OpenGL can't do the work. `name` is for the log.
    QImage run(const QImage &image, QSize resultSize, QLatin1StringView name,
               const std::function<bool(Objects &)> &drawSteps);
    /// Creates the OpenGL context and the objects in it. Returns false, and writes the reason to the log, when OpenGL
    /// can't be used.
    bool init();
    /// Draws a small test picture with each preset and with apply(), and compares the results with CrtFilter. Returns
    /// false, and writes the reason to the log, when OpenGL fails or draws a result that differs too much (for example,
    /// because the driver of the graphics card has compiled a shader wrongly).
    bool test();
    /// Makes the context current. When it can't, it stops using OpenGL and returns false.
    bool makeCurrent();
    /// Writes `problem` to the log, and deletes the context, so that CrtFilter does the work from now on.
    void stopUsingOpenGL(const QString &problem);
    /// Deletes the objects, the context and the surface. This frees their memory on the graphics card.
    void destroyContext();

    /// init() has run.
    bool m_checked = false;
    /// OpenGL can be used.
    bool m_usable = false;
    /// test() is running. Then run() writes no benchmark lines to the log.
    bool m_testing = false;
    /// The largest width (or height) of an image that OpenGL can draw into on this graphics card.
    int m_maxImageSize = 0;
    // m_surface comes before m_context, so that the context is destroyed first.
    std::unique_ptr<QOffscreenSurface> m_surface;
    std::unique_ptr<QOpenGLContext> m_context;
    /// They can be deleted only while m_context is current.
    std::unique_ptr<Objects> m_objects;
    /// Limits the benchmark lines in the log to one per second.
    QElapsedTimer m_benchmarkTimer;
};

#endif // CRTFILTERGL_H
