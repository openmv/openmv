# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# ADC Internal Channels Example
#
# This example shows how to read internal ADC channels.

import pyb

adc = pyb.ADCAll(12)
print(
    "VREF = %.1fv VBAT = %.1fv Temp = %d"
    % (adc.read_core_vref(), adc.read_core_vbat(), adc.read_core_temp())
)
