# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# SPI Control
#
# This example shows how to use the SPI bus to directly drive the SSD1351
# OLED Breakout Board for the OpenMV AE3 without using the built-in display
# driver. To run this example you will need the OLED Breakout Board.

import csi
import time
import struct
from machine import Pin, SPI

WIDTH = 128
HEIGHT = 128

# OLED Breakout Board pin mapping on the OpenMV AE3.
cs = Pin("P3", Pin.OUT, value=1)
rs = Pin("P8", Pin.OUT, value=1)   # data/command select
rst = Pin("P7", Pin.OUT, value=1)  # reset

# The hardware SPI bus for the OpenMV AE3 OLED Breakout Board is SPI bus 0.
spi = SPI(0, baudrate=WIDTH * HEIGHT * 60 * 16, polarity=0, phase=0)


def write_command(cmd, *args):
    rs.low()
    cs.low()
    spi.write(bytes([cmd]))
    cs.high()
    rs.high()
    if args:
        cs.low()
        spi.write(bytes(args))
        cs.high()


def write_image(img):
    # Byte-swap RGB565 pixels into the SSD1351's expected big-endian order.
    pixels = struct.unpack("H" * (img.size() // 2), img)
    swapped = struct.pack(">" + "H" * len(pixels), *pixels)
    cs.low()
    spi.write(swapped)
    cs.high()


# Reset the LCD.
rst.low()
time.sleep_ms(100)
rst.high()
time.sleep_ms(100)

# SSD1351 init sequence.
write_command(0xFD, 0x12)              # COMMAND_LOCK: unlock IC MCU interface
write_command(0xFD, 0xB1)              # COMMAND_LOCK: unlock A2,B1,B3,BB,BE,C1
write_command(0xB2, 0xA4, 0x00, 0x00)  # DISPLAY_ENHANCEMENT
write_command(0xB3, 0xF0)              # CLOCK_DIV
write_command(0xCA, 0x7F)              # MUX_RATIO
write_command(0xA0, 0x74)              # SET_REMAP
write_command(0xA2, 0x00)              # DISPLAY_OFFSET
write_command(0xB1, 0x32)              # PRECHARGE
write_command(0xBB, 0x1F)              # PRECHARGE_LEVEL
write_command(0xC7, 0x0A)              # CONTRAST_MASTER
write_command(0xC1, 0xFF, 0xFF, 0xFF)  # CONTRAST_ABC
write_command(0xB6, 0x01)              # PRECHARGE2
write_command(0xAF)                    # DISPLAY_ON

csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.window((WIDTH, HEIGHT))

clock = time.clock()

while True:
    clock.tick()
    img = csi0.snapshot()

    write_command(0x5C)  # WRITE_RAM
    write_image(img)

    print(clock.fps())
