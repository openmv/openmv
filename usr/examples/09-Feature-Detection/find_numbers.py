# LetNet Example
import sensor, image, time

sensor.reset()                      # Reset and initialize the sensor.
sensor.set_contrast(3)
sensor.set_pixformat(sensor.GRAYSCALE) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.set_windowing((28, 28))

sensor.skip_frames(time = 2000)     # Wait for settings take effect.
sensor.set_auto_gain(False)
sensor.set_auto_exposure(False)

clock = time.clock()                # Create a clock object to track the FPS.

while(True):
    clock.tick()                    # Update the FPS clock.
    img = sensor.snapshot()         # Take a picture and return the image.
    out = img.invert().find_number()
    if out[1] > 3.0:
        print(out[0])
    #print(clock.fps())             # Note: OpenMV Cam runs about half as fast when connected
                                    # to the IDE. The FPS should increase once disconnected.
