# Image Memory Stream I/O Example
#
# This example shows how to use the ImageIO stream to record frames in memory and play them back.
# Note: While this should work on any board, the board should have an SDRAM to be of any use.
import sensor, image, time

# Number of frames to pre-allocate and record
N_FRAMES = 500

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)

# This frame size must match the image size passed to ImageIO
sensor.set_windowing((120, 120))
sensor.skip_frames(time = 2000)

clock = time.clock()

# Write to memory stream
stream = image.ImageIO((120, 120, 2), N_FRAMES)

for i in range(0, N_FRAMES):
    clock.tick()
    stream.write(sensor.snapshot())
    print(clock.fps())

while (True):
    # Rewind stream and play back at 100FPS
    stream.seek(0)
    for i in range(0, N_FRAMES):
        img = stream.read(copy_to_fb=True)
        # Do machine vision algorithms on the image here.
        time.sleep_ms(10)
