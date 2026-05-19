#include "logger.h"
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

bool LoggerRingBuffer::push(const void* data)
{
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

bool LoggerRingBuffer::pop(void* data)
{
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
// DataLogger
// ============================================================================

int DataLogger::build_logging_list(std::vector<DarttField*>& subscribed_list)
{
    return 0;
}

void DataLogger::start()
{
}

void DataLogger::stop()
{
}

void DataLogger::package()
{
}

void DataLogger::file_writer_loop()
{
}
