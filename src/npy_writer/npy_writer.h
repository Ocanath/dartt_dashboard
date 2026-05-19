#pragma once
#include <cstdio>
#include <cstdint>
#include <string>
#include <atomic>

class NpyWriter
{
public:
    enum type {
        UINT8,
        UINT16,
        UINT32,
        UINT64,
        INT8,
        INT16,
        INT32,
        INT64,
        FLOAT32,
        DOUBLE64
    };
    int open(std::string name, type dtype);
    type get_dtype() const { return _dtype; }
    int add_uint8(uint8_t val);
    int add_uint16(uint16_t val);
    int add_uint32(uint32_t val);
    int add_uint64(uint64_t val);
    int add_int8(int8_t val);
    int add_int16(int16_t val);
    int add_int32(int32_t val);
    int add_int64(int64_t val);
    int add_float32(float val);
    int add_double64(double val);
    int close();
    static int open_count();
private:
    FILE*       _file         = nullptr;
    const char* _descr        = nullptr;
    uint64_t    _sample_count = 0;
    std::string _filename;
    type        _dtype;

    static std::atomic<int> _open_count;
};

