# Example 7 - Save Image Demo
#
# Note: You will need an SD card to run this demo.
#
# You can use your OpenMV Cam to save image files.

import sensor, image, pyb

RED_LED_PIN = 1
BLUE_LED_PIN = 3

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QVGA) # or sensor.QQVGA (or others)

pyb.LED(RED_LED_PIN).on()
# We're letting the camera run for a bit here to make sure the image is ready.
for i in range(30):
    sensor.snapshot()

pyb.LED(RED_LED_PIN).off()
pyb.LED(BLUE_LED_PIN).on()

sensor.snapshot().save("demo.jpg") # or "demo.bmp" (or others)

pyb.LED(BLUE_LED_PIN).off()
print("Done! Reset the camera to see the saved recording.")
