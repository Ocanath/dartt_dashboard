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
    int add_sample(void* data, size_t size, type dtype);
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

