# Async Read Thread & Streaming Mode

A background reader thread replaces the blocking `dartt_read_multi` loop. It decodes COBS
frames, writes read replies into `periph_buf`, and keeps the render loop unblocked. Streaming
mode lets the device push read replies without requests.

## Source Layout

```
src/
  reader/            # DarttLink (dartt_link.h/.cpp), standalone CMake target
  wav_writer.h/.cpp  # WavWriter
  logger.*, log_channel.*  # DataLogger / per-field log rings
  plotting.*         # Plotter, plot_mutex
  buffer_sync.*      # periph_buf / field sync helpers
  main.cpp           # starts DarttLink, registers read-reply callback
```

## DarttLink

Sole owner of the transport handle. Two threads:

- **Read thread:** drains RX, decodes frames, updates `periph_buf`, fires the callback,
  dispatches the next read request.
- **Write thread:** sleeps on a condition variable, sends queued pre-encoded frames.

### Bus Arbitration

Full-duplex (`is_full_duplex`): `bus_mutex_` is skipped on the write thread.

Half-duplex (default): priority, highest first:

1. **Incoming RX:** read thread holds `bus_mutex_` from the first byte until the kernel buffer drains.
2. **Writes:** write thread takes `bus_mutex_` before sending.
3. **Read requests:** sent only when no writes are pending.

A read request holds `bus_mutex_` until its reply arrives or times out, so writes cannot
interleave. In gapless streaming, writes and requests starve by design; transmitting would
collide with the device.

### Writes

Render loop calls `enqueue_writes`, which chunks the slice to the RX buffer size, builds
COBS-encoded frames, and queues them. Pending writes are counted in `write_pending_`.

### Read Requests

- `subscribe_region` / `clear_subscriptions` edit `subscribed_regions_` (render thread only).
- `build_read_requests` rebuilds `read_request_list_` of pre-encoded frames (under `read_request_mutex_`), chunked to the TX buffer size.
- `dispatch_read_requests` runs on the read thread after each reply and on timeout. It returns if writes are pending; otherwise it sends the next frame round-robin.
- On timeout (`read_request_timeout_ms`) it sends the next request in the list; it does not repeat the timed-out one.

### Streaming Mode

`streaming_mode` is set by the UI, which clears subscriptions and rebuilds an empty request
list, so nothing is dispatched. The read thread accepts replies regardless.

## Read-Reply Callback

Fired on the read thread after `periph_buf` is updated. `periph_buf_mutex` is **not** held
during the callback. The `main.cpp` callback:

- Try-locks `periph_buf_mutex`; on failure the field update is skipped.
- Copies updated, non-dirty subscribed fields, computes display values, pushes to `log_ring`, notifies `DataLogger`, writes WAV samples.
- Try-locks `plot_mutex`; on failure the plot enqueue is skipped, so rendering cannot starve the read thread.

## Binary Frame Logger

Off by default. Appends each received read-reply COBS frame verbatim (including the `0x00`
delimiter, no timestamp) to a `.bin` file. Replayable through `cobs_stream`.

## WAV Writer

Off by default. `WavWriter` is global (one file for all subscribed fields). Samples are
buffered in memory and written on `close(sample_rate)`; the caller supplies the rate.

## Thread Safety

| Resource               | Accessed by                          | Protection           |
|------------------------|--------------------------------------|----------------------|
| Serial handle          | read (RX), write (TX)                | `bus_mutex_` (half-duplex) |
| `tx_queue_`            | render (push), write (pop)           | `tx_queue_mutex_` + `tx_cv_` |
| `read_request_list_`   | render (build), read (dispatch)      | `read_request_mutex_` |
| `subscribed_regions_`  | render only                          | none                 |
| `periph_buf`, field values | read (write), UI (read)          | `periph_buf_mutex`   |
| `Line::points`         | read (enqueue), render (iterate)     | `Plotter::plot_mutex` |
| `WavWriter` samples    | read (write), UI (close)             | `WavWriter::mutex_`  |
| `bin_log_file_`        | read thread; UI open/close           | none (**race**)      |

## Remaining Work

- Wire binary logger open/close into the UI.
- Guard `bin_log_file_` against concurrent open/close from the UI thread.
