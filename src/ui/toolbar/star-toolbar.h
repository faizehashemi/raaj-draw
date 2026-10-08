// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_STAR_TOOLBAR_H
#define SEEN_STAR_TOOLBAR_H

/**
 * @file
 * Star aux toolbar
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
#include "xml/node-observer.h"

namespace Gtk {
class Builder;
class Label;
class ToggleButton;
} // namespace Gtk

class SPDesktop;

namespace Inkscape {
class Selection;

namespace XML {
class Node;
}

namespace UI {
namespace Tools {
class ToolBase;
}

namespace Widget {
class SpinButton;
}

namespace Toolbar {

class StarToolbar final
    : public Toolbar
    , private XML::NodeObserver
{
public:
    StarToolbar(SPDesktop *desktop);
    ~StarToolbar() override;

private:
    using ValueChangedMemFun = void (StarToolbar::*)();
    Glib::RefPtr<Gtk::Builder> _builder;

    Gtk::Label &_mode_item;
    std::vector<Gtk::ToggleButton *> _flat_item_buttons;
    UI::Widget::SpinButton &_magnitude_item;
    Gtk::Box &_spoke_box;
    UI::Widget::SpinButton &_spoke_item;
    UI::Widget::SpinButton &_roundedness_item;
    UI::Widget::SpinButton &_randomization_item;

    XML::Node *_repr{nullptr};

    bool _batchundo = false;
    bool _freeze{false};
    sigc::connection _changed;

    void setup_derived_spin_button(UI::Widget::SpinButton &btn, Glib::ustring const &name, double default_value,
                                   ValueChangedMemFun const value_changed_mem_fun);
    void side_mode_changed(int mode);
    void magnitude_value_changed();
    void proportion_value_changed();
    void rounded_value_changed();
    void randomized_value_changed();
    void defaults();
    void watch_tool(SPDesktop *desktop, UI::Tools::ToolBase *tool);
    void selection_changed(Selection *selection);

    void notifyAttributeChanged(Inkscape::XML::Node &node, GQuark name, Inkscape::Util::ptr_shared old_value,
                                Inkscape::Util::ptr_shared new_value) final;
};

}
}
}

#endif /* !SEEN_SELECT_TOOLBAR_H */
