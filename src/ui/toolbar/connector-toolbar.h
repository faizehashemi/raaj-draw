// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_CONNECTOR_TOOLBAR_H
#define SEEN_CONNECTOR_TOOLBAR_H

/**
 * @file
 * Connector aux toolbar
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
#include "ui/widget/spinbutton.h"
#include "xml/node-observer.h"

namespace Gtk {
class Builder;
class ToggleButton;
} // namespace Gtk

class SPDesktop;

namespace Inkscape {
class Selection;

namespace XML {
class Node;
}

namespace UI {
namespace Toolbar {

class ConnectorToolbar final
    : public Toolbar
    , private XML::NodeObserver
{
public:
    ConnectorToolbar(SPDesktop *desktop);
    ~ConnectorToolbar() override;

private:
    using ValueChangedMemFun = void (ConnectorToolbar::*)();

    Glib::RefPtr<Gtk::Builder> _builder;

    Gtk::ToggleButton &_orthogonal_btn;
    Gtk::ToggleButton &_directed_btn;
    Gtk::ToggleButton &_overlap_btn;

    UI::Widget::SpinButton &_curvature_item;
    UI::Widget::SpinButton &_spacing_item;
    UI::Widget::SpinButton &_length_item;

    bool _freeze{false};

    Inkscape::XML::Node *_repr{nullptr};

    void setup_derived_spin_button(UI::Widget::SpinButton &btn, Glib::ustring const &name, double default_value,
                                   ValueChangedMemFun const value_changed_mem_fun);
    void path_set_avoid();
    void path_set_ignore();
    void orthogonal_toggled();
    void graph_layout();
    void directed_graph_layout_toggled();
    void nooverlaps_graph_layout_toggled();
    void curvature_changed();
    void spacing_changed();
    void length_changed();
    void selection_changed(Inkscape::Selection *selection);

	void notifyAttributeChanged(Inkscape::XML::Node &node, GQuark name,
								Inkscape::Util::ptr_shared old_value,
								Inkscape::Util::ptr_shared new_value) final;

public:
    static void event_attr_changed(Inkscape::XML::Node *repr,
                                   gchar const         *name,
                                   gchar const         * /*old_value*/,
                                   gchar const         * /*new_value*/,
                                   bool                  /*is_interactive*/,
                                   gpointer             data);
};

}
}
}

#endif /* !SEEN_CONNECTOR_TOOLBAR_H */
