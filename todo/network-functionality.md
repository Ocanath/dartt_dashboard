# Network Functionality

Theoretically, this program should have the ability to inject networking code directly into the dartt_link layer. The dartt_link class should only require some hardware interface members: a UDP socket, TCP socket in addition to the serial socket. The idea here is that the `send_raw` and `read_bytes` primitives should be transport agnostic. The UDP side of things does make this tricky - while the throughput will likely be higher strict ordering is not guaranteed under UDP and that interferes with dartt somewhat.



 The target address and port for each was previously assigned via the UI in the dev/tcp branch (now local, stale on the remote). We'll need some kind of UI cleanup