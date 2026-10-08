// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_SPIRAL_TOOLBAR_H
#define SEEN_SPIRAL_TOOLBAR_H

/**
 * @file
 * Spiral aux toolbar
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
class Adjustment;
} // namespace Gtk

class SPDesktop;

namespace Inkscape {
class Selection;

namespace XML {
class Node;
}

namespace UI {
namespace Widget {
class SpinButton;
}

namespace Toolbar {

class SpiralToolbar final
    : public Toolbar
    , private XML::NodeObserver
{
public:
    SpiralToolbar(SPDesktop *desktop);
    ~SpiralToolbar() override;

private:
    Glib::RefPtr<Gtk::Builder> _builder;
    Gtk::Label &_mode_item;

    UI::Widget::SpinButton &_revolution_item;
    UI::Widget::SpinButton &_expansion_item;
    UI::Widget::SpinButton &_t0_item;

    bool _freeze{false};

    XML::Node *_repr{nullptr};

    void value_changed(Glib::RefPtr<Gtk::Adjustment> &adj, Glib::ustring const &value_name);
    void defaults();
    void selection_changed(Inkscape::Selection *selection);

    std::unique_ptr<sigc::connection> _connection;

    void event_attr_changed(XML::Node &repr);

	void notifyAttributeChanged(Inkscape::XML::Node &node, GQuark key, Inkscape::Util::ptr_shared oldval, Inkscape::Util::ptr_shared newval) final;

    void setup_derived_spin_button(UI::Widget::SpinButton &btn, Glib::ustring const &name, double default_value);
};

} // namespace Toolbar
} // namespace UI
} // namespace Inkscape

#endif /* !SEEN_SPIRAL_TOOLBAR_H */
