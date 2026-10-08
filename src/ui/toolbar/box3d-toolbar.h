// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_BOX3D_TOOLBAR_H
#define SEEN_BOX3D_TOOLBAR_H

/**
 * @file
 * 3d box aux toolbar
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

#include "axis-manip.h"
#include "toolbar.h"
#include "xml/node-observer.h"

namespace Gtk {
class Builder;
class ToggleButton;
} // namespace Gtk

class Persp3D;
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

namespace Tools {
class ToolBase;
}

namespace Toolbar {

class Box3DToolbar final
    : public Toolbar
    , private XML::NodeObserver
{
public:
    Box3DToolbar(SPDesktop *desktop);
    ~Box3DToolbar() override;

private:
    Glib::RefPtr<Gtk::Builder> _builder;

    UI::Widget::SpinButton &_angle_x_item;
    UI::Widget::SpinButton &_angle_y_item;
    UI::Widget::SpinButton &_angle_z_item;

    Gtk::ToggleButton &_vp_x_state_btn;
    Gtk::ToggleButton &_vp_y_state_btn;
    Gtk::ToggleButton &_vp_z_state_btn;

    XML::Node *_repr{nullptr};
    bool _freeze{false};

    void angle_value_changed(Glib::RefPtr<Gtk::Adjustment> &adj,
                             Proj::Axis                     axis);
    void vp_state_changed(Proj::Axis axis);
    void check_ec(SPDesktop* desktop, Inkscape::UI::Tools::ToolBase* tool);
    void selection_changed(Inkscape::Selection *selection);
    void resync_toolbar(Inkscape::XML::Node *persp_repr);
    void set_button_and_adjustment(Persp3D *persp, Proj::Axis axis, UI::Widget::SpinButton &spin_btn,
                                   Gtk::ToggleButton &toggle_btn);

    sigc::connection _changed;

	void notifyAttributeChanged(Inkscape::XML::Node &node, GQuark name,
								Inkscape::Util::ptr_shared old_value,
								Inkscape::Util::ptr_shared new_value) final;

    void setup_derived_spin_button(UI::Widget::SpinButton &btn, Glib::ustring const &name, Proj::Axis const axis);
};

}
}
}

#endif /* !SEEN_BOX3D_TOOLBAR_H */
