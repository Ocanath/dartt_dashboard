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

## TCP

The TCP behavior will be to establish a socket connection with the specified target address on the specified port. 
In the TCP case, the read reply timeout of 10ms is overly aggressive and should be relaxed by the software. It should be reset to 10ms when serial is active.

## UDP

The UDP socket will bind to `0.0.0.0` on the specified port. It will target the specified target address on the specified port.


