"""
UART Transport Example for OpenMV Protocol

This example shows how to create a UART-based transport channel and register
built-in protocol channels in Python.

The transport implements the physical layer interface required by the protocol:
- read(): Physical read from UART with timeout
- write(): Physical write to UART with timeout
- is_active(): Check if UART connection is available
- size(): Return number of bytes available to read
"""
import protocol
from machine import UART


class StaticChannel:
    def __init__(self):
        pass

    def size(self):
        return len("HelloWorld!")

    def read(self, offset, size):
        print(f"StaticChannel read {size} bytes from offset {offset}")
        return "HelloWorld!"


class UartTransport:
    """UART-based transport channel for OpenMV Protocol"""

    def __init__(self, uart_id=1, baudrate=115200, timeout=1000, rxbuf=1024):
        self.uart = UART(
            uart_id, baudrate, rxbuf=rxbuf, timeout=timeout, timeout_char=500, flow=UART.RTS | UART.CTS
        )
        self.buf = memoryview(bytearray(rxbuf))

    def is_active(self):
        return True

    def size(self):
        return self.uart.any()

    def read(self, offset, size):
        size = self.uart.readinto(self.buf, size)
        return None if size is None else self.buf[:size]

    def write(self, offset, data):
        return self.uart.write(data)

    def flush(self):
        self.uart.flush()


if __name__ == "__main__":
    MAX_PAYLOAD = 4096 - 10 - 2

    # Initialize and configure the protocol
    protocol.init(
        crc=True,  # Enable CRC
        seq=True,  # Enable sequence checking
        ack=True,  # Wait for CKs
        events=True,  # Enable async-events
        soft_reboot=False,  # Disable soft-reboots (required)
        max_payload=MAX_PAYLOAD,  # Max packet payload
        rtx_retries=3,  # Retransmission retry count
        rtx_timeout_ms=500,  # Timeout before retransmission (doubled after each try)
        lock_interval_ms=10,  # Minimum locking interval
        timer_ms=10,  # Schedules the protocol task every 10ms
    )

    # Register the transport
    protocol.register(
        name="uart",
        flags=protocol.CHANNEL_FLAG_PHYSICAL,
        backend=UartTransport(3, timeout=5000, rxbuf=8 * 1024, baudrate=921600),
    )

    # Register custom channel
    protocol.register(name="static", backend=StaticChannel())
