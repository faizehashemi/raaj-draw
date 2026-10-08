// SPDX-License-Identifier: GPL-2.0-or-later
#include "ui/util.h"
#include "helper/geom.h"
#include "util.h"

namespace Inkscape {
namespace UI {
namespace Widget {

void region_to_path(Cairo::RefPtr<Cairo::Context> const &cr, Cairo::RefPtr<Cairo::Region> const &reg)
{
    for (int i = 0; i < reg->get_num_rectangles(); i++) {
        auto rect = reg->get_rectangle(i);
        cr->rectangle(rect.x, rect.y, rect.width, rect.height);
    }
}

Cairo::RefPtr<Cairo::Region> shrink_region(Cairo::RefPtr<Cairo::Region> const &reg, int d, int t)
{
    // Find the bounding rect, expanded by 1 in all directions.
    auto rect = geom_to_cairo(expandedBy(cairo_to_geom(reg->get_extents()), 1));

    // Take the complement of the region within the rect.
    auto reg2 = Cairo::Region::create(rect);
    reg2->subtract(reg);

    // Increase the width and height of every rectangle by d.
    auto reg3 = Cairo::Region::create();
    for (int i = 0; i < reg2->get_num_rectangles(); i++) {
        auto rect = reg2->get_rectangle(i);
        rect.x += t;
        rect.y += t;
        rect.width += d;
        rect.height += d;
        reg3->do_union(rect);
    }

    // Take the complement of the region within the rect.
    reg2 = Cairo::Region::create(rect);
    reg2->subtract(reg3);

    return reg2;
}

std::array<float, 3> checkerboard_darken(std::array<float, 3> const &rgb, float amount)
{
    std::array<float, 3> hsl;
    SPColor::rgb_to_hsl_floatv(&hsl[0], rgb[0], rgb[1], rgb[2]);
    hsl[2] += (hsl[2] < 0.08 ? 0.08 : -0.08) * amount;

    std::array<float, 3> rgb2;
    SPColor::hsl_to_rgb_floatv(&rgb2[0], hsl[0], hsl[1], hsl[2]);

    return rgb2;
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
