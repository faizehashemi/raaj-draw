// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SEEN_CHOOSE_FILE_H
#define SEEN_CHOOSE_FILE_H

#include <string>
#include <utility>
#include <vector>

namespace Glib {
class ustring;
} // namespace Glib

namespace Gtk {
class Window;
} // namespace Gtk

namespace Inkscape {

// select file for saving data
std::string choose_file_save(Glib::ustring const &title, Gtk::Window *parent,
                             Glib::ustring const &mime_type, Glib::ustring const &file_name,
                             std::string& current_folder);

// open single file for reading data
std::string choose_file_open(Glib::ustring const &title, Gtk::Window *parent,
                             std::vector<Glib::ustring> const &mime_types,
                             std::string& current_folder);
std::string choose_file_open(Glib::ustring const &title, Gtk::Window *parent,
                             std::vector<std::pair<Glib::ustring, Glib::ustring>> const &filters,
                             std::string& current_folder);

} // namespace Inkscape

#endif // SEEN_CHOOSE_FILE_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim:filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99:
