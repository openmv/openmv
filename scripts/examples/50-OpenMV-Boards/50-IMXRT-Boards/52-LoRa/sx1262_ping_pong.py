# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# SX1262 LoRa Ping-Pong
#
# Bidirectional LoRa between two OpenMV RT1062 boards using the blocking
# micropython-lib sync API. Set NODE_ID to 0 on one board (pinger) and 1 on
# the other (ponger).
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

import time
from machine import Pin, SPI
from lora import SX1262

NODE_ID = 0  # Set to 1 on the second board.

lora_cfg = {
    "freq_khz": 915000,
    "sf": 9,
    "bw": "125",  # kHz
    "coding_rate": 7,  # 4/7
    "preamble_len": 8,
    "syncword": 0x12,
    "output_power": 14,  # dBm
    "crc_en": True,
}

modem = SX1262(
    spi=SPI(1, baudrate=2_000_000, polarity=0, phase=0),
    cs=Pin("P3"),
    busy=Pin("P7"),
    dio1=Pin("P13"),
    reset=Pin("P6"),
    dio3_tcxo_millivolts=1600,
    lora_cfg=lora_cfg,
)

if NODE_ID == 0:
    while True:
        modem.send(b"Ping")
        rx = modem.recv(timeout_ms=2000)
        print("RX:", rx)
        time.sleep(5)
else:
    while True:
        rx = modem.recv()
        if rx == b"Ping":
            modem.send(b"Pong")
