# This file is part of the OpenMV project.
#
# Copyright (c) 2025 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2025 Kwabena W. Agyeman <kwagyeman@openmv.io>
# Copyright (c) 2017 https://github.com/rdagger/micropython-ssd1351
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# SSD1351 LCD controller driver.

from micropython import const

WRITE_RAM = const(0x5C)
SET_REMAP = const(0xA0)
DISPLAY_OFFSET = const(0xA2)
DISPLAY_OFF = const(0xAE)
DISPLAY_ON = const(0xAF)
PRECHARGE = const(0xB1)
DISPLAY_ENHANCEMENT = const(0xB2)
CLOCK_DIV = const(0xB3)
PRECHARGE2 = const(0xB6)
PRECHARGE_LEVEL = const(0xBB)
CONTRAST_ABC = const(0xC1)
CONTRAST_MASTER = const(0xC7)
MUX_RATIO = const(0xCA)
COMMAND_LOCK = const(0xFD)


class SSD1351:

    def __init__(self):
        pass

    def init(self, dc):
        dc.bus_write(COMMAND_LOCK, 0x12)  # Unlock IC MCU interface
        dc.bus_write(COMMAND_LOCK, 0xB1)  # A2,B1,B3,BB,BE,C1
        dc.bus_write(DISPLAY_ENHANCEMENT, bytes([0xA4, 0x00, 0x00]))
        dc.bus_write(CLOCK_DIV, 0xF0)  # Clock divider F1 or F0
        dc.bus_write(MUX_RATIO, 0x7F)  # Mux ratio
        dc.bus_write(SET_REMAP, 0x74)  # Segment remapping
        dc.bus_write(DISPLAY_OFFSET, 0x00)  # Set display offset
        dc.bus_write(PRECHARGE, 0x32),  # Precharge
        dc.bus_write(PRECHARGE_LEVEL, 0x1F)  # Precharge level
        dc.bus_write(CONTRAST_MASTER, 0x0A)  # Contrast master
        dc.bus_write(CONTRAST_ABC, bytes([0xFF, 0xFF, 0xFF]))  # Contrast RGB
        dc.bus_write(PRECHARGE2, 0x01)  # Precharge2

    def ram_write(self, dc):
        dc.bus_write(WRITE_RAM)

    def display_on(self, dc):
        dc.bus_write(DISPLAY_ON)

    def display_off(self, dc):
        dc.bus_write(DISPLAY_OFF)
