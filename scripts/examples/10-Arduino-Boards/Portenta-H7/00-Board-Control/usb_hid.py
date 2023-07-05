# Making OpenMV Camera act as a Mouse using HID.
#
# First we need to create boot.py file to change the default USB mode (VCP+MSC).
# Note: It is recommended to save this file to uSD card not the flash storage.
# This will make it easier to restore the default OpenMV (VCP+MSC) USB mode later
# by just deleting boot.py from uSD using the PC.
#
# Add the following script to boot.py:
#
# import pyb
# pyb.usb_mode('VCP+HID') # serial device + mouse (UNCOMMENT THIS LINE!)
# pyb.usb_mode('VCP+MSC') # serial device + storage device (default)
# pyb.usb_mode('VCP+HID', hid=pyb.hid_keyboard) # serial device + keyboard
#
# Copy boot.py to the root of the uSD card and restart the camera, it should now
# act as a serial device and a mouse.
#
# Connect to the camera using the IDE and run this script, you should see the mouse move.
#
# Note: To restore the default VCP+MSC USB mode, either use the PC to remove boot.py
# from the uSD card, or use the following Python line: import os; os.remove('boot.py')

import pyb
import time

hid = pyb.USB_HID()

while True:
    # x, y and scroll
    # move 10 pixels to the right
    hid.send((0, 10, 0, 0))
    time.sleep_ms(500)
