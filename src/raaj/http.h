// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 * Raaj Draw: minimal HTTPS client for talking to draw.raajsoftware.com.
 *
 * Windows uses WinHTTP (part of Windows, uses the system certificate store);
 * other platforms use libcurl. Requests are blocking: call them from a worker thread.
 *
 * Copyright (C) 2026 Raaj Software
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef RAAJ_HTTP_H
#define RAAJ_HTTP_H

#include <map>
#include <string>

namespace Raaj::Http {

struct Response
{
    long status = 0;     ///< HTTP status; 0 when the request did not reach the server
    std::string body;
    std::string error;   ///< transport error (no connection, TLS, timeout)
};

/// Send a request. A non-empty body is sent as JSON; a non-empty bearer adds an Authorization header.
Response request(std::string const &method, std::string const &url, std::string const &body = {},
                 std::string const &bearer = {});

/// Parse the server's "key=value" lines (responses asked for with ?format=kv).
std::map<std::string, std::string> parse_kv(std::string const &body);

/// Quote a string as a JSON string literal.
std::string json_string(std::string const &s);

} // namespace Raaj::Http

#endif // RAAJ_HTTP_H
