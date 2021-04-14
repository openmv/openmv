# Blinky example

import time
from board import LED
LED_RED=1
LED_GREEN=2
LED_BLUE=3
LED_YELLOW=4

while (True):
    LED(LED_BLUE).on()
    time.sleep_ms(150)
    LED(LED_BLUE).off()
    time.sleep_ms(100)
    LED(LED_BLUE).on()
    time.sleep_ms(150)
    LED(LED_BLUE).off()
    time.sleep_ms(600)
