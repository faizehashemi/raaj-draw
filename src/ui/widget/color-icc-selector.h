// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * TODO: insert short description here
 *//*
 * Authors: see git history
 *
 * Copyright (C) 2018 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
#ifndef SEEN_SP_COLOR_ICC_SELECTOR_H
#define SEEN_SP_COLOR_ICC_SELECTOR_H

#include <memory>
#include <glibmm/ustring.h>
#include <gtkmm/grid.h>
#include "ui/selected-color.h"

namespace Inkscape {

class ColorProfile;

namespace UI::Widget {

class ColorICCSelectorImpl;

class ColorICCSelector final
    : public Gtk::Grid
  {
public:
    ColorICCSelector(SelectedColor &color, bool no_alpha);
    ~ColorICCSelector() final;

    ColorICCSelector(const ColorICCSelector &obj) = delete;
    ColorICCSelector &operator=(const ColorICCSelector &obj) = delete;

    void init(bool no_alpha);

protected:
    void on_show() final;

    virtual void _colorChanged();

    void _recalcColor(bool changing);

private:
    friend class ColorICCSelectorImpl;
    std::unique_ptr<ColorICCSelectorImpl> _impl;
};


class ColorICCSelectorFactory final : public ColorSelectorFactory {
public:
    Gtk::Widget *createWidget(SelectedColor &color, bool no_alpha) const final;
    Glib::ustring modeName() const final;
};

} // namespace UI::Widget

} // namespace Inkscape

#endif // SEEN_SP_COLOR_ICC_SELECTOR_H

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
