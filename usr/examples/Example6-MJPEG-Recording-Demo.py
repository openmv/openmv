# Example 6 - MJPEG Recording Demo
#
# Note: You will need an SD card to run this demo.
#
# You can use your OpenMV Cam to record mjpeg files. You can either feed the
# recorder object JPEG frames or RGB565/Grayscale frames. Once you've finished
# recording a Mjpeg file you can use VLC to play it. If you are on Ubuntu then
# the built-in video player will work too.

import sensor, image, time, mjpeg, pyb

RED_LED_PIN = 1
BLUE_LED_PIN = 3

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QVGA) # or sensor.QQVGA (or others)
clock = time.clock() # Tracks FPS.

pyb.LED(RED_LED_PIN).on()
# We're letting the camera run for a bit here to make sure the image is ready.
for i in range(30):
    clock.tick()
    sensor.snapshot()

pyb.LED(RED_LED_PIN).off()
pyb.LED(BLUE_LED_PIN).on()

mjpeg = mjpeg.Mjpeg("demo.mjpeg")

print("You're on camera!")
for i in range(200):
    clock.tick()
    mjpeg.add_frame(sensor.snapshot())
    print(clock.fps())

mjpeg.close(clock.fps())
pyb.LED(BLUE_LED_PIN).off()
print("Done! Reset the camera to see the saved recording.")
