# Canny Edge and Hough Transform Example:
#
# This example demonstrates using the Canny edge detector
# And the Hough transform to find straight lines in an image.

import sensor, image, time
sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # or sensor.RGB565
sensor.set_framesize(sensor.QQVGA) # or sensor.QVGA (or others)
clock = time.clock() # Tracks FPS.

while(True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot() # Take a picture and return the image.
    img.find_edges(image.EDGE_CANNY, threshold=(50, 80))  # Find edges
    lines = img.find_lines(threshold=50) # Find lines.
    for l in lines: img.draw_line(l, color=(127)) # Draw lines
    print(clock.fps()) # Note: Your OpenMV Cam runs about half as fast while