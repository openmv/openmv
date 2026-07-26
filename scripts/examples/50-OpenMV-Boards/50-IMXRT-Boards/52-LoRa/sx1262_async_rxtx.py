# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# SX1262 LoRa Async TX/RX
#
# Non-blocking send and receive using micropython-lib's AsyncSX1262 and
# asyncio. Both tasks run concurrently; send has priority over receive.
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
# Match radio parameters with the blocking sender/receiver examples.
# Crystal-based modules: omit dio3_tcxo_millivolts=.

import asyncio
from machine import Pin, SPI
from lora import AsyncSX1262

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

modem = AsyncSX1262(
    spi=SPI(1, baudrate=2_000_000, polarity=0, phase=0),
    cs=Pin("P3"),
    busy=Pin("P7"),
    dio1=Pin("P13"),
    reset=Pin("P6"),
    dio3_tcxo_millivolts=1600,
    lora_cfg=lora_cfg,
)


async def send_coro():
    n = 0
    while True:
        await modem.send(f"Hello OpenMV #{n}".encode())
        print("TX done:", n)
        await asyncio.sleep(1)
        n += 1


async def recv_coro():
    while True:
        rx = await modem.recv(5000)
        if rx:
            print("RX:", rx, "RSSI:", rx.rssi, "SNR:", rx.snr, "CRC:", rx.valid_crc)
        else:
            print("timeout")


async def main():
    await asyncio.gather(send_coro(), recv_coro())


asyncio.run(main())
