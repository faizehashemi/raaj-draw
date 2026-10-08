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

#ifndef SEEN_HSLUV_H
#define SEEN_HSLUV_H

#include <array>
#include <2geom/line.h>

namespace Hsluv {

// Types
using Triplet = std::array<double, 3>;

/**
 * Used to represent the in RGB gamut colors polygon of the HSLuv color wheel.
 */
struct PickerGeometry {
    std::vector<Geom::Point> vertices; ///< Vertices, in counter-clockwise order.
    double outer_circle_radius; ///< Smallest circle with center at origin such that polygon fits inside.
    double inner_circle_radius; ///< Largest circle with center at origin such that it fits inside polygon.
};

// Functions
/** Apply sRGB gamma compression to a linear RGB color component. */
double from_linear(double c);

/** Convert an sRGB color component to linear RGB (de-gamma). */
double to_linear(double c);

/**
 * Return the bounds of the Luv colors in RGB gamut.
 *
 * @param l Lightness. Between 0.0 and 100.0.
 * @return Bounds of Luv colors in RGB gamut.
 */
std::array<Geom::Line, 6> get_bounds(double l);

/**
 * Convert Luv to RGB.
 *
 * @param l Luminance. Between 0.0 and 100.0.
 * @param u U coordinate.
 * @param v V coordinate.
 * @return An RGB triplet, with all components between 0.0 and 1.0.
 */
Triplet luv_to_rgb(double l, double u, double v);

/**
 * Convert Lab to sRGB.
 *
 * @param L Luminance. Between 0.0 and 100.0.
 * @param a A coordinate.
 * @param b B coordinate.
 * @param ref_white_xyz White reference in normalized XYZ color space.
 * @return An sRGB triplet, with all components between 0.0 and 1.0.
 */
Triplet lab_to_rgb(double L, double a, double b, const Triplet& ref_white_xyz);
/**
 * Convert Lab to sRGB using D65 illuminant reference point.
 *
 * @param L Luminance. Between 0.0 and 100.0.
 * @param a A coordinate.
 * @param b B coordinate.
 * @return An sRGB triplet, with all components between 0.0 and 1.0.
 */
Triplet lab_to_rgb(double L, double a, double b);

Triplet cielab_to_xyz(double L, double a, double b, const Triplet& ref_white_xyz);

/**
 * Convert HSLuv to Luv.
 *
 * @param hsl A pointer to a buffer of length 3 containing an HSLuv color:
 * [0]: Hue between 0.0 and 360.0.
 * [1]: Saturation between 0.0 and 100.0.
 * [2]: Lightness between 0.0 and 100.0.
 * @return An LUV triplet, with luminance between 0.0 and 100.0.
 */
Triplet hsluv_to_luv(double *hsl);

/**
 * Convert Luv to HSLuv.
 *
 * @param l Luminance. Between 0.0 and 100.0.
 * @param u U coordinate.
 * @param v V coordinate.
 * @return An HSLuv triplet containing:
 * [0]: Hue between 0.0 and 360.0;
 * [1]: Saturation between 0.0 and 100.0;
 * [2]: Lightness between 0.0 and 100.0.
 */
Triplet luv_to_hsluv(double l, double u, double v);

/**
 * Convert RGB to HSLuv.
 *
 * @param r Red. Between 0.0 and 1.0.
 * @param g Green. Between 0.0 and 1.0.
 * @param b Blue. Between 0.0 and 1.0.
 * @return An HSLuv triplet containing:
 * [0]: Hue between 0.0 and 360.0;
 * [1]: Saturation between 0.0 and 100.0;
 * [2]: Lightness between 0.0 and 100.0.
 */
Triplet rgb_to_hsluv(double r, double g, double b);

/**
 * Convert HSLuv to RGB.
 *
 * @param h Hue. Between 0.0 and 360.0.
 * @param s Saturation. Between 0.0 and 100.0.
 * @param l Lightness. Between 0.0 and 100.0.
 * @return An RGB triplet, with all components between 0.0 and 1.0.
 */
Triplet hsluv_to_rgb(double h, double s, double l);

/**
 * Calculate the perceptual lightness of an HSLuv color.
 *
 * @param l The lightness component in HSLuv coordinates, between 0.0 and 100.0.
 * @return The perceptual lightness between 0.0 (black) and 1.0 (white).
 */
double perceptual_lightness(double l);

/**
 * Calculate the perceptual lightness of an RGB color.
 *
 * @param rgb A triplet of RGB components between 0.0 and 1.0.
 * @return The perceptual lightness between 0.0 (black) and 1.0 (white).
 */
double rgb_to_perceptual_lightness(Triplet const &rgb);

/**
 * Get a contrasting grayscale color suitable for UI elements shown against
 * a background color with the specified perceptual lightness.
 *
 * @param perceptual_lightness The perceptual lightness of the background, between 0.0 and 1.0.
 * @return A pair consisting of grayscale and alpha components representing a color which will
 *         be easy to spot against the background. Both components are between 0.0 and 1.0.
 */
std::pair<double, double> get_contrasting_color(double perceptual_lightness);

} // namespace Hsluv

#endif  // SEEN_HSLUV_H
