# DAC Timed Write Example
#
# This example shows how to use the DAC pin output onboard your OpenMV Cam.

import math
from pyb import DAC

# create a buffer containing a sine-wave
buf = bytearray(100)
for i in range(len(buf)):
    buf[i] = 128 + int(127 * math.sin(2 * math.pi * i / len(buf)))

# output the sine-wave at 400Hz
dac = DAC("P6")
dac.write_timed(buf, 400 * len(buf), mode=DAC.CIRCULAR)
