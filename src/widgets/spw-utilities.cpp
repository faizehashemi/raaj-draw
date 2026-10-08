// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Inkscape Widget Utilities
 *
 * Authors:
 *   Bryce W. Harrington <brycehar@bryceharrington.org>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2003 Bryce W. Harrington
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "spw-utilities.h"

#include <cstring>
#include <gtk/gtk.h>
#include <gtkmm/widget.h>

Glib::ustring sp_get_action_target(Gtk::Widget* widget) {
    Glib::ustring target;

    if (widget && GTK_IS_ACTIONABLE(widget->gobj())) {
        auto variant = gtk_actionable_get_action_target_value(GTK_ACTIONABLE(widget->gobj()));
        auto type = variant ? g_variant_get_type_string(variant) : nullptr;
        if (type && std::strcmp(type, "s") == 0) {
            target = g_variant_get_string(variant, nullptr);
        }
    }

    return target;
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
