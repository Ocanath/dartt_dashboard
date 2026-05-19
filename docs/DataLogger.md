# Data Logger

### Overview
This class performs asynchronous filewriting/logging of npy files (numpy native data format) while preserving realtime performance of the dashboard. The TU contains a logger class definition and owns it's own ImGui window, with basic features like starting/stopping logging (which persists in the settings json), and .npz package filenames.

At a high level, the DataLogger plugs into the rest of the code in the following way:

1. Read callback iterates through the DarttFields in the subscribed list. Each new incoming element will get enqueued to a ring/circular FIFO buffer.
2. The DataLogger runs an async process for dispatching file writes via the NpyWriter class. It drains the ring buffer(s) as it writes.

The DataLogger owns a File Writer List, containing npy file writers and a ring buffer used to share data from the link to the logger. 

### Rebuilding the File Writer List

The numpy writer list is built from the subscribed fields list - i.e. the callsite to `build_logging_list` must be coupled with the subscribed list build callsite, and guarded with the same mutex for thread safety. It inherits the symbol names from its associated `DarttField` element and has a 1:1 association (index mapped) with each DarttField element. It carries both the `NpyWriter` instance and the `RingBuffer` instance for each symbol/subscribed field, which is used to transport the data from the I/O link to the logger. This follows a separate data path from plotting (for now), as it uses native data types to minimize logging file sizes- for instance, a stream of int16_t will be 2 or 4 times smaller in size than a `float` or `double` respectively, with no data loss. 

The memory in this list is accessed in three threads - the main rendering thread (`build_logging_list`), the write-to-file dispatch thread (`.pop()` and npy writer `add_` calls), and the link callback (`push()`). Thread safety is guaranteed for `pop()` and `push()` via a SPSC ring buffer implementation, and via mutex for the rendering/writing threads.

### Logger Ring Buffer

The ring buffer class `LoggerRingBuffer` is a canonical single-publisher single-consumer (SPSC) lock-free ring buffer. This means that there must be exactly *one* callsite to `push()` and one callsite to `pop()` for each instance, or else thread safety is not guaranteed. In this design, the `push()` callsite goes in the DarttLink read callback per field value, and the `pop()` callsite goes in the file writer thread.



