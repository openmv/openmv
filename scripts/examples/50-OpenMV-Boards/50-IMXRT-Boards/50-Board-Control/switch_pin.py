# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Switch Pin
#
# The user switch on your OpenMV Cam is readable via the
# "SW" Pin. The pin is debounced in hardware via an RC circuit.
# So, you just need to read the pin state to get if the
# switch is pressed or not.

import machine

sw = machine.Pin("SW", machine.Pin.IN)
r = machine.LED("LED_RED")

while True:
    r.value(not sw.value())
