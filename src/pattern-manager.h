// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef INKSCAPE_PATTERN_MANAGER_H
#define INKSCAPE_PATTERN_MANAGER_H

#include <gtkmm/treemodel.h>
#include <vector>
#include <unordered_map>
#include "ui/widget/pattern-store.h"

class SPPattern;
class SPDocument;

namespace Inkscape {

class PatternManager {
public:
    static PatternManager& get();
    ~PatternManager() = default;

    struct Category : Glib::Object {
        const Glib::ustring name;
        const std::vector<SPPattern*> patterns;
        const bool all;

        Category(Glib::ustring name, std::vector<SPPattern*> patterns, bool all)
            : name(std::move(name)), patterns(std::move(patterns)), all(all) {}
    };

    class PatternCategoryColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        PatternCategoryColumns() {
            add(name);
            add(category);
            add(all_patterns);
        }
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<Glib::RefPtr<Category>> category;
        Gtk::TreeModelColumn<bool> all_patterns;
    } columns;

    // get all stock pattern categories
    Glib::RefPtr<Gtk::TreeModel> get_categories();

    // get pattern description item
    Glib::RefPtr<Inkscape::UI::Widget::PatternItem> get_item(SPPattern* pattern);

    // get pattern image on a solid background for use in UI lists
    Cairo::RefPtr<Cairo::Surface> get_image(SPPattern* pattern, int width, int height, double device_scale);

    // get pattern image on checkerboard background for use as a larger preview
    Cairo::RefPtr<Cairo::Surface> get_preview(SPPattern* pattern, int width, int height, unsigned int rgba_background, double device_scale);

private:
    PatternManager();
    Glib::RefPtr<Gtk::TreeModel> _model;
    std::vector<std::shared_ptr<SPDocument>> _documents;
    std::vector<Glib::RefPtr<Category>> _categories;
    std::unordered_map<SPPattern*, Glib::RefPtr<Inkscape::UI::Widget::PatternItem>> _cache;
    std::shared_ptr<SPDocument> _preview_doc;
    std::shared_ptr<SPDocument> _big_preview_doc;
};

} // namespace

#endif
