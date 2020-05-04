#pragma once

#include <mutex>
#include <atomic>

namespace cube
{
    using Mutex = std::mutex;
    using Lock = std::unique_lock<Mutex>;
    template <typename T>
    using Atomic = std::atomic<T>;

    using ThreadSignal = std::condition_variable;
} // namespace cube
