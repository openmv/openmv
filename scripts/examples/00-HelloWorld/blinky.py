# Blinky example

import time
from machine import LED

led = LED("LED_BLUE")

while True:
    led.on()
    time.sleep_ms(500)
    led.off()
    time.sleep_ms(500)
