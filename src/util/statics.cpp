// SPDX-License-Identifier: GPL-2.0-or-later
#include "statics.h"

Inkscape::Util::StaticsBin &Inkscape::Util::StaticsBin::get()
{
    static StaticsBin instance;
    return instance;
}

void Inkscape::Util::StaticsBin::destroy()
{
    while (head) {
        head->destroy();
        head = head->next;
    }
}

Inkscape::Util::StaticsBin::~StaticsBin()
{
    // If this assertion triggers, then destroy() wasn't called close enough to the end of main().
    assert(!head && "StaticsBin::destroy() must be called before main() exit");
}
