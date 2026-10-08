// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * CMYK to sRGB conversion routines
 *
 * Author:
 *   Michael Kowalski
 *
 * Copyright (C) 2023 Michael Kowalski
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "cmyk-conv.h"
#include <algorithm>
#include <lcms2.h>
#include <glib.h>

namespace Inkscape {

CmykConverter::CmykConverter(cmsHPROFILE profile, int intent) {
    _intent = intent;
    auto color_space = cmsGetColorSpace(profile);
    if (color_space != cmsSigCmykData && color_space != cmsSigCmyData) {
        g_warning("Select CMYK ICC profile to convert CMYK to sRGB");
        return;
    }
    //todo
    if (cmsIsIntentSupported(_profile, intent, LCMS_USED_AS_OUTPUT)) {
    }
    _profile = profile;
    _srgb = cmsCreate_sRGBProfile();
    auto fmt = color_space == cmsSigCmykData ? TYPE_CMYK_16 : TYPE_CMY_16;
    _cmy = color_space == cmsSigCmyData;
    _transform = cmsCreateTransform(profile, fmt, _srgb, TYPE_RGBA_8, intent, 0);
}

CmykConverter::~CmykConverter() {
    if (_transform) cmsDeleteTransform(_transform);
    // _srgb is virtual and probably not something that can be freed
}

// Simple CMYK to sRGB conversion using interpolation from plain cyan, magenta and yellow colors
std::array<uint8_t, 3> simple_cmyk_to_rgb(float c, float m, float y, float k) {
    auto invlerp = [](float f1, float p) {
        // CSS Color module 5 allows for a wide range of CMYK values, but clamps them to 0%-100%
        p = std::clamp(p, 0.0f, 100.0f);
        f1 = (255 - f1) / 255.0f;
        return 1.0f - (f1 * p / 100.0f);
    };

    // interpolate cyan
    auto cr = invlerp(0x00, c);
    auto cg = invlerp(0xa4, c);
    auto cb = invlerp(0xdb, c);
    // magenta
    auto mr = invlerp(0xd7, m);
    auto mg = invlerp(0x15, m);
    auto mb = invlerp(0x7e, m);
    // and yellow
    auto yr = invlerp(0xff, y);
    auto yg = invlerp(0xf1, y);
    auto yb = invlerp(0x08, y);
    // and combine them
    auto bk = 1 - k / 100.0f;
    uint8_t r =  (cr * mr * yr) * bk * 255;
    uint8_t g =  (cg * mg * yg) * bk * 255;
    uint8_t b =  (cb * mb * yb) * bk * 255;
    return {r, g, b};
}

std::array<uint8_t, 3> CmykConverter::cmyk_to_rgb(float c, float m, float y, float k) {
    if (_profile) {
        cmsUInt16Number tmp[4] = { cmsUInt16Number(c / 100.0f * 0xffff), cmsUInt16Number(m / 100.0f * 0xffff), cmsUInt16Number(y / 100.0f * 0xffff), cmsUInt16Number(k / 100.0f * 0xffff) };

        uint8_t post[4] = { 0, 0, 0, 0 };
        cmsDoTransform(_transform, tmp, post, 1);

        if (_cmy && k > 0) {
            // if profile cannot transform black, then this is only a crude approximation
            auto black = 1 - k / 100.0f;
            post[0] *= black;
            post[1] *= black;
            post[2] *= black;
        }
        return { post[0], post[1], post[2] };
    }
    else {
        // no ICC profile available
        return simple_cmyk_to_rgb(c, m, y, k);
    }
}

} // namespace
