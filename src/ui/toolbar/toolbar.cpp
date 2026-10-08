// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * TODO: insert short description here
 *//*
 * Authors: see git history
 *
 * Copyright (C) 2018 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "toolbar.h"
#include <gtkmm/enums.h>
#include <gtkmm/object.h>
#include <gtkmm/separator.h>

#include "ui/widget/toolbar-menu-button.h"

namespace Inkscape::UI::Toolbar {

Toolbar::Toolbar(SPDesktop *desktop)
    : _desktop(desktop)
{}

void Toolbar::addCollapsibleButton(UI::Widget::ToolbarMenuButton *button)
{
    _expanded_menu_btns.emplace(button);
    _expanded_children = _expanded_menu_btns.top()->get_children();
    _collapsed_children = std::vector<std::pair<int, Gtk::Widget *>>{};
}

void Toolbar::get_preferred_width_vfunc(int &min_w, int &nat_w) const
{
    _toolbar->get_preferred_width(min_w, nat_w);

    if (_toolbar->get_orientation() == Gtk::ORIENTATION_HORIZONTAL && !_expanded_menu_btns.empty()) {
        // HACK: Return too-small value to allow shrinking.
        min_w = 0;
    }
}

void Toolbar::get_preferred_height_vfunc(int &min_h, int &nat_h) const
{
    _toolbar->get_preferred_height(min_h, nat_h);

    if (_toolbar->get_orientation() == Gtk::ORIENTATION_VERTICAL && !_expanded_menu_btns.empty()) {
        // HACK: Return too-small value to allow shrinking.
        min_h = 0;
    }
}

void Toolbar::on_size_allocate(Gtk::Allocation &allocation)
{
    _resize_handler(allocation);
    Gtk::Box::on_size_allocate(allocation);
}

static int min_dimension(Gtk::Widget const *widget, Gtk::Orientation const orientation)
{
    int min = 0;
    int nat = 0;

    if (orientation == Gtk::ORIENTATION_HORIZONTAL) {
        widget->get_preferred_width(min, nat);
    } else {
        widget->get_preferred_height(min, nat);
    }

    return min;
};

void Toolbar::_resize_handler(Gtk::Allocation &allocation)
{
    if (!_toolbar) {
        return;
    }

    auto const orientation = _toolbar->get_orientation();
    auto const allocated_size = orientation == Gtk::ORIENTATION_VERTICAL ? allocation.get_height() : allocation.get_width();
    int min_size = min_dimension(_toolbar, orientation);

    // Special case for the commands toolbar
    if (_toolbar->get_name() == "commands-toolbar") {
        // if the allocated size is the same as the last allocated size,
        // run the resize handler once more just to make sure widgets go to the right place
        if (_resizing && allocated_size == _last_allocated) {
            _resizing = false;
            _resize_handler(allocation);
        } else {
            _resizing = true;
            // The menu button should be the last object in the ui file
            auto menu_btn = _expanded_menu_btns.top();

            // Check if the toolbar is expanding or not. This requires tracking and 
            // comparing the last allocated width.
            bool is_expanding = allocated_size > _last_allocated;
            _last_allocated = allocated_size;

            // The compensation value is normally the height or width of a button,
            // assigned below in the Separator part.
            // There are 2 cases to concern: with how GTK3 works,
            // - when expanding, the total minimum size glitches with separators
            // - when shrinking, no such thing happens (as separators get moved into the popover)
            if (((is_expanding && allocated_size < (min_size - _compensation))
                   || (!is_expanding && allocated_size < min_size))
                   && !_expanded_children.empty()){
                // collapse until allocated size is larger than min size
                // add 1 to compensate the reported allocated size (prevent glitching)
                while (allocated_size + 1 < min_size && !_expanded_children.empty()) {
                    _move_child(_toolbar, menu_btn->get_popover_box());
                    min_size = min_dimension(_toolbar, orientation);
                }
                menu_btn->set_visible(true);
            } else if (allocated_size > min_size && !_collapsed_children.empty()) {
                // get the size of the next widget
                min_size = min_dimension(_toolbar, orientation);
                auto [pos, child] = _collapsed_children.back();
                int next_min = min_dimension(child, orientation);
                // GTK3 returns the wrong height for separators.
                // Use the compensation value to counter that.
                if (auto const sep = dynamic_cast<Gtk::Separator *>(child)) {
                    _compensation = _default_compensation;
                } else {
                    if (_default_compensation == 0) {
                        // get size of a non-separator
                        _default_compensation = next_min;
                    }
                    _compensation = 0;
                }
                // make sure that the allocated size is larger than
                // the minimum + next widget size + compensation
                // (to prevent GTK3 from prematurely expanding icons)
                // minus 1 to compensate the reported allocated size (prevent glitching)
                while (allocated_size - 1 > min_size + next_min + _compensation && !_collapsed_children.empty()) {
                    _move_child(menu_btn->get_popover_box(), _toolbar, true);
                    // min size needs to be recalculated every round
                    min_size = min_dimension(_toolbar, orientation);
                }
                // if there is only 1 collapsed child left, expand it
                if (_collapsed_children.size() == 1) {
                    _move_child(menu_btn->get_popover_box(), _toolbar, true);
                }
            }
            // hide menu button if all buttons are expanded
            if (_collapsed_children.empty()) {
                menu_btn->set_visible(false);
            }
        }
    } else {
        if (allocated_size < min_size) {
            // Shrinkage required.

            // While there are still expanded buttons to collapse...
            while (allocated_size < min_size && !_expanded_menu_btns.empty()) {
                // Collapse the topmost expanded button.
                auto menu_btn = _expanded_menu_btns.top();
                _move_children(_toolbar, menu_btn->get_popover_box(), menu_btn->get_children());
                menu_btn->set_visible(true);

                _expanded_menu_btns.pop();
                _collapsed_menu_btns.push(menu_btn);

                min_size = min_dimension(_toolbar, orientation);
            }

        } else if (allocated_size > min_size) {
            // Once the allocated size of the toolbar is greater than its
            // minimum size, try to re-insert a group of elements back
            // into the toolbar.

            // While there are collapsed buttons to expand...
            while (!_collapsed_menu_btns.empty()) {
                // See if we have enough space to expand the topmost collapsed button.
                auto menu_btn = _collapsed_menu_btns.top();
                int req_size = min_size + menu_btn->get_required_width();

                if (req_size > allocated_size) {
                    // Not enough space - stop.
                    break;
                }

                // Move a group of widgets back into the toolbar.
                _move_children(menu_btn->get_popover_box(), _toolbar, menu_btn->get_children(), true);
                menu_btn->set_visible(false);

                _collapsed_menu_btns.pop();
                _expanded_menu_btns.push(menu_btn);

                min_size = min_dimension(_toolbar, orientation);
            }
        }
    }
}

void Toolbar::_move_children(Gtk::Box *src, Gtk::Box *dest, std::vector<std::pair<int, Gtk::Widget *>> const &children,
                             bool is_expanding)
{
    for (auto [pos, child] : children) {
        src->remove(*child);
        dest->add(*child);

        // is_expanding will be true when the children are being put back into
        // the toolbar. In that case, insert the children at their previous
        // positions.
        if (is_expanding) {
            dest->reorder_child(*child, pos);
        }
    }
}

void Toolbar::_move_child(Gtk::Box *src, Gtk::Box *dest, bool is_expanding)
{
    if (is_expanding && !_collapsed_children.empty()) {
        auto child_element = _collapsed_children.back();
        auto [pos, child] = _collapsed_children.back();
        src->remove(*child);
        dest->add(*child);
        _collapsed_children.pop_back();
        _expanded_children.push_back(child_element);
        dest->reorder_child(*child, pos);
    } else if (!_expanded_children.empty()) {
        auto child_element = _expanded_children.back();
        auto [pos, child] = _expanded_children.back();
        src->remove(*child);
        dest->add(*child);
        _expanded_children.pop_back();
        _collapsed_children.push_back(child_element);
        dest->reorder_child(*child, 0);
    }
}

} // namespace Inkscape::UI::Toolbar

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
