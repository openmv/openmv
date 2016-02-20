import pyb, sensor, image, math
sensor.reset()
sensor.set_framesize(sensor.QVGA)
sensor.set_pixformat(sensor.GRAYSCALE)
low_threshold = (0, 50)
high_threshold = (205, 255)
while(True):
    # Test low threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([low_threshold])
        sum, x, y, rads = img.orientation_radians()
        img.draw_keypoints([(x, y, rads)], color = 127, size = 20)
    # Test high threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([high_threshold])
        sum, x, y, rads = img.orientation_radians()
        img.draw_keypoints([(x, y, rads)], color = 127, size = 20)
    # Test not low threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([low_threshold], invert = 1)
        sum, x, y, rads = img.orientation_radians()
        img.draw_keypoints([(x, y, rads)], color = 127, size = 20)
    # Test not high threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([high_threshold], invert = 1)
        sum, x, y, rads = img.orientation_radians()
        img.draw_keypoints([(x, y, rads)], color = 127, size = 20)
