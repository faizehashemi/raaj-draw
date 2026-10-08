// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 * Raaj Draw: account sign-in and plan check (see account.h).
 *
 * Copyright (C) 2026 Raaj Software
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "raaj/account.h"

#include <algorithm>
#include <ctime>
#include <functional>
#include <thread>

#include <glib.h>
#include <glibmm/i18n.h>
#include <glibmm/main.h>
#include <glibmm/markup.h>
#include <glibmm/miscutils.h>
#include <gtk/gtk.h>
#include <gtkmm/application.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/dialog.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/spinner.h>
#include <gtkmm/window.h>

#include "io/resource.h"
#include "raaj/http.h"

namespace Raaj {

namespace {

constexpr int CHECK_EVERY_SECONDS = 10 * 60;
constexpr std::int64_t OFFLINE_DAYS = 7; // how long the app keeps working without reaching the server
constexpr char const *GROUP = "account";

enum Response { RESPONSE_SIGNIN = 1, RESPONSE_CHOOSE_PLAN, RESPONSE_RECHECK, RESPONSE_SIGNOUT, RESPONSE_RETRY };

std::int64_t now() { return static_cast<std::int64_t>(std::time(nullptr)); }

/// Run work() on a worker thread, then done(result) on the main thread.
template <typename T>
void run_async(std::function<T()> work, std::function<void(T)> done)
{
    std::thread([work = std::move(work), done = std::move(done)]() {
        T result = work();
        auto *call = new std::function<void()>([done, result]() { done(result); });
        g_idle_add(
            [](gpointer data) -> gboolean {
                auto *fn = static_cast<std::function<void()> *>(data);
                (*fn)();
                delete fn;
                return G_SOURCE_REMOVE;
            },
            call);
    }).detach();
}

void open_url(Gtk::Window *parent, std::string const &url)
{
    GError *error = nullptr;
    if (!gtk_show_uri_on_window(parent ? parent->gobj() : nullptr, url.c_str(), GDK_CURRENT_TIME, &error)) {
        g_warning("Raaj Draw: could not open %s: %s", url.c_str(), error ? error->message : "");
        g_clear_error(&error);
    }
}

std::string state_file() { return Inkscape::IO::Resource::profile_path("raaj-account.ini"); }

std::string device_name()
{
    char const *host = g_get_host_name();
#ifdef _WIN32
    char const *os = "Windows";
#elif defined(__APPLE__)
    char const *os = "macOS";
#else
    char const *os = "Linux";
#endif
    return std::string(host && *host ? host : "Computer") + " (" + os + ")";
}

Glib::ustring time_left(std::int64_t seconds)
{
    if (seconds < 60) {
        return _("less than a minute");
    }
    if (seconds < 3600) {
        auto m = static_cast<unsigned long>(seconds / 60);
        return Glib::ustring::compose(ngettext("%1 minute", "%1 minutes", m), m);
    }
    if (seconds < 2 * 86400) {
        auto h = static_cast<unsigned long>(seconds / 3600);
        return Glib::ustring::compose(ngettext("%1 hour", "%1 hours", h), h);
    }
    auto d = static_cast<unsigned long>(seconds / 86400);
    return Glib::ustring::compose(ngettext("%1 day", "%1 days", d), d);
}

Status parse_status(Http::Response const &r)
{
    Status st;
    if (r.status == 0) {
        st.network = true;
        st.error = r.error.empty() ? std::string(_("No connection")) : r.error;
        return st;
    }
    auto kv = Http::parse_kv(r.body);
    if (r.status == 401) {
        st.unauthorized = true;
        return st;
    }
    if (r.status != 200) {
        st.error = kv.count("error") ? kv["error"] : Glib::ustring::compose(_("Server error %1"), r.status).raw();
        return st;
    }
    st.ok = true;
    st.active = kv["access_active"] == "true";
    st.state = kv["access_status"];
    st.plan_name = kv["access_plan_name"];
    st.name = kv["user_name"];
    st.email = kv["user_email"];
    st.paid_until = kv["access_paid_until"];
    st.seconds_left = g_ascii_strtoll(kv["access_seconds_left"].c_str(), nullptr, 10);
    return st;
}

Status fetch_status(std::string const &token)
{
    return parse_status(Http::request("GET", Account::site() + "/api/app/status?format=kv", {}, token));
}

/// A small dialog: app icon, a bold title and a wrapped message.
struct MessageDialog
{
    Gtk::Dialog dialog;
    Gtk::Label title;
    Gtk::Label body;
    Gtk::Box box{Gtk::ORIENTATION_VERTICAL, 10};
    Gtk::Image icon;

    MessageDialog(Gtk::Window *parent, Glib::ustring const &heading, Glib::ustring const &text)
        : dialog(_("Raaj Draw"), true)
    {
        if (parent) {
            dialog.set_transient_for(*parent);
        }
        dialog.set_position(Gtk::WIN_POS_CENTER);
        dialog.set_resizable(false);
        dialog.set_deletable(false);
        icon.set_from_icon_name("org.inkscape.Inkscape", Gtk::ICON_SIZE_DIALOG);
        icon.set_pixel_size(64);
        title.set_markup("<span size='large' weight='bold'>" + Glib::Markup::escape_text(heading) + "</span>");
        title.set_line_wrap(true);
        title.set_max_width_chars(48);
        body.set_line_wrap(true);
        body.set_max_width_chars(52);
        body.set_selectable(false);
        set_text(text);
        box.set_border_width(18);
        box.pack_start(icon, false, false);
        box.pack_start(title, false, false);
        box.pack_start(body, false, false);
        dialog.get_content_area()->pack_start(box, true, true);
    }

    void set_text(Glib::ustring const &text) { body.set_text(text); }
    int run()
    {
        dialog.show_all();
        return dialog.run();
    }
};

} // namespace

Account &Account::get()
{
    static Account instance;
    return instance;
}

std::string Account::site()
{
    char const *override_site = g_getenv("RAAJDRAW_SITE");
    return override_site && *override_site ? override_site : "https://draw.raajsoftware.com";
}

// ---------------------------------------------------------------- persistence

void Account::load()
{
    GKeyFile *kf = g_key_file_new();
    if (g_key_file_load_from_file(kf, state_file().c_str(), G_KEY_FILE_NONE, nullptr)) {
        gchar *token = g_key_file_get_string(kf, GROUP, "token", nullptr);
        _token = token ? token : "";
        g_free(token);
        _access_until = g_key_file_get_int64(kf, GROUP, "access_until", nullptr);
        _checked_at = g_key_file_get_int64(kf, GROUP, "checked_at", nullptr);
        gchar *name = g_key_file_get_string(kf, GROUP, "name", nullptr);
        gchar *email = g_key_file_get_string(kf, GROUP, "email", nullptr);
        gchar *plan = g_key_file_get_string(kf, GROUP, "plan_name", nullptr);
        _last.name = name ? name : "";
        _last.email = email ? email : "";
        _last.plan_name = plan ? plan : "";
        g_free(name);
        g_free(email);
        g_free(plan);
    }
    g_key_file_free(kf);
}

void Account::save()
{
    GKeyFile *kf = g_key_file_new();
    g_key_file_set_string(kf, GROUP, "token", _token.c_str());
    g_key_file_set_int64(kf, GROUP, "access_until", _access_until);
    g_key_file_set_int64(kf, GROUP, "checked_at", _checked_at);
    g_key_file_set_string(kf, GROUP, "name", _last.name.c_str());
    g_key_file_set_string(kf, GROUP, "email", _last.email.c_str());
    g_key_file_set_string(kf, GROUP, "plan_name", _last.plan_name.c_str());
    GError *error = nullptr;
    if (!g_key_file_save_to_file(kf, state_file().c_str(), &error)) {
        g_warning("Raaj Draw: could not save the account file: %s", error ? error->message : "");
        g_clear_error(&error);
    }
    g_key_file_free(kf);
}

void Account::clear_token()
{
    _token.clear();
    _access_until = 0;
    _checked_at = 0;
    _last = Status{};
    save();
}

void Account::apply(Status const &st)
{
    _last = st;
    _checked_at = now();
    _access_until = st.active ? now() + st.seconds_left : 0;
    save();
}

bool Account::cache_valid() const
{
    auto t = now();
    return !_token.empty() && t < _access_until && t - _checked_at < OFFLINE_DAYS * 86400;
}

// ---------------------------------------------------------------- blocking dialogs

bool Account::run_signin(Gtk::Window *parent)
{
    MessageDialog md(parent, _("Sign in to Raaj Draw"),
                     _("Raaj Draw uses your Raaj Software account. Sign in with your browser — new accounts get a "
                       "free 15-minute demo."));
    Gtk::Label code;
    code.set_selectable(true);
    Gtk::Label hint;
    hint.set_line_wrap(true);
    hint.set_max_width_chars(52);
    Gtk::Spinner spinner;
    md.box.pack_start(code, false, false);
    md.box.pack_start(hint, false, false);
    md.box.pack_start(spinner, false, false);
    auto signin_button = md.dialog.add_button(_("Sign in with your browser"), RESPONSE_SIGNIN);
    md.dialog.add_button(_("Quit"), Gtk::RESPONSE_CANCEL);
    md.dialog.set_default_response(RESPONSE_SIGNIN);

    // Shared with the background callbacks, which may arrive after this dialog is gone.
    struct Flow
    {
        bool alive = true;
        std::string device_code;
        std::string url;
        int interval = 5;
        std::string token;
        sigc::connection poll;
    };
    auto flow = std::make_shared<Flow>();
    Gtk::Dialog *dlg = &md.dialog;

    auto poll_once = [flow, dlg, &hint, &spinner]() {
        run_async<Http::Response>(
            [code = flow->device_code]() {
                return Http::request("POST", Account::site() + "/api/device/token?format=kv",
                                     "{\"device_code\":" + Http::json_string(code) + "}");
            },
            [flow, dlg, &hint, &spinner](Http::Response r) {
                if (!flow->alive) {
                    return;
                }
                auto kv = Http::parse_kv(r.body);
                if (r.status == 200 && !kv["token"].empty()) {
                    flow->token = kv["token"];
                    flow->poll.disconnect();
                    dlg->response(Gtk::RESPONSE_OK);
                } else if (r.status == 410) {
                    flow->poll.disconnect();
                    spinner.stop();
                    hint.set_text(_("The sign-in code expired. Click “Sign in with your browser” to try again."));
                } else if (r.status != 428 && r.status != 0) {
                    flow->poll.disconnect();
                    spinner.stop();
                    hint.set_text(kv.count("error") ? kv["error"] : std::string(_("Sign-in failed. Please try again.")));
                }
                // 428 (waiting for approval) and network hiccups: keep polling
            });
    };

    md.dialog.show_all();
    code.hide();
    spinner.hide();
    for (;;) {
        int response = md.dialog.run();
        if (response == Gtk::RESPONSE_OK) {
            flow->alive = false;
            _token = flow->token;
            _checked_at = 0;
            _access_until = 0;
            save();
            return true;
        }
        if (response != RESPONSE_SIGNIN) {
            flow->alive = false;
            flow->poll.disconnect();
            return false;
        }
        // Start a new device login.
        flow->poll.disconnect();
        signin_button->set_sensitive(false);
        hint.set_text(_("Contacting Raaj Draw…"));
        spinner.show();
        spinner.start();
        run_async<Http::Response>(
            []() {
                return Http::request("POST", Account::site() + "/api/device/start?format=kv",
                                     "{\"device_name\":" + Http::json_string(device_name()) + "}");
            },
            [flow, dlg, signin_button, &code, &hint, &spinner, poll_once](Http::Response r) {
                if (!flow->alive) {
                    return;
                }
                signin_button->set_sensitive(true);
                auto kv = Http::parse_kv(r.body);
                if (r.status != 201 || kv["device_code"].empty()) {
                    spinner.stop();
                    hint.set_text(r.status == 0 ? Glib::ustring::compose(_("Could not reach Raaj Draw: %1"), r.error)
                                                : Glib::ustring(kv["error"]));
                    return;
                }
                flow->device_code = kv["device_code"];
                flow->url = kv["verification_url"];
                flow->interval = std::max(2, static_cast<int>(g_ascii_strtoll(kv["interval"].c_str(), nullptr, 10)));
                code.set_markup("<span size='xx-large' weight='bold' font_family='monospace'>" +
                                Glib::Markup::escape_text(kv["user_code"]) + "</span>");
                code.show();
                hint.set_text(Glib::ustring::compose(
                    _("Your browser has opened. Sign in there and confirm this code. If it did not open, go to %1"),
                    flow->url));
                open_url(dlg, flow->url);
                flow->poll = Glib::signal_timeout().connect_seconds(
                    [poll_once]() {
                        poll_once();
                        return true;
                    },
                    flow->interval);
            });
    }
}

Status Account::run_checking(Gtk::Window *parent)
{
    MessageDialog md(parent, _("Raaj Draw"), _("Checking your Raaj Draw plan…"));
    Gtk::Spinner spinner;
    md.box.pack_start(spinner, false, false);
    md.dialog.add_button(_("Quit"), Gtk::RESPONSE_CANCEL);
    spinner.start();

    auto result = std::make_shared<Status>();
    auto alive = std::make_shared<bool>(true);
    Gtk::Dialog *dlg = &md.dialog;
    run_async<Status>([token = _token]() { return fetch_status(token); },
                      [result, alive, dlg](Status st) {
                          if (!*alive) {
                              return;
                          }
                          *result = st;
                          dlg->response(Gtk::RESPONSE_OK);
                      });
    int response = md.run();
    *alive = false;
    if (response != Gtk::RESPONSE_OK) {
        Status quit;
        quit.error = "quit";
        return quit;
    }
    return *result;
}

Account::Choice Account::run_expired(Gtk::Window *parent, Status const &st)
{
    Glib::ustring heading = st.plan_name.empty() ? _("Your free demo has ended")
                                                 : Glib::ustring::compose(_("Your %1 plan has ended"), st.plan_name);
    MessageDialog md(parent, heading,
                     _("Choose a plan on the Raaj Draw website to keep drawing. Plans start at ₹300 a month; yearly "
                       "plans get two months free. After paying, click “Check again”."));
    md.dialog.add_button(_("Choose a plan"), RESPONSE_CHOOSE_PLAN);
    md.dialog.add_button(_("Check again"), RESPONSE_RECHECK);
    md.dialog.add_button(_("Sign out"), RESPONSE_SIGNOUT);
    md.dialog.add_button(_("Quit"), Gtk::RESPONSE_CANCEL);
    md.dialog.set_default_response(RESPONSE_CHOOSE_PLAN);
    md.dialog.show_all();
    for (;;) {
        int response = md.dialog.run();
        if (response == RESPONSE_CHOOSE_PLAN) {
            open_url(&md.dialog, site() + "/account");
            continue;
        }
        if (response == RESPONSE_RECHECK) {
            return Choice::Recheck;
        }
        if (response == RESPONSE_SIGNOUT) {
            return Choice::SignOut;
        }
        return Choice::Quit;
    }
}

bool Account::run_unreachable(Gtk::Window *parent, std::string const &error)
{
    MessageDialog md(parent, _("Can't reach Raaj Draw"),
                     Glib::ustring::compose(_("Raaj Draw needs to check your plan, but the server could not be "
                                              "reached (%1). Check your internet connection and try again."),
                                            error));
    md.dialog.add_button(_("Try again"), RESPONSE_RETRY);
    md.dialog.add_button(_("Quit"), Gtk::RESPONSE_CANCEL);
    md.dialog.set_default_response(RESPONSE_RETRY);
    return md.run() == RESPONSE_RETRY;
}

// ---------------------------------------------------------------- start-up

bool Account::start(Gtk::Application *app)
{
    _app = app;
    load();
    for (;;) {
        if (_token.empty() && !run_signin(nullptr)) {
            return false;
        }
        if (cache_valid()) {
            // Recently checked and still active: start at once and confirm in the background.
            schedule();
            check_in_background();
            return true;
        }
        Status st = run_checking(nullptr);
        if (st.error == "quit") {
            return false;
        }
        if (st.unauthorized) {
            clear_token();
            continue;
        }
        if (!st.ok) {
            if (!run_unreachable(nullptr, st.error)) {
                return false;
            }
            continue;
        }
        apply(st);
        if (st.active) {
            schedule();
            return true;
        }
        switch (run_expired(nullptr, st)) {
            case Choice::Recheck: continue;
            case Choice::SignOut: sign_out(nullptr); continue;
            case Choice::Quit: return false;
        }
    }
}

// ---------------------------------------------------------------- while running

Gtk::Window *Account::active_window() const
{
    return _app ? _app->get_active_window() : nullptr;
}

void Account::schedule()
{
    _timer.disconnect();
    _timer = Glib::signal_timeout().connect_seconds(sigc::mem_fun(*this, &Account::on_timer), CHECK_EVERY_SECONDS);
    // Re-check right when the demo or plan runs out.
    _expiry.disconnect();
    std::int64_t left = _access_until - now();
    if (left > 0 && left < 24 * 3600) {
        _expiry = Glib::signal_timeout().connect_seconds(
            [this]() {
                check_in_background();
                return false;
            },
            static_cast<unsigned>(left + 5));
    }
}

bool Account::on_timer()
{
    check_in_background();
    return true;
}

void Account::check_in_background()
{
    if (_busy || _enforcing || _token.empty()) {
        return;
    }
    _busy = true;
    run_async<Status>([token = _token]() { return fetch_status(token); },
                      [this](Status st) {
                          _busy = false;
                          handle_background(st);
                      });
}

void Account::handle_background(Status const &st)
{
    if (_enforcing) {
        return;
    }
    if (st.ok) {
        apply(st);
        schedule();
        if (!st.active) {
            enforce();
        }
    } else if (st.unauthorized) {
        clear_token();
        enforce();
    } else if (!cache_valid()) {
        // Offline (or server trouble) for too long, or the plan ran out meanwhile.
        enforce();
    }
}

void Account::enforce()
{
    _enforcing = true;
    for (;;) {
        auto *parent = active_window();
        if (_token.empty()) {
            if (!run_signin(parent)) {
                break;
            }
        }
        Status st = run_checking(parent);
        if (st.error == "quit") {
            break;
        }
        if (st.unauthorized) {
            clear_token();
            continue;
        }
        if (!st.ok) {
            if (cache_valid()) {
                _enforcing = false;
                return;
            }
            if (!run_unreachable(parent, st.error)) {
                break;
            }
            continue;
        }
        apply(st);
        if (st.active) {
            _enforcing = false;
            schedule();
            return;
        }
        auto choice = run_expired(parent, st);
        if (choice == Choice::Quit) {
            break;
        }
        if (choice == Choice::SignOut) {
            sign_out(parent);
        }
    }
    _enforcing = false;
    quit_app();
}

void Account::quit_app()
{
    // The normal Quit asks to save changed documents.
    if (_app) {
        _app->activate_action("quit");
    }
}

void Account::sign_out(Gtk::Window *)
{
    std::string token = _token;
    clear_token();
    if (!token.empty()) {
        // End the session on the server too (frees the computer slot); no need to wait for it.
        run_async<int>([token]() {
            Http::request("POST", site() + "/api/auth/logout", "{}", token);
            return 0;
        }, [](int) {});
    }
}

void Account::show_account_dialog(Gtk::Window *parent)
{
    Glib::ustring who = _last.name.empty() ? _last.email : _last.name + " (" + _last.email + ")";
    Glib::ustring plan;
    std::int64_t left = _access_until - now();
    if (_last.state == "demo") {
        plan = Glib::ustring::compose(_("Free demo — %1 left."), time_left(left > 0 ? left : 0));
    } else if (!_last.plan_name.empty()) {
        plan = Glib::ustring::compose(_("%1 plan — %2 left."), _last.plan_name, time_left(left > 0 ? left : 0));
    } else {
        plan = _("No active plan.");
    }
    MessageDialog md(parent, _("Raaj Draw account"),
                     Glib::ustring::compose(_("Signed in as %1.\n%2"), who, plan));
    md.dialog.set_deletable(true);
    md.dialog.add_button(_("Manage plan"), RESPONSE_CHOOSE_PLAN);
    md.dialog.add_button(_("Sign out"), RESPONSE_SIGNOUT);
    md.dialog.add_button(_("Close"), Gtk::RESPONSE_CLOSE);
    md.dialog.set_default_response(Gtk::RESPONSE_CLOSE);
    int response = md.run();
    md.dialog.hide();
    if (response == RESPONSE_CHOOSE_PLAN) {
        open_url(parent, site() + "/account");
    } else if (response == RESPONSE_SIGNOUT) {
        sign_out(parent);
        enforce(); // sign in again (possibly as someone else) or quit
    }
}

} // namespace Raaj
