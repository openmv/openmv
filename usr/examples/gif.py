# Gif recording example:
#
# You can use your OpenMV Cam to record gif files. You can either feed the
# recorder object RGB565 frames or Grayscale frames. Use photo editing software
# like GIMP to compress and optimize the Gif before uploading it to the web.

import sensor, image, time, gif

sensor.reset()
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.RGB565) # you can also use grayscale

# Warm up the cam
for i in range(10):
    sensor.snapshot()

# FPS clock
clock = time.clock()
gif = gif.Gif("/test.gif", loop=True) # video setup to use current resolution

for i in range(30):
    clock.tick()
    img = sensor.snapshot()
    gif.add_frame(img, delay=10) # centi seconds
    # Print FPS.
    # Note: Actual FPS is higher, the IDE slows down streaming.
    print(clock.fps())

gif.close()
print("done")
