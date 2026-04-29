# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# ADC Read Example.
#
# This example shows how to use the ADC to read an analog pin.
# P8 and P9 are the only ADC-capable I/O pins on the OpenMV AE3.
# These pins are 1.8V tolerant (not 3.3V like the other I/O pins).

import time
from machine import ADC

adc = ADC("P8")  # Must be "P8" or "P9" on the OpenMV AE3.

while True:
    # The ADC has 16-bits of resolution for 65536 values.
    print("ADC = %fv" % ((adc.read_u16() * 1.8) / 65535))
    time.sleep_ms(100)
