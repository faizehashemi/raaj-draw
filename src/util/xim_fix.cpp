// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file
 * xim_fix.cpp - Work around the XIM input method module
 *
 * Authors:
 *   Luca Bacci <luca.bacci@outlook.com>
 *
 * Copyright (C) 2023 the Authors.
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <cassert>

#include <algorithm>
#include <string>
#include <vector>

#include <boost/algorithm/string/split.hpp>

namespace Inkscape::Util {

bool workaround_xim_module(std::string &gtk_im_module)
{
    if (gtk_im_module.empty()) {
        return false;
    }

    std::vector<std::string> modules;
    boost::split(modules, gtk_im_module, [] (char c) { return c == ':'; });

    if (modules.empty()) {
        return false;
    }

    auto iter = std::remove(modules.begin(), modules.end(), "xim");
    if (iter == modules.end()) {
        return false;
    }

    modules.erase(iter, modules.end());

    gtk_im_module.clear();
    if (!modules.empty()) {
        for (size_t i = 0; i + 1 < modules.size(); ++i) {
            gtk_im_module += modules[i] + ':';
        }
        gtk_im_module += modules.back();
    }

    return true;
}

} // namespace Inkscape::Util

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

