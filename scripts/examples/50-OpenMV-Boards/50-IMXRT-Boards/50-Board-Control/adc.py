# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# ADC Read Example.
#
# This example shows how to use the ADC to read an analog pin.

import time
from machine import ADC

adc = ADC("P6")  # Must always be "P6".

while True:
    # The ADC has 16-bits of resolution for 65536 values.
    print("ADC = %fv" % ((adc.read_u16() * 3.3) / 65535))
    time.sleep_ms(100)
