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

// blurRows() and blurColumns(): blurs `source` along each row (`direction` (1, 0)) or along each column (0, 1).
// Outside the image, the pixel at the edge repeats. RADIUS is the radius of the blur. With VARIABLE_RADIUS, RADIUS is
// the largest radius, and the uniform `radius` is the radius now. (The bloom of Glow needs this, because its radius
// depends on the zoom.)

uniform sampler2D source;   // `size` pixels big
uniform ivec2 size;
uniform ivec2 direction;
uniform float kernel[2 * RADIUS + 1];   // The first 2 * radius + 1 values
#ifdef VARIABLE_RADIUS
uniform int radius;
#else
const int radius = RADIUS;
#endif

out vec4 blurred;

void main()
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    vec3 sum = vec3(0.0);
    for (int k = 0; k <= 2 * radius; k++) {
        sum += kernel[k] * fetch(source, pixel + (k - radius) * direction, size);
    }
    blurred = vec4(sum, 1.0);
}
