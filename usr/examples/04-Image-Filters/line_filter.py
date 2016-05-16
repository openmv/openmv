# Line Filter Example
#
# The sensor module can preform some basic image processing during the image readout without
# Additional overhead. This example shows off how to apply some basic line filters in Python.
#
# WARNING - This feature does Not work fast enough on M4 when line pre-processing is implemented
# in Python. In the future this might be fixed somehow, for now You'll see a partial framebuffer.

import sensor, image, time

# Initialize the camera sensor.
sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)
clock = time.clock() # Tracks FPS.

# Copy source to destination.
# Note source is YUYV destination is 1BPP Grayscale
def line_filter_copy(src, dst):
    for i in range(0, len(dst), 1):
        dst[i] = src[i<<1]

# Segment the image by following thresholds.
# Note source is YUYV destination is 1BPP Grayscale
def line_filter_bw(src, dst):
    for i in range(0, len(dst), 1):
        if (src[i<<1] > 200 and src[i<<1] < 255):
            dst[i] = 0xFF
        else:
            dst[i] = 0x00

while(True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    lines = 0
    img = sensor.snapshot(line_filter = line_filter_copy) # Take a picture and return the image.
    #print(clock.fps()) # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
