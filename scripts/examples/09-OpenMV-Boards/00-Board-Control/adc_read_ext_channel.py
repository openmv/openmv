# ADC Read Example.
#
# This example shows how to use the ADC to read an analog pin.

import time
from pyb import ADC

adc = ADC("P6")  # Must always be "P6".

while True:
    # The ADC has 12-bits of resolution for 4096 values.
    print("ADC = %fv" % ((adc.read() * 3.3) / 4095))
    time.sleep_ms(100)
