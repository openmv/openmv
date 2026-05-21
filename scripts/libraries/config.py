import binascii
import machine
from machine import LED
import uasyncio as asyncio

# Map board UID (hex bytes) to node address.
UID_TO_ADDR = {
    b'e606fe64d709051c': 216,
    b'e076465dd7193d2a': 217,
    b"e076465dd709102e": 218,
    b'e606fe64d7091126': 219,
    b"e076465dd7091027": 220,
    b"e076465dd7090d1c": 221,
    b"e076465dd719431e": 222,
    b"e076465dd7091843": 223,
    b"e076465dd719421e": 224,
    b"e076465dd7194025": 225,
    b"e076465dd7194318": 226,
    b"e076465dd7193a09": 227,
    b"e076465dd7090e41": 228,
    b"e606fe64d7090425": 229,
    b"e606fe64d7110c31": 231,
    b"e606fe64d7110b25": 232,
}

# XX.XX.X
major = 2  # (0-63)
minor = 0  # (0-99)
patch = 2  # (0-9)
# version as integer value, max_val = 64_999 < 65_535 (2 bytes)
VERSION = major * 1_000 + minor * 10 + patch

def get_version_str(version):
    """
    Decode packed version integer to XX.XX.X string (inverse of major*1000 + minor*10 + patch).

    Args:
        version (int): Packed version, e.g. 2001 for 2.0.1.

    Returns:
        str: Version string, e.g. "02.00.1".
    """
    patch = version % 10
    minor = (version // 10) % 100
    major = version // 1000
    return "{:02d}.{:02d}.{}".format(major, minor, patch)


uid = binascii.hexlify(machine.unique_id())
my_addr = UID_TO_ADDR.get(uid)

def get_my_addr(default=None):
    """Return node address for current board UID."""
    if my_addr is None:
        print("Error: my_addr not defined for this device.")
        return None
    return my_addr

async def led_restart_blinker():
    led = LED("LED_GREEN")
    blink_count = 5
    blink_duration = 0.1
    for i in range(blink_count):
        led.on()
        await asyncio.sleep(blink_duration)
        led.off()
        await asyncio.sleep(blink_duration)
