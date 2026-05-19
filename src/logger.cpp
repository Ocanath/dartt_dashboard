#include "logger.h"
#include "config.h"
#include <cassert>
#include <cstring>

// ============================================================================
// Internal helpers
// ============================================================================

static bool field_type_to_npy_type(FieldType ftype, NpyWriter::type& out)
{
    switch (ftype)
    {
        case FieldType::UINT8:
        {
            out = NpyWriter::UINT8;
            return true;
        }
        case FieldType::UINT16:
        {
            out = NpyWriter::UINT16;
            return true;
        }
        case FieldType::UINT32:
        {
            out = NpyWriter::UINT32;
            return true;
        }
        case FieldType::UINT64:
        {
            out = NpyWriter::UINT64;
            return true;
        }
        case FieldType::INT8:
        {
            out = NpyWriter::INT8;
            return true;
        }
        case FieldType::INT16:
        {
            out = NpyWriter::INT16;
            return true;
        }
        case FieldType::INT32:
        {
            out = NpyWriter::INT32;
            return true;
        }
        case FieldType::INT64:
        {
            out = NpyWriter::INT64;
            return true;
        }
        case FieldType::FLOAT:
        {
            out = NpyWriter::FLOAT32;
            return true;
        }
        case FieldType::DOUBLE:
        {
            out = NpyWriter::DOUBLE64;
            return true;
        }
        default:
        {
            return false;
        }
    }
}

/*helper function to drain the ring buffer of a log channel and write to file*/
static bool drain_ring_buffer(LogChannel* pLogChannel)
{
    if (pLogChannel == nullptr)
    {
        return false;
    }
    DarttValue val = {};
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

// ============================================================================
// DataLogger
// ============================================================================

void DataLogger::init(std::mutex& periph_buf_mutex)
{
    periph_buf_mutex_ = &periph_buf_mutex;
}

// called within the main/GUI thread under periph_buf_mutex
LoggerError DataLogger::build_logging_list(std::vector<DarttField*>& subscribed_list)
{
    // clear log channels from previous subscribed list
    if (subscribed_list_ != nullptr)
    {
        for (size_t i = 0; i < subscribed_list_->size(); i++)
        {
            (*subscribed_list_)[i]->log_channel.ptr.reset();
        }
    }

    subscribed_list_ = &subscribed_list;

    for (size_t i = 0; i < subscribed_list.size(); i++)
    {
        DarttField* field = subscribed_list[i];
        NpyWriter::type npy_type;
        if (!field_type_to_npy_type(field->type, npy_type))
        {
            return LOGGER_ERR_BAD_TYPE;
        }
        field->log_channel.ptr = std::make_unique<LogChannel>(field->nbytes);
        if (field->log_channel.ptr->writer.open(field->name, npy_type) != 0)
        {
            return LOGGER_ERR_OPEN_FAILED;
        }
    }
    return LOGGER_OK;
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
    {
        fwriter_thread_.join();
    }
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
        {
            break;
        }
        if (periph_buf_mutex_ && periph_buf_mutex_->try_lock())
        {
            std::lock_guard<std::mutex> lock(*periph_buf_mutex_, std::adopt_lock);
            if (subscribed_list_ != nullptr)
            {
                for (size_t i = 0; i < subscribed_list_->size(); i++)
                {
                    DarttField* field = (*subscribed_list_)[i];
                    if (field->log_channel.ptr)
                    {
                        drain_ring_buffer(field->log_channel.ptr.get());
                    }
                }
            }
        }
    }
    package();
}
