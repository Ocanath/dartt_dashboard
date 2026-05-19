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

//called within the main/GUI thread.
LoggerError DataLogger::build_logging_list(std::vector<DarttField*>& subscribed_list)
{
    std::lock_guard<std::mutex> lock(channels_mutex_);
    channels_.clear();
    channels_.reserve(subscribed_list.size());
    for (size_t i = 0; i < subscribed_list.size(); i++)
    {
        DarttField* field = subscribed_list[i];
        NpyWriter::type npy_type;
        if (!field_type_to_npy_type(field->type, npy_type))
        {
			return LOGGER_ERR_BAD_TYPE;
		}
        std::unique_ptr<LogChannel> ch = std::make_unique<LogChannel>(field->nbytes);
        if (ch->writer.open(field->name, npy_type) != 0)
		{
			return LOGGER_ERR_OPEN_FAILED;
		}
        channels_.push_back(std::move(ch));
    }
    return LOGGER_OK;
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
