// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_IMAGE_PROPERTIES_H
#define SEEN_IMAGE_PROPERTIES_H

#include <gtkmm/box.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/radiobutton.h>
#include "helper/auto-connection.h"
#include "object/sp-image.h"
#include "ui/operation-blocker.h"

namespace Inkscape {
namespace UI {
namespace Widget {

class ImageProperties : public Gtk::Box {
public:
    ImageProperties();
    ~ImageProperties() override = default;

    void update(SPImage* image);

private:
    void on_style_updated() override;
    void update_bg_color();

    Glib::RefPtr<Gtk::Builder> _builder;

    Gtk::DrawingArea& _preview;
    Gtk::RadioButton& _aspect;
    Gtk::RadioButton& _stretch;
    Gtk::ComboBoxText& _rendering;
    Gtk::Button& _embed;
    int _preview_max_height;
    int _preview_max_width;
    SPImage* _image = nullptr;
    OperationBlocker _update;
    Cairo::RefPtr<Cairo::Surface> _preview_image;
    uint32_t _background_color = 0;
};

}}} // namespaces

#endif // SEEN_IMAGE_PROPERTIES_H
