#pragma once
#include <atomic>
#include <cstdint>
#include <cstddef>

template <size_t Capacity>
struct RingBuffer {
    static_assert((Capacity & (Capacity - 1)) == 0,
                  "Capacity must be power of 2");

    static constexpr size_t kCapacity = Capacity;
    static constexpr size_t kMask = Capacity - 1;

    RingBuffer() : head_(0), tail_(0) {}

    bool push(uint8_t byte) {
        size_t h = head_.load(std::memory_order_relaxed);
        size_t next = (h + 1) & kMask;
        if (next == tail_.load(std::memory_order_acquire)) {
            overflow_count_.fetch_add(1, std::memory_order_relaxed);
            return false;
        }
        buf_[h] = byte;
        head_.store(next, std::memory_order_release);
        return true;
    }

    bool pop(uint8_t &out) {
        size_t t = tail_.load(std::memory_order_relaxed);
        if (t == head_.load(std::memory_order_acquire)) {
            return false;
        }
        out = buf_[t];
        tail_.store((t + 1) & kMask, std::memory_order_release);
        return true;
    }

    size_t available() const {
        size_t h = head_.load(std::memory_order_acquire);
        size_t t = tail_.load(std::memory_order_acquire);
        if (h >= t) return h - t;
        return Capacity - t + h;
    }

    size_t overflow_count() const {
        return overflow_count_.load(std::memory_order_relaxed);
    }

    void clear_overflow() {
        overflow_count_.store(0, std::memory_order_relaxed);
    }

    void reset() {
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
        overflow_count_.store(0, std::memory_order_relaxed);
    }

private:
    uint8_t buf_[Capacity];
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
    std::atomic<size_t> overflow_count_;
};
