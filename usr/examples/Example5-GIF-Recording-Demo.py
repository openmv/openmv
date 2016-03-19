# Example 5 - GIF Recording Demo
#
# Note: You will need an SD card to run this demo.
#
# You can use your OpenMV Cam to record gif files. You can either feed the
# recorder object RGB565 frames or Grayscale frames. Use photo editing software
# like GIMP to compress and optimize the Gif before uploading it to the web.

import sensor, image, time, gif, pyb

RED_LED_PIN = 1
BLUE_LED_PIN = 3

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QQVGA) # or sensor.QVGA (or others)
clock = time.clock() # Tracks FPS.

pyb.LED(RED_LED_PIN).on()
# We're letting the camera run for a bit here to make sure the image is ready.
for i in range(30):
    clock.tick()
    sensor.snapshot()

pyb.LED(RED_LED_PIN).off()
pyb.LED(BLUE_LED_PIN).on()

gif = gif.Gif("demo.gif", loop=True)

print("You're on camera!")
for i in range(100):
    clock.tick()
    # clock.avg() returns the milliseconds between frames - gif delay is in
    gif.add_frame(sensor.snapshot(), delay=int(clock.avg()/10)) # centiseconds.
    print(clock.fps())

gif.close()
pyb.LED(BLUE_LED_PIN).off()
print("Done! Reset the camera to see the saved recording.")
