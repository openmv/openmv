# Pin Control Example
#
# This example shows how to use the I/O pins in GPIO mode.

from machine import Pin

# Connect a switch to pin 0 that will pull it low when the switch is closed.
# Pin 1 will then light up.
pin0 = Pin("D0", Pin.IN, Pin.PULL_UP)
pin1 = Pin("D1", Pin.OUT, Pin.PULL_NONE)

while True:
    pin1.value(not pin0.value())
