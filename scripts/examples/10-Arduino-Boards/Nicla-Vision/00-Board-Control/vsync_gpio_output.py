# VSYNC GPIO output example.
#
# This example shows how to toggle a pin on VSYNC interrupt.

import sensor
import time
from pyb import Pin

sensor.reset()                      # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)

# This pin will be toggled on/off on VSYNC rising and falling edges.
led_pin = Pin('LEDB', Pin.OUT_PP, Pin.PULL_NONE)
sensor.set_vsync_callback(lambda state, led=led_pin: led_pin.value(state))

clock = time.clock()                # Create a clock object to track the FPS.

while(True):
    clock.tick()                    # Update the FPS clock.
    img = sensor.snapshot()         # Take a picture and return the image.
    print(clock.fps())              # Note: OpenMV Cam runs about half as fast when connected
                                    # to the IDE. The FPS should increase once disconnected.
