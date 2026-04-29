# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Charge Pin
#
# The charge pin is an internal Pin on your OpenMV Cam driven by the
# battery management IC. It is asserted low while the attached battery is
# being charged. The blue light reflects the pin state.

import machine

chg = machine.Pin("CHG", machine.Pin.IN)
b = machine.LED("LED_BLUE")

while True:
    b.value(chg.value())
