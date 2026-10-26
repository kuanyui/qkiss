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

// The video signal becomes light: videoSignal(), then the gamma of the CRT (step 1 of renderRoyale(), for Sharp and
// Soft). With SRGB, it becomes light like sRGB values instead (srgbToLinear() in computeBeamAndHalo(), for Glow and
// apply()). RADIUS is CrtFilter::SVIDEO_RADIUS.

uniform sampler2D image;   // At KiSS resolution, `size` pixels big
uniform ivec2 size;
#ifndef SRGB
uniform float crtGamma;
#endif
#if RADIUS > 0
uniform float kernel[2 * RADIUS + 1];   // The blur of the S-Video cable
#endif

out vec4 light;

void main()
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    vec3 signal = fetch(image, pixel, size);
#if RADIUS > 0
    // The cable carries the brightness (luma) and the color differences (red - luma, blue - luma), and only the color
    // differences lose detail: they are blurred along the row.
    const vec3 LUMA = vec3(0.299, 0.587, 0.114);
    float luma = dot(signal, LUMA);
    vec2 difference = vec2(0.0);
    for (int k = 0; k <= 2 * RADIUS; k++) {
        vec3 neighbor = fetch(image, pixel + ivec2(k - RADIUS, 0), size);
        difference += kernel[k] * (neighbor.rb - dot(neighbor, LUMA));
    }
    signal.rb = luma + difference;
    signal.g = (luma - 0.299 * signal.r - 0.114 * signal.b) / 0.587;
    // A color that bleeds into another color can come out more saturated than the screen can show. It is cut off.
    signal = clamp(signal, 0.0, 1.0);
#endif
#ifdef SRGB
    // srgbToLinear(): a straight line for dark values, a curve for the others
    vec3 dark = signal / 12.92;
    vec3 bright = pow((signal + 0.055) / 1.055, vec3(2.4));
    light = vec4(mix(dark, bright, greaterThan(signal, vec3(0.04045))), 1.0);
#else
    light = vec4(pow(signal, vec3(crtGamma)), 1.0);
#endif
}
