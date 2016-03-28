# Mjpeg recording example:
#
# You can use your OpenMV Cam to record mjpeg files. You can either feed the
# recorder object JPEG frames or RGB565/Grayscale frames. Once you've finished
# recording a Mjpeg file you can use VLC to play it. If you are on Ubuntu then
# the built-in video player will work too.

import sensor, image, time, mjpeg

sensor.reset()
sensor.set_framesize(sensor.QVGA)
sensor.set_pixformat(sensor.RGB565) # you can also use grayscale

# Warm up the cam
for i in range(10):
    sensor.snapshot()

# FPS clock
clock = time.clock()
mjpeg = mjpeg.Mjpeg("/test.mjpeg") # video setup to use current resolution

for i in range(100):
    clock.tick()
    img = sensor.snapshot()
    mjpeg.add_frame(img)
    # Print FPS.
    # Note: Actual FPS is higher, the IDE slows down streaming.
    print(clock.fps())

mjpeg.close(clock.fps())
print("done")
