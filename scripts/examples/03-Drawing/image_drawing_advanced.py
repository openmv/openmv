# Draw Image Testing script with bounce
#
# Exercise draw image with many different values for testing

import sensor, image, time, pyb

sensor.reset()
sensor.set_pixformat(sensor.RGB565) # or GRAYSCALE...
sensor.set_framesize(sensor.QQVGA) # or QQVGA...
sensor.skip_frames(time = 2000)
clock = time.clock()

BOUNCE = True
RESCALE = True

SMALL_IMAGE_SCALE = 3

CYCLE_FORMATS = True
CYCLE_MASK = True

# Used when CYCLE_FORMATS or CYCLE_MASK is true
value_mixer = 0

# Location of small image
x=100
y=50

# Bounce direction
xd=.1
yd=.1

# Small image scaling
rescale = 1.0
rd=0.01
max_rescale = 5
min_rescale = -max_rescale

# Boundary to bounce within
xmin = -sensor.width() / SMALL_IMAGE_SCALE - 8
ymin = -sensor.height() / SMALL_IMAGE_SCALE - 8
xmax = sensor.width() + 8
ymax = sensor.height() + 8

while(True):
    clock.tick()

    status = ""
    value_mixer = value_mixer + 1

    img = sensor.snapshot()
    # Makes a scaled copy of the sensor
    small_img = img.mean_pooled(SMALL_IMAGE_SCALE, SMALL_IMAGE_SCALE)

    status = 'rgb565 '
    if CYCLE_FORMATS:
        image_format = (value_mixer >> 8) & 3
        # To test combining different formats
        if (image_format==1): small_img = small_img.to_bitmap(copy=True); status = 'bitmap '
        if (image_format==2): small_img = small_img.to_grayscale(copy=True); status = 'grayscale '
        if (image_format==3): small_img = small_img.to_rgb565(copy=True); status = 'rgb565 '

    # update small image location
    if BOUNCE:
        x = x + xd
        if (x<xmin or x>xmax):
            xd = -xd

        y = y + yd
        if (y<ymin or y>ymax):
            yd = -yd

    # Update small image scale
    if RESCALE:
        rescale = rescale + rd
        if (rescale<min_rescale or rescale>max_rescale):
            rd = -rd

    # Find the center of the image
    scaled_width = int(small_img.width() * abs(rescale))
    scaled_height= int(small_img.height() * abs(rescale))
    draw_x = int(x - (scaled_width >> 1))
    draw_y = int(y - (scaled_height >> 1))

    apply_mask = CYCLE_MASK and ((value_mixer >> 9) & 1)
    if apply_mask:
        img.draw_image(small_img, draw_x, draw_y, mask=small_img.to_bitmap(copy=True), x_scale=-rescale, y_scale=rescale, alpha=240)
        status += 'alpha:240 '
        status += '+mask '
    else:
        img.draw_image(small_img, draw_x, draw_y, x_scale=-rescale, y_scale=rescale, alpha=128)
        status += 'alpha:128 '

    img.draw_string(8, 0, status, mono_space = False)

    print(clock.fps())
