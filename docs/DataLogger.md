# Data Logger

This class performs asynchronous filewriting/logging of npy files (numpy native data format) while preserving realtime performance of the dashboard. The TU contains a logger class definition and owns it's own ImGui window, with basic features like starting/stopping logging (which persists in the settings json), and .npz package filenames.

At a high level, the DataLogger plugs into the rest of the code in the following way:

1. Read callback iterates through the DarttFields in the subscribed list. Each new incoming element will get enqueued to a ring/circular FIFO buffer.
2. The DataLogger runs an async process for dispatching filewrites via the NpyWriter class. It drains the ring buffer(s) as it writes.

The numpy writer list 