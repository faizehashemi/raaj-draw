// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Paper sizes that can have an orientation.
 *
 * Authors:
 *   Martin Owens <doctormo@geek-2.com>
 *
 * Copyright (C) 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef EXTENSION_INTERNAL_TEMPLATE_PAPER_H
#define EXTENSION_INTERNAL_TEMPLATE_PAPER_H

#include "extension/internal/template-base.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

class TemplatePaper : public TemplateBase
{
public:
    TemplatePaper(){};
    static void init();

protected:
    Geom::Point get_template_size(Inkscape::Extension::Template *tmod) const override;

private:
};

} // namespace Internal
} // namespace Extension
} // namespace Inkscape
#endif /* EXTENSION_INTERNAL_TEMPLATE_PAPER_H */
