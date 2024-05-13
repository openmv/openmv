# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Thermal Overlay Demo
#
# This example shows off how to overlay a heatmap onto your OpenMV Cam's
# live video output from the main camera.

import sensor
import image
import time
import fir

# or image.BILINEAR or 0 (nearest neighbor)
drawing_hint = image.BICUBIC | image.CENTER | image.SCALE_ASPECT_KEEP

ALT_OVERLAY = False  # Set to True to allocate a second ir image.

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time=2000)

# Initialize the thermal sensor
fir.init()

# Allocate another frame buffer for smoother video.
extra_fb = sensor.alloc_extra_fb(sensor.width(), sensor.height(), sensor.RGB565)

# FPS clock
clock = time.clock()

while True:
    clock.tick()

    # Capture an image
    img = sensor.snapshot()

    # Capture FIR data
    #   ta: Ambient temperature
    #   ir: Object temperatures (IR array)
    #   to_min: Minimum object temperature
    #   to_max: Maximum object temperature
    try:
        ta, ir, to_min, to_max = fir.read_ir()
    except OSError:
        continue

    if not ALT_OVERLAY:
        # Scale the image and belnd it with the framebuffer
        fir.draw_ir(img, ir, hint=drawing_hint)
    else:
        # Create a secondary image and then blend into the frame buffer.
        extra_fb.clear()
        fir.draw_ir(extra_fb, ir, alpha=256, hint=drawing_hint)
        img.blend(extra_fb, alpha=128)

    # Draw ambient, min and max temperatures.
    img.draw_string(8, 0, "Ta: %0.2f C" % ta, color=(255, 0, 0), mono_space=False)
    img.draw_string(
        8, 8, "To min: %0.2f C" % to_min, color=(255, 0, 0), mono_space=False
    )
    img.draw_string(
        8, 16, "To max: %0.2f C" % to_max, color=(255, 0, 0), mono_space=False
    )

    # Force high quality streaming...
    img.to_jpeg(quality=90)

    # Print FPS.
    print(clock.fps())
