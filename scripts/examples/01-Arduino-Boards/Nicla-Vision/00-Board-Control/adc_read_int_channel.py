# ADC Internal Channels Example
#
# This example shows how to read internal ADC channels.

import time, pyb

adc  = pyb.ADCAll(12)
print("VREF = %.1fv VBAT = %.1fv Temp = %d" % (adc.read_core_vref(), adc.read_core_vbat(), adc.read_core_temp()))
