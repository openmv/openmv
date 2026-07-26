# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# SX1262 LoRa Image Stream (UDP-style, max rate)
#
# Fire-and-forget JPEG snapshot stream over SX1262. No ACKs / retransmits
# (UDP-like). Sender blasts fragments at the radio's highest practical LoRa
# rate; receiver reassembles what arrives and drops incomplete/late frames.
#
# Set ROLE = "sender" on the camera that captures, "receiver" on the viewer.
#
# OpenMV RT1062 wiring (connect each OpenMV pin to the SX1262 module pin
# with the same function):
#   OpenMV P0  -> module MOSI  (SPI1, auto-muxed)
#   OpenMV P1  -> module MISO  (SPI1, auto-muxed)
#   OpenMV P2  -> module SCLK  (SPI1, auto-muxed)
#   OpenMV P3  -> module SS    (chip select / NSS)
#   OpenMV P6  -> module RST
#   OpenMV P7  -> module BUSY
#   OpenMV P13 -> module DIO1  (IRQ)
#
# Both boards must use the same radio parameters.
# Crystal-based modules: omit dio3_tcxo_millivolts=.
#
# Packet format (max 255 bytes total):
#   [0]=0x4C magic, [1]=frame_id, [2]=chunk_index, [3]=chunk_count, [4:]=data

import time
import csi
import image
from machine import Pin, SPI
from lora import SX1262

ROLE = "sender"  # "sender" or "receiver"

# Higher quality / resolution (still constrained by LoRa airtime).
FRAME_W = 160
FRAME_H = 120
JPEG_QUALITY = 50
MAX_JPEG = 12288
CHUNK_DATA = 251  # 255 - 4 byte header
# Tiny gap so the receiver radio can keep up (still no ACK).
CHUNK_GAP_MS = 2

# Drop a partial frame if the next chunk does not arrive in time (UDP-like).
RX_CHUNK_TIMEOUT_MS = 150
RX_IDLE_TIMEOUT_MS = 300

# Fastest settings allowed by micropython-lib lora-sx126x (SF min is 6).
lora_cfg = {
    "freq_khz": 915000,
    "sf": 6,  # fastest SF accepted by this driver (SF5 is rejected)
    "bw": "500",  # max bandwidth
    "coding_rate": 5,  # 4/5 least FEC overhead
    "preamble_len": 6,  # short preamble
    "syncword": 0x12,
    "output_power": 22,  # dBm — max for SX1262
    "crc_en": True,
}

MAGIC = 0x4C


def make_modem():
    return SX1262(
        spi=SPI(1, baudrate=2_000_000, polarity=0, phase=0),
        cs=Pin("P3"),
        busy=Pin("P7"),
        dio1=Pin("P13"),
        reset=Pin("P6"),
        dio3_tcxo_millivolts=1600,
        lora_cfg=lora_cfg,
    )


def send_frame(modem, frame_id, jpeg):
    # Blast chunks with no ACK (UDP-style).
    n = (len(jpeg) + CHUNK_DATA - 1) // CHUNK_DATA
    if n == 0 or n > 255:
        print("TX skip: bad chunk count", n)
        return False
    fid = frame_id & 0xFF
    for i in range(n):
        start = i * CHUNK_DATA
        part = jpeg[start : start + CHUNK_DATA]
        modem.send(bytes((MAGIC, fid, i, n)) + part)
        if CHUNK_GAP_MS:
            time.sleep_ms(CHUNK_GAP_MS)
    return True


def show_jpeg(jpeg):
    # Put JPEG in the IDE frame buffer and force a preview refresh.
    img = image.Image(FRAME_W, FRAME_H, image.JPEG, buffer=jpeg, copy_to_fb=True)
    img.flush()
    return img


def run_sender():
    modem = make_modem()
    cam = csi.CSI()
    cam.reset()
    cam.pixformat(csi.GRAYSCALE)
    cam.framesize(csi.QQVGA)  # 160x120
    cam.snapshot(time=500)

    frame_id = 0
    print("LoRa UDP-style image sender (max rate, no ACK)")
    while True:
        img = cam.snapshot().to_jpeg(quality=JPEG_QUALITY, copy=True)
        jpeg = bytes(img)
        size = len(jpeg)
        if size == 0 or size > MAX_JPEG:
            print("skip jpeg size", size)
            continue
        n = (size + CHUNK_DATA - 1) // CHUNK_DATA
        t0 = time.ticks_ms()
        ok = send_frame(modem, frame_id, jpeg)
        dt = time.ticks_diff(time.ticks_ms(), t0)
        if ok:
            print("TX frame", frame_id, "bytes", size, "chunks", n, "ms", dt)
        frame_id = (frame_id + 1) & 0xFF


def run_receiver():
    # Init CSI so the IDE frame-buffer streaming path is active, even though
    # we never snapshot — we only push received JPEGs into the FB.
    cam = csi.CSI()
    cam.reset()
    cam.pixformat(csi.GRAYSCALE)
    cam.framesize(csi.QQVGA)
    cam.snapshot(time=100)

    modem = make_modem()
    chunks = {}
    cur_id = None
    cur_total = 0
    last_rx_ms = time.ticks_ms()
    print("LoRa UDP-style image receiver (no ACK) — watch IDE frame buffer")
    while True:
        rx = modem.recv(timeout_ms=RX_IDLE_TIMEOUT_MS)
        now = time.ticks_ms()

        # Incomplete frame timed out — drop it.
        if cur_id is not None and time.ticks_diff(now, last_rx_ms) >= RX_CHUNK_TIMEOUT_MS:
            print("drop frame", cur_id, "got", len(chunks), "/", cur_total)
            chunks = {}
            cur_id = None
            cur_total = 0

        if rx is None or len(rx) < 5:
            continue

        magic, fid, idx, total = rx[0], rx[1], rx[2], rx[3]
        if magic != MAGIC or total == 0 or idx >= total:
            continue

        # New frame id while previous incomplete → drop previous (UDP).
        if cur_id is not None and fid != cur_id:
            if len(chunks) != cur_total:
                print("drop frame", cur_id, "got", len(chunks), "/", cur_total)
            chunks = {}
            cur_id = None
            cur_total = 0

        if cur_id is None:
            cur_id = fid
            cur_total = total
            chunks = {}

        if total != cur_total:
            continue

        chunks[idx] = bytes(rx[4:])
        last_rx_ms = now

        if len(chunks) != cur_total:
            continue

        try:
            jpeg = b"".join(chunks[i] for i in range(cur_total))
        except KeyError:
            print("drop frame", cur_id, "missing chunk")
            chunks = {}
            cur_id = None
            cur_total = 0
            continue

        done_id = cur_id
        chunks = {}
        cur_id = None
        cur_total = 0

        try:
            show_jpeg(jpeg)
            print("RX frame", done_id, "bytes", len(jpeg))
        except Exception as e:
            try:
                with open("lora_rx.jpg", "wb") as f:
                    f.write(jpeg)
                img = image.Image("lora_rx.jpg", copy_to_fb=True)
                img.flush()
                print("RX frame", done_id, "via file")
            except Exception as e2:
                print("display failed:", e, e2)


if ROLE == "sender":
    run_sender()
elif ROLE == "receiver":
    run_receiver()
else:
    raise ValueError('ROLE must be "sender" or "receiver"')
