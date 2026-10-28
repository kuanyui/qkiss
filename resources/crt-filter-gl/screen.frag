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

// The presets Sharp and Soft at device resolution, up to the bloom (renderRoyale(), the end of step 2 and step 3):
// - The light of each row spreads from the columns of KiSS pixels to the columns of device pixels: in an S curve for
//   Sharp, like CRT-Royale, or as our spots for Soft (with SOFT), like render() for Glow.
// - The slot mask lets only part of the light through.
// - The share of the light that a pixel can't keep goes to the bloom [the brightpass].
// It draws into two textures: the light that the pixel keeps (`kept`), and the light that goes to the bloom.
//
// Parts of this shader are ported from CRT-Royale, through renderRoyale() in src/crtfilter.cpp. CRT-Royale is a CRT
// shader for RetroArch, Copyright (C) 2014 TroggleMonkey <trogglemonkey@gmx.com>, under the GNU General Public
// License, version 2 or (at your option) any later version:
// https://github.com/libretro/slang-shaders/tree/master/crt/shaders/crt-royale (commit 901441a). The names in
// [brackets] are CRT-Royale's names.

// The scanlines: a column for each column of KiSS pixels, and a row for each row of device pixels. For Soft, after the
// beam blend.
uniform sampler2D scanlines;
uniform sampler2D area;        // The light around each pixel, at KiSS resolution, `kissSize` pixels big
uniform ivec2 kissSize;
uniform sampler2D mask;        // CrtFilter::LookData::mask, `maskSize` x `maskSize` pixels
uniform int maskSize;
uniform ivec2 maskOffset;      // The column and the row of the mask at the top left pixel: 0 ~ maskSize - 1
uniform float scale;           // Device pixels per KiSS pixel
uniform float amplify;         // 1 / CrtFilter::LookData::maskAverage
uniform float bloomUnderestimate;
uniform float centerWeight;    // CrtFilter::LookData::bloomCenterWeight
#ifdef SOFT
uniform sampler2D beam;        // The light after the beam blend, at KiSS resolution: the brightness of each spot
uniform sampler2D cdfTable;    // CrtFilter::LookData::gaussianCdf
uniform float cdfMax;          // CrtFilter::LookData::gaussianCdfMax
uniform float spotSigmaDark;   // CrtFilter::LookData::spotSigmaDark and spotSigmaBright
uniform float spotSigmaBright;
#endif

layout(location = 0) out vec4 kept;
layout(location = 1) out vec4 brightpass;

// The share of the light of one color that goes to the bloom: just enough that the pixel doesn't get brighter than 1,
// assuming that its neighbors give all their light to the bloom too.
float bloomShare(float intensity, float around)
{
    if (!(intensity > 0.0)) { return 0.0; }
    float fromNeighbors = max(0.0, around - centerWeight * intensity);
    return clamp(((1.0 - bloomUnderestimate * fromNeighbors) / (bloomUnderestimate * intensity) - 1.0)
                 / (centerWeight - 1.0), 0.0, 1.0);
}

void main()
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);
#ifdef SOFT
    // Soft: our beam blend and spots, as in render() for Glow (see spots.frag), but on the scanlines. The width of a
    // spot comes from the brightness of its pixel in the KiSS row of this row.
    float u = (float(pixel.x) + 0.5) / scale;   // In KiSS pixels. Pixel i covers u = i ~ i + 1.
    int row = min(int((float(pixel.y) + 0.5) / scale), kissSize.y - 1);
    int nearest = min(int(u), kissSize.x - 1);
    vec3 screen = vec3(0.0);
    for (int i = nearest - 2; i <= nearest + 2; i++) {
        if (i < 0 || i >= kissSize.x) { continue; }   // Outside the image, there are no spots.
        vec3 sigma = spotSigmaDark + (spotSigmaBright - spotSigmaDark) * texelFetch(beam, ivec2(i, row), 0).rgb;
        screen += texelFetch(scanlines, ivec2(i, pixel.y), 0).rgb * spotShare(cdfTable, cdfMax, u - float(i), sigma);
    }
#else
    // Sharp, like CRT-Royale [beam_horiz_filter]: between the middles of two KiSS pixels, the light fades from one to
    // the other in an S curve.
    float u = (float(pixel.x) + 0.5) / scale - 0.5;   // In KiSS pixels, from the middle of the first one
    float left = floor(u);
    float t = u - left;
    float rightWeight = t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
    int leftColumn = clamp(int(left), 0, kissSize.x - 1);
    int rightColumn = clamp(int(left) + 1, 0, kissSize.x - 1);
    vec3 screen = (1.0 - rightWeight) * texelFetch(scanlines, ivec2(leftColumn, pixel.y), 0).rgb
                + rightWeight * texelFetch(scanlines, ivec2(rightColumn, pixel.y), 0).rgb;
#endif

    vec3 masked = screen * texelFetch(mask, (maskOffset + pixel) % maskSize, 0).rgb;
    vec3 intensity = masked * amplify;
    vec3 around = interpolate(area, kissSize, (vec2(pixel) + 0.5) / scale);
    // Without a loop over the colors, and without reading `brightpass` back: the old version had both, and with it the
    // OpenGL driver of Intel UHD Graphics on Windows (10th gen Core, drivers 30 and 31) sent no light to the bloom.
    // Which of the two caused that is unknown.
    vec3 share = vec3(bloomShare(intensity.r, around.r), bloomShare(intensity.g, around.g),
                      bloomShare(intensity.b, around.b));
    vec3 bright = masked * share;
    brightpass = vec4(bright, 1.0);
    kept = vec4(masked - bright, 1.0);
}
