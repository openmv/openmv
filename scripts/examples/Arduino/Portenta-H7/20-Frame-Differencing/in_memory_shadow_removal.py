# In Memory Shadow Removal w/ Frame Differencing Example
#
# This example demonstrates using frame differencing with your OpenMV Cam using
# shadow removal to help reduce the affects of cast shadows in your scene.

import sensor, image, pyb, os, time

TRIGGER_THRESHOLD = 5

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QQVGA) # or sensor.QVGA (or others)
if sensor.get_id() == sensor.OV7725: # Reduce sensor PLL from 6x to 4x.
    sensor.__write_reg(0x0D, (sensor.__read_reg(0x0D) & 0x3F) | 0x40)
sensor.skip_frames(time = 2000) # Let new settings take affect.
sensor.set_auto_gain(False) # Turn this off too.
clock = time.clock() # Tracks FPS.

# Take from the main frame buffer's RAM to allocate a second frame buffer.
# There's a lot more RAM in the frame buffer than in the MicroPython heap.
# However, after doing this you have a lot less RAM for some algorithms...
# So, be aware that it's a lot easier to get out of RAM issues now. However,
# frame differencing doesn't use a lot of the extra space in the frame buffer.
# But, things like AprilTags do and won't work if you do this...
extra_fb = sensor.alloc_extra_fb(sensor.width(), sensor.height(), sensor.GRAYSCALE)

print("About to save background image...")
sensor.skip_frames(time = 2000) # Give the user time to get ready.
extra_fb.replace(sensor.snapshot())
print("Saved background image - Now frame differencing!")

while(True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot() # Take a picture and return the image.

    # Note that for shadow removal to work the background image must be
    # shadow free and have the same lighting as the latest image. Unlike max()
    # shadow removal won't remove all dark objects unless they were shadows...

    # Replace the image with the "abs(NEW-OLD)" frame difference.
    img.remove_shadows(extra_fb).difference(extra_fb)

    hist = img.get_histogram()
    # This code below works by comparing the 99th percentile value (e.g. the
    # non-outlier max value against the 90th percentile value (e.g. a non-max
    # value. The difference between the two values will grow as the difference
    # image seems more pixels change.
    diff = hist.get_percentile(0.99).l_value() - hist.get_percentile(0.90).l_value()
    triggered = diff > TRIGGER_THRESHOLD

    print(clock.fps(), triggered) # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
