# Data Logger

### Overview
This class performs asynchronous filewriting/logging of npy files (numpy native data format) while preserving realtime performance of the dashboard. The TU contains a logger class definition and owns its own ImGui window, with basic features like starting/stopping logging (which persists in the settings json), and .npz package filenames.

At a high level, the DataLogger plugs into the rest of the code in the following way:

1. Read callback iterates the subscribed list. For each updated field, if `field->log_ring` is non-null, the raw value is pushed to the ring buffer. After the loop, `DataLogger::notify()` is called once to wake the writer thread.
2. The DataLogger runs an async process for dispatching file writes via the NpyWriter class. It drains its internal channel ring buffers as it writes.

### Ownership and Portability

`DataLogger` is a self-contained, portable module. It has no knowledge of `DarttField` or `FieldType` — it accepts only primitive inputs and exposes a `LoggerRingBuffer*` handle back to the caller.

`DataLogger` owns a `std::vector<std::unique_ptr<LogChannel>> channels_`. Each `LogChannel` holds one `LoggerRingBuffer` and one `NpyWriter`. Adding a channel is done via:

```cpp
LoggerRingBuffer* add_channel(const std::string& filename, NpyWriter::type dtype, size_t element_size);
```

The returned pointer is stored in `DarttField::log_ring` (a raw non-owning pointer, null when not logging). All wiring — including the `FieldType` to `NpyWriter::type` conversion — lives in the application integration layer.

`LogChannel` contains a `LoggerRingBuffer` with `std::atomic` members, making it non-movable. `unique_ptr` is used so the vector can hold them without moves, and the returned ring buffer pointer remains stable for the lifetime of the channel.

### Rebuilding the Channel List

When the subscribed list is rebuilt (on `subscribed_dirty`), the sequence under `periph_buf_mutex` is:

1. Null all `field->log_ring` pointers on the old subscribed list — stops the read callback from pushing to stale rings. Critical to prevent use-after-free or similar memory corruption issues.
2. Rebuild the subscribed list via `collect_subscribed_fields`.
3. If the logger is running: call `data_logger.clear_channels()`, then for each new subscribed field call `add_channel` and store the returned pointer in `field->log_ring`. Note - this means enabling logging at the top level must dispatch a new subscribed list rebuild.

This guarantees no dangling pointer use: the ring pointers are nulled before the channels are destroyed, and both operations happen under the same mutex that the writer thread respects.

### Logger Ring Buffer

The ring buffer class `LoggerRingBuffer` is a canonical single-publisher single-consumer (SPSC) lock-free ring buffer. There must be exactly *one* callsite to `push()` and one to `pop()` per instance. In this design, `push()` is called from the read callback (one thread), and `pop()` is called from the file writer thread.

### Writer Thread Wakeup

The file writer thread blocks on a `std::condition_variable` (`cv_`) rather than sleeping. The read callback calls `DataLogger::notify()` once per frame (after pushing all updated fields), which signals the cv and wakes the writer thread immediately. This avoids system timer resolution floors, which are on the order of milliseconds and would otherwise create overflow risk at high baud rates. `DataLogger::stop()` also signals the cv so the thread can observe `running_ = false` and exit cleanly.
