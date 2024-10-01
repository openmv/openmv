# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows how to overlay a depth map onto frames
# captured from the main camera.
import sensor
import image
import time
import tof

sensor.reset()  # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.VGA)  # Set frame size to QVGA (320x240)
sensor.set_framerate(30)
sensor.set_windowing((400, 400))  # Set window size to 240x240

# Initialize the ToF sensor
tof.init()

# FPS clock
clock = time.clock()

while True:
    clock.tick()
    # Capture an image
    img = sensor.snapshot()
    # Capture TOF data [depth map, min distance, max distance]
    try:
        depth, dmin, dmax = tof.read_depth(vflip=True, hmirror=True)
    except RuntimeError:
        tof.reset()
        continue

    # Scale the image and belnd it with the framebuffer
    tof.draw_depth(
        img,
        depth,
        x_scale=img.width() / 8,
        y_scale=img.height() / 8,
        hint=image.BILINEAR,
        alpha=100,
        scale=(0, 4000),
        color_palette=tof.PALETTE_RAINBOW,
    )

    # Draw min and max distance.
    img.draw_string(
        0, 0, f"Distance min: {int(dmin):4d}mm max: {int(dmax):4d}mm", color=(255, 0, 0), mono_space=False
    )

    # Print FPS.
    print(clock.fps())
