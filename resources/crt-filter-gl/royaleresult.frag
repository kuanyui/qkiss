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

// The end of the presets Sharp and Soft (renderRoyale(), the end of step 3 and step 4): the bloom along each column,
// then the light that scatters in the glass, then the gamma of the screen that shows the picture now. RADIUS is the
// radius of the bloom.
//
// Parts of this shader are ported from CRT-Royale, through renderRoyale() in src/crtfilter.cpp. CRT-Royale is a CRT
// shader for RetroArch, Copyright (C) 2014 TroggleMonkey <trogglemonkey@gmx.com>, under the GNU General Public
// License, version 2 or (at your option) any later version:
// https://github.com/libretro/slang-shaders/tree/master/crt/shaders/crt-royale (commit 901441a).

uniform sampler2D bloomRows;   // The light that goes to the bloom, blurred along each row
uniform sampler2D kept;        // The light that each pixel keeps
uniform ivec2 outSize;         // The size of `bloomRows`, `kept` and the result
uniform sampler2D diffusion;   // The light that scatters in the glass, at KiSS resolution, `kissSize` pixels big
uniform ivec2 kissSize;
uniform float bloomKernel[2 * RADIUS + 1];
uniform float scale;           // Device pixels per KiSS pixel
uniform float amplify;         // 1 / CrtFilter::LookData::maskAverage
uniform float diffusionWeight;
uniform float lcdGamma;

out vec4 color;

void main()
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    vec3 bloom = vec3(0.0);
    for (int k = 0; k <= 2 * RADIUS; k++) {
        bloom += bloomKernel[k] * fetch(bloomRows, pixel + ivec2(0, k - RADIUS), outSize);
    }
    vec3 onScreen = (texelFetch(kept, pixel, 0).rgb + bloom) * amplify;
    vec3 scattered = interpolate(diffusion, kissSize, (vec2(pixel) + 0.5) / scale);
    vec3 mixed = onScreen + diffusionWeight * (scattered - onScreen);
    vec3 value = pow(clamp(mixed, 0.0, 1.0), vec3(1.0 / lcdGamma));
    // Rounded to a whole step of 1/255 here, like std::lround() in CrtFilter. Then the framebuffer (8 bits per color)
    // stores exactly this value.
    color = vec4(floor(value * 255.0 + 0.5) / 255.0, 1.0);
}
