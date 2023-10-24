# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# LED Control Example
#
# This example shows how to control the RGB LED.
import time
from machine import LED

red_led = LED("LED_RED")
green_led = LED("LED_GREEN")
blue_led = LED("LED_BLUE")


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
