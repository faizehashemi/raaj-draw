// SPDX-License-Identifier: GPL-2.0-or-later
/** @file Tests for the OKLab/OKLch color space backend.
 */
/*
 * Authors:
 *   Rafał Siejakowski <rs@rs-math.net>
 *
 * Copyright (C) 2022 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <gtest/gtest.h>
#include <glib.h>

#include "color.h"
#include "oklab.h"

unsigned constexpr L=0, A=1, B=2;
double constexpr EPS = 1e-7;

inline Oklab::Triplet random_triplet()
{
    return { g_random_double_range(0.0, 1.0),
             g_random_double_range(0.0, 1.0),
             g_random_double_range(0.0, 1.0) };
}

/** Test converting black and white to OKLab. */
TEST(OklabColorTest, BlackWhite)
{
    using namespace Oklab;

    auto const black = linear_rgb_to_oklab({0, 0, 0});
    EXPECT_NEAR(black[L], 0.0, EPS);
    EXPECT_NEAR(black[A], 0.0, EPS);
    EXPECT_NEAR(black[B], 0.0, EPS);

    auto const white = linear_rgb_to_oklab({1.0, 1.0, 1.0});
    EXPECT_NEAR(white[L], 1.0, EPS);
    EXPECT_NEAR(white[A], 0.0, EPS);
    EXPECT_NEAR(white[B], 0.0, EPS);
}

/** Test linear RGB -> OKLab -> linear RGB roundtrip. */
TEST(OKlabColorTest, RGBRoundrtip)
{
    using namespace Oklab;
    g_random_set_seed(13375336); // We always seed for tests' repeatability

    for (unsigned i = 0; i < 10'000; i++) {
        Triplet rgb = random_triplet();
        auto const roundtrip = oklab_to_linear_rgb(linear_rgb_to_oklab(rgb));
        for (size_t i : {0, 1, 2}) {
            EXPECT_NEAR(roundtrip[i], rgb[i], EPS);
        }
    }
}

/** Test OKLab -> linear RGB -> OKLab roundtrip. */
TEST(OKlabColorTest, OklabRoundrtip)
{
    using namespace Oklab;
    g_random_set_seed(0xCAFECAFE);

    for (unsigned i = 0; i < 10'000; i++) {
        Triplet lab = linear_rgb_to_oklab(random_triplet());
        auto const roundtrip = linear_rgb_to_oklab(oklab_to_linear_rgb(lab));
        for (size_t i : {0, 1, 2}) {
            EXPECT_NEAR(roundtrip[i], lab[i], EPS);
        }
    }
}

/** Test OKLab -> OKLch -> OKLab roundtrip. */
TEST(OKlabColorTest, PolarRectRoundrtip)
{
    using namespace Oklab;
    g_random_set_seed(0xB747A380);

    for (unsigned i = 0; i < 10'000; i++) {
        Triplet lab = linear_rgb_to_oklab(random_triplet());
        auto const roundtrip = oklch_to_oklab(oklab_to_oklch(lab));
        for (size_t i : {1, 2}) { // No point testing [0] since L == L
            EXPECT_NEAR(roundtrip[i], lab[i], EPS);
        }
    }
}

/** Test OKLch -> OKLab -> OKLch roundtrip. */
TEST(OKlabColorTest, RectPolarRoundrtip)
{
    using namespace Oklab;
    g_random_set_seed(0xFA18B52);

    for (unsigned i = 0; i < 10'000; i++) {
        Triplet lch = oklab_to_oklch(linear_rgb_to_oklab(random_triplet()));
        auto const roundtrip = oklab_to_oklch(oklch_to_oklab(lch));
        for (size_t i : {1, 2}) {  // No point testing [0]
            EXPECT_NEAR(roundtrip[i], lch[i], EPS);
        }
    }
}

/** Test maximum chroma calculations. */
TEST(OKlabColorTest, Saturate)
{
    using namespace Oklab;
    g_random_set_seed(0x987654);

    /** Test whether a number lies near to the endpoint of the unit interval. */
    auto const near_end = [](double x) -> bool {
        return x > 0.999 || x < 0.0001;
    };

    for (unsigned i = 0; i < 10'000; i++) {
        // Get a random l, h pair and compute the maximum chroma.
        auto [l, _, h] = oklab_to_oklch(linear_rgb_to_oklab(random_triplet()));
        auto const chromax = max_chroma(l, h);

        // Try maximally saturating the color and verifying that after converting
        // the result to RGB we end up hitting the boundary of the sRGB gamut.
        auto [r, g, b] = oklab_to_linear_rgb(oklch_to_oklab({l, chromax, h}));
        EXPECT_TRUE(near_end(r) || near_end(g) || near_end(b));
    }
}

/** Test OKHSL -> OKLab -> OKHSL conversion roundtrip. */
TEST(OKlabColorTest, HSLabRoundtrip)
{
    using namespace Oklab;
    g_random_set_seed(908070);

    for (unsigned i = 0; i < 10'000; i++) {
        auto const hsl = random_triplet();
        if (hsl[1] < 0.001) {
            // Grayscale colors don't have unique hues,
            // so we skip them (mapping is not bijective).
            continue;
        }
        auto const roundtrip = oklab_to_okhsl(okhsl_to_oklab(hsl));
        for (size_t i : {0, 1, 2}) {
            EXPECT_NEAR(roundtrip[i], hsl[i], EPS);
        }
    }
}

/** Test OKLab -> OKHSL -> OKLab conversion roundtrip. */
TEST(OKlabColorTest, LabHSLRoundtrip)
{
    using namespace Oklab;
    g_random_set_seed(5043071);

    for (unsigned i = 0; i < 10'000; i++) {
        auto const lab = linear_rgb_to_oklab(random_triplet());
        auto const roundtrip = okhsl_to_oklab(oklab_to_okhsl(lab));
        for (size_t i : {0, 1, 2}) {
            EXPECT_NEAR(roundtrip[i], lab[i], EPS);
        }
    }
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :