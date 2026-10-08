// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef INKSCAPE_CMYK_H
#define INKSCAPE_CMYK_H

#include <array>
#include <lcms2.h>
#include "color/cms-color-types.h"

namespace Inkscape {

// Convert CMYK to sRGB
class CmykConverter {
public:
    // Conversion using ICC profile gives the best results and should always be used
    // whenever there is a profile selected/available
    CmykConverter(cmsHPROFILE profile, int intent = INTENT_PERCEPTUAL);

    // Simple (but not simplistic) CMYK to sRGB conversion to show approximately what
    // CMYK colors may look like on an sRGB device (without ICC profile)
    CmykConverter() {}

    ~CmykConverter();

    // if profile has been selected and can decode cmy/k, then return true
    bool profile_used() const { return _profile != nullptr; }

    // CMYK channels from 0 to 100 (percentage) to sRGB (channels 0..255)
    std::array<uint8_t, 3> cmyk_to_rgb(float c, float m, float y, float k);

private:
    cmsHPROFILE _profile = nullptr;
    cmsHTRANSFORM _transform = nullptr;
    cmsHPROFILE _srgb = nullptr;
    bool _cmy = false;
    int _intent = 0;
};

} // namespace

#endif
