# Emboss Snapshot Example
#
# Note: You will need an SD card to run this example.
#
# You can use your OpenMV Cam to save modified image files.

import sensor, image, pyb

RED_LED_PIN = 1
BLUE_LED_PIN = 3

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QVGA) # or sensor.QQVGA (or others)
sensor.skip_frames(time = 2000) # Let new settings take affect.

pyb.LED(RED_LED_PIN).on()
sensor.skip_frames(time = 2000) # Give the user time to get ready.

pyb.LED(RED_LED_PIN).off()
pyb.LED(BLUE_LED_PIN).on()

print("You're on camera!")
img = sensor.snapshot()

img.morph(1, [+2, +1, +0,\
              +1, +1, -1,\
              +0, -1, -2]) # Emboss the image.

img.save("example.jpg") # or "example.bmp" (or others)

pyb.LED(BLUE_LED_PIN).off()
print("Done! Reset the camera to see the saved image.")
