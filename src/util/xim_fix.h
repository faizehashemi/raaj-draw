// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file
 * xim_fix.h - Work around the XIM input method module
 *
 * Authors:
 *   Luca Bacci <luca.bacci@outlook.com>
 *
 * Copyright (C) 2023 the Authors.
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef SEEN_INKSCAPE_UTIL_XIM_FIX_H
#define SEEN_INKSCAPE_UTIL_XIM_FIX_H

namespace Inkscape::Util {

/**
 * @brief
 * Removes the XIM input method module from the module list. This is
 * useful for working around broken XIM input method module behaviour
 * which is known to cause graphical artifacts.
 *
 * See https://gitlab.com/inkscape/inkscape/-/issues/3664
 *
 * @gtk_im_modules - Reference to a std::string which will be modified in-place.
 *                   Shall contain a list of module names separated by colon.
 *                   Can be retrieved from the GTK_IM_MODULE environment variable
 *                   or the "gtk-im-module" property of GtkSettings, for example.
 *                   The returned string may be empty.
 *
 * @return true if xim was stripped out, false if no change was applied
 */
bool workaround_xim_module(std::string &gtk_im_modules);

} // namespace Inkscape::Util

#endif // SEEN_INKSCAPE_UTIL_XIM_FIX_H

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

