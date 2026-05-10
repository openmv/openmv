# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# SPI Control
#
# This example shows how to use the SPI bus on your OpenMV Cam to directly
# drive the LCD shield without using the built-in lcd shield driver. You
# will need the LCD shield to run this example.

import csi
import time
import struct
from machine import Pin, SPI

WIDTH = 128
HEIGHT = 160

cs = Pin("P3", Pin.OUT)
rst = Pin("P7", Pin.OUT)
rs = Pin("P8", Pin.OUT)
# The hardware SPI bus for your OpenMV N6 LCD Shield is SPI bus 2.

spi = SPI(2, baudrate=15000000, polarity=0, phase=0)


def write_command_byte(c):
    cs.low()
    rs.low()
    spi.write(bytes([c]))
    cs.high()


def write_data_byte(c):
    cs.low()
    rs.high()
    spi.write(bytes([c]))
    cs.high()


def write_command(c, *data):
    write_command_byte(c)
    if data:
        for d in data:
            write_data_byte(d)


def write_image(img):
    # The LCD controller expects RGB565 with the high byte first while the
    # OpenMV image stores RGB565 as little-endian uint16, so byte-swap before
    # sending the pixels.
    pixels = struct.unpack("H" * (img.size() // 2), img)
    swapped = struct.pack(">" + "H" * len(pixels), *pixels)
    cs.low()
    rs.high()
    spi.write(swapped)
    cs.high()


# Reset the LCD.
rst.low()
time.sleep_ms(100)
rst.high()
time.sleep_ms(100)

write_command(0x11)  # Sleep Exit
time.sleep_ms(120)

# Memory Data Access Control
# Write 0xC8 for BGR mode.
write_command(0x36, 0xC0)

# Interface Pixel Format
write_command(0x3A, 0x05)

# Display On
write_command(0x29)

csi0 = csi.CSI()
csi0.reset()  # Initialize the camera sensor.
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)  # 320x200 on the OpenMV N6.
csi0.snapshot(time=2000)  # Let new settings take affect.

# Crop a 4:5 region (matching the 128x160 LCD aspect ratio) from the center
# of the 320x200 frame, then scale it down to the LCD size.
ROI = ((320 - 160) // 2, 0, 160, 200)
SCALE = WIDTH / ROI[2]

clock = time.clock()  # Tracks FPS.

while True:
    clock.tick()  # Track elapsed milliseconds between snapshots().
    img = csi0.snapshot()  # Take a picture and return the image.

    # img.scale() crops the source ROI first and then scales it.
    img = img.scale(roi=ROI, x_scale=SCALE, y_scale=SCALE)

    write_command(0x2C)  # Write image command...
    write_image(img)

    print(clock.fps())  # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
