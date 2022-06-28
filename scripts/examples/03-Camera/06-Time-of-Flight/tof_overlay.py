# Time of Flight overlay Demo
#
# This example shows off how to overlay a depth map onto
# OpenMV Cam's live video output from the main camera.
import sensor, image, time, tof

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.set_windowing((0, 0, 240, 240))

# Initialize the ToF sensor
tof.init()

# FPS clock
clock = time.clock()

while (True):
    clock.tick()

    # Capture an image
    img = sensor.snapshot()

    # Capture TOF data [depth map, min distance, max distance]
    try:
        depth, dmin, dmax = tof.read_depth()
    except OSError:
        continue

    # Scale the image and belnd it with the framebuffer
    tof.draw_depth(img, depth, hint=image.BILINEAR,
            alpha=200, scale=(0, 4000), color_palette=tof.PALETTE_IRONBOW)

    # Draw min and max distance.
    img.draw_string(8, 0, "Min distance: %d mm" % dmin, color = (255, 0, 0), mono_space = False)
    img.draw_string(8, 8, "Max distance: %d mm" % dmax, color = (255, 0, 0), mono_space = False)

    # Force high quality streaming
    img.compress(quality=90)

    # Print FPS.
    print(clock.fps())
