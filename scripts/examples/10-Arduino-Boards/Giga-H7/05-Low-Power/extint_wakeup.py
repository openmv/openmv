# ExtInt Wake-Up from Stop Mode Example
# This example demonstrates using external interrupts to wake up from low-power mode.

import time
import machine
from machine import LED
from machine import Pin
from pyb import ExtInt


def callback(line):
    pass


led = LED("LED_BLUE")
pin = Pin("D0", Pin.IN, Pin.PULL_UP)
ext = ExtInt(pin, ExtInt.IRQ_FALLING, Pin.PULL_UP, callback=lambda line: None)

# Enter Stop Mode. Note the IDE will disconnect.
machine.sleep()

while True:
    led.on()
    time.sleep_ms(100)
    led.off()
    time.sleep_ms(100)
