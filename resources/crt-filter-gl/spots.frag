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

// Step 1 of render() for Glow: each row of KiSS pixels, at each column of device pixels: the light of the spots of the
// pixels nearby. A spot is the pixel spread out by a Gaussian, so it adds the part of the Gaussian that falls on the
// column. A bright spot is wider, so it reaches into darker neighbors. The result has a row for each row of KiSS
// pixels, and a column for each column of device pixels.

uniform sampler2D beam;        // The light after the beam blend, at KiSS resolution, `kissSize` pixels big
uniform ivec2 kissSize;
uniform float scale;           // Device pixels per KiSS pixel
uniform sampler2D cdfTable;    // CrtFilter::LookData::gaussianCdf
uniform float cdfMax;          // CrtFilter::LookData::gaussianCdfMax
uniform float spotSigmaDark;   // CrtFilter::LookData::spotSigmaDark and spotSigmaBright
uniform float spotSigmaBright;

out vec4 light;

void main()
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);   // A column of device pixels, and a row of KiSS pixels
    float u = (float(pixel.x) + 0.5) / scale;   // In KiSS pixels. Pixel i covers u = i ~ i + 1.
    int nearest = min(int(u), kissSize.x - 1);
    vec3 sum = vec3(0.0);
    for (int i = nearest - 2; i <= nearest + 2; i++) {
        if (i < 0 || i >= kissSize.x) { continue; }   // Outside the image, there are no spots.
        vec3 value = texelFetch(beam, ivec2(i, pixel.y), 0).rgb;
        vec3 sigma = spotSigmaDark + (spotSigmaBright - spotSigmaDark) * value;
        sum += value * spotShare(cdfTable, cdfMax, u - float(i), sigma);
    }
    light = vec4(sum, 1.0);
}
