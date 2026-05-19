#pragma once

#include "npy_writer.h"
#include "config.h"
#include <vector>
#include <thread>
#include <atomic>
#include <cstdint>

class LoggerRingBuffer
{
public:
    static constexpr size_t capacity = 32;

    LoggerRingBuffer() = default;
    LoggerRingBuffer(size_t element_size);

    bool push(const void* data); // false = full (drop)
    bool pop(void* data);        // false = empty

private:
    std::vector<uint8_t> buf_;
    size_t element_size_ = 0;
    std::atomic<size_t> head_{0}; // producer index
    std::atomic<size_t> tail_{0}; // consumer index
};

struct LogChannel
{
    LoggerRingBuffer ring;
    NpyWriter        writer;
};

class DataLogger
{
public:
    int build_logging_list(std::vector<DarttField*>& subscribed_list); // call under periph_buf_mutex, coupled with subscribed list rebuild
    void start();
    void stop();
    void package();

private:
    void file_writer_loop();
    std::thread                              fwriter_thread_;
    std::atomic<bool>                        running_{false};
    std::vector<std::unique_ptr<LogChannel>> channels_; // 1:1 index-mapped to subscribed_list
};
