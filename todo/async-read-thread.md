# Async Read Thread & Streaming Mode

## Overview

Replace the synchronous `dartt_read_multi` polling loop (currently blocking inside the
ImGui event loop) with a dedicated background reader thread. The reader continuously
drains the RX kernel buffer, decodes COBS frames, parses dartt read-reply payloads, and
writes results directly into `periph_buf`. This unblocks the graphics loop, dramatically
increases effective read bandwidth, and unlocks **streaming mode** where the device can
push unsolicited read-reply frames without a prior request.

---

## New Source Layout

```
src/
  reader/
    dartt_link.h         # DarttLink class declaration
    dartt_link.cpp       # DarttLink implementation
    CMakeLists.txt       # Standalone build target for easy submodule linking
  plotting.h / plotting.cpp   (modified — add WAV writer)
  dartt_init.h / dartt_init.cpp  (simplified — serial/socket ownership moves to DarttLink)
  main.cpp               (modified — remove dartt_read_multi loop, start DarttLink)
  buffer_sync.h / buffer_sync.cpp  (possibly modified — read-side sync helpers)
```

---

## Component 1: DarttLink Class

`DarttLink` is the sole owner of the transport handle (Serial, UDP socket, or TCP socket).
It runs two threads: a **read thread** that continuously drains the RX kernel buffer, and a
**write thread** that drains a TX queue dispatched from the rendering loop.

### Responsibilities
- Own the Serial/UDP/TCP handle exclusively — no other code touches it
- **Read thread**: drain RX bytes via `cobs_stream()`, decode frames, write read-reply
  data into `periph_buf` under `periph_buf_mutex`, dispatch read requests after each reply
- **Write thread**: block on a condition variable, dequeue pre-encoded raw frames, send
  them over the wire
- Arbitrate half-duplex bus access via `bus_mutex_`
- Optionally write raw COBS-delimited read-reply frames to a `.bin` log file

### Bus Priority

In **full-duplex** mode, TX and RX are independent lines. Writes go out at any time with
no coordination — `bus_mutex_` is skipped on both threads entirely.

In **half-duplex** mode, the bus is shared and all TX must defer to incoming RX. Three
actors compete. Priority order (highest first):

1. **Incoming RX** — read thread holds `bus_mutex_` from first received byte until the
   kernel buffer drains (`read_bytes` returns 0). All TX is locked out.
2. **Writes** — write thread acquires `bus_mutex_` before `send_raw()`. Checked before
   read requests, so dirty writes always go out first after the bus goes idle.
3. **Read requests** — sent by the read thread after the bus goes idle and the write
   queue is empty.

**Important**: in half-duplex, once a read request is dispatched, a reply is guaranteed
to follow. The bus must remain reserved for that incoming reply — no writes should be
sent in the window between transmitting the read request and receiving the first reply
byte. This is enforced by `bus_mutex_`: `dispatch_read_requests` acquires it before
`send_raw` and holds it until the read thread re-enters `read_loop` and picks up the
incoming reply bytes. Writes are blocked for the entire request/reply round-trip.

### Half-duplex vs Full-duplex (`is_full_duplex`)

In half-duplex mode (RS485, default), `bus_mutex_` enforces the priority above.

**Intentional starvation in streaming mode**: when the peripheral streams data gaplessly,
the read thread holds `bus_mutex_` nearly continuously. The write thread and read-request
dispatch will starve — this is correct behaviour. A TX attempt during gapless streaming
would cause a bus collision.

In full-duplex mode (`is_full_duplex = true`), TX and RX are independent lines. Both
threads skip `bus_mutex_` entirely.

### TX Queue (writes)

The rendering loop pushes **complete, pre-encoded raw frames** via `enqueue_write_frame()`.
DarttLink treats all outgoing data as opaque byte frames; COBS encoding and frame
construction happen before enqueue via `enqueue_writes()`.

The write thread blocks on a `std::condition_variable` when the queue is empty (zero CPU).

### Read Request List

`build_read_requests()` is called by the render loop whenever subscriptions change. It
rebuilds `read_request_list_` — a vector of pre-encoded COBS read-request frames — under
`read_request_mutex_`. The read thread dispatches from this list.

`dispatch_read_requests()` is called by the read thread (from `process_frame`) after each
successfully decoded reply. It:
1. Checks `tx_queue_` — if non-empty, returns immediately (writes take priority)
2. Acquires `read_request_mutex_`, sends the next frame from `read_request_list_` via
   `send_raw()` (round-robin)
3. Tracks a timestamp; if no reply arrives within `read_request_timeout_ms`, retransmits

`subscribed_regions_` is render-loop-only; it has no mutex. `read_request_list_` is shared
between render loop (write via `build_read_requests`) and read thread (read via
`dispatch_read_requests`); it is protected by `read_request_mutex_`.


## Component 2: Frame Construction

All frame construction is internal to DarttLink.

- `create_write_frame(ctl, frame)` — bounds-checks ctl against ctl_base, computes field
  index, builds dartt write message, COBS-encodes in-place into `frame` (vector).
- `create_read_request_frame(ctl, frame)` — same pattern but builds a read request frame.
  Size is always fixed: `NUM_BYTES_ADDRESS + NUM_BYTES_INDEX + NUM_BYTES_NUMWORDS_READREQUEST
  + NUM_BYTES_CHECKSUM + NUM_BYTES_COBS_OVERHEAD`.
- `enqueue_writes(ctl_slice)` — mirrors `dartt_write_multi` chunking; splits slice into
  target_serbuf_rx_size-aligned chunks, calls `create_write_frame` + `enqueue_write_frame`
  for each; notifies write thread.
- `enqueue_read_requests(ctl_region)` — mirrors `dartt_read_multi` TX side; splits region
  into target_serbuf_tx_size-aligned chunks, calls `create_read_request_frame` +
  `push_back(move(frame))` into `read_request_list_`.

---

## Component 3: Transport Ownership

`DarttLink` is the **sole owner** of Serial / UdpState / TcpState. The `serial_mutex` that
previously had to be passed around externally is now internal as `bus_mutex_`.

---

## Component 4: Plotter Integration via Callback

After `periph_buf` is updated, the read thread fires `on_read_reply_cb_` (if set) with a
pointer to `periph_buf` and a user context pointer. The callback runs under
`periph_buf_mutex` — callers must not re-acquire it.

The render loop registers a callback that calls `sync_periph_buf_to_fields` and
`enqueue_data` on the plotter. `Plotter` needs a `plot_mutex` held by both the callback
(write) and `Plotter::render()` (read).

---

## Component 5: Binary Frame Logger

Default off. When enabled, every raw COBS-encoded dartt read-reply frame received by the
read thread is appended to a `.bin` file — bytes exactly as they arrive on the wire,
including the `0x00` COBS delimiter. No timestamp prefix.

File format: concatenated COBS frames separated by `0x00` delimiters. Can be fed back into
`cobs_stream` byte-by-byte for offline replay.

---

## Component 6: WAV Writer in Plotter

Default off. Each `Line` (or subset) streams its `enqueue_data` output to a `.wav` file.

- IEEE 754 32-bit float WAV (format tag 3, `WAVE_FORMAT_IEEE_FLOAT`)
- Timestamps via `std::chrono::steady_clock` (microsecond resolution — `SDL_GetTicks64`
  is millisecond-only, insufficient for audio rates)
- Streaming WAV trick: write placeholder header, seek back and fill sizes on `close()`
- One WAV file per `Line`

---

## Thread Safety Summary

| Shared Resource          | Accessed by                                    | Protection               |
|--------------------------|------------------------------------------------|--------------------------|
| Serial / socket handle   | read thread (RX), write thread (TX)            | `bus_mutex_` (half-dup)  |
| `tx_queue_`              | render thread (enqueue), write thread (dequeue)| `tx_queue_mutex_` + `tx_cv_` |
| `read_request_list_`     | render thread (build), read thread (dispatch)  | `read_request_mutex_`    |
| `subscribed_regions_`    | render thread only                             | none                     |
| `periph_buf`             | read thread (write), UI thread (read)          | `periph_buf_mutex`       |
| `DarttField.value`       | read thread (write), UI thread (read/display)  | `periph_buf_mutex`       |
| `Line::points`           | read thread (enqueue), render thread (iterate) | `Plotter::plot_mutex`    |
| `bin_log_file_`          | read thread only                               | none                     |
| `WavWriter` buffers      | read thread (write), stop()/flush              | `Line::wav_mutex`        |

---

## Streaming Mode Behaviour

When `streaming_mode = true`:
- `dispatch_read_requests()` is skipped — peripheral drives its own TX cadence
- Read thread accepts any arriving read-reply frame regardless of whether a request was pending
- Maximum read bandwidth — limited only by baud rate and device firmware dispatch rate

---

## Implementation Status

- [x] DarttLink skeleton — read + write threads, TX queue, bus_mutex_, is_full_duplex
- [x] Frame parsing — dartt_frame_to_payload, periph_buf write under periph_buf_mutex,
      read_reply_cb
- [x] Frame construction — create_write_frame, create_read_request_frame, enqueue_writes,
      enqueue_read_requests (split_read_requests), build_read_requests
- [ ] dispatch_read_requests — send next read request from read thread; priority check;
      timeout/retry; round-robin index
- [ ] Wire dispatch_read_requests into process_frame + read_loop timeout path
- [ ] Plotter integration — plot_mutex, callback wiring in main.cpp
- [ ] Streaming mode — skip dispatch when enabled
- [ ] Binary logger — UI toggle, open/close API (implemented, needs UI wiring)
- [ ] WAV writer
- [ ] main.cpp integration — remove dartt_read_multi/dartt_write_multi call sites

---

## Open Questions

- **WAV sample rate**: Will `std::chrono::steady_clock` give us enough resolution to
  derive a stable sample rate? WAV expects a declared fixed rate in the header — we may
  need to let the user set it explicitly rather than inferring from timestamps.

- **Bin log timestamping**: No timestamps — raw COBS-delimited wire content only.

- **Multi-config**: Only one config loaded at a time. Multiple instances = multiple
  dashboard processes.
