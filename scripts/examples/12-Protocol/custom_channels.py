import time
import protocol


class StaticChannel:
    def __init__(self):
        pass

    def size(self):
        return len("HelloWorld!")

    def read(self, offset, size):
        print(f"StaticChannel read {size} bytes from offset {offset}")
        return "HelloWorld!"


class BufferChannel:
    """A simple channel backed by a buffer that supports read/write operations."""

    def __init__(self, buffer_size=1024):
        self.buffer = bytearray(buffer_size)
        self.data_size = 0  # Actual amount of valid data in buffer

    def init(self):
        """Initialize the channel. Return None for success."""
        print(f"BufferChannel initialized with {len(self.buffer)} bytes")

    def deinit(self):
        """Deinitialize the channel. Return None for success."""
        print("BufferChannel deinitialized")

    def poll(self):
        """Check if channel has data available. Return bool."""
        return self.data_size > 0

    def size(self):
        """Return the amount of valid data in the buffer."""
        return self.data_size

    def read(self, offset, size):
        """Read data from the buffer. Return bytes/bytearray."""
        if offset >= self.data_size:
            return b""  # No data at this offset

        end_pos = min(offset + size, self.data_size)
        data = bytes(self.buffer[offset:end_pos])
        print(f"BufferChannel read {len(data)} bytes from offset {offset}")
        return data

    def write(self, offset, data):
        """Write data to the buffer. data is a bytearray. Return bytes written or None."""
        if offset + len(data) > len(self.buffer):
            # Truncate if it would exceed buffer size
            available = len(self.buffer) - offset
            data = data[:available] if available > 0 else b""

        if len(data) > 0:
            self.buffer[offset : offset + len(data)] = data
            # Update data_size if we wrote beyond current end
            self.data_size = max(self.data_size, offset + len(data))

        print(f"BufferChannel wrote {len(data)} bytes to offset {offset}")
        return len(data)  # Return bytes written (or None for default success)


# Register custom channels
ch1 = protocol.register(name="static", backend=StaticChannel())
ch2 = protocol.register(name="buffer", backend=BufferChannel())

while True:
    ch1.send_event(0xAA, wait_ack=False)
    time.sleep_ms(100)
