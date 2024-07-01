import sensor, image, pyb, machine,time, sys


RED_LED_PIN = 1
BLUE_LED_PIN = 3

# Initialize the camera
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)
while True:
pyb.LED(RED_LED_PIN).on()
sensor.skip_frames(time=2000)



