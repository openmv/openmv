# MLX90621 Camera Demo
#
# This example shows off how to overlay a heatmap onto your OpenMV Cam's
# live video output from the main camera.

import image, time, fir

drawing_hint = image.BICUBIC # or image.BILINEAR or 0 (nearest neighbor)

# Initialize the thermal sensor
fir.init(type=fir.FIR_MLX90621)
w = fir.width() * 20
h = fir.height() * 20

# FPS clock
clock = time.clock()

while (True):
    clock.tick()

    try:
        img = fir.snapshot(x_size=w, y_size=h,
                           color_palette=fir.PALETTE_IRONBOW, hint=drawing_hint,
                           copy_to_fb=True)
    except OSError:
        continue

    # Print FPS.
    print(clock.fps())
