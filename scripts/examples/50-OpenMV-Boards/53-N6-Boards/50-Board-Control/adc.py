# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# ADC Read Example.
#
# This example shows how to use the ADC to read an analog pin.
# On the OpenMV N6 the analog input on the P6 connector pin is routed
# through a level translator to a separate ADC channel. Use the "P6_ADC"
# alias to access it. "BAT_ADC" reads the battery voltage.

import time
from machine import ADC

adc = ADC("P6_ADC")  # Use "BAT_ADC" to read the battery voltage instead.

while True:
    # The ADC has 16-bits of resolution for 65536 values.
    print("ADC = %fv" % ((adc.read_u16() * 3.3) / 65535))
    time.sleep_ms(100)
