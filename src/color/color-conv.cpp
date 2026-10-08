// SPDX-License-Identifier: GPL-2.0-or-later

#include <iomanip>
#include <sstream>

#include "color-conv.h"

namespace Inkscape {
namespace Util {

std::string rgba_color_to_string(unsigned int rgba) {
    std::ostringstream ost;
    ost << "#" << std::setfill ('0') << std::setw(8) << std::hex << rgba;
    return ost.str();
}

std::optional<unsigned int> string_to_rgba_color(const char* str) {
    if (!str || *str != '#') {
        return std::optional<unsigned int>();
    }
    try {
        return static_cast<unsigned int>(std::stoul(str + 1, nullptr, 16));
    }
    catch (...) {
        return std::optional<unsigned int>();
    }
}

}
}