# Main Module Example
#
# When your OpenMV Cam is disconnected from your computer it will either run the
# main.py script on the SD card (if attached) or the main.py script on
# your OpenMV Cam's internal flash drive.

import time, pyb

led = pyb.LED(3)    # Red LED = 1, Green LED = 2, Blue LED = 3.
usb = pyb.USB_VCP() # This is a serial port object that allows you to communciate
                    # with your computer. While it is not open the code below runs.

while(not usb.isconnected()):
    led.on()
    time.sleep_ms(150)
    led.off()
    time.sleep_ms(100)
    led.on()
    time.sleep_ms(150)
    led.off()
    time.sleep_ms(600)

led = pyb.LED(2) # Switch to using the green LED.

while(usb.isconnected()):
    led.on()
    time.sleep_ms(150)
    led.off()
    time.sleep_ms(100)
    led.on()
    time.sleep_ms(150)
    led.off()
    time.sleep_ms(600)
