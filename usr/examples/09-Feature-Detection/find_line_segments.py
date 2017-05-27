# Find Line Segments Example
#
# This example shows off how to find line segments in the image. For each line object
# found in the image a line object is returned which includes the line's rotation.

# Note: Line detection is done by using the Hough Transform:
# http://en.wikipedia.org/wiki/Hough_transform
# Please read about it above for more information on what `theta` and `rho` are.

enable_lens_corr = True # turn on for straighter lines...

import sensor, image, time

sensor.reset()
sensor.set_pixformat(sensor.RGB565) # grayscale is faster
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time = 2000)
clock = time.clock()

# All lines also have `x1()`, `y1()`, `x2()`, and `y2()` methods to get their end-points
# and a `line()` method to get all the above as one 4 value tuple for `draw_line()`.

while(True):
    clock.tick()
    img = sensor.snapshot()
    if enable_lens_corr: img.lens_corr(1.8) # for 2.8mm lens...

    # `threshold` controls how many lines in the image are found. Only lines with
    # edge difference magnitude sums greater than `threshold` are detected...

    # More about `threshold` - each pixel in the image contributes a magnitude value
    # to a line. The sum of all contributions is the magintude for that line. Then
    # when lines are merged their magnitudes are added togheter. Note that `threshold`
    # filters out lines with low magnitudes before merging. To see the magnitude of
    # un-merged lines set `theta_margin` and `rho_margin` to 0...

    # `theta_margin` and `rho_margin` control merging similar lines. If two lines
    # theta and rho value differences are less than the margins then they are merged.

    # Setting both the above to zero will greatly increase segment detection at the
    # cost of a lot of FPS. This is because when less lines are merged more pixels
    # are tested... which takes longer but covers more possibilities...

    # `segment_threshold` controls line segment extraction. It's a threshold on the
    # magnitude response per pixel under an infinite line. Pixels with a magnitude
    # above threshold are added to the line segment.

    # `find_line_segments` merges detected lines that are no more than 5 pixels apart
    # and no more than 15 degrees different to create nice continous line segments.

    for l in img.find_line_segments(threshold = 1000, theta_margin = 15, rho_margin = 15, segment_threshold = 100):
        img.draw_line(l.line(), color = (255, 0, 0))
        # print(l)

    print("FPS %f" % clock.fps())
