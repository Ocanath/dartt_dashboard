#include "npy_writer.h"
#include <cstdio>
#include <cstdint>

int main()
{
    {
        NpyWriter w;
        if (w.open("test_float32.npy", NpyWriter::FLOAT32) != 0)
		{
            fprintf(stderr, "failed to open test_float32.npy\n");
            return 1;
        }
        int i;
        for (i = 0; i < 64; i++)
        {
			w.add_float32((float)i);
		}
        w.close();
        printf("wrote test_float32.npy (%d samples)\n", i);
    }

    {
        NpyWriter w;
        if (w.open("test_double64.npy", NpyWriter::DOUBLE64) != 0) 
		{
            fprintf(stderr, "failed to open test_double64.npy\n");
            return 1;
        }
        int i;
        for (i = 0; i < 64; i++)
		{
			w.add_double64((double)i / 2.0);
		}
        w.close();
        printf("wrote test_double64.npy (%d samples)\n", i);
    }

    {
        NpyWriter w;
        if (w.open("test_uint8.npy", NpyWriter::UINT8) != 0) 
		{
            fprintf(stderr, "failed to open test_uint8.npy\n");
            return 1;
        }
        uint8_t i;
        for (i = 0; i < 32; i++)
        {
 			w.add_uint8(i);
		}
        w.close();
        printf("wrote test_uint8.npy (%d samples)\n", (int)i);
    }

    {
        NpyWriter w;
        if (w.open("test_uint16.npy", NpyWriter::UINT16) != 0) 
		{
            fprintf(stderr, "failed to open test_uint16.npy\n");
            return 1;
        }
        uint16_t i;
        for (i = 0; i < 32; i++)
		{
			w.add_uint16(i);
		}
        w.close();
        printf("wrote test_uint16.npy (%d samples)\n", (int)i);
    }

    {
        NpyWriter w;
        if (w.open("test_uint32.npy", NpyWriter::UINT32) != 0) 
		{
            fprintf(stderr, "failed to open test_uint32.npy\n");
            return 1;
        }
        uint32_t i;
        for (i = 0; i < 32; i++)
        {
			w.add_uint32(i);
		}
        w.close();
        printf("wrote test_uint32.npy (%u samples)\n", i);
    }

    {
        NpyWriter w;
        if (w.open("test_uint64.npy", NpyWriter::UINT64) != 0) 
		{
            fprintf(stderr, "failed to open test_uint64.npy\n");
            return 1;
        }
        uint64_t i;
        for (i = 0; i < 32; i++)
		{
			w.add_uint64(i);
		}
        w.close();
        printf("wrote test_uint64.npy (%llu samples)\n", (unsigned long long)i);
    }

    {
        NpyWriter w;
        if (w.open("test_int8.npy", NpyWriter::INT8) != 0) 
		{
            fprintf(stderr, "failed to open test_int8.npy\n");
            return 1;
        }
        int8_t i;
        for (i = -10; i < 32; i++)
		{
			w.add_int8(i);
		}
        w.close();
        printf("wrote test_int8.npy (i ends at %d)\n", (int)i);
    }

    {
        NpyWriter w;
        if (w.open("test_int16.npy", NpyWriter::INT16) != 0) 
		{
            fprintf(stderr, "failed to open test_int16.npy\n");
            return 1;
        }
        int16_t i;
        for (i = -10; i < 32; i++)
		{
			w.add_int16(i);
		}
        w.close();
        printf("wrote test_int16.npy (i ends at %d)\n", (int)i);
    }

    {
        NpyWriter w;
        if (w.open("test_int32.npy", NpyWriter::INT32) != 0) 
		{
            fprintf(stderr, "failed to open test_int32.npy\n");
            return 1;
        }
        int32_t i;
        for (i = -10; i < 32; i++)
		{
			w.add_int32(i);
		}
        w.close();
        printf("wrote test_int32.npy (i ends at %d)\n", i);
    }

    {
        NpyWriter w;
        if (w.open("test_int64.npy", NpyWriter::INT64) != 0) 
		{
            fprintf(stderr, "failed to open test_int64.npy\n");
            return 1;
        }
        int64_t i;
        for (i = -10; i < 32; i++)
		{
			w.add_int64(i);
		}
        w.close();
        printf("wrote test_int64.npy (i ends at %lld)\n", (long long)i);
    }

    return 0;
}
