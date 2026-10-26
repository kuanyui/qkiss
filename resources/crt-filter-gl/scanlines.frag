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

// addScanlines() with scanlineShare(): the scanlines in each column of `light`. The result has the columns of `light`,
// and a row for each row of device pixels. Each row adds the light of the scanlines of the KiSS rows above, its own row
// and the row below: the part of each scanline's light that falls between the top and the bottom of the row.

// A row for each row of KiSS pixels, and a column for each column of KiSS pixels (Sharp and Soft) or of device pixels
// (Glow). `lightSize` pixels big.
uniform sampler2D light;
uniform ivec2 lightSize;
// The table of the preset (CrtFilter::LookData::ScanlineTable). `share` has a row for each level and a column for each
// step. `alpha` has one row, with a column for each level.
uniform sampler2D shareTable;
uniform sampler2D alphaTable;
uniform int levels;             // CrtFilter::LookData::scanlineLevels
uniform int steps;              // CrtFilter::LookData::scanlineSteps
uniform float tMax;             // CrtFilter::LookData::scanlineTMax
uniform float scale;            // Device pixels per KiSS pixel
uniform float scanlineMiddle;   // CrtFilter::scanlineMiddle()

out vec4 scanlines;

/// scanlineShare(): the share of a scanline's light that lies between its middle and `distance` rows of KiSS pixels
/// below it. `level` is the brightness of the scanline (0 ~ levels), and `alpha` is its width.
float scanlineShare(int level, float alpha, float distance)
{
    float t = abs(distance) / alpha * (float(steps) / tMax);   // In steps of the table
    float share = 0.5;
    if (t < float(steps)) {
        int step = int(t);
        float share0 = texelFetch(shareTable, ivec2(step, level), 0).r;
        float share1 = texelFetch(shareTable, ivec2(step + 1, level), 0).r;
        share = share0 + (t - float(step)) * (share1 - share0);
    }
    return (distance < 0.0) ? -share : share;
}

/// The light of a scanline with the brightness `level` that falls on a row of device pixels, as a share of its
/// brightness, divided by the height of the row (`rowHeight`, in rows of KiSS pixels). The middle of the row is
/// `distance` rows of KiSS pixels below the middle of the scanline.
float weight(int level, float distance, float rowHeight)
{
    float alpha = texelFetch(alphaTable, ivec2(level, 0), 0).r;
    return (scanlineShare(level, alpha, distance + rowHeight / 2.0)
            - scanlineShare(level, alpha, distance - rowHeight / 2.0)) / rowHeight;
}

void main()
{
    ivec2 pixel = ivec2(gl_FragCoord.xy);   // A column of `light`, and a row of device pixels
    float rowHeight = 1.0 / scale;   // In rows of KiSS pixels
    float v = (float(pixel.y) + 0.5) / scale;   // In KiSS pixels. Row j covers v = j ~ j + 1.
    int row = min(int(v), lightSize.y - 1);
    vec3 sum = vec3(0.0);
    for (int scanline = row - 1; scanline <= row + 1; scanline++) {
        if (scanline < 0 || scanline >= lightSize.y) { continue; }
        float distance = v - (float(scanline) + scanlineMiddle);
        vec3 rowValue = texelFetch(light, ivec2(pixel.x, scanline), 0).rgb;
        for (int color = 0; color < 3; color++) {
            // Interpolated between the 2 nearest levels
            float level = clamp(rowValue[color], 0.0, 1.0) * float(levels);
            int level0 = min(int(level), levels - 1);
            float weight0 = weight(level0, distance, rowHeight);
            float weight1 = weight(level0 + 1, distance, rowHeight);
            sum[color] += rowValue[color] * (weight0 + (level - float(level0)) * (weight1 - weight0));
        }
    }
    scanlines = vec4(sum, 1.0);
}
