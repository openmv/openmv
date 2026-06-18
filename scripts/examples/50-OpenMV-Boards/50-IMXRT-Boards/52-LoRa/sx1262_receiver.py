# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# SX1262 LoRa Receiver
#
# Receives LoRa packets continuously and prints RSSI, SNR, and payload.
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
# Match radio parameters with the sender example.

from machine import SPI
from sx1262 import SX1262

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

while True:
    msg, status = sx.recv()
    if len(msg) > 0:
        print("RX:", msg)
        print("RSSI:", sx.getRSSI(), "dBm")
        print("SNR:", sx.getSNR(), "dB")
        print("Status:", SX1262.STATUS[status])
