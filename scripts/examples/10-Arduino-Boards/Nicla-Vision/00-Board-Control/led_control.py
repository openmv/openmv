# LED Control Example
#
# This example shows how to control the RGB LED.
import time
from pyb import LED

red_led = LED(1)
green_led = LED(2)
blue_led = LED(3)


def led_control(x):
    if (x & 1) == 0:
        red_led.off()
    elif (x & 1) == 1:
        red_led.on()
    if (x & 2) == 0:
        green_led.off()
    elif (x & 2) == 2:
        green_led.on()
    if (x & 4) == 0:
        blue_led.off()
    elif (x & 4) == 4:
        blue_led.on()


while True:
    for i in range(16):
        led_control(i)
        time.sleep_ms(500)
