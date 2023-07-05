# DAC Control Example
#
# This example shows how to use the DAC pin output onboard your OpenMV Cam.

import time
from pyb import DAC

dac = DAC("P6")  # Must always be "P6".

while True:
    # The DAC has 8-12 bits of resolution (default 8-bits).
    for i in range(256):
        dac.write(i)
        time.sleep_ms(20)
    for i in range(256):
        dac.write(255 - i)
        time.sleep_ms(20)
