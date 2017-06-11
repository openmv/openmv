# Image Writer Example
#
# USE THIS EXAMPLE WITH A USD CARD! Reset the camera after recording to see the file.
#
# This example shows how to use the Image Writer object to record snapshots of what your
# OpenMV Cam sees for later analysis using the Image Reader object. Images written to disk
# by the Image Writer object are stored in a simple file format readable by your OpenMV Cam.

import sensor, image, pyb, time

record_time = 10000 # 10 seconds in milliseconds

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time = 2000)
clock = time.clock()

img_writer = image.ImageWriter("/stream.bin")

# Red LED on means we are capturing frames.
red_led = pyb.LED(1)
red_led.on()

start = pyb.millis()
while pyb.elapsed_millis(start) < record_time:
    clock.tick()
    img = sensor.snapshot()
    # Modify the image if you feel like here...

    img_writer.add_frame(img)
    print(clock.fps())

img_writer.close()

# Blue LED on means we are done.
red_led.off()
blue_led = pyb.LED(3)
blue_led.on()

print("Done")
while(True):
    pyb.wfi()
