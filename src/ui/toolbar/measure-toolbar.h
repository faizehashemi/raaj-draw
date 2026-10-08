// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_MEASURE_TOOLBAR_H
#define SEEN_MEASURE_TOOLBAR_H

/**
 * @file
 * Measure aux toolbar
 */
/* Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Frank Felfe <innerspace@iname.com>
 *   John Cliff <simarilius@yahoo.com>
 *   David Turner <novalis@gnu.org>
 *   Josh Andler <scislac@scislac.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Tavmjong Bah <tavmjong@free.fr>
 *   Abhishek Sharma
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *   Vaibhav Malik <vaibhavmalik2018@gmail.com>
 *
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2003 MenTaLguY
 * Copyright (C) 1999-2011 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "toolbar.h"

namespace Gtk {
class Builder;
class ToggleButton;
} // namespace Gtk

class SPDesktop;

namespace Inkscape {
namespace UI {

namespace Widget {
class SpinButton;
class UnitTracker;
}

namespace Toolbar {

class MeasureToolbar final : public Toolbar
{
public:
    MeasureToolbar(SPDesktop *desktop);
    ~MeasureToolbar() override;

private:
    using ValueChangedMemFun = void (MeasureToolbar::*)();

    Glib::RefPtr<Gtk::Builder> _builder;
    UI::Widget::UnitTracker *_tracker;
    UI::Widget::SpinButton &_font_size_item;
    UI::Widget::SpinButton &_precision_item;
    UI::Widget::SpinButton &_scale_item;

    Gtk::ToggleButton &_only_selected_btn;
    Gtk::ToggleButton &_ignore_1st_and_last_btn;
    Gtk::ToggleButton &_inbetween_btn;
    Gtk::ToggleButton &_show_hidden_btn;
    Gtk::ToggleButton &_all_layers_btn;

    UI::Widget::SpinButton &_offset_item;

    void setup_derived_spin_button(UI::Widget::SpinButton &btn, Glib::ustring const &name, double default_value,
                                   ValueChangedMemFun const value_change_mem_fun);
    void fontsize_value_changed();
    void unit_changed(int notUsed);
    void precision_value_changed();
    void scale_value_changed();
    void offset_value_changed();
    void toggle_only_selected();
    void toggle_ignore_1st_and_last();
    void toggle_show_hidden();
    void toggle_show_in_between();
    void toggle_all_layers();
    void reverse_knots();
    void to_phantom();
    void to_guides();
    void to_item();
    void to_mark_dimension();
};
}
}
}

#endif /* !SEEN_MEASURE_TOOLBAR_H */
