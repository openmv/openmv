import sensor, avi, time

REC_LENGTH = 15 # recording length in seconds

# Set sensor parameters
sensor.reset()
sensor.set_brightness(-2)
sensor.set_contrast(1)

sensor.set_framesize(sensor.QVGA)
sensor.set_pixformat(sensor.JPEG)

vid = avi.AVI("1:/test.avi", 320, 240, 15)

start = time.ticks()
clock = time.clock()

while ((time.ticks()-start) < (REC_LENGTH*1000)):
    clock.tick()
    # capture and store frame
    image = sensor.snapshot()
    vid.add_frame(image)
    print(clock.fps())

vid.flush()
