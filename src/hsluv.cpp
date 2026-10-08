// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * HSLuv-C: Human-friendly HSL
 *
 * Authors:
 *   2015 Alexei Boronine (original idea, JavaScript implementation)
 *   2015 Roger Tallada (Obj-C implementation)
 *   2017 Martin Mitas (C implementation, based on Obj-C implementation)
 *   2021 Massinissa Derriche (C++ implementation for Inkscape, based on C implementation)
 *
 * Copyright (C) 2021 Authors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "hsluv.h"

#include <iostream>
#include <limits>
#include <cmath>
#include <algorithm>
#include <2geom/line.h>
#include <2geom/ray.h>

namespace Hsluv {

/* for sRGB, reference white D65 */
static const Triplet m[3] = {
    {  3.24096994190452134377, -1.53738317757009345794, -0.49861076029300328366 },
    { -0.96924363628087982613,  1.87596750150772066772,  0.04155505740717561247 },
    {  0.05563007969699360846, -0.20397695888897656435,  1.05697151424287856072 }
};

/* for XYZ */
static const Triplet m_inv[3] = {
    {  0.41239079926595948129,  0.35758433938387796373,  0.18048078840183428751 },
    {  0.21263900587151035754,  0.71516867876775592746,  0.07219231536073371500 },
    {  0.01933081871559185069,  0.11919477979462598791,  0.95053215224966058086 }
};

static const double REF_U = 0.19783000664283680764;
static const double REF_V = 0.46831999493879100370;

// CIE LUV constants
static const double KAPPA = 903.29629629629629629630;
static const double EPSILON = 0.00885645167903563082;

/**
 * Calculate the bounds of the Luv colors in RGB gamut.
 *
 * @param l Lightness. Between 0.0 and 100.0.
 * @return Bounds of Luv colors in RGB gamut.
 */
std::array<Geom::Line, 6> get_bounds(double l)
{
    std::array<Geom::Line, 6> bounds;

    double tl = l + 16.0;
    double sub1 = (tl * tl * tl) / 1560896.0;
    double sub2 = (sub1 > EPSILON ? sub1 : (l / KAPPA));
    int channel;
    int t;

    for(channel = 0; channel < 3; channel++) {
        double m1 = m[channel][0];
        double m2 = m[channel][1];
        double m3 = m[channel][2];

        for (t = 0; t < 2; t++) {
            double top1 = (284517.0 * m1 - 94839.0 * m3) * sub2;
            double top2 = (838422.0 * m3 + 769860.0 * m2 + 731718.0 * m1) * l * sub2 -  769860.0 * t * l;
            double bottom = (632260.0 * m3 - 126452.0 * m2) * sub2 + 126452.0 * t;

            bounds[channel * 2 + t].setCoefficients(top1, -bottom, top2);
        }
    }

    return bounds;
}

/**
 * Calculate the maximum in gamut chromaticity for the given luminance and hue.
 *
 * @param l Luminance.
 * @param h Hue.
 * @return The maximum chromaticity.
 */
static double max_chroma_for_lh(double l, double h)
{
    double min_len = std::numeric_limits<double>::max();
    auto const ray = Geom::Ray(Geom::Point(0, 0), Geom::rad_from_deg(h));

    for (auto const &line : get_bounds(l)) {
        auto intersections = line.intersect(ray);
        if (intersections.empty()) {
            continue;
        }
        double len = intersections[0].point().length();

        if (len >= 0 && len < min_len) {
            min_len = len;
        }
    }

    return min_len;
}

/**
 * Calculate the dot product of the given arrays.
 *
 * @param t1 The first array.
 * @param t2 The second array.
 * @return The resulting dot product.
 */
static double dot_product(const Triplet &t1, const Triplet &t2)
{
    return (t1[0] * t2[0] + t1[1] * t2[1] + t1[2] * t2[2]);
}

/**
 * Convenience function used for RGB conversions.
 *
 * @param c Value.
 * @return RGB color component.
 */
double from_linear(double c)
{
    if (c <= 0.0031308) {
        return 12.92 * c;
    } else {
        return 1.055 * std::pow(c, 1.0 / 2.4) - 0.055;
    }
}

/**
 * Convenience function used for RGB conversions.
 *
 * @param c Value.
 * @return XYZ color component.
 */
double to_linear(double c)
{
    if (c > 0.04045) {
        return std::pow((c + 0.055) / 1.055, 2.4);
    } else {
        return c / 12.92;
    }
}

/**
 * @overload
 * @param t RGB color components.
 * @return XYZ color components.
 */
static Triplet to_linear(const Triplet &t)
{
    return {
        to_linear(t[0]),
        to_linear(t[1]),
        to_linear(t[2])
    };
}

/**
 * Convert a color from the the XYZ colorspace to the RGB colorspace.
 *
 * @param in_out[in,out] The XYZ color converted to a RGB color.
 */
static void xyz2rgb(Triplet &in_out)
{
    Triplet result;
    for (size_t i : {0, 1, 2}) {
        result[i] = from_linear(dot_product(m[i], in_out));
    }
    in_out = result;
}

/**
 * Convert a color from the the RGB colorspace to the XYZ colorspace.
 *
 * @param in_out[in,out] The RGB color converted to a XYZ color.
 */
static void rgb2xyz(Triplet &in_out)
{
    Triplet rgbl = to_linear(in_out);
    for (size_t i : {0, 1, 2}) {
        in_out[i] = dot_product(m_inv[i], rgbl);
    }
}

/**
 * Utility function used to convert from the XYZ colorspace to CIELuv.
 * https://en.wikipedia.org/wiki/CIELUV
 *
 * @param y Y component of the XYZ color.
 * @return Luminance component of Luv color.
 */
static double y2l(double y)
{
    if (y <= EPSILON)
        return y * KAPPA;
    else
        return 116.0 * std::cbrt(y) - 16.0;
}

/**
 * Utility function used to convert from CIELuv colorspace to XYZ.
 *
 * @param l Luminance component of Luv color.
 * @return Y component of the XYZ color.
 */
static double l2y(double l)
{
    if (l <= 8.0) {
        return l / KAPPA;
    } else {
        double x = (l + 16.0) / 116.0;
        return (x * x * x);
    }
}

/**
 * Convert a color from the the XYZ colorspace to the Luv colorspace.
 *
 * @param in_out[in,out] The XYZ color converted to a Luv color.
 */
static void xyz2luv(Triplet &in_out)
{
    double const denominator = in_out[0] + (15.0 * in_out[1]) + (3.0 * in_out[2]);
    double var_u = 4.0 * in_out[0] / denominator;
    double var_v = 9.0 * in_out[1] / denominator;
    double l = y2l(in_out[1]);
    double u = 13.0 * l * (var_u - REF_U);
    double v = 13.0 * l * (var_v - REF_V);

    in_out[0] = l;
    if (l < 0.00000001) {
        in_out[1] = 0.0;
        in_out[2] = 0.0;
    } else {
        in_out[1] = u;
        in_out[2] = v;
    }
}

/**
 * Convert a color from the the Luv colorspace to the XYZ colorspace.
 *
 * @param in_out[in,out] The Luv color converted to a XYZ color.
 */
static void luv2xyz(Triplet &in_out)
{
    if (in_out[0] <= 0.00000001) {
        /* Black would create a divide-by-zero error. */
        in_out[0] = 0.0;
        in_out[1] = 0.0;
        in_out[2] = 0.0;
        return;
    }

    double var_u = in_out[1] / (13.0 * in_out[0]) + REF_U;
    double var_v = in_out[2] / (13.0 * in_out[0]) + REF_V;
    double y = l2y(in_out[0]);
    double x = -(9.0 * y * var_u) / ((var_u - 4.0) * var_v - var_u * var_v);
    double z = (9.0 * y - (15.0 * var_v * y) - (var_v * x)) / (3.0 * var_v);

    in_out[0] = x;
    in_out[1] = y;
    in_out[2] = z;
}

/**
 * Convert a color from the the Luv colorspace to the LCH colorspace.
 *
 * @param in_out[in,out] The Luv color converted to a LCH color.
 */
static void luv2lch(Triplet &in_out)
{
    double l = in_out[0];
    auto uv = Geom::Point(in_out[1], in_out[2]);
    double h;
    double const c = uv.length();

    /* Grays: disambiguate hue */
    if (c < 0.00000001) {
        h = 0;
    } else {
        h = Geom::deg_from_rad(Geom::atan2(uv));
        if (h < 0.0) {
            h += 360.0;
        }
    }

    in_out[0] = l;
    in_out[1] = c;
    in_out[2] = h;
}

/**
 * Convert a color from the the LCH colorspace to the Luv colorspace.
 *
 * @param in_out[in,out] The LCH color converted to a Luv color.
 */
static void lch2luv(Triplet &in_out)
{
    double sinhrad, coshrad;
    Geom::sincos(Geom::rad_from_deg(in_out[2]), sinhrad, coshrad);
    double u = coshrad * in_out[1];
    double v = sinhrad * in_out[1];

    in_out[1] = u;
    in_out[2] = v;
}

/**
 * Convert a color from the the HSLuv colorspace to the LCH colorspace.
 *
 * @param in_out[in,out] The HSLuv color converted to a LCH color.
 */
static void hsluv2lch(Triplet &in_out)
{
    double h = in_out[0];
    double s = in_out[1];
    double l = in_out[2];
    double c;

    /* White and black: disambiguate chroma */
    if(l > 99.9999999 || l < 0.00000001) {
        c = 0.0;
    } else {
        c = max_chroma_for_lh(l, h) / 100.0 * s;
    }

    /* Grays: disambiguate hue */
    if (s < 0.00000001) {
        h = 0.0;
    }

    in_out[0] = l;
    in_out[1] = c;
    in_out[2] = h;
}

/**
 * Convert a color from the the LCH colorspace to the HSLuv colorspace.
 *
 * @param in_out[in,out] The LCH color converted to a HSLuv color.
 */
static void lch2hsluv(Triplet &in_out)
{
    double l = in_out[0];
    double c = in_out[1];
    double h = in_out[2];
    double s;

    /* White and black: disambiguate saturation */
    if (l > 99.9999999 || l < 0.00000001) {
        s = 0.0;
    } else {
        s = c / max_chroma_for_lh(l, h) * 100.0;
    }

    /* Grays: disambiguate hue */
    if (c < 0.00000001) {
        h = 0.0;
    }

    in_out[0] = h;
    in_out[1] = s;
    in_out[2] = l;
}

// Interface functions
Triplet luv_to_rgb(double l, double u, double v)
{
    Triplet result{l, u, v};
    luv2xyz(result);
    xyz2rgb(result);

    for (size_t i : {0, 1, 2}) {
        result[i] = std::clamp(result[i], 0.0, 1.0);
    }
    return result;
}

Triplet hsluv_to_luv(double h, double s, double l)
{
    Triplet result{h, s, l};
    hsluv2lch(result);
    lch2luv(result);
    return result;
}

Triplet luv_to_hsluv(double l, double u, double v)
{
    Triplet result{l, u, v};
    luv2lch(result);
    lch2hsluv(result);
    return result;
}

Triplet rgb_to_hsluv(double r, double g, double b)
{
    Triplet result{r, g, b};
    rgb2xyz(result);
    xyz2luv(result);
    luv2lch(result);
    lch2hsluv(result);
    return result;
}

Triplet hsluv_to_rgb(double h, double s, double l)
{
    Triplet result{h, s, l};
    hsluv2lch(result);
    lch2luv(result);
    luv2xyz(result);
    xyz2rgb(result);

    for (size_t i : {0, 1, 2}) {
        result[i] = std::clamp(result[i], 0.0, 1.0);
    }
    return result;
}

Triplet hsluv_to_luv(double *hsl)
{
    return hsluv_to_luv(hsl[0], hsl[1], hsl[2]);
}

double perceptual_lightness(double l)
{
    return l <= 0.885645168 ? l * 0.09032962963 : std::cbrt(l) * 0.249914424 - 0.16;
}

double rgb_to_perceptual_lightness(Triplet const &rgb)
{
    return perceptual_lightness(rgb_to_hsluv(rgb[0], rgb[1], rgb[2])[2]);
}

std::pair<double, double> get_contrasting_color(double l)
{
    double constexpr l_threshold = 0.85;
    if (l > l_threshold) { // Draw dark over light.
        auto t = (l - l_threshold) / (1.0 - l_threshold);
        return {0.0, 0.4 - 0.1 * t};
    } else { // Draw light over dark.
        auto t = (l_threshold - l) / l_threshold;
        return {1.0, 0.6 + 0.1 * t};
    }
}

// http://www.brucelindbloom.com/index.html?Eqn_Lab_to_XYZ.html
Triplet cielab_to_xyz(double L, double a, double b, const Triplet& ref_white_xyz) {
    auto fy = (L + 16) / 116;
    auto fx = a / 500 + fy;
    auto fz = fy - b / 200;

    constexpr auto e = 0.008856;
    constexpr auto k = 903.3;

    auto fx3 = std::pow(fx, 3);
    auto fy3 = std::pow(fy, 3);
    auto fz3 = std::pow(fz, 3);

    auto xr = fx3 > e ? fx3 : (116 * fx - 16) / k;
    auto yr = L > k * e ? fy3 : L / k;
    auto zr = fz3 > e ? fz3 : (116 * fz - 16) / k;

    return {xr * ref_white_xyz[0], yr * ref_white_xyz[1], zr * ref_white_xyz[2]};
}

Triplet lab_to_rgb(double L, double a, double b, const Triplet& ref_white_xyz) {
    auto result = cielab_to_xyz(L, a, b, ref_white_xyz);
    xyz2rgb(result);
    for (size_t i : {0, 1, 2}) {
        result[i] = std::clamp(result[i], 0.0, 1.0);
    }
    return result;
}

// CIE standard illuminant D65, Observer= 2° [0.9504, 1.0000, 1.0888].
// Simulates noon daylight with correlated color temperature of 6504 K.
constexpr Triplet illuminant_d65{ 0.9504, 1.0000, 1.0888 };

Triplet lab_to_rgb(double L, double a, double b) {
    return lab_to_rgb(L, a, b, illuminant_d65);
}

} // namespace Hsluv
