# Blinky example

import time
from board import LED

led_red = LED(1)
led_green = LED(2)
led_blue = LED(3)
led_yellow = LED(4)

while True:
    led_blue.on()
    time.sleep_ms(250)
    led_blue.off()

    led_red.on()
    time.sleep_ms(250)
    led_red.off()

    led_green.on()
    time.sleep_ms(250)
    led_green.off()

    led_yellow.on()
    time.sleep_ms(250)
    led_yellow.off()

    time.sleep_ms(500)
