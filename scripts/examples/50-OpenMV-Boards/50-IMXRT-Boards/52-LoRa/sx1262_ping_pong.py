# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# SX1262 LoRa Ping-Pong
#
# Demonstrates bidirectional LoRa communication between two OpenMV RT1062 boards.
# Set NODE_ID to 0 on one board (pinger) and 1 on the other (ponger).
#
# OpenMV RT1062 wiring:
#   SCK  -> P2 (SPI1, auto-muxed)
#   MOSI -> P0 (SPI1, auto-muxed)
#   MISO -> P1 (SPI1, auto-muxed)
#   NSS  -> P3
#   BUSY -> P4
#   RST  -> P5
#   DIO1 -> P7
#
# Both boards must use the same radio parameters.
#
# Driver dependency (not frozen in firmware):
#   Copy all .py files from micropySX126X lib/ to the camera flash or SD card:
#   https://github.com/ehong-tl/micropySX126X/tree/master/lib
#   OpenMV RT1062 requires upstream HAL patches in sx126x.py (time import,
#   SPI(1) bus-only init, optional spi= kwarg). See upstream PR to micropySX126X.

import time
from machine import SPI
from sx1262 import SX1262

NODE_ID = 0  # Set to 1 on the second board.

SPI_BUS = 1
PIN_CS = "P3"
PIN_BUSY = "P4"
PIN_RST = "P5"
PIN_DIO1 = "P7"

FREQ = 915.0
BW = 125.0
SF = 9
CR = 7

spi = SPI(SPI_BUS, baudrate=2_000_000, polarity=0, phase=0)
sx = SX1262(SPI_BUS, 0, 0, 0, PIN_CS, PIN_DIO1, PIN_RST, PIN_BUSY, spi=spi)

sx.begin(
    freq=FREQ,
    bw=BW,
    sf=SF,
    cr=CR,
    syncWord=0x12,
    power=14,
    currentLimit=60.0,
    preambleLength=8,
    implicit=False,
    implicitLen=0xFF,
    crcOn=True,
    txIq=False,
    rxIq=False,
    tcxoVoltage=1.6,
    useRegulatorLDO=False,
    blocking=True,
)


def on_irq(events):
    if events & SX1262.RX_DONE:
        msg, err = sx.recv()
        print("RX:", msg, "status:", SX1262.STATUS[err])
        if NODE_ID == 1 and msg == b"Ping":
            sx.send(b"Pong")
    elif events & SX1262.TX_DONE:
        print("TX done.")


sx.setBlockingCallback(False, on_irq)

if NODE_ID == 0:
    while True:
        sx.send(b"Ping")
        time.sleep(5)
