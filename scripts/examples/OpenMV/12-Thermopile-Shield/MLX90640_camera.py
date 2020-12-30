# MLX90640 Camera Demo
#
# This example shows off how to overlay a heatmap onto your OpenMV Cam's
# live video output from the main camera.

import sensor, image, time, fir

drawing_hint = image.BICUBIC # or image.BILINEAR or 0 (nearest neighbor)

# Initialize the thermal sensor
fir.init(type=fir.FIR_MLX90640)
w = fir.width() * 10
h = fir.height() * 10

# FPS clock
clock = time.clock()

while (True):
    clock.tick()

    try:
        img = fir.snapshot(x_size=w, y_size=h,
                           color_palette=sensor.PALETTE_IRONBOW, hint=drawing_hint,
                           copy_to_fb=True)
    except OSError:
        continue

    # Print FPS.
    print(clock.fps())
