from utils import int_to_nbytes
import ubinascii as _b64mod
import logger


HEARTBEAT_PAYLOAD_SIZE = 35
_HB_NODE_LIST_LEN = 3


def _to_nbyte_int(payload, start, nbytes):
    return int.from_bytes(payload[start:start + nbytes], "big")


def build_heartbeat_payload(
    image_taken,
    image_sent,
    image_dropped,
    image_failed,
    image_queued,
    radio_succ,
    radio_err,
    internet_succ,
    internet_err,
    fs_succ,
    fs_err,
    neighbours=None,
    shortest_path=None,
    process_id="",
):  # 35 bytes
    hbmsg_bytes = b""
    hbmsg_bytes += int_to_nbytes(image_taken, 2)
    hbmsg_bytes += int_to_nbytes(image_sent, 2)
    hbmsg_bytes += int_to_nbytes(image_dropped, 2)
    hbmsg_bytes += int_to_nbytes(image_failed, 2)
    hbmsg_bytes += int_to_nbytes(image_queued, 2)

    hbmsg_bytes += int_to_nbytes(radio_succ, 3)
    hbmsg_bytes += int_to_nbytes(radio_err, 3)
    hbmsg_bytes += int_to_nbytes(internet_succ, 3)
    hbmsg_bytes += int_to_nbytes(internet_err, 3)
    hbmsg_bytes += int_to_nbytes(fs_succ, 2)
    hbmsg_bytes += int_to_nbytes(fs_err, 2)

    neighbours = neighbours or []
    for i in range(_HB_NODE_LIST_LEN):
        node_id = neighbours[i] if i < len(neighbours) else 0
        hbmsg_bytes += int_to_nbytes(node_id, 1)

    shortest_path = shortest_path or []
    for i in range(_HB_NODE_LIST_LEN):
        node_id = shortest_path[i] if i < len(shortest_path) else 0
        hbmsg_bytes += int_to_nbytes(node_id, 1)

    proc_id = (process_id or "")[:3]
    proc_id = proc_id + ("_" * (3 - len(proc_id)))
    hbmsg_bytes += proc_id.encode()
    return hbmsg_bytes


def parse_heartbeat_rawbytes(payload):
    if not isinstance(payload, (bytes, bytearray)):
        raise ValueError("Heartbeat payload must be bytes")
    if len(payload) != HEARTBEAT_PAYLOAD_SIZE:
        raise ValueError("Invalid heartbeat payload size: {}".format(len(payload)))

    idx = 0
    parsed = {
        "image_taken": _to_nbyte_int(payload, idx, 2),
        "image_sent": _to_nbyte_int(payload, idx + 2, 2),
        "image_dropped": _to_nbyte_int(payload, idx + 4, 2),
        "image_failed": _to_nbyte_int(payload, idx + 6, 2),
        "image_queued": _to_nbyte_int(payload, idx + 8, 2),
        "radio_succ": _to_nbyte_int(payload, idx + 10, 3),
        "radio_err": _to_nbyte_int(payload, idx + 13, 3),
        "internet_succ": _to_nbyte_int(payload, idx + 16, 3),
        "internet_err": _to_nbyte_int(payload, idx + 19, 3),
        "fs_succ": _to_nbyte_int(payload, idx + 22, 2),
        "fs_err": _to_nbyte_int(payload, idx + 24, 2),
        "neighbours": [
            _to_nbyte_int(payload, idx + 26, 1),
            _to_nbyte_int(payload, idx + 27, 1),
            _to_nbyte_int(payload, idx + 28, 1),
        ],
        "shortest_path": [
            _to_nbyte_int(payload, idx + 29, 1),
            _to_nbyte_int(payload, idx + 30, 1),
            _to_nbyte_int(payload, idx + 31, 1),
        ],
        "process_id": payload[idx + 32:idx + 35].decode().rstrip("_"),
    }
    return parsed


def parse_heartbeat_b64bytes(b64_bytes):  # string bytes should be send as encoded, data.encode()
    if not isinstance(b64_bytes, bytes):
        logger.error("Heartbeat b64 payload must be bytes")
        return {}
    try:
        raw_bytes = _b64mod.a2b_base64(b64_bytes)
    except Exception as e:
        logger.error(f"Invalid base64 heartbeat payload: {str(e)}")
        return {}

    return parse_heartbeat_rawbytes(raw_bytes)
