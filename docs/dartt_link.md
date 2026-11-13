# Dartt Link Async Read/Write


## Framing mode (COBS / HDLC)

`DarttLink::set_framing_mode(FramingMode::COBS | FramingMode::HDLC)` selects the byte-stuffing layer
used for both TX frames (`encode_frame`) and RX decoding (`read_loop`). HDLC uses the PPP library
(`0x7E` trailing delimiter, `0x7D` escape). The device firmware must use the same mode.
Switching rebuilds the cached read requests and drops any partially received frame.
The mode is selectable in the UI next to the baudrate and is saved as `serial_settings.framing_mode`
(0 = COBS, default; 1 = HDLC) in the config JSON.
The binary frame logger is COBS-only.
