# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Pure Thermal Buzzer Example Script
#
# Thanks for buying the Pure Thermal OpenMV! This is a simple example
# script showing off how to control the buzzer onboard.

from pyb import Timer, Pin
import time

# Timer2, Channel2
tim2 = Timer(2, freq=4000)
ch2 = tim2.channel(2, Timer.PWM, pin=Pin("BUZZER"), pulse_width_percent=50)

# Play a Catchy Jingle by controlling the buzzer frequency and duty cycle
# using a lookup table of freq and duty cycle values.
jingle = [
    (1000, 50), (1200, 60), (1400, 70), (1600, 80),
    (1800, 90), (2000, 100), (2200, 90), (2400, 80),
    (2600, 70), (2800, 60), (3000, 50), (3200, 40),
]

while True:
    for freq, duty in jingle:
        tim2.freq(freq)
        ch2.pulse_width_percent(duty)
        time.sleep_ms(200)
