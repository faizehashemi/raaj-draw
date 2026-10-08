// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
#ifndef INKSCAPE_UTIL_COLOR_CONV_H
#define INKSCAPE_UTIL_COLOR_CONV_H

#include <optional>
#include <string>

namespace Inkscape {
namespace Util {

// Convert RGBA color to '#rrggbbaa' hex string
std::string rgba_color_to_string(unsigned int rgba);

// Parse hex string '#rrgbbaa' and return RGBA color
std::optional<unsigned int> string_to_rgba_color(const char* str);

} } // namespace

#endif
