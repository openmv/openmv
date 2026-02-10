# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# SPI Control
#
# This example shows how to use the SPI bus to directly control the LCD shield without
# using the built-in LCD driver. You will need the LCD shield to run this example.

import csi
import time
from machine import Pin, SPI
import struct

cs = Pin("P3", Pin.OUT)
rst = Pin("P7", Pin.OUT)
rs = Pin("P8", Pin.OUT)
# The hardware SPI bus for your OpenMV Cam is always SPI bus 1.

# NOTE: The SPI clock frequency will not always be the requested frequency. The hardware only supports
# frequencies that are the bus frequency divided by a prescaler (which can be 2, 4, 8, 16, 32, 64, 128 or 256).
spi = SPI(1, baudrate=int(1000000000 / 66), polarity=0, phase=0)


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
    cs.low()
    rs.high()
    reversed_img = struct.unpack('H' * (img.size() // 2), img)
    reversed_array = struct.pack('>' + 'H' * len(reversed_img), *reversed_img)
    spi.write(reversed_array)
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

csi0 = csi.CSI()
csi0.reset()  # Initialize the camera sensor.
csi0.pixformat(csi.RGB565)  # must be this
csi0.framesize(csi.QQVGA2)  # must be this
csi0.snapshot(time=2000)  # Let new settings take affect.

clock = time.clock()  # Tracks FPS.

while True:
    clock.tick()  # Track elapsed milliseconds between snapshots().
    img = csi0.snapshot()  # Take a picture and return the image.

    write_command(0x2C)  # Write image command...
    write_image(img)

    # Display On
    write_command(0x29)

    print(clock.fps())  # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
