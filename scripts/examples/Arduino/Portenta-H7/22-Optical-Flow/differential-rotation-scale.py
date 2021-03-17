# Differential Optical Flow Rotation/Scale
#
# This example shows off using your OpenMV Cam to measure
# rotation/scale by comparing the current and the previous
# image against each other. Note that only rotation/scale is
# handled - not X and Y translation in this mode.

# To run this demo effectively please mount your OpenMV Cam on a steady
# base and SLOWLY rotate the camera around the lens and move the camera
# forward/backwards to see the numbers change.
# I.e. Z direction changes only.

import sensor, image, time, math

# NOTE!!! You have to use a small power of 2 resolution when using
# find_displacement(). This is because the algorithm is powered by
# something called phase correlation which does the image comparison
# using FFTs. A non-power of 2 resolution requires padding to a power
# of 2 which reduces the usefulness of the algorithm results. Please
# use a resolution like B64X64 or B64X32 (2x faster).

# Your OpenMV Cam supports power of 2 resolutions of 64x32, 64x64,
# 128x64, and 128x128. If you want a resolution of 32x32 you can create
# it by doing "img.pool(2, 2)" on a 64x64 image.

sensor.reset()                      # Reset and initialize the sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.B64X64) # Set frame size to 64x64... (or 64x32)...
sensor.skip_frames(time = 2000)     # Wait for settings take effect.
clock = time.clock()                # Create a clock object to track the FPS.

# Take from the main frame buffer's RAM to allocate a second frame buffer.
# There's a lot more RAM in the frame buffer than in the MicroPython heap.
# However, after doing this you have a lot less RAM for some algorithms...
# So, be aware that it's a lot easier to get out of RAM issues now.
extra_fb = sensor.alloc_extra_fb(sensor.width(), sensor.height(), sensor.GRAYSCALE)
extra_fb.replace(sensor.snapshot())

while(True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot() # Take a picture and return the image.

    # This algorithm is hard to test without a perfect jig... So, here's a cheat to see it works.
    # Put in a z_rotation value below and you should see the r output be equal to that.
    if(0):
        expected_rotation = 20.0
        extra_fb.rotation_corr(z_rotation=(-expected_rotation))

    # This algorithm is hard to test without a perfect jig... So, here's a cheat to see it works.
    # Put in a zoom value below and you should see the z output be equal to that.
    if(0):
        expected_zoom = 0.8
        extra_fb.rotation_corr(zoom=(2.00-expected_zoom))

    displacement = extra_fb.find_displacement(img, logpolar=True)
    extra_fb.replace(img)

    # Offset results are noisy without filtering so we drop some accuracy.
    rotation_change = int(math.degrees(displacement.rotation()) * 5) / 5.0
    zoom_amount = displacement.scale()

    if(displacement.response() > 0.1): # Below 0.1 or so (YMMV) and the results are just noise.
        print("{0:+f}r {1:+f}z {2} {3} FPS".format(rotation_change, zoom_amount, \
              displacement.response(),
              clock.fps()))
    else:
        print(clock.fps())
