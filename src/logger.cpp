#include "logger.h"
#include <cstring>

union LogVal {
    uint8_t  u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    int8_t   i8;
    int16_t  i16;
    int32_t  i32;
    int64_t  i64;
    float    f32;
    double   f64;
};

static bool drain_ring_buffer(LogChannel* pLogChannel)
{
    if (pLogChannel == nullptr)
        return false;

    LogVal val = {};
    NpyWriter::type dtype = pLogChannel->writer.get_dtype();
    while (pLogChannel->ring.pop(&val, sizeof(val)))
    {
        switch (dtype)
        {
            case NpyWriter::UINT8:
            {
                pLogChannel->writer.add_uint8(val.u8);
                break;
            }
            case NpyWriter::UINT16:
            {
                pLogChannel->writer.add_uint16(val.u16);
                break;
            }
            case NpyWriter::UINT32:
            {
                pLogChannel->writer.add_uint32(val.u32);
                break;
            }
            case NpyWriter::UINT64:
            {
                pLogChannel->writer.add_uint64(val.u64);
                break;
            }
            case NpyWriter::INT8:
            {
                pLogChannel->writer.add_int8(val.i8);
                break;
            }
            case NpyWriter::INT16:
            {
                pLogChannel->writer.add_int16(val.i16);
                break;
            }
            case NpyWriter::INT32:
            {
                pLogChannel->writer.add_int32(val.i32);
                break;
            }
            case NpyWriter::INT64:
            {
                pLogChannel->writer.add_int64(val.i64);
                break;
            }
            case NpyWriter::FLOAT32:
            {
                pLogChannel->writer.add_float32(val.f32);
                break;
            }
            case NpyWriter::DOUBLE64:
            {
                pLogChannel->writer.add_double64(val.f64);
                break;
            }
            default:
            {
                break;
            }
        }
    }
    return true;
}

LoggerRingBuffer* DataLogger::add_channel(const std::string& filename,
                                           NpyWriter::type dtype,
                                           size_t element_size)
{
    std::unique_ptr<LogChannel> ch = std::make_unique<LogChannel>(element_size);
    if (ch->writer.open(filename, dtype) != 0)
        return nullptr;
    LoggerRingBuffer* ring = &ch->ring;
    channels_.push_back(std::move(ch));
    return ring;
}

void DataLogger::clear_channels()
{
    channels_.clear();
}

void DataLogger::start()
{
    running_ = true;
    fwriter_thread_ = std::thread(&DataLogger::file_writer_loop, this);
}

void DataLogger::stop()
{
    running_ = false;
    cv_.notify_one();
    if (fwriter_thread_.joinable())
        fwriter_thread_.join();
}

void DataLogger::notify()
{
    cv_.notify_one();
}

void DataLogger::package()
{
}

void DataLogger::file_writer_loop()
{
    while (running_)
    {
        {
            std::unique_lock<std::mutex> cv_lock(cv_mutex_);
            cv_.wait(cv_lock);
        }
        if (!running_)
            break;

        // if (periph_buf_mutex_ && periph_buf_mutex_->try_lock())//replace with channels_mutex_ try lock on next commit
        {
            // std::lock_guard<std::mutex> lock(*periph_buf_mutex_, std::adopt_lock);
            for (size_t i = 0; i < channels_.size(); i++)
            {
				drain_ring_buffer(channels_[i].get());
			}
        }
    }
    package();
}
