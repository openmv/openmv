# Rotation Correction
#
# This example shows off how to use the rotation_corr() to undo perspective rotations
# in 3 dimensions along with zooming in and out on the image. While this demo rotates
# the image around for fun you can use this feature to fix perspective issues related
# to how your OpenMV Cam is mounted.

import sensor, image, time

# Degrees per frame to rotation by...
X_ROTATION_DEGREE_RATE = 5
Y_ROTATION_DEGREE_RATE = 0.5
Z_ROTATION_DEGREE_RATE = 0
X_OFFSET = 0
Y_OFFSET = 0

ZOOM_AMOUNT = 1 # Lower zooms out - Higher zooms in

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)
clock = time.clock()

x_rotation_counter = 0
y_rotation_counter = 0
z_rotation_counter = 0

while(True):
    clock.tick()

    img = sensor.snapshot().rotation_corr(x_rotation = x_rotation_counter, \
                                          y_rotation = y_rotation_counter, \
                                          z_rotation = z_rotation_counter, \
                                          x_translation = X_OFFSET, \
                                          y_translation = Y_OFFSET, \
                                          zoom = ZOOM_AMOUNT)

    x_rotation_counter += X_ROTATION_DEGREE_RATE
    y_rotation_counter += Y_ROTATION_DEGREE_RATE
    z_rotation_counter += Z_ROTATION_DEGREE_RATE

    print(clock.fps())
