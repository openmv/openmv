# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# SX1261 LoRa Receiver
#
# Receives LoRa packets and prints RSSI, SNR, CRC, and payload.
# Uses micropython-lib's lora-sx126x driver (frozen on OPENMV_RT1060).
#
# Use this example for SX1261 modules. For SX1262, use sx1262_receiver.py
# instead (different PA / max power). Radio params must match the peer.
#
# OpenMV RT1062 wiring (connect each OpenMV pin to the SX1261 module pin
# with the same function):
#   OpenMV P0  -> module MOSI  (SPI1, auto-muxed)
#   OpenMV P1  -> module MISO  (SPI1, auto-muxed)
#   OpenMV P2  -> module SCLK  (SPI1, auto-muxed)
#   OpenMV P3  -> module SS    (chip select / NSS)
#   OpenMV P6  -> module RST
#   OpenMV P7  -> module BUSY
#   OpenMV P13 -> module DIO1  (IRQ)
#
# Match radio parameters with the SX1261 sender example.
# Crystal-based modules: omit dio3_tcxo_millivolts=.

from machine import Pin, SPI
from lora import SX1261

lora_cfg = {
    "freq_khz": 915000,
    "sf": 9,
    "bw": "125",  # kHz
    "coding_rate": 7,  # 4/7
    "preamble_len": 8,
    "syncword": 0x12,
    "output_power": 14,  # dBm (SX1261 max is about +15)
    "crc_en": True,
}

modem = SX1261(
    spi=SPI(1, baudrate=2_000_000, polarity=0, phase=0),
    cs=Pin("P3"),
    busy=Pin("P7"),
    dio1=Pin("P13"),
    reset=Pin("P6"),
    dio3_tcxo_millivolts=1600,
    lora_cfg=lora_cfg,
)

while True:
    rx = modem.recv(timeout_ms=5000)
    if rx:
        print("RX:", rx, "RSSI:", rx.rssi, "SNR:", rx.snr, "CRC:", rx.valid_crc)
    else:
        print("timeout")
