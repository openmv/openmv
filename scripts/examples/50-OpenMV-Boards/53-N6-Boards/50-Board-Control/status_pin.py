# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Status Pin
#
# The status pin is an internal Pin on your OpenMV Cam driven by the
# battery management IC. It indicates whether the system is running
# from VIN/USB. The green light reflects the pin state.

import machine

st = machine.Pin("ST", machine.Pin.IN)
g = machine.LED("LED_GREEN")

while True:
    g.value(st.value())
