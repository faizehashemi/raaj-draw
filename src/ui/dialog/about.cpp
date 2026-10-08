// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * @brief A dialog for the about screen
 *
 * Copyright (C) Martin Owens 2019 <doctormo@gmail.com>
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "about.h"

#include <algorithm>
#include <cstddef>
#include <cairo.h>
#include <cairomm/context.h>
#include <cairomm/refptr.h>
#include <cairomm/surface.h>
#include <cstdlib>
#include <fstream>
#include <glibmm/miscutils.h>
#include <glibmm/ustring.h>
#include <gtkmm/image.h>
#include <gtkmm/window.h>
#include <random>
#include <regex>
#include <sstream>
#include <streambuf>
#include <string>
#include <utility>
#include <vector>
#include <glibmm/main.h>
#include <gtkmm/aspectframe.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/clipboard.h>
#include <gtkmm/label.h>
#include <gtkmm/notebook.h>
#include <gtkmm/textview.h>
#include <gtkmm/window.h>
#include <sigc++/adaptors/bind.h>

#include "desktop.h"
#include "display/cairo-utils.h"
#include "helper/auto-connection.h"
#include "hsluv.h"
#include "inkscape-version-info.h"
#include "inkscape.h"
#include "inkscape-window.h"
#include "io/resource.h"
#include "helper/sigc-track-obj.h"
#include "ui/builder-utils.h"
#include "ui/builder-utils.h"
#include "ui/svg-renderer.h"
#include "ui/themes.h"
#include "ui/util.h"

// how long to show each about screen in seconds
constexpr int SLIDESHOW_DELAY_sec = 10;

using namespace Inkscape::IO;

namespace Inkscape::UI::Dialog {

namespace {

class AboutWindow : public Gtk::Window {
public:
    AboutWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder): Gtk::Window(cobject) {
        find_about_screens();
        if (_about_screens.empty()) {
            g_error("AboutWindow: Missing about screens.");
            return;
        }

        builder->get_widget("viewer1", _viewer1);
        builder->get_widget("viewer2", _viewer2);

        builder->get_widget("aspect-frame", _frame);
        _frame->unset_label();
        Gtk::Overlay* overlay;
        builder->get_widget("overlay", overlay);
        _frame->show_all();

        builder->get_widget("dialog-footer", _footer);
    }

    void show_window() {
        _refresh = Glib::signal_timeout().connect_seconds([=](){
            transition();
            return true;
        }, SLIDESHOW_DELAY_sec);

        // reset the stage
        _viewer1->clear();
        _viewer2->clear();
        _about_index = 0;
        _tick = false;
        auto ctx = _viewer2->get_style_context();
        ctx->remove_class("fade-out");
        ctx->remove_class("fade-in");

        set_visible(true);
        transition();
    }

    ~AboutWindow() override = default;

private:
    std::vector<std::string> _about_screens;
    size_t _about_index = 0;
    bool _tick = false;
    Gtk::Box* _footer;
    Glib::RefPtr<Gtk::CssProvider> _footer_style;
    Glib::RefPtr<Glib::TimeoutSource> _timer;
    Gtk::Image* _viewer1;
    Gtk::Image* _viewer2;
    auto_connection _refresh;
    Gtk::AspectFrame* _frame = nullptr;

    void find_about_screens() {
        auto path = Glib::build_filename(get_path_string(Resource::SYSTEM, Resource::SCREENS), "about");
        Resource::get_filenames_from_path(_about_screens, path, {".svgz"}, {});
        if (_about_screens.empty()) {
            g_warning("Error loading about screens SVGZs: no such documents in share/screen/about folder.");
            // fall back
            _about_screens.push_back(Resource::get_filename(Resource::SCREENS, "about.svg", true, false));
        }
        std::sort(_about_screens.begin(), _about_screens.end());
    }

    Cairo::RefPtr<Cairo::ImageSurface> load_next(Gtk::Image* viewer, const Glib::ustring& fname, int device_scale) {
        svg_renderer renderer(fname.c_str());
        auto surface = renderer.render_surface(device_scale);
        if (surface) {
            auto width = renderer.get_width_px();
            auto height = renderer.get_height_px();
            _frame->property_ratio() = width / height;
            viewer->set_size_request(width, height);
        }
        viewer->set(surface);
        return surface;
    }

    void set_footer_matching_color(Cairo::RefPtr<Cairo::ImageSurface>& image) {
        if (!image) return;

        auto scale = get_scale_factor();

        // extract color from a strip at the bottom of the rendered about image
        int width = image->get_width();
        int height = 5 * scale;
        int y = (image->get_height() - height) / scale;
        auto surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, width, height);
        cairo_surface_set_device_scale(surface->cobj(), scale, scale);
        auto ctx = Cairo::Context::create(surface);
        ctx->set_source(image, 0, -y);
        ctx->paint();
        double r,g,b,a;
        ink_cairo_surface_average_color_premul(surface->cobj(), r, g, b, a);

        // calculate footer color: light/dark depending on a theme
        bool dark = INKSCAPE.themecontext->isCurrentThemeDark(this);
        // color of the image strip to HSL, so we can manipulate its lightness
        auto [h, s, l] = Hsluv::rgb_to_hsluv(r, g, b);
        // for a dark theme come up with a darker shade, for a light time - with a lighter one
        l = dark ? l * 0.7 : l + (100 - l) * 0.5;
        // clip them to remove extremes
        l = dark ? std::min(l, 30.0) : std::max(l, 80.0);
        // limit saturation to improve contrast with some artwork
        s = dark ? std::min(s, 80.0) : s;
        auto rgb = Hsluv::hsluv_to_rgb(h, s, l);
        auto style_context = _footer->get_style_context();
        _footer_style = Gtk::CssProvider::create();
        auto color_str = rgba_to_css_color(rgb[0], rgb[1], rgb[2]);
        _footer_style->load_from_data("box {background-color:" + color_str + ";}");
        if (_footer_style) style_context->remove_provider(_footer_style);
        style_context->add_provider(_footer_style, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    // load next about screen
    void transition() {
        _tick = !_tick;
        auto nv = _tick ? _viewer1 : _viewer2;
        auto image = load_next(nv, _about_screens[_about_index++ % _about_screens.size()], get_scale_factor());

        auto ctx = _viewer2->get_style_context();
        if (_tick) {
            ctx->add_class("fade-out");
            ctx->remove_class("fade-in");
        }
        else {
            ctx->remove_class("fade-out");
            ctx->add_class("fade-in");
        }

        set_footer_matching_color(image);
    }
};

} // namespace


static bool show_copy_button(Gtk::Button * const button, Gtk::Label * const label)
{
    reveal_widget(button, true);
    reveal_widget(label, false);
    return false;
}

static void copy(Gtk::Button * const button, Gtk::Label * const label, Glib::ustring const &text)
{
    auto clipboard = Gtk::Clipboard::get();
    clipboard->set_text(text);
    if (label) {
        reveal_widget(button, false);
        reveal_widget(label, true);
        Glib::signal_timeout().connect_seconds(SIGC_TRACKING_ADAPTOR(
            sigc::bind(&show_copy_button, button, label), *button), 2);
    }
}

// Free function to handle key events
static gboolean on_key_pressed(GtkEventControllerKey const * const controller,
                           unsigned const keyval, 
                           unsigned const keycode,
                           GdkModifierType const state,
                           void *user_data) {
    if (keyval == GDK_KEY_Escape) {
        auto const window = static_cast<AboutWindow*>(user_data);
        window->close();
        return true;
    }
    return false;
}

template <class Random>
[[nodiscard]] static auto get_shuffled_lines(std::string const &filename, Random &&random)
{
    std::ifstream fn{Resource::get_filename(Resource::DOCS, filename.c_str())};
    std::vector<std::string> lines;
    std::size_t capacity = 0;
    for (std::string line; getline(fn, line);) {
        capacity += line.size() + 1;
        lines.push_back(std::move(line));
    }
    std::shuffle(lines.begin(), lines.end(), random);
    return std::pair{std::move(lines), capacity};
}

void show_about()
{
    // Load builder file here
    auto builder = create_builder("inkscape-about.glade");
    auto window            = &get_derived_widget<AboutWindow>(builder, "about-screen-window");
    auto tabs              = &get_widget<Gtk::Notebook>(builder, "tabs");
    auto version      = &get_widget<Gtk::Button>  (builder, "version");
    auto label        = &get_widget<Gtk::Label>   (builder, "version-copied");
    auto debug_info   = &get_widget<Gtk::Button>  (builder, "debug_info");
    auto label2       = &get_widget<Gtk::Label>   (builder, "debug-info-copied");
    auto copyright    = &get_widget<Gtk::Label>   (builder, "copyright");
    auto authors      = &get_widget<Gtk::TextView>(builder, "credits-authors");
    auto translators  = &get_widget<Gtk::TextView>(builder, "credits-translators");
    auto license      = &get_widget<Gtk::Label>   (builder, "license-text");

    // Automatic signal handling (requires -rdynamic compile flag)
    //gtk_builder_connect_signals(builder->gobj(), NULL);

    auto text = Inkscape::inkscape_version();
    version->set_label(text);
    version->signal_clicked().connect(
        sigc::bind(&copy, version, label, std::move(text)));

    debug_info->signal_clicked().connect(
        sigc::bind(&copy, version, label2, Inkscape::debug_info()));

    copyright->set_label(
        Glib::ustring::compose(copyright->get_label(), Inkscape::inkscape_build_year()));

    std::random_device rd;
    std::mt19937 g(rd());
    auto const [authors_data, capacity] = get_shuffled_lines("AUTHORS", g);
    std::string str_authors;
    str_authors.reserve(capacity);
    for (auto const &author : authors_data) {
        str_authors.append(author).append(1, '\n');
    }
    authors->get_buffer()->set_text(str_authors.c_str());

    auto const [translators_data, capacity2] = get_shuffled_lines("TRANSLATORS", g);
    std::string str_translators;
    str_translators.reserve(capacity2);
    std::regex e("(.*?)(<.*|)");
    for (auto const &translator : translators_data) {
        str_translators.append(std::regex_replace(translator, e, "$1")).append(1, '\n');
    }
    translators->get_buffer()->set_text(str_translators.c_str());

    std::ifstream fn(Resource::get_filename(Resource::DOCS, "LICENSE"));
    std::string str((std::istreambuf_iterator<char>(fn)),
                        std::istreambuf_iterator<char>());
    license->set_markup(str.c_str());

    // Connect the key event to the on_key_pressed function
    auto const controller = gtk_event_controller_key_new(GTK_WIDGET(window->gobj()));
    g_signal_connect(controller, "key-pressed", G_CALLBACK(on_key_pressed), window);

    if (window) {
        // dispose of about dialog when it gets closed
        window->signal_delete_event().connect([=](GdkEventAny*){
            delete window;
            return false;
        });
        if (auto top = SP_ACTIVE_DESKTOP ? SP_ACTIVE_DESKTOP->getInkscapeWindow() : nullptr) {
            window->set_transient_for(*top);
        }
        tabs->set_current_page(0);
        window->show_window();
    } else {
        g_error("About screen window couldn't be loaded. Missing window id in glade file.");
    }
}

} // namespace Inkscape::UI::Dialog

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
