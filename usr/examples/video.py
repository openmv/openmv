import sensor, avi, time, led
REC_LENGTH = 10 # recording length in seconds

# Set sensor parameters
sensor.reset()
sensor.set_contrast(2)
sensor.set_framesize(sensor.VGA)
sensor.set_pixformat(sensor.JPEG)
sensor.set_quality(95)

led.off(led.RED)
time.sleep(5*1000)

path = "%d.mjpeg"%random(0, 1000)
vid = avi.AVI(path, 640, 480)

led.on(led.RED)
clock = time.clock()
start = time.ticks()
while ((time.ticks()-start) < (REC_LENGTH*1000)):
    clock.tick()
    image = sensor.snapshot()
    vid.add_frame(image)

vid.flush(int(clock.fps()))
led.off(led.RED)

while (True):
    led.toggle(led.BLUE)
    time.sleep(500)
    led.toggle(led.BLUE)
    time.sleep(500)
