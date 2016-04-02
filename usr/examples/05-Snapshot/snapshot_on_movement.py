# Snapshot on Movement Example
#
# Note: You will need an SD card to run this example.
#
# This example demonstrates using frame differencing with your OpenMV Cam to do
# motion detection. After motion is detected your OpenMV Cam will take picture.

import sensor, image, pyb, os

RED_LED_PIN = 1
BLUE_LED_PIN = 3

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QVGA) # or sensor.QQVGA (or others)
sensor.skip_frames(10) # Let new settings take affect.
sensor.set_whitebal(False) # Turn off white balance.

if not "temp" in os.listdir(): os.mkdir("temp") # Make a temp directory

while(True):

    pyb.LED(RED_LED_PIN).on()
    print("About to save background image...")
    sensor.skip_frames(60) # Give the user time to get ready.

    pyb.LED(RED_LED_PIN).off()
    sensor.snapshot().save("temp/bg.bmp")
    print("Saved background image - Now detecting motion!")
    pyb.LED(BLUE_LED_PIN).on()

    diff = 10 # We'll say we detected motion after 10 frames of motion.
    while(diff):
        img = sensor.snapshot()
        img.difference("temp/bg.bmp")
        img.binary([(20, 100, -128, 127, -128, 127)])
        sum = img.pixels()
        if sum > 100: # Over 100 pixels need to change to detect motion.
            diff -= 1

    pyb.LED(BLUE_LED_PIN).off()
    print("Movement detected! Saving image...")
    sensor.snapshot().save("temp/snapshot-%d.jpg" % pyb.rng()) # Save Pic.
