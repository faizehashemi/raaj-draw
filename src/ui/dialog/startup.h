// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * @brief A dialog for the start screen
 */
/*
 * Copyright (C) Martin Owens 2020 <doctormo@gmail.com>
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include <string>             // for string
#include <gdk/gdk.h>          // for GdkModifierType
#include <glibmm/refptr.h>    // for RefPtr
#include <glibmm/ustring.h>   // for ustring
#include <gtk/gtk.h>          // for GtkEventControllerKey
#include <gtkmm/dialog.h>     // for Dialog
#include <gtkmm/treemodel.h>  // for TreeModel

namespace Gtk {
class Builder;
class Button;
class ComboBox;
class Label;
class Notebook;
class Overlay;
class TreeView;
class Widget;
class Window;
} // namespace Gtk

class SPDocument;

namespace Inkscape::UI {

namespace Widget {
class TemplateList;
} // namespace Widget

namespace Dialog {

class StartScreen : public Gtk::Dialog {
public:
    StartScreen();
    ~StartScreen() override;

    static void migrate_settings();

    SPDocument* get_document() { return _document; }

    void show_now();
    void setup_welcome();
    bool is_welcome_instantiated();

protected:
    void on_response(int response_id) override;

private:
    void notebook_next(Gtk::Widget *button);
    gboolean on_key_pressed(GtkEventControllerKey const *controller,
                        unsigned keyval, unsigned keycode, GdkModifierType state);
    Gtk::TreeModel::Row active_combo(std::string widget_name);
    void set_active_combo(std::string widget_name, std::string unique_id);
    void show_toggle();
    void enlist_recent_files();
    void enlist_keys();
    void filter_themes(Gtk::ComboBox *themes);
    void keyboard_changed();
    void banner_switch(unsigned page_num);

    void theme_changed();
    void canvas_changed();
    void refresh_theme(Glib::ustring theme_name);
    void refresh_dark_switch();

    void new_document();
    void load_document();
    void on_recent_changed();
    void on_kind_changed(Gtk::Widget *tab, unsigned page_num);

private:
    const std::string opt_shown;

    Glib::RefPtr<Gtk::Builder> build_splash;
    Gtk::Overlay &banners;
    Gtk::Button &close_btn;
    Gtk::Label &messages;

    Glib::RefPtr<Gtk::Builder> build_welcome;
    UI::Widget::TemplateList *templates = nullptr;
    Gtk::TreeView *recentfiles = nullptr;

    SPDocument* _document = nullptr;
    bool _welcome_instantiated = false;
};

} // namespace Dialog

} // namespace Inkscape::UI

#endif // STARTSCREEN_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
