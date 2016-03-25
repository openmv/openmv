# Thermopile Shield Demo 2
#
# Note: To run this example you will need a Thermopile Shield for your OpenMV
#       Cam and a LCD Shield.

import sensor, image, time, fir, lcd

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QVGA) # or sensor.QQVGA (or others)
fir.init() # Initialize the thermal sensor
lcd.init() # Initialize the lcd sensor

clock = time.clock() # Tracks FPS.

while(True):
    clock.tick()
    ta, ir, min_temp, max_temp = fir.read_ir()
    img = sensor.snapshot()
    fir.display_ir(img, ir) # draws on img
    lcd.display(img)
    print("FPS: %f - Ambient Temp: %f C - Min Temp %f C - Max Temp %f C" % \
        (clock.fps(), ta, min_temp, max_temp))
