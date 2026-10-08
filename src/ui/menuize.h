// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * Helper functions to make children in GtkPopovers act like GtkMenuItem of GTK3
 */
/*
 * Authors:
 *   Daniel Boles <dboles.src+inkscape@gmail.com>
 *
 * Copyright (C) 2023 Daniel Boles
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef SEEN_UI_MENUIZE_H
#define SEEN_UI_MENUIZE_H

#include <memory>
#include <glibmm/refptr.h>

namespace Gio {
class MenuModel;
} // namespace Gio

namespace Gtk {
class Popover;
class Widget;
} // namespace Gtk

namespace Inkscape::UI {

/// Make items behave like GtkMenu: focus if hovered & style focus+hover same by
/// * If hovered by pointer, grab key focus on self & clear focus+hover on rest;
/// * If key-focused in/out, ‘fake’ correspondingly appearing as hovered or not.
void menuize(Gtk::Widget &widget);

/// Temporarily disable :relative-to widget tooltip @ ::show; restore @ ::closed
void autohide_tooltip(Gtk::Popover &popover);

/// menuize() all ModelButtons in @a Popover
void menuize_popover(Gtk::Popover &popover);

/// Create Popover bound to model, relative to the parent widget, with menuize()d ModelButtons
[[nodiscard]] std::unique_ptr<Gtk::Popover>
    make_menuized_popover(Glib::RefPtr<Gio::MenuModel> model, Gtk::Widget &parent);

} // namespace Inkscape::UI

#endif // SEEN_UI_MENUIZE_H

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
