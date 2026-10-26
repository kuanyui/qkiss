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

// The end of computeBeamAndHalo(), for Glow and apply(): the glow along each column, then the halo. The glow is the
// beam, blurred along each row (blur.frag) and then along each column (here). The halo is the glow minus the beam: the
// light that the glow adds to darker pixels. The glow only adds light, so the halo is never below 0. RADIUS is
// CrtFilter::GLOW_RADIUS.

uniform sampler2D blurredRows;   // The beam, blurred along each row, `size` pixels big
uniform sampler2D beam;          // The light after the beam blend, `size` pixels big
uniform ivec2 size;              // At KiSS resolution
uniform float kernel[2 * RADIUS + 1];   // CrtFilter::LookData::glowKernel

out vec4 halo;

void main()
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    vec3 glow = vec3(0.0);
    for (int k = 0; k <= 2 * RADIUS; k++) {
        glow += kernel[k] * fetch(blurredRows, pixel + ivec2(0, k - RADIUS), size);
    }
    halo = vec4(max(vec3(0.0), glow - texelFetch(beam, pixel, 0).rgb), 1.0);
}
