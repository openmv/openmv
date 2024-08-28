# Untitled - By: kriti - Tue Jul 2 2024

import time, machine
import pyb 

uart = UART(1, 9600)                        
uart.init(9600, bits=8, parity=None, stop=1) 
RED_LED_PIN = 1
BLUE_LED_PIN = 3

while(True):
    pyb.LED(BLUE_LED_PIN).on()
    pyb.delay(1000)
    pyb.LED(BLUE_LED_PIN).off()
    pyb.delay(1000)
    pyb.LED(RED_LED_PIN).on()
    pyb.delay(1000)
    pyb.LED(RED_LED_PIN).off()
    pyb.delay(1000)
    # uart.write("bootloader running")


