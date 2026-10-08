// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * TODO: insert short description here
 *//*
 * Authors:
 * see git history
 * John Cliff <simarilius@yahoo.com>
 *
 * Copyright (C) 2012 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
#ifndef SEEN_INK_STOCK_ITEMS_H
#define SEEN_INK_STOCK_ITEMS_H

#include <functional>
#include <memory>
#include <vector>

class SPObject;
class SPDocument;

SPObject *get_stock_item(char const *urn, bool stock = false, SPDocument* stock_doc = nullptr);

std::vector<std::shared_ptr<SPDocument>> sp_get_paint_documents(const std::function<bool (SPDocument*)>& filter);

#endif // SEEN_INK_STOCK_ITEMS_H
