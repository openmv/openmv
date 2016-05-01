# Iris Detection 2 Example
#
# This example shows how to find the eye gaze (pupil detection) after finding
# the eyes in an image. This script uses the find_eyes function which determines
# the center point of roi that should contain a pupil. It does this by basically
# finding the center of the darkest area in the eye roi which is the pupil center.
#
# Note: This script does not detect a face first, use it with the telephoto lens.

import sensor, time, image

# Reset sensor
sensor.reset()

# Sensor settings
sensor.set_contrast(1)
sensor.set_gainceiling(16)
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

# Load Haar Cascade
# By default this will use all stages, lower satges is faster but less accurate.
eyes_cascade = image.HaarCascade("eye", stages=24)
print(eyes_cascade)

# FPS clock
clock = time.clock()

while (True):
    clock.tick()
    # Capture snapshot
    img = sensor.snapshot()
    # Find eyes !
    # Note: Lower scale factor scales-down the image more and detects smaller objects.
    # Higher threshold results in a higher detection rate, with more false positives.
    eyes = img.find_features(eyes_cascade, threshold=0.5, scale=1.5)

    # Find iris
    for e in eyes:
        iris = img.find_eye(e)
        img.draw_rectangle(e)
        img.draw_cross(iris[0], iris[1])

    # Print FPS.
    # Note: Actual FPS is higher, streaming the FB makes it slower.
    print(clock.fps())
