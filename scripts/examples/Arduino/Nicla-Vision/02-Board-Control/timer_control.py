# Timer Control Example
#
# This example shows how to use a timer for callbacks.

import time
from pyb import Pin, Timer, LED

blue_led  = LED(3)

# we will receive the timer object when being called
# Note: functions that allocate memory are Not allowed in callbacks
def tick(timer):            
    blue_led.toggle()
    
tim = Timer(2, freq=1)      # create a timer object using timer 2 - trigger at 1Hz
tim.callback(tick)          # set the callback to our tick function

while (True):
    time.sleep_ms(1000)
