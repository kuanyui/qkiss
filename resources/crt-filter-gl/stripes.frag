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

// The phosphor stripes of render() for Glow (the end of step 2), and the light that goes to the bloom (step 3). Light
// above full brightness can't be shown, so it goes to the bloom, which spreads it to the pixels around it.
// It draws into two textures: the light that the pixel keeps (`kept`, at most 1), and the light above 1 (`excess`).

uniform sampler2D scanlines;   // The light on the screen, at device resolution
uniform int stripeWidth;       // CrtFilter::stripeWidth(), in device pixels. 0: no stripes
uniform int stripeOffset;      // The column of the stripes at the left pixel: 0 ~ 3 * stripeWidth - 1
uniform float otherLight;      // CrtFilter::LookData::stripeOtherLight
uniform float compensation;    // CrtFilter::stripeCompensation()

layout(location = 0) out vec4 kept;
layout(location = 1) out vec4 excess;

void main()
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    vec3 light = texelFetch(scanlines, pixel, 0).rgb;
    if (stripeWidth > 0) {
        // Red, green or blue (0, 1, 2). Each stripe lets through all the light of its own color.
        int stripeColor = (stripeOffset + pixel.x) % (3 * stripeWidth) / stripeWidth;
        vec3 share = vec3(otherLight);
        share[stripeColor] = 1.0;
        light *= share;
    }
    light *= compensation;
    excess = vec4(max(vec3(0.0), light - 1.0), 1.0);
    kept = vec4(light - excess.rgb, 1.0);
}
