# VSYNC GPIO output example.
#
# This example shows how to toggle the IR LED pin on VSYNC interrupt.

import sensor, image, time
from pyb import Pin

sensor.reset()                      # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.skip_frames(time = 2000)     # Wait for settings take effect.

# IR LED pin object
ir_led_pin = Pin('LED_IR', Pin.OUT_PP, Pin.PULL_NONE)
# This pin will be toggled on/off on VSYNC (start of frame) interrupt.
sensor.set_vsync_output(ir_led_pin)

clock = time.clock()                # Create a clock object to track the FPS.

while(True):
    clock.tick()                    # Update the FPS clock.
    img = sensor.snapshot()         # Take a picture and return the image.
    # Turn off the IR LED after snapshot.
    ir_led_pin.off()
    print(clock.fps())              # Note: OpenMV Cam runs about half as fast when connected
                                    # to the IDE. The FPS should increase once disconnected.
