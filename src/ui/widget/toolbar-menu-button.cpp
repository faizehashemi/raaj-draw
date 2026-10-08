// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author:
 *   Vaibhav Malik <vaibhavmalik2018@gmail.com>
 *
 * Copyright (C) 2023 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "toolbar-menu-button.h"

#include <gdk/gdkkeysyms.h>
#include <gtkmm/box.h>
#include <gtkmm/spinbutton.h>
#include "ui/util.h"

namespace Inkscape::UI::Widget {

ToolbarMenuButton::ToolbarMenuButton(BaseObjectType *cobject, Glib::RefPtr<Gtk::Builder> const &)
    : Gtk::MenuButton(cobject)
{
    // Workaround to hide the widget by default and when there are no children in the popover.
    set_visible(false);
    signal_show().connect([this]{
        g_assert(_popover_box);
        if (_popover_box && _popover_box->get_children().empty()) {
            set_visible(false);
        }
    }, false);
}

void ToolbarMenuButton::init(int priority, std::string tag, Gtk::Box *popover_box, std::vector<Gtk::Widget *> &children)
{
    _priority = priority;
    _tag = std::move(tag);
    _popover_box = popover_box;

    // Automatically fetch all the children having "tag" as their style class.
    // This approach will allow even non-programmers to group the children into
    // popovers. Store the position of the child in the toolbar as well. It'll
    // be used to reinsert the child in the right place when the toolbar is
    // large enough.
    int pos = 0;
    for (auto child : children) {
        auto style_context = child->get_style_context();
        bool const is_child = style_context->has_class(_tag);
        if (is_child) {
            _children.emplace_back(pos, child);
        }
        // When pressing Enter, the spinbutton emits the Activate signal causing the popover to be
        // closed. Capture them and do a manual update instead.
        for_each_descendant(*child, [&](Gtk::Widget &widget) {
            if (Gtk::SpinButton *child_spinbutton = dynamic_cast<Gtk::SpinButton *>(&widget)) {
                child_spinbutton->signal_key_press_event().connect([child_spinbutton](GdkEventKey *event) -> bool {
                    if (event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter) {
                        child_spinbutton->update();
                        return true;
                    } else {
                        return false;
                    }
                }, false);
            }
            return ForEachResult::_continue;
        });
        pos++;
    }
}

static int minw(Gtk::Widget const *widget)
{
    g_assert(widget);
    if (!widget) return 0;

    int min = 0;
    int nat = 0;
    widget->get_preferred_width(min, nat);
    return min;
};

int ToolbarMenuButton::get_required_width() const
{
    return minw(_popover_box) - minw(this);
}

} // namespace Inkscape::UI::Widget

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
