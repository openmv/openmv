# GIF Video Recording on Movement Example
#
# Note: You will need an SD card to run this example.
#
# You can use your OpenMV Cam to record gif files. You can either feed the
# recorder object RGB565 frames or Grayscale frames. Use photo editing software
# like GIMP to compress and optimize the Gif before uploading it to the web.
#
# This example demonstrates using frame differencing with your OpenMV Cam to do
# motion detection. After motion is detected your OpenMV Cam will take video.

import sensor, image, time, gif, pyb, os

RED_LED_PIN = 1
BLUE_LED_PIN = 3

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QQVGA) # or sensor.QVGA (or others)
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
        for blob_l in img.find_blobs([(20, 100, -128, 127, -128, 127)]):
            for blob in blob_l:
                # Over 100 pixels need to change to detect motion.
                if (diff and (blob[4] > 100)): diff -= 1

    g = gif.Gif("example-%d.gif" % pyb.rng(), loop=True)

    clock = time.clock() # Tracks FPS.
    print("You're on camera!")
    for i in range(100):
        clock.tick()
        # clock.avg() returns the milliseconds between frames - gif delay is in
        g.add_frame(sensor.snapshot(), delay=int(clock.avg()/10)) # centiseconds.
        print(clock.fps())

    g.close()
    pyb.LED(BLUE_LED_PIN).off()
    print("Restarting...")
