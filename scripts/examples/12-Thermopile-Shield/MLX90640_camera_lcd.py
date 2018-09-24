# MLX90640 Camera Demo
#
# This example shows off how to overlay a heatmap onto your OpenMV Cam's
# live video output from the main camera.

import image, time, fir, lcd

# Initialize the thermal sensor
fir.init(type=fir.FIR_MLX90640, refresh=16) # Hz (higher end OpenMV Cam's may be able to run faster)

# Init the lcd.
lcd.init()

# FPS clock
clock = time.clock()

while (True):
    clock.tick()

    img = fir.snapshot(copy_to_fb=True)

    lcd.display(img)

    # Print FPS.
    print(clock.fps())
