# SPI Control
#
# This example shows how to use the SPI bus on your OpenMV Cam to directly
# the LCD shield without using the built-in lcd shield driver. You will need
# the LCD shield to run this example.

import sensor
import time
from pyb import Pin, SPI

cs = Pin("P3", Pin.OUT_OD)
rst = Pin("P7", Pin.OUT_PP)
rs = Pin("P8", Pin.OUT_PP)
# The hardware SPI bus for your OpenMV Cam is always SPI bus 2.

# NOTE: The SPI clock frequency will not always be the requested frequency. The hardware only supports
# frequencies that are the bus frequency divided by a prescaler (which can be 2, 4, 8, 16, 32, 64, 128 or 256).
spi = SPI(2, SPI.MASTER, baudrate=int(1000000000 / 66), polarity=0, phase=0)


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

sensor.reset()  # Initialize the camera sensor.
sensor.set_pixformat(sensor.GRAYSCALE)  # must be this
sensor.set_framesize(sensor.QQVGA2)  # must be this
sensor.skip_frames(time=2000)  # Let new settings take affect.
clock = time.clock()  # Tracks FPS.

while True:
    clock.tick()  # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot()  # Take a picture and return the image.

    write_command(0x2C)  # Write image command...
    write_image(img)

    print(clock.fps())  # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
