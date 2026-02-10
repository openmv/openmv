# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# SPI Control
#
# This example shows how to use the SPI bus to control the
# 1.8" TFT LCD display (JD-T18003-T01) with ST7735R driver.

import csi
import time
from machine import Pin
from machine import SPI

cs = Pin("D2", Pin.OPEN_DRAIN)
rst = Pin("D3", Pin.OUT)
rs = Pin("D4", Pin.OUT)

# NOTE: The SPI clock frequency will not always be the requested frequency. The hardware only supports
# frequencies that are the bus frequency divided by a prescaler (which can be 2, 4, 8, 16, 32, 64, 128 or 256).
spi = SPI(5, baudrate=int(1000000000 / 66), polarity=0, phase=0)


def write_command_byte(c):
    cs.low()
    rs.low()
    spi.send(c)
    cs.high()


def write_data_byte(c):
    cs.low()
    rs.high()
    spi.send(c)
    cs.high()


def write_command(c, *data):
    write_command_byte(c)
    if data:
        for d in data:
            write_data_byte(d)


def write_image(img):
    cs.low()
    rs.high()
    spi.send(img)
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
csi0.framesize(csi.QQVGA2)
csi0.snapshot(time=2000)  # Let new settings take affect.
clock = time.clock()  # Tracks FPS.

while True:
    clock.tick()  # Track elapsed milliseconds between snapshots().
    img = csi0.snapshot()  # Take a picture and return the image.

    write_command(0x2C)  # Write image command...
    write_image(img)

    print(clock.fps())  # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
