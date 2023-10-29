# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# ADC Read Example.
#
# This example shows how to use the ADC to read an analog pin.

import time
from pyb import ADC

adc = ADC("A0")

while True:
    # The ADC has 12-bits of resolution for 4096 values.
    print("ADC = %fv" % ((adc.read() * 3.3) / 4095))
    time.sleep_ms(100)
