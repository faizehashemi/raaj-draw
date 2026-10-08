// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 * Raaj Draw: minimal HTTPS client (see http.h).
 *
 * Copyright (C) 2026 Raaj Software
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "raaj/http.h"

#include <cstdio>
#include <glib.h>

#include "inkscape-version.h"

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#else
#include <curl/curl.h>
#include <mutex>
#endif

namespace Raaj::Http {

namespace {

std::string user_agent()
{
    return std::string("RaajDraw/") + Inkscape::version_string_without_revision;
}

#ifdef _WIN32

std::wstring widen(std::string const &s)
{
    gunichar2 *w = g_utf8_to_utf16(s.c_str(), -1, nullptr, nullptr, nullptr);
    if (!w) {
        return {};
    }
    std::wstring out(reinterpret_cast<wchar_t *>(w));
    g_free(w);
    return out;
}

std::string last_error(char const *what)
{
    char buf[96];
    std::snprintf(buf, sizeof(buf), "%s failed (error %lu)", what, static_cast<unsigned long>(GetLastError()));
    return buf;
}

Response do_request(std::string const &method, std::string const &url, std::string const &body,
                    std::string const &bearer)
{
    Response r;
    std::wstring wurl = widen(url);

    wchar_t host[256] = {};
    wchar_t path[2048] = {};
    wchar_t extra[2048] = {};
    URL_COMPONENTS uc{};
    uc.dwStructSize = sizeof(uc);
    uc.lpszHostName = host;
    uc.dwHostNameLength = sizeof(host) / sizeof(host[0]);
    uc.lpszUrlPath = path;
    uc.dwUrlPathLength = sizeof(path) / sizeof(path[0]);
    uc.lpszExtraInfo = extra;
    uc.dwExtraInfoLength = sizeof(extra) / sizeof(extra[0]);
    if (!WinHttpCrackUrl(wurl.c_str(), 0, 0, &uc)) {
        r.error = last_error("WinHttpCrackUrl");
        return r;
    }
    std::wstring target = std::wstring(path) + extra;

    HINTERNET session = WinHttpOpen(widen(user_agent()).c_str(), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                    WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) {
        r.error = last_error("WinHttpOpen");
        return r;
    }
    WinHttpSetTimeouts(session, 10000, 10000, 20000, 20000);

    HINTERNET connection = WinHttpConnect(session, host, uc.nPort, 0);
    HINTERNET req = nullptr;
    if (!connection) {
        r.error = last_error("WinHttpConnect");
    } else {
        DWORD flags = uc.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0;
        req = WinHttpOpenRequest(connection, widen(method).c_str(), target.c_str(), nullptr, WINHTTP_NO_REFERER,
                                 WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
        if (!req) {
            r.error = last_error("WinHttpOpenRequest");
        }
    }

    if (req) {
        std::wstring headers;
        if (!body.empty()) {
            headers += L"Content-Type: application/json\r\n";
        }
        if (!bearer.empty()) {
            headers += widen("Authorization: Bearer " + bearer + "\r\n");
        }
        BOOL sent = WinHttpSendRequest(req, headers.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : headers.c_str(),
                                       headers.empty() ? 0 : static_cast<DWORD>(-1L),
                                       body.empty() ? WINHTTP_NO_REQUEST_DATA : const_cast<char *>(body.data()),
                                       static_cast<DWORD>(body.size()), static_cast<DWORD>(body.size()), 0);
        if (!sent || !WinHttpReceiveResponse(req, nullptr)) {
            r.error = last_error("WinHttpSendRequest");
        } else {
            DWORD status = 0;
            DWORD size = sizeof(status);
            WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX);
            r.status = static_cast<long>(status);
            for (;;) {
                DWORD available = 0;
                if (!WinHttpQueryDataAvailable(req, &available) || available == 0) {
                    break;
                }
                std::string chunk(available, '\0');
                DWORD read = 0;
                if (!WinHttpReadData(req, chunk.data(), available, &read) || read == 0) {
                    break;
                }
                r.body.append(chunk.data(), read);
                if (r.body.size() > 1024 * 1024) { // our responses are tiny
                    break;
                }
            }
        }
        WinHttpCloseHandle(req);
    }
    if (connection) {
        WinHttpCloseHandle(connection);
    }
    WinHttpCloseHandle(session);
    return r;
}

#else // libcurl

size_t on_data(char *ptr, size_t size, size_t count, void *userdata)
{
    auto out = static_cast<std::string *>(userdata);
    if (out->size() > 1024 * 1024) {
        return 0; // our responses are tiny; stop anything unexpected
    }
    out->append(ptr, size * count);
    return size * count;
}

/// The AppImage bundles a libcurl built on Ubuntu, which only knows Debian's certificate path.
char const *ca_bundle()
{
    static char const *const paths[] = {
        "/etc/ssl/certs/ca-certificates.crt",                // Debian, Ubuntu, Arch
        "/etc/pki/tls/certs/ca-bundle.crt",                  // Fedora, RHEL
        "/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem", // newer Fedora
        "/etc/ssl/ca-bundle.pem",                            // openSUSE
        "/etc/ssl/cert.pem",                                 // Alpine, macOS
    };
    for (auto path : paths) {
        if (g_file_test(path, G_FILE_TEST_EXISTS)) {
            return path;
        }
    }
    return nullptr;
}

Response do_request(std::string const &method, std::string const &url, std::string const &body,
                    std::string const &bearer)
{
    static std::once_flag once;
    std::call_once(once, [] { curl_global_init(CURL_GLOBAL_DEFAULT); });

    Response r;
    CURL *h = curl_easy_init();
    if (!h) {
        r.error = "curl_easy_init failed";
        return r;
    }
    struct curl_slist *headers = nullptr;
    if (!body.empty()) {
        headers = curl_slist_append(headers, "Content-Type: application/json");
    }
    if (!bearer.empty()) {
        headers = curl_slist_append(headers, ("Authorization: Bearer " + bearer).c_str());
    }
    std::string agent = user_agent();
    curl_easy_setopt(h, CURLOPT_URL, url.c_str());
    curl_easy_setopt(h, CURLOPT_CUSTOMREQUEST, method.c_str());
    if (!body.empty()) {
        curl_easy_setopt(h, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(h, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
    }
    curl_easy_setopt(h, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(h, CURLOPT_USERAGENT, agent.c_str());
    curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, on_data);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, &r.body);
    curl_easy_setopt(h, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(h, CURLOPT_TIMEOUT, 25L);
    curl_easy_setopt(h, CURLOPT_NOSIGNAL, 1L);
    if (auto cainfo = ca_bundle()) {
        curl_easy_setopt(h, CURLOPT_CAINFO, cainfo);
    }
    CURLcode rc = curl_easy_perform(h);
    if (rc != CURLE_OK) {
        r.error = curl_easy_strerror(rc);
    } else {
        curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &r.status);
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(h);
    return r;
}

#endif

} // namespace

Response request(std::string const &method, std::string const &url, std::string const &body, std::string const &bearer)
{
    return do_request(method, url, body, bearer);
}

std::map<std::string, std::string> parse_kv(std::string const &body)
{
    std::map<std::string, std::string> out;
    size_t pos = 0;
    while (pos < body.size()) {
        size_t end = body.find('\n', pos);
        if (end == std::string::npos) {
            end = body.size();
        }
        std::string line = body.substr(pos, end - pos);
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        size_t eq = line.find('=');
        if (eq != std::string::npos && eq > 0) {
            out[line.substr(0, eq)] = line.substr(eq + 1);
        }
        pos = end + 1;
    }
    return out;
}

std::string json_string(std::string const &s)
{
    std::string out = "\"";
    for (unsigned char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    return out + "\"";
}

} // namespace Raaj::Http
