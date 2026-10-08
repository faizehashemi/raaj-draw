// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_RECT_TOOLBAR_H
#define SEEN_RECT_TOOLBAR_H

/**
 * @file
 * Rect aux toolbar
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
class Button;
class Label;
class Adjustment;
} // namespace Gtk

class SPDesktop;
class SPItem;
class SPRect;

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
class Label;
class SpinButton;
class UnitTracker;
}

namespace Toolbar {

class RectToolbar final
    : public Toolbar
    , private Inkscape::XML::NodeObserver
{
public:
    RectToolbar(SPDesktop *desktop);
    ~RectToolbar() override;

private:
    Glib::RefPtr<Gtk::Builder> _builder;
    std::unique_ptr<UI::Widget::UnitTracker> _tracker;

    XML::Node *_repr{nullptr};
    SPItem *_item;

    Gtk::Label &_mode_item;
    UI::Widget::SpinButton &_width_item;
    UI::Widget::SpinButton &_height_item;
    UI::Widget::SpinButton &_rx_item;
    UI::Widget::SpinButton &_ry_item;
    Gtk::Button &_not_rounded;

    bool _freeze{false};
    bool _single{true};

    void setup_derived_spin_button(UI::Widget::SpinButton &btn, Glib::ustring const &name,
                                   void (SPRect::*setter_fun)(gdouble));
    void value_changed(Glib::RefPtr<Gtk::Adjustment> &adj, Glib::ustring const &value_name,
                       void (SPRect::*setter)(gdouble));

    void sensitivize();
    void defaults();
    void watch_ec(SPDesktop* desktop, Inkscape::UI::Tools::ToolBase* tool);
    void selection_changed(Inkscape::Selection *selection);

    sigc::connection _changed;

    void notifyAttributeChanged(Inkscape::XML::Node &node, GQuark name, Inkscape::Util::ptr_shared old_value,
                                Inkscape::Util::ptr_shared new_value) final;
};

}
}
}

#endif /* !SEEN_RECT_TOOLBAR_H */
