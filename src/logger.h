#pragma once

#include "log_channel.h"
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <string>

enum LoggerError {
    LOGGER_OK              =  0,
    LOGGER_ERR_OPEN_FAILED = -2,
    LOGGER_ERR_BUSY        = -3,
};

class DataLogger
{
public:
    // Returns pointer to the channel's ring buffer, or nullptr on open failure.
    // Must be called under periph_buf_mutex.
    LoggerRingBuffer* add_channel(const std::string& filename,
                                  NpyWriter::type dtype,
                                  size_t element_size);

    // Destroy all channels. Must be protected under the same mutex guarding the add_channel wiring before rebuilding.
    void clear_channels();

    void start();
    void stop();
    void package();
    void notify(); // call from read callback after pushing data
    bool is_running() const { return running_; }

private:
    void file_writer_loop();

    std::vector<std::unique_ptr<LogChannel>> channels_;
    std::thread              fwriter_thread_;
    std::atomic<bool>        running_{false};
    std::condition_variable  cv_;
    std::mutex               cv_mutex_;
};
