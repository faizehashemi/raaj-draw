// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * Static objects with destruction before main() exit.
 */
#ifndef INKSCAPE_UTIL_STATICS_BIN_H
#define INKSCAPE_UTIL_STATICS_BIN_H

#include <optional>
#include <cassert>

namespace Inkscape {
namespace Util {

class StaticBase;

/**
 * The following system provides a framework for resolving gnarly issues to do with threads, statics, and libraries at program exit.
 * Instead of the function-local static initialisation idiom
 *
 *     X &get()
 *     {
 *         static X x;
 *         return x;
 *     }
 *
 * you instead write
 *
 *     X &get()
 *     {
 *         static Static<X> x;
 *         return x.get();
 *     }
 *
 * Similarities:
 *     - Both allow creation of singleton objects which are destroyed in the reverse order of constructor completion.
 *     - Both allow dependency registration and automatically prevent dependency loops.
 *
 * Differences:
 *     - The second kind are destructed before the end of main(), the first kind after.
 *     - Only the second supports on-demand destruction and re-initialisation - necessary if you want to write isolated tests.
 *     - Only the first supports thread-safe initialisation; the second must be initialised in the main thread.
 *     - Only the first supports automatic destruction; the second must be destroyed manually with StaticsBin::get().destroy().
 *
 * Rationale examples:
 *     - FontFactory depends on Harfbuzz depends on FreeType, but Harfbuzz doesn't register the dependency so FreeType fails
 *       to outlive both FontFactory and Harfbuzz. (We say X depends on Y if the lifetime of X must be contained in Y; such
 *       a situation can be guaranteed by having X::X call Y::get.) FreeType is inaccessible, so we cannot register the
 *       dependency ourselves without fragile hacks. Therefore, FontFactory must be destroyed before the end of main().
 *
 *     - Background threads can access statics of the first kind. If they run past end of main(), they may find them destructed.
 *       Therefore either the static that joins all threads must be destructed before the end of main(), or must register a
 *       dependency on every static that any background thread might use, which is infeasible and error-prone.
 */

/**
 * Maintains the list of statics that need to be destroyed,
 * destroys them, and complains if it's not asked to do so in time.
 */
class StaticsBin
{
public:
    static StaticsBin &get();
    void destroy();
    ~StaticsBin();

private:
    StaticBase *head = nullptr;

    template <typename T>
    friend class Static;
};

/// Base class for statics, allowing type-erased destruction.
class StaticBase
{
protected:
    StaticBase *next;
    ~StaticBase() = default;
    virtual void destroy() = 0;
    friend class StaticsBin;
};

/// Wrapper for a static of type T.
template <typename T>
class Static final : public StaticBase
{
public:
    T &get()
    {
        if (!opt) {
            opt.emplace();
            auto &bin = StaticsBin::get();
            next = bin.head;
            bin.head = this;
        }
        return *opt;
    }

private:
    std::optional<T> opt;
    void destroy() override { opt.reset(); }
};

} // namespace Util
} // namespace Inkscape

#endif // INKSCAPE_UTIL_STATICS_BIN_H
