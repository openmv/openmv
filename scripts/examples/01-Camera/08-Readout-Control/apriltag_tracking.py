# This example shows off how to use readout window control to readout a small part of a camera
# sensor pixel array at a very high speed and move that readout window around.

# This example is was designed and tested on the OpenMV Cam H7 Plus using the OV5640 sensor.

import sensor, image, time

# This example script forces the exposure to a constant value for the whole time. However, you may
# wish to dynamically adjust the exposure when the readout window shrinks to a small size.
EXPOSURE_MICROSECONDS = 20000

SEARCHING_RESOLUTION = sensor.QVGA
TRACKING_RESOLUTION = sensor.QQVGA # or sensor.QQQVGA

TRACKING_LOW_RATIO_THRESHOLD = 0.2 # Go to a smaller readout window when tag side vs res is smaller.
TRACKING_HIGH_RATIO_THRESHOLD = 0.8 # Go to a larger readout window when tag side vs res is larger.

sensor.reset()                         # Reset and initialize the sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # Set pixel format to GRAYSCALE
sensor.set_framesize(SEARCHING_RESOLUTION)
sensor.skip_frames(time = 1000)        # Wait for settings take effect.
clock = time.clock()                   # Create a clock object to track the FPS.

sensor.set_auto_gain(False)            # Turn off as it will oscillate.
sensor.set_auto_exposure(False, exposure_us=EXPOSURE_MICROSECONDS)
sensor.skip_frames(time = 1000)

# sensor_w and sensor_h are the image sensor raw pixels w/h (x/y are 0 initially).
x, y, sensor_w, sensor_h = sensor.ioctl(sensor.IOCTL_GET_READOUT_WINDOW)

while(True):
    clock.tick()
    img = sensor.snapshot()

    # Tracks TAG36H11 by default.
    tags = img.find_apriltags()

    if len(tags):
        best_tag = max(tags, key = lambda x: x.decision_margin())
        img.draw_rectangle(best_tag.rect())

        # This needs to be less than the sensor output at default so we can move it around.
        readout_window_w = ((sensor_w // sensor.width()) * sensor.width()) / 2
        readout_window_h = ((sensor_h // sensor.height()) * sensor.height()) / 2

        def get_mapped_centroid(t):
            # By default the readout window is set the whole sensor pixel array with x/y==0.
            # The resolution you see if produced by taking pixels from the readout window on
            # the camera. The x/y location is relative to the sensor center.
            x, y, w, h = sensor.ioctl(sensor.IOCTL_GET_READOUT_WINDOW)

            # The camera driver will try to scale to fit whatever resolution you pass to max
            # width/height that fit on the sensor while keeping the aspect ratio.
            ratio = min(w / float(sensor.width()), h / float(sensor.height()))

            # Reference cx() to the center of the viewport and then scale to the readout.
            mapped_cx = (t.cx() - (sensor.width() / 2.0)) * ratio
            # Since we are keeping the aspect ratio there might be an offset in x.
            mapped_cx += (w - (sensor.width() * ratio)) / 2.0
            # Add in our displacement from the sensor center
            mapped_cx += x + (sensor_w / 2.0)

            # Reference cy() to the center of the viewport and then scale to the readout.
            mapped_cy = (t.cy() - (sensor.height() / 2.0)) * ratio
            # Since we are keeping the aspect ratio there might be an offset in y.
            mapped_cy += (h - (sensor.height() * ratio)) / 2.0
            # Add in our displacement from the sensor center
            mapped_cy += y + (sensor_h / 2.0)

            return (mapped_cx, mapped_cy) # X/Y location on the sensor array.

        def center_on_tag(t, res):
            global readout_window_w
            global readout_window_h
            mapped_cx, mapped_cy = get_mapped_centroid(t)

            # Switch to the res (if res was unchanged this does nothing).
            sensor.set_framesize(res)

            # Construct readout window. x/y are offsets from the center.
            x = int(mapped_cx - (sensor_w / 2.0))
            y = int(mapped_cy - (sensor_h / 2.0))
            w = int(readout_window_w)
            h = int(readout_window_h)

            # Focus on the centroid.
            sensor.ioctl(sensor.IOCTL_SET_READOUT_WINDOW, (x, y, w, h))

            # See if we are hitting the edge.
            new_x, new_y, w, h = sensor.ioctl(sensor.IOCTL_GET_READOUT_WINDOW)

            # You can use these error values to drive servos to move the camera if you want.
            x_error = x - new_x
            y_error = y - new_y

            if x_error < 0: print("-X Limit Reached ", end="")
            if x_error > 0: print("+X Limit Reached ", end="")
            if y_error < 0: print("-Y Limit Reached ", end="")
            if y_error > 0: print("+Y Limit Reached ", end="")

        center_on_tag(best_tag, TRACKING_RESOLUTION)

        loss_count = 0

        # This loop will track the tag at a much higher readout speed and lower resolution.
        while(True):
            clock.tick()
            img = sensor.snapshot()

            # Tracks TAG36H11 by default.
            tags = img.find_apriltags()

            # If we loose the tag then we need to find a new one.
            if not len(tags):
                # Handle a few bad frames due to tag flicker.
                if (loss_count < 2):
                    loss_count += 1
                    continue
                # Reset resolution.
                sensor.set_framesize(SEARCHING_RESOLUTION)
                sensor.ioctl(sensor.IOCTL_SET_READOUT_WINDOW, (sensor_w, sensor_h))
                break

            loss_count = 0

            # Narrow down the blob list and highlight the blob.
            best_tag = max(tags, key = lambda x: x.decision_margin())
            img.draw_rectangle(best_tag.rect())

            print(clock.fps(), "TAG cx:%d, cy:%d" % get_mapped_centroid(best_tag))

            w_ratio = best_tag.w() / sensor.width()
            h_ratio = best_tag.h() / sensor.height()

            # Shrink the tracking window until the tag fits.
            while (w_ratio < TRACKING_LOW_RATIO_THRESHOLD) or (h_ratio < TRACKING_LOW_RATIO_THRESHOLD):
                readout_window_w /= 2
                readout_window_h /= 2
                w_ratio *= 2
                h_ratio *= 2

            # Enlarge the tracking window until the tag fits.
            while (TRACKING_HIGH_RATIO_THRESHOLD < w_ratio) or (TRACKING_HIGH_RATIO_THRESHOLD < h_ratio):
                readout_window_w *= 2
                readout_window_h *= 2
                w_ratio /= 2
                h_ratio /= 2

            center_on_tag(best_tag, TRACKING_RESOLUTION)

    print(clock.fps())
