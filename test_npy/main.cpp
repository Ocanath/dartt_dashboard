#include "npy_writer.h"
#include <cstdio>

int main()
{
    // float32 ramp: 0.0, 1.0, ..., 63.0
    {
        NpyWriter w;
        if (w.open("test_float32.npy", NpyWriter::FLOAT32) != 0) {
            fprintf(stderr, "failed to open test_float32.npy\n");
            return 1;
        }
        for (int i = 0; i < 64; i++) {
            float v = (float)i;
            w.add_float32(v);
        }
        w.close();
        printf("wrote test_float32.npy (64 samples)\n");
    }

	//double
	{
        NpyWriter w;
        if (w.open("test_double64.npy", NpyWriter::DOUBLE64) != 0) {
            fprintf(stderr, "failed to open test_double64.npy\n");
            return 1;
        }
        for (int i = 0; i < 64; i++) {
            double v = (double)i/2.;
            w.add_double64(v);
        }
        w.close();
        printf("wrote test_double64.npy (64 samples)\n");
    }

    // uint32 ramp: 0, 1, ..., 31
    {
        NpyWriter w;
        if (w.open("test_uint32.npy", NpyWriter::UINT32) != 0) {
            fprintf(stderr, "failed to open test_uint32.npy\n");
            return 1;
        }
        for (uint32_t i = 0; i < 32; i++)
            w.add_uint32(i);
        w.close();
        printf("wrote test_uint32.npy (32 samples)\n");
    }

	//int32
    {
        NpyWriter w;
        if (w.open("test_int32.npy", NpyWriter::INT32) != 0) {
            fprintf(stderr, "failed to open test_int32.npy\n");
            return 1;
        }
        for (int32_t i = -10; i < 32; i++)
		{
            w.add_int32(i);
		}
        w.close();
        printf("wrote test_int32.npy (32 samples)\n");
    }

	//int16
    {
        NpyWriter w;
        if (w.open("test_int16.npy", NpyWriter::INT16) != 0) {
            fprintf(stderr, "failed to open test_int16.npy\n");
            return 1;
        }
        for (int32_t i = -10; i < 32; i++)
		{
            w.add_int16(i);
		}
        w.close();
        printf("wrote test_int16.npy (32 samples)\n");
    }
	//uint16
    {
        NpyWriter w;
        if (w.open("test_uint16.npy", NpyWriter::UINT16) != 0) {
            fprintf(stderr, "failed to open test_uint16.npy\n");
            return 1;
        }
        for (int32_t i = 0; i < 32; i++)
		{
            w.add_uint16(i);
		}
        w.close();
        printf("wrote test_uint16.npy (32 samples)\n");
    }
	//int8
    {
        NpyWriter w;
        if (w.open("test_int8.npy", NpyWriter::INT8) != 0) {
            fprintf(stderr, "failed to open test_int8.npy\n");
            return 1;
        }
        for (int32_t i = -10; i < 32; i++)
		{
            w.add_int8(i);
		}
        w.close();
        printf("wrote test_int8.npy (32 samples)\n");
    }

	//uint8
    {
        NpyWriter w;
        if (w.open("test_uint8.npy", NpyWriter::UINT8) != 0) {
            fprintf(stderr, "failed to open test_uint8.npy\n");
            return 1;
        }
        for (int32_t i = 0; i < 32; i++)
		{
            w.add_uint8(i);
		}
        w.close();
        printf("wrote test_uint8.npy (32 samples)\n");
    }


	//uint64
    {
        NpyWriter w;
        if (w.open("test_uint64.npy", NpyWriter::UINT64) != 0) {
            fprintf(stderr, "failed to open test_uint64.npy\n");
            return 1;
        }
        for (int32_t i = 0; i < 32; i++)
		{
            w.add_uint64(i);
		}
        w.close();
        printf("wrote test_uint64.npy (32 samples)\n");
    }

	
	//int64
    {
        NpyWriter w;
        if (w.open("test_int64.npy", NpyWriter::INT64) != 0) {
            fprintf(stderr, "failed to open test_int64.npy\n");
            return 1;
        }
        for (int32_t i = -10; i < 32; i++)
		{
            w.add_int64(i);
		}
        w.close();
        printf("wrote test_int64.npy (32 samples)\n");
    }

    return 0;
}
