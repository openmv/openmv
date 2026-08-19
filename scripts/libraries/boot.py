import logger
from machine import RTC, UART, Pin, LED
import machine
from app_controller import AppController, WIFI_SOCKET_SESSION_TIMEOUT_S
from db_store import DbStore
import uasyncio as asyncio
import utime
import sensor
import image
import os                   # file system access
import sys
import time
import binascii
import struct
import sys
import random
import ubinascii
import json
import gc                   # garbage collection for memory management
import hashlib
import enc
from config import (
    uid,
    get_my_addr,
    led_restart_blinker,
    VERSION
)
from utils import int_to_nbytes, pack_image_meta_header, get_free_memory, get_uptime_minutes
from sx1262 import SX1262
from gps_driver import GPSDriver
from internet_driver import InternetDriver, INTERNET_RECHECK_INTERVAL_SEC
from _sx126x import ERR_NONE, ERR_CRC_MISMATCH, ERR_UNKNOWN, SX126X_IRQ_CRC_ERR, SX126X_IRQ_HEADER_ERR, SX126X_IRQ_RX_DONE, SX126X_IRQ_TIMEOUT, SX126X_IRQ_TX_DONE, SX126X_SYNC_WORD_PRIVATE, SX126X_IRQ_ALL
import detect
from detect import PIR_PIN, turn_ON_IR_emitter, turn_OFF_IR_emitter

# -----------------------------------▼▼▼▼▼-----------------------------------
# -------------------- TESTING VARIABLES, TODO PRODUCTION --------------------
# ---------------------------------------------------------------------------
PRODUCTION_MODE = True
FAKE_LAYOUT_ENABLED = False
ENCRYPTION_ENABLED = True

INSTALL_MODE_WAIT_TIME = 60
app_controller = None
app_handler = None
APP_DEBUGGING = False
SAVE_LOGS = False

DECRYPT_IMAGE_ON_HOPS = False
if not PRODUCTION_MODE:
    DECRYPT_IMAGE_ON_HOPS = True
FLAKINESS = 0
ALERT_TEXT_PAUSED = True
USE_PIR_SENSOR = True
# -----------------------------------▲▲▲▲▲-----------------------------------

def get_rand(len=3):
    # Input: None; Output: str random 3-letter uppercase identifier
    rstr = ""
    for i in range(len):
        rstr += chr(65+random.randint(0,25))
    return rstr

PROCESS_ID_STR = get_rand(3)
db_store = None


# CURRENT DATA/MEMORY VARS
img_capture_count = 0 # Counter to keep track of saved images
img_file_counter = 0

ack_msgs_recd = [] # USED only to check ack of sent messages
MAX_ACK_MSGS_RECD = 500          # Maximum messages in received buffer
MAX_AGE_MSG_RCD_SEC = 20   # 20 sec, after 20 sec messages will be removed
# Chunk slots in CHUNK_STORAGE_BUFFER; block[0]=len (0=empty), block[1:]=payload
trans_chunk_epoch_ms = None  # Epoch time when transfer started
MEM_CLEANUP_INTERVAL_SEC = 30  # Run memory cleanup every 30 seconds

APP_DISARMED = False
is_install_mode = False

# -----------------------------------▼▼▼▼▼-----------------------------------
# FIXED VARIABLES
led = LED("LED_BLUE")
led.off()

# TIME VARS
MIN_SLEEP = 0.1    # max 0.1, 0.02 (works with highest data rate)
ACK_SLEEP = 1       # max 2, 1 (works with highest data rate)
CHUNK_SLEEP = 0.1  # max 0.1, 0.04 (works with highest data rate)

# SIZE VARS
CHUNK_DATA_SIZE = 45
PACKET_PAYLOAD_LIMIT = 60
PACKET_BODY_LIMIT = 50
RSA_ENCRYPTION_LIMIT = 117

HB_WAIT = 180
D_MSG_WAIT = 60
DISCOVERY_COUNT = 100
SPATH_WAIT = 30
SPATH_WAIT_2 = 1200
SCAN_WAIT = 30
SCAN_WAIT_2 = 1200
VALIDATE_WAIT_SEC = 1200
PHOTO_TAKING_DELAY = 600

GPS_WAIT_SEC = 30
GPS_WAIT_REFRESH_SEC = 1800

NETWORK_EMPTY_SLEEP = 15 # 15 sec, when no path is there
NETWORK_IN_TRANS_SLEEP = 10 # 10 sec, sleep when trans mode in progress
NETWORK_IMPROVE_SLEEP = 30 # 30 sec, connected, but loopking for better path
NETWORK_IMPROVE_COUNT = 10 # 10 times, loopking for better path

NETWORK_STABLE_SLEEP = 600 # 600 second, 10 minutes
NET_PATH_EXPIRY_MS = 1800000 # 1800 second, 30 minutes

TRANSMODE_LOCK_TIMEOUT = 600 # TODO PRODUCTION
TRANSMODE_INACTIVITY_LIMIT = 40 # 20 second
CHUNK_BURST_SIZE = 20
CHUNK_BURST_RX_SLEEP = 0.2
# Config test for SF7
LORA_FREQ = 868.0
LORA_BW = 125             # 125kHz provides better sensitivity than wider bandwidths
LORA_SF = 7               # SF9: Medium speed, excellent range margin for 600-800m
LORA_CR = 6               # CR 4/6: Good error correction for reliable communication
LORA_POWER = 22           # Maximum power for strong signal margin
LORA_PREAMBLE = 10        # Longer preamble for better sync detection

gps_module = None
internet_module = None
rtc = None
wifi_nic = None
# Shared UART instance for TracX module (used by both GPS and Internet drivers)
tracx_uart = None
# Lock to prevent GPS and cellular from using UART concurrently (both use same TracX module)
tracx_uart_lock = asyncio.Lock()
# -----------------------------------▲▲▲▲▲-----------------------------------




# -----------------------------------▼▼▼▼▼-----------------------------------
# STATE VARIABLES
# -------- Start FPS clock -----------
# clock = time.clock()            # measure frame/sec

my_addr = None
network_paths = []
seen_neighbours = []

gps_str = ""
gps_last_time = -1

consecutive_hb_failures = 0
lora_init_count = 0
lora_init_in_progress = False

trans_in_progress = False
trans_paired_device = None

trans_data_id = None
trans_msg_typ = None
trans_chunks_count = 0
trans_chunk_epoch_ms = None
trans_chunk_md5 = None
trans_prev_data_id = None
trans_last_actvity_time = None

# Global pre-allocated buffer for image recompilation (120KB)
IMAGE_RECOMPILE_BUFFER = None  # Will be initialized at startup
DATA_BUFFER_SIZE = 120 * 1024 # 120KB

MAX_CHUNK_COUNT = (DATA_BUFFER_SIZE + CHUNK_DATA_SIZE - 1) // CHUNK_DATA_SIZE
CHUNK_BLOCK_SIZE = 1 + CHUNK_DATA_SIZE  # [len:1][body:45]
CHUNK_STORAGE_SIZE = MAX_CHUNK_COUNT * CHUNK_BLOCK_SIZE
CHUNK_STORAGE_BUFFER = None

# radio sent
radio_sent_succ_count = 0
radio_sent_fail_count = 0
# radio receive
radio_recd_succ_count = 0
radio_recd_err_count = 0
radio_recd_crcerr_count = 0
radio_recd_hasherr_count = 0
# last sums
radio_succ_count_prev = 0
radio_fail_count_prev = 0

# file system
gps_success_count = 0
gps_failure_count = 0


busy_devices = [] # device those are busy in sending/receiving images

# Interrupt-driven receive state
lora_rx_event = asyncio.Event()  # Event signaled when packet received
lora_rx_data = None               # Received packet data
lora_rx_status = None              # Receive status

# Packet processing queue - decouples fast packet reception from slow processing
# This prevents blocking the receive loop when processing heavy operations (file I/O, network uploads)
packet_queue = []  # Queue for packets to be processed asynchronously
packet_queue_lock = asyncio.Lock()  # Lock for thread-safe queue access

# -----------------------------------▲▲▲▲▲-----------------------------------


# FILE CHUNKING VARS
MSG_TYPE_BYTE_LEN = 1
NODE_ID_BYTE_LEN = 1
MSG_ID_BYTE_LEN = 3
MSG_UID_LEN = MSG_TYPE_BYTE_LEN + 3*NODE_ID_BYTE_LEN + MSG_ID_BYTE_LEN
# PACKET CRC CHECKSUM LEN
CRC_CHECKSUM_LEN = 4
HEADER_LEN = MSG_UID_LEN + CRC_CHECKSUM_LEN
HEADER_JOINED_LEN = HEADER_LEN + 1 # 1 byte for ; separator

IMG_ID_LEN = 3 # UXK, BTQ
IMG_ID_BYTES = 2
CHUNK_ID_BYTES = 2



# -----------------------------------▼▼▼▼▼-----------------------------------
# --------- DEBUGGING ONLY ---- REMOVE BEFORE FINAL -------------------------
# --------- DEBUGGING ONLY ---- REMOVE BEFORE FINAL -------------------------
# -----------------------------------▲▲▲▲▲-----------------------------------


encnode = None
clock_start_ms = None
LOGS_DIR = None
FS_ROOT = "/sdcard"

async def init_device():
    global encnode
    global db_store
    global my_addr
    global rtc

    print("UID: ", f"{uid}")
    my_addr = get_my_addr()
    if my_addr is None:
        logger.error(f"error in main.py: Unknown device UID for {uid}")
        sys.exit()
    print(f"MY_ADDR: {my_addr}")

    encnode = enc.EncNode(my_addr)

    rtc = machine.RTC()
    rtc.datetime((2024, 1, 1, 0, 0, 0, 0, 0))
    global clock_start_ms
    clock_start_ms = utime.ticks_ms() # get millisecond counter

    global PROCESS_DIR, LOGS_DIR, PROCESS_ID_STR
    LOGS_DIR =    f"{FS_ROOT}/{PROCESS_ID_STR}/logs"

    if PROCESS_ID_STR is None:
        logger.error(f"[INIT] ===> PROCESS_ID_STR is not set, exiting...")
        sys.exit()
    try:
        db_store = DbStore(PROCESS_ID_STR, my_addr)
        logger.info(f"[INIT] DbStore initialized for process {PROCESS_ID_STR}")
    except Exception as e:
        logger.error(f"[INIT] Failed to initialize DbStore: {e}")
        return False
    logger.info(
        f"[INIT] ===> MyAddr = {my_addr}, "
        f"uid={uid.decode()}, PROCESS_ID_STR={PROCESS_ID_STR} <===\n"
    )

    # MEMORY FREE, ALLOCATION =====>
    gc.enable()
    free_before = get_free_memory()
    logger.info(f"[IMG RX] Free mem at init: {free_before}KB")
    if not init_file_recompile_buffer():
        logger.warning("[MEM] Image recompile buffer not available, will use dynamic allocation")
        sys.exit()
    if not init_chunk_storage_buffer():
        logger.warning("[MEM] Chunk storage buffer not available")
        sys.exit()

    return True

def init_file_recompile_buffer():
    """Initialize the global file recompilation buffer at startup when memory is available"""
    global IMAGE_RECOMPILE_BUFFER
    try:
        # Allocate 120KB buffer upfront when memory is less fragmented
        IMAGE_RECOMPILE_BUFFER = bytearray(DATA_BUFFER_SIZE)  # 120KB
        logger.info(f"[MEM] Pre-allocated file recompile buffer: {len(IMAGE_RECOMPILE_BUFFER)/1024:.1f}KB")
        return True
    except MemoryError as e:
        logger.error(f"[MEM] Failed to allocate file recompile buffer: {e}")
        IMAGE_RECOMPILE_BUFFER = None
        return False
    except Exception as e:
        logger.error(f"[MEM] Error allocating file recompile buffer: {e}")
        IMAGE_RECOMPILE_BUFFER = None
        return False

def init_chunk_storage_buffer():
    """Pre-allocate fixed-size chunk slots for transfer-mode receive (one file at a time)."""
    global CHUNK_STORAGE_BUFFER
    try:
        CHUNK_STORAGE_BUFFER = bytearray(CHUNK_STORAGE_SIZE)
        logger.info(
            f"[MEM] Pre-allocated chunk storage: {MAX_CHUNK_COUNT} blocks x {CHUNK_BLOCK_SIZE}B "
            f"({CHUNK_STORAGE_SIZE/1024:.1f}KB)"
        )
        return True
    except MemoryError as e:
        logger.error(f"[MEM] Failed to allocate chunk storage buffer: {e}")
        CHUNK_STORAGE_BUFFER = None
        return False
    except Exception as e:
        logger.error(f"[MEM] Error allocating chunk storage buffer: {e}")
        CHUNK_STORAGE_BUFFER = None
        return False

def _chunk_block_offset(chunk_id):
    return chunk_id * CHUNK_BLOCK_SIZE

def _chunk_payload_len(chunk_id): # first bytes contain the len of payload body
    return CHUNK_STORAGE_BUFFER[_chunk_block_offset(chunk_id)]

def reset_trans_chunk_storage(chunk_count=None):
    """Clear block headers (len=0); buffer is not freed."""
    if CHUNK_STORAGE_BUFFER is None:
        return
    limit = chunk_count if chunk_count is not None else MAX_CHUNK_COUNT
    for chunk_id in range(limit):
        CHUNK_STORAGE_BUFFER[_chunk_block_offset(chunk_id)] = 0

def running_as_cc():
    global internet_module
    if internet_module and internet_module.has_internet:
        return True
    else:
        return False

def running_as_unit():
    return not running_as_cc()

async def logger_state():
    global db_store
    fileiter = 0
    while True:
        if trans_in_progress:
            await asyncio.sleep(5)
            continue
        await asyncio.sleep(60)
        fileiter += 1
        log_file = f"{LOGS_DIR}/logs_{fileiter}.txt"
        logs_list = logger.return_saved_logs_and_clear()
        logger.info(f"Saving logs file {log_file} with {len(logs_list)} entries")
        logs_data = ("\n".join(logs_list)).encode()
        await db_store.save_file(logs_data, log_file) # file system success counted inside

async def reboot_device():
    global db_store
    try:
        log_file = f"{LOGS_DIR}/logs_LAST.txt"
        logs_list = logger.get_saved_logs()
        logger.info(f"Saving logs file {log_file} with {len(logs_list)} entries")
        logs_data = ("\n".join(logs_list)).encode()
        await db_store.save_file(logs_data, log_file)
        print("REBOOTING DEVICE\n\n")
        machine.reset()
    except Exception as e: # Fail safe reboot
        machine.reset()

def get_epoch_ms(): # unix epoch milliseconds, eg. 1381791310000
    return utime.time_ns() // 1_000_000

def get_epoch_sec(): # unix epoch seconds, eg. 1736931600
    return int(utime.ticks_ms() / 1000)

def get_ms_diff(): # milliseconds, from the device start time
    delta = utime.ticks_diff(utime.ticks_ms(), clock_start_ms)
    return delta

def get_sec_diff(): # NOT in use, use get_sec_sec() instead
    # Input: None; Output: int seconds since clock_start_ms
    return int(utime.ticks_diff(utime.ticks_ms(), clock_start_ms) / 1000) # compute time difference

def time_msec(): # Not in use, use get_ms_diff instead
    # Input: None; Output: int milliseconds since clock_start_ms
    delta = utime.ticks_diff(utime.ticks_ms(), clock_start_ms) # compute time difference
    return delta


# TypeSourceDestRRRandom
def encode_node_id(node_id):
    # Input: node_id: int; Output: single-byte representation
    if not isinstance(node_id, int):
        logger.error(f"[LORA] node id must be int, got {type(node_id)}")
        raise TypeError(f"node id must be int, got {type(node_id)}")
    if not 0 <= node_id <= 255:
        logger.error(f"[LORA] node id {node_id} out of range (0-255)")
        raise ValueError(f"node id {node_id} out of range (0-255)")
    return bytes((node_id,))

def encode_dest(dest):
    # Input: dest: int; Output: single-byte representation or broadcast marker
    if dest in (0, 65535):
        return b'*'
    return encode_node_id(dest)

def get_msg_header(msg_typ, creator, dest, msgbytes):
    """
    Build LoRa packet header (msg_uid + CRC-32 of payload).

    Args:
        msg_typ (str): Message type, 1 ASCII byte (e.g. "I", "H", "B").
        creator (int): Source node id (encoded to 1 byte).
        dest (int): Destination node id (encoded to 1 byte).
        msgbytes (bytes): Payload; CRC is computed over this only.

    Returns:
        tuple[bytes, bytes]: (msg_uid, crc_checksum) => 7 + 4 = 11 bytes.
            Wire format: msg_uid + crc_checksum + b";" + msgbytes (12 + len(msgbytes) bytes).
    """
    rrr = get_rand(len=3)
    msg_uid = (
        msg_typ.encode()  # 1 byte
        + encode_node_id(creator)  # 1 byte
        + encode_node_id(my_addr)  # 1 byte
        + encode_dest(dest)  # 1 byte
        + rrr.encode()  # 3 bytes
    )
    # binascii.crc32 returns 32-bit int; struct.pack puts it in 4 bytes (big-endian)
    crc = binascii.crc32(msgbytes) & 0xFFFFFFFF
    crc_checksum = struct.pack(">I", crc)
    return msg_uid, crc_checksum

def parse_header(databytes):
    # Input: databytes: bytes; Output: tuple(success, msg_uid, msg_typ, creator, sender, receiver, msg) or None
    global MSG_UID_LEN, HEADER_LEN, HEADER_JOINED_LEN, CRC_CHECKSUM_LEN
    global radio_recd_succ_count, radio_recd_hasherr_count, radio_recd_err_count
    msg_uid = b""
    if databytes == None:
        logger.warning(f"[LORA] Weird that databytes is none")
        return (False, None, None, None, None, None, None)

    if len(databytes) < HEADER_JOINED_LEN:
        logger.error(f"[LORA] databytes too short {len(databytes)} < {HEADER_JOINED_LEN}")
        return (False, None, None, None, None, None, None)
    try:
        if chr(databytes[HEADER_LEN]) != ';':
            radio_recd_err_count += 1
            logger.error(f"[LORA] error parsing data[HEADER_LEN], recieved: {chr(databytes[HEADER_LEN])}, expected ;")
            return (False, None, None, None, None, None, None)
        else:
            # MSG_UID
            msg_uid = databytes[:MSG_UID_LEN]
            msg_typ = chr(msg_uid[0])
            creator = int(msg_uid[1])
            sender = int(msg_uid[2])
            if msg_uid[3] == 42 or msg_uid == b"*": # byte with value 42 maps to *
                receiver = -1
            else:
                receiver=int(msg_uid[3])


            msgbytes = databytes[HEADER_JOINED_LEN:]

            # CRC_CHECKSUM: verify CRC of payload against the 4 bytes in header
            crc_checksum = databytes[MSG_UID_LEN:HEADER_LEN]
            if len(crc_checksum) != CRC_CHECKSUM_LEN:
                radio_recd_err_count += 1
                logger.error(f"[RECV] invalid header CRC_CHECKSUM length in {databytes}")
                return (False, None, None, None, None, None, None)
            calc_crc = binascii.crc32(msgbytes) & 0xFFFFFFFF
            recv_crc = struct.unpack(">I", crc_checksum)[0]
            if calc_crc != recv_crc:
                radio_recd_hasherr_count += 1
                # TODO COUNT CURRUPTED PACKETS
                logger.error(f"[RECV] payload CRC_CHECKSUM mismatch (msgbytes checksum), dropping packet: {databytes}")
                return (False, None, None, None, None, None, None)
            radio_recd_succ_count += 1
            return (True, msg_uid, msg_typ, creator, sender, receiver, msgbytes)
    except Exception as e:
        radio_recd_err_count += 1
        logger.error(f"[RECV] error parsing header: {databytes[:HEADER_LEN]} : {e}")
        return (False, None, None, None, None, None, None)

def ellepsis(msg):
    # Input: msg: str; Output: str truncated with ellipsis if necessary
    if len(msg) > 200:
        return msg[:100] + "......." + msg[-100:]
    return msg

def ack_needed(msg_typ): # msg_type P is devided in (B,I,E)
    # Input: msg_typ: str; Output: bool indicating if acknowledgement required
    if msg_typ in ["A", "W", "N", "I", "K"]:
        return False
    # H = heartbeat (separate blocks); K = Android/app-origin message
    if msg_typ in ["H", "B", "E", "V", "C", "Z"]:
        return True
    return False

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.HD)
sensor.skip_frames(time=2000)

sent_count = 0
recv_msg_count = {}

URL_OLD = "https://n8n.vyomos.org/webhook/watchmen-detect/"
# URL = "https://hqapi.vyomos.org/watchmen-detect/"
URL = "https://api.vyomiq.io/watchmen-detect/"


# -----------------------------------▼▼▼▼▼-----------------------------------
# TRANSFER MODE Lock

def get_transmode_lock(device_id, filedata_id, msg_typ, chunk_count, md5): # check and just lock for image
    global trans_in_progress, trans_paired_device
    global trans_data_id, trans_msg_typ, trans_chunks_count, trans_chunk_epoch_ms, trans_chunk_md5
    if is_install_mode: # safty return
        logger.error(f"[IMG] TRANS MODE not allowed in install mode")
        return False
    if trans_in_progress == True: # TRANS MODE already in use
        return False
    if chunk_count > MAX_CHUNK_COUNT:
        logger.error(
            f"[MEM] chunk_count {chunk_count} exceeds MAX_CHUNK_COUNT {MAX_CHUNK_COUNT} "
            f"for filedata_id={filedata_id}"
        )
        return False
    trans_in_progress = True
    trans_paired_device = device_id
    trans_data_id = filedata_id
    trans_msg_typ = msg_typ
    trans_chunks_count = chunk_count
    trans_chunk_md5 = md5

    logger.info(f"[IMG] ●●●●●●●●●●❯❯ TRANS MODE started, device:{device_id}, msg_typ:{trans_msg_typ}, filedata_id:{filedata_id} ❮❮●●●●●●●●●●")
    return True

async def keep_transmode_lock(device_id, filedata_id):
    # Input: None; Output: None (sets trans_in_progress flag with auto release after timeout / inactivity)
    global trans_in_progress, trans_paired_device
    global trans_data_id, trans_msg_typ, trans_chunks_count, trans_chunk_epoch_ms, trans_chunk_md5
    global trans_last_actvity_time

    # Track when this lock started
    start_ms = get_epoch_ms()

    # If we don't have any activity yet, treat "now" as the last activity
    if trans_last_actvity_time is None:
        trans_last_actvity_time = start_ms

    while True:
        await asyncio.sleep(5)
        if not (trans_in_progress and trans_paired_device == device_id and trans_data_id == filedata_id):
            logger.debug(
                f"[IMG] ○○○○○○○○○○❯❯ TRANS MODE already ended, device:{device_id}, msg_typ:{trans_msg_typ}, filedata_id:{filedata_id} ❮❮○○○○○○○○○○"
            )
            break

        now_ms = get_epoch_ms()
        timeout_expired = (now_ms - start_ms) >= TRANSMODE_LOCK_TIMEOUT * 1000
        inactivity_expired = (now_ms - trans_last_actvity_time) >= TRANSMODE_INACTIVITY_LIMIT * 1000
        if timeout_expired or inactivity_expired:
            reason = []
            if timeout_expired:
                reason.append("MAX_TIMEOUT")
            if inactivity_expired:
                reason.append("INACTIVITY_TIMEOUT")
            reason_str = "&".join(reason)

            logger.warning(
                f"[IMG] ●●●●●●●●●●❯❯ TRANS MODE ended, device:{device_id}, msg_typ:{trans_msg_typ}, filedata_id:{filedata_id}, by {reason_str} ❮❮●●●●●●●●●●"
            )

            chunks_to_clear = trans_chunks_count
            was_receiving = chunks_to_clear and chunks_to_clear > 0
            trans_in_progress = False
            trans_paired_device = None
            trans_data_id = None
            trans_msg_typ = None
            trans_chunks_count = 0
            trans_chunk_md5 = None
            trans_last_actvity_time = None

            if was_receiving:
                reset_trans_chunk_storage(chunks_to_clear)
                trans_chunk_epoch_ms = None
            gc.collect()
            logger.debug(
                f"[MEM] Cleared old chunks in get_transmode_lock for filedata_id={filedata_id}, by {reason_str} "
            )
            break

def check_transmode_lock(device_id, filedata_id): # check if transfer lock is active or not
    global trans_in_progress, trans_paired_device
    global trans_data_id, trans_last_actvity_time
    # # If filedata_id is None, only check device_id (for backward compatibility)
    # if filedata_id is None: # TODO
    #     return trans_in_progress and trans_paired_device == device_id
    # # Otherwise check both device_id and filedata_id
    if trans_in_progress and trans_paired_device == device_id and trans_data_id == filedata_id:
        trans_last_actvity_time = get_epoch_ms()
        return True
    else:
        return False


def delete_transmode_lock(device_id, filedata_id, trans_success=False): # calledat send_done and recive_done
    # Input: None; Output: None (clears trans_in_progress flag)
    global trans_in_progress, trans_paired_device
    global trans_data_id, trans_msg_typ, trans_chunks_count, trans_chunk_epoch_ms, trans_prev_data_id, trans_chunk_md5
    global trans_last_actvity_time
    if trans_in_progress and trans_paired_device == device_id and trans_data_id == filedata_id:  # TODO, these has to handled using someuniqueness
        logger.info(f"[IMG] ●●●●●●●●●●❯❯ TRANS MODE ended for device:{device_id}, msg_typ:{trans_msg_typ}, filedata_id:{filedata_id}, by logic ❮❮●●●●●●●●●●")
        chunks_to_clear = trans_chunks_count
        was_receiving = chunks_to_clear and chunks_to_clear > 0
        trans_in_progress = False
        trans_paired_device = None
        trans_data_id = None
        trans_msg_typ = None
        trans_chunks_count = 0
        trans_chunk_md5 = None
        trans_last_actvity_time = None
        if trans_success:
            trans_prev_data_id = filedata_id

        if was_receiving:
            reset_trans_chunk_storage(chunks_to_clear)
            trans_chunk_epoch_ms = None
            gc.collect()
            logger.debug(f"[MEM] Cleared old chunks in delete transmode lock for filedata_id={filedata_id}, by logic ")
    else:
        logger.debug(f"[IMG] ○○○○○○○○○○❯❯ TRANS MODE already ended, for device {device_id} and filedata_id {filedata_id} ❮❮○○○○○○○○○○") # will move it to debug later
# -----------------------------------▲▲▲▲▲-----------------------------------



# -----------------------------------▼▼▼▼▼-----------------------------------
# STORE for BUSY DEVICES
BUSY_WAIT_TIME = 20
WAIT_MESSAGE = f"{20}"
def is_device_free(device_id):
    global busy_devices
    # return not device_id in busy_devices
    if device_id in busy_devices:
        return False
    return True

def is_device_busy(device_id):
    global busy_devices
    # return device_id in busy_devices
    if device_id in busy_devices:
        return True
    return False

async def device_busy_life(device_id): # device_busy_cycle
    # Input: device_id: int; Output: None (sets trans_in_progress flag with auto release after timeout)
    global busy_devices
    busy_devices.append(device_id)
    logger.info(f"Device marked busy, device:{device_id}")
    await asyncio.sleep(BUSY_WAIT_TIME) # At this point this process might complete, also other might start
    busy_devices.remove(device_id)
    logger.info(f"Device marked free, device:{device_id}")
# -----------------------------------▲▲▲▲▲-----------------------------------


# -----------------------------------▼▼▼▼▼-----------------------------------
# LoRa Setup and Transmission
# ---------------------------------------------------------------------------
loranode = None
async def init_lora():
    # Input: None; Output: None (initializes global loranode, updates lora_reinit_count)
    global loranode, lora_init_count, lora_init_in_progress
    if lora_init_in_progress:
        logger.info(f"[LORA] Initialization already in progress, skipping duplicate call")
        return False
    lora_init_in_progress = True
    try:
        lora_init_count += 1
        logger.info(f"[LORA] Initializing LoRa SX1262 module... my lora addr = {my_addr}")

        # Initialize SX1262 with SPI pin configuration
        loranode = SX1262(
            spi_bus=1,
            clk='P2',      # SCLK
            mosi='P0',     # MOSI
            miso='P1',     # MISO
            cs='P3',       # Chip Select
            irq='P13',     # DIO1 (IRQ)
            rst='P6',      # Reset
            gpio='P7',     # BUSY
            spi_baudrate=2000000,
            spi_polarity=0,
            spi_phase=0
        )

        # Configure LoRa with fast communication settings
        status = loranode.begin(
            freq=LORA_FREQ,
            bw=LORA_BW,
            sf=LORA_SF,
            cr=LORA_CR,
            syncWord=SX126X_SYNC_WORD_PRIVATE,
            power=LORA_POWER,
            currentLimit=60.0,
            preambleLength=LORA_PREAMBLE,
            implicit=False,
            crcOn=True,
            tcxoVoltage=1.6,
            useRegulatorLDO=False,
            blocking=False  # Non-blocking interrupt-driven mode
        )

        if status != ERR_NONE:
            logger.error(f"[LORA] Failed to initialize SX1262, status: {status}")
            loranode = None
            return False

        # Set up interrupt callback for RX_DONE and TX_DONE
        loranode.setBlockingCallback(blocking=False, callback=lora_event_callback)

        logger.info(f"[LORA] LoRa SX1262 initialized successfully")
        return True
    except Exception as e:
        logger.error(f"[LORA] Exception during initialization: {str(e)}")
        loranode = None
        return False
    finally:
        lora_init_in_progress = False

def lora_event_callback(events): # TODO Anand, merge radio_read into this function

    """
    Interrupt callback function - called automatically when RX_DONE or TX_DONE occurs.
    This runs in interrupt context, so keep it minimal and fast.

    FIX: Added proper interrupt status clearing and race condition protection
    to prevent packet loss when multiple interrupts fire rapidly.
    """
    global lora_rx_data, lora_rx_status, lora_rx_event
    global radio_recd_err_count

    if events & SX126X_IRQ_RX_DONE:
        # Packet received - read it immediately
        try:
            # FIX: Clear interrupt status IMMEDIATELY to allow next packet to trigger interrupt
            # This is critical - if we don't clear it, the radio won't generate new interrupts
            # for subsequent packets, causing packet loss

            # Read the packet from radio buffer
            msg, status = loranode.recv(len=0)

            # FIX: Race condition protection - only update data if event is not already set
            # This prevents overwriting a packet that hasn't been processed yet by the async task.
            # If the previous packet is still being processed, we might lose this packet,
            # but that's better than corrupting the previous packet data.
            if not lora_rx_event.is_set():
                lora_rx_data = msg
                lora_rx_status = status
                # Signal async task to process the packet
                lora_rx_event.set()
            else: # Previous packet not processed yet - log warning
                radio_recd_err_count += 1
                logger.warning(f"[LORA] Interrupt fired but previous packet not processed yet - this packet is skipped")

            try:
                loranode.clearIrqStatus(SX126X_IRQ_RX_DONE)
                loranode.startReceive()
            except:
                pass
        except Exception as e:
            logger.error(f"[LORA] Error reading packet in interrupt callback: {e}")
            try:
                loranode.clearIrqStatus(SX126X_IRQ_ALL)
                loranode.startReceive()  # Restart RX mode after error
            except:
                pass

            # Only set error status if event is not already set (race condition protection)
            if not lora_rx_event.is_set():
                lora_rx_data = None
                lora_rx_status = ERR_UNKNOWN
                lora_rx_event.set()
    elif events & (SX126X_IRQ_CRC_ERR | SX126X_IRQ_HEADER_ERR):
        try:
            msg, status = loranode.recv(len=0)
            try:
                loranode.clearIrqStatus(SX126X_IRQ_CRC_ERR | SX126X_IRQ_HEADER_ERR)
            except:
                pass
            loranode.startReceive()
            if not lora_rx_event.is_set():
                lora_rx_data = None
                lora_rx_status = ERR_CRC_MISMATCH
                lora_rx_event.set()
        except Exception as e:
            logger.error(f"[LORA] Error handling CRC/Header error in interrupt callback: {e}")
            try:
                loranode.clearIrqStatus(SX126X_IRQ_ALL)
                loranode.startReceive()
            except:
                pass
    elif events & SX126X_IRQ_TIMEOUT:
        try:
            loranode.clearIrqStatus(SX126X_IRQ_TIMEOUT)
            loranode.startReceive()  # Restart RX mode after timeout
        except Exception as e:
            logger.error(f"[LORA] Error handling timeout in interrupt callback: {e}")
            try:
                loranode.clearIrqStatus(SX126X_IRQ_ALL)
                loranode.startReceive()
            except:
                pass
    elif events & SX126X_IRQ_TX_DONE:
        # Transmission complete - radio automatically returns to RX mode
        # FIX: Clear TX interrupt status to prevent interrupt register from filling up
        try:
            loranode.clearIrqStatus(SX126X_IRQ_TX_DONE)
        except:
            pass
    else:
        logger.warning(f"[LORA] Unknown interrupt event: {events}, resetting status, receive mode...")
        try:
            loranode.clearIrqStatus(SX126X_IRQ_ALL)
            loranode.startReceive()
        except:
            pass


async def lora_health_monitor():  # is_lora_ready is nit being used
    global loranode, lora_init_in_progress
    global radio_sent_succ_count, radio_sent_fail_count, radio_recd_succ_count, radio_recd_err_count, radio_recd_crcerr_count, radio_recd_hasherr_count
    global radio_succ_count_prev, radio_fail_count_prev
    RADIO_HEALTH_INTERVAL = 120
    while True:
        if lora_init_in_progress:
            await asyncio.sleep(RADIO_HEALTH_INTERVAL)
            continue
        else:
            try:
                if loranode is None:
                    logger.info(f"[LORA] LoRa not initialized, initializing...")
                    succ = await init_lora()
                    if not succ:
                        logger.error(
                            f"[LORA] Failed to initialize LoRa, retrying in 60 seconds"
                        )
                        await asyncio.sleep(RADIO_HEALTH_INTERVAL)
                        continue
                    else:
                        logger.info(f"[LORA] LoRa initialized successfully")
                        await asyncio.sleep(RADIO_HEALTH_INTERVAL)
                else:  # loranode might be invalid
                    radio_succ_count = radio_sent_succ_count + radio_recd_succ_count
                    radio_fail_count = (
                        radio_sent_fail_count
                        + radio_recd_err_count
                        + radio_recd_crcerr_count
                        + radio_recd_hasherr_count
                    )

                    succ_diff = radio_succ_count - radio_succ_count_prev
                    fail_diff = radio_fail_count - radio_fail_count_prev

                    total_msg = max(succ_diff + fail_diff, 1)
                    error_percentage = int(max(fail_diff, 0) * 100 / total_msg)
                    if total_msg >= 20:  # for 20 msg, check 50% error
                        if error_percentage >= 50:
                            logger.info(
                                f"[LORA] Radio is not working fine, error pct: {error_percentage}%, initializing..."
                            )
                            succ = await init_lora()
                            if not succ:
                                logger.error(
                                    f"[LORA] Failed to initialize LoRa, retrying in 60 seconds"
                                )
                                await asyncio.sleep(RADIO_HEALTH_INTERVAL)
                                continue
                            else:
                                logger.info(f"[LORA] LoRa initialized successfully")
                                await asyncio.sleep(RADIO_HEALTH_INTERVAL)
                        else:
                            logger.info("Radio is workinng fine")
                            await asyncio.sleep(RADIO_HEALTH_INTERVAL)
                        radio_succ_count_prev = radio_succ_count
                        radio_fail_count_prev = radio_fail_count
                    elif total_msg >= 10:  # for 10 msg, check 80% error
                        if error_percentage >= 80:
                            logger.info(
                                f"[LORA] Radio is not working fine, error pct: {error_percentage}%, initializing..."
                            )
                            succ = await init_lora()
                            if not succ:
                                logger.error(
                                    f"[LORA] Failed to initialize LoRa, retrying in 60 seconds"
                                )
                                await asyncio.sleep(RADIO_HEALTH_INTERVAL)
                                continue
                            else:
                                logger.info(f"[LORA] LoRa initialized successfully")
                                await asyncio.sleep(RADIO_HEALTH_INTERVAL)
                        else:
                            logger.info("Radio is workinng fine")
                            await asyncio.sleep(RADIO_HEALTH_INTERVAL)
                    else:
                        logger.debug(
                            f"less radio steps available, will re-analyse ater somtime."
                        )
                        await asyncio.sleep(RADIO_HEALTH_INTERVAL)
            except Exception as e:
                logger.error(f"[LORA] Exception in health monitor: {e}")
                await asyncio.sleep(RADIO_HEALTH_INTERVAL)


def is_lora_ready():
    # Input: None; Output: bool indicating if LoRa is ready to send
    global lora_init_in_progress, loranode
    if loranode is None:
        if not lora_init_in_progress:
            logger.error(f"[LORA] Not connected to radio network, init started in background.., msg marked as failed")
            asyncio.create_task(init_lora())
        else:
            logger.debug(f"[LORA] Not connected to radio network, init already in progress, msg marked as failed")
        return False
    return True


async def radio_read(): # TODO Anand, merge
    """
    Interrupt-driven LoRa receive loop with packet queuing.
    Queues packets immediately to prevent blocking the receive loop.
    This allows rapid packet reception even when processing is slow.
    """
    global lora_rx_data, lora_rx_status, lora_rx_event, packet_queue, packet_queue_lock
    global radio_recd_crcerr_count

    logger.info(f"===> Radio Read, LoRa interrupt-driven receive loop started... <===\n")
    while True:
        try:
            # FIX: Clear event BEFORE waiting to handle any stale events from previous iterations
            # This ensures we start with a clean state and don't process old data
            lora_rx_event.clear()

            # Wait for interrupt event (blocks until callback fires)
            # This is much more efficient than polling - task is suspended until data arrives
            # The callback will set this event when RX_DONE interrupt occurs
            await lora_rx_event.wait()

            # CRITICAL FIX: Queue packet immediately without processing
            # This allows interrupt callback to fire again quickly for next packet
            # Heavy processing (file I/O, network uploads) happens in background queue processor
            if lora_rx_status == ERR_NONE:
                # Valid packet received
                if lora_rx_data and len(lora_rx_data) > 0:
                    # Restore newlines (they were replaced during send to avoid packet corruption)
                    message = lora_rx_data.replace(b"{}[]", b"\n")
                    # Get RSSI after successful receive (must be called soon after recv)
                    rssi = loranode.getRSSI()
                    # Add to queue instead of processing directly - this is FAST
                    async with packet_queue_lock:
                        packet_queue.append((message, rssi))
                    # Log queue size periodically for monitoring
                    queue_size = len(packet_queue)
                    if queue_size > 10 and queue_size % 5 == 0:
                        logger.warning(f"[QUEUE] Packet queue size: {queue_size} - processing may be slow")
            elif lora_rx_status == ERR_CRC_MISMATCH:
                # Corrupted packet - log but don't process
                # The radio detected a CRC error, but we still received the packet
                radio_recd_crcerr_count += 1
                logger.warning(f"[LORA] CRC error on received packet, dropped this packet")
            else:
                # Other error - log and continue
                # This could be timeout, header error, etc.
                logger.warning(f"[LORA] Receive error status: {lora_rx_status}, dropped this packet")

            # FIX: Clear received data immediately after queuing to prevent race conditions
            # This allows the interrupt callback to fire again quickly
            lora_rx_data = None
            lora_rx_status = None

        except Exception as e:
            lora_rx_data = None
            lora_rx_status = None
            logger.error(f"[LORA] Exception in radio_read: {e}")
            sys.print_exception(e)
            await asyncio.sleep(0.1)  # Brief pause on error

async def process_packet_queue(): # TODO Anand, (no change)
    """
    Background task to process queued packets asynchronously.
    This prevents blocking the receive loop when doing heavy operations.

    Prioritizes I (chunk) packets for fast file transfer.
    """
    global packet_queue, packet_queue_lock

    logger.info(f"===> Packet Queue Processor started... <===\n")
    while True:
        try:
            packet_data = None

            # Get packet from queue with priority for I (chunk) packets
            async with packet_queue_lock:
                if len(packet_queue) == 0:
                    packet_data = None
                else:
                    # PRIORITY FIX: Process I (chunk) packets first for fast image transfer
                    # I chunks are time-sensitive and need fast processing
                    i_chunk_index = None
                    for i, (msg, rssi) in enumerate(packet_queue):
                        try:
                            # Quick parse to check message type (just first byte after header)
                            # I chunks have format: msg_uid;filedata_id(3) + chunk_idx(2) + data
                            # After parsing header, msg_typ is at msg_uid[0]
                            if len(msg) >= HEADER_LEN:  # Minimum header length
                                msg_typ_char = chr(msg[0])
                                if msg_typ_char == "I":  # I chunk packet
                                    i_chunk_index = i
                                    break
                        except:
                            pass

                    # If I chunk found, process it first; otherwise process first packet
                    if i_chunk_index is not None:
                        packet_data = packet_queue.pop(i_chunk_index)
                    else:
                        packet_data = packet_queue.pop(0)

            if packet_data:
                message, rssi = packet_data
                # Now process the packet - this can be slow (file I/O, network, etc.)
                # But it doesn't block the receive loop anymore
                process_message(message, rssi)
            else:
                # No packets in queue, yield to other tasks
                # Small sleep prevents busy-waiting and allows other tasks to run
                await asyncio.sleep(0.01)

        except Exception as e:
            logger.error(f"[QUEUE] Error processing packet from queue: {e}")
            sys.print_exception(e)
            await asyncio.sleep(0.1)  # Brief pause on error

# -----------------------------------▲▲▲▲▲-----------------------------------


# Memory Management Functions
def cleanup_old_ack_messages():
    """Remove old messages from buffers based on age and size limits
    Explicitly frees memory by clearing old list references before reassignment.
    Critical for OpenMV RT1062 with limited heap memory.
    """
    global ack_msgs_recd
    current_time = get_ms_diff()
    age_threshold_ms = MAX_AGE_MSG_RCD_SEC * 1000

    # PART 1 - delete the exipred
    old_list = ack_msgs_recd

    ack_msgs_recd = [(msg_uid, msg, t) for msg_uid, msg, t in ack_msgs_recd
                 if (current_time - t) < age_threshold_ms]

    old_list.clear()
    del old_list

    # PART 2 - delete the older, if limit exceeded
    if len(ack_msgs_recd) > MAX_ACK_MSGS_RECD:
        new_old_list = ack_msgs_recd
        ack_msgs_recd = sorted(ack_msgs_recd, key=lambda x: x[2], reverse=True)[:MAX_ACK_MSGS_RECD]

        new_old_list.clear()
        del new_old_list

    gc.collect()


def cleanup_chunk_map_by_msg_id(filedata_id):
    """Reset pre-allocated chunk slots for the current transfer."""
    global trans_chunk_epoch_ms, trans_data_id, trans_chunks_count
    if trans_chunks_count and trans_chunks_count > 0 and filedata_id == trans_data_id:
        chunks_to_clear = trans_chunks_count
        trans_chunks_count = 0
        trans_chunk_epoch_ms = None
        reset_trans_chunk_storage(chunks_to_clear)
    else:
        if trans_chunks_count == 0:
            logger.debug(f"[CHUNK] chunks storage already cleared for filedata_id:{filedata_id}")
        elif filedata_id != trans_data_id:
            logger.warning(f"[CHUNK] filedata_id mismatch: requested {filedata_id}, but current trans_data_id is {trans_data_id}")


async def periodic_health_stats_loop():
    """Periodically clean up memory buffers and run garbage collection"""
    global seen_neighbours
    global db_store
    while True:
        try:
            global trans_in_progress
            seen_nodes = []
            for item in seen_neighbours:
                seen_nodes.append(item["node"])
            seen_nodes_str = ",".join([str(n) for n in seen_nodes])
            img_queued_count = db_store.get_img_queued_count() if db_store is not None else -1
            if trans_in_progress:
                logger.info(f"[MEM] ⛃⛃⛃⛁⛁⛁, img_queued: {img_queued_count}, img_sent: {db_store.get_img_sent_count()}, img_dropped: {db_store.get_img_dropped_count()}, img_failed: {db_store.get_img_failed_count()}, network paths: {len(network_paths)}, seen_neighbours: [{seen_nodes_str}], TRANS MODE, no cleanup!!")
                await asyncio.sleep(MEM_CLEANUP_INTERVAL_SEC)
                continue
            free_before = get_free_memory()

            cleanup_old_ack_messages()

            gc.collect()

            free_after = get_free_memory()
            freed = free_after - free_before if free_before > 0 and free_after > 0 else 0
            logger.info(f"[MEM] ⛃⛃⛃⛁⛁⛁ Cleanup complete (free: {free_after}KB, freed: {freed}KB), img_queued: {img_queued_count}, img_sent: {db_store.get_img_sent_count()}, img_dropped: {db_store.get_img_dropped_count()}, img_failed: {db_store.get_img_failed_count()}, network paths: {len(network_paths)}, seen_neighbours: [{seen_nodes_str}]")
            await asyncio.sleep(MEM_CLEANUP_INTERVAL_SEC)
        except Exception as e:
            logger.error(f"[MEM] error in memory cleanup: {e}")
            await asyncio.sleep(MEM_CLEANUP_INTERVAL_SEC)

# MSG TYPE = H(eartbeat), A(ck), B(egin), E(nd), C(hunk), S(hortest path)

def radio_send(dest, data, msg_uid):
    # Input: dest: int, data: bytes; Output: None (sends bytes via LoRa, logs send)
    global sent_count
    sent_count = sent_count + 1
    if len(data) > 254:
        return False, f"[LORA] msg too large : {len(data)}"
    # Replace newlines to avoid packet corruption
    data = data.replace(b"\n", b"{}[]")

    # New driver sends raw bytes (destination is already in the data packet header)
    bytes_sent, status = loranode.send(data)
    if status != ERR_NONE:
        return False, f"[LORA] Send failed with status: {status}"
    # Map 0-210 bytes to 1-10 asterisks, anything above 210 = 10 asterisks
    # data_masked_log = min(10, max(1, (len(data) + 20) // 21))
    # logger.info(f"[⮕ SENT to {dest}] [{'*' * data_masked_log}] {len(data)} bytes, MSG_UID = {msg_uid}")
    return True, None

async def send_single_packet(msg_typ, creator, msgbytes, dest, retry_count = 3):
    try:
        # Input: msg_typ: str, creator: int, msgbytes: bytes, dest: int; Output: tuple(success: bool, missing_chunks: list)
        msg_uid, crc_checksum = get_msg_header(msg_typ, creator, dest, msgbytes)
        databytes = msg_uid + crc_checksum + b";" + msgbytes
        ackneeded = ack_needed(msg_typ)
        sent_time = get_ms_diff()

        data_masked_log = min(10, max(1, (len(databytes) + 20) // 21))

        if not ackneeded:
            succ, err = radio_send(dest, databytes, msg_uid)
            if not succ:
                logger.error(f"[LORA] Error sending message: {err}, MSG_UID = {msg_uid}")
                return (False, [])
            await asyncio.sleep(MIN_SLEEP)
            if msg_typ != "I":
                logger.info(f"[⮕ SENT to {dest}] [{'*' * data_masked_log}] {databytes} bytes, MSG_UID = {msg_uid}")
            return (True, [])

        global radio_sent_succ_count, radio_sent_fail_count
        for retry_i in range(retry_count):
            succ, err = radio_send(dest, databytes, msg_uid)
            if not succ:
                logger.error(f"[LORA] Error sending message {1+retry_i}/{retry_count}: {err}, MSG_UID = {msg_uid}")
                continue
            if msg_typ != "I":
                logger.info(f"[⮕ SENT to {dest}] [{'*' * data_masked_log}] {databytes} bytes, MSG_UID = {msg_uid}")
            await asyncio.sleep(ACK_SLEEP)
            first_log_flag = True
            ack_msg_recheck_count = 3
            for i in range(ack_msg_recheck_count): # ack_msk recheck
                ack_recd_time, missing_chunks = get_ack_msg_info(msg_uid)
                if ack_recd_time > 0:
                    logger.info(f"[ACK] Msg {msg_uid} : was acked in {ack_recd_time - sent_time} msecs")
                    radio_sent_succ_count += 1
                    return (True, missing_chunks)
                else:
                    if first_log_flag:
                        logger.info(f"[ACK] Still waiting for ack, MSG_UID =  {msg_uid} # {i}")
                        first_log_flag = False
                    else:
                        logger.debug(f"[ACK] Still waiting for ack, MSG_UID = {msg_uid} # {i}")
                    await asyncio.sleep(
                        ACK_SLEEP * min(i + 1, 3)
                    )  # progressively more sleep, capped at 3x
            logger.warning(f"[ACK] Failed to get ack, MSG_UID = {msg_uid}, retry # {retry_i+1}/{retry_count}")
        logger.error(f"[LORA] Failed to send message, MSG_UID = {msg_uid}")
        radio_sent_fail_count += 1
        return (False, [])
    except Exception as e:
        logger.error(f"[LORA] Exception in send_single_packet: {e}")
        radio_sent_fail_count += 1
        return (False, [])

def make_chunks(msgbytes):
    # Input: msgbytes: bytes; Output: list of bytes chunks up to 200 bytes each (safe size for 255 byte LoRa limit)
    # Calculation: Max LoRa packet (255) - msg_uid+separator (8) - filedata_id (IMG_ID_BYTES) - chunk_index (3) = 242 bytes available
    # Using 200 bytes per chunk for safety margin (prevents ERR_PACKET_TOO_LONG errors)
    # Total packet: 7 (msg_uid) + 1 (;) + IMG_ID_BYTES (filedata_id) + 3 (chunk_index) + 200 (data) = 213 bytes (safe)
    global CHUNK_DATA_SIZE
    chunks = []
    while len(msgbytes) > CHUNK_DATA_SIZE:
        chunks.append(msgbytes[0:CHUNK_DATA_SIZE])
        msgbytes = msgbytes[CHUNK_DATA_SIZE:]
    if len(msgbytes) > 0:
        chunks.append(msgbytes)
    return chunks

def encrypt_if_needed(msg_typ, msg):
    try:
        # Input: msg_typ: str message type, msg: bytes; Output: bytes (possibly encrypted message)
        if not ENCRYPTION_ENABLED:
            return msg
        # H = heartbeat, "P": file data
        if is_hybrid_encrypted(msg_typ):
            msgbytes = enc.encrypt_hybrid(msg, encnode.get_pub_key())
            logger.debug(f"{msg_typ} : Len msg = {len(msg)}, len msgbytes = {len(msgbytes)}")
            return msgbytes
        return msg
    except Exception as e:
        logger.error(f"Error in encrypt_if_needed error: {e}")
        return None

def is_rsa_encrypted(msg_typ):
    if not ENCRYPTION_ENABLED:
        return False
    if msg_typ in ["*"]: # not data is rsa_encrypted
        return True
    return False

def is_hybrid_encrypted(msg_typ):
    if not ENCRYPTION_ENABLED:
        return False
    if msg_typ == "P":
        return True
    return False

# === Send Function ===

async def send_msg(msg_typ, creator, msgbytes, dest, retry_count=3): # all messages except file data
    try:
        if not is_lora_ready():
            return False
        # Input: msg_typ: str, creator: int, msgbytes: bytes, dest: int; Output: bool success indicator
        if len(msgbytes) <= PACKET_BODY_LIMIT:
            logger.info(f"[⋙ sending....] dest={dest}, msg_typ:{msg_typ}, len:{len(msgbytes)} bytes, single packet")
            succ, _ = await send_single_packet(msg_typ, creator, msgbytes, dest, retry_count)
            return succ
        else:
            logger.error(f"msgbtyes size exceeds the payload body limit, {len(msgbytes)} bytes > {PACKET_BODY_LIMIT} bytes")
            return False
    except Exception as e:
        logger.error(f"[LORA] Exception in send_msg: {e}")
        return False

async def send_msg_big(msg_typ, creator, msgbytes, dest, epoch_ms, md5): # file sending
    if not is_lora_ready():
        return False, "LORA not ready"
    if msg_typ == "P":
        global radio_sent_succ_count, radio_sent_fail_count
        filedata_id = get_rand(len=IMG_ID_LEN)
        if get_transmode_lock(dest, filedata_id, msg_typ, 0, ""): # dummy 0, as we are just sending
            asyncio.create_task(keep_transmode_lock(dest, filedata_id))
            # sending start
            chunks = make_chunks(msgbytes)
            logger.info(f"[⋙ sending....] dest={dest}, msg_typ:{msg_typ}, len:{len(msgbytes)} bytes, filedata_id:{filedata_id}, image_payload in {len(chunks)} chunks")
            big_succ, _ = await send_single_packet("B", creator, f"{filedata_id}:{msg_typ}:{len(chunks)}:{md5}", dest)
            if not big_succ:
                # logger.info(f"[CHUNK] Failed sending chunk begin")
                delete_transmode_lock(dest, filedata_id)
                return False, "Failed sending chunk begin"

            for chunk_id in range(len(chunks)):
                if check_transmode_lock(dest, filedata_id): # check old logs is still in progress or not
                    if chunk_id % 10 == 0:
                        logger.info(f"⋙ Sending chunks to {dest}, ({chunk_id}-{min(chunk_id+10, len(chunks))})/{len(chunks)}...")
                    await asyncio.sleep(CHUNK_SLEEP)
                    # Adding meta data to chunk bytes, 3 + 2 + 45 = 50 bytes max
                    chunkbytes = filedata_id.encode() + chunk_id.to_bytes(CHUNK_ID_BYTES) + chunks[chunk_id]
                    _ = await send_single_packet("I", creator, chunkbytes, dest)
                    if(chunk_id+1) % CHUNK_BURST_SIZE == 0:
                        logger.info(f"[CHUNK] pause {CHUNK_BURST_RX_SLEEP}s after {chunk_id+1} chunks for RX")
                        await asyncio.sleep(CHUNK_BURST_RX_SLEEP)
                else:
                    # logger.error(f"TRANS MODE ended, marking data send as failed, timeout error")
                    return False, "TRANS MODE ended, timeout error"
            max_miss_retries = 30
            for retry_i in range(max_miss_retries):
                if retry_i == 0:
                    await asyncio.sleep(0.1)  # Faster first check
                else:
                    await asyncio.sleep(CHUNK_SLEEP)
                succ, missing_chunks = await send_single_packet("E", creator, f"{filedata_id}:{epoch_ms}", dest, retry_count = 5)
                if not succ:
                    # logger.error(f"[CHUNK] Failed sending chunk end")
                    return False, "Failed sending chunk end"
                else:
                    if (
                        missing_chunks is None
                        or len(missing_chunks) == 0
                        or (len(missing_chunks) == 1 and missing_chunks[0] == -1)
                    ):
                        logger.info(f"[CHUNK] Successfully sent all chunks (missing_chunks={missing_chunks})")
                        delete_transmode_lock(dest, filedata_id)
                        if retry_i==0:
                            radio_sent_succ_count += len(chunks)
                        return True, None

                    if retry_i==0:
                            radio_sent_fail_count += len(missing_chunks)
                            radio_sent_succ_count += (len(chunks) - len(missing_chunks))
                    logger.info(
                        f"[CHUNK] Receiver still missing {len(missing_chunks)} chunks after retry {retry_i}: {missing_chunks}"
                    )
                    if check_transmode_lock(dest, filedata_id): # check old logs is still in progress or not
                        for idx, chunk_id in enumerate(missing_chunks):
                            if idx % 10 == 0:
                                logger.info(f"⋙ Sending missing chunks to {dest}, ({idx}-{min(idx+10, len(missing_chunks))})/{len(missing_chunks)}...")
                            await asyncio.sleep(CHUNK_SLEEP)
                            chunkbytes = filedata_id.encode() + chunk_id.to_bytes(CHUNK_ID_BYTES) + chunks[chunk_id]
                            _ = await send_single_packet("I", creator, chunkbytes, dest)
                    else:
                        # logger.error(f"TRANS MODE ended, marking data send as failed, timeout error")
                        return False, "TRANS MODE ended, timeout error"

            delete_transmode_lock(dest, filedata_id)
            return False, f"Failed sending data, max retries reached-{max_miss_retries}"
        else:
            logger.warning(f"TRANS MODE already in use, could not get lock...")
            return False, "TRANS MODE already in use, could not get lock..."
    else:
        logger.warning(f"Invalid message type: {msg_typ}")
        return False, "Invalid message type"

def get_ack_msg_info(msg_uid):
    global ack_msgs_recd, MSG_UID_LEN
    # Input: msg_uid: bytes; Output: tuple(ack_recd_time:int, missingids:list or None)
    for (recd_msg_uid, msgbytes, t) in ack_msgs_recd:
        # msgbytes of ack message is msg_uid only (+ missing_chunks string?)
        if len(msgbytes) >= MSG_UID_LEN:
            ack_msg_uid = msgbytes[:MSG_UID_LEN]
            if msg_uid == ack_msg_uid:
                missingids = []
                # case for missing chunks
                if len(msgbytes) >= MSG_UID_LEN + 1 and msgbytes[MSG_UID_LEN:MSG_UID_LEN+1] == b':': # This will always we true
                    payload = msgbytes[MSG_UID_LEN+1:]
                    if payload:
                        try:
                            if len(payload) % CHUNK_ID_BYTES != 0:
                                logger.error(
                                    f"[ACK] Missing IDs payload length {len(payload)} not multiple of CHUNK_ID_BYTES={CHUNK_ID_BYTES}, ignoring payload"
                                )
                                return (0, []) # ignoring invalid payload
                            for i in range(0, len(payload), CHUNK_ID_BYTES):
                                chunk_id = int.from_bytes(payload[i:i+CHUNK_ID_BYTES], "big")
                                missingids.append(chunk_id)
                        except Exception as e: # failed to parse payload
                            logger.warning(f"[ACK] Failed to parse missing IDs payload {payload}: {e}")
                            return (0, [])
                logger.debug(f"[ACK] Matched ACK for {msg_uid}, missing chunks: {missingids}")
                return (t, missingids)
        else:
            logger.debug(f"[ACK] ACK payload too short: {len(msgbytes)} bytes, expected at least {HEADER_LEN-1}")
    return (0, [])




# ---------------------------------------------------------------------------
# Chunk Assembly Helpers
# ---------------------------------------------------------------------------

def begin_chunk(msgbytes):
    # Input: msgbytes: bytes; Output: tuple(filedata_id:str, numchunks:int, md5:str) or (None, None, None) on parse error
    msg_data = msgbytes.decode()
    parts = msg_data.split(":")
    if len(parts) != 4:
        logger.error(f"[CHUNK] begin message unparsable {msg_data}")
        return (None, None, None)
    filedata_id = parts[0]
    msg_typ = parts[1]
    # epoch_ms = get_epoch_ms()
    numchunks = int(parts[2])
    md5 = parts[3]

    # This function just parses the B packet and returns the info
    logger.info(f"[IMG RX] Received B packet for filedata_id={filedata_id}, expected_chunks={numchunks}")
    return (filedata_id, msg_typ, numchunks, md5)


def get_missing_chunks(filedata_id):
    # Input: filedata_id: str chunk identifier; Output: list of int missing chunk indices
    global trans_chunks_count, trans_data_id
    if filedata_id != trans_data_id:
        logger.warning(f"[CHUNK] get_missing_chunks: no chunks storage for filedata_id={filedata_id}")
        return []
    if trans_chunks_count == 0:
        logger.warning(f"[CHUNK] get_missing_chunks: trans_chunks_count not set")
        return []

    missing_chunks = []
    for chunk_id in range(trans_chunks_count):
        if _chunk_payload_len(chunk_id) == 0:
            missing_chunks.append(chunk_id)
    return missing_chunks

def add_chunk(msgbytes):
    # Input: msgbytes: bytes containing chunk id + index + payload; Output: None (stores chunk data)
    global trans_data_id, trans_chunks_count
    if len(msgbytes) < IMG_ID_BYTES + CHUNK_ID_BYTES + 1:
        logger.error(f"[CHUNK] not enough bytes {len(msgbytes)} : {msgbytes}")
        return
    try:
        filedata_id = msgbytes[0:IMG_ID_BYTES+1].decode()
        chunk_id = int.from_bytes(msgbytes[IMG_ID_BYTES+1:IMG_ID_BYTES+CHUNK_ID_BYTES+1], 'big')
        chunk_data = msgbytes[IMG_ID_BYTES+CHUNK_ID_BYTES+1:]

        # Verify this chunk belongs to current transfer
        if filedata_id != trans_data_id:
            logger.error(f"[CHUNK] no chunks storage for filedata_id={filedata_id}, chunk_index={chunk_id} (chunk may have arrived before B packet or chunks were cleared)")
            return

        # Verify chunk_id is within valid range
        if trans_chunks_count == 0 or chunk_id >= trans_chunks_count:
            logger.error(f"[CHUNK] chunk_id {chunk_id} out of range (expected 0-{trans_chunks_count-1})")
            return

        chunk_len = len(chunk_data)
        if chunk_len > CHUNK_DATA_SIZE:
            logger.error(f"[CHUNK] chunk_id {chunk_id} payload {chunk_len}B exceeds slot size {CHUNK_DATA_SIZE}B")
            return

        # Copy into pre-allocated block (replaces if duplicate, which is fine)
        block = _chunk_block_offset(chunk_id)
        CHUNK_STORAGE_BUFFER[block + 1:block + 1 + chunk_len] = chunk_data
        CHUNK_STORAGE_BUFFER[block] = chunk_len

        missing = get_missing_chunks(filedata_id)
        received = trans_chunks_count - len(missing)
        # Log progress every 20 chunks or when complete for debugging
        if received % 10 == 0 or received == trans_chunks_count:
            logger.info(f"[IMG] Received chunk {chunk_id}: {received}/{trans_chunks_count} chunks complete (missing={len(missing)})")
    except Exception as e:
        logger.error(f"[CHUNK] Error adding chunk: {e}, msgbytes_len={len(msgbytes)}")

def get_data_for_chunk_id(chunkiter):
    # Input: chunkiter: int chunk index; Output: memoryview or None for specific chunk
    global trans_chunks_count
    if trans_chunks_count == 0 or chunkiter >= trans_chunks_count:
        return None
    chunk_len = _chunk_payload_len(chunkiter)
    if chunk_len == 0:
        return None
    block = _chunk_block_offset(chunkiter)
    return memoryview(CHUNK_STORAGE_BUFFER)[block + 1:block + 1 + chunk_len]

def recompile_msg(filedata_id):
    # Input: filedata_id: str chunk identifier; Output: bytes reconstructed message or None if incomplete
    global trans_data_id, trans_chunks_count, IMAGE_RECOMPILE_BUFFER

    if trans_chunks_count == 0 or filedata_id != trans_data_id:
        logger.warning(f"[CHUNK] recompile_msg: no chunks storage for filedata_id={filedata_id}")
        return None

    if len(get_missing_chunks(filedata_id)) > 0:
        return None

    # Use pre-allocated buffer if available, otherwise fall back to dynamic allocation
    if IMAGE_RECOMPILE_BUFFER is not None:
        try:
            # Calculate total size needed
            total_size = 0
            chunk_sizes = []
            for chunk_id in range(trans_chunks_count):
                chunk_data = get_data_for_chunk_id(chunk_id)
                if chunk_data is None:
                    logger.error(f"[CHUNK] recompile_msg: missing chunk {chunk_id} for filedata_id={filedata_id}")
                    return None
                chunk_size = len(chunk_data)
                chunk_sizes.append(chunk_size)
                total_size += chunk_size

            # Validate size fits in buffer
            if total_size > len(IMAGE_RECOMPILE_BUFFER):
                logger.error(f"[CHUNK] Image size {total_size} exceeds buffer size {len(IMAGE_RECOMPILE_BUFFER)}")
                return None

            # Work directly with the pre-allocated buffer (NO SLICING - that would copy!)
            # Copy chunks into the buffer
            offset = 0
            for chunk_id in range(trans_chunks_count):
                chunk_data = get_data_for_chunk_id(chunk_id)
                chunk_size = chunk_sizes[chunk_id]
                # Direct assignment into buffer - no copy
                IMAGE_RECOMPILE_BUFFER[offset:offset + chunk_size] = chunk_data
                offset += chunk_size

            # return bytes(IMAGE_RECOMPILE_BUFFER[:total_size])
            # Return memoryview directly - zero-copy, no allocation
            # memoryview slicing is zero-copy (unlike bytearray slicing which creates a copy)
            # file.write() in MicroPython accepts memoryview, so no conversion needed
            # This completely avoids memory allocation for the return value
            return memoryview(IMAGE_RECOMPILE_BUFFER)[:total_size]

        except MemoryError as e:
            logger.error(f"[CHUNK] MemoryError in recompile_msg for {filedata_id}: {e}")
            free_mem = get_free_memory()
            logger.info(f"[MEM] Free memory after MemoryError: {free_mem}KB")
            return None
        except Exception as e:
            logger.error(f"[CHUNK] Exception in recompile_msg for {filedata_id}: {e}")
            free_mem = get_free_memory()
            logger.info(f"[MEM] Free memory after exception: {free_mem}KB")
            return None
    else:
        logger.error(f"[CHUNK] recompile_msg not IMAGE_RECOMPILE_BUFFER initialized")
        return None
        # try:
        #     total_size = 0
        #     chunk_sizes = []
        #     for chunk_id in range(trans_chunks_count):
        #         chunk_data = get_data_for_chunk_id(chunk_id)
        #         if chunk_data is None:
        #             logger.error(f"[CHUNK] recompile_msg_fallback: missing chunk {chunk_id} for filedata_id={filedata_id}")
        #             return None
        #         chunk_size = len(chunk_data)
        #         chunk_sizes.append(chunk_size)
        #         total_size += chunk_size

        #     # Pre-allocate bytearray with exact size needed
        #     recompiled = bytearray(total_size)

        #     # Second pass: copy chunks into pre-allocated bytearray
        #     offset = 0
        #     for chunk_id in range(trans_chunks_count):
        #         chunk_data = get_data_for_chunk_id(chunk_id)
        #         recompiled[offset:offset + chunk_sizes[chunk_id]] = chunk_data
        #         offset += chunk_sizes[chunk_id]

        #     return bytes(recompiled)
        # except Exception as e:
        #     logger.error(f"[CHUNK] Exception in recompile_msg_fallback for {filedata_id}: {e}")
        #     free_before = get_free_memory()
        #     logger.info(f"[MEM] Free memory after recompile_msg_fallback in fail: {free_before/1024:.1f}KB")
        #     return None


# Note only sends as many as wouldnt go beyond frame size
# Assumption is that subsequent end chunks would get the rest
def end_chunk(msg):
    # is_all_chunk_arrived, missing_chunk_str, filedata_id, recompiled_msgbytes, epoch_ms
    global trans_data_id, trans_prev_data_id, trans_chunks_count
    parts = msg.split(":")
    if len(parts) != 2:
        logger.error(f"[CHUNK] end message unparsable {msg}")
        return (False, "", None, None, None)
    filedata_id = parts[0]
    epoch_ms = int(parts[1])

    if filedata_id != trans_prev_data_id and filedata_id != trans_data_id:
        logger.warning(f"[CHUNK] end_chunk: filedata_id={filedata_id} is not in chunks storage, cannot determine missing chunks")
        return (False, b"", filedata_id, None, epoch_ms) # TODO check for "0"

    missing_chunks = get_missing_chunks(filedata_id)
    if len(missing_chunks) > 0:
        logger.info(f"[CHUNK] I am missing {len(missing_chunks)}/{trans_chunks_count} chunks: first 20 = {missing_chunks[:20]}")
        # Reserve space for msg_uid + ":" + safety; pack missing IDs as fixed-width bytes
        max_missing_bytes = PACKET_PAYLOAD_LIMIT - HEADER_JOINED_LEN - MSG_UID_LEN - 1
        missing_bytes = bytearray()
        for idx, chunk_id in enumerate(missing_chunks):
            chunk_bytes = chunk_id.to_bytes(CHUNK_ID_BYTES, "big")
            if len(missing_bytes) + len(chunk_bytes) <= max_missing_bytes:
                missing_bytes.extend(chunk_bytes)
            else:
                # Truncate - sender will send remaining chunks after getting this list in next round
                logger.warning(f"[CHUNK] Missing chunk list truncated at {idx}/{len(missing_chunks)} chunks due to payload limit (will request remaining in next end packet)")
                break
        return (False, bytes(missing_bytes), filedata_id, None, epoch_ms)
    else:
        recompiled_msgbytes = recompile_msg(filedata_id)
        if recompiled_msgbytes:
            return (True, b"", filedata_id, recompiled_msgbytes, epoch_ms)
        else:
            if filedata_id == trans_prev_data_id: # This has been proccessed before
                logger.warning(f"[CHUNK] end_chunk: filedata_id={filedata_id} has been proccessed before, sending success...")
                return (True, b"", filedata_id, None, epoch_ms)
            else:
                logger.error(f"[CHUNK] Failed to recompile message for {filedata_id}")
                return (False, b"", filedata_id, None, epoch_ms)


# ---------------------------------------------------------------------------
# Command Center Integration
# ---------------------------------------------------------------------------

async def init_tracx_internet():
    # Input: None; Output: bool indicating cellular initialization success (updates internet_module)
    """Initialize the cellular connection"""
    global internet_module, tracx_uart, tracx_uart_lock
    logger.info("\n[CELL] === Initializing Internet Module ===")

    # Create shared UART if not already created
    if tracx_uart is None:
        from machine import UART
        from internet_driver import UART_ID, BAUDRATE
        logger.info(f"[CELL] Creating shared UART (id={UART_ID}, baud={BAUDRATE})...")
        tracx_uart = UART(UART_ID, BAUDRATE, timeout=2000)
        await asyncio.sleep(1)  # safe sleep for stable power supply

    # Hold UART lock during init to avoid conflict with GPS
    async with tracx_uart_lock:
        internet_module = InternetDriver(uart=tracx_uart)
        await internet_module.establish_internet()
        if not internet_module.initialized:
            logger.fatal("[CELL] Internet initialization failed; will retry periodically")
            return False
    logger.info("[CELL] Internet module ready")
    return True

async def keep_checking_internet():
    """Retry establish_internet() every INTERNET_RECHECK_INTERVAL_SEC while offline."""
    global internet_module, tracx_uart_lock, network_paths
    if internet_module and internet_module.has_internet:
        logger.info("[CELL] Internet health loop started in background (already online)")
    else:
        logger.info("[CELL] Internet health loop started in background currenlty offline)")
    while True:
        await asyncio.sleep(INTERNET_RECHECK_INTERVAL_SEC)
        if internet_module and internet_module.has_internet:
            continue
        logger.info("[CELL] Periodic internet check (has_internet=False)...")
        try:
            async with tracx_uart_lock:
                await internet_module.establish_internet(retry_count=2)
                if internet_module and internet_module.has_internet:
                    network_paths = []
                    logger.info("[CELL] ᯤᯤᯤᯤᯤᯤ❯❯ Now established, device will as as CC now... ❮❮ᯤᯤᯤᯤᯤᯤ")
        except Exception as e:
            logger.warning(f"[CELL] Periodic internet check failed: {e}")

async def upload_payload_to_server(payload, msg_typ, creator): # FINAL
    """Unified payload upload: sends data to cloud via cellular (for command center)."""
    global internet_module, tracx_uart_lock

    if not running_as_cc():
        logger.warning(f"upload called from unit node, skipping uploads")
        return False
    if internet_module.is_busy:
        logger.warning("Internet module is busy, skipping upload...")
        return False
    if not internet_module:
        app_controller.create_and_send_message("verify_internet", {"message": "Internet module not initialized"}, timeout=0.5)
        logger.warning(f"Internet module not initialized")
        return False

    try:
        # Give a small yield to allow other tasks to complete (important for UART sharing)
        await asyncio.sleep_ms(100)

        logger.debug(f"msg_typ:{msg_typ} from node {creator} - Starting cellular upload...")
        app_controller.create_and_send_message("verify_internet", {"message": "Starting cellular upload"}, timeout=0.5)
        signal_strength = internet_module.get_last_signal_strength()
        app_controller.create_and_send_message("verify_internet", {"message": f"Uploading data, signal strength: {signal_strength if signal_strength is not None else 'unknown'}%"}, timeout=0.5)

        # Hold UART lock to prevent GPS from using it concurrently
        async with tracx_uart_lock:
            result ,_,response_message = await internet_module.upload_data(payload, URL)

        if result:
            logger.info(f"msg_typ:{msg_typ} from node {creator} sent to cloud successfully")
            app_controller.create_and_send_message("verify_internet", {"message": "Sent to cloud successfully"}, timeout=0.5)
            return True
        else:
            app_controller.create_and_send_message("verify_internet", {"message": f"Failed to send to cloud via cellular: {response_message}"}, timeout=0.5)
            logger.error(f"msg_typ:{msg_typ} from node {creator} failed to send to cloud via cellular, error:{response_message}")
            return False

    except Exception as e:
        app_controller.create_and_send_message("verify_internet", {"message": "Failed to send to cloud via cellular"}, timeout=0.5)
        logger.error(f"msg_typ:{msg_typ} from node {creator} error sending to cloud via cellular: {e}")
        import sys
        sys.print_exception(e)
        return False


async def send_file_main(msg_typ, creator, enc_msgbytes, epoch_ms, md5, encryption_enabled, next_dst):
    """
    Send one encrypted file/blob to cloud (command center) or to next hop via chunked LoRa.
    msg_typ: "P" (image/event).
    next_dst: required for mesh nodes; ignored when running_as_cc().
    Returns: sent_succ (bool)
    """
    if msg_typ not in ("P"):
        logger.error(f"[FILE] send_file_main: invalid msg_typ={msg_typ}")
        return False

    log_tag = "[IMG]"
    server_msg_typ = "F"
    file_label = f"{creator}_{epoch_ms}.enc"
    sent_succ = False

    if running_as_cc():
        if isinstance(enc_msgbytes, bytes) or isinstance(enc_msgbytes, bytearray) or isinstance(enc_msgbytes, memoryview):
            data_b64_str = ubinascii.b2a_base64(enc_msgbytes).rstrip().decode()
        else:
            data_b64_str = enc_msgbytes
        server_payload = {
            "machine_id": creator,
            "msg_typ": server_msg_typ,
            "data": data_b64_str,
            "epoch_ms": epoch_ms,
            "enc": encryption_enabled,
        }
        logger.info(f"{log_tag} ⋙⋙⋙ Uploading encrypted image (size: {len(enc_msgbytes)} bytes), file: {creator}_{epoch_ms}")
        sent_succ = await upload_payload_to_server(server_payload, server_msg_typ, creator)
        return sent_succ
    else:
        logger.info(f"{log_tag} Sending file msg_typ={msg_typ}, creator={creator}, size={len(enc_msgbytes)} bytes")
        logger.info(f"{log_tag} ⋙⋙⋙ sending encrypted file to {next_dst}, file: {file_label}")
        try:
            if next_dst:
                if is_device_busy(next_dst):
                    logger.warning(f"{log_tag} Device {next_dst} is busy, skipping send")
                    sent_succ = False
                else:
                    sent_succ, err_msg = await send_msg_big(msg_typ, creator, enc_msgbytes, next_dst, epoch_ms, md5)
                    if not sent_succ:
                        logger.error(f"{log_tag} forwarding to {next_dst} failed, error: {err_msg}")
            else:
                logger.error(f"{log_tag} can't forwar file msg_typ=[{msg_typ}] because I dont have next device in spath yet")
                sent_succ = False
        except Exception as e:
            logger.error(f"{log_tag} unexpected error sending file to next device: {e}")
            sent_succ = False
    return sent_succ


async def send_file_main_or_enqueue(msg_typ, creator, enc_msgbytes, epoch_ms, md5, encryption_enabled, next_dst):
    """
    Try send_file_main (cloud or mesh) first. If that fails, persist via db_store->store_image ("P") so the existing send loops can retry later.
    Returns Boolean, True if sent successful or Queued, False otherwise.
    """
    global db_store
    if msg_typ not in ("P"):
        logger.error(f"[FILE] send_file_main_or_enqueue: invalid msg_typ={msg_typ}")
        return False
    if not md5:
        md5 = ubinascii.hexlify(hashlib.md5(enc_msgbytes).digest()).decode()
    transmission_start = get_ms_diff()
    sent = await send_file_main(msg_typ, creator, enc_msgbytes, epoch_ms, md5, encryption_enabled, next_dst)
    if sent:
        transmission_end = get_ms_diff()
        transmission_time = transmission_end - transmission_start
        if msg_typ == "P":
            db_store.update_img_sent_count(1)
        logger.info(f"[IMG] ✔✔✔ Data[{msg_typ}] transmission completed in {transmission_time/1000:.4f} seconds, file: {creator}_{epoch_ms} to {next_dst}")
        return True
    else:
        logger.error(f"[FILE] send failed, enqueuing to db_store")
        if msg_typ == "P":
            store_succ, err = db_store.store_image(epoch_ms, creator, 0, enc_msgbytes)
            if not store_succ:
                logger.error(f"[FILE] store_image enqueue failed, creator={creator} epoch={epoch_ms}, error={err}")
            return store_succ
        else:
            logger.error(f"[FILE] send_file_main_or_enqueue: invalid msg_typ={msg_typ}")
            return False

# ---------------------------------------------------------------------------
# Heartbeat (H)
# ---------------------------------------------------------------------------

hb_map = {}

async def hb_process(msg_uid, msgbytes, sender):
    # Input: msg_uid: bytes, msgbytes: bytes, sender: int; Output: None (routes or logs heartbeat data)
    creator = int(msg_uid[1])
    if running_as_cc():
        if creator not in hb_map:
            hb_map[creator] = 0
        hb_map[creator] += 1
        logger.info(f"[HB] HB Counts = {hb_map}")

        # Send raw heartbeat data (encrypted or not) to cloud
        if isinstance(msgbytes, bytes):
            hb_b64_str = ubinascii.b2a_base64(msgbytes).rstrip().decode()
        else:
            hb_b64_str = msgbytes

        epoch_ms = get_epoch_ms()
        server_payload =  {
            "machine_id": creator,
            "msg_typ":  "H",
            "data": hb_b64_str,
            "epoch_ms": epoch_ms,
            "enc": ENCRYPTION_ENABLED
        }

        logger.info(f"[HB] Uploading raw heartbeat data of length {len(msgbytes)} bytes...")
        asyncio.create_task(upload_payload_to_server(server_payload, "H", creator))
        if ENCRYPTION_ENABLED and is_rsa_encrypted("H"):
            try:
                decrypted_msg = enc.decrypt_rsa(msgbytes, encnode.get_prv_key(creator))
                logger.debug(f"[HB] HB send msg = {decrypted_msg}")
            except Exception as e:
                logger.error(f"[HB] Failed to decrypt HB message: {e}")
        else:
            try:
                logger.debug(f"[HB] HB send msg = {msgbytes.decode()}")
            except Exception as e:
                logger.error(f"[HB] Failed to decode HB message: {e}")

        return
    else:
        next_dst = next_device_in_spath()
        if next_dst:
            sent_succ = False
            logger.info(f"[HB] Propogating H to {next_dst}")
            sent_succ = await send_msg("H", creator, msgbytes, next_dst)
            if not sent_succ:
                logger.error(f"[HB] forwarding HB to {next_dst} failed")
        else:
            logger.error(f"[HB] can't forward HB because I dont have next device in spath yet")

# # @@@@@@@@@@@@@@@@@@@@
# # Method 2
# # @@@@@@@@@@@@@@@@@@@@

pir_burst_in_progress = False
pir_trigger_event = asyncio.Event()
pir_last_trigger_time = 0
PIR_DEBOUNCE_MS = 2000  # 2 seconds debounce to prevent multiple triggers from single motion

def pir_interrupt_handler(pin):
    """IRQ handler for PIR sensor - triggers on RISING edge. Ignored while burst capture is in progress."""
    global pir_last_trigger_time, pir_trigger_event, pir_burst_in_progress
    if pir_burst_in_progress:
        return
    current_time = utime.ticks_ms()
    # Debounce: ignore triggers within PIR_DEBOUNCE_MS of last trigger
    if utime.ticks_diff(current_time, pir_last_trigger_time) > PIR_DEBOUNCE_MS:
        pir_last_trigger_time = current_time
        # Set event to wake up person_detection_loop
        pir_trigger_event.set()
        # logger.info(f"[PIR] Motion detected (interrupt)")

async def _send_file_and_account(event_epoch_ms, enc_msgbytes, next_dst):
    """Run send/enqueue off the PIR capture path; apply img counters when it finishes."""
    global img_capture_count
    try:
        fs_succ = await send_file_main_or_enqueue(
            "P", my_addr, enc_msgbytes, event_epoch_ms, None, ENCRYPTION_ENABLED, next_dst
        )
        if not fs_succ:
            img_capture_count -= 1
            return
    except Exception as e:
        logger.error(f"[PIR] Failed to save encrypted file: {event_epoch_ms}, error: {e}")
        img_capture_count -= 1

# PIR interrupt → capture → frame buffer → save → encrypt → queue
# HIGH_TARGET_SIZE = 25 * 1024  # 25 KB, 556 chunks
# LOW_TARGET_SIZE = 18 * 1024  # 18 KB, 388 chunks
HIGH_TARGET_SIZE = 45 * 1024
LOW_TARGET_SIZE = 35 * 1024
HIGH_COMP_QUALITY = 80
MIN_COMP_QUALITY = 15

global_comp_quality = 20

def capture_image():
    """Capture one image, store raw copy, compress, and auto-adjust global quality."""
    global img_capture_count, img_file_counter, global_comp_quality
    global db_store
    global HIGH_COMP_QUALITY, MIN_COMP_QUALITY, HIGH_TARGET_SIZE, LOW_TARGET_SIZE

    turn_ON_IR_emitter()
    try:
        img_snapshot = sensor.snapshot()
        turn_OFF_IR_emitter()
        sensor.flush()

        img_capture_count += 1
        img_file_counter += 1
        event_epoch_ms = get_epoch_ms() + img_file_counter

        jpeg_bytearray = bytes(img_snapshot.compress(quality=global_comp_quality))
        size = len(jpeg_bytearray)
        logger.info(
            f"Compressed image size: {size}, compress_quality-{global_comp_quality}"
        )  # TODO, move it debug later

        if size > HIGH_TARGET_SIZE:
            global_comp_quality = max(MIN_COMP_QUALITY, global_comp_quality - 5)
        if size < LOW_TARGET_SIZE:
            global_comp_quality = min(HIGH_COMP_QUALITY, global_comp_quality + 5)

        return img_snapshot, jpeg_bytearray, event_epoch_ms
    except Exception as e:
        logger.error(f"[PIR] Failed to capture image: {e}")
        return None, None, None
    finally:
        turn_OFF_IR_emitter()

async def person_detection_loop():
    """
    PIR interrupt-driven: on trigger, capture 5 images in a burst (no sleep between).
    All 5 images share one event. IRQ is debounced until burst finishes.
    """
    global pir_trigger_event, pir_burst_in_progress, img_capture_count, img_file_counter
    global gps_str, gps_last_time
    global APP_DISARMED, network_paths
    global db_store
    global global_comp_quality
    global HIGH_COMP_QUALITY, MIN_COMP_QUALITY, HIGH_TARGET_SIZE, LOW_TARGET_SIZE
    global USE_PIR_SENSOR
    global gps_module

    PIR_PIN.irq(trigger=Pin.IRQ_RISING, handler=pir_interrupt_handler)

    BURST_SIZE = 3
    BURST_SLEEP_TIME = 2.5
    logger.info(f"[PIR] Burst detection initialized on pin {PIR_PIN} ({BURST_SIZE} images per trigger)")
    while True:
        if is_install_mode:
            await asyncio.sleep(5)
            continue
        if APP_DISARMED or ((not running_as_cc()) and len(network_paths) == 0):
            logger.debug("Not detecting movement because disarmed")
            await asyncio.sleep(5)
            continue
        if USE_PIR_SENSOR:
            await pir_trigger_event.wait()
            pir_trigger_event.clear()
        pir_burst_in_progress = True
        logger.info(f"[IMG] ●●●●●●●●●●❯❯ Motion detected - capturing {BURST_SIZE} image... ❮❮●●●●●●●●●●")

        try:
            for i in range(BURST_SIZE):
                img_snapshot = None
                sleep_in_bursts = BURST_SLEEP_TIME if i<BURST_SIZE-1 else 0
                try:
                    led.on()
                    logger.info(f"[PIR] 🅾🅾🅾🅾🅾🅾❯❯ Capturing {i+1}/{BURST_SIZE} image... ❮❮🅾🅾🅾🅾🅾🅾")
                    img_snapshot, imgbytes, event_epoch_ms = capture_image()
                    if img_snapshot is None or imgbytes is None or event_epoch_ms is None:
                        logger.error(f"[PIR] Failed to capture image: ")
                        img_capture_count -= 1
                        led.off()
                        continue

                    db_store.store_image_raw(event_epoch_ms, my_addr, img_snapshot)
                    if img_snapshot is not None:
                        del img_snapshot
                        img_snapshot = None

                    image_id = get_rand(3)
                    lat = 0
                    lon = 0
                    try:
                        if gps_module is not None:
                            logger.debug("Getting GPS location..")
                            lat, lon = gps_module.get_saved_gps_location()
                            logger.debug(f"GPS location: {lat}, {lon}")
                        else:
                            logger.debug("[WARNING] : GPS module is not initialized, skipping GPS location")
                    except Exception as e: # Not falat error
                        logger.warning(f"[PIR] Failed to get GPS location: {e}")
                    
                    try:
                        logger.info(f"[PIR] GPS Data LAT={lat}, LON={lon}")
                        imgbytes = pack_image_meta_header(
                            lat, lon, event_epoch_ms, image_id, "F"
                        ) + imgbytes
                    except Exception as e:
                        logger.error(f"[PIR] Failed to pack image meta header: {e}")
                        img_capture_count -= 1
                        led.off()
                        continue

                    try:
                        enc_msgbytes = encrypt_if_needed("P", imgbytes)
                        try:
                            del imgbytes
                        except NameError:
                            pass
                        if enc_msgbytes is None:
                            logger.error("[PIR] encrypt_if_needed returned enc_msgbytes=None")
                            img_capture_count -= 1
                            led.off()
                            continue
                        next_dst = next_device_in_spath() if not running_as_cc() else None
                        asyncio.create_task(_send_file_and_account(event_epoch_ms, enc_msgbytes, next_dst))

                    except Exception as e:
                        logger.error(f"[PIR] Failed to save encrypted file: {event_epoch_ms}, error: {e}")
                        img_capture_count -= 1
                        continue

                    led.off()
                    await asyncio.sleep(sleep_in_bursts)
                except Exception as e:
                    led.off()
                    await asyncio.sleep(sleep_in_bursts)
                    logger.fatal(f"[PIR] unexpected error in image taking and saving for burst {i}: {e}")
                finally:
                    try:
                        if img_snapshot is not None:
                            del img_snapshot
                            gc.collect()
                    except Exception as e:
                        logger.warning(f"warning cleaning up image: {e}, can be ignored...")
                    led.off()
            await asyncio.sleep(35 if USE_PIR_SENSOR else 900)
        except Exception as e:
            await asyncio.sleep(35 if USE_PIR_SENSOR else 900)
            logger.error(f"[PIR] unexpected error in event taking and saving: {e}")

        finally:
            pir_burst_in_progress = False

async def image_sending_loop():
    # Input: None; Output: None (periodically sends queued images across mesh)
    global trans_in_progress
    global db_store

    IMAGE_SENDING_EMPTY_DELAY = 30
    IMAGE_SENDING_LITE_DELAY = 40
    IMAGE_SENDING_NEXT_INTERVAL = 50
    IMAGE_SENDING_FAILED_PAUSE = 60
    IMAGE_SENDING_FAILED_PAUSE_2 = 80

    while True:
        if is_install_mode:
            await asyncio.sleep(IMAGE_SENDING_EMPTY_DELAY)
            continue
        logger.debug(f"Image sending outer loop, size of img list = {db_store.get_img_queued_count()}")
        if db_store.get_img_queued_count() == 0:
            logger.debug("[IMG] No image found to send, skipping sending...")
            await asyncio.sleep(IMAGE_SENDING_EMPTY_DELAY)
            continue
        next_dst = next_device_in_spath()
        if not running_as_cc() and not next_dst:
            logger.warning("[IMG] No shortest path yet so cant send...")
            await asyncio.sleep(IMAGE_SENDING_LITE_DELAY)
            continue

        if trans_in_progress: # in receiving mode, so skip sending
            logger.info(f"[IMG] Trans mode is active, skipping sending...")
            await asyncio.sleep(IMAGE_SENDING_LITE_DELAY)
            continue

        if is_device_busy(next_dst):
            logger.debug(f"[IMG] Device {next_dst} is busy, skipping sending...")
            await asyncio.sleep(IMAGE_SENDING_LITE_DELAY)
            continue

        while db_store.get_img_queued_count() > 0:
            if trans_in_progress:
                logger.info(f"[IMG] Trans mode is active, breaking file sending while loop...")
                break
            epoch_ms, creator, retry, enc_msgbytes, md5 = db_store.get_next_image_to_send()
            if epoch_ms is None:
                logger.debug("[IMG] No valid next image to send (db_store returned empty), breaking inner loop")
                break
            logger.info(f"[IMG] Processing image_sending_loop : {creator}_{epoch_ms}.enc (md5={md5})")
            try:
                # Read encrypted bytes directly from file
                logger.debug(f"[IMG] Reading encrypted image of creator: {creator}, file: {creator}_{epoch_ms}.enc")

                transmission_start = get_ms_diff()
                sent_succ = await send_file_main(
                    "P", creator, enc_msgbytes, epoch_ms, md5, ENCRYPTION_ENABLED, next_dst
                )
                if not sent_succ:
                    store_succ, err = db_store.store_image(epoch_ms, creator, retry + 1, enc_msgbytes, False)
                    if not store_succ:
                        logger.error(f"[IMG] Failed to re-queue image {creator}_{epoch_ms}.enc after send failure, error={err}")
                    else:
                        logger.warning(f"[IMG] upload_payload to server failed, image of creator={creator}, re-queued: {creator}_{epoch_ms}.enc")
                    await asyncio.sleep(IMAGE_SENDING_LITE_DELAY)
                    continue

                transmission_end = get_ms_diff()
                transmission_time = transmission_end - transmission_start
                db_store.update_img_sent_count(1)
                logger.info(f"[IMG] ✔✔✔ Image transmission completed in {transmission_time} ms ({transmission_time/1000:.4f} seconds), file: {creator}_{epoch_ms} to {next_dst}")

                if db_store.get_img_queued_count() > 0:
                    await asyncio.sleep(IMAGE_SENDING_NEXT_INTERVAL)

            except Exception as e:
                logger.error(f"[IMG] unexpected error processing image event {creator}_{epoch_ms}.enc: {e}, re-queued")
                store_succ, err = db_store.store_image(epoch_ms, creator, retry + 1, enc_msgbytes, False)
                if not store_succ:
                    logger.error(f"[IMG] Failed to re-queue image {creator}_{epoch_ms}.enc after exception, error={err}")
                else:
                    logger.warning(f"[IMG] upload_payload to server failed, image of creator={creator}, re-queued: {creator}_{epoch_ms}.enc")
                await asyncio.sleep(IMAGE_SENDING_LITE_DELAY)
                continue
            finally:
                try:
                    if enc_msgbytes is not None:
                        del enc_msgbytes
                except NameError:
                    pass
                except:
                    pass
                gc.collect()

        if db_store.get_img_queued_count() == 0:
            logger.info(f"[IMG] Queue empty, all images uploaded")
        if db_store.get_img_queued_count() > 0:
            await asyncio.sleep(random.uniform(IMAGE_SENDING_FAILED_PAUSE, IMAGE_SENDING_FAILED_PAUSE_2))

def process_message(databytes, rssi=None):
    # Input: databytes: bytes raw LoRa payload; rssi: int or None RSSI value in dBm; Output: bool indicating if message was processed
    global is_install_mode, db_store

    success, msg_uid, msg_typ, creator, sender, receiver, msgbytes = parse_header(databytes)
    if not success:
        logger.error(f"[LORA] failure parsing incoming databytes : {databytes}")
        return False
    if random.randint(1,100) <= FLAKINESS:
        logger.warning(f"[LORA] flakiness dropping {databytes}")
        return True

    if receiver != -1 and my_addr != receiver:
        logger.debug(f"[LORA] skipping message as it is for dst:{receiver}, not for me (my_addr:{my_addr}), msg_uid:{msg_uid}")
        return

    # if is_install_mode and msg_typ not in ["X", "Y", "Z", "A", "H"]:
    #     logger.debug(f"[LORA] skipping message as it is in install mode and msg_typ not in [X, Y, Z, A, H]")
    #     return False

    recv_log = ""
    if receiver == -1:
        recv_log = "⬇ BCAST"
    else:
        recv_log = "⬇ RECV"

    data_masked_log = min(10, max(1, (len(databytes) + 20) // 21))
    rssi_log = f", rssi: {rssi}" if rssi is not None else ""
    if is_install_mode and msg_typ not in ["X", "Y", "Z", "A", "H", "K"]:
        logger.info(f"[{recv_log} from {sender}{rssi_log}] [{'*' * data_masked_log}] {len(databytes)} bytes, msg_typ= {msg_typ}, MSG_UID = {msg_uid}, skipping msg in install mode...")
        return
    elif msg_typ != "I":
        logger.info(f"[{recv_log} from {sender}{rssi_log}] [{'*' * data_masked_log}] {len(databytes)} bytes, MSG_UID = {msg_uid}")

    # logger.info(f"[PARSED HEADER] msg_uid:{msg_uid}, msg_typ:{msg_typ}, creator:{creator}, sender:{sender}, receiver:{receiver}, len-msgbytes:{len(msgbytes)}")
    if sender not in recv_msg_count:
        recv_msg_count[sender] = 0
    recv_msg_count[sender] += 1
    ackmessage = msg_uid
    if msg_typ == "H":
        asyncio.create_task(send_msg("A", my_addr, ackmessage, sender))
        # Validate heartbeat message payload length for encrypted messages
        if is_rsa_encrypted("H") and len(msgbytes) != 128:
            logger.error(
                f"[HB] Invalid payload length: {len(msgbytes)} bytes, expected 128 bytes for encrypted message. "
                f"MID: {msg_uid}, may be corrupted or incomplete."
            )
        else:
            asyncio.create_task(hb_process(msg_uid, msgbytes, sender))
    elif msg_typ == "W":  # wait message
        asyncio.create_task(device_busy_life(sender))
    elif msg_typ == "B": # TODO need to ignore duplicate images, and send some response in A itself
        try:
            filedata_id, msg_typ, numchunks, md5 = begin_chunk(msgbytes) # msg_typ as "P"
            if filedata_id is None or numchunks is None:
                logger.error(f"[CHUNK] Invalid B packet, cannot get filedata_id/numchunks")
                return False
            if is_install_mode:
                logger.warmning(f"[CHUNK] TRANS MODE not allowed in install mode, ignoring 'B'...")
                return False
            # Check if this is a duplicate B packet for the same transfer
            if check_transmode_lock(sender, filedata_id):
                # Same sender and filedata_id - send ACK anyway (duplicate begin packet)
                logger.debug(f"[CHUNK] Duplicate B packet for same transfer, sending ACK")
                asyncio.create_task(send_msg("A", my_addr, ackmessage, sender))
            elif db_store.storage_available(creator):
                if get_transmode_lock(sender, filedata_id, msg_typ, numchunks, md5):
                    asyncio.create_task(keep_transmode_lock(sender, filedata_id))
                    asyncio.create_task(send_msg("A", my_addr, ackmessage, sender))
                else:
                    logger.warning(f"[CHUNK] TRANS MODE already in use for different transfer, sending W...")
                    asyncio.create_task(send_msg("W", my_addr, WAIT_MESSAGE, sender))
                    return False
            else:
                logger.warning(f"[CHUNK] Storage not available, sending W...")
                asyncio.create_task(send_msg("W", my_addr, WAIT_MESSAGE, sender))
                return False
        except Exception as e:
            logger.error(f"[CHUNK] decoding unicode {e} : {msgbytes}")
            return False
    elif msg_typ == "I":
        try:
            if len(msgbytes) > IMG_ID_BYTES:
                filedata_id = msgbytes[0:IMG_ID_BYTES+1].decode()
                if check_transmode_lock(sender, filedata_id):
                    add_chunk(msgbytes)
                else:
                    logger.warning(f"[IMG RX] No transmode lock found for filedata_id {filedata_id}, skipping chunk...")
            else:
                logger.warning(f"[IMG RX] Chunk I message too short ({len(msgbytes)} bytes), cannot extract filedata_id")
        except Exception as e:
            logger.error(f"[IMG RX] Error processing chunk I: {e}")
    elif msg_typ == "E": #
        global trans_msg_typ, trans_chunk_md5
        global stats_failed_count
        # Process end chunk and respond with missing chunks list or completion confirmation
        free_before = get_free_memory()
        logger.info(f"[IMG RX] Free memory before End chunk: {free_before}KB")
        try:
            alldone, missing_bytes, filedata_id, recompiled_msgbytes, epoch_ms = end_chunk(msgbytes.decode()) # TODO later, check how can we validate file
        except UnicodeError as e:
            logger.error(f"[IMG RX] Unicode decode error in End chunk: {e} : {msgbytes}")
            return False
        except Exception as e:
            logger.error(f"[IMG RX] Error in end_chunk for End chunk: {e}")
            return False
        if alldone:
            if recompiled_msgbytes:
                try:
                    computed_md5 = ubinascii.hexlify(hashlib.md5(recompiled_msgbytes).digest()).decode()
                    # Only validate when sender sent a non-empty md5 (legacy or chunk-forwarded entries may have no md5)
                    if trans_chunk_md5 and trans_chunk_md5 != computed_md5:
                        logger.error(f"[IMG RX] Invalid md5 for the file got transferred, expected={trans_chunk_md5}, computed={computed_md5}")
                        del recompiled_msgbytes
                        gc.collect()
                        return False
                    else:
                        logger.info(f"[IMG RX] ✔✔✔ [VALID MD5 FILE] for the file got transferred")
                except Exception as e:
                    logger.error(f"[IMG RX] Error checking md5 for the file got transferred: {e}")
                    del recompiled_msgbytes
                    gc.collect()
                    return False

                try:
                    cleanup_chunk_map_by_msg_id(filedata_id)
                except Exception as e:
                    logger.error(f"[IMG RX] Error cleaning up chunk map for filedata_id {filedata_id}: {e}")

                try:
                    async def _chunk_end_send_or_enqueue(trans_msg_typ_curr, trans_md5_curr):
                        nonlocal recompiled_msgbytes
                        global db_store
                        next_dst = next_device_in_spath() if not running_as_cc() else None
                        ok = await send_file_main_or_enqueue(
                            trans_msg_typ_curr, creator, recompiled_msgbytes, epoch_ms, trans_md5_curr, ENCRYPTION_ENABLED, next_dst
                        )
                        if not ok:
                            logger.error(
                                f"[CHUNK] send_file_main_or_enqueue failed for type={trans_msg_typ_curr} creator={creator} epoch={epoch_ms}"
                            )
                            try:
                                del recompiled_msgbytes
                            except Exception:
                                pass
                            gc.collect()
                            return
                        logger.info(f"[CHUNK] file type={trans_msg_typ_curr} sent or queued for {creator}_{epoch_ms}.enc")

                        global DECRYPT_IMAGE_ON_HOPS
                        if trans_msg_typ_curr == "P" and DECRYPT_IMAGE_ON_HOPS:
                            try:
                                img_bytes = None
                                img = None
                                try:
                                    img_bytes = enc.decrypt_hybrid(recompiled_msgbytes, encnode.get_prv_key(creator))
                                    img = image.Image(320, 240, image.JPEG, buffer=img_bytes)
                                    db_store.store_image_raw(epoch_ms, creator, img)
                                    logger.info(f"[IMG RX] Saved raw image: {creator}_{epoch_ms}_raw.jpg: raw size = {len(img_bytes)} bytes")
                                except Exception as e:
                                    logger.error(f"[IMG RX] Failed to decrypt/save raw image: {e}")
                                finally:
                                    if img_bytes is not None:
                                        del img_bytes
                                    if img is not None:
                                        del img
                            except Exception as e:
                                logger.error(f"[IMG RX] Failed to decrypt/save raw image: {e}")
                        try:
                            del recompiled_msgbytes
                        except Exception:
                            pass
                        gc.collect()


                    # send ack only after saving the IMG; empty payload after ":" means no missing chunks
                    ackmessage += b":"
                    trans_msg_typ_copy = trans_msg_typ
                    trans_chunk_md5_copy = trans_chunk_md5
                    async def send_ack_multiple(): # send ACK 2 times
                        msg_count = 2
                        for i in range(msg_count):
                            await send_msg("A", creator, ackmessage, sender)
                            if i < msg_count-1:
                                await asyncio.sleep(ACK_SLEEP)  # Delay between multiple ACK sends for reliability
                    asyncio.create_task(send_ack_multiple())
                    delete_transmode_lock(sender, filedata_id, True)

                    asyncio.create_task(_chunk_end_send_or_enqueue(trans_msg_typ_copy, trans_chunk_md5_copy))
                except Exception as e:
                    logger.error(f"[IMG RX] Error scheduling chunk end (send/enqueue): {e}")
                    try:
                        del recompiled_msgbytes
                    except Exception:
                        pass
                    gc.collect()
            else:
                logger.warning(f"[CHUNK] img not recompiled, might have complied last time")
        else:
            if not missing_bytes: # ERROR case, not compiled, not missing
                delete_transmode_lock(sender, filedata_id, False)
            else:
                ackmessage += b":" + missing_bytes
                asyncio.create_task(send_msg("A", my_addr, ackmessage, sender))
    elif msg_typ == "X":
        asyncio.create_task(network_response_generate(sender))
    elif msg_typ == "Y":
        asyncio.create_task(network_response_consume(msgbytes , sender))
    elif msg_typ == "Z":
        asyncio.create_task(send_msg("A", my_addr, ackmessage, sender))
    elif msg_typ == "K":
        msgstr = msgbytes.decode()
        global trans_paired_device, trans_data_id
        if msgstr == "install_mode":
            if not is_install_mode:
                asyncio.create_task(enter_install_mode())
            asyncio.create_task(send_msg("J", my_addr, b"recvdInstallMode", sender))
            delete_transmode_lock(trans_paired_device, trans_data_id)
        elif msgstr == "exit_install_mode":
            if is_install_mode:
                asyncio.create_task(exit_install_mode())
                asyncio.create_task(send_msg("J", my_addr, b"recvdExitInstallMode", sender))
        elif msgstr == "get_device_status":
            connection_status = app_controller.check_connection()
            msg = {"M": "I" if is_install_mode else "P", "Wi": connection_status["wifi_connected"], "So": connection_status["socket_connected"]}
            asyncio.create_task(send_msg("J", my_addr, json.dumps(msg).encode(), sender))
        else:
            logger.error(f"[APP] unknown command: {msgstr}")

    elif msg_typ == "A":
        ack_msgs_recd.append((msg_uid, msgbytes, get_ms_diff())) # storing only ack, of every message, like ack of B, E...
        logger.debug(f"[ACK] Received ACK message: {msg_uid}, payload: {msgbytes}")
    elif msg_typ == "D":
        logger.info(f"[DBG] Received DEBUG message, msg={msgbytes}, ignoring...")
    else:
        logger.info(f"[LORA] Unseen messages type {msg_typ}, sender={sender}, creator={creator} in {msgbytes}")
    return True

# ---------------------------------------------------------------------------
# Network Maintenance and Heartbeats (H)
# ---------------------------------------------------------------------------

def build_heartbeat_payload():  # HARD limit is 50 bytes
    """
    Build fixed-size compact heartbeat payload:
      image_taken(2), image_sent(2), image_dropped(2), image_failed(2), image_queued(2),
      radio_succ(3), radio_err(3), internet_succ(3), internet_err(3),
      fs_succ(2), fs_err(2),
      neighbours(3 x 1), shortest_path(3 x 1), process_id(3),
      version(2) — packed config.VERSION (XX.XX.X as uint16 BE, e.g. 2001 = 2.0.1),
      signal_strength(1) — cellular 0-100 % (last AT+CSQ sample; 0 if unknown),
      network_type(1) — 0=unknown, 1=LTE_HOME, 2=LTE_ROAM, 3=3G_PS_HOME, 4=3G_PS_ROAM,
      is_cc_unit(1) — 1 if running as CC, else 0,
      free_memory(2) — free heap KB (get_free_memory),
      device_uptime(2) — minutes since boot (get_uptime_minutes).

    Total size: 44 bytes (hard limit 50).
    """
    global img_capture_count, db_store, internet_module, PROCESS_ID_STR
    global radio_sent_succ_count, radio_sent_fail_count, radio_recd_succ_count, radio_recd_err_count, radio_recd_crcerr_count, radio_recd_hasherr_count

    radio_succ_count = radio_sent_succ_count + radio_recd_succ_count
    radio_fail_count = radio_sent_fail_count + radio_recd_err_count + radio_recd_crcerr_count + radio_recd_hasherr_count
    
    signal_strength = 0
    network_type = 0 # 1 byte data
    is_cc_unit = 0 # 1 byte data
    free_memory = get_free_memory() # 2 bytes data
    device_uptime = get_uptime_minutes() # 2 bytes data, in minutes, max value 43200 got 30 days
    
    if internet_module:
        signal_strength = internet_module.get_last_signal_strength() or 0
        network_type = internet_module.get_last_network_type() or 0
    
    if running_as_cc():
        is_cc_unit = 1

    hbmsg_bytes = b""
    hbmsg_bytes += int_to_nbytes(img_capture_count, 2)
    hbmsg_bytes += int_to_nbytes(db_store.get_img_sent_count(), 2)
    hbmsg_bytes += int_to_nbytes(db_store.get_img_dropped_count(), 2)
    hbmsg_bytes += int_to_nbytes(db_store.get_img_failed_count(), 2)
    hbmsg_bytes += int_to_nbytes(db_store.get_img_queued_count(), 2)

    hbmsg_bytes += int_to_nbytes(radio_succ_count, 3)
    hbmsg_bytes += int_to_nbytes(radio_fail_count, 3)
    if internet_module and internet_module.initialized:
        hbmsg_bytes += int_to_nbytes(internet_module.get_upload_success_count(), 3)
        hbmsg_bytes += int_to_nbytes(internet_module.get_upload_fail_count(), 3)
    else:
        hbmsg_bytes += int_to_nbytes(0, 3)
        hbmsg_bytes += int_to_nbytes(0, 3)
    hbmsg_bytes += int_to_nbytes(db_store.get_fs_succ_count(), 2)
    hbmsg_bytes += int_to_nbytes(db_store.get_fs_err_count(), 2)
    # 3 neighbours and 3 node from sortest path
    neighbours = get_curr_neighbours() or []
    for i in range(3):
        node_id = neighbours[i] if i < len(neighbours) else 0
        hbmsg_bytes += int_to_nbytes(node_id, 1)

    shortest_path = get_curr_spath() or []
    for i in range(3):
        node_id = shortest_path[i] if i < len(shortest_path) else 0
        hbmsg_bytes += int_to_nbytes(node_id, 1)
    # Process ID
    proc_id = (PROCESS_ID_STR or "")[:3]
    proc_id = proc_id + ("_" * (3 - len(proc_id)))
    hbmsg_bytes += proc_id.encode()
    hbmsg_bytes += int_to_nbytes(VERSION, 2)
    hbmsg_bytes += int_to_nbytes(signal_strength, 1)
    hbmsg_bytes += int_to_nbytes(network_type, 1)
    hbmsg_bytes += int_to_nbytes(is_cc_unit, 1)
    hbmsg_bytes += int_to_nbytes(free_memory, 2)
    hbmsg_bytes += int_to_nbytes(device_uptime, 2)
    return hbmsg_bytes

async def send_heartbeat():
    # Input: None; Output: bool indicating whether heartbeat was successfully sent to a neighbour
    hbmsg_bytes = build_heartbeat_payload()

    msgbytes = encrypt_if_needed("H", hbmsg_bytes)
    sent_succ = False
    if running_as_cc():
        if isinstance(msgbytes, bytes):
            hb_b64_str = ubinascii.b2a_base64(msgbytes)
        else:
            hb_b64_str = msgbytes
        epoch_ms = get_epoch_ms()
        server_payload =  {
                "machine_id": my_addr,
                "msg_typ":  "H",
                "data": hb_b64_str,
                "epoch_ms": epoch_ms,
                "enc": False
            }
        logger.info(f"[HB] sending raw HB to cloud, len={len(msgbytes)}")
        sent_succ = await upload_payload_to_server(server_payload, "H", my_addr)
        return sent_succ
    else:
        next_dst = next_device_in_spath()
        if next_dst:
            sent_succ = await send_msg("H", my_addr, msgbytes, next_dst)
            if sent_succ:
                logger.info(f"[HB] Heartbeat sent successfully to {next_dst}")
                return True
        else:
            logger.error(f"[HB] can't send HB because I dont have next device in spath yet")
            return False
    return False


async def keep_generating_heartbeat():
    # Input: None; Output: None (loops to periodically send heartbeats and handle retries)
    global consecutive_hb_failures, trans_in_progress
    global APP_DISARMED, network_paths
    global internet_module
    print_pause = True
    print_resume = False
    while True:
        await asyncio.sleep(3)
        global trans_in_progress
        if trans_in_progress:
            if print_pause:
                logger.info("[HB] PAUSED")
            print_pause = False
            print_resume = True
            await asyncio.sleep(200)
            continue
        else:
            if print_resume:
                logger.info("[HB] RESUMED")
            print_resume = False
            print_pause = True

        if running_as_unit() and len(network_paths)==0:
            logger.debug("Not sending heartbeat, because I am a unit with no network paths")
            await asyncio.sleep(5)
            continue

        if running_as_cc() and internet_module.is_busy:
            logger.debug("Not sending heartbeat, because I am a CC and internet module is busy")
            await asyncio.sleep(5)
            continue

        sent_succ = await asyncio.create_task(send_heartbeat())
        if not sent_succ:
            consecutive_hb_failures += 1
            logger.warning(f"consecutive heartbeat failures = {consecutive_hb_failures}")
            if consecutive_hb_failures >= 25: # TODO NEED to discuss more
                logger.error("Too many consecutive heartbeat failures, Rebooting device")
                try:
                    await reboot_device()
                except Exception as e:
                    logger.error(f"reinitializing LoRa: {e}")
        else:
            consecutive_hb_failures = 0
            logger.info("[HB] ✔✔✔ HB SUCCESS")
        await asyncio.sleep(HB_WAIT + random.randint(3,10))

async def send_debugmsg():
    # Input: None; Output: bool indicating whether debug payload was sent as broadcast
    dbgmsg_bytes = build_heartbeat_payload()
    msgbytes = encrypt_if_needed("D", dbgmsg_bytes)
    if msgbytes is None:
        logger.error("[DBG] Failed to build debug message payload")
        return False

    # Fire-and-forget debug stream: broadcast without expecting ACK.
    sent_succ = await send_msg("D", my_addr, msgbytes, 65535)
    if sent_succ:
        logger.info(f"[DBG] Debug message broadcasted, len={len(msgbytes)}")
        return True
    logger.warning("[DBG] Debug message broadcast failed")
    return False


async def keep_generating_debugmsg():
    while True:
        await asyncio.create_task(send_debugmsg())
        await asyncio.sleep(D_MSG_WAIT + random.randint(3,10))

def get_curr_spath():
    """ Returns the path with minimum length from network_paths """
    global network_paths
    if not network_paths:
        return None
    shortest = min(network_paths, key=lambda x: len(x["path"]))
    return shortest["path"]

def get_curr_neighbours():
    """ Returns the neighbours list from seen_neighbours """
    global seen_neighbours
    try:
        if not seen_neighbours:
            return []
        return [x.get("node") for x in seen_neighbours if isinstance(x, dict) and "node" in x]
    except Exception as e:
        logger.error(f"[NET] error in get_curr_neighbours: {e}")
        return []

def next_device_in_spath():
    """ Returns the first hop (next node) toward CC from current path, or None. """
    sp = get_curr_spath()
    return sp[0] if sp and len(sp) > 0 else None

async def network_request_loop():
    global network_paths, seen_neighbours, NETWORK_EMPTY_SLEEP, NETWORK_IN_TRANS_SLEEP, NETWORK_IMPROVE_SLEEP, NETWORK_IMPROVE_COUNT, NETWORK_STABLE_SLEEP
    network_improve_count = NETWORK_IMPROVE_COUNT
    while True:
        try:
            # clean any old spath
            curr_epoch_ms = get_epoch_ms()
            new_net_paths = [x for x in network_paths if curr_epoch_ms - x["at"] < NET_PATH_EXPIRY_MS]
            removed_net_paths = [x for x in network_paths if curr_epoch_ms - x["at"] >= NET_PATH_EXPIRY_MS]
            if len(removed_net_paths) > 0:
                logger.warning(f"[NET] cleaning old network_paths, removed: {removed_net_paths}")
            network_paths = new_net_paths

            # clean any old neighbours
            new_neighbours = [x for x in seen_neighbours if curr_epoch_ms - x["at"] < NET_PATH_EXPIRY_MS]
            removed_neighbours = [x for x in seen_neighbours if curr_epoch_ms - x["at"] >= NET_PATH_EXPIRY_MS]
            if len(removed_neighbours) > 0:
                logger.warning(f"[NET] cleaning old seen_neighbours, removed: {removed_neighbours}")
            seen_neighbours = new_neighbours

            global trans_in_progress
            if trans_in_progress:
                logger.debug(f"skipping network request, because image in progress")
                await asyncio.sleep(NETWORK_IN_TRANS_SLEEP)
                continue

            scanmsg = encode_node_id(my_addr)
            # 65535 is for Broadcast
            await send_msg("X", my_addr, scanmsg, 65535)

            if len(network_paths) == 0:
                logger.info(f"[NET] - sleeping for {NETWORK_EMPTY_SLEEP} seconds, for next network `discovery`")
                await asyncio.sleep(NETWORK_EMPTY_SLEEP)
            else:
                if network_improve_count > 0:
                    logger.info(f"[NET] - sleeping for {NETWORK_IMPROVE_SLEEP} seconds, for next network `improvement`")
                    await asyncio.sleep(NETWORK_IMPROVE_SLEEP)
                    network_improve_count -= 1
                else:
                    logger.info(f"[NET] - sleeping for {NETWORK_STABLE_SLEEP} seconds, for next network `refresh`")
                    await asyncio.sleep(NETWORK_STABLE_SLEEP)
        except Exception as e:
            logger.error(f"[NET] error in spath request loop: {e}")
            await asyncio.sleep(1)

async def network_response_generate(target):
    await asyncio.sleep(random.uniform(0, 10)) # to not overload the requester node
    try:
        if running_as_cc():
            if APP_DISARMED:
                logger.warning("Not initiating shortest path because APP_DISARMED")
                return
            # If we are the CC, respond with just our address (empty path means we are the destination)
            new_spath_msg = str(my_addr)
            logger.debug(f"[NET] sending network response as CC to :{target}, spth: {new_spath_msg}")
            asyncio.create_task(send_msg("Y", my_addr, new_spath_msg.encode(), target))
            return

        curr_spath = get_curr_spath()
        if curr_spath is None or len(curr_spath) == 0:
            logger.warning(f"[NET] No path to CC available, sending empty spath to {target}")
            new_spath_msg = ""
        else:
            new_spath = [my_addr] + curr_spath
            new_spath_msg = ",".join([str(x) for x in new_spath])
            logger.debug(f"[NET] sending network reponse to :{target}, spth: {new_spath_msg}")
        asyncio.create_task(send_msg("Y", my_addr, new_spath_msg.encode(), target))
    except Exception as e:
        logger.error(f"[NET] error in network response generation: {e}")


async def network_response_consume(msg, sender):
    global network_paths, seen_neighbours
    epoch_ms = get_epoch_ms()
    # Remove old entry if exists and add new one with updated timestamp
    seen_neighbours = [x for x in seen_neighbours if x.get("node") != sender]
    seen_neighbours.append({"node": sender, "at": epoch_ms})
    logger.info(f"[NET] updating {sender} in seen_neighbours")

    if running_as_cc():
        logger.debug(f"Ignoring shortest path since I am cc")
        return
    if len(msg) == 0:
        logger.error(f"empty spath_received message received")
        return
    try:
        # Decode bytes to string before splitting
        msg_str = msg.decode() if isinstance(msg, bytes) else msg
        spath_received = [int(x.strip()) for x in msg_str.split(",")]
    except Exception as e:
        logger.error(f"Error parsing spath message: '{msg}', error: {e}")
        return

    if my_addr in spath_received:
        logger.debug(f"[cyclic, ignoring {my_addr} already in {spath_received}")
        return

    if len(spath_received) == 0:
        logger.error(f"empty spath received")
        return


    network_paths = [x for x in network_paths if x.get("next") != sender]
    network_paths.append({"path": spath_received, "at": epoch_ms, "next": sender})
    logger.info(f"[NET] updated/added spath via node: {sender} to CC: {spath_received}")


# ---------------------------------------------------------------------------
# GPS Acquisition Loop
# ---------------------------------------------------------------------------

async def keep_updating_gps():
    # Input: None; Output: None (continuously reads GPS hardware and updates global state)
    global gps_str, gps_last_time, rtc, gps_module, tracx_uart, tracx_uart_lock, gps_success_count, gps_failure_count
    logger.info("[GPS] Initializing GPS...")

    # Wait for LoRa to settle
    await asyncio.sleep(3)

    # Create shared UART if not already created (may have been created by internet module)
    if tracx_uart is None:
        from machine import UART
        from gps_driver import UART_ID, BAUDRATE
        logger.info(f"[GPS] Creating shared UART (id={UART_ID}, baud={BAUDRATE})...")
        tracx_uart = UART(UART_ID, BAUDRATE, timeout=2000)
        import utime
        await asyncio.sleep(2)  # Wait for module to initialize

    # Create GPS module instance with shared UART
    if gps_module is None:
        logger.info("[GPS] Creating GPS driver instance with shared UART")
        try:
            gps_module = GPSDriver(uart=tracx_uart)
        except Exception as e:
            logger.error(f"[GPS] Failed to create GPS driver instance: {e}")
            return

    try:
        async with tracx_uart_lock:
            if not gps_module.initialize_gps():
                logger.info("[GPS] GPS initialization failed!")
                return
        logger.info("[GPS] GPS hardware initialized successfully - starting continuous read loop")
    except Exception as e:
        logger.info(f"[GPS] GPS initialization failed: {e}")
        return

    read_count = 0
    last_successful_read = 0
    rtc_updated = False

    # Continuous reading loop
    while True:
        try:
            read_count += 1

            # Skip GPS if heavy operations are running
            if trans_in_progress:
                await asyncio.sleep(GPS_WAIT_SEC * 2)
                continue

            # Check if gps_module is still available (might be None if reinitializing)
            if gps_module is None:
                logger.warning("[GPS] GPS module is None, waiting...")
                await asyncio.sleep(5)
                continue

            # Query GPS location and time (hold UART lock to avoid conflict with cellular)
            async with tracx_uart_lock:
                lat, lon, time_str = gps_module.get_gps_location()

            if lat is not None and lon is not None and time_str is not None:
                gps_success_count += 1
                gps_str = f"{lat:.6f},{lon:.6f}"
                logger.info(f"[GPS] {gps_str} Time: {time_str}")
                gps_last_time = get_ms_diff()
                last_successful_read = read_count

                # Update RTC with GPS time (local time) - only once when we get first fix
                if not rtc_updated:
                    try:
                        time_components = gps_module.get_gps_time_components(time_str)
                        if time_components:
                            rtc.datetime(time_components)
                            logger.info(f"[GPS] RTC updated with GPS time: {time_str}")
                            rtc_updated = True
                    except Exception as e:
                        logger.warning(f"[GPS] Failed to update RTC: {e}")
            else:
                gps_failure_count += 1
                if read_count % 20 == 1:
                    logger.info("[GPS] GPS has no fix")

            # Reinitialize if too many failures
            if last_successful_read > 0 and (read_count - last_successful_read) > 100:
                logger.info("[GPS] GPS not working, reinitializing...")
                try:
                    if gps_module is not None:
                        async with tracx_uart_lock:
                            gps_module.initialize_gps()
                    await asyncio.sleep(2)
                    last_successful_read = read_count
                except Exception as e:
                    logger.error(f"[GPS] error in GPS reinit: {e}")
                    await asyncio.sleep(10)

        except Exception as e:
            logger.error(f"[GPS] error in GPS read: {e}")
            await asyncio.sleep(2)

        # Wait before next query: 5 sec when no fix, 30 min when fix obtained
        if gps_str:
            await asyncio.sleep(max(1, GPS_WAIT_REFRESH_SEC))
        else:
            await asyncio.sleep(max(1, GPS_WAIT_SEC))

# ---------------------------------------------------------------------------
# All Handlers
# ---------------------------------------------------------------------------
app_controller = None
class AppHandler:
    def __init__(self):
        pass

    def is_cc(self):
        return running_as_cc()

    def disarm(self):
        global APP_DISARMED
        APP_DISARMED = True

    def arm(self):
        global APP_DISARMED
        APP_DISARMED = False

    def send_machine_stats_data(self): # TODO, akash remove if extra
        global gps_str, gps_last_time, gps_success_count, gps_failure_count
        gps_coords = gps_str if gps_str else ""
        if gps_last_time != -1:
            gps_staleness = int((get_ms_diff() - gps_last_time) / 1000)
        else:
            gps_staleness = -1
        curr_spath = get_curr_spath()
        msmsgstr = f"{my_addr}:{get_epoch_sec()}:{img_capture_count}:{gps_coords}:{gps_staleness}:{gps_success_count}:{gps_failure_count}:{get_curr_neighbours()}:{curr_spath}"
        return msmsgstr

    def send_hb_data(self):
        global gps_str, gps_last_time
        gps_coords = gps_str if gps_str else ""
        if gps_last_time != -1:
            gps_staleness = int((get_ms_diff() - gps_last_time) / 1000)
        else:
            gps_staleness = -1
        curr_spath = get_curr_spath()
        hbmsgstr = f"{my_addr}:{get_epoch_sec()}:{img_capture_count}:{gps_coords}:{gps_staleness}:{get_curr_neighbours()}:{curr_spath}:{APP_DISARMED}"
        return hbmsgstr

    async def check_radio_connectivity_with(self, target_addr, num_messages=10, byte_count=0):
        # TODO later, write the fucntion for one radio message, and combine them in app_contoller.py, then we dont need to call app_controller here
        """Send 10 empty Z (connectivity) messages, count ACKs, return (success_count, success_rate, transfer_rate)."""
        logger.info(f"[{my_addr}] : Checking radio connectivity with {target_addr}")
        if not isinstance(byte_count, int):
            logger.error(f"[{my_addr}] : byte_count must be an integer")
            return (0, 0, 0)
        if not isinstance(target_addr, int):
            logger.error(f"[{my_addr}] : target_addr must be an integer")
            return (0, 0, 0)

        TEST_MESSAGE_COUNT = num_messages
        success_count = 0
        success_msg_elapsed_ms = (
            0  # sum of round-trip times for successful messages only (ms)
        )
        payload_bytes = bytes(byte_count) if byte_count > 0 else b""
        monitor_from = get_epoch_ms()
        for i in range(TEST_MESSAGE_COUNT):
            succ = await send_msg("Z", my_addr, payload_bytes, target_addr, 1)
            if succ:
                success_count += 1
                success_msg_elapsed_ms += get_epoch_ms() - monitor_from
                logger.info(f"{my_addr} Successfully sent connectivity check message to {target_addr} (attempt {i+1}/{TEST_MESSAGE_COUNT})")
            else:
                logger.info(f"{my_addr} Failed to send connectivity check message to {target_addr} (attempt {i+1}/{TEST_MESSAGE_COUNT})")

            app_controller.create_and_send_message("radio_check", {"target_addr": target_addr, "attempt": i+1, "total_attempts": TEST_MESSAGE_COUNT,"result": "pass" if succ else "fail"}, timeout=0.5)

            monitor_from = get_epoch_ms()

        success_rate = (success_count / TEST_MESSAGE_COUNT) * 100
        # Convert ms to seconds; avoid divide-by-zero when no successes
        success_msg_elapsed_sec = (
            success_msg_elapsed_ms / 1000.0 if success_msg_elapsed_ms > 0 else 1.0
        )
        transfer_rate = (
            success_count / (success_msg_elapsed_sec * 2) if success_count > 0 else 0
        )  # as message is two-way (send + ack)
        return (success_count, success_rate, transfer_rate)

    async def check_network(self):
        if len(seen_neighbours) == 0:
            seen_neighbours_str = ",".join([str(n.get("node")) for n in seen_neighbours])
            network_paths_str = ",".join([str(p.get("path")) for p in network_paths])
            app_controller.create_and_send_message("check_network", {"message": f"No network to check, neighbours: {seen_neighbours_str if len(seen_neighbours_str) > 0 else 'None'}, shortest paths: {network_paths_str if len(network_paths_str) > 0 else 'None'}.", "seen_neighbours":seen_neighbours, "network_paths":network_paths}, timeout=0.5)
            return False
        else:
            app_controller.create_and_send_message("check_network", {"seen_neighbours":seen_neighbours, "network_paths":network_paths}, timeout=0.5)
            return True

    def list_images(self):
        # Returns live refs (no copy) to save RAM; caller must not mutate the lists.
        global trans_in_progress
        global db_store
        img_queued_list = db_store.get_image_list()
        try:
            return {
                "queued_list": img_queued_list,
                "sent_count": db_store.get_img_sent_count(),
                "dropped_count": db_store.get_img_dropped_count(),
                "failed_count": db_store.get_img_failed_count(),
                "progress": trans_in_progress,
            }
        except Exception as e:
            logger.error(f"[APP] error in list_images: {e}")
            return {"queued": [], "sent": [], "failed": [], "progress": False}

    def clear_all_queue(self):
        try:
            global db_store
            db_store.clear_image_list()
            return True
        except Exception as e:
            logger.error(f"[APP] error in clear_all_queue: {e}")
            return False

    def get_saved_logs(self):
        return logger.return_saved_logs_and_clear()

    def capture_image_to_verify_camera(self, type, quality=None):
        """
        Capture a JPEG image for verify_internet as bytes (no filesystem writes).
        """
        try:
            turn_ON_IR_emitter()
            img = sensor.snapshot()
            turn_OFF_IR_emitter()
            if img is None:
                return None
            if quality:
                jpeg_bytearray = img.compress(quality=quality)
            else:
                jpeg_bytearray = img.compress()
            del img
            gc.collect()
            return bytes(jpeg_bytearray)
        except Exception as e:
            app_controller.create_and_send_message("verify_internet", {"message": f"capture_image_to_verify_camera: {e}", "result": "fail"}, timeout=0.5)
            logger.error(f"[{type}] capture_image_to_verify_camera: {e} [Fail]")
            return None
        finally:
            turn_OFF_IR_emitter()

    async def try_create_cc(self):
        try:
            global internet_module, network_paths, tracx_uart_lock
            async with tracx_uart_lock:
                await internet_module.establish_internet(retry_count=2)
            if internet_module and internet_module.has_internet:
                network_paths = []
                app_controller.create_and_send_message("verify_internet", {"message": "Now established, device will act as CC now", "result": "pass"}, timeout=0.5)
                return True
            else:
                app_controller.create_and_send_message("verify_internet", {"message": f"Failed to create CC", "result": "fail"}, timeout=0.5)
                return False
        except Exception as e:
            app_controller.create_and_send_message("verify_internet", {"message": f"Error while initializing internet connection", "result": "fail"}, timeout=0.5)
            logger.error(f"[try_create_cc] error in try_create_cc: {e}")
            return False

    async def verify_internet_capture_and_upload(self):
        try:
            img_bytes = self.capture_image_to_verify_camera(type="verify_internet", quality=25)
            if not img_bytes:
                return False
            return await self.upload_verify_image_to_server(img_bytes)
        finally:
            img_bytes = None
            gc.collect()

    async def upload_verify_image_to_server(self, img_bytes):
        try:
            if not img_bytes:
                app_controller.create_and_send_message("verify_internet", {"message": "upload_verify_image_to_server: empty image bytes", "result": "fail"}, timeout=0.5)
                return False
            app_controller.create_and_send_message("verify_internet", {"message": f"image size: {round(len(img_bytes)/1024, 1)} kb"}, timeout=0.5)

            img_b64_str = ubinascii.b2a_base64(img_bytes).rstrip().decode()
            gc.collect()

            server_payload = {
                "machine_id": my_addr,
                "msg_typ": "event",
                "data": img_b64_str,
                "epoch_ms": get_epoch_ms(),
                "enc": False,
            }
            return await upload_payload_to_server(
                server_payload,
                "event",
                my_addr
            )

        except Exception as e:
            app_controller.create_and_send_message("verify_internet", {"message": f"upload_verify_image_to_server: {e}", "result": "fail"}, timeout=0.5)
            logger.error(f"[verify_internet] upload_verify_image_to_server: {e}")
            return False

    async def send_image_to_app(self):
        img_bytes = None
        try:
            img_bytes = self.capture_image_to_verify_camera(type="verify_image")
            if not img_bytes:
                logger.error("[verify_image] capture_image_to_verify_camera returned no data")
                return False

            total_size = len(img_bytes)
            CHUNK_SIZE = 4096
            total_chunks = (total_size + CHUNK_SIZE - 1) // CHUNK_SIZE
            app_controller.create_and_send_message("image_transfer_start",{"total_size": total_size,"total_chunks": total_chunks,}, timeout=1.0)

            for chunk_index in range(total_chunks):
                start = chunk_index * CHUNK_SIZE
                end = min(start + CHUNK_SIZE, total_size)
                chunk_data = img_bytes[start:end]
                chunk_b64_str = (ubinascii.b2a_base64(chunk_data).decode("utf-8").strip())

                app_controller.create_and_send_message("image_transfer_chunk",{"chunk_index": chunk_index, "chunk_size": end - start, "data": chunk_b64_str},timeout=5.0)
                await asyncio.sleep(0)

            app_controller.create_and_send_message( "image_transfer_end", {"total_size": total_size, "total_chunks": total_chunks,},timeout=1.0)
            logger.info(f"[verify_image] Sent verify image: {round(total_size/1024)} bytes in {total_chunks} chunks")
            return True

        except Exception as e:
            logger.error(f"[verify_image] send_image_to_app error: {e}")
            return False
        finally:
            img_bytes = None
            gc.collect()

async def enter_install_mode():
    global is_install_mode, app_controller
    if is_install_mode:
        logger.warning("[INSTALL] 𓆦𓆦𓆦𓆦𓆦𓆦❯❯ Already in install mode, ignoring req... ❮❮𓆦𓆦𓆦𓆦𓆦𓆦")
        return True

    logger.info("[INSTALL] 𓆦𓆦𓆦𓆦𓆦𓆦❯❯ Entering install mode... ❮❮𓆦𓆦𓆦𓆦𓆦𓆦")
    is_install_mode = True
    await app_controller.start()

    logger.info(f"[INSTALL] Waiting up to {INSTALL_MODE_WAIT_TIME}s for app connection...")
    await asyncio.sleep(INSTALL_MODE_WAIT_TIME)

    logger.info("[INSTALL] Checking if app/wifi_connection still alive after waittime...")
    while True:
        app_alive = app_controller.app_alive()
        if not app_alive:
            logger.info("[INSTALL] App/wifi_connection not alive, exiting install mode")
            break
        await asyncio.sleep(1)

    is_install_mode = False
    await app_controller.stop()
    # asyncio.create_task(send_msg("K", my_addr, b"ME_I", 100)) # Akash TODO check
    logger.info("[INSTALL] 𓆦𓆦𓆦𓆦𓆦𓆦❯❯ Install mode stopped/exited ❮❮𓆦𓆦𓆦𓆦𓆦𓆦")


async def exit_install_mode():
    global is_install_mode, app_controller
    if not is_install_mode:
        logger.warning("[INSTALL] 𓆦𓆦𓆦𓆦𓆦𓆦❯❯ Not in install mode, ignoring req... ❮❮𓆦𓆦𓆦𓆦𓆦𓆦")
        return

    logger.info("[INSTALL] 𓆦𓆦𓆦𓆦𓆦𓆦❯❯ Exiting install mode... ❮❮𓆦𓆦𓆦𓆦𓆦𓆦")
    is_install_mode = False
    await app_controller.stop()  # disconnects socket + wifi, breaks enter_install_mode loop via app_alive()
    logger.info("[INSTALL] 𓆦𓆦𓆦𓆦𓆦𓆦❯❯ Switched back to PROCESS MODE ❮❮𓆦𓆦𓆦𓆦𓆦𓆦")

# ---------------------------------------------------------------------------
# Application Entry Point
# ---------------------------------------------------------------------------
async def keep_blinking_restart_led():
    while True:
        await led_restart_blinker()
        await asyncio.sleep(6)


async def main():
    global app_handler, app_controller
    print(f"Entering MAIN loop... [PROCESS MODE]")
    # await led_restart_blinker()
    asyncio.create_task(keep_blinking_restart_led())

    if not await init_device():
        await reboot_device()

    # HEALTH STATS ===>
    asyncio.create_task(periodic_health_stats_loop())
    
    await init_tracx_internet()
    asyncio.create_task(keep_checking_internet())

    def clear_install_mode_flag():
        print(f"clear install mode flag")
        global is_install_mode
        is_install_mode = False

    app_handler = AppHandler()
    app_controller = AppController(
        app_handler,
        my_addr,
        on_install_mode_exit=clear_install_mode_flag
    )

    # RADIO, QUEUE =====>
    await init_lora()
    asyncio.create_task(lora_health_monitor())

    asyncio.create_task(radio_read())
    asyncio.create_task(process_packet_queue())  # Process queued packets asynchronously
    asyncio.create_task(keep_updating_gps())
    await asyncio.sleep(1)
    asyncio.create_task(network_request_loop())
    asyncio.create_task(keep_generating_heartbeat())
    asyncio.create_task(keep_generating_debugmsg())

    # IMAGE DETECTION =====>
    asyncio.create_task(person_detection_loop())

    # TRANSMISION =====>
    asyncio.create_task(image_sending_loop())

    if SAVE_LOGS:
        asyncio.create_task(logger_state())

    for i in range(24*7*4):  # 4 weeks
        await asyncio.sleep(3600)
        logger.info(f"Finished HOUR {i}")
        if i >= 6:
            logger.error(f"============= >>>>>> Rebooting device since it has been {i} HOURS <<<<<<< ====================")
            await reboot_device()

try:
    asyncio.run(main())
    # asyncio.run(main_radio())
except KeyboardInterrupt:
    logger.info("stopped by user via keyboard interrupt")
except Exception as e:
    logger.error(f"error in main.py: {e}")
finally:
    try:
        print(" SHUTTING DOWN - ")
        print(os.listdir(LOGS_DIR))
    except Exception as e:
        logger.error(f"error in main.py: {e}")
