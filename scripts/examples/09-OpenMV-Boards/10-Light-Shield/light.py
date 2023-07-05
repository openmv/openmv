import time
from pyb import Pin, Timer

# 50kHz pin6 timer2 channel1
light = Timer(2, freq=50000).channel(1, Timer.PWM, pin=Pin("P6"))
light.pulse_width_percent(100)  # adjust light 0~100

while True:
    time.sleep_ms(1000)
