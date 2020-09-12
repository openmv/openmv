# MLX90640 Camera Demo
#
# This example shows off how to overlay a heatmap onto your OpenMV Cam's
# live video output from the main camera.

import image, time, fir

# Initialize the thermal sensor
fir.init(type=fir.FIR_MLX90640, refresh=32) # 16Hz, 32Hz or 64Hz.

# FPS clock
clock = time.clock()

while (True):
    clock.tick()

    img = fir.snapshot(copy_to_fb=True)

    # Print FPS.
    print(clock.fps())
