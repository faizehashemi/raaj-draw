// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * Marker edit mode toolbar - onCanvas marker editing of marker orientation, position, scale
 *//*
 * Authors:
 * see git history
 * Rachana Podaralla <rpodaralla3@gatech.edu>
 * Vaibhav Malik <vaibhavmalik2018@gmail.com>
 *
 * Copyright (C) 2018 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "marker-toolbar.h"

#include <gtkmm/box.h>

#include "ui/builder-utils.h"

namespace Inkscape::UI::Toolbar {

MarkerToolbar::MarkerToolbar(SPDesktop *desktop)
    : Toolbar(desktop)
    , _builder(create_builder("toolbar-marker.ui"))
{
    _toolbar = &get_widget<Gtk::Box>(_builder, "marker-toolbar");
    add(*_toolbar);
}

} // namespace Inkscape::UI::Toolbar
