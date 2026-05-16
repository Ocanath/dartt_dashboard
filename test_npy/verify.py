import numpy as np

files = [
    "test_float32.npy",
    "test_double64.npy",
    "test_uint8.npy",
    "test_uint16.npy",
    "test_uint32.npy",
    "test_uint64.npy",
    "test_int8.npy",
    "test_int16.npy",
    "test_int32.npy",
    "test_int64.npy",
]

for f in files:
    arr = np.load(f)
    print(f"{f}: dtype={arr.dtype}, shape={arr.shape}")
    print(f"  {arr}")
    print()
