#pragma once

#include "log_channel.h"

struct DarttField; // full definition in config.h
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

enum LoggerError {
    LOGGER_OK               =  0,
    LOGGER_ERR_BAD_TYPE     = -1, // non-primitive field type in subscribed list
    LOGGER_ERR_OPEN_FAILED  = -2, // NpyWriter::open() failed
    LOGGER_ERR_BUSY         = -3, // operation not permitted while logger is running
};

class DataLogger
{
public:
    void init(std::mutex& periph_buf_mutex);
    LoggerError build_logging_list(std::vector<DarttField*>& subscribed_list); // call under periph_buf_mutex
	LoggerError clean_logging_list(std::vector<DarttField*>& subscribed_list); // call under periph_buf_mutex
    void start();
    void stop();
    void package();
    void notify(); // call from read callback after pushing to field->log_channel
    bool is_running() const { return running_; }

private:
    void file_writer_loop();
    std::thread              fwriter_thread_;
    std::atomic<bool>        running_{false};
    std::mutex*              periph_buf_mutex_ = nullptr;
	std::vector<DarttField*> * p_subscribed_list_ = nullptr;
    std::condition_variable  cv_;
    std::mutex               cv_mutex_;
};
