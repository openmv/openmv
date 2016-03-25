# Thermopile Shield Demo
#
# Note: To run this example you will need a Thermopile Shield for your OpenMV Cam.
#
# The Thermopile Shield allows your OpenMV Cam to see heat!

import sensor, image, time, fir

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QVGA) # or sensor.QQVGA (or others)
fir.init() # Initialize the thermal sensor

clock = time.clock() # Tracks FPS.

while(True):
    clock.tick()
    ta, ir, min_temp, max_temp = fir.read_ir()
    fir.display_ir(sensor.snapshot(), ir)
    print("FPS: %f - Ambient Temp: %f C - Min Temp %f C - Max Temp %f C" % \
        (clock.fps(), ta, min_temp, max_temp))
