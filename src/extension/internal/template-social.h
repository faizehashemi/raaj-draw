// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Various pixel based social media formats.
 *
 * Authors:
 *   Martin Owens <doctormo@geek-2.com>
 *
 * Copyright (C) 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef EXTENSION_INTERNAL_TEMPLATE_SOCIAL_H
#define EXTENSION_INTERNAL_TEMPLATE_SOCIAL_H

#include "extension/internal/template-base.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

class TemplateSocial : public TemplateBase
{
public:
    TemplateSocial(){};
    static void init();
};

} // namespace Internal
} // namespace Extension
} // namespace Inkscape
#endif /* EXTENSION_INTERNAL_TEMPLATE_SOCIAL_H */
