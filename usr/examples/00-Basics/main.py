import time, pyb

led = pyb.LED(3)
usb = pyb.USB_VCP()

while (usb.isconnected()==False):
   led.on()
   time.sleep(150)
   led.off()
   time.sleep(100)
   led.on()
   time.sleep(150)
   led.off()
   time.sleep(600)