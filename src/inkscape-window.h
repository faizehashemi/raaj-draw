// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file
 * Inkscape - An SVG editor.
 */
/*
 * Authors:
 *   Tavmjong Bah
 *
 * Copyright (C) 2018 Authors
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 * Read the file 'COPYING' for more information.
 *
 */

#ifndef INKSCAPE_WINDOW_H
#define INKSCAPE_WINDOW_H

#include <gtkmm/applicationwindow.h>
#include "helper/auto-connection.h"

namespace Gtk { class Box; }

class InkscapeApplication;
class SPDocument;
class SPDesktop;
class SPDesktopWidget;

class InkscapeWindow final : public Gtk::ApplicationWindow {
public:
    InkscapeWindow(SPDocument* document);
    ~InkscapeWindow() final;

    SPDocument*      get_document()       { return _document; }
    SPDesktop*       get_desktop()        { return _desktop; }
    SPDesktopWidget* get_desktop_widget() { return _desktop_widget; }
    void change_document(SPDocument* document);

private:
    InkscapeApplication *_app = nullptr;
    SPDocument*          _document = nullptr;
    SPDesktop*           _desktop = nullptr;
    SPDesktopWidget*     _desktop_widget = nullptr;
    Gtk::Box*      _mainbox = nullptr;
    bool _resized = false;
    Inkscape::auto_connection _resize;
    Inkscape::auto_connection _alloc;

    void setup_view();
    void add_document_actions();

public:
    // TODO: Can we avoid it being public? Probably yes in GTK4.
    bool on_key_press_event(GdkEventKey* event) final;

private:
    bool on_window_state_changed(GdkEventWindowState const *event);
    void on_is_active_changed();
    bool on_delete_event(GdkEventAny* event) final;
    bool on_configure_event(GdkEventConfigure *event) final;

    void update_dialogs();
};

#endif // INKSCAPE_WINDOW_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
