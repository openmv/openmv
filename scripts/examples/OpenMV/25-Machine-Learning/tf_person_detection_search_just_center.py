# TensorFlow Lite Person Dection Example
#
# Google's Person Detection Model detects if a person is in view.
#
# In this example we slide the detector window over the image and get a list
# of activations. Note that use a CNN with a sliding window is extremely compute
# expensive so for an exhaustive search do not expect the CNN to be real-time.

import sensor, image, time, os, tf

sensor.reset()                         # Reset and initialize the sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)      # Set frame size to QVGA (320x240)
sensor.set_windowing((240, 240))       # Set 240x240 window.
sensor.skip_frames(time=2000)          # Let the camera adjust.

# Load the built-in person detection network (the network is in your OpenMV Cam's firmware).
net = tf.load('person_detection')
labels = ['person', 'no_person']

clock = time.clock()
while(True):
    clock.tick()

    img = sensor.snapshot()

    # net.classify() will run the network on an roi in the image (or on the whole image if the roi is not
    # specified). A classification score output vector will be generated for each location. At each scale the
    # detection window is moved around in the ROI using x_overlap (0-1) and y_overlap (0-1) as a guide.
    # If you set the overlap to 0.5 then each detection window will overlap the previous one by 50%. Note
    # the computational work load goes WAY up the more overlap. Finally, for multi-scale matching after
    # sliding the network around in the x/y dimensions the detection window will shrink by scale_mul (0-1)
    # down to min_scale (0-1). For example, if scale_mul is 0.5 the detection window will shrink by 50%.
    # Note that at a lower scale there's even more area to search if x_overlap and y_overlap are small...

    # Setting x_overlap=-1 forces the window to stay centered in the ROI in the x direction always. If
    # y_overlap is not -1 the method will search in all vertical positions.

    # Setting y_overlap=-1 forces the window to stay centered in the ROI in the y direction always. If
    # x_overlap is not -1 the method will serach in all horizontal positions.

    # default settings just do one detection... change them to search the image...
    for obj in net.classify(img, min_scale=0.5, scale_mul=0.5, x_overlap=-1, y_overlap=-1):
        print("**********\nDetections at [x=%d,y=%d,w=%d,h=%d]" % obj.rect())
        for i in range(len(obj.output())):
            print("%s = %f" % (labels[i], obj.output()[i]))
        img.draw_rectangle(obj.rect())
        img.draw_string(obj.x()+3, obj.y()-1, labels[obj.output().index(max(obj.output()))], mono_space = False)
    print(clock.fps(), "fps")
