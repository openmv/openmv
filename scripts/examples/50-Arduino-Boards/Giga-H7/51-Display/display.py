# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# LCD, touch panel and camera example.

import sensor
import time
import image
import display
from gt911 import GT911
from machine import I2C

IMG_OFFSET = 80
touch_detected = False
points_colors = ((255, 0, 0), (0, 255, 0), (0, 0, 255), (0, 255, 255), (255, 255, 0))

sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.VGA)  # Set frame size to QVGA (320x240)

lcd = display.DSIDisplay(
    framesize=display.FWVGA, portrait=True, refresh=60, controller=display.ST7701()
)

# Note use pin numbers or names not Pin objects because the
# driver needs to change pin directions to reset the controller.
touch = GT911(
    I2C(4, freq=400_000),
    reset_pin="PI2",
    irq_pin="PI1",
    touch_points=5,
    refresh_rate=240,
    reverse_x=True,
    touch_callback=lambda pin: globals().update(touch_detected=True),
)

# Create a clock object to track the FPS.
clock = time.clock()

while True:
    clock.tick()  # Update the FPS clock.

    # Capture a new frame
    img = sensor.snapshot()

    # Draw touch points if touch was detected.
    if touch_detected:
        n, points = touch.read_points()
        for i in range(0, n):
            img.draw_circle(
                points[i][0] - IMG_OFFSET,
                points[i][1],
                points[i][2] * 3,
                points_colors[points[i][3]],
                thickness=2,
            )
        touch_detected = False

    # Draw the image on the display.
    lcd.write(img, y=IMG_OFFSET, hint=image.TRANSPOSE | image.VFLIP)

    print(clock.fps())
