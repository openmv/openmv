# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows how to overlay a depth map onto frames
# captured from the main camera.
import csi
import image
import time
import tof

csi0 = csi.CSI()
csi0.reset()  # Reset and initialize the sensor.
csi0.pixformat(csi.RGB565)  # Set pixel format to RGB565 (or GRAYSCALE)
csi0.framesize(csi.VGA)  # Set frame size to QVGA (320x240)
csi0.framerate(30)
csi0.window((400, 400))  # Set window size to 240x240

# Initialize the ToF sensor
tof.init()

# FPS clock
clock = time.clock()

while True:
    clock.tick()

    # Capture an image
    img = csi0.snapshot()

    # Capture TOF data [depth map, min distance, max distance, target status].
    # min/max only cover zones with a valid target (status 5, 6 or 9).
    try:
        depth, dmin, dmax, status = tof.read_depth(vflip=True, hmirror=True)
    except RuntimeError:
        continue

    # Scale the image and blend it with the framebuffer. Zones without a
    # valid target are drawn as 0 when the status is passed.
    tof.draw_depth(
        img,
        depth,
        status=status,
        x_scale=img.width() / 8,
        y_scale=img.height() / 8,
        hint=image.BILINEAR,
        alpha=100,
        scale=(0, 4000),
        color_palette=image.PALETTE_DEPTH,
    )

    # Draw min and max distance and the number of zones with a valid target.
    valid = sum(s in (5, 6, 9) for s in status)
    img.draw_string(
        (0, 0),
        f"Distance min: {int(dmin):4d}mm max: {int(dmax):4d}mm zones: {valid:2d}/{len(status)}",
        color=(255, 0, 0),
    )

    # Print FPS.
    print(clock.fps())
