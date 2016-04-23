# Marker Tracking Example
#
# This example shows how to use the find_markers function to merge blobs for
# different colors into one blob that represents a marker.
#
# Each blob that find_blobs returns has a bit in a bitmask set for the color
# that blob was produced by which was passed to find_blobs. E.g. if you pass
# find blobs 3 colors then you'll get blobs with possibly a color value of
# (2^0), (2^1), or (2^2). These color values can be or'ed togheter because
# they are a single bit each to represent a mutli-colored blob which you
# can then classify as a marker.

import sensor, image, time

# For color tracking to work really well you should ideally be in a very, very,
# very, controlled enviroment where the lighting is constant. Additionally, if
# you want to track more than 2 colors you need to set the boundaries for them
# very narrowly. If you try to track... generally red, green, and blue then
# you will end up just tracking everything which you don't want.
red_threshold   = (  40,   60,   60,   90,   50,   70)
blue_threshold  = (   0,   20,  -10,   30,  -60,   10)
# You may need to tweak the above settings for tracking red and blue things...
# Select an area in the Framebuffer to copy the color settings.

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565) # use RGB565.
sensor.set_framesize(sensor.QQVGA) # use QQVGA for speed.
sensor.skip_frames(10) # Let new settings take affect.
sensor.set_whitebal(False) # turn this off.
clock = time.clock() # Tracks FPS.

while(True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot() # Take a picture and return the image.

    blobs = img.find_blobs([red_threshold, blue_threshold])
    merged_blobs = img.find_markers(blobs)
    if merged_blobs:
        for b in merged_blobs:
            # Draw a rect around the blob.
            img.draw_rectangle(b[0:4]) # rect
            img.draw_cross(b[5], b[6]) # cx, cy
            # Draw the color label. b[8] is the color label.
            img.draw_string(b[0]+2, b[1]+2, "%d" % b[8])

    print(clock.fps()) # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
