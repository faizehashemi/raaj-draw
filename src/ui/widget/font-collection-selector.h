// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * This file contains the definition of the FontCollectionSelector class. This widget
 * defines a treeview to provide the interface to create, read, update and delete font
 * collections and their respective fonts. This class contains all the code related to
 * population of collections and their fonts in the TreeStore.
 */
/*
 * Author:
 *   Vaibhav Malik <vaibhavmalik2018@gmail.com> 
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_FONT_COLLECTION_SELECTOR_H
#define INKSCAPE_UI_WIDGET_FONT_COLLECTION_SELECTOR_H

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>
#include <gtkmm/frame.h>
#include <gtkmm/grid.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>
#include <sigc++/signal.h>

namespace Gtk {
class CellRenderer;
class CellRendererText;
class TreeStore;
} // namespace Gtk

namespace Inkscape::UI::Widget {

class IconRenderer;

/**
 * A container of widgets for selecting font faces.
 */
class FontCollectionSelector final : public Gtk::Grid
{
public:
    enum
    {
        TEXT_COLUMN,
        FONT_COUNT_COLUMN,
        ICON_COLUMN,
        N_COLUMNS
    };
    enum SelectionStates {SYSTEM_COLLECTION = -1, USER_COLLECTION, USER_COLLECTION_FONT};

    FontCollectionSelector();
    ~FontCollectionSelector() final;

    // Basic setup.
    void setup_tree_view(Gtk::TreeView*);
    void change_frame_name(const Glib::ustring&);
    void setup_signals();

    Glib::ustring get_text_cell_markup(Gtk::TreeModel::const_iterator const &iter);

    // Custom renderers.
    void text_cell_data_func        (Gtk::CellRenderer *renderer,
                                     Gtk::TreeModel::const_iterator const &iter);
    void font_count_cell_data_func(Gtk::CellRenderer *renderer, Gtk::TreeModel::const_iterator const &iter);
    void icon_cell_data_func        (Gtk::CellRenderer *renderer,
                                     Gtk::TreeModel::const_iterator const &iter);
    void check_button_cell_data_func(Gtk::CellRenderer *renderer,
                                     Gtk::TreeModel::const_iterator const &iter);
    bool row_separator_func(Glib::RefPtr<Gtk::TreeModel> const &model,
                            Gtk::TreeModel::const_iterator const &iter);

    void populate_collections();

    void populate_user_collections();
    void populate_fonts(const Glib::ustring&);

    // Signal handlers
    void on_delete_icon_clicked(Glib::ustring const&);
    void on_create_collection();
    bool on_rename_collection(const Glib::ustring &, const Glib::ustring &);
    void on_delete_button_pressed();
    void on_edit_button_pressed();

    sigc::connection connect_signal_changed(sigc::slot <void (int)> slot) {
        return signal_changed.connect(slot);
    }

private:
    void deletion_warning_message_dialog(Glib::ustring const &collection_name, sigc::slot<void(int)> onresponse);
    gboolean on_key_pressed(GtkEventControllerKey const * controller,
                        unsigned keyval, unsigned keycode, GdkModifierType state);

    bool on_drag_motion(const Glib::RefPtr<Gdk::DragContext> &, int, int, guint) override;
    void on_drag_data_received(const Glib::RefPtr<Gdk::DragContext>, int, int, const Gtk::SelectionData&, guint, guint);
    bool on_drag_drop(const Glib::RefPtr<Gdk::DragContext> &, int, int, guint) override;
    // bool on_drag_failed(const Glib::RefPtr<Gdk::DragContext> &, const Gtk::DragResult);
    void on_drag_leave(const Glib::RefPtr<Gdk::DragContext> &, guint) override;
    // void on_drag_start(const Glib::RefPtr<Gdk::DragContext> &); 
    void on_drag_end(const Glib::RefPtr<Gdk::DragContext> &) override;
    void on_selection_changed();

    class FontCollectionClass : public Gtk::TreeModelColumnRecord
    {
    public:
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<int> font_count;
        Gtk::TreeModelColumn<bool> is_editable;

        FontCollectionClass()
        {
            add(name);
            add(font_count);
            add(is_editable);
        }
    };
    FontCollectionClass FontCollection;

    Gtk::TreeView *treeview = nullptr;
    Gtk::Frame frame;
    Gtk::ScrolledWindow scroll;
    Gtk::TreeViewColumn text_column;
    Gtk::TreeViewColumn font_count_column;
    Gtk::TreeViewColumn del_icon_column;
    Gtk::CellRendererText *cell_text = nullptr;
    Gtk::CellRendererText *cell_font_count = nullptr;
    UI::Widget::IconRenderer *del_icon_renderer = nullptr;

    Glib::RefPtr<Gtk::TreeStore> store;
    bool new_entry = false;

    sigc::signal <void (int)> signal_changed;
};

} // namespace Inkscape::UI::Widget

#endif // INKSCAPE_UI_WIDGET_FONT_COLLECTION_SELECTOR_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
