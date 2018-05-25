# Timer Control Example
#
# This example shows how to use a timer for callbacks.

import time
from pyb import Pin, Timer

def tick(timer):            # we will receive the timer object when being called
    print("Timer callback")
    
tim = Timer(4, freq=1)      # create a timer object using timer 4 - trigger at 1Hz
tim.callback(tick)          # set the callback to our tick function

while (True):
    time.sleep(1000)