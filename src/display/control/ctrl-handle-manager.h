// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef INKSCAPE_DISPLAY_CONTROL_CTRL_HANDLE_MANAGER_H
#define INKSCAPE_DISPLAY_CONTROL_CTRL_HANDLE_MANAGER_H
/*
 * Authors:
 *   PBS <pbs3141@gmail.com>
 *
 * Copyright (C) 2023 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <memory>
#include <sigc++/connection.h>
#include <sigc++/slot.h>

namespace Inkscape::Handles {

class Css;

class Manager
{
public:
    static Manager &get();

    std::shared_ptr<Css const> getCss() const;

    sigc::connection connectCssUpdated(sigc::slot<void()> &&slot);

protected:
    ~Manager() = default;
};

} // namespace Inkscape::Handles

#endif // INKSCAPE_DISPLAY_CONTROL_CTRL_HANDLE_MANAGER_H

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
