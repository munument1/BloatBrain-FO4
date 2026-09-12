#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <utility>

namespace BloatBrain
{
    // A one-slot handoff between the game thread and network worker. Values are
    // copied under the lock so neither side retains shared mutable state.
    // Clearing also resets the tick epoch after a connection reset.
    template <class T>
    class LatestMailbox
    {
    public:
        [[nodiscard]] bool Publish(T value)
        {
            std::scoped_lock lock(mutex_);
            if (latest_ && value.tick <= latest_->tick) {
                return false;
            }
            latest_ = std::move(value);
            return true;
        }

        [[nodiscard]] std::optional<T> Snapshot() const
        {
            std::scoped_lock lock(mutex_);
            return latest_;
        }

        [[nodiscard]] std::optional<T> SnapshotNewerThan(std::uint64_t tick) const
        {
            std::scoped_lock lock(mutex_);
            if (!latest_ || latest_->tick <= tick) {
                return std::nullopt;
            }
            return latest_;
        }

        void Clear()
        {
            std::scoped_lock lock(mutex_);
            latest_.reset();
        }

    private:
        mutable std::mutex mutex_;
        std::optional<T> latest_;
    };
}
