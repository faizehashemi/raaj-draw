// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef INKSCAPE_UI_WIDGET_CANVAS_NOTICE_H
#define INKSCAPE_UI_WIDGET_CANVAS_NOTICE_H

#include <glibmm/refptr.h>
#include <gtkmm/builder.h>

#include <gtkmm/revealer.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>

#include "helper/auto-connection.h"

namespace Inkscape {
namespace UI {
namespace Widget {

class CanvasNotice : public Gtk::Revealer {
public:
    static CanvasNotice *create();

    CanvasNotice(BaseObjectType *cobject, Glib::RefPtr<Gtk::Builder> refGlade);
    void show(Glib::ustring const &msg, unsigned timeout = 0);
    void hide();
private:
    Glib::RefPtr<Gtk::Builder> _builder;

    Gtk::Image& _icon;
    Gtk::Label& _label;

    Inkscape::auto_connection _timeout;
};

}}} // namespaces

#endif // INKSCAPE_UI_WIDGET_CANVAS_NOTICE_H
