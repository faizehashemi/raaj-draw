// SPDX-License-Identifier: GPL-2.0-or-later
#include <vector>
#include <algorithm>
#include <mutex>
#include <chrono>
#include "async.h"
#include "util/statics.h"

namespace {

// Todo: Replace when C++ gets an .is_ready().
bool is_ready(std::future<void> const &future)
{
    return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

// Holds on to asyncs and waits for them to finish at program exit.
class AsyncBin
{
public:
    static auto const &get()
    {
        /*
         * Using Static<AsyncBin> to ensure destruction before main() exits, so that lifetimes
         * of background threads are synchronized with the destruction of statics.
         */
        static Inkscape::Util::Static<AsyncBin const> instance;
        return instance.get();
    }

    void add(std::future<void> &&future) const
    {
        auto g = std::lock_guard(mutables);
        futures.erase(
            std::remove_if(futures.begin(), futures.end(), [] (auto const &future) {
                return is_ready(future);
            }),
            futures.end());
        futures.emplace_back(std::move(future));
    }

    ~AsyncBin() { drain(); }

private:
    mutable std::mutex mutables;
    mutable std::vector<std::future<void>> futures;

    auto grab() const
    {
        auto g = std::lock_guard(mutables);
        return std::move(futures);
    }

    void drain() const { while (!grab().empty()) {} }
};

} // namespace

namespace Inkscape {
namespace Async {
namespace detail {

void extend(std::future<void> &&future) { AsyncBin::get().add(std::move(future)); }

} // namespace detail
} // namespace Async
} // namespace Inkscape
