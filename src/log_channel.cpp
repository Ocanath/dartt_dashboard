#include "log_channel.h"
#include <cassert>
#include <cstring>

// ============================================================================
// LoggerRingBuffer
// ============================================================================

LoggerRingBuffer::LoggerRingBuffer(size_t element_size)
    : element_size_(element_size)
{
    buf_.resize(element_size * capacity);
}

bool LoggerRingBuffer::push(const void* data, size_t size)
{
    assert(size >= element_size_);
    size_t head = head_.load(std::memory_order_relaxed);
    size_t tail = tail_.load(std::memory_order_acquire);
    if ((head - tail) >= capacity)
    {
        return false;
    }
    std::memcpy(buf_.data() + (head % capacity) * element_size_, data, element_size_);
    head_.store(head + 1, std::memory_order_release);
    return true;
}

bool LoggerRingBuffer::pop(void* data, size_t size)
{
    assert(size >= element_size_);
    size_t tail = tail_.load(std::memory_order_relaxed);
    size_t head = head_.load(std::memory_order_acquire);
    if (head == tail)
    {
        return false;
    }
    std::memcpy(data, buf_.data() + (tail % capacity) * element_size_, element_size_);
    tail_.store(tail + 1, std::memory_order_release);
    return true;
}

// ============================================================================
// LogChannelHandle — rule of 5
// All definitions are out-of-line so that LogChannel is fully defined at the
// point where unique_ptr<LogChannel> is constructed, reset, or destroyed.
// ============================================================================

LogChannelHandle::LogChannelHandle(const LogChannelHandle&)
{
    // copy = null: logging state does not transfer on field copy
}

LogChannelHandle::LogChannelHandle(LogChannelHandle&& o) noexcept
    : ptr(std::move(o.ptr))
{
}

LogChannelHandle& LogChannelHandle::operator=(const LogChannelHandle&)
{
    ptr.reset();
    return *this;
}

LogChannelHandle& LogChannelHandle::operator=(LogChannelHandle&& o) noexcept
{
    ptr = std::move(o.ptr);
    return *this;
}

LogChannelHandle::~LogChannelHandle() = default;
