# Read ADC Example
#
# This example shows how to read the ADC on your OpenMV Cam.

import time
from pyb import ADC

adc = ADC("P6") # Must always be "P6".

while(True):
    # The ADC has 12-bits of resolution for 4096 values.
    print("ADC = %fv" % ((adc.read() * 3.3) / 4095))
    time.sleep(100)
