# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# PWM Control Example
#
# This example shows how to do PWM with your OpenMV Cam.

import time
from machine import PWM

# P7 and P8 may share the same PWM module they need
# to have the same frequency.
p7 = PWM("P7", freq=100, duty_u16=32768)
p8 = PWM("P8", freq=100, duty_u16=32768)

# P9 and P10 may share the same PWM module they need
# to have the same frequency.
p9 = PWM("P9", freq=100, duty_u16=32768)
p10 = PWM("P10", freq=100, duty_u16=32768)

while True:
    for i in range(0, 65536, 256):
        p7.duty_u16(65535 - i)
        time.sleep_ms(10)
    p7.duty_u16(32768)

    for i in range(0, 65536, 256):
        p8.duty_u16(65535 - i)
        time.sleep_ms(10)
    p8.duty_u16(32768)

    for i in range(0, 65536, 256):
        p9.duty_u16(65535 - i)
        time.sleep_ms(10)
    p9.duty_u16(32768)

    for i in range(0, 65536, 256):
        p10.duty_u16(65535 - i)
        time.sleep_ms(10)
    p10.duty_u16(32768)
