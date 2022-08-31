# This example shows off how to use readout window control to readout a small part of a camera
# sensor pixel array at a very high speed and move that readout window around.

# This example is was designed and tested on the OpenMV Cam H7 Plus using the OV5640 sensor.

import sensor, image, time

EXPOSURE_MICROSECONDS = 1000
TRACKING_THRESHOLDS = [(128, 255)] # When you lower the exposure you darken everything.

SEARCHING_RESOLUTION = sensor.VGA
SEARCHING_AREA_THRESHOLD = 16
SEARCHING_PIXEL_THRESHOLD = SEARCHING_AREA_THRESHOLD

TRACKING_RESOLUTION = sensor.QQVGA
TRACKING_AREA_THRESHOLD = 256
TRACKING_PIXEL_THRESHOLD = TRACKING_AREA_THRESHOLD

TRACKING_EDGE_TOLERANCE = 0.05 # Blob can move 5% away from the center.

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

    # We need to find an IR object to track - it's likely to be really bright.
    blobs = img.find_blobs(TRACKING_THRESHOLDS,
                           area_threshold=SEARCHING_AREA_THRESHOLD,
                           pixels_threshold=SEARCHING_PIXEL_THRESHOLD)

    if len(blobs):
        most_dense_blob = max(blobs, key = lambda x: x.density())
        img.draw_rectangle(most_dense_blob.rect())

        def get_mapped_centroid(b):
            # By default the readout window is set the whole sensor pixel array with x/y==0.
            # The resolution you see if produced by taking pixels from the readout window on
            # the camera. The x/y location is relative to the sensor center.
            x, y, w, h = sensor.ioctl(sensor.IOCTL_GET_READOUT_WINDOW)

            # The camera driver will try to scale to fit whatever resolution you pass to max
            # width/height that fit on the sensor while keeping the aspect ratio.
            ratio = min(w / float(sensor.width()), h / float(sensor.height()))

            # Reference cx() to the center of the viewport and then scale to the readout.
            mapped_cx = (b.cx() - (sensor.width() / 2.0)) * ratio
            # Since we are keeping the aspect ratio there might be an offset in x.
            mapped_cx += (w - (sensor.width() * ratio)) / 2.0
            # Add in our displacement from the sensor center
            mapped_cx += x + (sensor_w / 2.0)

            # Reference cy() to the center of the viewport and then scale to the readout.
            mapped_cy = (b.cy() - (sensor.height() / 2.0)) * ratio
            # Since we are keeping the aspect ratio there might be an offset in y.
            mapped_cy += (h - (sensor.height() * ratio)) / 2.0
            # Add in our displacement from the sensor center
            mapped_cy += y + (sensor_h / 2.0)

            return (mapped_cx, mapped_cy) # X/Y location on the sensor array.

        def center_on_blob(b, res):
            mapped_cx, mapped_cy = get_mapped_centroid(b)

            # Switch to the res (if res was unchanged this does nothing).
            sensor.set_framesize(res)

            # Construct readout window. x/y are offsets from the center.
            x = int(mapped_cx - (sensor_w / 2.0))
            y = int(mapped_cy - (sensor_h / 2.0))
            w = sensor.width()
            h = sensor.height()

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

        center_on_blob(most_dense_blob, TRACKING_RESOLUTION)

        # This loop will track the blob at a much higher readout speed and lower resolution.
        while(True):
            clock.tick()
            img = sensor.snapshot()

            # Find the blob in the lower resolution image.
            blobs = img.find_blobs(TRACKING_THRESHOLDS,
                                   area_threshold=TRACKING_AREA_THRESHOLD,
                                   pixels_threshold=TRACKING_PIXEL_THRESHOLD)

            # If we loose the blob then we need to find a new one.
            if not len(blobs):
                # Reset resolution.
                sensor.set_framesize(SEARCHING_RESOLUTION)
                sensor.ioctl(sensor.IOCTL_SET_READOUT_WINDOW, (sensor_w, sensor_h))
                break

            # Narrow down the blob list and highlight the blob.
            most_dense_blob = max(blobs, key = lambda x: x.density())
            img.draw_rectangle(most_dense_blob.rect())

            print(clock.fps(), "BLOB cx:%d, cy:%d" % get_mapped_centroid(most_dense_blob))

            x_diff = most_dense_blob.cx() - (sensor.width() / 2.0)
            y_diff = most_dense_blob.cy() - (sensor.height() / 2.0)

            w_threshold = (sensor.width() / 2.0) * TRACKING_EDGE_TOLERANCE
            h_threshold = (sensor.height() / 2.0) * TRACKING_EDGE_TOLERANCE

            # Re-center on the blob if it starts going out of view (costs FPS).
            if abs(x_diff) > w_threshold or abs(y_diff) > h_threshold:
                center_on_blob(most_dense_blob, TRACKING_RESOLUTION)

    print(clock.fps())
