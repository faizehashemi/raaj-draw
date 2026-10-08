// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 * Raaj Draw: account sign-in and plan check.
 *
 * The app signs in through the browser (device login at draw.raajsoftware.com/device), keeps the
 * session token in the profile directory, and checks the account's free demo or paid plan at start
 * and every few minutes. When neither is active, a modal dialog asks the person to choose a plan.
 * Only the GUI is checked: command-line use (also used by extensions) is not affected.
 *
 * Copyright (C) 2026 Raaj Software
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef RAAJ_ACCOUNT_H
#define RAAJ_ACCOUNT_H

#include <cstdint>
#include <memory>
#include <string>

#include <sigc++/connection.h>

namespace Gtk {
class Application;
class Window;
} // namespace Gtk

namespace Raaj {

struct Status
{
    bool ok = false;           ///< the server answered with a status
    bool unauthorized = false; ///< the session token is no longer valid (signed out elsewhere)
    bool network = false;      ///< the server could not be reached
    std::string error;
    bool active = false;
    std::string state;         ///< demo | paid | expired
    std::string plan_name;
    std::string name;
    std::string email;
    std::string paid_until;
    std::int64_t seconds_left = 0;
};

class Account
{
public:
    static Account &get();

    /// Called once when the GUI starts. Returns once the person is signed in with an active demo or
    /// plan; returns false when they chose to quit.
    bool start(Gtk::Application *app);

    /// Help → Raaj Draw Account.
    void show_account_dialog(Gtk::Window *parent);

    /// Site URL (https://draw.raajsoftware.com, or $RAAJDRAW_SITE for testing).
    static std::string site();

private:
    Account() = default;

    // persistence (profile dir / raaj-account.ini)
    void load();
    void save();
    void clear_token();
    void apply(Status const &st);
    bool cache_valid() const;

    // blocking steps (nested dialog loops)
    bool run_signin(Gtk::Window *parent);
    Status run_checking(Gtk::Window *parent);
    enum class Choice { Recheck, SignOut, Quit };
    Choice run_expired(Gtk::Window *parent, Status const &st);
    bool run_unreachable(Gtk::Window *parent, std::string const &error);

    // background checks once the app is running
    void schedule();
    bool on_timer();
    void check_in_background();
    void handle_background(Status const &st);
    void enforce();
    void quit_app();
    void sign_out(Gtk::Window *parent);
    Gtk::Window *active_window() const;

    Gtk::Application *_app = nullptr;
    std::string _token;
    Status _last;
    std::int64_t _access_until = 0; // unix time, local clock, when the demo/plan ends
    std::int64_t _checked_at = 0;   // unix time of the last successful check
    bool _busy = false;             // a check is in flight
    bool _enforcing = false;        // a blocking dialog is open
    sigc::connection _timer;
    sigc::connection _expiry;
};

} // namespace Raaj

#endif // RAAJ_ACCOUNT_H
