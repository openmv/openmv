# Global Shutter Triggered Mode Example
#
# This example shows off setting the global shutter camera into triggered mode. In triggered mode
# snapshot() controls EXACTLY when integration of the camera pixels start such that you can sync
# taking pictures to some external movement. Since the camera captures all pixels at the same time
# (as it is a global shutter camera versus a rolling shutter camera) movement in the image will
# only be captured for the integration time and not the integration time multipled by the number
# of rows in the image. Additionally, sensor noise is reduced in triggered mode as the camera will
# not read out rows until after exposing which results in a higher quality image.
#
# That said, your maximum frame rate will be reduced by 2 to 3 as frames are no longer generated
# continously by the camera and because you have to wait for the integration to finish before
# readout of the frame.

import sensor
import time

sensor.reset()                      # Reset and initialize the sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # Set pixel format to GRAYSCALE
sensor.set_framesize(sensor.VGA)    # Set frame size to VGA (640x480)
sensor.skip_frames(time = 2000)     # Wait for settings take effect.
clock = time.clock()                # Create a clock object to track the FPS.

sensor.ioctl(sensor.IOCTL_SET_TRIGGERED_MODE, True)

while(True):
    clock.tick()                    # Update the FPS clock.
    img = sensor.snapshot()         # Take a picture and return the image.
    print(clock.fps())              # Note: OpenMV Cam runs about half as fast when connected
                                    # to the IDE. The FPS should increase once disconnected.
