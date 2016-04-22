# Line Following Example
#
# Making a line following robot requires a lot of effort. This example script
# shows how to do the computer vision part of the line following robot. You
# can use the output from this script to drive a differential drive robot to
# follow a line. This script just generates a single turn value that tells
# your robot to go left or right.
#
# For this script to work properly you should point the camera at a line at a
# 45 or so degree angle. Please make sure that only the line is within the
# camera's field of view.

import sensor, image, time, math

# Tracks a white line. Use [(0, 64)] for a tracking a black line.
GRAYSCALE_THRESHOLD = [(128, 255)]

# Each roi is (x, y, w, h). The line detection algorithm will try to find the
# centroid of the largest blob in each roi. The x position of the centroids
# will then be averaged with different weights where the most weight is assigned
# to the roi near the bottom of the image and less to the next roi and so on.
ROIS = [ # [ROI, weight]
        (0, 100, 160, 20, 0.7), # You'll need to tweak the weights for you app
        (0, 050, 160, 20, 0.3), # depending on how your robot is setup.
        (0, 000, 160, 20, 0.1)
       ]

# Compute the weight divisor
weight_sum = 0
for r in ROIS: weight_sum += r[4]

# Camera setup...
sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # use grayscale.
sensor.set_framesize(sensor.QQVGA) # use QQVGA for speed.
sensor.skip_frames(10) # Let new settings take affect.
sensor.set_whitebal(False) # turn this off.
clock = time.clock() # Tracks FPS.

while(True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot() # Take a picture and return the image.

    centroid_sum = 0
    for r in ROIS:
        blobs = img.find_blobs(GRAYSCALE_THRESHOLD, roi=r[0:4]) # r[0:4] is roi tuple.
        merged_blobs = img.find_markers(blobs) # merge overlapping blobs

        if merged_blobs:
            # Find the index of the blob with the most pixels.
            most_pixels = 0
            largest_blob = 0
            for i in range(len(merged_blobs)):
                if merged_blobs[i][4] > most_pixels:
                    most_pixels = merged_blobs[i][4] # [4] is pixels.
                    largest_blob = i

            # Draw a rect around the blob.
            img.draw_rectangle(merged_blobs[largest_blob][0:4]) # rect
            img.draw_cross(merged_blobs[largest_blob][5], # cx
                           merged_blobs[largest_blob][6]) # cy

            # [5] of the blob is the x centroid - r[4] is the weight.
            centroid_sum += merged_blobs[largest_blob][5] * r[4]

    center_pos = (centroid_sum / weight_sum) # Determine center of line.

    # Convert the center_pos to a deflection angle. We're using a non-linear
    # operation so that the response gets stronger the farther off the line we
    # are. Non-linear operations are good to use on the output of algorithms
    # like this to cause a response "trigger".
    deflection_angle = 0
    # The 80 is from half the X res, the 60 is from half the Y res. The
    # equation below is just computing the angle of a triangle where the
    # opposite side of the triangle is the deviation of the center position
    # from the center and the adjacent side is half the Y res. This limits
    # the angle output to around -45 to 45. (It's not quite -45 and 45).
    deflection_angle = -math.atan((center_pos-80)/60)

    # Convert angle in radians to degrees.
    deflection_angle = math.degrees(deflection_angle)

    # Now you have an angle telling you how much to turn the robot by which
    # incorporates the part of the line nearest to the robot and parts of
    # the line farther away from the robot for a better prediction.
    print("Turn Angle: %f" % deflection_angle)

    print(clock.fps()) # Note: Your OpenMV Cam runs about half as fast while
    # connected to your computer. The FPS should increase once disconnected.
