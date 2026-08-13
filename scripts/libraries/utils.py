import struct
import gc
import utime

# Binary header prepended to JPEG bytes before base64 upload (API strips first N bytes).
# 2-byte LE payload length + 20-byte payload (lat, lon, epoch, image_id, type).
IMAGE_META_LEN_FIELD_BYTES = 2
IMAGE_BASIC_PAYLOAD_LEN = 20


def clamp_to_bytes(value, nbytes):
    """Clamp integer to the max storable value for nbytes."""
    if value is None:
        value = 0
    if value < 0:
        value = 0
    max_val = (1 << (8 * nbytes)) - 1
    if value > max_val:
        return max_val
    return value


def int_to_nbytes(value, nbytes):
    v = clamp_to_bytes(int(value), nbytes)
    return v.to_bytes(nbytes, "big")


def _lat_lon_to_micro_deg_int(coord):
    """Round coordinate to 6 decimal places, scale by 1e6, return signed int32-safe int."""
    if coord is None:
        coord = 0
    try:
        x = float(coord)
    except (TypeError, ValueError):
        x = 0.0
    x = round(x, 6)
    micro = int(round(x * 1_000_000))
    if micro > 0x7FFFFFFF:
        micro = 0x7FFFFFFF
    elif micro < -0x80000000:
        micro = -0x80000000
    return micro


def pack_image_meta_header(lat, lon, epoch_ms, image_id, file_type):
    """
    Fixed IMAGE_META_HEADER_LEN-byte prefix (no padding).

    Little-endian layout:
      0:2    payload_len uint16 — bytes following this field (IMAGE_META_PAYLOAD_LEN)
      2:6    lat  int32  (degrees * 1e6, 6 decimal places)
      6:10   lon  int32  (degrees * 1e6, 6 decimal places)
      10:18  epoch_ms uint64 as two uint32 (lo, hi)
      18:21  image_id 3 ASCII bytes (padded/truncated)
      21:22  frame type: ord('B') background, ord('C') cropped/stitched, ord('F') full frame (AI)
    """
    img_meta_header_len = IMAGE_META_LEN_FIELD_BYTES + IMAGE_BASIC_PAYLOAD_LEN
    buf = bytearray(img_meta_header_len)

    # bytes 0-1: payload length (data after this uint16)
    struct.pack_into("<H", buf, 0, IMAGE_BASIC_PAYLOAD_LEN)

    # bytes 2-5: lat int32 LE (degrees * 1e6, 6 decimal places)
    # bytes 6-9: lon int32 LE (same encoding)
    lat_i = _lat_lon_to_micro_deg_int(lat)
    lon_i = _lat_lon_to_micro_deg_int(lon)
    struct.pack_into("<ii", buf, 2, lat_i, lon_i)

    # bytes 10-17: epoch_ms uint64 LE as uint32 lo + uint32 hi
    em = int(epoch_ms)
    if em < 0:
        em = 0
    struct.pack_into("<II", buf, 10, em & 0xFFFFFFFF, (em >> 32) & 0xFFFFFFFF)

    # bytes 18-20: image_id, 3 ASCII chars (pad spaces / truncate)
    sid = image_id if isinstance(image_id, str) else str(image_id)
    sid = (sid + "   ")[:3]
    buf[18:21] = sid.encode()[:3]

    # byte 21: frame type — B background full, C cropped/stitched, F full frame (AI)
    if file_type not in ("B", "C", "F"):
        file_type = "B"
    buf[21] = ord(file_type)

    return bytes(buf)


def get_free_memory():
    """Return available free memory in KB as an integer."""
    try:
        gc.collect()
        free_bytes = gc.mem_free()
    except AttributeError:
        free_bytes = -1
    except Exception:
        free_bytes = -1

    if free_bytes < 0:
        return 0
    return free_bytes // 1024


def get_uptime_minutes():
    """
        Return integer minutes elapsed since device boot.
        For 30 days = 43200 < 65535 (2 bytes)
    """
    try:
        return utime.ticks_ms() // 60000
    except Exception:
        return 0

def get_uptime_seconds():
    """
        Return integer seconds elapsed since device boot.
        For 30 days = 43200 < 65535 (2 bytes)
    """
    try:
        return utime.ticks_ms() // 1000
    except Exception:
        return 0
