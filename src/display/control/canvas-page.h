// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * 
 *//*
 * Authors:
 *   Martin Owens 2021
 * 
 * Copyright (C) 2021 Martin Owens
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
#ifndef SEEN_CANVAS_PAGE_H
#define SEEN_CANVAS_PAGE_H

#include <2geom/rect.h>
#include <glib.h>
#include <vector>

#include "canvas-item-ptr.h"

namespace Inkscape {
namespace UI::Widget { class Canvas; }

class CanvasItemGroup;
class CanvasItemText;

class CanvasPage
{
public:
    CanvasPage();
    ~CanvasPage();

    void update(Geom::Rect size, Geom::OptRect margin, Geom::OptRect bleed, const char *txt, bool outline = false);
    void add(Geom::Rect size, CanvasItemGroup *background_group, CanvasItemGroup *foreground_group);
    void remove(UI::Widget::Canvas *canvas);
    void show();
    void hide();
    void set_guides_visible(bool show);

    bool setOnTop(bool on_top);
    bool setShadow(int shadow);
    bool setPageColor(uint32_t border, uint32_t bg, uint32_t canvas, uint32_t margin, uint32_t bleed);
    bool setLabelStyle(const std::string &style);

    bool is_selected = false;
private:
    void _updateTextItem(CanvasItemText *label, Geom::Rect page, std::string txt);

    // This may make this look like a CanvasItemGroup, but it's not one. This
    // isn't a collection of items, but a set of items in multiple Canvases.
    // Each item can belong in either a foreground or background group.
    std::vector<CanvasItemPtr<CanvasItem>> canvas_items;

    int _shadow_size = 0;
    bool _border_on_top = true;
    uint32_t _background_color = 0xffffffff;
    uint32_t _border_color = 0x00000040;
    uint32_t _canvas_color = 0xffffffff;
    uint32_t _margin_color = 0x1699d771; // Blue'ish
    uint32_t _bleed_color = 0xbe310e62; // Red'ish

    std::string _label_style = "default";
};

} // namespace Inkscape

#endif // SEEN_CANVAS_PAGE_H

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
