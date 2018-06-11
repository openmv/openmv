# LeNet Search Just Center Example
#
# LeNet is a convolutional nueral network designed to classify it's field of view into digits 0-9.
#
# In this example we slide the LeNet detector window over the image and get a list of activations
# where there might be an object. Note that use a CNN with a sliding window is extremely compute
# expensive so for an exhaustive search do not expect the CNN to be real-time.

import sensor, image, time, os, nn

sensor.reset()                         # Reset and initialize the sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)      # Set frame size to QVGA (320x240)
sensor.set_windowing((128, 128))       # Set 128x128 window.
sensor.skip_frames(time=500)           # Don't let autogain run very long.
sensor.set_auto_gain(False)            # Turn off autogain.
sensor.set_auto_exposure(False)        # Turn off whitebalance.

# Load lenet network (You can get the network from OpenMV IDE).
net = nn.load('/lenet.network')
labels = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9']

clock = time.clock()
while(True):
    clock.tick()

    img = sensor.snapshot()
    tmp_img = img.copy().binary([(150, 255)], invert=True)

    # net.search() will search an roi in the image for the network (or the whole image if the roi is not
    # specified). At each location to look in the image if one of the classifier outputs is larger than
    # threshold the location and label will be stored in an object list and returned. At each scale the
    # detection window is moved around in the ROI using x_overlap (0-1) and y_overlap (0-1) as a guide.
    # If you set the overlap to 0.5 then each detection window will overlap the previous one by 50%. Note
    # the computational work load goes WAY up the more overlap. Finally, for mult-scale matching after
    # sliding the network around in the x/y dimensions the detection window will shrink by scale_mul (0-1)
    # down to min_scale (0-1). For example, if scale_mul is 0.5 the detection window will shrink by 50%.
    # Note that at a lower scale there's even more area to search if x_overlap and y_overlap are small...
    # contrast_threshold skips running the CNN in areas that are flat.

    # Setting x_overlap=-1 forces the window to stay centered in the ROI in the x direction always. If
    # y_overlap is not -1 the method will search in all vertical positions.

    # Setting y_overlap=-1 forces the window to stay centered in the ROI in the y direction always. If
    # x_overlap is not -1 the method will serach in all horizontal positions.

    for obj in net.search(tmp_img, threshold=0.8, min_scale=0.4, scale_mul=0.8, \
            x_overlap=-1, y_overlap=-1, contrast_threshold=0.5):
        print("Detected %s - Confidence %f%%" % (labels[obj.index()], obj.value()))
        img.draw_rectangle(obj.rect())
    print(clock.fps())
