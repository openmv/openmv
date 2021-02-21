# Himax motion detection example.

import sensor, image, time, pyb
from pyb import Pin, ExtInt

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)

# The sensor is less noisy with lower FPS.
sensor.set_framerate(15)

# Configure and enable motion detection
sensor.ioctl(sensor.IOCTL_HIMAX_MD_THRESHOLD, 0x01)
sensor.ioctl(sensor.IOCTL_HIMAX_MD_WINDOW, (0, 0, 320, 240))
sensor.ioctl(sensor.IOCTL_HIMAX_MD_CLEAR)
sensor.ioctl(sensor.IOCTL_HIMAX_MD_ENABLE, True)

motion_detected = False
def on_motion(line):
    global motion_detected
    motion_detected = True

led = pyb.LED(3)
# Configure external interrupt pin. When motion is detected, this pin is pulled high
ext = ExtInt(Pin("PC15"), ExtInt.IRQ_RISING, Pin.PULL_DOWN, on_motion)

clock = time.clock()
while(True):
    clock.tick()
    img = sensor.snapshot()
    if (motion_detected):
        led.on()
        time.sleep_ms(500)
        # Clear motion detection flag
        sensor.ioctl(sensor.IOCTL_HIMAX_MD_CLEAR)
        motion_detected = False
        led.off()
    print(clock.fps())
