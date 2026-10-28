# Network Functionality

Theoretically, this program should have the ability to inject networking code directly into the dartt_link layer. The dartt_link class should only require some hardware interface members: a UDP socket, TCP socket in addition to the serial socket. The idea here is that the `send_raw` and `read_bytes` primitives should be transport agnostic. The UDP side of things does make this tricky - while the throughput will likely be higher strict ordering is not guaranteed under UDP and that interferes with dartt somewhat.

The target address and port for each was previously assigned via the UI in the dev/tcp branch (now local, stale on the remote). We'll ape the same thing - there will be one interface selected at a time, and the 'connect' button will sanitize the user input in the UI and pass it to the link unit. 


In both the TCP and UDP cases, we will just pass the encoded frames along the transport layer. On the TCP receive side this makes things simple, since we have frame delimiters for the stream. On the UDP side it's meaningless - two additional bytes of overhead in a UDP frame are meaningless.

This also means that forwarding devices will natively forward the frames over serial.

If we, in the future, want to employ a TCP/UDP translation layer to something that doesn't use COBS+serial, we'll inject a decode call in the transport function before sending it out, and mask it with a toggle. That is unnecessary for the scope of this project at the moment.

## UI

The UI should include a button for Serial, UDP and TCP. The TCP and UDP connections shall have a 'connect' button, port and target IP field. When the 'connect' button is pressed the target ip and port will be parsed and passed to init calls for the dartt_link TU.

## Configuration JSON

The transport link setting will be saved and recalled in the configuration JSON. In the case of TCP and UDP the relevant settings (port and target) will be saved and recalled as well.

## Link layer changes

The socket read call *must not block*. The read loop thread has its own timeout logic and is designed for non-blocking reads, which drain the kernel buffer. So the udp and tcp socket handling must mirror this.

The `chunk` buffer which `read_bytes` copies to in the `read_loop` should be simply increased to capture the full datagram size. It is currently too small at 64 bytes. This design is fine for serial but datagrams arrive complete. Simply expanding the size of this buffer is sufficient.

The read loop explicitly checks `serial.connected()`. This should be replaced with a general link layer wrapper that publishes the connected() status of the active interface.



## TCP

The TCP behavior will be to establish a socket connection with the specified target address on the specified port. 
In the TCP case, the read reply timeout of 10ms is overly aggressive and should be relaxed by the software. It should be reset to 10ms when serial is active.

### TCP_NODELAY (Nagle's algorithm)

By default a TCP socket runs Nagle's algorithm, which holds small outgoing segments until the previously sent data is ACKed (coalescing them to avoid tinygrams). This can be disabled with the `TCP_NODELAY` flag set on the TCP socket. Note: in tinycsocket **v0.3** (our pinned version) this option is bugged — it is exposed only as `TCS_SO_IP_NODELAY` at level `TCS_SOL_IP`, but `TCP_NODELAY` must be set at the TCP level (`SOL_TCP`/`IPPROTO_TCP`); setting it at the IP level is a no-op (or silently sets `IP_TOS` on Linux, since both constants are numerically 1) and leaves Nagle fully enabled. v0.3 doesn't define a `TCS_SOL_TCP` constant, but the workaround is to pass the protocol constant `TCS_IPPROTO_TCP` (== `IPPROTO_TCP` == `SOL_TCP`) as the option *level*: `tcs_set_option(sock, TCS_IPPROTO_TCP, TCS_SO_IP_NODELAY, &one, sizeof(one))`. Check the return code, and verify with Wireshark / latency quantization that Nagle is actually off.


## UDP

The UDP socket will bind to `0.0.0.0` on the specified port. It will target the specified target address on the specified port. 


