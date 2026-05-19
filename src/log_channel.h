#pragma once
#include "npy_writer.h"
#include <vector>
#include <atomic>
#include <memory>
#include <cstdint>

// ============================================================================
// LoggerRingBuffer — canonical SPSC lock-free ring buffer
// One push() callsite (read callback), one pop() callsite (writer thread).
// ============================================================================

class LoggerRingBuffer
{
public:
    static constexpr size_t capacity = 32;

    LoggerRingBuffer() = default;
    LoggerRingBuffer(size_t element_size);

    bool push(const void* data, size_t size); // false = full (drop)
    bool pop(void* data, size_t size);        // false = empty

private:
    std::vector<uint8_t> buf_;
    size_t element_size_ = 0;
    std::atomic<size_t> head_{0}; // producer index
    std::atomic<size_t> tail_{0}; // consumer index
};

// ============================================================================
// LogChannel — ring buffer + npy writer pair, one per subscribed field
// ============================================================================

struct LogChannel
{
    LogChannel(size_t element_size) : ring(element_size) {}
    LoggerRingBuffer ring;
    NpyWriter        writer;
};

// ============================================================================
// LogChannelHandle — owns a LogChannel via unique_ptr with rule of 5.
// Isolates non-copyable/non-movable members from DarttField, following the
// same pattern as DarttFieldState for std::atomic.
// Copy = null (logging state does not transfer on field copy).
// ============================================================================

struct LogChannelHandle
{
    LogChannelHandle() = default;
    LogChannelHandle(const LogChannelHandle&);
    LogChannelHandle(LogChannelHandle&&) noexcept;
    LogChannelHandle& operator=(const LogChannelHandle&);
    LogChannelHandle& operator=(LogChannelHandle&&) noexcept;
    ~LogChannelHandle();

    std::unique_ptr<LogChannel> ptr;
};
