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

#ifndef SEEN_MARKER_TOOLBAR_H
#define SEEN_MARKER_TOOLBAR_H

#include <gtkmm/builder.h>

#include "toolbar.h"

namespace Inkscape::UI::Toolbar {

class MarkerToolbar final : public Toolbar
{
public:
    MarkerToolbar(SPDesktop *desktop);

private:
    Glib::RefPtr<Gtk::Builder> _builder;
};

} // namespace Inkscape::UI::Toolbar

#endif
