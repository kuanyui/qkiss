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

// The end of the preset Glow (render(), the end of step 3 and step 4): the bloom along each column, then the glow, then
// linear light becomes sRGB values. RADIUS is the largest radius of the bloom, and `bloomRadius` the radius now, which
// depends on the zoom.

uniform sampler2D bloomRows;   // The light that goes to the bloom, blurred along each row
uniform sampler2D kept;        // The light that each pixel keeps
uniform ivec2 outSize;         // The size of `bloomRows`, `kept` and the result
uniform float bloomKernel[2 * RADIUS + 1];   // CrtFilter::glowBloomKernel(): the first 2 * bloomRadius + 1 values
uniform int bloomRadius;
uniform sampler2D halo;        // The glow that the beam adds to darker pixels, `kissSize` pixels big
uniform ivec2 kissSize;
uniform float scale;           // Device pixels per KiSS pixel
uniform float glowAmount;      // CrtFilter::LookData::glowAmount
uniform sampler2D srgbTable;   // CrtFilter::LookData::srgbTable

out vec4 color;

void main()
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    vec3 bloom = vec3(0.0);
    for (int k = 0; k <= 2 * bloomRadius; k++) {
        bloom += bloomKernel[k] * fetch(bloomRows, pixel + ivec2(0, k - bloomRadius), outSize);
    }
    // The glow comes last, so the scanlines and the stripes don't cover it.
    vec3 light = texelFetch(kept, pixel, 0).rgb + bloom
               + glowAmount * interpolate(halo, kissSize, (vec2(pixel) + 0.5) / scale);
    color = vec4(linearToSrgb(srgbTable, light), 1.0);
}
