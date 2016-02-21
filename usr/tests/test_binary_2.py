import pyb, sensor, image, math
sensor.reset()
sensor.set_framesize(sensor.QVGA)
sensor.set_pixformat(sensor.RGB565)
red_threshold = (0,100,   0,127,   0,127) # L A B
green_threshold = (0,100,   -128,0,   0,127) # L A B
blue_threshold = (0,100,   -128,127,   -128,0) # L A B
while(True):
    # Test red threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([red_threshold])
        sum, x, y, rads = img.orientation_radians()
        img.draw_keypoints([(x, y, rads)], color = (255, 0, 0), size = 20)
    # Test green threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([green_threshold])
        sum, x, y, rads = img.orientation_radians()
        img.draw_keypoints([(x, y, rads)], color = (0, 255, 0), size = 20)
    # Test blue threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([blue_threshold])
        sum, x, y, rads = img.orientation_radians()
        img.draw_keypoints([(x, y, rads)], color = (0, 0, 255), size = 20)
    # Test not red threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([red_threshold], invert = 1)
        sum, x, y, rads = img.orientation_radians()
        img.draw_keypoints([(x, y, rads)], color = (255, 0, 0), size = 20)
    # Test not green threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([green_threshold], invert = 1)
        sum, x, y, rads = img.orientation_radians()
        img.draw_keypoints([(x, y, rads)], color = (0, 255, 0), size = 20)
    # Test not blue threshold
    for i in range(100):
        img = sensor.snapshot()
        img.binary([blue_threshold], invert = 1)
        sum, x, y, rads = img.orientation_radians()
        img.draw_keypoints([(x, y, rads)], color = (0, 0, 255), size = 20)
