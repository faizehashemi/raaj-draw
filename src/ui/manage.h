// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * Helper function to tie lifetime of a RefPtr-owned object to a managing object
 */
/*
 * Authors:
 *   Daniel Boles <dboles.src+inkscape@gmail.com>
 *
 * Copyright (C) 2023 Daniel Boles
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef SEEN_UI_MANAGE_HPP
#define SEEN_UI_MANAGE_HPP

#include <cassert>
#include <map>
#include <utility>
#include <glibmm/refptr.h>
#include <glibmm/objectbase.h>

namespace Inkscape::UI {

namespace Manage::Detail {

/* Glib::Binding overrides GBindingʼs reference-counting. GBinding will be
   freed when either of the bound objects is or when unbind() is called on it.
   However, Glib::Binding is only unreferenced when the final RefPtr to it goes
   out of scope. So, keep a RefPtr to each binding for each bound object. The
   end result of all this is basically getting the C-style lifetime management
   back… so that bindings are automatically managed by their objects in gtkmmui.
   The same goes for Gtk::Gesture, which normally needs a RefPtr kept around.
   This will not be needed in gtkmm 4; Bindings and Gestures are managed there.
   See https://gitlab.gnome.org/GNOME/glibmm/issues/62 for discussion on this
   and controller.h for a practical example of how it is used. */
/* In addition, as we still need to use C API for some signal handlers in GTK3,
   this can also be useful as a place to stash slots, thereby ensuring they live
   as long as C handlers will need to call to them, _but_ are destroyed with the
   Widget or other related object, rather than leaking or needing manual code */

// N.B.: NOT unordered as we (e.g. popup-menu.cpp) require elements to stay at same address forever
template <typename Secondary>
inline auto s_map = std::multimap<Glib::ObjectBase const *, Secondary>{};

template <typename Secondary>
[[nodiscard]] static auto erase(void * const data)
{
    auto const primary = const_cast<Glib::ObjectBase const *>(
                        static_cast<Glib::ObjectBase       *>(data) );
    [[maybe_unused]] auto const count = s_map<Secondary>.erase(primary);
    assert(count > 0);
    return data;
}

template <typename Secondary>
static void add_callback(Glib::ObjectBase const &primary)
{
    auto const data = static_cast<void             *>(
                       const_cast<Glib::ObjectBase *>(&primary) );
    primary.add_destroy_notify_callback(data, &erase<Secondary>);
}

template <typename Secondary>
Secondary &manage(Secondary secondary, Glib::ObjectBase const &primary)
{
    auto &map = s_map<Secondary>;
    auto const [lower, upper] = map.equal_range(&primary);
    bool const existed = lower != map.end() && lower->first == &primary;
    auto const it = map.insert(upper, {&primary, std::move(secondary)});

    if (!existed) {
        add_callback<Secondary>(primary);
    }

    return it->second;
}

} // namespace Manage::Detail

/// Ensure that a slot will stay alive while another object, e.g. a Widget does.
/// The manage()d slot is moved to a new address & we return a reference to that
template <typename Function>
[[nodiscard]] sigc::slot<Function> &manage(sigc::slot<Function> &&secondary,
                                           Glib::ObjectBase const &primary)
{
    assert(static_cast<bool>(secondary)); // Check slot is !empty
    return Manage::Detail::manage(std::move(secondary), primary);
}

/// Ensure a secondary GObject will stay referenced as long as a primary one is.
/// This can be used to tie lifetime of Gtk::Gestures or Glib::Bindings to their
/// widget/object(s), instead of having to keep a RefPtr as is otherwise needed.
/// Overloading ObjectBase avoids instantiating a map for each GObject subclass.
inline void manage(Glib::RefPtr<Glib::ObjectBase const> secondary,
                   Glib::ObjectBase const &primary)
{
    assert(static_cast<bool>(secondary)); // object !=null
    Manage::Detail::manage(std::move(secondary), primary);
}

} // namespace Inkscape::UI

#endif // SEEN_UI_MANAGE_HPP

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim:filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99:
