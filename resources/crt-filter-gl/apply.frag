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

// CrtFilter::apply(), for a small zoom: the light after the beam blend, with the glow, as sRGB values. At KiSS
// resolution.

uniform sampler2D beam;        // The light after the beam blend
uniform sampler2D halo;        // The glow that the beam adds to darker pixels
uniform float glowAmount;      // CrtFilter::LookData::glowAmount
uniform sampler2D srgbTable;   // CrtFilter::LookData::srgbTable

out vec4 color;

void main()
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    vec3 light = texelFetch(beam, pixel, 0).rgb + glowAmount * texelFetch(halo, pixel, 0).rgb;
    color = vec4(linearToSrgb(srgbTable, light), 1.0);
}
