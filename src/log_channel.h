#pragma once
#include "npy_writer.h"
#include <vector>
#include <atomic>
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

