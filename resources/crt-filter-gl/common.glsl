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

#version 330 core

// CrtFilterGL puts this file in front of each fragment shader of the CRT Simulation, because GLSL has no #include.
// The shaders do the work of CrtFilter (src/crtfilter.cpp). Their comments name the functions of CrtFilter that do the
// same work, such as interpolate().
//
// Each shader draws one pixel of its result. ivec2(gl_FragCoord.xy) is the column and the row of that pixel, counted
// from the top left. (Row 0 of each texture is the top row, because CrtFilterGL uploads the image and reads the result
// back without turning them upside down.)
//
// The textures only grow, so they can be larger than the images in them. So the shaders get the size of each image,
// and they read only inside it.

/// The value of `image` at `pixel`. `image` is `size` pixels big. Outside of it, the pixel at the edge repeats, like
/// in blurRows().
vec3 fetch(sampler2D image, ivec2 pixel, ivec2 size)
{
    return texelFetch(image, clamp(pixel, ivec2(0), size - 1), 0).rgb;
}

/// interpolate(): the value of `image` (one pixel per KiSS pixel, `size` pixels big) at the point `point` in KiSS
/// pixels, interpolated between the 4 nearest pixels. The middle of the pixel in column i and row j is at
/// (i + 0.5, j + 0.5).
vec3 interpolate(sampler2D image, ivec2 size, vec2 point)
{
    vec2 position = clamp(point - 0.5, vec2(0.0), vec2(size - 1));
    ivec2 pixel0 = min(ivec2(position), size - 1);
    ivec2 pixel1 = min(pixel0 + 1, size - 1);
    vec2 fraction = position - vec2(pixel0);
    vec3 top = texelFetch(image, pixel0, 0).rgb * (1.0 - fraction.x)
             + texelFetch(image, ivec2(pixel1.x, pixel0.y), 0).rgb * fraction.x;
    vec3 bottom = texelFetch(image, ivec2(pixel0.x, pixel1.y), 0).rgb * (1.0 - fraction.x)
                + texelFetch(image, pixel1, 0).rgb * fraction.x;
    return top * (1.0 - fraction.y) + bottom * fraction.y;
}

/// linearToSrgb(): linear light (any value; it is cut off to 0.0 ~ 1.0) -> sRGB value. `table` is
/// CrtFilter::LookData::srgbTable, one row of 8-bit values. The light is rounded to one of its steps, like in
/// CrtFilter. The result is a whole step of 1/255, so the framebuffer (8 bits per color) stores exactly this value.
vec3 linearToSrgb(sampler2D table, vec3 light)
{
    int last = textureSize(table, 0).x - 1;
    ivec3 index = clamp(ivec3(light * float(last) + 0.5), 0, last);
    return vec3(texelFetch(table, ivec2(index.r, 0), 0).r, texelFetch(table, ivec2(index.g, 0), 0).r,
                texelFetch(table, ivec2(index.b, 0), 0).r);
}

/// gaussianCdf(): how much of a Gaussian with sigma 1 lies to the left of `t`: 0 ~ 1. `table` is
/// CrtFilter::LookData::gaussianCdf, one row of values for t = -tMax ~ tMax (CrtFilter::LookData::gaussianCdfMax).
float gaussianCdf(sampler2D table, float tMax, float t)
{
    int steps = textureSize(table, 0).x - 1;
    float position = (clamp(t, -tMax, tMax) + tMax) * (float(steps) / (2.0 * tMax));
    int i = min(int(position), steps - 1);
    float value0 = texelFetch(table, ivec2(i, 0), 0).r;
    float value1 = texelFetch(table, ivec2(i + 1, 0), 0).r;
    return value0 + (position - float(i)) * (value1 - value0);
}

/// The spots of Glow and Soft: each pixel is a spot of light, which spreads to the left and right like a Gaussian with
/// `sigma` (in KiSS pixels, one for each color). Returns the share of the light of the spot of the pixel in column i
/// that falls on the point u, where `distance` = u - i. `table` and `tMax` are for gaussianCdf().
vec3 spotShare(sampler2D table, float tMax, float distance, vec3 sigma)
{
    vec3 share;
    for (int color = 0; color < 3; color++) {
        share[color] = gaussianCdf(table, tMax, distance / sigma[color])
                     - gaussianCdf(table, tMax, (distance - 1.0) / sigma[color]);
    }
    return share;
}
