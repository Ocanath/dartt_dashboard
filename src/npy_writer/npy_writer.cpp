#include "npy_writer.h"
#include <cstring>
#include <cinttypes>

// 10 (magic + version + header_len field) + HEADER_LEN must be a multiple of 64
// 10 + 118 = 128 = 2×64
static constexpr uint16_t HEADER_LEN  = 118;
static constexpr size_t   IO_BUF_SIZE = 64 * 1024;

std::atomic<int> NpyWriter::_open_count{0};

static const char* to_descr(NpyWriter::type t)
{
    switch (t) {
        case NpyWriter::UINT8:    return "|u1";
        case NpyWriter::UINT16:   return "<u2";
        case NpyWriter::UINT32:   return "<u4";
        case NpyWriter::UINT64:   return "<u8";
        case NpyWriter::INT8:     return "|i1";
        case NpyWriter::INT16:    return "<i2";
        case NpyWriter::INT32:    return "<i4";
        case NpyWriter::INT64:    return "<i8";
        case NpyWriter::FLOAT32:  return "<f4";
        case NpyWriter::DOUBLE64: return "<f8";
        default:                  return "<f4";
    }
}

static void write_header(FILE* f, const char* descr, uint64_t count)
{
    char dict[HEADER_LEN];
    int n = snprintf(dict, sizeof(dict),
        "{'descr': '%s', 'fortran_order': False, 'shape': (%" PRIu64 ",), }",
        descr, count);
    memset(dict + n, ' ', HEADER_LEN - 1 - n);
    dict[HEADER_LEN - 1] = '\n';

    const uint8_t magic[] = {0x93, 'N', 'U', 'M', 'P', 'Y', 0x01, 0x00};
    const uint8_t hlen[2] = { HEADER_LEN & 0xFF, (HEADER_LEN >> 8) & 0xFF };
    fwrite(magic, 1, 8, f);
    fwrite(hlen,  1, 2, f);
    fwrite(dict,  1, HEADER_LEN, f);
}

int NpyWriter::open(std::string name, type dtype)
{
    _file = fopen(name.c_str(), "wb");
    if (!_file)
        return -1;
    setvbuf(_file, nullptr, _IOFBF, IO_BUF_SIZE);
    _descr        = to_descr(dtype);
    _dtype        = dtype;
    _sample_count = 0;
    _filename     = name;
    write_header(_file, _descr, 0);
    _open_count.fetch_add(1, std::memory_order_relaxed);
    return 0;
}

// ---- 1-byte types ----

int NpyWriter::add_uint8(uint8_t val)
{
    if (!_file)
	{
		return -1;
	}
    if (_dtype != UINT8)
	{
		return -2;
	} 
    fwrite(&val, 1, 1, _file);
    _sample_count++;
    return 0;
}

int NpyWriter::add_int8(int8_t val)
{
    if (!_file) 
	{
		return -1;
	}
    if (_dtype != INT8) 
	{
		return -2;
	}
    uint8_t b = (uint8_t)(val);
    fwrite(&b, 1, 1, _file);
    _sample_count++;
    return 0;
}

// ---- 2-byte types ----

int NpyWriter::add_uint16(uint16_t val)
{
    if (!_file)
	{
		return -1;
	}
    if (_dtype != UINT16) 
	{
		return -2;
	}

	unsigned char buf[sizeof(uint16_t)];

	for(size_t i = 0; i < sizeof(uint16_t); i++)
	{
		int shift = (int)i*8;
		buf[i] = (unsigned char)((val  >> shift) & 0xFFu);
	}

    fwrite(buf, 1, sizeof(uint16_t), _file);
    _sample_count++;
    return 0;
}

int NpyWriter::add_int16(int16_t val)
{
    if (!_file) 
	{
		return -1;
	}
    if (_dtype != INT16) 
	{
		return -2;
	}
    uint16_t u     = (uint16_t)(val);
	unsigned char buf[sizeof(uint16_t)];

	for(size_t i = 0; i < sizeof(uint16_t); i++)
	{
		int shift = (int)i*8;
		buf[i] = (unsigned char)((u  >> shift) & 0xFFu);
	}

    fwrite(buf, 1, sizeof(uint16_t), _file);
    _sample_count++;
    return 0;
}

// ---- 4-byte types ----

int NpyWriter::add_uint32(uint32_t val)
{
    if (!_file) 
	{
		return -1;
	}
    if (_dtype != UINT32) 
	{
		return -2;
	}
	unsigned char buf[sizeof(uint32_t)];

	for(size_t i = 0; i < sizeof(uint32_t); i++)
	{
		int shift = (int)i*8;
		buf[i] = (unsigned char)((val  >> shift) & 0xFFu);
	}

    fwrite(buf, 1, sizeof(uint32_t), _file);
    _sample_count++;
    return 0;
}

int NpyWriter::add_int32(int32_t val)
{
    if (!_file) 
	{
		return -1;
	}
    if (_dtype != INT32) 
	{
		return -2;
	}
    uint32_t u     = (uint32_t)(val);
	unsigned char buf[sizeof(uint32_t)];

	for(size_t i = 0; i < sizeof(uint32_t); i++)
	{
		int shift = (int)i*8;
		buf[i] = (unsigned char)((u  >> shift) & 0xFFu);
	}

    fwrite(buf, 1, sizeof(uint32_t), _file);
    _sample_count++;
    return 0;
}

int NpyWriter::add_float32(float val)
{
    if (!_file) 
	{
		return -1;
	}
    if (_dtype != FLOAT32) 
	{
		return -2;
	}
    uint32_t u;
    memcpy(&u, &val, sizeof(u));

	unsigned char buf[sizeof(uint32_t)];
	for(size_t i = 0; i < sizeof(uint32_t); i++)
	{
		int shift = (int)i*8;
		buf[i] = (unsigned char)((u  >> shift) & 0xFFu);
	}

    fwrite(buf, 1, 4, _file);
    _sample_count++;
    return 0;
}

// ---- 8-byte types ----

int NpyWriter::add_uint64(uint64_t val)
{
    if (!_file)
	{
		return -1;
	}
    if (_dtype != UINT64) 
	{
		return -2;
	}
    
	uint8_t buf[sizeof(uint64_t)];
    for (size_t i = 0; i < sizeof(uint64_t); i++)
	{
		int shift = (int)i*8;
		buf[i] = (unsigned char)((val  >> shift) & 0xFFu);
	}

    fwrite(buf, 1, sizeof(uint64_t), _file);
    _sample_count++;
    return 0;
}

int NpyWriter::add_int64(int64_t val)
{
    if (!_file)
	{
		return -1;
	}
    if (_dtype != INT64) 
	{
		return -2;
	}
    uint64_t u = (uint64_t)(val);
	uint8_t buf[sizeof(uint64_t)];
    for (size_t i = 0; i < sizeof(uint64_t); i++)
	{
		int shift = (int)i*8;
		buf[i] = (unsigned char)((u  >> shift) & 0xFFu);
	}
    fwrite(buf, 1, sizeof(uint64_t), _file);
    _sample_count++;
    return 0;
}

int NpyWriter::add_double64(double val)
{
    if (!_file) 
	{
		return -1;
	}
    if (_dtype != DOUBLE64) 
	{
		return -2;
	}
    uint64_t u;
    memcpy(&u, &val, sizeof(u));
	uint8_t buf[sizeof(uint64_t)];
    for (size_t i = 0; i < sizeof(uint64_t); i++)
	{
		int shift = (int)i*8;
		buf[i] = (unsigned char)((u  >> shift) & 0xFFu);
	}
    fwrite(buf, 1, sizeof(uint64_t), _file);
    _sample_count++;
    return 0;
}

// ---- lifecycle ----

int NpyWriter::close()
{
    if (!_file)
	{
        return -1;
	}
    fflush(_file);
    fseek(_file, 0, SEEK_SET);
    write_header(_file, _descr, _sample_count);
    fclose(_file);
    _file = nullptr;
    _open_count.fetch_sub(1, std::memory_order_relaxed);
    printf("Wrote %s (%llu samples)\n", _filename.c_str(), (unsigned long long)_sample_count);
    return 0;
}

int NpyWriter::open_count()
{
    return _open_count.load(std::memory_order_relaxed);
}
