#pragma once

#include "npy_writer.h"
#include "config.h"
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <cstdint>

enum LoggerError {
    LOGGER_OK               =  0,
    LOGGER_ERR_BAD_TYPE     = -1, // non-primitive field type in subscribed list
    LOGGER_ERR_OPEN_FAILED  = -2, // NpyWriter::open() failed
    LOGGER_ERR_BUSY         = -3, // operation not permitted while logger is running
};

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

struct LogChannel
{
    LogChannel(size_t element_size) : ring(element_size) {}
    LoggerRingBuffer ring;
    NpyWriter        writer;
};

class DataLogger
{
public:
    LoggerError build_logging_list(std::vector<DarttField*>& subscribed_list); // call under periph_buf_mutex, coupled with subscribed list rebuild
    void start();
    void stop();
    void package();
    void notify();                                          // signal writer thread that data is available; call from read callback after push
    void push(size_t channel_idx, const void* data, size_t nbytes); // call from read callback, 1:1 indexed with subscribed_list

private:
    void file_writer_loop();
    std::thread                              fwriter_thread_;
    std::atomic<bool>                        running_{false};
	/*new language feature (to me:) unique_ptr, which does the work of creating a heap-allocated instance and a pointer reference to it, with automatic memory management*/
    std::vector<std::unique_ptr<LogChannel>> channels_; // 1:1 index-mapped to subscribed_list. Using unique_ptr for automatic heap allocation mapping - due to LogChannel atomics
	std::mutex              channels_mutex_;
    std::condition_variable cv_;
    std::mutex              cv_mutex_;
};
